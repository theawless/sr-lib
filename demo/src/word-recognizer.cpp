/// Word recognition
/// Analyze the signal over many short segments, called frames.
/// A hamming window is applied on each frame. Using durbin's solve we find the
/// predict the a's. Then the a's are converted to cepstral coefficients which is
/// a better representation for speech. To reduce cepstral sensitivity, we use a sine
/// window on the cepstral coefficients. Recognition is done by finding the vowel
/// which has the least tokhura distance from the test signal.
/// After generating LPCC, we do vector quantisation.
/// By creating a codebook using kmeans and lbg algorithm, we find the vector of states.
/// HMM is trained with this data using baum-welch algorithm + viterbi approach.
/// By calculating P* we can find the most probable state.

#include <iostream>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include "audio.h"
#include "codebook.h"
#include "hmm.h"
#include "mfcc.h"
#include "model.h"
#include "utils.h"

using namespace std;

class WordRecognizer
{
private:
	const int N = 5;
	const int M = 16;
	const int n_retrain = 3;

	const int n_train_utterances;
	const int n_total_utterances;
	const string folder;
	const vector<string> audio_names;
	vector<Model> models;

	AudioProcessor audio_processor;
	MFCC mfcc;
	Codebook codebook;

	/// Loads, preprocesses the amplitudes and then returns their lpc coefficients.
	vector<vector<double>> get_coefficients(string filename)
	{
		vector<vector<double>> coefficients;
		string mfcs_filename = filename + ".mfcs";
		cout << "Getting " << mfcs_filename << endl;

		coefficients = Utils::get_matrix_from_file<double>(mfcs_filename);
		if (!coefficients.empty())
		{
			return coefficients;
		}

		vector<double> samples = Utils::get_vector_from_file<double>(filename + ".txt");
		vector<vector<double>> frames = audio_processor.process(samples);
		for (int i = 0; i < frames.size(); ++i)
		{
			coefficients.push_back(mfcc.mfcc(frames[i]));
		}
		Utils::set_matrix_to_file<double>(coefficients, mfcs_filename);

		return coefficients;
	}

	/// Builds the universe by accumulation lpcs of all frames of all signals.
	vector<vector<double>> get_universe()
	{
		vector<vector<double>> universe;
		const string universe_filename = folder + "coeff.universe";
		cout << "Getting " << universe_filename << endl;

		universe = Utils::get_matrix_from_file<double>(universe_filename);
		if (!universe.empty())
		{
			return universe;
		}

		for (int i = 0; i < audio_names.size(); ++i)
		{
			for (int j = 0; j < n_train_utterances; ++j)
			{
				string filename = folder + audio_names[i] + "_" + to_string(j);
				vector<vector<double>> coefficients = get_coefficients(filename);
				for (int i = 0; i < coefficients.size(); ++i)
				{
					universe.push_back(coefficients[i]);
				}
			}
		}
		Utils::set_matrix_to_file(universe, universe_filename);

		return universe;
	}

	/// Builds a codebook from the universe using lbg.
	void build_codebook()
	{
		const string codebook_filename = folder + "coeff.codebook";
		cout << "Getting " << codebook_filename << endl;

		codebook.centroids = Utils::get_matrix_from_file<double>(codebook_filename);
		if (!codebook.centroids.empty())
		{
			return;
		}

		vector<vector<double>> universe = get_universe();
		codebook.generate(universe);
		Utils::set_matrix_to_file<double>(codebook.centroids, codebook_filename);
	}

	/// Gets the observations sequence from the codebook.
	vector<int> get_observations(string filename)
	{
		vector<int> observations;
		string obs_filename = filename + ".obs";
		cout << "Getting " << obs_filename << endl;

		observations = Utils::get_vector_from_file<int>(obs_filename);
		if (!observations.empty())
		{
			return observations;
		}

		vector<vector<double>> coefficients = get_coefficients(filename);
		observations = codebook.observations(coefficients);
		Utils::set_vector_to_file<int>(observations, obs_filename);

		return observations;
	}

