#pragma once

#include <vector>

namespace std {
	typedef struct {
		std::vector<std::vector<double>> a;
		std::vector<std::vector<double>> b;
		std::vector<double> pi;
	} Model;
}

/// Calculates alpha values and P(O|lambda).
std::pair<double, std::vector<std::vector<double>>> forward(const std::Model &lambda, const std::vector<int> &o);

/// Gets the base feed forward model.
std::Model bakis(int N, int M);

/// Merges the given models.
std::Model merge(const std::vector<std::Model>& lambdas);

/// Optimises the given model using given observation sequence.
std::Model optimise(const std::Model &lambda, const std::vector<int> &o);
