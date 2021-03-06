#pragma once

#include <memory>
#include <string>
#include <vector>

#include "codebook.h"
#include "config.h"
#include "feature.h"
#include "model.h"
#include "preprocess.h"
#include "threads.h"

class ModelTrainer
{
public:
	class Builder
	{
	public:
		/// Constructor.
		Builder(const std::string &train_folder, const std::string &model_folder, const std::vector<std::string> &words, const Config &config);

		/// Build the ModelTrainer.
		std::unique_ptr<ModelTrainer> build() const;

	private:
		const std::string train_folder;
		const std::string model_folder;
		const std::vector<std::string> words;
		const bool q_cache;
		const int n_thread;
		const bool q_trim;
		const int x_frame;
		const int x_overlap;
		const std::string cepstral;
		const int n_cepstra;
		const bool q_gain;
		const bool q_delta;
		const bool q_accel;
		const int x_codebook;
		const int n_state;
		const int n_bakis;
		const int n_retrain;

		/// Initialise cepstral.
		std::unique_ptr<ICepstral> get_cepstral() const;
	};

	/// Train.
	void train() const;

private:
	static constexpr char const *wav_ext = ".wav";
	static constexpr char const *features_ext = ".features";
	static constexpr char const *universe_ext = "sr-lib.universe";
	static constexpr char const *codebook_ext = "sr-lib.codebook";
	static constexpr char const *observations_ext = ".observations";
	static constexpr char const *model_ext = ".model";

	const std::string train_folder;
	const std::string model_folder;
	const std::vector<std::string> words;
	const bool q_cache;
	const std::unique_ptr<ThreadPool> thread_pool;
	const Preprocessor preprocessor;
	const std::unique_ptr<ICepstral> cepstral;
	const LBG lbg;
	const Model::Builder model_builder;
	const int n_retrain;

	/// Constructor.
	ModelTrainer(std::string train_folder, std::string model_folder, std::vector<std::string> words, bool q_cache, std::unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, std::unique_ptr<ICepstral> cepstral, LBG lbg, Model::Builder model_builder, int n_retrain);

	/// Build the codebook using lbg.
	Codebook get_codebook() const;

	/// Build the universe by accumulating features from all words.
	std::vector<Feature> get_universe() const;

	/// Build the word universe by accumulating features from all utterances.
	std::vector<Feature> get_word_universe(int word_index) const;

	/// Load and preprocess the samples, and return their features.
	std::vector<Feature> get_features(int utterance_index, int word_index) const;

	/// Get the model for given word index.
	Model get_word_model(int word_index, const Codebook &codebook) const;

	/// Optimise the train model using the observations from utterances of the given filename.
	Model get_utterance_model(int word_index, const Model &train_model, const Codebook &codebook) const;

	/// Get the observations sequence from the codebook.
	std::vector<int> get_observations(int utterance_index, int word_index, const Codebook &codebook) const;
};
