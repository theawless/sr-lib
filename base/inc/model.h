#pragma once

#include <vector>
#include <iostream>

struct Model
{
public:
	std::vector<std::vector<double>> a;
	std::vector<std::vector<double>> b;
	std::vector<double> pi;

	/// Returns whether the model is empty.
	bool empty() const;

	/// Gets the base feed forward model.
	static Model bakis(int N, int M, int step);

	/// Merges the given models.
	static Model merge(const std::vector<Model> &models);

	/// Operators for loading and saving.
	friend std::istream &operator>>(std::istream &input, Model &model);
	friend std::ostream &operator<<(std::ostream &output, const Model &model);
};
