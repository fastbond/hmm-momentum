#include "corpus.h"

/*
Need to rework this to:
1) Read corpus into vector<char> using alphabet/vocab map
2) Convert into observation sequence using vocab map
*/


std::vector<int> readCorpus(std::string filename, std::map<char, int> vocabMap, int startPos, int maxLen) {
	std::vector<int> observations;
	int line_metadata_prefix_len = 15;

	std::ifstream file(filename.c_str());
	std::string line;
	int charsSkipped = 0;
	if (file.is_open()) {
		std::cout << "opened file" << std::endl;
		while (std::getline(file, line)) {
			if (line.length() <= line_metadata_prefix_len)
				continue;
			bool passed_leading_spaces = false;
			line.append(" "); //add a space to end of each line, as doc doesnt keep space between words separated by linebreaks
			for (int i = line_metadata_prefix_len; i < line.length(); i++) {
				char c = toupper(line[i]);

				if (!passed_leading_spaces) {
					if (c == ' ')
						continue;
					else
						passed_leading_spaces = true;
				}

				if (vocabMap.count(c) > 0) {
					if (charsSkipped < startPos) { //skip startPos chars
						charsSkipped++;
						continue;
					}
					else
						observations.push_back(vocabMap[c]); //add to observations
				}
				if (observations.size() >= maxLen)
					break;
			}
			if (observations.size() >= maxLen)
				break;
		}
		file.close();
	}

	return observations;
}



std::vector<std::vector<double>> computeDigrams(const std::vector<int>& observations, int N) {
	std::vector<std::vector<double>> digrams(N, std::vector<double>(N, 0.0));
	if (observations.size() < 2)
		return digrams;

	int state, prevState = observations[0];
	for (int i = 1; i < observations.size(); i++) {
		state = observations[i];
		digrams[prevState][state] += 1.0;
		prevState = state;
	}

	for (int i = 0; i < digrams.size(); i++) {
		double sum = 0.0;
		for (int j = 0; j < digrams[i].size(); j++)
			sum += digrams[i][j];
		if (sum != 0.0)
			for (int j = 0; j < digrams[i].size(); j++)
				digrams[i][j] /= sum;
	}

	return digrams;
}
