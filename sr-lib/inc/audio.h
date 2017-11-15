#pragma once

#include <vector>

/// Fix DC offset and normalise the data.
std::vector<double> preprocess(const std::vector<double> &amplitudes);

/// Segment the speech.
std::vector<std::vector<double>> fixed_segment(const std::vector<double> &amplitudes);
