#include <iostream>
#include <algorithm>

#include "audio.h"
#include "codebook.h"
#include "hmm.h"
#include "lpc.h"
#include "utils.h"

#define RECORD_DURATION 0.8
#define RECORD_FOLDER "B:\\record\\digit_0.8\\"

#define N_VALUE 8
#define M_VALUE 32
#define T_VALUE 160

#define N_RETRAIN 3
#define N_TRAIN_UTTERANCES 15
#define TEST_FILENAME RECORD_FOLDER "test"
#define UNIVERSE_FILENAME RECORD_FOLDER "universe"
#define CODEBOOK_FILENAME RECORD_FOLDER "codebook"

using namespace std;

const vector<string> TRAIN_ROLLS = { "140101002" };

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
	vector<vector<double>> segments = fixed_segment(amplitudes);
	coefficients = speech_to_coefficients(segments);

	if (cache) {
		set_matrix_to_csv(coefficients, lpcs_filename);
	}

	return coefficients;
}

/// Builds the universe by accumulation lpcs of all frames of all signals.
static vector<vector<double>> get_universe() {
	vector<vector<double>> universe;
	cout << "Getting universe" << endl;

	universe = get_matrix_from_csv(UNIVERSE_FILENAME);
	if (!universe.empty()) {
		return universe;
	}

	for (string roll : TRAIN_ROLLS) {
		for (int i = 0; i < 10; ++i) {
			for (int j = 1; j <= N_TRAIN_UTTERANCES; ++j) {
				string filename = string(RECORD_FOLDER) + roll + "_" + to_string(i) + "_" + pad_number(j, N_TRAIN_UTTERANCES);
				vector<vector<double>> coefficients = get_coefficients(filename, true);
				for (int i = 0; i < coefficients.size(); ++i) {
					universe.push_back(coefficients[i]);
				}
			}
		}
	}
	set_matrix_to_csv(universe, UNIVERSE_FILENAME);

	return universe;
}

/// Builds a codebook from the universe using lbg.
static vector<vector<double>> get_codebook() {
	vector<vector<double>> codebook;
	cout << "Getting codebook" << endl;

	codebook = get_matrix_from_csv(CODEBOOK_FILENAME);
	if (!codebook.empty()) {
		return codebook;
	}

	vector<vector<double>> universe = get_universe();
	codebook = lbg_codebook(universe, M_VALUE);
	set_matrix_to_csv(codebook, CODEBOOK_FILENAME);

	return codebook;
}

/// Gets the observations sequence from the codebook.
static vector<int> get_observations(string filename, const vector<vector<double>> &codebook, bool cache) {
	vector<int> observations;
	string obs_filename = filename + "_obs";
	cout << "Getting observation sequence for " << filename << endl;

	if (cache) {
		vector<double> temp_obs = get_vector_from_txt(obs_filename);
		for (int i = 0; i < temp_obs.size(); ++i) {
			observations.push_back(temp_obs[i]);
		}
		if (!observations.empty()) {
			return observations;
		}
	}

	vector<vector<double>> coefficients = get_coefficients(filename, cache);
	observations = observation_sequence(coefficients, codebook);

	if (cache) {
		set_vector_to_txt(vector<double>(observations.begin(), observations.end()), obs_filename);
	}

	return observations;
}

/// Optimises the given train model use the observations of the given filename.
static Model get_utterance_model(string filename, const Model &train_model, const vector<vector<double>> &codebook, int t) {
	Model model;
	string base_filename = filename + "_lambda_" + to_string(t);
	string a_filename = base_filename + "_a";
	string b_filename = base_filename + "_b";
	string pi_filename = base_filename + "_pi";
	cout << "Getting model for " << base_filename << endl;

	model.a = get_matrix_from_csv(a_filename);
	model.b = get_matrix_from_csv(b_filename);
	model.pi = get_vector_from_txt(pi_filename);
	if (!model.pi.empty()) {
		return model;
	}

	vector<int> observations = get_observations(filename, codebook, true);
	model = optimise(train_model, observations);

	set_matrix_to_csv(model.a, a_filename);
	set_matrix_to_csv(model.b, b_filename);
	set_vector_to_txt(model.pi, pi_filename);

	return model;
}

/// Gets the model for all utterances and merges them.
static Model get_digit_model(const Model &train_model, const vector<vector<double>> &codebook, int d, int t) {
	vector<Model> utterance_models;

	for (string roll : TRAIN_ROLLS) {
		for (int i = 1; i <= N_TRAIN_UTTERANCES; ++i) {
			string filename = string(RECORD_FOLDER) + roll + "_" + to_string(d) + "_" + pad_number(i, N_TRAIN_UTTERANCES);
			Model model = get_utterance_model(filename, train_model, codebook, t);

			utterance_models.push_back(model);
		}
	}

	return merge(utterance_models);
}

/// Gets and trains the models for all digits.
static vector<Model> get_models(const vector<vector<double>> &codebook) {
	vector<Model> models;

	for (int i = 0; i < 10; ++i) {
		Model model = bakis(N_VALUE, M_VALUE);

		// Train the model.
		for (int j = 1; j <= N_RETRAIN; ++j) {
			model = get_digit_model(model, codebook, i, j);
		}

		models.push_back(model);
	}

	return models;
}

/// Decides the probable digit by comparing P for given models for given observations.
static int decide_digit(const vector<Model> &models, const vector<int> observations) {
	double max_P = numeric_limits<double>::min();
	int most_probable_digit = -1;

	for (int i = 0; i < models.size(); ++i) {
		double P = forward(models[i], observations).first;
		if (P > max_P) {
			max_P = P;
			most_probable_digit = i;
		}
	}

	return most_probable_digit;
}

/// The entry of the program, it takes user input, calls required functions and outputs the results.
int main() {
	vector<vector<double>> codebook = get_codebook();
	vector<Model> models = get_models(codebook);

	string ready;
	while (cout << "Are you ready to speak? (y/n) " && cin >> ready) {
		if (ready != "Y" && ready != "y") {
			continue;
		}

		capture_audio(TEST_FILENAME, RECORD_DURATION);
		vector<int> observations = get_observations(TEST_FILENAME, codebook, false);
		cout << "The recognised digit is: " << decide_digit(models, observations) << endl;
	}

	return 0;
}
