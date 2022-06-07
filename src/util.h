#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <filesystem>

std::string to_string_trimmed(double d, int p);
std::string to_string_trimmed(double d);

void normalize(std::vector<double>& vec);

void printMatrix(const std::vector<std::vector<double>>& matrix);
void saveMatrix(const std::vector<std::vector<double>>& matrix, const std::string& fname);
std::vector<std::vector<double>> loadMatrix(const std::string& filename);

double direction(double x1, double x2);

double direction(double x1, double x2, double threshold);

FILE* openFile(const std::string& fname, const std::string& rwType);


// Defined here to avoid compilation error I don't understand
//https://stackoverflow.com/questions/11512674/error-lnk2019-unresolved-external-symbol-when-using-templates
//template <class T> int find(T val, const std::vector<T>& list);
//inline
template <class T>
int find(T val, const std::vector<T>& list) {
	for (int i = 0; i < list.size(); i++) {
		if (val == list[i])
			return i;
	}

	return -1;
};