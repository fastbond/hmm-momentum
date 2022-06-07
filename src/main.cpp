#pragma once

#include "HMM.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "opcodes.h"
#include "util.h"
#include "schedule.h"


/*
	Reminder: Still need to clean this up.
		-Take a look at this and standardize variable names.
		-Re-add cmd line input instead of hardcoded args
		-Make class instead of nested vectors
		-Could use some additional comments for clarity
*/



/*
 * Save a vector of doubles to file, one entry per line
 */
void saveHistory(const std::string& fname, std::vector<double>& history) {
	FILE* file = openFile(fname, "w");
	//fopen_s(&file, fname.c_str(), "w");

	if (file == NULL)
		return;

	for (double d : history) {
		fprintf(file, "%f\n", d);
	}

	fclose(file);
}



/*
 * Load a vector of doubles from file.  Expects one double per line
 */
std::vector<double> loadHistory(const std::string& fname) {
	std::vector<double> history;

	std::ifstream file(fname);
	if (file.is_open()) {
		double score;
		while (file >> score) {
			history.push_back(score);
		}

		file.close();
	}

	return history;
}



/*
 * Load max_families strings from a file
 */
std::vector<std::string> loadFamilyNames(const std::string& fname, int max_families) {
	std::vector<std::string> families;

	std::ifstream file(fname);
	if (file.is_open()) {
		std::string family;
		while (file >> family && families.size() < max_families) {
			families.push_back(family);
		}

		file.close();
	}

	return families;
}



/* 
 * Save label,score pairs to file
 */
void saveFamilyScores(const std::string& fname, std::vector<double>& scores, std::vector<int> labels) {
	FILE* file = openFile(fname, "w");
	//fopen_s(&file, fname.c_str(), "w");

	if (file == NULL)
		return;

	for (int i = 0; i < scores.size(); i++) {
		fprintf(file, "%d,%f\n", labels[i], scores[i]);
	}

	fclose(file);
}



/*
 * Save scores to file separated by commas.  Each line contains the true label, 
 * followed by the corresponding vector of scores separated by commas.  Also 
 * includes a header line with the entries in families. 
 *
 * Assumes matching lengths for each vector.
 * should make this more flexible with only some families etc but im tired
 */
void saveScores(const std::string& fname, std::vector<std::vector<double>>& scores, std::vector<int>& labels, const std::vector<std::string>& families) {
	FILE* file = openFile(fname, "w");
	//fopen_s(&file, fname.c_str(), "w");

	if (file == NULL)
		return;

	//header
	fprintf(file, "Class Id,");
	for (int i = 0; i < families.size() - 1; i++)
		fprintf(file, "%s,", families[i].c_str());
	fprintf(file, "%s\n", families[families.size() - 1].c_str());

	for (int i = 0; i < labels.size(); i++) {
		fprintf(file, "%d,", labels[i]);
		for (int family = 0; family < scores.size(); family++) {
			fprintf(file, "%f", scores[family][i]);
			if (family != scores.size() - 1)
				fprintf(file, ",");
		}

		fprintf(file, "\n");
	}

	fclose(file);
}




/*
 * Create a training set from the given sequences based on trainStart, trainEnd.  
 */
std::vector<std::vector<int>> makeTrainSet(std::vector<std::vector<int>>& sequences, int T, int trainStart, int trainEnd) {
	std::vector<std::vector<int>> trainSequences;
	int t = 0;
	for (int i = trainStart; i < trainEnd; i++) {
		std::vector<int>& obsSeq = sequences[i];
		if (t + obsSeq.size() <= T) {
			trainSequences.push_back(sequences[i]);
			t += obsSeq.size();
		}
		else if (t < T) {
			trainSequences.push_back(std::vector<int>(T - t, 0));
			std::vector<int>& seq = trainSequences[trainSequences.size() - 1];
			for (int o = 0; o < T - t; o++)
				seq[o] = obsSeq[o];
			t = T;
			break;
		}
		else {
			std::cout << "ERROR: BAD SPLIT.  TRAIN SPLIT WAS LARGER THAN EXPECTED" << std::endl;
		}
	}

	return trainSequences;
}



/*
 * Increment the current train/test split by manipulating each family's trainStart,trainEnd in trainRanges.
 */
void nextSplit(std::vector<std::vector<int>>& trainRanges, std::vector<std::vector<std::vector<int>>>& observationSequences, int T) {
	int num_families = observationSequences.size();
	for (int family = 0; family < num_families; family++) {
		int start = trainRanges[family][1]; //start at prev end
		int end = start;
		int t = 0;
		bool reset = false;
		while (t < T) { //also need to check if end is > total num of sequences in that family
			if (end >= observationSequences[family].size()) {
				if (reset) {
					//std::cout << "FAILED TO TRAIN/TEST SPLIT: " << families[family] << std::endl;
					throw std::runtime_error(std::string("FAILED TO TRAIN/TEST SPLIT: family " + family));
				}
				shuffle(observationSequences[family].begin(), observationSequences[family].end(), std::default_random_engine(rand()));
				start = 0;
				end = start;
				t = 0;
				reset = true;
			}
			t += observationSequences[family][end].size();
			end++;
		}
		trainRanges[family][0] = start;
		trainRanges[family][1] = end;
	}
}



