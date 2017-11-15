#pragma once

#include <vector>

/// Calculates the tokhura distances for the complete universe wrt all centroids.
std::vector<std::vector<double>> distance_matrix(const std::vector<std::vector<double>> &universe, const std::vector<std::vector<double>> &centroids);

/// Classifies the universe into buckets.
std::vector<int> classify_into_buckets(const std::vector<std::vector<double>> &universe, const std::vector<std::vector<double>> &centroids, const std::vector<std::vector<double>> &distances);

/// Can be called directly with random initial centroids, or as a co-routine in LBG algorithm.
std::vector<std::vector<double>> optimize(const std::vector<std::vector<double>> &universe, const std::vector<std::vector<double>> &initial_centroids);
