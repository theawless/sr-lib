#include "k-means.h"

#include <limits>

#include "logger.h"
#include "maths.h"

using namespace std;

void KMeans::relocate(const vector<int> &indices, vector<vector<double>> &centroids) const
{
	vector<int> bucket_sizes(centroids.size(), 0);

	centroids = vector<vector<double>>(centroids.size(), vector<double>(centroids[0].size(), 0.0));
	for (int i = 0; i < universe.size(); ++i)
	{
		for (int j = 0; j < universe[0].size(); ++j)
		{
			centroids[indices[i]][j] += universe[i][j];
		}
		bucket_sizes[indices[i]]++;
	}
	for (int i = 0; i < centroids.size(); ++i)
	{
		for (int j = 0; j < centroids[0].size(); ++j)
		{
			centroids[i][j] /= bucket_sizes[i];
		}
	}
}

KMeans::KMeans(const std::vector<std::vector<double>> &universe) : universe(universe)
{
}

vector<vector<double>> KMeans::optimise(const std::vector<std::vector<double>> &old_centroids) const
{
	int iteration = 0;
	vector<vector<double>> centroids = old_centroids;
	pair<double, vector<int>> buckets, new_buckets = classify(centroids);

	do
	{
		buckets = new_buckets;
		iteration += 1;
		Logger::log("Kmeans: iteration:", iteration, "distortion is:", buckets.first);

		relocate(buckets.second, centroids);
		new_buckets = classify(centroids);
	} while (buckets.first - new_buckets.first > convergence_threshold && iteration < convergence_max_iterations);

	return centroids;
}

pair<double, vector<int>> KMeans::classify(const std::vector<std::vector<double>> &centroids) const
{
	double distortion = 0.0;
	vector<int> indices(universe.size(), 0);

	for (int i = 0; i < universe.size(); ++i)
	{
		double min_distance = numeric_limits<double>::max();
		int min_j = 0;
		for (int j = 0; j < centroids.size(); ++j)
		{
			const double distance = Maths::distance(universe[i], centroids[j]);
			if (distance < min_distance)
			{
				min_distance = distance;
				min_j = j;
			}
		}
		distortion += min_distance;
		indices[i] = min_j;
	}

	return pair<double, vector<int>>(distortion, indices);
}
