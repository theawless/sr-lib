#include "k-means.h"

#include <limits>
#include <iostream>

#include "maths.h"

using namespace std;

void KMeans::relocate(const vector<int> &indices, vector<vector<double>> &centroids)
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

pair<double, vector<int>> KMeans::classify(const std::vector<std::vector<double>> &centroids)
{
	double distortion = 0.0;
	vector<int> indices(universe.size(), 0);

	for (int i = 0; i < universe.size(); ++i)
	{
		double min_distance = numeric_limits<double>::max();
		int min_j = 0;
		for (int j = 0; j < centroids.size(); ++j)
		{
			double distance = Maths::tokhura_distance(universe[i], centroids[j]);
			if (distance < min_distance)
			{
				min_distance = distance;
				min_j = j;
			}
		}
		indices[i] = min_j;
		distortion += min_distance;
	}

	return pair<double, vector<int>>(distortion, indices);
}

vector<vector<double>> KMeans::optimize(const std::vector<std::vector<double>> &old_centroids)
{
	int iteration = 0;
	vector<vector<double>> centroids = old_centroids;
	pair<double, vector<int>> buckets, new_buckets = classify(centroids);

	do
	{
		buckets = new_buckets;
		iteration += 1;
		cout << "Kmeans: iteration: " << iteration << ", distortion is: " << buckets.first << endl;

		relocate(buckets.second, centroids);
		new_buckets = classify(centroids);
	} while (buckets.first - new_buckets.first > convergence_threshold && iteration < convergence_max_iterations);

	return centroids;
}
