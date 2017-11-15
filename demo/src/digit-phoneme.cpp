#include <iostream>
#include <map>
#include <limits>
#include <string>

#include "audio.h"
#include "lpc.h"
#include "phoneme.h"
#include "trim.h"
#include "maths.h"
#include "utils.h"

#define RECORD_DURATION 3
#define RECORD_FOLDER "B:\\record\\digit\\"

#define N_TRAIN_UTTERANCES 15
#define TEST_FILENAME RECORD_FOLDER "test"

using namespace std;

const vector<string> TRAIN_ROLLS = { "140101002" };

/// Loads, preprocesses the amplitudes and then returns the lpc coefficients of their phonemes.
static pair<vector<Phoneme>, vector<vector<double>>> get_features(string filename, bool cache) {
	vector<vector<double>> coefficients;
	vector<Phoneme> phonemes;

	string lpcs_filename = filename + "_lpcs";
	string phonemes_filename = filename + "_phs";
	cout << "Getting features for " << filename << endl;

	if (cache) {
		coefficients = get_matrix_from_csv(lpcs_filename);
		vector<double> temp_phonemes = get_vector_from_txt(phonemes_filename);
		for (int i = 0; i < temp_phonemes.size(); ++i) {
			phonemes.push_back((Phoneme)(int)temp_phonemes[i]);
		}
		if (!coefficients.empty()) {
			return pair<vector<Phoneme>, vector<vector<double>>>(phonemes, coefficients);
		}
	}

	vector<double> unprocessed_amplitudes = get_vector_from_txt(filename);
	vector<double> amplitudes = preprocess(unprocessed_amplitudes);
	pair<vector<double>, pair<vector<double>, vector<double>>> trimmed_amplitudes = trim_by_auto(amplitudes);
	vector<vector<double>> segments = fixed_segment(trimmed_amplitudes.first);
	coefficients = speech_to_coefficients(segments);
	phonemes = tag_phonemes(segments, trimmed_amplitudes.second.first);

	if (cache) {
		set_matrix_to_csv(coefficients, lpcs_filename);
		set_vector_to_txt(vector<double>(phonemes.begin(), phonemes.end()), phonemes_filename);
	}

	return pair<vector<Phoneme>, vector<vector<double>>>(phonemes, coefficients);
}

/// Gets map of digits and their lpc features.
static multimap<int, pair<vector<Phoneme>, vector<vector<double>>>> get_train_features() {
	multimap<int, pair<vector<Phoneme>, vector<vector<double>>>> train_features;

	for (int i = 0; i < 10; ++i) {
		for (string roll : TRAIN_ROLLS) {
			for (int j = 1; j <= N_TRAIN_UTTERANCES; ++j) {
				string filename = string(RECORD_FOLDER) + roll + "_" + to_string(i) + "_" + pad_number(j, N_TRAIN_UTTERANCES);

				pair<vector<Phoneme>, vector<vector<double>>> features = get_features(filename, true);
				train_features.insert(pair<int, pair<vector<Phoneme>, vector<vector<double>>>>(i, features));
			}
		}
	}

	return train_features;
}

double features_similarity(const pair<vector<Phoneme>, vector<vector<double>>> &A, const pair<vector<Phoneme>, vector<vector<double>>> &B) {
	double sum_distance = 0;
	double n_distances = 0;

	int i = 0, j = 0;
	while (i < A.second.size() && j < B.second.size()) {
		if (A.first[i] == Silence) {
			i++; j++;
		}
		else if (B.first[j] == Silence) {
			j++; i++;
		}
		else if (A.first[i] != B.first[j]) {
			j++;
		}
		else {
			sum_distance += tokhura_distance(A.second[i], B.second[j]);
			n_distances++;
			i++; j++;
		}
	}

	return sum_distance / n_distances;
}

