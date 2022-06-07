#pragma once

#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "util.h"


std::map<std::string, int> getOpcodeMap(std::string& data_dir, std::vector<std::string>& families, int top_opcodes);
std::map<std::string, int> assignOpcodes(const std::map<std::string, int>& opcodeCounts, int numAssignments);

std::vector<std::vector<std::vector<int>>> getObservationSequences(std::string& data_dir, std::vector<std::string>& families, std::map<std::string, int>& opcodeMap, int min_size);

std::map<std::string, int> loadOpcodeMap(const std::string& opcodeFile);
void saveOpcodeMap(std::map<std::string, int>& opcodeMap, const std::string& opcodeFile);

