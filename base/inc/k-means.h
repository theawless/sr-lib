#pragma once

#include <utility>
#include <vector>

#include "features.h"

class KMeans
{
public:
	/// Constructor.
	KMeans(const std::vector<Feature> &universe);

	/// Optimise the centroids.
	std::vector<Feature> optimise(const std::vector<Feature> &centroids) const;

	/// Classify the universe into buckets.
	std::pair<double, std::vector<int>> classify(const std::vector<Feature> &centroids) const;

private:
	static constexpr double convergence_threshold = 0.0000001;
	static constexpr int convergence_max_iterations = 50;

	const std::vector<Feature> universe;

	/// Locate centroids by taking means of vectors that have been put into respective buckets.
	void relocate(const std::vector<int> &indices, std::vector<Feature> &centroids) const;
};
