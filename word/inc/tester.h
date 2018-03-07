#include <string>
#include <vector>

#include "codebook.h"
#include "config.h"
#include "mfcc.h"
#include "model.h"
#include "preprocess.h"
#include "threads.h"

class Tester
{
private:
	Config config;
	ThreadPool thread_pool;
	Preprocessor preprocessor;
	MFCC mfcc;
	Codebook codebook;
	std::vector<Model> models;

	/// Loads, preprocesses the amplitudes and then returns their coefficients.
	std::vector<std::vector<double>> get_coefficients(std::string filename);

	/// Loads the codebook.
	void load_codebook();

	/// Gets the observations sequence from the codebook.
	std::vector<int> get_observations(std::string filename);

	/// Loads the models for all words.
	void load_models();

	/// Decides the most probable word index in audio names.
	int decide_word(const std::vector<int> &observations);

public:
	/// Constructor.
	Tester(const Config &config);

	/// Recognizes the word.
	int test(std::string filename);
};
