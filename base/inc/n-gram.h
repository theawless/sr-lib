#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Gram
{
	std::map<std::vector<std::string>, int> counts;

	/// Return whether empty.
	bool empty() const;

	/// Operators for loading and saving.
	friend std::istream &operator>>(std::istream &input, Gram &gram);
	friend std::ostream &operator<<(std::ostream &output, const Gram &gram);
};

class MLE
{
private:
	std::vector<Gram> grams;

public:
	MLE(std::vector<Gram> grams);

	/// Return the probability of word with given context.
	double score(const std::vector<std::string> &context, const std::string &word) const;
};
