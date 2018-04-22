#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "codebook.h"
#include "config.h"
#include "feature.h"
#include "model.h"
#include "preprocess.h"
#include "threads.h"

class ModelTester
{
public:
	class Builder
	{
	public:
		/// Constructor.
		Builder(const std::string &folder, const Config &config);

		/// Build the ModelTester.
		std::unique_ptr<ModelTester> build() const;

	private:
		static constexpr char const *codebook_ext = "sr-lib.codebook";
		static constexpr char const *model_ext = ".model";

		const std::string folder;
		const int n_thread;
		const bool q_trim;
		const int x_frame;
		const int x_overlap;
		const std::string cepstral;
		const int n_cepstra;
		const int n_predict;
		const bool q_gain;
		const bool q_delta;
		const bool q_accel;

		/// Initialise cepstral.
		std::unique_ptr<ICepstral> get_cepstral() const;

		/// Load the codebook.
		Codebook get_codebook() const;

		/// Load the models.
		std::vector<Model> get_models() const;
	};

	/// Return the scores for all models.
	std::pair<bool, std::vector<double>> test(const std::string &filename) const;

private:
	static constexpr char const *wav_ext = ".wav";

	const std::unique_ptr<ThreadPool> thread_pool;
	const Preprocessor preprocessor;
	const std::unique_ptr<ICepstral> cepstral;
	const Codebook codebook;
	const std::vector<Model> models;

	/// Constructor.
	ModelTester(std::unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, std::unique_ptr<ICepstral> cepstral, Codebook codebook, std::vector<Model> models);

	/// Get the observations sequence from the codebook.
	std::vector<int> get_observations(const std::string &filename) const;

	/// Load and preprocess the samples, and return their features.
	std::vector<Feature> get_features(const std::string &filename) const;
};
