#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "gram-tester.h"
#include "model-tester.h"

class Recogniser
{
public:
	class Builder
	{
	public:
		/// Constructor.
		Builder(const std::string &model_folder, const std::vector<std::string> &words, const std::vector<std::vector<std::string>> &sentences, const Config &config);

		/// Build the Recogniser.
		std::unique_ptr<Recogniser> build() const;

	private:
		const std::string model_folder;
		const std::vector<std::string> words;
		const std::vector<std::vector<std::string>> sentences;
		const Config config;
		const double gram_weight;
		const double cutoff_score;
	};

	/// Recognise the word with previous context.
	std::pair<bool, std::string> recognise(const std::string &filename);

	/// Clear the context.
	void reset();

private:
	const std::vector<std::string> words;
	const std::vector<std::vector<std::string>> sentences;
	const std::unique_ptr<ModelTester> model_tester;
	const std::unique_ptr<GramTester> gram_tester;
	const double gram_weight;
	const double cutoff_score;
	std::vector<std::string> context;

	/// Constructor.
	Recogniser(std::vector<std::string> words, std::vector<std::vector<std::string>> sentences, std::unique_ptr<ModelTester> model_tester, std::unique_ptr<GramTester> gram_tester, double gram_weight, double cutoff_score);
};