/// Decides digit by taking the one which is most similar.
static int decide_digit(const multimap<int, pair<vector<Phoneme>, vector<vector<double>>>> &train_features, const pair<vector<Phoneme>, vector<vector<double>>> &test_features) {
	int most_similar_digit = -1;
	double min_similarity_distance = numeric_limits<double>::max();

	for (multimap<int, pair<vector<Phoneme>, vector<vector<double>>>>::const_iterator i = train_features.begin(); i != train_features.end(); ++i) {
		double similarity_distance = features_similarity(i->second, test_features);
		if (similarity_distance == 0) {
			continue;
		}
		cout << "Similarity with '" << i->first << "' :" << similarity_distance << endl;
		if (similarity_distance < min_similarity_distance) {
			min_similarity_distance = similarity_distance;
			most_similar_digit = i->first;
		}
	}

	return most_similar_digit;
}

/// The entry of the program, it takes user input, calls required functions and outputs the results.
int main2() {
	multimap<int, pair<vector<Phoneme>, vector<vector<double>>>> train_features = get_train_features();

	string ready;
	while (cout << "Are you ready to speak? (y/n) " && cin >> ready) {
		if (ready != "Y" && ready != "y") {
			continue;
		}

		capture_audio(TEST_FILENAME, RECORD_DURATION);
		pair<vector<Phoneme>, vector<vector<double>>> test_features = get_features(TEST_FILENAME, false);
		cout << "The recognised digit is: " << decide_digit(train_features, test_features) << endl;
	}

	return 0;
}

/// The entry of the program, it takes user input, calls required functions and outputs the results.
int main1() {
	string train_filename, test_filename;
	while (cout << "Enter filename: " && cin >> test_filename) {
		int most_similar_digit = -1;
		double min_similarity_distance = numeric_limits<double>::max();

		test_filename = string(RECORD_FOLDER) + test_filename;
		pair<vector<Phoneme>, vector<vector<double>>> test_features = get_features(test_filename, true);
		for (string roll : TRAIN_ROLLS) {
			for (int i = 0; i < 10; ++i) {
				for (int j = 1; j <= N_TRAIN_UTTERANCES; ++j) {
					train_filename = string(RECORD_FOLDER) + roll + "_" + to_string(i) + "_" + pad_number(j, N_TRAIN_UTTERANCES);
					if (train_filename == test_filename) {
						continue;
					}
					pair<vector<Phoneme>, vector<vector<double>>> train_features = get_features(train_filename, true);
					double similarity_distance = features_similarity(train_features, test_features);
					if (similarity_distance < min_similarity_distance) {
						min_similarity_distance = similarity_distance;
						most_similar_digit = i;
					}
					cout << "Similarity with " << train_filename << " is: " << similarity_distance << endl;
				}
			}
		}
		cout << "Closest digit is: " << most_similar_digit << endl;
	}

	return 0;
}

/// The entry of the program, it takes user input, calls required functions and outputs the results.
int main() {
	double min_similarity_distance = numeric_limits<double>::max();
	multimap<int, pair<vector<Phoneme>, vector<vector<double>>>> train_features = get_train_features();
	vector<int> n_hits(10, 0);
	for (string roll : TRAIN_ROLLS) {
		for (int i = 0; i < 10; ++i) {
			for (int j = 1; j <= N_TRAIN_UTTERANCES; ++j) {
				string test_filename = string(RECORD_FOLDER) + roll + "_" + to_string(i) + "_" + pad_number(j, N_TRAIN_UTTERANCES);
				pair<vector<Phoneme>, vector<vector<double>>> test_features = get_features(test_filename, true);
				double digit = decide_digit(train_features, test_features);
				cout << "Closest digit for " << test_filename << "is: " << digit << endl;
				if (i == digit) n_hits[i]++;
			}
		}
	}

	int total_n_hits = 0;
	for (int n_hit : n_hits) {
		total_n_hits += n_hit;
	}
	cout << "n_hits is: " << total_n_hits;

	return 0;
}
