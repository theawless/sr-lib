#include "recognizer.h"

#include <fstream>
#include <iostream>
#include <numeric>
#include <thread>
#include <tuple>

#include "hmm.h"
#include "logger.h"
#include "utils.h"

using namespace std;

vector<vector<double>> Recognizer::get_coefficients(string filename, bool cache)
{
	vector<vector<double>> coefficients;
	string mfcs_filename = filename + ".mfcs";
	Logger::logger() << "Getting " << mfcs_filename << endl;

	if (cache)
	{
		coefficients = Utils::get_matrix_from_file<double>(mfcs_filename);
		if (!coefficients.empty())
		{
			return coefficients;
		}
	}

	vector<double> samples = Utils::get_vector_from_file<double>(filename + ".samples");
	vector<vector<double>> frames = audio_processor.process(samples);
	vector<future<vector<double>>> coefficients_futures;
	for (int i = 0; i < frames.size(); ++i)
	{
		coefficients_futures.push_back(thread_pool.enqueue(&MFCC::mfcc, mfcc, frames[i]));
	}
	for (int i = 0; i < frames.size(); ++i)
	{
		coefficients.push_back(coefficients_futures[i].get());
	}
	Utils::set_matrix_to_file<double>(coefficients, mfcs_filename);

	return coefficients;
}

vector<vector<double>> Recognizer::get_universe()
{
	vector<vector<double>> universe;
	const string universe_filename = config.folder + "coeff.universe";
	Logger::logger() << "Getting " << universe_filename << endl;

	universe = Utils::get_matrix_from_file<double>(universe_filename);
	if (!universe.empty())
	{
		return universe;
	}

	for (int i = 0; i < config.audio_names.size(); ++i)
	{
		for (int j = 0; j < config.n_utterances[i]; ++j)
		{
			string filename = config.folder + config.audio_names[i] + "_" + to_string(j);
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

void Recognizer::build_codebook()
{
	const string codebook_filename = config.folder + "coeff.codebook";
	Logger::logger() << "Getting " << codebook_filename << endl;

	codebook.centroids = Utils::get_matrix_from_file<double>(codebook_filename);
	if (!codebook.centroids.empty())
	{
		return;
	}

	vector<vector<double>> universe = get_universe();
	codebook.generate(universe);
	Utils::set_matrix_to_file<double>(codebook.centroids, codebook_filename);
}

vector<int> Recognizer::get_observations(string filename, bool cache)
{
	vector<int> observations;
	string obs_filename = filename + ".obs";
	Logger::logger() << "Getting " << obs_filename << endl;

	if (cache)
	{
		observations = Utils::get_vector_from_file<int>(obs_filename);
		if (!observations.empty())
		{
			return observations;
		}
	}

	vector<vector<double>> coefficients = get_coefficients(filename, false);
	observations = codebook.observations(coefficients);
	Utils::set_vector_to_file<int>(observations, obs_filename);

	return observations;
}

Model Recognizer::get_utterance_model(const Model &train_model, int word_index, int train_index, int utterance_index)
{
	Model model;
	string model_ext = ".model";
	string filename = config.folder + config.audio_names[word_index] + "_" + to_string(utterance_index);
	string model_filename = filename + model_ext + "_" + to_string(train_index);
	Logger::logger() << "Getting " << model_filename << endl;

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

Model Recognizer::get_word_model(const Model &train_model, int word_index, int train_index)
{
	Model model;
	string model_ext = ".model";
	string filename = config.folder + config.audio_names[word_index];
	string model_filename = filename + model_ext + "_" + to_string(train_index);
	Logger::logger() << "Getting " << model_filename << endl;

	model = Utils::get_item_from_file<Model>(model_filename);
	if (!model.empty())
	{
		return model;
	}

	vector<Model> utterance_models;
	for (int i = 0; i < config.n_utterances[word_index]; ++i)
	{
		Model utterance_model = get_utterance_model(train_model, word_index, train_index, i);
		utterance_models.push_back(utterance_model);
	}
	model = Model::merge(utterance_models);
	Utils::set_item_to_file<Model>(model, model_filename);

	return model;
}

void Recognizer::build_models()
{
	for (int i = 0; i < config.audio_names.size(); ++i)
	{
		Model model;
		string model_ext = ".model";
		string filename = config.folder + config.audio_names[i];
		string model_filename = filename + model_ext;
		Logger::logger() << "Getting " << model_filename << endl;

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

int Recognizer::decide_word(const vector<int> &observations)
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
		double P = P_futures[i].get().first;
		if (max_P < P)
		{
			max_P = P;
			most_probable_word_index = i;
		}
	}

	return most_probable_word_index;
}

Recognizer::Recognizer(const Config &config) :config(config), audio_processor(AudioProcessor(320, 80)),
mfcc(MFCC()), codebook(Codebook(config.M)), thread_pool(config.n_thread * thread::hardware_concurrency())
{
	Logger::logger().add_log(config.folder + "recognizer.log");

	build_codebook();
	build_models();
}

int Recognizer::recognize(string filename, bool cache)
{
	vector<int> observations = get_observations(config.folder + filename, cache);
	int word_index = decide_word(observations);

	return word_index;
}
