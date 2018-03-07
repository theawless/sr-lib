#include "tester.h"

#include <fstream>
#include <iostream>
#include <numeric>
#include <thread>
#include <tuple>

#include "hmm.h"
#include "logger.h"
#include "utils.h"

using namespace std;

vector<vector<double>> Tester::get_coefficients(string filename)
{
	vector<vector<double>> coefficients;
	string coeffs_filename = filename + ".coeffs";
	Logger::logger().log("Getting", coeffs_filename);

	vector<double> samples = Utils::get_vector_from_file<double>(filename + ".samples");
	vector<vector<double>> frames = preprocessor.process(samples);
	vector<future<vector<double>>> coefficients_futures;
	for (int i = 0; i < frames.size(); ++i)
	{
		coefficients_futures.push_back(thread_pool.enqueue(&MFCC::coeffs, mfcc, frames[i]));
	}
	for (int i = 0; i < frames.size(); ++i)
	{
		coefficients.push_back(coefficients_futures[i].get());
	}

	return coefficients;
}

void Tester::load_codebook()
{
	const string codebook_filename = config.folder + "coeff.codebook";
	Logger::logger().log("Getting", codebook_filename);

	codebook.centroids = Utils::get_matrix_from_file<double>(codebook_filename);
}

vector<int> Tester::get_observations(string filename)
{
	vector<int> observations;
	string obs_filename = filename + ".obs";
	Logger::logger().log("Getting", obs_filename);

	vector<vector<double>> coefficients = get_coefficients(filename);
	observations = codebook.observations(coefficients);

	return observations;
}

void Tester::load_models()
{
	for (int i = 0; i < config.audio_names.size(); ++i)
	{
		Model model;
		string model_ext = ".model";
		string filename = config.folder + config.audio_names[i];
		string model_filename = filename + model_ext;
		Logger::logger().log("Getting", model_filename);

		model = Utils::get_item_from_file<Model>(model_filename);
		models.push_back(model);
	}
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
		double P = P_futures[i].get().first;
		if (max_P < P)
		{
			max_P = P;
			most_probable_word_index = i;
		}
	}

	return most_probable_word_index;
}

Tester::Tester(const Config &config) :config(config), preprocessor(320, 80),
mfcc(), codebook(config.M), thread_pool(config.n_thread * thread::hardware_concurrency())
{
	Logger::logger().add_log(config.folder + "tester.log");

	load_codebook();
	load_models();
}

int Tester::test(string filename)
{
	vector<int> observations = get_observations(config.folder + filename);
	int word_index = decide_word(observations);

	return word_index;
}
