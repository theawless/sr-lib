#include <string>
#include <vector>

#include "codebook.h"
#include "config.h"
#include "mfcc.h"
#include "model.h"
#include "preprocess.h"
#include "threads.h"

class Trainer
{
private:
	Config config;
	ThreadPool thread_pool;
	Preprocessor preprocessor;
	MFCC mfcc;
	Codebook codebook;
	std::vector<Model> models;

	/// Loads, preprocesses the amplitudes and then returns their lpc coefficients.
	std::vector<std::vector<double>> get_coefficients(std::string filename);

	/// Builds the universe by accumulation lpcs of all frames of all signals.
	std::vector<std::vector<double>> get_universe();

	/// Builds a codebook from the universe using lbg.
	void build_codebook();

	/// Gets the observations sequence from the codebook.
	std::vector<int> get_observations(std::string filename);

	/// Optimises the given train model use the observations of the given filename.
	Model get_utterance_model(const Model &train_model, int word_index, int train_index, int utterance_index);

	/// Gets the model for all utterances and merges them.
	Model get_word_model(const Model &train_model, int word_index, int train_index);

	/// Trains the models for all words.
	void build_models();

public:
	/// Constructor.
	Trainer(const Config &config);
};
