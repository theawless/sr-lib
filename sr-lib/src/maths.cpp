#include "maths.h"

#include <cmath>
#include <numeric>

using namespace std;

vector<double> autocorrelation(const vector<double> &f, int mi, int mf) {
	vector<double> AC(mf + 1 - mi, 0.0);

	for (int i = mi; i < mf + 1; ++i) {
		for (int j = 0; j < f.size() - i; ++j) {
			AC[i - mi] += f[j] * f[j + i];
		}
	}

	return AC;
}

double energy(const vector<double> &f) {
	return autocorrelation(f, 0, 0)[0];
}

double magnitude_energy(const vector<double> &f) {
	return accumulate(f.begin(), f.end(), 0.0, [](double a, double b) { return abs(a) + abs(b); });
}

int zcr(const vector<double> &f) {
	double s = 0.0, old_s = 0.0;

	int zcr = 0.0;
	for (int i = 0; i < f.size(); ++i) {
		s = f[i];
		if (s * old_s < 0) {
			zcr += 1;
		}
		old_s = s;
	}

	return zcr;
}

vector<double> linear_window(const vector<double> &f) {
	int N = f.size();
	vector<double> lf(f);

	for (int i = 0; i < N; ++i) {
		lf[i] *= (double)(N - i) / N;
	}

	return lf;
}

vector<double> hamming_window(const vector<double> &f) {
	const double PI = atan(1.0) * 4.0;
	int N = f.size();

	vector<double> w(N);
	for (int i = 0; i < N; ++i) {
		w[i] = 0.54 - (0.46 * cos((2.0 * PI * i) / (N - 1)));
	}
	vector<double> hf(N);
	for (int i = 0; i < N; ++i) {
		hf[i] = w[i] * f[i];
	}

	return hf;
}

vector<double> sine_window(const vector<double> &c) {
	const double PI = atan(1.0) * 4.0;
	int Q = c.size();

	vector<double> w(Q, 1);
	for (int i = 0; i < Q; ++i) {
		w[i] += (Q / 2.0) * sin((PI * (i + 1)) / Q);
	}
	vector<double> C(c);
	for (int i = 0; i < Q; ++i) {
		C[i] *= w[i];
	}

	return C;
}

vector<double> durbin_solve(const vector<double> R) {
	int p = R.size() - 1;
	vector<double> E(p + 1, 0.0);
	vector<vector<double>> a(p + 1, vector<double>(p + 1, 0.0));

	// First iteration.
	E[0] = R[0];
	a[1][1] = R[1] / R[0];
	E[1] = (1 - a[1][1] * a[1][1]) * E[0];

	for (int i = 2; i < p + 1; ++i) {
		a[i][i] = R[i];
		for (int j = 1; j < i; ++j) {
			a[i][i] -= a[i - 1][j] * R[i - j];
		}

		if (E[i - 1] != 0) {
			a[i][i] /= E[i - 1];
		}

		for (int j = 1; j < i; ++j) {
			a[i][j] = a[i - 1][j] - a[i][i] * a[i - 1][i - j];
		}
		E[i] = (1.0 - a[i][i] * a[i][i]) * E[i - 1];
	}

	return vector<double>(a[p].begin() + 1, a[p].end());
}

vector<double> cepstral_coefficients(const vector<double> &A) {
	vector<double> C(A);

	for (int i = 1; i < A.size(); ++i) {
		for (int j = 0; j < i; ++j) {
			C[i] += ((j + 1) * C[j] * A[i - j - 1]) / (i + 1);
		}
	}

	return C;
}

double tokhura_distance(const vector<double> &A, const vector<double> &B) {
	double d = 0.0;

	double w[12] = { 1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0 };
	for (int i = 0; i < A.size(); ++i) {
		d += w[i] * (A[i] - B[i]) * (A[i] - B[i]);
	}

	return d;
}
