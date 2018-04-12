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
		Builder(const std::string &folder, const std::vector<std::string> &words, const Config &config);

		/// Build the ModelTrainer.
		std::unique_ptr<ModelTrainer> build() const;

	private:
		const std::string folder;
		const std::vector<std::string> words;
		const int n_thread;
		const int x_frame;
		const int x_overlap;
		const std::string cepstral;
		const int n_cepstra;
		const int n_predict;
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
	static constexpr char wav_ext[] = ".wav";
	static constexpr char features_ext[] = ".features";
	static constexpr char universe_ext[] = "sr-lib.universe";
	static constexpr char codebook_ext[] = "sr-lib.codebook";
	static constexpr char observations_ext[] = ".observations";
	static constexpr char model_ext[] = ".model";

	const std::string folder;
	const std::vector<std::string> words;
	const std::unique_ptr<ThreadPool> thread_pool;
	const Preprocessor preprocessor;
	const std::unique_ptr<ICepstral> cepstral;
	const LBG lbg;
	const Model::Builder model_builder;
	const int n_retrain;

	/// Constructor.
	ModelTrainer(std::string folder, std::vector<std::string> words, std::unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, std::unique_ptr<ICepstral> cepstral, LBG lbg, Model::Builder model_builder, int n_retrain);

	/// Build the codebook using lbg.
	Codebook get_codebook() const;

	/// Build the universe by accumulating features from all words.
	std::vector<Feature> get_universe() const;

	/// Build the word universe by accumulating features from all utterances.
	std::vector<Feature> get_word_universe(const std::string &filename) const;

	/// Load and preprocess the samples, and return their features.
	std::vector<Feature> get_features(const std::string &filename) const;

	/// Get the model for given word index.
	Model get_word_model(int word_index, const Codebook &codebook) const;

	/// Optimise the train model using the observations from utterances of the given filename.
	Model get_utterance_model(const std::string &filename, const Codebook &codebook, const Model &train_model) const;

	/// Get the observations sequence from the codebook.
	std::vector<int> get_observations(const std::string &filename, const Codebook &codebook) const;
};
