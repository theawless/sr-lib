#include "tester.h"

#include <limits>
#include <tuple>

#include "hmm.h"
#include "logger.h"
#include "lpc.h"
#include "mfc.h"
#include "utils.h"
#include "wav.h"

using namespace std;

ICepstral *Tester::setup_cepstral(const Parameters &parameters)
{
	if (parameters.cepstral == "lpc")
	{
		return new LPC(parameters.n_cepstra, parameters.q_gain, parameters.q_delta, parameters.q_accel, parameters.n_predict);
	}
	else if (parameters.cepstral == "mfc")
	{
		return new MFC(parameters.n_cepstra, parameters.q_gain, parameters.q_delta, parameters.q_accel);
	}
	else
	{
		return nullptr;
	}
}

void Tester::load_codebook()
{
	const string codebook_filename = parameters.folder + "words.codebook";
	Logger::log("Getting", codebook_filename);

	codebook.centroids = Utils::get_matrix_from_file<double>(codebook_filename);
}

vector<vector<double>> Tester::get_features(string filename) const
{
	vector<vector<double>> features;
	const string features_filename = filename + ".features";
	Logger::log("Getting", features_filename);

	const Wav wav_file(filename + ".wav");
	const vector<vector<double>> frames = preprocessor.process(wav_file.samples<double>());
	features = cepstral->features(frames);

	return features;
}

void Tester::load_models()
{
	for (int i = 0; i < parameters.words.size(); ++i)
	{
		Model model;
		const string model_ext = ".model";
		const string filename = parameters.folder + parameters.words[i].first;
		const string model_filename = filename + model_ext;
		Logger::log("Getting", model_filename);

		model = Utils::get_item_from_file<Model>(model_filename);
		models.push_back(model);
	}
}

vector<int> Tester::get_observations(string filename) const
{
	vector<int> observations;
	const string obs_filename = filename + ".observations";
	Logger::log("Getting", obs_filename);

	const vector<vector<double>> features = get_features(filename);
	observations = codebook.observations(features);

	return observations;
}

int Tester::decide_word(const vector<int> &observations)
{
	vector<future<pair<double, vector<vector<double>>>>> P_futures;
	double max_P = numeric_limits<double>::min();
	int most_probable_word_index = -1;

	for (int i = 0; i < models.size(); ++i)
	{
		P_futures.push_back(thread_pool.enqueue(&HMM::forward, HMM(models[i]), observations));
	}
	for (int i = 0; i < models.size(); ++i)
	{
		const double P = P_futures[i].get().first;
		if (max_P < P)
		{
			max_P = P;
			most_probable_word_index = i;
		}
	}

	return most_probable_word_index;
}

Tester::Tester(const Parameters &parameters) :parameters(parameters), preprocessor(parameters.x_frame, parameters.x_overlap),
cepstral(setup_cepstral(parameters)), codebook(parameters.x_codebook), thread_pool(parameters.n_thread)
{
	load_codebook();
	load_models();
}

string Tester::test(string filename)
{
	const vector<int> observations = get_observations(parameters.folder + filename);
	const int word_index = decide_word(observations);

	return word_index < parameters.words.size() ? parameters.words[word_index].first : "";
}
