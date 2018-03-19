#pragma once

#include <vector>

namespace Maths
{
	/// Find energy of a vector.
	double energy(const std::vector<double> &f);

	/// Find the magnitude maximum of a vector.
	double maximum_absolute(const std::vector<double> &f);

	/// Find zcr of a vector.
	int zcr(const std::vector<double> &f);
};
