#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "codebook.h"
#include "config.h"
#include "features.h"
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
		Builder(const std::string &folder, const std::vector<std::pair<std::string, int>> &utterances, const Config &config);

		/// Builds the CodebookTrainer.
		std::unique_ptr<ModelTrainer> build() const;

	private:
		const std::string folder;
		const std::vector<std::pair<std::string, int>> utterances;
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

	/// Constructor.
	ModelTrainer(const std::string &folder, const std::vector<std::pair<std::string, int>> &utterances, std::unique_ptr<ThreadPool> thread_pool, Preprocessor preprocessor, std::unique_ptr<ICepstral> cepstral, LBG lbg, Model::Builder model_builder, int n_retrain);

private:
	const std::string folder;
	const std::vector<std::pair<std::string, int>> utterances;
	const std::unique_ptr<ThreadPool> thread_pool;
	const Preprocessor preprocessor;
	const std::unique_ptr<ICepstral> cepstral;
	const LBG lbg;
	const Model::Builder model_builder;
	const int n_retrain;

	/// Builds a codebook from the universe using lbg.
	Codebook get_codebook() const;

	/// Builds the universe by accumulating features from all utterances.
	std::vector<Feature> get_universe() const;

	/// Loads and preprocesses the samples and returns their features.
	std::vector<Feature> get_features(const std::string &filename) const;

	/// Gets the model for all utterances and merges them.
	Model get_word_model(const Model &train_model, int word_index, int train_index, const Codebook &codebook) const;

	/// Optimises the given train model use the observations of the given filename.
	Model get_utterance_model(const Model &train_model, int word_index, int train_index, int utterance_index, const Codebook &codebook) const;

	/// Gets the observations sequence from the codebook.
	std::vector<int> get_observations(const std::string &filename, const Codebook &codebook) const;
};
