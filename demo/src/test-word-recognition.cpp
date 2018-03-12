#include <iostream>
#include <string>
#include <numeric>
#include <vector>

#include "config.h"
#include "logger.h"
#include "tester.h"
#include "trainer.h"

using namespace std;

int main()
{
	Config config("B:/record/digit_0.8_2/");
	config.load("sr-lib.config");

	vector<int> saved_n_utterances(config.n_utterances);
	for (int i = 0; i < config.n_utterances.size(); ++i)
	{
		config.n_utterances[i] *= 0.70;
	}

	Trainer trainer(config);
	Tester tester(config);

	vector<int> n_hits(config.audio_names.size(), 0), n_errs(config.audio_names.size(), 0);
	for (int i = 0; i < config.audio_names.size(); ++i)
	{
		for (int j = config.n_utterances[i]; j < saved_n_utterances[i]; ++j)
		{
			string test_filename = config.audio_names[i] + "_" + to_string(j);
			int word_index = tester.test(test_filename);

			if (word_index == i)
			{
				n_hits[i]++;
			}
			if (word_index == -1)
			{
				n_errs[i]++;
			}
			Logger::log("The recognised word is:", word_index == -1 ? "###" : config.audio_names[word_index]);
		}
	}

	Logger::log("n_hits is:", accumulate(n_hits.begin(), n_hits.end(), 0));
	Logger::log("n_errs is:", accumulate(n_errs.begin(), n_errs.end(), 0));
}
