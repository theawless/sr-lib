#include "codebook.h"

#include <iostream>
#include <vector>

#include "k-means.h"

using namespace std;

/// Finds the initial mean of the universe. This is the first centroid.
static vector<double> universe_mean(const vector<vector<double>> &universe) {
	vector<double> mean(universe[0].size(), 0.0);

	for (int i = 0; i < universe.size(); ++i) {
		for (int j = 0; j < universe[0].size(); ++j) {
			mean[j] += universe[i][j];
		}
	}
	for (int i = 0; i < mean.size(); ++i) {
		mean[i] /= universe.size();
	}

	return mean;
}

/// Splits the current centroids into 2 parts.
static vector<vector<double>> split_centroids(const vector<vector<double>> &old_centroids) {
	const double epsilon = 0.025;
	vector<vector<double>> centroids(old_centroids.size() * 2, vector<double>(old_centroids[0].size(), 0.0));

	for (int i = 0; i < old_centroids.size(); ++i) {
		for (int j = 0; j < old_centroids[0].size(); ++j) {
			centroids[i][j] = old_centroids[i][j] + epsilon;
			centroids[old_centroids.size() + i][j] = old_centroids[i][j] - epsilon;
		}
	}

	return centroids;
}

vector<vector<double>> lbg_codebook(const vector<vector<double>> &universe, int M) {
	int m = 1;
	vector<vector<double>> centroids(1, universe_mean(universe));

	do {
		m *= 2;
		cout << "LBG: m is: " << m << endl;

		centroids = split_centroids(centroids);
		centroids = optimize(universe, centroids);
	} while (m < M);

	return centroids;
}

vector<int> observation_sequence(const vector<vector<double>> &observations, const vector<vector<double>> &codebook) {
	vector<vector<double>> distances = distance_matrix(observations, codebook);

	return classify_into_buckets(observations, codebook, distances);
}
