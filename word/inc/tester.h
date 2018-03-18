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
private:
	const Parameters &parameters;
	ThreadPool thread_pool;
	const Preprocessor preprocessor;
	const std::unique_ptr<ICepstral> cepstral;
	Codebook codebook;
	std::vector<Model> models;

	/// Initialise cepstral.
	static ICepstral *setup_cepstral(const Parameters &parameters);

	/// Loads the codebook.
	void load_codebook();

	/// Loads, preprocesses the samples and returns their features.
	std::vector<std::vector<double>> get_features(std::string filename) const;

	/// Loads the models for all words.
	void load_models();

	/// Gets the observations sequence from the codebook.
	std::vector<int> get_observations(std::string filename) const;

	/// Decides the most probable word index in audio names.
	int decide_word(const std::vector<int> &observations);

public:
	/// Constructor.
	Tester(const Parameters &parameters);

	/// Recognizes the word.
	std::string test(std::string filename);
};
