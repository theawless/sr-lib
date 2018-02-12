#pragma once

#include <vector>

/// Gives back a representation of speech in terms of lpc coefficients.
std::vector<std::vector<double>> speech_to_coefficients(const std::vector<std::vector<double>> &segments);

/// Compares two lpc coefficients vectors.
double coefficients_similarity(const std::vector<std::vector<double>> &A, const std::vector<std::vector<double>> &B);
