#pragma once

#include <tuple>
#include <vector>

#include "model.h"

class HMM
{
public:
	/// Constructor.
	HMM(const Model &lambda);

	/// Optimise the given model with given observation sequence.
	Model optimise(const std::vector<int> &o);

	/// Calculate alpha values and P(O|lambda).
	std::pair<double, std::vector<std::vector<double>>> forward(const std::vector<int> &o) const;

private:
	static constexpr double minimum_probability = 10e-60;
	static constexpr double convergence_threshold = 1.001;
	static constexpr int convergence_max_iterations = 50;

	Model lambda;

	/// Tweak values of lambda.
	void tweak();

	/// Calculate P* and optimal states.
	std::pair<double, std::vector<int>> viterbi(const std::vector<int> &o) const;

	/// Calculate beta values.
	std::vector<std::vector<double>> backward(const std::vector<int> &o) const;

	/// Improve Model by using Baum Whelch algorithm.
	void restimate(const std::vector<int> &o);
};
