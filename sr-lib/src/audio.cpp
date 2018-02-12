#include "audio.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "utils.h"

#define SAMPLES_PER_FRAME 320
#define SAMPLES_WINDOW_OVERLAP 80
#define NORMALISATION_AMPLITUDE 5000

using namespace std;

/// Fix DC offset of the signal.
static vector<double> fix_dc_offset(const vector<double> &amplitudes) {
	vector<double> vec(amplitudes.size());

	double sum_amplitude = accumulate(amplitudes.begin(), amplitudes.end(), 0.0);
	double mean_amplitude = sum_amplitude / amplitudes.size();
	for (int i = 0; i < amplitudes.size(); ++i) {
		vec[i] = amplitudes[i] - mean_amplitude;
	}

	return vec;
}

/// Premphasize - boost the higher frequencies.
static vector<double> preemphasize(const vector<double> &amplitudes) {
	const double alpha = 0.95;
	vector<double> vec(amplitudes);

	for (int i = 1; i < vec.size(); ++i) {
		vec[i] -= alpha * amplitudes[i - 1];
	}

	return vec;
}

/// Normalise amplitudes of the signal.
static vector<double> normalise(const vector<double> &amplitudes) {
	vector<double> vec(amplitudes.size());

	double max_amplitude = *max_element(amplitudes.begin(), amplitudes.end(), [](double a, double b) {	return abs(a) < abs(b);	});
	for (int i = 0; i < amplitudes.size(); ++i) {
		vec[i] = amplitudes[i] * NORMALISATION_AMPLITUDE / max_amplitude;
	}

	return vec;
}

vector<double> preprocess(const vector<double> &amplitudes) {
	vector<double> dc_fixed_amplitudes = fix_dc_offset(amplitudes);
	vector<double> normalised_amplitudes = normalise(dc_fixed_amplitudes);
	vector<double> preemphasize_amplitudes = preemphasize(normalised_amplitudes);

	return preemphasize_amplitudes;
}

vector<vector<double>> fixed_segment(const vector<double> &amplitudes) {
	vector<vector<double>> segments;

	for (int i = 0; i <= amplitudes.size() - SAMPLES_PER_FRAME; i += SAMPLES_WINDOW_OVERLAP) {
		vector<double>::const_iterator left = amplitudes.begin() + i;
		vector<double>::const_iterator right = left + SAMPLES_PER_FRAME;
		vector<double> frame(left, right);
		segments.push_back(frame);
	}

	return segments;
}
