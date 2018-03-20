#pragma once

#include <vector>

#include "features.h"

struct Codebook
{
public:
	std::vector<Feature> centroids;

	/// Find the indexes of observation sequence of observations in codebook.
	std::vector<int> observations(const std::vector<Feature> &features) const;

	/// Operators for loading and saving.
	friend std::istream &operator>>(std::istream &input, Codebook &codebook);
	friend std::ostream &operator<<(std::ostream &output, const Codebook &codebook);
};

class LBG
{
public:
	/// Constructor.
	LBG(int M);

	/// Call kmeans co-coroutine and split the centroids and repeat till M is reached.
	Codebook generate(const std::vector<Feature> &universe) const;

private:
	static constexpr double epsilon = 0.025;

	const int M;

	/// Finds the initial mean of the universe. This is the first centroid.
	static Feature mean(const std::vector<Feature> &universe);

	/// Splits the current codebook into 2.
	static void split(std::vector<Feature> &centroids);
};
