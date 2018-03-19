#include "codebook.h"

#include "k-means.h"
#include "logger.h"
#include "utils.h"

using namespace std;

vector<int> Codebook::observations(const vector<Feature> &features) const
{
	return KMeans(features).classify(centroids).second;
}

istream &operator>>(istream &input, Codebook &codebook)
{
	codebook.centroids = Utils::get_vector_from_stream<Feature>(input, '\n');

	return input;
}

ostream &operator<<(ostream &output, const Codebook &codebook)
{
	output << Utils::get_string_from_vector<Feature>(codebook.centroids, '\n');

	return output;
}

Feature LBG::mean(const vector<Feature> &universe)
{
	Feature mean{ vector<double>(universe[0].coefficients.size(), 0.0) };

	for (int i = 0; i < universe.size(); ++i)
	{
		for (int j = 0; j < universe[0].coefficients.size(); ++j)
		{
			mean.coefficients[j] += universe[i].coefficients[j];
		}
	}
	for (int i = 0; i < mean.coefficients.size(); ++i)
	{
		mean.coefficients[i] /= universe.size();
	}

	return mean;
}

void LBG::split(vector<Feature> &centroids)
{
	const int N = centroids.size();
	centroids.resize(N * 2, Feature{ vector<double>(centroids[0].coefficients.size(), 0.0) });

	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < centroids[0].coefficients.size(); ++j)
		{
			centroids[N + i].coefficients[j] = centroids[i].coefficients[j] - epsilon;
			centroids[i].coefficients[j] += epsilon;
		}
	}
}

LBG::LBG(int M) : M(M)
{
}

Codebook LBG::generate(const vector<Feature> &universe) const
{
	Codebook codebook;

	int m = 1;
	const KMeans kmeans(universe);
	codebook.centroids = vector<Feature>(1, mean(universe));
	do
	{
		m *= 2;
		Logger::log("LBG: m is:", m);

		split(codebook.centroids);
		codebook.centroids = kmeans.optimise(codebook.centroids);
	} while (m < M);

	return codebook;
}
