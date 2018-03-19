#pragma once

#include <tuple>
#include <vector>

#include "features.h"

class KMeans
{
private:
	static constexpr double convergence_threshold = 0.0000001;
	static constexpr int convergence_max_iterations = 50;

	const std::vector<Feature> &universe;

	/// Locates centroids by taking means of vectors that have been put into respective buckets.
	void relocate(const std::vector<int> &indices, std::vector<Feature> &centroids) const;

public:
	/// Constructor.
	KMeans(const std::vector<Feature> &universe);

	/// Can be called directly with random initial centroids, or as a co-routine in LBG algorithm.
	std::vector<Feature> optimise(const std::vector<Feature> &centroids) const;

	/// Classifies the universe into buckets.
	std::pair<double, std::vector<int>> classify(const std::vector<Feature> &centroids) const;
};
