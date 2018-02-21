#pragma once

#include <complex>
#include <map>
#include <vector>

/// Thanks to https://github.com/dspavankumar/compute-mfcc
class MFCC
{
private:
	const double PI = 4 * atan(1.0);
	const int hz_sampling = 16000;
	const int n_cepstra = 12;
	const int n_fft = 512;
	const int n_fft_bins = 247;
	const int n_filters = 40;
	const double hz_low = 50;
	const double hz_high = 6500;

	std::map<int, std::map<int, std::complex<double>>> twiddle;
	std::vector<std::vector<double>> filter_bank;
	std::vector<std::vector<double>> dct_matrix;

	/// Hertz to mel.
	double hertz2mel(double f);

	/// Mel to hertz.
	double mel2hertz(double m);

	/// Compute twiddle factor.
	void setup_twiddle();

	/// Compute filterbank.
	void setup_filterbank();

	/// Compute dct matrix.
	void setup_dct();

	/// Cooley-Tukey DIT-FFT recursive function.
	std::vector<std::complex<double>> fft(const std::vector<std::complex<double>> &x);

	/// Find power spectrum.
	std::vector<double> power_spectrum(const std::vector<double> &r_frame);

	/// Apply log Mel filterbank.
	std::vector<double> lmfb(const std::vector<double> &P);

	/// Computing discrete cosine transform.
	std::vector<double> dct(const std::vector<double> &H);

	/// Balance the spectrum by subtracting mean.
	void normalise(std::vector<double> &C);

public:
	/// Constructor.
	MFCC();

	/// Find mfcc for given frame.
	std::vector<double> mfcc(const std::vector<double> &frame);
};
