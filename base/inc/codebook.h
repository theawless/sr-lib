#pragma once

#include <vector>

#include "feature.h"

struct Codebook
{
public:
	std::vector<Feature> centroids;

	/// Return whether empty.
	bool empty() const;

	/// Find the buckets where the features lie in codebook.
	std::vector<int> observations(const std::vector<Feature> &features) const;

	/// Operators for loading and saving.
	friend std::istream &operator>>(std::istream &input, Codebook &codebook);
	friend std::ostream &operator<<(std::ostream &output, const Codebook &codebook);
};

/// An Algorithm for Vector Quantizer Design - Y. Linde, A. Buzo and R. Gray.
class LBG
{
public:
	/// Constructor.
	LBG(int x_codebook);

	/// Call Kmeans coroutine and split the centroids and till codebook size is reached.
	Codebook generate(const std::vector<Feature> &universe) const;

private:
	static constexpr double epsilon = 0.025;

	const int x_codebook;

	/// Find the initial centroid of the universe.
	static Feature mean(const std::vector<Feature> &universe);

	/// Split the given centroids.
	static void split(std::vector<Feature> &centroids);
};
