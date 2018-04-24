#include "model-trainer.h"

#include <future>
#include <thread>
#include <utility>

#include "file-io.h"
#include "hmm.h"
#include "logger.h"
#include "lpc.h"
#include "mfc.h"
#include "wav.h"

using namespace std;

ModelTrainer::Builder::Builder(const string &folder, const vector<string> &words, const Config &config) :
	folder(folder), words(words),
	q_cache(config.get_val<bool>("q_cache", true)), n_thread(config.get_val<int>("n_thread", 4 * thread::hardware_concurrency())),
	q_trim(config.get_val<bool>("q_trim", true)), x_frame(config.get_val<int>("x_frame", 300)), x_overlap(config.get_val<int>("x_overlap", 80)),
	cepstral(config.get_val<string>("cepstral", "mfc")), n_cepstra(config.get_val<int>("n_cepstra", 12)), n_predict(config.get_val<int>("n_predict", 12)),
	q_gain(config.get_val<bool>("q_gain", false)), q_delta(config.get_val<bool>("q_delta", false)), q_accel(config.get_val<bool>("q_accel", false)),
	x_codebook(config.get_val<int>("x_codebook", 16)),
	n_state(config.get_val<int>("n_state", 5)), n_bakis(config.get_val<int>("n_bakis", 1)), n_retrain(config.get_val<int>("n_retrain", 3))
{
}

unique_ptr<ModelTrainer> ModelTrainer::Builder::build() const
{
	return unique_ptr<ModelTrainer>(new ModelTrainer(folder, words, q_cache, unique_ptr<ThreadPool>(new ThreadPool(n_thread)), Preprocessor(q_trim, x_frame, x_overlap), get_cepstral(), LBG(x_codebook), Model::Builder(n_state, x_codebook, n_bakis), n_retrain));
}

unique_ptr<ICepstral> ModelTrainer::Builder::get_cepstral() const
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

void ModelTrainer::train() const
{
	const Codebook codebook = get_codebook();

	vector<future<Model>> model_futures;
	for (int i = 0; i < words.size(); ++i)
	{
		model_futures.push_back(thread_pool->enqueue(&ModelTrainer::get_word_model, this, i, codebook));
	}
	for (int i = 0; i < words.size(); ++i)
	{
		model_futures[i].get();
	}
}

ModelTrainer::ModelTrainer(string folder, vector<string> words, bool q_cache, unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, unique_ptr<ICepstral> cepstral, LBG lbg, Model::Builder model_builder, int n_retrain) :
	folder(folder), words(words),
	q_cache(q_cache), thread_pool(move(thread_pool)),
	preprocessor(preprocessor), cepstral(move(cepstral)), lbg(lbg), model_builder(model_builder), n_retrain(n_retrain)
{
	train();
}

Codebook ModelTrainer::get_codebook() const
{
	Codebook codebook;
	const string codebook_filename = folder + codebook_ext;
	Logger::log("Getting", codebook_filename);

	if (q_cache)
	{
		codebook = FileIO::get_item_from_file<Codebook>(codebook_filename);
		if (!codebook.empty())
		{
			return codebook;
		}
	}

	const vector<Feature> universe = get_universe();
	codebook = lbg.generate(universe);
	FileIO::set_item_to_file<Codebook>(codebook, codebook_filename);

	return codebook;
}

vector<Feature> ModelTrainer::get_universe() const
{
	vector<Feature> universe;
	const string universe_filename = folder + universe_ext;
	Logger::log("Getting", universe_filename);

	if (q_cache)
	{
		universe = FileIO::get_vector_from_file<Feature>(universe_filename, '\n');
		if (!universe.empty())
		{
			return universe;
		}
	}

	vector<future<vector<Feature>>> word_universe_futures;
	for (int i = 0; i < words.size(); ++i)
	{
		const string word_filename = folder + words[i];
		word_universe_futures.push_back(thread_pool->enqueue(&ModelTrainer::get_word_universe, this, word_filename));
	}
	for (int i = 0; i < words.size(); ++i)
	{
		const vector<Feature> word_universe = word_universe_futures[i].get();
		universe.insert(universe.end(), word_universe.begin(), word_universe.end());
	}

	FileIO::set_vector_to_file<Feature>(universe, universe_filename, '\n');

	return universe;
}

vector<Feature> ModelTrainer::get_word_universe(const string &filename) const
{
	vector<Feature> word_universe;

	for (int i = 0; ; ++i)
	{
		const string utterance_filename = filename + "_" + to_string(i);
		const vector<Feature> features = get_features(utterance_filename);
		if (features.empty())
		{
			break;
		}

		word_universe.insert(word_universe.end(), features.begin(), features.end());
	}

	return word_universe;
}

vector<Feature> ModelTrainer::get_features(const string &filename) const
{
	vector<Feature> features;
	const string features_filename = filename + features_ext;
	Logger::log("Getting", features_filename);

	if (q_cache)
	{
		features = FileIO::get_vector_from_file<Feature>(features_filename);
		if (!features.empty())
		{
			return features;
		}
	}

	const string wav_filename = filename + wav_ext;
	const Wav wav_file = FileIO::get_item_from_file<Wav>(wav_filename);
	const vector<double> samples = wav_file.samples<double>();
	if (samples.empty())
	{
		// file not found
		return features;
	}

	const vector<vector<double>> frames = preprocessor.process(samples);
	features = cepstral->features(frames);
	FileIO::set_vector_to_file<Feature>(features, features_filename);

	return features;
}

Model ModelTrainer::get_word_model(int word_index, const Codebook &codebook) const
{
	Model model;
	const string model_filename = folder + to_string(word_index) + model_ext;
	Logger::log("Getting", model_filename);

	if (q_cache)
	{
		model = FileIO::get_item_from_file<Model>(model_filename);
		if (!model.empty())
		{
			return model;
		}
	}

	model = model_builder.bakis();
	for (int j = 0; j < n_retrain; ++j)
	{
		Model train_model;
		const string filename = folder + words[word_index];
		const string train_model_filename = filename + model_ext + "_" + to_string(j);
		Logger::log("Getting", train_model_filename);

		if (q_cache)
		{
			train_model = FileIO::get_item_from_file<Model>(train_model_filename);
			if (!train_model.empty())
			{
				model = train_model;
				continue;
			}
		}

		train_model = get_utterance_model(filename, codebook, model);
		FileIO::set_item_to_file<Model>(train_model, train_model_filename);
		model = train_model;
	}
	FileIO::set_item_to_file<Model>(model, model_filename);

	return model;
}

Model ModelTrainer::get_utterance_model(const string &filename, const Codebook &codebook, const Model &train_model) const
{
	vector<Model> utterance_models;

	for (int i = 0; ; ++i)
	{
		const string utterance_filename = filename + "_" + to_string(i);
		const vector<int> observations = get_observations(utterance_filename, codebook);
		if (observations.empty())
		{
			// no more utterances
			break;
		}

		utterance_models.push_back(HMM(train_model).optimise(observations));
	}

	return model_builder.merge(utterance_models);
}

vector<int> ModelTrainer::get_observations(const string &filename, const Codebook &codebook) const
{
	vector<int> observations;
	const string obs_filename = filename + observations_ext;
	Logger::log("Getting", obs_filename);

	if (q_cache)
	{
		observations = FileIO::get_vector_from_file<int>(obs_filename);
		if (!observations.empty())
		{
			return observations;
		}
	}

	const vector<Feature> features = get_features(filename);
	if (features.empty())
	{
		return observations;
	}

	observations = codebook.observations(features);
	FileIO::set_vector_to_file<int>(observations, obs_filename);

	return observations;
}
