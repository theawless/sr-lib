#include <numeric>
#include <string>
#include <vector>

#include "config.h"
#include "logger.h"
#include "tester.h"
#include "trainer.h"

using namespace std;

int main()
{
	const string folder = "B:\\record\\digit_0.8_2\\";
	Config config(folder + "sr-lib.config");
	WordConfig word_config(folder + "words.config");
	config.load(); word_config.load();

	vector<pair<string, int>> saved_words = word_config.words();
	vector<pair<string, int>> new_words(saved_words);
	for (int i = 0; i < new_words.size(); ++i)
	{
		new_words[i].second *= 0.70;
	}

	Logger::info("Training");
	Parameters parameters(folder, new_words, config);
	Trainer trainer(parameters);
	Tester tester(parameters);

	vector<int> n_hits(saved_words.size(), 0), n_errs(saved_words.size(), 0);
	for (int i = 0; i < saved_words.size(); ++i)
	{
		for (int j = new_words[i].second; j < saved_words[i].second; ++j)
		{
			string test_filename = saved_words[i].first + "_" + to_string(j);
			string word = tester.test(test_filename);

			if (word == saved_words[i].first)
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
