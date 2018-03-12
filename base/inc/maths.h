#pragma once

#include <vector>

namespace Maths
{
	static const double tokhura[12] = { 1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0 };

	/// Find energy of a vector.
	double energy(const std::vector<double> &f);

	/// Find the magnitude energy of a vector.
	double energy_absolute(const std::vector<double> &f);

	/// Find the magnitude maximum of a vector.
	double maximum_absolute(const std::vector<double> &f);

	/// Find zcr of a vector.
	int zcr(const std::vector<double> &f);

	/// Find Tokhura's distance betweent two vectors.
	double tokhura_distance(const std::vector<double> &A, const std::vector<double> &B);
};
