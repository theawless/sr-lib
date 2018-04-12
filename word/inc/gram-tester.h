#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "n-gram.h"

class GramTester
{
public:
	class Builder
	{
	public:
		/// Constructor.
		Builder(const std::string &folder, const Config &config);

		/// Build the GramTester.
		std::unique_ptr<GramTester> build() const;

	private:
		static constexpr char gram_ext[] = ".gram";

		const std::string folder;
		const int n_gram;
		const bool q_dfa;

		/// Load the grams. 
		std::vector<Gram> get_grams() const;
	};

	/// Get the gram score.
	std::pair<bool, double> test(const std::vector<std::string> &context, const std::string &word) const;

private:
	const int n_gram;
	const bool q_dfa;
	const MLE mle;

	/// Constructor.
	GramTester(int n_gram, bool q_dfa, MLE mle);
};
