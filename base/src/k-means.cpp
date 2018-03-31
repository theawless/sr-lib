#include "k-means.h"

#include <limits>

#include "logger.h"

using namespace std;

KMeans::KMeans(const vector<Feature> &universe) :
	universe(universe)
{
}

vector<Feature> KMeans::optimise(const vector<Feature> &old_centroids) const
{
	vector<Feature> centroids = old_centroids;

	int iteration = 0;
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

pair<double, vector<int>> KMeans::classify(const vector<Feature> &centroids) const
{
	pair<double, vector<int>> buckets(0.0, vector<int>(universe.size(), 0));

	for (int i = 0; i < universe.size(); ++i)
	{
		double min_distance = numeric_limits<double>::max();
		int min_j = 0;
		for (int j = 0; j < centroids.size(); ++j)
		{
			const double distance = universe[i].distance(centroids[j]);
			if (distance < min_distance)
			{
				min_distance = distance;
				min_j = j;
			}
		}
		buckets.first += min_distance;
		buckets.second[i] = min_j;
	}

	return buckets;
}

void KMeans::relocate(const vector<int> &indices, vector<Feature> &centroids) const
{
	vector<int> bucket_sizes(centroids.size(), 0);

	centroids = vector<Feature>(centroids.size(), Feature{ vector<double>(centroids[0].coefficients.size(), 0.0) });
	for (int i = 0; i < universe.size(); ++i)
	{
		for (int j = 0; j < universe[0].coefficients.size(); ++j)
		{
			centroids[indices[i]].coefficients[j] += universe[i].coefficients[j];
		}
		bucket_sizes[indices[i]]++;
	}
	for (int i = 0; i < centroids.size(); ++i)
	{
		for (int j = 0; j < centroids[0].coefficients.size(); ++j)
		{
			centroids[i].coefficients[j] /= bucket_sizes[i];
		}
	}
}
