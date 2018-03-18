#pragma once

#include <cmath>
#include <complex>
#include <map>
#include <vector>

#include "features.h"

/// Thanks to https://github.com/dspavankumar/compute-mfcc
class MFC : public ICepstral
{
private:
	static constexpr int n_fft = 512;
	static constexpr double hz_low = 50;
	static constexpr double hz_high = 6500;
	static constexpr int n_filters = 40;
	static constexpr int n_fft_bins = 247;

	const int hz_sampling;
	const std::map<int, std::map<int, std::complex<double>>> twiddle;
	const std::vector<std::vector<double>> filter_bank;
	const std::vector<std::vector<double>> dct_matrix;

	/// Compute twiddle factor.
	static std::map<int, std::map<int, std::complex<double>>> setup_twiddle();

	/// Compute filterbank.
	static std::vector<std::vector<double>> setup_filter_bank(double hz_sampling);

	/// Hertz to mel.
	static double hertz2mel(double f);

	/// Mel to hertz.
	static double mel2hertz(double m);

	/// Compute dct matrix.
	static std::vector<std::vector<double>> setup_dct_matrix(int n_cepstra);

	/// Find mfcc for given frame.
	std::vector<double> coefficients(const std::vector<double> &frame) const;

	/// Find power spectrum.
	std::vector<double> power_spectrum(const std::vector<double> &frame) const;

	/// Cooley-Tukey DIT-FFT recursive function.
	std::vector<std::complex<double>> fft(const std::vector<std::complex<double>> &x) const;

	/// Apply log Mel filterbank.
	std::vector<double> lmfb(const std::vector<double> &P) const;

	/// Computing discrete cosine transform.
	std::vector<double> dct(const std::vector<double> &H) const;

	/// Balance the coefficients by subtracting mean.
	void normalise(std::vector<double> &C) const;

public:
	/// Constructor.
	MFC(int n_cepstra, bool q_gain, bool q_delta, bool q_accel, int hz_sampling);
};
