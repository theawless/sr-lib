#include "gram-tester.h"

#include <limits>

#include "file-io.h"
#include "logger.h"

using namespace std;

GramTester::Builder::Builder(const string &model_folder, const Config &config) :
	model_folder(model_folder),
	n_gram(config.get_val<int>("n_gram", numeric_limits<int>::max())), q_dfa(config.get_val<bool>("q_dfa", true))
{
}

unique_ptr<GramTester> GramTester::Builder::build() const
{
	vector<Gram> grams = get_grams();
	return unique_ptr<GramTester>(new GramTester(grams.size() - 1, q_dfa, MLE(grams)));
}

vector<Gram> GramTester::Builder::get_grams() const
{
	vector<Gram> grams;

	for (int i = 0; i <= n_gram; ++i)
	{
		Logger::log("Loading gram:", i);
		const string gram_filename = model_folder + to_string(i) + gram_ext;
		const Gram gram = FileIO::get_item_from_file<Gram>(gram_filename);
		if (gram.empty())
		{
			// case when n_gram was set to maximum because q_dfa was true
			break;
		}

		grams.push_back(gram);
	};

	return grams;
}

pair<bool, double> GramTester::test(const vector<string> &full_context, const string &word) const
{
	pair<bool, double> score(false, 0.0);

	vector<string> context;
	if (full_context.size() < n_gram)
	{
		context = vector<string>(full_context.begin(), full_context.end());
	}
	else
	{
		if (q_dfa)
		{
			// illegal case
			return score;
		}
		else
		{
			// use the most recent words
			context = vector<string>(full_context.end() - n_gram + 1, full_context.end());
		}
	}

	score.first = true;
	score.second = mle.score(context, word);

	return score;
}

GramTester::GramTester(int n_gram, bool q_dfa, MLE mle) :
	n_gram(n_gram), q_dfa(q_dfa), mle(mle)
{
}
