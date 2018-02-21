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
	bool empty();

	/// Gets the base feed forward model.
	static Model bakis(int N, int M);

	/// Merges the given models.
	static Model merge(const std::vector<Model> &models);

	/// Input/Output for loading/saving.
	friend std::istream &operator>>(std::istream &input, Model &model);
	friend std::ostream &operator<<(std::ostream &output, const Model &model);
};
