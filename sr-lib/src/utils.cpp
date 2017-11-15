#include "utils.h"

#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#define SAMPLE_RATE 16000
#define DELETE_COMMAND "del"
#define RECORD_COMMAND "\"C:\\Program Files (x86)\\sox-14-4-2\\sox.exe\" -t waveaudio -c 1 -r 16000 -d -t s16 -q"

using namespace std;

void capture_audio(string filename, double duration) {
	string data_filename = filename + ".dat";

	string record_command = string(RECORD_COMMAND) + " " + data_filename + " trim 0 " + to_string(duration);
	cout << "Recording started, duration: " << duration << endl;
	system(record_command.c_str());

	fstream data(data_filename, ios::in | ios::binary);
	vector<int16_t> amplitudes(SAMPLE_RATE * duration);
	data.read((char *)&amplitudes[0], amplitudes.size() * sizeof(amplitudes[0]));
	set_vector_to_txt(vector<double>(amplitudes.begin(), amplitudes.end()), filename);

	data.close();
	string delete_command = string(DELETE_COMMAND) + " " + data_filename;
	system(delete_command.c_str());
}

vector<double> get_vector_from_txt(string filename) {
	string text_filename = filename + ".txt";
	fstream file = fstream(text_filename, ios::in);
	string token;
	vector<double> vec;

	while (getline(file, token)) {
		try {
			vec.push_back(stod(token));
		}
		catch (...) {
			// Ignore unrecognised content.
		}
	}
	file.close();

	return vec;
}

void set_vector_to_txt(const vector<double> &vec, string filename) {
	string text_filename = filename + ".txt";
	fstream file = fstream(text_filename, ios::out);
	file.precision(numeric_limits<double>::max_digits10);
	file.setf(ios::scientific);

	for (int i = 0; i < vec.size(); ++i) {
		file << vec[i] << endl;
	}

	file.close();
}

vector<vector<double>> get_matrix_from_csv(string filename) {
	string csv_filename = filename + ".csv";
	fstream csv(csv_filename, ios::in);
	string line_string;
	vector<vector<double>> mat;

	while (getline(csv, line_string)) {
		string token;
		vector<double> vec;
		stringstream line(line_string);

		while (getline(line, token, ',')) {
			vec.push_back(stod(token));
		}
		mat.push_back(vec);
	}
	csv.close();

	return mat;
}

void set_matrix_to_csv(const vector<vector<double>> &mat, string filename) {
	string csv_filename = filename + ".csv";
	fstream csv(csv_filename, ios::out);
	csv.precision(numeric_limits<double>::max_digits10);
	csv.setf(ios::scientific);

	for (int i = 0; i < mat.size(); ++i) {
		for (int j = 0; j < mat[i].size() - 1; ++j) {
			csv << mat[i][j] << ',';
		}
		csv << mat[i][mat[i].size() - 1] << endl;
	}

	csv.close();
}

vector<vector<double>> get_vector_chunks(const vector<double> &unchunked, int chunk_size) {
	vector<vector<double>> chunked;

	for (int i = 0; i < unchunked.size(); i += chunk_size) {
		vector<double>::const_iterator left = unchunked.begin() + i;
		vector<double>::const_iterator right = i + chunk_size < unchunked.size() ? left + chunk_size : unchunked.end();
		chunked.push_back(vector<double>(left, right));
	}

	return chunked;
}

string pad_number(int num, int max_num) {
	stringstream stream;
	stream << setw(ceil(log10(max_num))) << setfill('0') << num;

	return stream.str();
}
