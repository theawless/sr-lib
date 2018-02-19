#pragma once

#include <tuple>
#include <vector>

#include "model.h"

class HMM
{
private:
	const double convergence_threshold = 1.001;
	const double convergence_max_iterations = 50;
	const double minimum_probability = 10e-60;
	Model lambda;

	/// Tweak values of lambda.
	void tweak();

	/// Calculates P* and optimal states.
	std::pair<double, std::vector<int>> viterbi(const std::vector<int> &o);
	std::pair<double, std::vector<int>> viterbi_logged(const std::vector<int> &o);

	/// Calculates beta values.
	std::vector<std::vector<double>> backward(const std::vector<int> &o);
	std::vector<std::vector<double>> backward_scaled(const std::vector<int> &o);

	/// Improves Model by using xi and gamma values.
	void restimate(const std::vector<int> &o);

public:
	/// Constructor.
	HMM(const Model &lambda);

	/// Calculates alpha values and P(O|lambda).
	std::pair<double, std::vector<std::vector<double>>> forward(const std::vector<int> &o);
	std::pair<double, std::vector<std::vector<double>>> forward_scaled(const std::vector<int> &o);

	/// Optimises the given model using given observation sequence.
	Model optimise(const std::vector<int> &o);
};
