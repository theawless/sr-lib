#include "phoneme.h"

#include <algorithm>
#include <numeric>

#include "maths.h"
#include "utils.h"

using namespace std;

/// Calculates autocorrelation for clipped values.
static vector<double> autocorrelation_normalised(const vector<double> &f, int mi, int mf) {
	vector<double> AC(mf + 1 - mi, 0.0);

	for (int i = mi; i < mf + 1; ++i) {
		for (int j = 0; j < f.size() - i; ++j) {
			int val;
			if (f[j] == 0 || f[j + i] == 0) {
				val = 0;
			}
			else if (f[j] == f[j + i]) {
				val = 1;
			}
			else {
				val = -1;
			}
			AC[i - mi] += val;
		}
	}

	return AC;
}


/// Calculates the clip threshold.
static double clip_threshold(const vector<double> &frame) {
	double left_max = *max_element(frame.begin(), frame.begin() + frame.size() / 3);
	double right_max = *max_element(frame.begin() + frame.size() * 2 / 3, frame.end());

	return 0.68 * min(left_max, right_max);
}

/// Clips the signal.
static vector<double> clip(const vector<double> &amplitudes, double threshold) {
	vector<double> clipped_amplitudes(amplitudes.size(), 0.0);

	for (int i = 0; i < amplitudes.size(); ++i) {
		if (amplitudes[i] > threshold) {
			clipped_amplitudes[i] = 1;
		}
		else if (amplitudes[i] < threshold) {
			clipped_amplitudes[i] = -1;
		}
		else {
			clipped_amplitudes[i] = 0;
		}
	}

	return clipped_amplitudes;
}

static double get_background_stats(const vector<double> &noise) {
	int n_frames = 0;
	double E = 0;

	for (int i = 0; i <= noise.size() - 320; i += 80) {
		vector<double> frame(noise.begin() + i, noise.begin() + i + 320);
		E += magnitude_energy(frame);
		n_frames++;
	}

	return E / n_frames * 0.3;
}

static bool is_voiced(const vector<double> &frame) {
	double threshold = clip_threshold(frame);
	vector<double> clipped_thresholds = clip(frame, threshold);
	double R0 = autocorrelation(clipped_thresholds, 0, 0)[0];
	vector<double> R = autocorrelation_normalised(clipped_thresholds, 20, 200);
	vector<double> linear_R = linear_window(R);

	double R_threshold = 0.3 * R0;
	double peak_R = *max_element(linear_R.begin(), linear_R.end());

	return peak_R > R_threshold;
}

static bool is_silence(const vector<double> frame, double noise_threshold) {
	double E = magnitude_energy(frame);
	return E < noise_threshold;
}

vector<Phoneme> tag_phonemes(const vector<vector<double>> &segments, const vector<double> &noise) {
	double noise_threshold = get_background_stats(noise);
	vector<Phoneme> phonemes;

	for (int i = 0; i < segments.size(); ++i) {
		Phoneme phoneme;
		if (is_silence(segments[i], noise_threshold)) {
			phoneme = Silence;
		}
		else {
			phoneme = is_voiced(segments[i]) ? Voiced : Unvoiced;
		}
		phonemes.push_back(phoneme);
	}

	return phonemes;
}
