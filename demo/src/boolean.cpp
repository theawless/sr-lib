/// Speech Assignment 1: Yes/No recognition
/// Abhinav Singh
/// 140101002
/// The ZCR of a 'Yes' sound is higher than the ZCR of a 'No' sound.
/// The 'S' is a fricative consonent and hence it has a higher frequency.
/// We find the threshold (the average of min and max ZCR) from the training data.
/// The max ZCR of the 'No' sound should be lower than this threshold.

#include <iostream>
#include <string>
#include <algorithm>

#include "audio.h"
#include "maths.h"
#include "trim.h"
#include "utils.h"

#define RECORD_DURATION 0.8
#define RECORD_FOLDER "B:\\record\\boolean\\"
#define TRAIN_FILENAME RECORD_FOLDER "train"
#define TEST_FILENAME RECORD_FOLDER "test"
#define SAMPLES_PER_FRAME 320

using namespace std;

/// Loads, preprocesses the amplitudes and then returns their zcrs.
static vector<int> get_zcrs(string filename, bool cache) {
	vector<int> zcrs;
	string zcrs_filename = filename + "_zcrs";
	cout << "Getting zcrs for " << filename << endl;

	if (cache) {
		vector<double> temp_zcrs = get_vector_from_txt(zcrs_filename);
		zcrs.insert(zcrs.end(), temp_zcrs.begin(), temp_zcrs.end());
		if (!zcrs.empty()) {
			return zcrs;
		}
	}

	vector<double> unprocessed_amplitudes = get_vector_from_txt(filename);
	vector<double> amplitudes = preprocess(unprocessed_amplitudes);
	vector<double> trimmed_amplitudes = trim_by_energy_threshold(amplitudes);
	vector<vector<double>> amplitudes_chunks = get_vector_chunks(trimmed_amplitudes, SAMPLES_PER_FRAME);
	for (int i = 0; i < amplitudes_chunks.size(); ++i) {
		zcrs.push_back(zcr(amplitudes_chunks[i]));
	}

	if (cache) {
		set_vector_to_txt(vector<double>(zcrs.begin(), zcrs.end()), zcrs_filename);
	}

	return zcrs;
}

/// Gets the threshold zcr (mean of max and min zcr) of the training signal.
static int get_threshold_zcr() {
	vector<int> zcrs = get_zcrs(TRAIN_FILENAME, true);
	int min_zcr = *min_element(zcrs.begin(), zcrs.end());
	int max_zcr = *max_element(zcrs.begin(), zcrs.end());

	return (max_zcr + min_zcr) / 2;
}

/// Gets the max zcr from the live signal.
static int get_test_zcr() {
	vector<int> zcrs = get_zcrs(TEST_FILENAME, false);
	int max_zcr = *max_element(zcrs.begin(), zcrs.end());

	return max_zcr;
}

/// Decides boolean by comparing zcrs.
static string decide_boolean(int threshold_zcr, int test_zcr) {
	return test_zcr > threshold_zcr ? "Yes" : "No";
}

/// The entry of the program, it takes user input, calls required functions and outputs the results.
int main() {
	int threshold_zcr = get_threshold_zcr();

	string ready;
	while (cout << "Are you ready to speak? (y/n) " && cin >> ready) {
		if (ready != "Y" && ready != "y") {
			continue;
		}

		capture_audio(TEST_FILENAME, RECORD_DURATION);
		int test_zcr = get_test_zcr();
		cout << "The signal is: " << decide_boolean(threshold_zcr, test_zcr) << endl;
	}

	return 0;
}
