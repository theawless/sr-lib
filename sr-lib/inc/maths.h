#pragma once

#include <vector>

/// Find autocorrelation of a vector, given mi and mf.
std::vector<double> autocorrelation(const std::vector<double> &f, int mi, int mf);

/// Find energy of a vector.
double energy(const std::vector<double> &f);

/// Find the magnitude energy of a vector.
double magnitude_energy(const std::vector<double> &f);

/// Find zcr of a vector.
int zcr(const std::vector<double> &f);

/// Apply a linear window to a vector.
std::vector<double> linear_window(const std::vector<double> &f);

/// Apply a hamming window to a vector.
std::vector<double> hamming_window(const std::vector<double> &f);

/// Apply a sine window to a vector.
std::vector<double> sine_window(const std::vector<double> &c);

/// Levinson Durbin algorithm (Toeplitz matrix solver)
std::vector<double> durbin_solve(const std::vector<double> R);

/// Find cepstral coefficients from the given vector.
std::vector<double> cepstral_coefficients(const std::vector<double> &A);

/// Find Tokhura's distance betweent two vectors.
double tokhura_distance(const std::vector<double> &A, const std::vector<double> &B);
