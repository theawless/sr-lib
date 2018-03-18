#include "codebook.h"

#include "k-means.h"
#include "logger.h"

using namespace std;

vector<double> Codebook::mean(const vector<vector<double>> &universe)
{
	vector<double> mean(universe[0].size(), 0.0);

	for (int i = 0; i < universe.size(); ++i)
	{
		for (int j = 0; j < universe[0].size(); ++j)
		{
			mean[j] += universe[i][j];
		}
	}
	for (int i = 0; i < mean.size(); ++i)
	{
		mean[i] /= universe.size();
	}

	return mean;
}

void Codebook::split()
{
	const int N = centroids.size();
	centroids.resize(N * 2, vector<double>(centroids[0].size(), 0.0));

	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < centroids[0].size(); ++j)
		{
			centroids[N + i][j] = centroids[i][j] - epsilon;
			centroids[i][j] += epsilon;
		}
	}
}

Codebook::Codebook(int M) : M(M)
{
}

void Codebook::generate(const vector<vector<double>> &universe)
{
	int m = 1;
	const KMeans kmeans(universe);
	centroids = vector<vector<double>>(1, mean(universe));

	do
	{
		m *= 2;
		Logger::log("LBG: m is:", m);

		split();
		centroids = kmeans.optimise(centroids);
	} while (m < M);
}

vector<int> Codebook::observations(const vector<vector<double>> &features) const
{
	return KMeans(features).classify(centroids).second;
}
