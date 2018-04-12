#pragma once

#include <complex>
#include <map>
#include <vector>

#include "feature.h"

/// Thanks to https://github.com/dspavankumar/compute-mfcc
class MFC : public ICepstral
{
public:
	/// Constructor.
	MFC(int n_cepstra, bool q_gain, bool q_delta, bool q_accel);

private:
	static constexpr int n_fft = 512;
	static constexpr double hz_low = 50;
	static constexpr double hz_high = 6500;
	static constexpr double hz_sampling = 16000;
	static constexpr int n_filters = 40;
	static constexpr int n_fft_bins = 247;

	const std::vector<std::vector<double>> filter_bank;
	const std::vector<std::vector<double>> dct_matrix;

	/// Compute filterbank.
	static std::vector<std::vector<double>> setup_filter_bank();

	/// Compute dct matrix.
	static std::vector<std::vector<double>> setup_dct_matrix(int n_cepstra);

	/// Find mfcc features for given frame.
	Feature feature(const std::vector<double> &frame) const;

	/// Find power spectrum.
	std::vector<double> power_spectrum(const std::vector<double> &frame) const;

	/// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency).
	/// Thanks to https://rosettacode.org/wiki/Fast_Fourier_transform
	void fft(std::vector<std::complex<double>> &x) const;

	/// Apply log Mel filterbank.
	std::vector<double> lmfb(const std::vector<double> &P) const;

	/// Compute discrete cosine transform.
	std::vector<double> dct(const std::vector<double> &H) const;

	/// Balance the coefficients by subtracting mean.
	void normalise(std::vector<double> &C) const;
};
