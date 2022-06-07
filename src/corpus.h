#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "util.h"

std::vector<int> readCorpus(std::string filename, std::map<char, int> vocabMap, int startPos, int maxLen);
std::vector<std::vector<double>> computeDigrams(const std::vector<int>& observations, int N);