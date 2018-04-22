#include <chrono>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "logger.h"
#include "model-tester.h"
#include "model-trainer.h"
#include "utils.h"

using namespace std;

int main()
{
	const string folder = "res/";

	const string config_filename = folder + "sr-lib.config";
	const string words_filename = folder + "sr-lib.words";
	const Config config = Utils::get_item_from_file<Config>(config_filename);
	const vector<string> words = Utils::get_vector_from_file<string>(words_filename);

	Logger::info("Training...");
	chrono::steady_clock::time_point start = chrono::steady_clock::now();
	const string train_folder = folder + "train/";
	const unique_ptr<ModelTrainer> model_trainer = ModelTrainer::Builder(train_folder, words, config).build();
	const unique_ptr<ModelTester> model_tester = ModelTester::Builder(train_folder, config).build();
	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	chrono::seconds time = chrono::duration_cast<chrono::seconds>(end - start);
	Logger::info("Time taken:", time.count(), "seconds");

	Logger::info("Testing...");
	start = chrono::steady_clock::now();
	const string test_folder = folder + "test/";
	vector<int> n_utterances(words.size(), 0), n_hits(words.size(), 0), n_errs(words.size(), 0);
	for (int i = 0; i < words.size(); ++i)
	{
		for (int j = 0; ; ++j)
		{
			const string test_filename = test_folder + words[i] + '_' + to_string(j);
			const pair<bool, vector<double>> scores = model_tester->test(test_filename);
			if (!scores.first)
			{
				break;
			}

			const int word_index = max_element(scores.second.begin(), scores.second.end()) - scores.second.begin();
			if (scores.second[word_index] == 0.0)
			{
				n_errs[i]++;
			}
			else if (word_index == i)
			{
				n_hits[i]++;
			}
			n_utterances[i]++;
		}
	}
	end = chrono::steady_clock::now();
	time = chrono::duration_cast<chrono::seconds>(end - start);
	Logger::info("Time taken:", time.count(), "seconds");

	const int total_utterances = accumulate(n_utterances.begin(), n_utterances.end(), 0);
	const int total_hits = accumulate(n_hits.begin(), n_hits.end(), 0);
	const int total_errs = accumulate(n_errs.begin(), n_errs.end(), 0);
	Logger::info("Hit percent is:", (total_hits * 100.0) / total_utterances);
	Logger::info("Err percent is:", (total_errs * 100.0) / total_utterances);
	cin.get();
}
