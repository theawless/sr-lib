#pragma once

#include <vector>

/// Call kmeans co-coroutine and split the centroids and repeat till M is reached.
std::vector<std::vector<double>> lbg_codebook(const std::vector<std::vector<double>> &universe, int M);

/// Find the indexes of observation sequence of observations in codebook.
std::vector<int> observation_sequence(const std::vector<std::vector<double>> &observations, const std::vector<std::vector<double>> &codebook);
