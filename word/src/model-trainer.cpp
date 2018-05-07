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

ModelTrainer::Builder::Builder(const string &train_folder, const string &model_folder, const vector<string> &words, const Config &config) :
	train_folder(train_folder), model_folder(model_folder), words(words),
	q_cache(config.get_val<bool>("q_cache", true)), n_thread(config.get_val<int>("n_thread", 4 * thread::hardware_concurrency())),
	q_trim(config.get_val<bool>("q_trim", true)), x_frame(config.get_val<int>("x_frame", 300)), x_overlap(config.get_val<int>("x_overlap", 80)),
	cepstral(config.get_val<string>("cepstral", "mfc")), n_cepstra(config.get_val<int>("n_cepstra", 12)),
	q_gain(config.get_val<bool>("q_gain", false)), q_delta(config.get_val<bool>("q_delta", true)), q_accel(config.get_val<bool>("q_accel", true)),
	x_codebook(config.get_val<int>("x_codebook", 128)),
	n_state(config.get_val<int>("n_state", 15)), n_bakis(config.get_val<int>("n_bakis", 3)), n_retrain(config.get_val<int>("n_retrain", 3))
{
}

unique_ptr<ModelTrainer> ModelTrainer::Builder::build() const
{
	return unique_ptr<ModelTrainer>(new ModelTrainer(train_folder, model_folder, words, q_cache, unique_ptr<ThreadPool>(new ThreadPool(n_thread)), Preprocessor(q_trim, x_frame, x_overlap), get_cepstral(), LBG(x_codebook), Model::Builder(n_state, x_codebook, n_bakis), n_retrain));
}

unique_ptr<ICepstral> ModelTrainer::Builder::get_cepstral() const
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

ModelTrainer::ModelTrainer(string train_folder, string model_folder, vector<string> words, bool q_cache, unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, unique_ptr<ICepstral> cepstral, LBG lbg, Model::Builder model_builder, int n_retrain) :
	train_folder(train_folder), model_folder(model_folder), words(words),
	q_cache(q_cache), thread_pool(move(thread_pool)),
	preprocessor(preprocessor), cepstral(move(cepstral)), lbg(lbg), model_builder(model_builder), n_retrain(n_retrain)
{
	train();
}

Codebook ModelTrainer::get_codebook() const
{
	Codebook codebook;
	const string codebook_filename = model_folder + codebook_ext;
	Logger::log("Getting codebook");

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
	Logger::log("Getting universe");
	vector<Feature> universe;
	const string universe_filename = model_folder + universe_ext;

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
		word_universe_futures.push_back(thread_pool->enqueue(&ModelTrainer::get_word_universe, this, i));
	}
	for (int i = 0; i < words.size(); ++i)
	{
		const vector<Feature> word_universe = word_universe_futures[i].get();
		universe.insert(universe.end(), word_universe.begin(), word_universe.end());
	}

	FileIO::set_vector_to_file<Feature>(universe, universe_filename, '\n');

	return universe;
}

vector<Feature> ModelTrainer::get_word_universe(int word_index) const
{
	vector<Feature> word_universe;

	for (int i = 0; ; ++i)
	{
		const vector<Feature> features = get_features(i, word_index);
		if (features.empty())
		{
			break;
		}

		word_universe.insert(word_universe.end(), features.begin(), features.end());
	}

	return word_universe;
}

vector<Feature> ModelTrainer::get_features(int utterance_index, int word_index) const
{
	Logger::log("Getting features:", word_index, utterance_index);
	vector<Feature> features;
	const string features_filename = model_folder + words[word_index] + '_' + to_string(utterance_index) + features_ext;

	if (q_cache)
	{
		features = FileIO::get_vector_from_file<Feature>(features_filename);
		if (!features.empty())
		{
			return features;
		}
	}

	const string wav_filename = train_folder + words[word_index] + '_' + to_string(utterance_index) + wav_ext;
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
	Logger::log("Getting model:", word_index);
	Model model;
	const string model_filename = model_folder + to_string(word_index) + model_ext;

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
		Logger::log("Getting train model:", word_index, j);
		Model train_model;
		const string train_model_filename = model_folder + words[word_index] + model_ext + "_" + to_string(j);

		if (q_cache)
		{
			train_model = FileIO::get_item_from_file<Model>(train_model_filename);
			if (!train_model.empty())
			{
				model = train_model;
				continue;
			}
		}

		train_model = get_utterance_model(word_index, model, codebook);
		FileIO::set_item_to_file<Model>(train_model, train_model_filename);
		model = train_model;
	}
	FileIO::set_item_to_file<Model>(model, model_filename);

	return model;
}

Model ModelTrainer::get_utterance_model(int word_index, const Model &train_model, const Codebook &codebook) const
{
	vector<Model> utterance_models;

	for (int i = 0; ; ++i)
	{
		const vector<int> observations = get_observations(i, word_index, codebook);
		if (observations.empty())
		{
			// no more utterances
			break;
		}

		utterance_models.push_back(HMM(train_model).optimise(observations));
	}

	return model_builder.merge(utterance_models);
}

vector<int> ModelTrainer::get_observations(int utterance_index, int word_index, const Codebook &codebook) const
{
	Logger::log("Getting observations:", word_index, utterance_index);
	vector<int> observations;
	const string obs_filename =  + observations_ext;

	if (q_cache)
	{
		observations = FileIO::get_vector_from_file<int>(obs_filename);
		if (!observations.empty())
		{
			return observations;
		}
	}

	const vector<Feature> features = get_features(utterance_index, word_index);
	if (features.empty())
	{
		return observations;
	}

	observations = codebook.observations(features);
	FileIO::set_vector_to_file<int>(observations, obs_filename);

	return observations;
}
