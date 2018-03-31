#include "model-tester.h"

#include <algorithm>
#include <cmath>
#include <future>

#include "hmm.h"
#include "logger.h"
#include "lpc.h"
#include "mfc.h"
#include "utils.h"
#include "wav.h"

using namespace std;

ModelTester::Builder::Builder(const string &folder, const Config &config) :
	folder(folder), n_thread(config.get_val<int>("n_thread", 4 * thread::hardware_concurrency())),
	x_frame(config.get_val<int>("x_frame", 300)), x_overlap(config.get_val<int>("x_overlap", 80)),
	cepstral(config.get_val<string>("cepstral", "mfc")), n_cepstra(config.get_val<int>("n_cepstra", 12)), n_predict(config.get_val<int>("n_predict", 12)),
	q_gain(config.get_val<bool>("q_gain", false)), q_delta(config.get_val<bool>("q_delta", false)), q_accel(config.get_val<bool>("q_accel", false))
{
}

unique_ptr<ModelTester> ModelTester::Builder::build() const
{
	return unique_ptr<ModelTester>(new ModelTester(unique_ptr<ThreadPool>(new ThreadPool(n_thread)), Preprocessor(x_frame, x_overlap), get_cepstral(), get_codebook(), get_models()));
}

unique_ptr<ICepstral> ModelTester::Builder::get_cepstral() const
{
	unique_ptr<ICepstral> icepstal;

	if (cepstral == "lpc")
	{
		icepstal.reset(new LPC(n_cepstra, q_gain, q_delta, q_accel, n_predict));
	}
	else if (cepstral == "mfc")
	{
		icepstal.reset(new MFC(n_cepstra, q_gain, q_delta, q_accel));
	}

	return icepstal;
}

Codebook ModelTester::Builder::get_codebook() const
{
	const string codebook_filename = folder + codebook_ext;
	Logger::log("Getting", codebook_filename);

	return Utils::get_item_from_file<Codebook>(codebook_filename);
}

vector<Model> ModelTester::Builder::get_models() const
{
	vector<Model> models;

	for (int i = 0; ; ++i)
	{
		Model model;
		const string filename = folder + to_string(i);
		const string model_filename = filename + model_ext;
		Logger::log("Getting", model_filename);

		model = Utils::get_item_from_file<Model>(model_filename);
		if (model.empty())
		{
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
	const string obs_filename = filename + observations_ext;
	Logger::log("Getting", obs_filename);

	const vector<Feature> features = get_features(filename);
	observations = codebook.observations(features);

	return observations;
}

vector<Feature> ModelTester::get_features(const string &filename) const
{
	vector<Feature> features;
	const string features_filename = filename + features_ext;
	Logger::log("Getting", features_filename);

	const string wav_filename = filename + wav_ext;
	const Wav wav_file(wav_filename);
	const vector<double> samples = wav_file.samples<double>();
	const vector<vector<double>> frames = preprocessor.process(samples);
	features = cepstral->features(frames);

	return features;
}