//Store and get from callback after?
//Or actively pass to a matrix during runtime?
class ModelStoreCallback : public Callback {
public:
	ModelStoreCallback(std::vector<int>& _testIters, std::vector<std::pair<HMM,double>>& _store) : testIters(_testIters) {
		store = &_store;
	};

	void onIterEnd(HMM& hmm, int iter, double score) {
		int idx = find(iter+1, testIters);
		if (idx == -1)
			return;

		if (score > (*store)[idx].second) {
			//(*store)[idx].first = hmm;
			(*store)[idx].first = HMM(hmm);
			(*store)[idx].second = score;
		}
	}

	std::vector<int> testIters;
	std::vector<std::pair<HMM, double>>* store;
};





class EntropyHistoryCallback : public Callback {
public:
	EntropyHistoryCallback(const std::string& fname) { 
		save_file = fname; 
	};

	void onTrainStart(HMM& hmm, int max_iters) {
		history.clear();
	}

	void onIterEnd(HMM& hmm, int iter, double score) {
		history.push_back(hmm.s_entropy());
	}

	void onTrainEnd(HMM& hmm, int iter, double score) {
		saveHistory(save_file, history);
	}

	std::vector<double> history;
	std::string save_file;
};






void classifyMalware() {
	//HMM settings
	int N = 15;
	int M = 30;
	int T = 10000;
	double momentum = 0.4;
	bool nesterov = true;
	double smoothing = 0.001;
	int iters = 300;
	int min_file_size = 1;
	Initialization init = Initialization::Random;

	//Experiment settings
	std::string description = "extra/malicia/varyN/TEST/";//"extended/test/multiIter/15 families/5 iters/";
	int runs = 100;
	int restarts = 1;
	bool compareToBase = false;
	std::vector<int> testIters = { 5,10,15,20,25,35,50,100,200,300 };
	int skipRuns = 0;
	
	//bool lazyLoad = false;	//just load all in memory with 1 pass, not implemented currently
	bool preloadOpcodeMap = true;
	bool saveNewOpcodeMap = true;
	std::string opcodeMapFile = "opcode_map_malicia.txt"; // "opcode_map_15of17.txt";

	//Data selection settings
	std::string data_dir = "data/malicia/"; //"data/malware/";//
	std::string family_list_file = "malicia_families.txt";//"test_families.txt";//"families.txt";//
	int top_families = 3;
	int top_opcodes = M;

	//Output settings
	std::string output_dir = "output/malware/" + description + "/";
	output_dir += "N=" + std::to_string(N);
	output_dir += " M=" + std::to_string(M);
	output_dir += " T=" + std::to_string(T);
	output_dir += " m=" + to_string_trimmed(momentum);
	output_dir += " nesterov=" + std::to_string((int)nesterov);
	output_dir += " smooth=" + to_string_trimmed(smoothing);
	output_dir += "/";

	//Read family file & get vector of family names
	std::cout << "Loading family names" << std::endl;
	std::vector<std::string> families = loadFamilyNames(family_list_file, top_families);

	// Load existing opcode to int mapping to use.
	// This needs to match the parameters used.  No error handling for bad choice implemented.
	std::map<std::string, int> opcodeMap;
	if (preloadOpcodeMap) {
		std::cout << "Loading opcode mapping" << std::endl;
		opcodeMap = loadOpcodeMap(opcodeMapFile);
	}

	// If it wasn't preloaded OR if it failed to load, then create mapping
	if (opcodeMap.size() == 0) {	
		if (preloadOpcodeMap)
			std::cout << "Failed to load opcode mapping" << std::endl;
		std::cout << "Generating opcode mapping" << std::endl;
		opcodeMap = getOpcodeMap(data_dir, families, top_opcodes);
		if (saveNewOpcodeMap)
			saveOpcodeMap(opcodeMap, opcodeMapFile);
	}

	//Create int sequences for each file in each family
	std::cout << "Creating observation sequences" << std::endl;
	std::vector<std::vector<std::vector<int>>> observationSequences = getObservationSequences(data_dir, families, opcodeMap, min_file_size);

	//Shuffle each family sequence prior to train/test splitting?
	//shuffle(familyObs.begin(), familyObs.end(), std::default_random_engine(rand()));

	//Start,end indices for the current train split for each family
	std::vector<std::vector<int>> trainRanges(families.size(), std::vector<int>(2, 0));


	//	For X times
	for (int run = 0; run < runs; run++) {
		std::cout << "\nRun #" << run << std::endl;

		srand((run*run + 69) * 1337); //i think srand(1) is weird?  //randomize this more if needed

		//Choose train/test sequences for this run for each family
		nextSplit(trainRanges, observationSequences, T);

		if (run < skipRuns)
			continue;

		// Models to score with at various iterations
		std::vector<std::vector<std::pair<HMM, double>>> models(families.size(), std::vector<std::pair<HMM, double>>(testIters.size(), std::make_pair(HMM(N, M), (double)-INFINITY)));

		//	train model(s) for each family
		for (int family = 0; family < families.size(); family++) {
			std::cout << "Family: " << families[family] << std::endl;

			//Create training set for this family
			std::vector<std::vector<int>> trainSequences = makeTrainSet(observationSequences[family], T, trainRanges[family][0], trainRanges[family][1]);

			ModelStoreCallback cb(testIters, models[family]);

			//	Need to consider how to do random seeding
			HMM bestModel(0, 0);
			double bestScore = -DBL_MAX;
			for (int restart = 0; restart < restarts; restart++) {
				std::cout << "Restart: " << restart << std::endl;
				std::vector<Callback*> callbacks;

				ClassicMomentum classic(momentum);
				NesterovMomentum nest(momentum);
				Momentum* mM;
				if (nesterov)
					mM = &nest;
				else
					mM = &classic;

				callbacks.push_back(&cb);

				EntropyHistoryCallback entropySaver(output_dir + std::to_string(run) + "_" + families[family] + "_" + std::to_string(restart) + "_hmmM_" + std::to_string(momentum) + "_entropy.csv");
				callbacks.push_back(&entropySaver);

				//int skip = 1;
				//SkipNSchedule sched(skip);
				//DisableSchedule sched(5, 30);
				//Skip1DisableSchedule sched(15, 50);
				//EntropySchedule sched(0.5, 1);
				//mM->addSchedule(sched);

				//	Set up HMM
				HMM hmmM(N, M); //momentum
				hmmM.initializeA(init, rand());
				hmmM.initializeB(init, rand());
				hmmM.initializePi(init, rand());

				//	Train HMM
				std::vector<double> history = hmmM.train(trainSequences, iters, *mM, smoothing, callbacks);

				//	Save score history
				//	TODO: Need to sort by description, run, etc
				saveHistory(output_dir + std::to_string(run) + "_" + families[family] + "_" + std::to_string(restart) + "_hmmM_" + std::to_string(momentum) + ".csv", history);

				double score;
				//if (history.size() == 0)
				//	score = -1000000;
				//else
				score = history[history.size() - 1];
				printf("Momentum = %f\n", score);

				if (score >= bestScore) {
					bestModel = hmmM;
					bestScore = score;
				}
			}
			

			printf("Best Score: %f\n", bestScore);

			//saveMatrix(bestModel.getA(), output_dir + "_" + families[family] + "_" + std::to_string(run) + "_xA_" + std::to_string(momentum) + ".csv");
			//saveMatrix(bestModel.getB(), output_dir + "_" + families[family] + "_" + std::to_string(run) + "_xB_" + std::to_string(momentum) + ".csv");
			//saveMatrix(bestModel.getPi(), output_dir + "_" + families[family] + "_" + std::to_string(run) + "_xPi_" + std::to_string(momentum) + ".csv");

		}

		// Get labels for all samples
		// Same for all families, so no need to recompute for each family
		std::vector<int> testLabels;                                
		for (int family = 0; family < families.size(); family++) {
			int trainStart = trainRanges[family][0];
			int trainEnd = trainRanges[family][1];
			for (int i = 0; i < observationSequences[family].size(); i++) {
				if (i < trainStart || i >= trainEnd) {
					testLabels.push_back(family);
				}
			}
		}

		// Have all models at iters for each family
		// For each iter
		//	for each family
		//		score all test samples and store
		//	saveScores in a separate dir/file
		//models
		for (int i = 0; i < testIters.size(); i++) {
			int iter = testIters[i];
			std::cout << iter << std::endl;
			//  scores for each family for each test sample
			std::vector<std::vector<double>> scores;
			for (int family = 0; family < families.size(); family++) {
				std::cout << families[family] << std::endl;
				HMM& hmm = models[family][i].first;
				std::vector<double> testScores;
				for (int test_family = 0; test_family < families.size(); test_family++) {
					int trainStart = trainRanges[test_family][0];
					int trainEnd = trainRanges[test_family][1];
					for (int i = 0; i < observationSequences[test_family].size(); i++) {
						if (i < trainStart || i >= trainEnd) {	// Exclude training samples
							std::vector<int>& o = observationSequences[test_family][i];
							double score = hmm.normalizedScore(o);	//alt. double score = bestModelM.score(o); for non normalized score by T
							testScores.push_back(score);
						}
					}
					std::cout << testScores.size() << std::endl;
				}
				scores.push_back(testScores);
			}
			saveScores(output_dir + std::to_string(iter) + " iters/" + "_scores_" + std::to_string(run) + ".csv", scores, testLabels, families);
		}
	}

}








int main(int argc, char** argv) {
	classifyMalware();

	return 0;
}