#pragma once

#include <vector>

/// Trims the signal by taking a fixed window nearby the frame with max energy.
std::vector<double> trim_by_energy_fixed(const std::vector<double> &amplitudes);

/// Trims the signal by taking a threshold window nearby the frame with max energy.
std::vector<double> trim_by_energy_threshold(const std::vector<double> &amplitudes);

/// Trims the signal using Rabiner's algo and returns beginning noise - trimmed signal - ending noise.
std::pair<std::vector<double>, std::pair<std::vector<double>, std::vector<double>>> trim_by_auto(const std::vector<double> &amplitudes);
