#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "config.h"
#include "logger.h"
#include "tester.h"
#include "trainer.h"
#include "utils.h"

using namespace std;

int main()
{
	const string folder = "B:\\record\\digit_0.8_2\\";

	const string config_filename = folder + "sr-lib.config";
	const string word_config_filename = folder + "words.config";
	const Config config = Utils::get_item_from_file<Config>(config_filename);
	const WordConfig word_config = Utils::get_item_from_file<WordConfig>(word_config_filename);

	const vector<pair<string, int>> all_utterances = word_config.utterances();
	vector<pair<string, int>> train_utterances(all_utterances);
	for (int i = 0; i < train_utterances.size(); ++i)
	{
		train_utterances[i].second *= 0.70;
	}

	Logger::info("Training...");
	const unique_ptr<ModelTrainer> model_trainer = ModelTrainer::Builder(folder, train_utterances, config).build();
	const unique_ptr<Tester> tester = Tester::Builder(folder, word_config.words(), config).build();

	vector<int> n_hits(all_utterances.size(), 0), n_errs(all_utterances.size(), 0);
	for (int i = 0; i < all_utterances.size(); ++i)
	{
		for (int j = train_utterances[i].second; j < all_utterances[i].second; ++j)
		{
			const string test_filename = all_utterances[i].first + "_" + to_string(j);
			const string word = tester->test(test_filename);

			if (word == all_utterances[i].first)
			{
				n_hits[i]++;
			}
			if (word == "")
			{
				n_errs[i]++;
			}
			Logger::info("The recognised word is:", word);
		}
	}

	Logger::info("n_hits is:", accumulate(n_hits.begin(), n_hits.end(), 0));
	Logger::info("n_errs is:", accumulate(n_errs.begin(), n_errs.end(), 0));
	cin.get();
}
