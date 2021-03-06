#include "gram-trainer.h"

#include <algorithm>
#include <functional>
#include <future>
#include <thread>
#include <utility>

#include "file-io.h"
#include "logger.h"

using namespace std;

GramTrainer::Builder::Builder(const string &model_folder, const vector<vector<string>> &sentences, const Config &config) :
	model_folder(model_folder), sentences(sentences),
	q_cache(config.get_val<bool>("q_cache", true)), n_thread(config.get_val<int>("n_thread", 4 * thread::hardware_concurrency())),
	n_gram(config.get_val<int>("n_gram", get_n_gram())), q_dfa(config.get_val<bool>("q_dfa", true))
{
}

unique_ptr<GramTrainer> GramTrainer::Builder::build() const
{
	return unique_ptr<GramTrainer>(new GramTrainer(model_folder, sentences, q_cache, unique_ptr<ThreadPool>(new ThreadPool(n_thread)), n_gram, q_dfa));
}

int GramTrainer::Builder::get_n_gram() const
{
	const function<bool(const vector<string> &, const vector<string> &)> compare_size = [](const vector<string> &a, const vector<string> &b)
	{
		return a.size() < b.size();
	};

	// because q_dfa is true, n_gram default will be max length possible
	return max_element(sentences.begin(), sentences.end(), compare_size)->size();
}

void GramTrainer::train() const
{
	vector<future<Gram>> gram_futures;
	for (int i = 0; i <= n_gram; ++i)
	{
		gram_futures.push_back(thread_pool->enqueue(&GramTrainer::get_gram, this, i));
	}
	for (int i = 0; i <= n_gram; ++i)
	{
		gram_futures[i].get();
	}
}

GramTrainer::GramTrainer(string model_folder, vector<vector<string>> sentences, bool q_cache, unique_ptr<ThreadPool> thread_pool, int n_gram, bool q_dfa) :
	model_folder(model_folder), sentences(sentences),
	thread_pool(move(thread_pool)), q_cache(q_cache),
	n_gram(n_gram), q_dfa(q_dfa)
{
	train();
}

Gram GramTrainer::get_gram(int n) const
{
	Logger::log("Getting gram:", n);
	Gram gram;
	const string gram_filename = model_folder + to_string(n) + gram_ext;

	if (q_cache)
	{
		gram = FileIO::get_item_from_file<Gram>(gram_filename);
		if (!gram.empty())
		{
			return gram;
		}
	}

	for (int i = 0; i < sentences.size(); ++i)
	{
		for (int j = 0; j <= (int)sentences[i].size() - n; ++j)
		{
			const vector<string> part(sentences[i].begin() + j, sentences[i].begin() + j + n);
			if (gram.counts.find(part) == gram.counts.end())
			{
				gram.counts[part] = 0;
			}
			gram.counts[part] += 1;

			if (q_dfa)
			{
				// ensure only sequential sentences
				break;
			}
		}
	}
	FileIO::set_item_to_file<Gram>(gram, gram_filename);

	return gram;
}
