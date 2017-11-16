#include "trim.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <tuple>

#include "maths.h"
#include "utils.h"

#define SAMPLES_PER_FRAME 320
#define TRIM_ENERGY_WINDOW 5
#define TRIM_ENERGY_THRESHOLD 0.001

using namespace std;

vector<double> trim_by_energy_fixed(const vector<double> &amplitudes) {
	vector<vector<double>> amplitudes_chunks = get_vector_chunks(amplitudes, SAMPLES_PER_FRAME);
	vector<double> energies;
	for (int i = 0; i < amplitudes_chunks.size(); ++i) {
		energies.push_back(energy(amplitudes_chunks[i]));
	}

	// Select frames around the frame with the max energy.
	int max_i = max_element(energies.begin(), energies.end()) - energies.begin();
	int left = (max_i - TRIM_ENERGY_WINDOW >= 0) ? max_i - TRIM_ENERGY_WINDOW : 0;
	int right = (max_i + TRIM_ENERGY_WINDOW < energies.size()) ? max_i + TRIM_ENERGY_WINDOW : energies.size();

	vector<double> trimmed_amplitudes;
	for (int i = left; i < right; i++) {
		trimmed_amplitudes.insert(trimmed_amplitudes.end(), amplitudes_chunks[i].begin(), amplitudes_chunks[i].end());
	}

	return trimmed_amplitudes;
}

vector<double> trim_by_energy_threshold(const vector<double> &amplitudes) {
	vector<vector<double>> amplitudes_chunks = get_vector_chunks(amplitudes, SAMPLES_PER_FRAME);
	vector<double> energies;
	for (int i = 0; i < amplitudes_chunks.size(); ++i) {
		energies.push_back(energy(amplitudes_chunks[i]));
	}

	vector<double> trimmed_amplitudes;
	double max_energy = *max_element(energies.begin(), energies.end());
	double energy_threshold = max_energy * TRIM_ENERGY_THRESHOLD;
	int left = 0, right = energies.size() - 1;

	// Remove chunks from both ends with energy less than threshold energy.
	while (energies[left] < energy_threshold) left++;
	while (energies[right] < energy_threshold) right--;
	for (int i = left; i <= right; i++) {
		trimmed_amplitudes.insert(trimmed_amplitudes.end(), amplitudes_chunks[i].begin(), amplitudes_chunks[i].end());
	}

	return trimmed_amplitudes;
}

static pair<double, pair<double, double>> get_noise_stats(const vector<double> &amplitudes) {
	// Find stats for background noise with assumption that first 100ms is noise.
	vector<double> noise_amplitudes(amplitudes.begin(), amplitudes.begin() + 30 * 100);
	vector<vector<double>> chunked_noise_amplitudes = get_vector_chunks(noise_amplitudes, 100);

	double A_IZC = 0.0;
	for (int i = 0; i < chunked_noise_amplitudes.size(); ++i) {
		vector<double> frame(chunked_noise_amplitudes[i].begin(), chunked_noise_amplitudes[i].end());
		A_IZC += zcr(frame);
	}
	A_IZC /= chunked_noise_amplitudes.size();

	double SD_IZC = 0.0;
	for (int i = 0; i < chunked_noise_amplitudes.size(); ++i) {
		vector<double> frame(chunked_noise_amplitudes[i].begin(), chunked_noise_amplitudes[i].end());
		SD_IZC += pow(zcr(frame) - A_IZC, 2);
	}
	SD_IZC = sqrt(SD_IZC / chunked_noise_amplitudes.size());

	double IZCT = A_IZC + 2 * SD_IZC;

	double IMX = numeric_limits<double>::min(), IMN = numeric_limits<double>::max();
	for (int i = 0; i < chunked_noise_amplitudes.size(); ++i) {
		vector<double> frame(chunked_noise_amplitudes[i].begin(), chunked_noise_amplitudes[i].end());
		double energy = magnitude_energy(frame);
		IMX = max(IMX, energy);
		IMN = min(IMN, energy);
	}

	double ITL = IMX / 2;
	double ITU = 4.0 * ITL;

	return pair<double, pair<double, double>>(IZCT, pair<double, double>(ITL, ITU));
}

pair<vector<double>, pair<vector<double>, vector<double>>> trim_by_auto(const vector<double> &amplitudes) {
	pair<double, pair<double, double>> noise_stats = get_noise_stats(amplitudes);
	double ITU = noise_stats.second.second, ITL = noise_stats.second.first, IZCT = noise_stats.first;
	vector<vector<double>> chunked_amplitudes = get_vector_chunks(amplitudes, 100);

	int N1 = -1;
	for (int i = 0; i < chunked_amplitudes.size(); ++i) {
		vector<double> frame(chunked_amplitudes[i].begin(), chunked_amplitudes[i].end());
		double energy = magnitude_energy(frame);
		if (energy > ITU) {
			if (N1 == -1) {
				N1 = i;
			}
			break;
		}
		else if (energy > ITL) {
			N1 = i;
		}
	}

	int N2 = -1;
	for (int i = chunked_amplitudes.size() - 1; i >= 0; --i) {
		vector<double> frame(chunked_amplitudes[i].begin(), chunked_amplitudes[i].end());
		double energy = magnitude_energy(frame);
		if (energy > ITU) {
			if (N2 == -1) {
				N2 = i;
			}
			break;
		}
		else if (energy > ITL) {
			N2 = i;
		}
	}

	int F1 = N1;
	for (int i = N1, j = 0; i >= N1 - 25 && i >= 0; --i) {
		vector<double> frame(chunked_amplitudes[i].begin(), chunked_amplitudes[i].end());
		if (zcr(frame) > IZCT) {
			j++;
			if (j >= 3) {
				F1 = i;
			}
		}
	}

	int F2 = N2;
	for (int i = N2, j = 0; i <= N2 + 25 && i < chunked_amplitudes.size(); ++i) {
		vector<double> frame(chunked_amplitudes[i].begin(), chunked_amplitudes[i].end());
		if (zcr(frame) > IZCT) {
			j++;
			if (j >= 3) {
				F2 = i;
			}
		}
	}

	vector<double> trimmed_amplitudes;
	for (int i = F1; i <= F2; ++i) {
		trimmed_amplitudes.insert(trimmed_amplitudes.end(), chunked_amplitudes[i].begin(), chunked_amplitudes[i].end());
	}
	vector<double> start_noise;
	for (int i = 0; i < F1; ++i) {
		start_noise.insert(start_noise.end(), chunked_amplitudes[i].begin(), chunked_amplitudes[i].end());
	}
	vector<double> end_noise;
	for (int i = F2; i < chunked_amplitudes.size(); ++i) {
		end_noise.insert(end_noise.end(), chunked_amplitudes[i].begin(), chunked_amplitudes[i].end());
	}

	return pair<vector<double>, pair<vector<double>, vector<double>>>(trimmed_amplitudes, pair<vector<double>, vector<double>>(start_noise, end_noise));
}
