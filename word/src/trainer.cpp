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

ModelTrainer::Builder::Builder(const string &folder, const vector<pair<string, int>> &utterances, const Config &config) : folder(folder), utterances(utterances),
n_thread(config.get_val<int>("n_thread", 4 * thread::hardware_concurrency())),
x_frame(config.get_val<int>("x_frame", 300)), x_overlap(config.get_val<int>("x_overlap", 80)),
cepstral(config.get_val<string>("cepstral", "mfc")), n_cepstra(config.get_val<int>("n_cepstra", 12)), n_predict(config.get_val<int>("n_predict", 12)),
q_gain(config.get_val<bool>("q_gain", false)), q_delta(config.get_val<bool>("q_delta", false)), q_accel(config.get_val<bool>("q_accel", false)),
x_codebook(config.get_val<int>("x_codebook", 16)),
n_state(config.get_val<int>("n_state", 5)), n_bakis(config.get_val<int>("n_bakis", 1)), n_retrain(config.get_val<int>("n_retrain", 3))
{
}

unique_ptr<ModelTrainer> ModelTrainer::Builder::build() const
{
	return unique_ptr<ModelTrainer>(new ModelTrainer(folder, utterances, unique_ptr<ThreadPool>(new ThreadPool(n_thread)), Preprocessor(x_frame, x_overlap), get_cepstral(), LBG(x_codebook), Model::Builder(n_state, x_codebook, n_bakis), n_retrain));
}

unique_ptr<ICepstral> ModelTrainer::Builder::get_cepstral() const
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

ModelTrainer::ModelTrainer(const string &folder, const vector<pair<string, int>> &utterances, unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, unique_ptr<ICepstral> cepstral, LBG lbg, Model::Builder model_builder, int n_retrain) :
	folder(folder), utterances(utterances), thread_pool(move(thread_pool)), preprocessor(preprocessor), cepstral(move(cepstral)), lbg(lbg), model_builder(model_builder), n_retrain(n_retrain)
{
	const Codebook codebook = get_codebook();
	for (int i = 0; i < utterances.size(); ++i)
	{
		Model model;
		const string model_ext = ".model";
		const string filename = folder + utterances[i].first;
		const string model_filename = filename + model_ext;
		Logger::log("Getting", model_filename);

		model = Utils::get_item_from_file<Model>(model_filename);
		if (!model.empty())
		{
			continue;
		}

		model = model_builder.bakis();
		for (int j = 0; j < n_retrain; ++j)
		{
			model = get_word_model(model, i, j, codebook);
		}
		Utils::set_item_to_file<Model>(model, model_filename);
	}
}

Codebook ModelTrainer::get_codebook() const
{
	Codebook codebook;
	const string codebook_filename = folder + "words.codebook";
	Logger::log("Getting", codebook_filename);

	codebook = Utils::get_item_from_file<Codebook>(codebook_filename);
	if (!codebook.centroids.empty())
	{
		return codebook;
	}

	const vector<Feature> universe = get_universe();
	codebook = lbg.generate(universe);
	Utils::set_item_to_file<Codebook>(codebook, codebook_filename);

	return codebook;
}

vector<Feature> ModelTrainer::get_universe() const
{
	vector<Feature> universe;
	const string universe_filename = folder + "words.universe";
	Logger::log("Getting", universe_filename);

	universe = Utils::get_vector_from_file<Feature>(universe_filename, '\n');
	if (!universe.empty())
	{
		return universe;
	}

	for (int i = 0; i < utterances.size(); ++i)
	{
		vector<future<vector<Feature>>> features_futures;
		for (int j = 0; j < utterances[i].second; ++j)
		{
			const string filename = folder + utterances[i].first + "_" + to_string(j);
			features_futures.push_back(thread_pool->enqueue(&ModelTrainer::get_features, this, filename));
		}
		for (int j = 0; j < utterances[i].second; ++j)
		{
			const vector<Feature> features = features_futures[j].get();
			universe.insert(universe.end(), features.begin(), features.end());
		}
	}
	Utils::set_vector_to_file<Feature>(universe, universe_filename, '\n');

	return universe;
}

vector<Feature> ModelTrainer::get_features(const string &filename) const
{
	vector<Feature> features;
	const string features_filename = filename + ".features";
	Logger::log("Getting", features_filename);

	features = Utils::get_vector_from_file<Feature>(features_filename);
	if (!features.empty())
	{
		return features;
	}

	const Wav wav_file(filename + ".wav");
	const vector<vector<double>> frames = preprocessor.process(wav_file.samples<double>());
	features = cepstral->features(frames);
	Utils::set_vector_to_file<Feature>(features, features_filename);

	return features;
}

Model ModelTrainer::get_word_model(const Model &train_model, int word_index, int train_index, const Codebook &codebook) const
{
	Model model;
	const string model_ext = ".model";
	const string filename = folder + utterances[word_index].first;
	const string model_filename = filename + model_ext + "_" + to_string(train_index);
	Logger::log("Getting", model_filename);

	model = Utils::get_item_from_file<Model>(model_filename);
	if (!model.empty())
	{
		return model;
	}

	vector<Model> utterance_models;
	vector<future<Model>> utterance_model_futures;
	for (int i = 0; i < utterances[word_index].second; ++i)
	{
		utterance_model_futures.push_back(thread_pool->enqueue(&ModelTrainer::get_utterance_model, this, train_model, word_index, train_index, i, codebook));
	}
	for (int i = 0; i < utterances[word_index].second; ++i)
	{
		utterance_models.push_back(utterance_model_futures[i].get());
	}
	model = model_builder.merge(utterance_models);
	Utils::set_item_to_file<Model>(model, model_filename);

	return model;
}

Model ModelTrainer::get_utterance_model(const Model &train_model, int word_index, int train_index, int utterance_index, const Codebook &codebook) const
{
	Model model;
	const string model_ext = ".model";
	const string filename = folder + utterances[word_index].first + "_" + to_string(utterance_index);
	const string model_filename = filename + model_ext + "_" + to_string(train_index);
	Logger::log("Getting", model_filename);

	model = Utils::get_item_from_file<Model>(model_filename);
	if (!model.empty())
	{
		return model;
	}

	const vector<int> observations = get_observations(filename, codebook);
	model = HMM(train_model).optimise(observations);
	Utils::set_item_to_file<Model>(model, model_filename);
	return model;
}

vector<int> ModelTrainer::get_observations(const string &filename, const Codebook &codebook) const
{
	vector<int> observations;
	const string obs_filename = filename + ".observations";
	Logger::log("Getting", obs_filename);

	observations = Utils::get_vector_from_file<int>(obs_filename);
	if (!observations.empty())
	{
		return observations;
	}
	move(filename);

	const vector<Feature> features = get_features(filename);
	observations = codebook.observations(features);
	Utils::set_vector_to_file<int>(observations, obs_filename);

	return observations;
}
