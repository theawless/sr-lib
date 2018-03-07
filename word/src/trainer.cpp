#include "trainer.h"

#include <fstream>
#include <iostream>
#include <numeric>
#include <thread>
#include <tuple>

#include "hmm.h"
#include "logger.h"
#include "utils.h"

using namespace std;

vector<vector<double>> Trainer::get_coefficients(string filename)
{
	vector<vector<double>> coefficients;
	string coeffs_filename = filename + ".coeffs";
	Logger::logger().log("Getting", coeffs_filename);

	coefficients = Utils::get_matrix_from_file<double>(coeffs_filename);
	if (!coefficients.empty())
	{
		return coefficients;
	}

	vector<double> samples = Utils::get_vector_from_file<double>(filename + ".samples");
	vector<vector<double>> frames = preprocessor.process(samples);
	for (int i = 0; i < frames.size(); ++i)
	{
		coefficients.push_back(mfcc.coeffs(frames[i]));
	}
	Utils::set_matrix_to_file<double>(coefficients, coeffs_filename);

	return coefficients;
}

vector<vector<double>> Trainer::get_universe()
{
	vector<vector<double>> universe;
	const string universe_filename = config.folder + "coeff.universe";
	Logger::logger().log("Getting", universe_filename);

	universe = Utils::get_matrix_from_file<double>(universe_filename);
	if (!universe.empty())
	{
		return universe;
	}

	for (int i = 0; i < config.audio_names.size(); ++i)
	{
		vector<future<vector<vector<double>>>> coefficients_futures;
		for (int j = 0; j < config.n_utterances[i]; ++j)
		{
			string filename = config.folder + config.audio_names[i] + "_" + to_string(j);
			coefficients_futures.push_back(thread_pool.enqueue(&Trainer::get_coefficients, this, filename));
		}
		for (int j = 0; j < config.n_utterances[i]; ++j)
		{
			vector<vector<double>> coefficients = coefficients_futures[j].get();
			universe.insert(universe.end(), coefficients.begin(), coefficients.end());
		}
	}
	Utils::set_matrix_to_file(universe, universe_filename);

	return universe;
}

void Trainer::build_codebook()
{
	const string codebook_filename = config.folder + "coeff.codebook";
	Logger::logger().log("Getting", codebook_filename);

	codebook.centroids = Utils::get_matrix_from_file<double>(codebook_filename);
	if (!codebook.centroids.empty())
	{
		return;
	}

	vector<vector<double>> universe = get_universe();
	codebook.generate(universe);
	Utils::set_matrix_to_file<double>(codebook.centroids, codebook_filename);
}

vector<int> Trainer::get_observations(string filename)
{
	vector<int> observations;
	string obs_filename = filename + ".obs";
	Logger::logger().log("Getting", obs_filename);

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

Model Trainer::get_utterance_model(const Model &train_model, int word_index, int train_index, int utterance_index)
{
	Model model;
	string model_ext = ".model";
	string filename = config.folder + config.audio_names[word_index] + "_" + to_string(utterance_index);
	string model_filename = filename + model_ext + "_" + to_string(train_index);
	Logger::logger().log("Getting", model_filename);

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

Model Trainer::get_word_model(const Model &train_model, int word_index, int train_index)
{
	Model model;
	string model_ext = ".model";
	string filename = config.folder + config.audio_names[word_index];
	string model_filename = filename + model_ext + "_" + to_string(train_index);
	Logger::logger().log("Getting", model_filename);

	model = Utils::get_item_from_file<Model>(model_filename);
	if (!model.empty())
	{
		return model;
	}

	vector<Model> utterance_models;
	vector<future<Model>> utterance_model_futures;
	for (int i = 0; i < config.n_utterances[word_index]; ++i)
	{
		utterance_model_futures.push_back(thread_pool.enqueue(&Trainer::get_utterance_model, this, train_model, word_index, train_index, i));
	}
	for (int i = 0; i < config.n_utterances[word_index]; ++i)
	{
		utterance_models.push_back(utterance_model_futures[i].get());
	}
	model = Model::merge(utterance_models);
	Utils::set_item_to_file<Model>(model, model_filename);

	return model;
}

void Trainer::build_models()
{
	for (int i = 0; i < config.audio_names.size(); ++i)
	{
		Model model;
		string model_ext = ".model";
		string filename = config.folder + config.audio_names[i];
		string model_filename = filename + model_ext;
		Logger::logger().log("Getting", model_filename);

		model = Utils::get_item_from_file<Model>(model_filename);
		if (!model.empty())
		{
			models.push_back(model);
			continue;
		}

		model = Model::bakis(config.N, config.M);
		for (int j = 0; j < config.n_retrain; ++j)
		{
			model = get_word_model(model, i, j);
		}
		models.push_back(model);
		Utils::set_item_to_file<Model>(model, model_filename);
	}
}

Trainer::Trainer(const Config &config) :config(config), preprocessor(320, 80),
mfcc(), codebook(config.M), thread_pool(config.n_thread * thread::hardware_concurrency())
{
	Logger::logger().add_log(config.folder + "trainer.log");

	build_codebook();
	build_models();
}
