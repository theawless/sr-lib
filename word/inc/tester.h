#include <memory>
#include <string>
#include <vector>

#include "codebook.h"
#include "config.h"
#include "features.h"
#include "model.h"
#include "preprocess.h"
#include "threads.h"

class Tester
{
public:
	class Builder
	{
	public:
		/// Constructor.
		Builder(const std::string &folder, const std::vector<std::string> &words, const Config &config);

		/// Builds the tester.
		std::unique_ptr<Tester> build() const;

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

		/// Initialise cepstral.
		std::unique_ptr<ICepstral> get_cepstral() const;

		/// Loads the codebook.
		Codebook get_codebook() const;

		/// Loads the models for all words.
		std::vector<Model> get_models() const;
	};

	/// Constructor.
	Tester(const std::string &folder, const std::vector<std::string> &words, std::unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, std::unique_ptr<ICepstral> cepstral, Codebook codebook, std::vector<Model> models);

	/// Recognizes the word.
	std::string test(const std::string &filename) const;

private:
	const std::string folder;
	const std::vector<std::string> words;
	const std::unique_ptr<ThreadPool> thread_pool;
	const Preprocessor preprocessor;
	const std::unique_ptr<ICepstral> cepstral;
	const Codebook codebook;
	const std::vector<Model> models;

	/// Loads, preprocesses the samples and returns their features.
	std::vector<Feature> get_features(const std::string &filename) const;

	/// Gets the observations sequence from the codebook.
	std::vector<int> get_observations(const std::string &filename) const;

	/// Decides the most probable word index in audio names.
	int decide_word(const std::vector<int> &observations) const;
};
