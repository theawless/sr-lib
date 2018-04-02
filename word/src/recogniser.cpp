#include "recogniser.h"

#include <algorithm>
#include <limits>
#include "logger.h"

using namespace std;

Recogniser::Builder::Builder(const string &folder, const vector<string> &words, const vector<vector<string>> &sentences, const Config &config) :
	folder(folder), words(words), sentences(sentences), config(config),
	gram_weight(config.get_val<double>("gram_weight", 0.5)), cutoff_score(config.get_val<double>("cutoff_score", 0.5))
{
}

unique_ptr<Recogniser> Recogniser::Builder::build() const
{
	return unique_ptr<Recogniser>(new Recogniser(words, sentences, ModelTester::Builder(folder, config).build(), GramTester::Builder(folder, config).build(), gram_weight, cutoff_score));
}

pair<bool, string> Recogniser::recognise(const string &filename)
{
	pair<bool, string> word(false, string());

	const pair<bool, vector<double>> model_scores = model_tester->test(filename);
	if (!model_scores.first || *max_element(model_scores.second.begin(), model_scores.second.end()) == 0.0)
	{
		return word;
	}

	double best_mixed_score = numeric_limits<double>::min();
	string best_word;
	for (int i = 0; i < words.size(); ++i)
	{
		const pair<bool, double> gram_score = gram_tester->test(context, words[i]);
		if (!gram_score.first || gram_score.second == 0.0)
		{
			continue;
		}

		double mixed_score = gram_weight * gram_score.second + (1 - gram_weight) * model_scores.second[i];
		if (best_mixed_score < mixed_score)
		{
			best_mixed_score = mixed_score;
			best_word = words[i];
		}
	}
	word.first = !best_word.empty() && best_mixed_score >= cutoff_score;
	word.second = best_word;

	if (word.first)
	{
		context.push_back(word.second);
	}

	return word;
}

void Recogniser::reset()
{
	context.clear();
}

Recogniser::Recogniser(vector<string> words, vector<vector<string>> sentences, unique_ptr<ModelTester> model_tester, unique_ptr<GramTester> gram_tester, double gram_weight, double cutoff_score) :
	words(words), sentences(sentences),
	model_tester(move(model_tester)), gram_tester(move(gram_tester)), gram_weight(gram_weight), cutoff_score(cutoff_score), context()
{
}
