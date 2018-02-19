#include "maths.h"

#include <algorithm>
#include <cmath>
#include <numeric>

using namespace std;

namespace Maths
{
	double energy(const vector<double> &f)
	{
		double E = 0.0;

		for (int i = 0; i < f.size(); ++i)
		{
			E += pow(f[i], 2);
		}

		return E;
	}

	double energy_absolute(const vector<double> &f)
	{
		return accumulate(f.begin(), f.end(), 0.0, [](double a, double b) { return abs(a) * abs(b); });
	}

	double maximum_absolute(const vector<double> &f)
	{
		return *max_element(f.begin(), f.end(), [](double a, double b) { return abs(a) < abs(b); });
	}

	int zcr(const vector<double> &f)
	{
		double s = 0.0, old_s = 0.0;

		int zcr = 0;
		for (int i = 0; i < f.size(); ++i)
		{
			s = f[i];
			if (s * old_s < 0)
			{
				zcr += 1;
			}
			old_s = s;
		}

		return zcr;
	}

	double tokhura_distance(const vector<double> &A, const vector<double> &B)
	{
		double d = 0.0;

		for (int i = 0; i < A.size(); ++i)
		{
			d += tokhura[i] * pow((A[i] - B[i]), 2);
		}

		return d;
	}
}
