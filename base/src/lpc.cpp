#include "lpc.h"

#include <cmath>
#include <numeric>

using namespace std;

vector<double> LPC::setup_sine_coefficients(int n_cepstra)
{
	vector<double> sine_coefficients(n_cepstra + 1, 1.0);

	const double pi = 4.0 * atan(1.0);
	for (int i = 0; i < n_cepstra + 1; ++i)
	{
		sine_coefficients[i] += n_cepstra / 2.0 * sin(i * pi / n_cepstra);
	}

	return sine_coefficients;
}

Feature LPC::feature(const vector<double> &frame) const
{
	const vector<double> R = auto_correlation(frame);
	const vector<double> A = durbin_solve(R);
	const double G_squared = gain(R, A);
	vector<double> C = cepstral_coefficients(G_squared, A);
	sine_window(C);

	return Feature{ C };
}

vector<double> LPC::auto_correlation(const vector<double> &frame) const
{
	vector<double> R(n_predict + 1, 0.0);

	for (int i = 0; i < n_predict + 1; ++i)
	{
		for (int j = 0; j < frame.size() - i; ++j)
		{
			R[i] += frame[j] * frame[j + i];
		}
	}

	return R;
}

vector<double> LPC::durbin_solve(const vector<double> &R) const
{
	vector<vector<double>> a(n_predict + 1, vector<double>(n_predict + 1, 0.0));
	vector<double> E(n_predict + 1, 0.0);

	E[0] = R[0];
	a[1][1] = R[1] / R[0];
	E[1] = (1.0 - a[1][1] * a[1][1]) * E[0];
	for (int i = 2; i < n_predict + 1; ++i)
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

	return a[n_predict];
}

double LPC::gain(const vector<double> &R, const vector<double> &A) const
{
	double G_squared = R[0];

	for (int i = 0; i < n_predict + 1; ++i)
	{
		G_squared -= A[i] * R[i];
	}

	return G_squared;
}

vector<double> LPC::cepstral_coefficients(double G_squared, const vector<double> &A) const
{
	vector<double> C(n_cepstra + 1, 0.0);

	C[0] = log(G_squared);
	for (int i = 1; i < n_predict + 1; ++i)
	{
		for (int j = 1; j < i; ++j)
		{
			C[i] += A[i] + (j * C[j] * A[i - j] / i);
		}
	}
	for (int i = n_predict + 1; i < n_cepstra + 1; ++i)
	{
		for (int j = i - n_predict; j < i; ++j)
		{
			C[i] += j * C[j] * A[i - j] / i;
		}
	}

	return C;
}

void LPC::sine_window(vector<double> &C) const
{
	for (int i = 0; i < n_cepstra + 1; ++i)
	{
		C[i] *= sine_coefficients[i];
	}
}

LPC::LPC(int n_cepstra, bool q_gain, bool q_delta, bool q_accel, int n_predict) : ICepstral(n_cepstra, q_gain, q_delta, q_accel),
n_predict(n_predict), sine_coefficients(setup_sine_coefficients(n_cepstra))
{
}
