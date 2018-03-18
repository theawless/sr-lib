#include <memory>
#include <string>
#include <vector>

#include "codebook.h"
#include "config.h"
#include "features.h"
#include "model.h"
#include "preprocess.h"
#include "threads.h"

class Trainer
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

	/// Builds a codebook from the universe using lbg.
	void build_codebook();

	/// Builds the universe by accumulation lpcs of all frames of all signals.
	std::vector<std::vector<double>> get_universe();

	/// Loads, preprocesses the samples and returns their features.
	std::vector<std::vector<double>> get_features(std::string filename) const;

	/// Trains the models for all words.
	void build_models();

	/// Gets the model for all utterances and merges them.
	Model get_word_model(const Model &train_model, int word_index, int train_index);

	/// Optimises the given train model use the observations of the given filename.
	Model get_utterance_model(const Model &train_model, int word_index, int train_index, int utterance_index) const;

	/// Gets the observations sequence from the codebook.
	std::vector<int> get_observations(std::string filename) const;

public:
	/// Constructor.
	Trainer(const Parameters &parameters);
};
