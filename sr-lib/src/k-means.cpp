#include "k-means.h"

#include <algorithm>
#include <limits>
#include <iostream>

#include "maths.h"

#define KMEANS_CONVERGENCE_THRESHOLD 0.0000001
#define KMEANS_CONVERGENCE_MAX_ITERATIONS 50

using namespace std;

/// Calculates the distortion.
static double distortion(const vector<vector<double>> &distances, const vector<int> &indices) {
	double d = 0.0;

	for (int i = 0; i < indices.size(); ++i) {
		d += distances[i][indices[i]];
	}

	return d;
}

vector<vector<double>> distance_matrix(const vector<vector<double>> &universe, const vector<vector<double>> &centroids) {
	vector<vector<double>> vec(universe.size(), vector<double>(centroids.size(), 0.0));

	for (int i = 0; i < universe.size(); ++i) {
		for (int j = 0; j < centroids.size(); ++j) {
			vec[i][j] = tokhura_distance(universe[i], centroids[j]);
		}
	}

	return vec;
}

vector<int> classify_into_buckets(const vector<vector<double>> &universe, const vector<vector<double>> &centroids, const vector<vector<double>> &distances) {
	vector<int> indices(universe.size(), 0);

	for (int i = 0; i < universe.size(); ++i) {
		int min_j = 0;
		double min_distance = numeric_limits<double>::max();
		for (int j = 0; j < centroids.size(); ++j) {
			double distance = distances[i][j];
			if (distance < min_distance) {
				min_distance = distance;
				min_j = j;
			}
		}
		indices[i] = min_j;
	}

	return indices;
}

/// Locates centroids by taking means of vectors that have been put into respective buckets.
static vector<vector<double>> locate_centroids(const vector<vector<double>> &universe, const vector<int> &indices, const vector<vector<double>> &old_centroids) {
	vector<vector<double>> centroids(old_centroids.size(), vector<double>(universe[0].size(), 0.0));
	vector<int> bucket_sizes(old_centroids.size());

	for (int i = 0; i < universe.size(); ++i) {
		for (int j = 0; j < universe[0].size(); ++j) {
			centroids[indices[i]][j] += universe[i][j];
		}
		bucket_sizes[indices[i]]++;
	}
	for (int i = 0; i < centroids.size(); ++i) {
		for (int j = 0; j < universe[0].size(); ++j) {
			centroids[i][j] /= bucket_sizes[i];
		}
	}

	return centroids;
}

vector<vector<double>> optimize(const vector<vector<double>> &universe, const vector<vector<double>> &initial_centroids) {
	double old_D;
	int iteration = 0;
	vector<vector<double>> centroids(initial_centroids);
	vector<vector<double>> distances = distance_matrix(universe, centroids);
	vector<int> indices = classify_into_buckets(universe, centroids, distances);
	double D = distortion(distances, indices);

	do {
		old_D = D;
		iteration += 1;
		cout << "Kmeans: iteration: " << iteration << ", distortion is: " << D << endl;

		centroids = locate_centroids(universe, indices, centroids);
		distances = distance_matrix(universe, centroids);
		indices = classify_into_buckets(universe, centroids, distances);
		D = distortion(distances, indices);
	} while (old_D - D > KMEANS_CONVERGENCE_THRESHOLD && iteration < KMEANS_CONVERGENCE_MAX_ITERATIONS);

	return centroids;
}
