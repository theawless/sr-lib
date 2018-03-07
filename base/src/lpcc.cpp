#include "lpcc.h"

#include <cmath>
#include <numeric>

using namespace std;

void LPCC::setup_sine_window()
{
	const double pi = 4.0 * atan(1.0);
	sine_coefficients = vector<double>(n_cepstra, 1);

	for (int i = 0; i < n_cepstra; ++i)
	{
		sine_coefficients[i] += (n_cepstra / 2.0) * sin((pi * (i + 1)) / n_cepstra);
	}
}

vector<double> LPCC::auto_correlation(const vector<double> &frame, int mi, int mf)
{
	vector<double> R(mf + 1 - mi, 0.0);

	for (int i = mi; i < mf + 1; ++i)
	{
		for (int j = 0; j < frame.size() - i; ++j)
		{
			R[i - mi] += frame[j] * frame[j + i];
		}
	}

	return R;
}

vector<double> LPCC::durbin_solve(const vector<double> R)
{
	int p = R.size() - 1;
	vector<double> E(p + 1, 0.0);
	vector<vector<double>> a(p + 1, vector<double>(p + 1, 0.0));

	// First iteration.
	E[0] = R[0];
	a[1][1] = R[1] / R[0];
	E[1] = (1 - a[1][1] * a[1][1]) * E[0];

	for (int i = 2; i < p + 1; ++i)
	{
		a[i][i] = R[i];
		for (int j = 1; j < i; ++j)
		{
			a[i][i] -= a[i - 1][j] * R[i - j];
		}

		if (E[i - 1] != 0)
		{
			a[i][i] /= E[i - 1];
		}

		for (int j = 1; j < i; ++j)
		{
			a[i][j] = a[i - 1][j] - a[i][i] * a[i - 1][i - j];
		}
		E[i] = (1.0 - a[i][i] * a[i][i]) * E[i - 1];
	}

	return vector<double>(a[p].begin() + 1, a[p].end());
}

vector<double> LPCC::cepstral_coefficients(const vector<double> &A)
{
	vector<double> C(A);

	for (int i = 1; i < A.size(); ++i)
	{
		for (int j = 0; j < i; ++j)
		{
			C[i] += ((j + 1) * C[j] * A[i - j - 1]) / (i + 1);
		}
	}

	return C;
}

void LPCC::sine_window(vector<double> &C)
{
	for (int i = 0; i < C.size(); ++i)
	{
		C[i] *= sine_coefficients[i];
	}
}

LPCC::LPCC()
{
	setup_sine_window();
}

vector<double> LPCC::coeffs(const vector<double> &frame)
{
	vector<double> R = auto_correlation(frame, 0, n_cepstra);
	vector<double> A = durbin_solve(R);
	vector<double> C = cepstral_coefficients(A);
	sine_window(C);

	return C;
}
