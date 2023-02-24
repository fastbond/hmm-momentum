#include "util.h"


// Yes these should probably be in a separate file as part of a matrix class,
// but making the class would have taken more time than it would have saved
void printMatrix(const std::vector<std::vector<double>>& matrix) {
	for (auto row : matrix) {
		for (double col : row) {
			printf("%f ", col);
		}
		printf("\n");
	}
	printf("\n");
}

void saveMatrix(const std::vector<std::vector<double>>& matrix, const std::string& fname) {
	FILE* file = openFile(fname, "w");
	//fopen_s(&file, fname.c_str(), "w");
	if (file == NULL)
		return;

	for (auto row : matrix) {
		for (double col : row) {
			fprintf(file, "%f ", col);
		}
		fprintf(file, "\n");
	}
	fprintf(file, "\n");
	fclose(file);
}

std::vector<std::vector<double>> loadMatrix(const std::string& filename) {
	std::vector<std::vector<double>> matrix;
	std::ifstream file(filename.c_str());
	std::string line;
	if (file.is_open()) {
		while (std::getline(file, line)) {
			if (line.length() == 0)
				continue;

			std::istringstream lineStream(line);
			std::vector<double> row;
			double entry;
			while (lineStream >> entry) {
				row.push_back(entry);
			}
			matrix.push_back(row);

		}
		file.close();
	}

	return matrix;
}


/*
Open or Write file?
What about read?
C FILE or C++ ofstream?
*/
namespace fs = std::filesystem; //putting it here out of laziness
FILE* openFile(const std::string& fname, const std::string& rwtype) {
	fs::path path = fname;
	if (fs::exists(path)) {
		if (!fs::is_regular_file(path)) {
			return NULL;
		}
	}

	fs::path dir = path.parent_path();
	if (!fs::exists(dir))
		fs::create_directories(dir);

	FILE* file;
	if (!fs::exists(path)) {
		fopen_s(&file, fname.c_str(), "a");
		if (file != NULL)
			fclose(file);
	}

	fopen_s(&file, fname.c_str(), rwtype.c_str());
	return file;
}



std::string to_string_trimmed(double d, int p) {
	std::ostringstream oss;
	//oss << std::setprecision(8) << std::noshowpoint << d;
	oss << std::setprecision(p) << d;
	return oss.str();
}

std::string to_string_trimmed(double d) {
	std::ostringstream oss;
	oss << std::setprecision(8) << d;
	return oss.str();
}



//Assumes no negatives, and will have issues if entire vector is of 0.0
void normalize(std::vector<double>& vec) {
	double sum = 0.0;
	for (int i = 0; i < vec.size(); i++) {
		sum += vec[i];
	}

	//zero vector
	if (sum == 0.0) {
		std::cout << "ERROR: trying to normalize a zero vector" << std::endl;
		exit(1);
	}

	for (int i = 0; i < vec.size(); i++)
		vec[i] /= sum;
}



double direction(double x1, double x2) {
	return direction(x1, x2, 0.0);
}


double direction(double x1, double x2, double threshold) {
	if (x2 - x1 > threshold)
		return 1;
	if (x1 - x2 > threshold)
		return -1;
	return 0;
}