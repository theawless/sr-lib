#pragma once

#include <iostream>
#include <vector>

struct Model
{
public:
	class Builder
	{
	public:
		/// Constuctor.
		Builder(int N, int M, int step);

		/// Get a feed forward model.
		Model bakis() const;

		/// Merge the given models.
		Model merge(const std::vector<Model> &models) const;

	private:
		int N;
		int M;
		int step;
	};

	std::vector<std::vector<double>> a;
	std::vector<std::vector<double>> b;
	std::vector<double> pi;

	/// Return whether empty.
	bool empty() const;

	/// Operators for loading and saving.
	friend std::istream &operator>>(std::istream &input, Model &model);
	friend std::ostream &operator<<(std::ostream &output, const Model &model);
};
