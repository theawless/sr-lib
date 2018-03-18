#include "trainer.h"

#include <limits>
#include <thread>
#include <tuple>

#include "hmm.h"
#include "logger.h"
#include "lpc.h"
#include "mfc.h"
#include "utils.h"
#include "wav.h"

using namespace std;

ICepstral *Trainer::setup_cepstral(const Parameters &parameters)
{
	if (parameters.cepstral == "lpc")
	{
		return new LPC(parameters.n_cepstra, parameters.q_gain, parameters.q_delta, parameters.q_accel, parameters.n_predict);
	}
	else if (parameters.cepstral == "mfc")
	{
		return new MFC(parameters.n_cepstra, parameters.q_gain, parameters.q_delta, parameters.q_accel, parameters.hz_sampling);
	}
	else
	{
		return nullptr;
	}
}

void Trainer::build_codebook()
{
	const string codebook_filename = parameters.folder + "words.codebook";
	Logger::log("Getting", codebook_filename);

	codebook.centroids = Utils::get_matrix_from_file<double>(codebook_filename);
	if (!codebook.centroids.empty())
	{
		return;
	}

	const vector<vector<double>> universe = get_universe();
	codebook.generate(universe);
	Utils::set_matrix_to_file<double>(codebook.centroids, codebook_filename);
}

vector<vector<double>> Trainer::get_universe()
{
	vector<vector<double>> universe;
	const string universe_filename = parameters.folder + "words.universe";
	Logger::log("Getting", universe_filename);

	universe = Utils::get_matrix_from_file<double>(universe_filename);
	if (!universe.empty())
	{
		return universe;
	}

	for (int i = 0; i < parameters.words.size(); ++i)
	{
		vector<future<vector<vector<double>>>> features_futures;
		for (int j = 0; j < parameters.words[i].second; ++j)
		{
			string filename = parameters.folder + parameters.words[i].first + "_" + to_string(j);
			features_futures.push_back(thread_pool.enqueue(&Trainer::get_features, this, filename));
		}
		for (int j = 0; j < parameters.words[i].second; ++j)
		{
			vector<vector<double>> features = features_futures[j].get();
			universe.insert(universe.end(), features.begin(), features.end());
		}
	}
	Utils::set_matrix_to_file(universe, universe_filename);

	return universe;
}

vector<vector<double>> Trainer::get_features(string filename) const
{
	vector<vector<double>> features;
	const string features_filename = filename + ".features";
	Logger::log("Getting", features_filename);

	features = Utils::get_matrix_from_file<double>(features_filename);
	if (!features.empty())
	{
		return features;
	}

	const Wav wav_file(filename + ".wav");
	const vector<vector<double>> frames = preprocessor.process(wav_file.samples<double>());
	features = cepstral->features(frames);
	Utils::set_matrix_to_file<double>(features, features_filename);

	return features;
}

void Trainer::build_models()
{
	for (int i = 0; i < parameters.words.size(); ++i)
	{
		Model model;
		const string model_ext = ".model";
		const string filename = parameters.folder + parameters.words[i].first;
		const string model_filename = filename + model_ext;
		Logger::log("Getting", model_filename);

		model = Utils::get_item_from_file<Model>(model_filename);
		if (!model.empty())
		{
			models.push_back(model);
			continue;
		}

		model = Model::bakis(parameters.n_state, parameters.x_codebook, parameters.n_bakis);
		for (int j = 0; j < parameters.n_retrain; ++j)
		{
			model = get_word_model(model, i, j);
		}
		models.push_back(model);
		Utils::set_item_to_file<Model>(model, model_filename);
	}
}

Model Trainer::get_word_model(const Model &train_model, int word_index, int train_index)
{
	Model model;
	const string model_ext = ".model";
	const string filename = parameters.folder + parameters.words[word_index].first;
	const string model_filename = filename + model_ext + "_" + to_string(train_index);
	Logger::log("Getting", model_filename);

	model = Utils::get_item_from_file<Model>(model_filename);
	if (!model.empty())
	{
		return model;
	}

	vector<Model> utterance_models;
	vector<future<Model>> utterance_model_futures;
	for (int i = 0; i < parameters.words[word_index].second; ++i)
	{
		utterance_model_futures.push_back(thread_pool.enqueue(&Trainer::get_utterance_model, this,
			train_model, word_index, train_index, i));
	}
	for (int i = 0; i < parameters.words[word_index].second; ++i)
	{
		utterance_models.push_back(utterance_model_futures[i].get());
	}
	model = Model::merge(utterance_models);
	Utils::set_item_to_file<Model>(model, model_filename);

	return model;
}

Model Trainer::get_utterance_model(const Model &train_model, int word_index, int train_index, int utterance_index) const
{
	Model model;
	const string model_ext = ".model";
	const string filename = parameters.folder + parameters.words[word_index].first + "_" + to_string(utterance_index);
	const string model_filename = filename + model_ext + "_" + to_string(train_index);
	Logger::log("Getting", model_filename);

	model = Utils::get_item_from_file<Model>(model_filename);
	if (!model.empty())
	{
		return model;
	}

	const vector<int> observations = get_observations(filename);
	model = HMM(train_model).optimise(observations);
	Utils::set_item_to_file<Model>(model, model_filename);

	return model;
}

vector<int> Trainer::get_observations(string filename) const
{
	vector<int> observations;
	const string obs_filename = filename + ".observations";
	Logger::log("Getting", obs_filename);

	observations = Utils::get_vector_from_file<int>(obs_filename);
	if (!observations.empty())
	{
		return observations;
	}

	const vector<vector<double>> features = get_features(filename);
	observations = codebook.observations(features);
	Utils::set_vector_to_file<int>(observations, obs_filename);

	return observations;
}

Trainer::Trainer(const Parameters &parameters) :parameters(parameters), preprocessor(parameters.x_frame, parameters.x_overlap),
cepstral(setup_cepstral(parameters)), codebook(parameters.x_codebook), thread_pool(parameters.n_thread)
{
	build_codebook();
	build_models();
}
