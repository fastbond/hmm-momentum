#include "opcodes.h"


/*
 * Generate mapping of opcodes to indices based on malware opcode sequence files in data_dir/family/.
 * Selects the top_opcodes number of most frequently occuring opcodes, and groups the rest together.
 */
std::map<std::string, int> getOpcodeMap(std::string& data_dir, std::vector<std::string>& families, int top_opcodes) {
	std::map<std::string, int> counts;
	for (auto& family : families) {
		std::cout << family << std::endl;
		std::string dir = data_dir + family;
		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			if (entry.is_regular_file()) {
				std::string fname = entry.path().string();
				std::ifstream file(fname.c_str());
				std::string line;
				if (file.is_open()) {
					while (std::getline(file, line)) {
						if (line.size() == 0)
							continue;
						if (counts.find(line) == counts.end())
							counts[line] = 0;
						counts[line] += 1;
					}
				}
			}
		}
	}

	//Map top X-1 opcodes to int values, with rest being grouped together as 1
	return assignOpcodes(counts, top_opcodes);
}


/*
 * Assign opcodes to indices based on the counts for each opcode in opcodeCounts.
 *
 * help from https://stackoverflow.com/questions/60223147/is-there-any-c-function-to-sort-a-map
 * probably needs to handle case where less opcodes than N
 */
std::map<std::string, int> assignOpcodes(const std::map<std::string, int>& opcodeCounts, int numAssignments) {
	//Sort map in decreasing order into vector
	std::vector<std::pair<std::string, int>> pairs;
	std::copy(opcodeCounts.begin(), opcodeCounts.end(),
		std::back_inserter<std::vector<std::pair<std::string, int>>>(pairs));
	std::sort(pairs.begin(), pairs.end(),
		[](const auto& a, const auto& b) { return a.second >= b.second; }
	);

	//map of opcode str : index
	//top N get index 0->N-1
	//rest get index N
	std::map<std::string, int> opcodeMap;
	for (int i = 0; i < pairs.size(); i++) {
		std::string opcode = pairs[i].first;
		if (i < numAssignments - 1)
			opcodeMap[opcode] = i;
		else
			opcodeMap[opcode] = numAssignments - 1;
	}

	return opcodeMap;
}



/*
 * Create observation sequences for each family
 */
// Families x Files x Opcodes
std::vector<std::vector<std::vector<int>>> getObservationSequences(std::string& data_dir, std::vector<std::string>& families, std::map<std::string, int>& opcodeMap, int min_size) {
	std::vector<std::vector<std::vector<int>>> opcodeSequences;
	for (auto& family : families) {
		std::cout << family << std::endl;
		opcodeSequences.push_back(std::vector<std::vector<int>>());
		std::vector<std::vector<int>>& familySequences = opcodeSequences[opcodeSequences.size() - 1];
		std::string dir = data_dir + family;
		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			if (entry.is_regular_file()) {
				std::string fname = entry.path().string();
				std::ifstream file(fname.c_str());
				std::string line;
				if (file.is_open()) {
					familySequences.push_back(std::vector<int>());
					std::vector<int>& opcodes = familySequences[familySequences.size() - 1];
					while (std::getline(file, line)) {
						if (line.size() == 0)
							continue;
						if (opcodeMap.find(line) != opcodeMap.end()) {
							opcodes.push_back(opcodeMap[line]);
						}
						else {
							std::cout << "Unexpected opcode found: " << line << std::endl;
							throw;
						}
					}
					if (opcodes.size() < min_size || opcodes.size() < 1)
						familySequences.pop_back();
				}
			}
		}
	}

	return opcodeSequences;
}




/*
 * Load a pre-generated opcode mapping from file.
 */
std::map<std::string, int> loadOpcodeMap(const std::string& opcodeFile) {
	std::map<std::string, int> opcodeMap;

	std::string opcode;
	int index;
	std::ifstream file(opcodeFile.c_str());
	if (file.is_open()) {
		while (file >> opcode >> index) {
			opcodeMap[opcode] = index;
		}
	}

	return opcodeMap;
}


/*
 * Save an opcode mapping to file
 */
void saveOpcodeMap(std::map<std::string, int>& opcodeMap, const std::string& opcodeFile) {
	FILE* file = openFile(opcodeFile, "w");
	if (file == NULL)
		return;

	for (const auto& entry : opcodeMap) {
		fprintf(file, "%s %d\n", entry.first, entry.second);
	}

	fclose(file);
}