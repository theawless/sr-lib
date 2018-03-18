#pragma once

#include <vector>

class Codebook
{
private:
	static constexpr double epsilon = 0.025;

	const int M;

	/// Finds the initial mean of the universe. This is the first centroid.
	static std::vector<double> mean(const std::vector<std::vector<double>> &universe);

	/// Splits the current centroids into 2 parts.
	void split();

public:
	std::vector<std::vector<double>> centroids;

	/// Constructor.
	Codebook(int M);

	/// Call kmeans co-coroutine and split the centroids and repeat till M is reached.
	void generate(const std::vector<std::vector<double>> &universe);

	/// Find the indexes of observation sequence of observations in codebook.
	std::vector<int> observations(const std::vector<std::vector<double>> &features) const;
};
