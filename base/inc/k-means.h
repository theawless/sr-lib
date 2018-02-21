#pragma once

#include <tuple>
#include <vector>

class KMeans
{
private:
	const double convergence_threshold = 0.0000001;
	const int convergence_max_iterations = 50;
	const std::vector<std::vector<double>> &universe;

	/// Locates centroids by taking means of vectors that have been put into respective buckets.
	void relocate(const std::vector<int> &indices, std::vector<std::vector<double>> &centroids);

public:
	/// Constructor.
	KMeans(const std::vector<std::vector<double>> &universe);

	/// Classifies the universe into buckets.
	std::pair<double, std::vector<int>> classify(const std::vector<std::vector<double>> &centroids);

	/// Can be called directly with random initial centroids, or as a co-routine in LBG algorithm.
	std::vector<std::vector<double>> optimize(const std::vector<std::vector<double>> &centroids);
};
