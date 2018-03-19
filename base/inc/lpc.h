#pragma once

#include <vector>

#include "features.h"

class LPC : public ICepstral
{
private:
	const int n_predict;
	const std::vector<double> sine_coefficients;

	/// Compute sine coefficients
	static std::vector<double> setup_sine_coefficients(int n_cepstra);

	/// Find lpcc for given frame.
	Feature feature(const std::vector<double> &frame) const;

	/// Find autocorrelation of a vector.
	std::vector<double> auto_correlation(const std::vector<double> &frame) const;

	/// Levinson Durbin algorithm (Toeplitz matrix solver)
	std::vector<double> durbin_solve(const std::vector<double> &R) const;

	/// Find the gain.
	double gain(const std::vector<double> &R, const std::vector<double> &A) const;

	/// Find cepstral coefficients.
	std::vector<double> cepstral_coefficients(double G_squared, const std::vector<double> &A) const;

	/// Apply sine window.
	void sine_window(std::vector<double> &C) const;

public:
	/// Constructor.
	LPC(int n_cepstra, bool q_gain, bool q_delta, bool q_accel, int n_predict);
};
