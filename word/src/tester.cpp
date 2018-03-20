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

Tester::Builder::Builder(const string &folder, const vector<string> &words, const Config &config) : folder(folder), words(words),
n_thread(config.get_val<int>("n_thread", 4 * thread::hardware_concurrency())),
x_frame(config.get_val<int>("x_frame", 300)), x_overlap(config.get_val<int>("x_overlap", 80)),
cepstral(config.get_val<string>("cepstral", "mfc")), n_cepstra(config.get_val<int>("n_cepstra", 12)), n_predict(config.get_val<int>("n_predict", 12)),
q_gain(config.get_val<bool>("q_gain", false)), q_delta(config.get_val<bool>("q_delta", false)), q_accel(config.get_val<bool>("q_accel", false))
{
}

unique_ptr<Tester> Tester::Builder::build() const
{
	return unique_ptr<Tester>(new Tester(folder, words, unique_ptr<ThreadPool>(new ThreadPool(n_thread)), Preprocessor(x_frame, x_overlap), get_cepstral(), get_codebook(), get_models()));
}

unique_ptr<ICepstral> Tester::Builder::get_cepstral() const
{
	if (cepstral == "lpc")
	{
		return unique_ptr<LPC>(new LPC(n_cepstra, q_gain, q_delta, q_accel, n_predict));
	}
	else if (cepstral == "mfc")
	{
		return unique_ptr<MFC>(new MFC(n_cepstra, q_gain, q_delta, q_accel));
	}
	else
	{
		return nullptr;
	}
}

Codebook Tester::Builder::get_codebook() const
{
	const string codebook_filename = folder + "words.codebook";
	Logger::log("Getting", codebook_filename);

	return Utils::get_item_from_file<Codebook>(codebook_filename);
}

vector<Model> Tester::Builder::get_models() const
{
	vector<Model> models;

	for (int i = 0; i < words.size(); ++i)
	{
		Model model;
		const string model_ext = ".model";
		const string filename = folder + words[i];
		const string model_filename = filename + model_ext;
		Logger::log("Getting", model_filename);

		model = Utils::get_item_from_file<Model>(model_filename);
		models.push_back(model);
	}

	return models;
}

Tester::Tester(const string &folder, const vector<string> &words, unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, unique_ptr<ICepstral> cepstral, Codebook codebook, vector<Model> models) :
	folder(folder), words(words), thread_pool(move(thread_pool)), preprocessor(preprocessor), cepstral(move(cepstral)), codebook(codebook), models(models)
{
}

string Tester::test(const string &filename) const
{
	const vector<int> observations = get_observations(folder + filename);
	const int word_index = decide_word(observations);

	return word_index < words.size() ? words[word_index] : "";
}

vector<Feature> Tester::get_features(const string &filename) const
{
	vector<Feature> features;
	const string features_filename = filename + ".features";
	Logger::log("Getting", features_filename);

	const Wav wav_file(filename + ".wav");
	const vector<vector<double>> frames = preprocessor.process(wav_file.samples<double>());
	features = cepstral->features(frames);

	return features;
}

vector<int> Tester::get_observations(const string &filename) const
{
	vector<int> observations;
	const string obs_filename = filename + ".observations";
	Logger::log("Getting", obs_filename);

	const vector<Feature> features = get_features(filename);
	observations = codebook.observations(features);

	return observations;
}

int Tester::decide_word(const vector<int> &observations) const
{
	vector<future<pair<double, vector<vector<double>>>>> P_futures;
	double max_P = numeric_limits<double>::min();
	int most_probable_word_index = -1;

	for (int i = 0; i < models.size(); ++i)
	{
		P_futures.push_back(thread_pool->enqueue(&HMM::forward, HMM(models[i]), observations));
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
