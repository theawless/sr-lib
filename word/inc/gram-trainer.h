#pragma once

#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "n-gram.h"
#include "threads.h"

class GramTrainer
{
public:
	class Builder
	{
	public:
		/// Constructor.
		Builder(const std::string &model_folder, const std::vector<std::vector<std::string>> &sentences, const Config &config);

		/// Build the GramTrainer.
		std::unique_ptr<GramTrainer> build() const;

	private:
		const std::string model_folder;
		const std::vector<std::vector<std::string>> sentences;
		const bool q_cache;
		const int n_thread;
		const int n_gram;
		const bool q_dfa;

		/// Get the default n.
		int get_n_gram() const;
	};

	/// Train.
	void train() const;

private:
	static constexpr char const *gram_ext = ".gram";

	const std::string model_folder;
	const std::vector<std::vector<std::string>> sentences;
	const bool q_cache;
	const std::unique_ptr<ThreadPool> thread_pool;
	const int n_gram;
	const bool q_dfa;

	/// Constructor.
	GramTrainer(std::string model_folder, std::vector<std::vector<std::string>> sentences, bool q_cache, std::unique_ptr<ThreadPool> thread_pool, int n_gram, bool q_dfa);

	/// Get the gram for given n.
	Gram get_gram(int n) const;
};
