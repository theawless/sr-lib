#include "lpcc.h"

#include <algorithm>
#include <numeric>

#include "maths.h"

#define P_VALUE 12

using namespace std;

static vector<double> frame_to_coefficients(const vector<double>& frame) {
	vector<double> hamming_frame = hamming_window(frame);
	vector<double> R = autocorrelation(hamming_frame, 0, P_VALUE);
	vector<double> A = durbin_solve(R);
	vector<double> c = cepstral_coefficients(A);
	vector<double> C = sine_window(c);

	return C;
}

vector<vector<double>> speech_to_coefficients(const vector<vector<double>> &segments) {
	vector<vector<double>> coefficients;

	for (int i = 0; i < segments.size(); ++i) {
		vector<double> C = frame_to_coefficients(segments[i]);
		coefficients.push_back(C);
	}

	return coefficients;
}

double coefficients_similarity(const vector<vector<double>> &A, const vector<vector<double>> &B) {
	double sum_distance = 0;
	double n_distances = min(A.size(), B.size());

	for (int i = 0; i < n_distances; ++i) {
		sum_distance += tokhura_distance(A[i], B[i]);
	}

	return sum_distance / n_distances;
}
