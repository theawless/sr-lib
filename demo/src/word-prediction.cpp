#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "logger.h"
#include "gram-tester.h"
#include "gram-trainer.h"
#include "utils.h"

using namespace std;

static vector<string> get_words(const vector<vector<string>> &sentences)
{
	vector<string> words;

	for (int i = 0; i < sentences.size(); ++i)
	{
		for (int j = 0; j < sentences[i].size(); ++j)
		{
			if (find(words.begin(), words.end(), sentences[i][j]) == words.end())
			{
				words.push_back(sentences[i][j]);
			}
		}
	}

	return words;
}

int main()
{
	const string folder = "res/";

	const string config_filename = folder + "sr-lib.config";
	const string sentences_filename = folder + "sr-lib.sentences";
	const Config config = Utils::get_item_from_file<Config>(config_filename);
	const vector<vector<string>> sentences = Utils::get_matrix_from_file<string>(sentences_filename, ' ');

	Logger::info("Training...");
	chrono::steady_clock::time_point start = chrono::steady_clock::now();
	const string train_folder = folder + "train/";
	const unique_ptr<GramTrainer> gram_trainer = GramTrainer::Builder(train_folder, sentences, config).build();
	const unique_ptr<GramTester> gram_tester = GramTester::Builder(train_folder, config).build();
	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	chrono::seconds time = chrono::duration_cast<chrono::seconds>(end - start);
	Logger::info("Time taken:", time.count());

	Logger::info("Testing...");
	const vector<string> words = get_words(sentences);
	string word;
	vector<string> context;
	do
	{
		Logger::info("Context is:", Utils::get_string_from_vector<string>(context, ' '));
		for (int i = 0; i < words.size(); ++i)
		{
			pair<bool, double> score = gram_tester->test(context, words[i]);
			Logger::info("Word is:", words[i], "Score is:", score.second);
		}
	} while (Logger::info("Enter a word:"), cin >> word, context.push_back(word), true);
}
