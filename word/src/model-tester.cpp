#include "model-tester.h"

#include <algorithm>
#include <cmath>
#include <future>
#include <thread>

#include "file-io.h"
#include "hmm.h"
#include "logger.h"
#include "lpc.h"
#include "mfc.h"
#include "wav.h"

using namespace std;

ModelTester::Builder::Builder(const string &model_folder, const Config &config) :
	model_folder(model_folder), n_thread(config.get_val<int>("n_thread", 4 * thread::hardware_concurrency())),
	q_trim(config.get_val<bool>("q_trim", true)), x_frame(config.get_val<int>("x_frame", 300)), x_overlap(config.get_val<int>("x_overlap", 80)),
	cepstral(config.get_val<string>("cepstral", "mfc")), n_cepstra(config.get_val<int>("n_cepstra", 12)),
	q_gain(config.get_val<bool>("q_gain", false)), q_delta(config.get_val<bool>("q_delta", true)), q_accel(config.get_val<bool>("q_accel", true))
{
}

unique_ptr<ModelTester> ModelTester::Builder::build() const
{
	return unique_ptr<ModelTester>(new ModelTester(unique_ptr<ThreadPool>(new ThreadPool(n_thread)), Preprocessor(q_trim, x_frame, x_overlap), get_cepstral(), get_codebook(), get_models()));
}

unique_ptr<ICepstral> ModelTester::Builder::get_cepstral() const
{
	unique_ptr<ICepstral> icepstal;

	if (cepstral == "lpc")
	{
		icepstal.reset(new LPC(n_cepstra, q_gain, q_delta, q_accel));
	}
	else if (cepstral == "mfc")
	{
		icepstal.reset(new MFC(n_cepstra, q_gain, q_delta, q_accel));
	}

	return icepstal;
}

Codebook ModelTester::Builder::get_codebook() const
{
	Logger::log("Loading codebook");
	const string codebook_filename = model_folder + codebook_ext;

	return FileIO::get_item_from_file<Codebook>(codebook_filename);
}

vector<Model> ModelTester::Builder::get_models() const
{
	vector<Model> models;

	for (int i = 0; ; ++i)
	{
		Logger::log("Loading model:", i);
		Model model;
		const string filename = model_folder + to_string(i);
		const string model_filename = filename + model_ext;

		model = FileIO::get_item_from_file<Model>(model_filename);
		if (model.empty())
		{
			// no more models
			break;
		}

		models.push_back(model);
	}

	return models;
}

pair<bool, vector<double>> ModelTester::test(const string &filename) const
{
	pair<bool, vector<double>> scores(false, vector<double>(models.size(), 0.0));

	const vector<int> observations = get_observations(filename);
	if (observations.empty())
	{
		return scores;
	}

	scores.first = true;
	vector<future<pair<double, vector<vector<double>>>>> P_futures;
	for (int i = 0; i < models.size(); ++i)
	{
		P_futures.push_back(thread_pool->enqueue(&HMM::forward, HMM(models[i]), observations));
	}
	for (int i = 0; i < models.size(); ++i)
	{
		scores.second[i] = P_futures[i].get().first;
	}
	const double max_score = *max_element(scores.second.begin(), scores.second.end());
	for (int i = 0; i < models.size(); ++i)
	{
		// https://stats.stackexchange.com/questions/66616/converting-normalizing-very-small-likelihood-values-to-probability
		scores.second[i] = exp(scores.second[i] - max_score);
	}

	return scores;
}

ModelTester::ModelTester(unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, unique_ptr<ICepstral> cepstral, Codebook codebook, vector<Model> models) :
	thread_pool(move(thread_pool)),
	preprocessor(preprocessor), cepstral(move(cepstral)), codebook(codebook), models(models)
{
}

vector<int> ModelTester::get_observations(const string &filename) const
{
	vector<int> observations;
	Logger::log("Getting observations");

	const vector<Feature> features = get_features(filename);
	if (features.empty())
	{
		return observations;
	}

	observations = codebook.observations(features);

	return observations;
}

vector<Feature> ModelTester::get_features(const string &filename) const
{
	vector<Feature> features;
	Logger::log("Getting features");

	const string wav_filename = filename + wav_ext;
	const Wav wav_file = FileIO::get_item_from_file<Wav>(wav_filename);
	const vector<double> samples = wav_file.samples<double>();
	if (samples.empty())
	{
		// wav file not found
		return features;
	}

	const vector<vector<double>> frames = preprocessor.process(samples);
	features = cepstral->features(frames);

	return features;
}
