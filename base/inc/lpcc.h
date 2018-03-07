#pragma once

#include <vector>

class LPCC
{
private:
	const int n_cepstra = 12;
	std::vector<double> sine_coefficients;

	/// Compute sine coefficients
	void setup_sine_window();

	/// Find autocorrelation of a vector, given mi and mf.
	std::vector<double> auto_correlation(const std::vector<double> &frame, int mi, int mf);

	/// Levinson Durbin algorithm (Toeplitz matrix solver)
	std::vector<double> durbin_solve(const std::vector<double> R);

	/// Find cepstral coefficients.
	std::vector<double> cepstral_coefficients(const std::vector<double> &A);

	/// Apply sine window.
	void sine_window(std::vector<double> &C);

public:
	/// Constructor.
	LPCC();

	/// LPCC representation of a single frame.
	std::vector<double> coeffs(const std::vector<double> &frame);
};