	/// Optimises the given train model use the observations of the given filename.
	Model get_utterance_model(const Model &train_model, string filename, string model_ext)
	{
		Model model;
		string model_filename = filename + model_ext;
		cout << "Getting " << model_filename << endl;

		model = Utils::get_item_from_file<Model>(model_filename);
		if (!model.empty())
		{
			return model;
		}

		vector<int> observations = get_observations(filename);
		model = HMM(train_model).optimise(observations);
		Utils::set_item_to_file<Model>(model, model_filename);

		return model;
	}

	/// Gets the model for all utterances and merges them.
	Model get_word_model(const Model &train_model, string filename, string model_ext)
	{
		Model model;
		string model_filename = filename + model_ext;
		cout << "Getting " << model_filename << endl;

		model = Utils::get_item_from_file<Model>(model_filename);
		if (!model.empty())
		{
			return model;
		}

		vector<Model> utterance_models;
		for (int i = 0; i < n_train_utterances; ++i)
		{
			Model utterance_model = get_utterance_model(train_model, filename + "_" + to_string(i), model_ext);
			utterance_models.push_back(utterance_model);
		}
		model = Model::merge(utterance_models);
		Utils::set_item_to_file<Model>(model, model_filename);

		return model;
	}

	/// Trains the models for all words.
	void build_models()
	{
		for (int i = 0; i < audio_names.size(); ++i)
		{
			Model model;
			string model_ext = ".model";
			string filename = folder + audio_names[i];
			string model_filename = filename + model_ext;
			cout << "Getting " << model_filename << endl;

			model = Utils::get_item_from_file<Model>(model_filename);
			if (!model.empty())
			{
				models.push_back(model);
				continue;
			}

			model = Model::bakis(N, M);
			for (int j = 0; j < n_retrain; ++j)
			{
				model = get_word_model(model, filename, model_ext + "_" + to_string(j));
			}
			models.push_back(model);
			Utils::set_item_to_file<Model>(model, model_filename);
		}
	}

	/// Decides the most probable word index in audio names.
	int decide_word(const vector<int> &observations)
	{
		double max_P = numeric_limits<double>::min();
		int most_probable_word_index = -1;

		for (int i = 0; i < models.size(); ++i)
		{
			double P = HMM(models[i]).forward(observations).first;
			if (max_P < P)
			{
				max_P = P;
				most_probable_word_index = i;
			}
		}

		return most_probable_word_index;
	}

public:
	WordRecognizer(string folder, vector<string> audio_names, int n_total_utterances, int n_train_utterances) :
		audio_processor(AudioProcessor(320, 80)), mfcc(MFCC()), codebook(Codebook(M)),
		folder(folder), audio_names(audio_names), n_total_utterances(n_total_utterances), n_train_utterances(n_train_utterances)
	{
		build_codebook();
		build_models();
	}

	pair<vector<int>, vector<int>> test()
	{
		vector<int> n_hits(audio_names.size(), 0), n_errs(audio_names.size(), 0);

		for (int i = 0; i < audio_names.size(); ++i)
		{
			for (int j = n_train_utterances; j < n_total_utterances; ++j)
			{
				string test_filename = folder + audio_names[i] + "_" + to_string(j);
				vector<int> observations = get_observations(test_filename);

				int word_index = decide_word(observations);
				if (word_index == i)
				{
					n_hits[i]++;
				}
				if (word_index == -1)
				{
					n_errs[i]++;
					continue;
				}
				cout << "The recognised word is: " << audio_names[word_index] << endl;
			}
		}
		return pair<vector<int>, vector<int>>(n_hits, n_errs);
	}
};

int main()
{
	int n_train_utterances = 9;
	int n_total_utterances = 15;
	string folder = "B:/record/digit_0.8_2/";
	vector<string> audio_names = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
	WordRecognizer word_recognizer(folder, audio_names, n_total_utterances, n_train_utterances);

	pair<vector<int>, vector<int>> results = word_recognizer.test();
	cout << "n_hits is: " << accumulate(results.first.begin(), results.first.end(), 0) << endl;
	cout << "n_errs is: " << accumulate(results.second.begin(), results.second.end(), 0) << endl;
}
