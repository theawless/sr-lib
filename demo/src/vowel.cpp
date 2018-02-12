/// Speech Assignment 2: Vowel recognition
/// Abhinav Singh
/// 140101002
/// Analyze the signal over many short segments, called frames.
/// A hamming window is applied on each frame. Using durbin's solve we find the
/// predict the a's. Then the a's are converted to cepstral coefficients which is
/// a better representation for speech. To reduce cepstral sensitivity, we use a sine
/// window on the cepstral coefficients. Recognition is done by finding the vowel
/// which has the least tokhura distance from the test signal.

#include <iostream>
#include <map>
#include <limits>
#include <string>

#include "audio.h"
#include "trim.h"
#include "lpcc.h"
#include "utils.h"

#define RECORD_DURATION 0.8
#define RECORD_FOLDER "B:\\record\\vowel\\"
#define TRAIN_FILENAME RECORD_FOLDER "train"
#define TEST_FILENAME RECORD_FOLDER "test"

using namespace std;

/// Loads, preprocesses the amplitudes and then returns their lpc coefficients.
static vector<vector<double>> get_coefficients(string filename, bool cache) {
	vector<vector<double>> coefficients;
	string lpcs_filename = filename + "_lpcs";
	cout << "Getting lpcs for " << filename << endl;

	if (cache) {
		coefficients = get_matrix_from_csv(lpcs_filename);
		if (!coefficients.empty()) {
			return coefficients;
		}
	}

	vector<double> unprocessed_amplitudes = get_vector_from_txt(filename);
	vector<double> amplitudes = preprocess(unprocessed_amplitudes);
	vector<double> trimmed_amplitudes = trim_by_energy_fixed(amplitudes);
	vector<vector<double>> segments = fixed_segment(trimmed_amplitudes);
	coefficients = speech_to_coefficients(segments);

	if (cache) {
		set_matrix_to_csv(coefficients, lpcs_filename);
	}

	return coefficients;
}

/// Gets map of vowels and their lpc coefficients.
static map<char, vector<vector<double>>> get_train_coefficients() {
	vector<char> vowels = { 'a', 'e', 'i', 'o', 'u' };
	map<char, vector<vector<double>>> train_coefficients;

	for (int i = 0; i < vowels.size(); ++i) {
		string filename = string(TRAIN_FILENAME) + "_" + string(1, vowels[i]);
		train_coefficients[vowels[i]] = get_coefficients(filename, true);
	}

	return train_coefficients;
}

/// Decides vowel by taking the one which is most similar.
static char decide_vowel(const map<char, vector<vector<double>>> &train_coefficients, const vector<vector<double>> &test_coefficients) {
	char most_similar_vowel = '-';
	double min_similarity_distance = numeric_limits<double>::max();

	for (map<char, vector<vector<double>>>::const_iterator i = train_coefficients.begin(); i != train_coefficients.end(); ++i) {
		double similarity_distance = coefficients_similarity(i->second, test_coefficients);
		cout << "Similarity with '" << i->first << "' :" << similarity_distance << endl;
		if (similarity_distance < min_similarity_distance) {
			min_similarity_distance = similarity_distance;
			most_similar_vowel = i->first;
		}
	}

	return most_similar_vowel;
}

/// The entry of the program, it takes user input, calls required functions and outputs the results.
int main() {
	map<char, vector<vector<double>>> train_coefficients = get_train_coefficients();

	string ready;
	while (cout << "Are you ready to speak? (y/n) " && cin >> ready) {
		if (ready != "Y" && ready != "y") {
			continue;
		}

		capture_audio(TEST_FILENAME, RECORD_DURATION);
		vector<vector<double>> test_coefficients = get_coefficients(TEST_FILENAME, false);
		cout << "The recognised vowel is: " << decide_vowel(train_coefficients, test_coefficients) << endl;
	}

	return 0;
}
