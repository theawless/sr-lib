#include "features.h"

#include <cmath>

#include "utils.h"

using namespace std;

double Feature::distance(const Feature &feature) const
{
	double distance = 0.0;

	for (int i = 0; i < coefficients.size() && i < feature.coefficients.size(); ++i)
	{
		distance += pow((coefficients[i] - feature.coefficients[i]), 2);
	}

	return distance;
}

istream &operator>>(istream &input, Feature &feature)
{
	feature.coefficients = Utils::get_vector_from_stream<double>(input);

	return input;
}

ostream &operator<<(ostream &output, const Feature &feature)
{
	output << Utils::get_string_from_vector<double>(feature.coefficients);

	return output;
}

ICepstral::ICepstral(int n_cepstra, bool q_gain, bool q_delta, bool q_accel)
	:n_cepstra(n_cepstra), q_gain(q_gain), q_delta(q_delta), q_accel(q_accel)
{
}

vector<Feature> ICepstral::features(const vector<vector<double>> &frames) const
{
	vector<Feature> mixed_features(frames.size(), Feature{ vector<double>() });

	vector<Feature> features(frames.size(), Feature{ vector<double>() });
	const int offset = q_gain ? 0 : 1;
	for (int i = 0; i < frames.size(); ++i)
	{
		const Feature frame_feature = feature(frames[i]);
		features[i].coefficients.insert(features[i].coefficients.end(), frame_feature.coefficients.begin() + offset, frame_feature.coefficients.end());
		mixed_features[i].coefficients.insert(mixed_features[i].coefficients.end(), features[i].coefficients.begin(), features[i].coefficients.end());
	}

	if (q_delta)
	{
		const vector<Feature> delta_features = delta(features, x_delta_window);
		for (int i = 0; i < frames.size(); ++i)
		{
			mixed_features[i].coefficients.insert(mixed_features[i].coefficients.end(), delta_features[i].coefficients.begin(), delta_features[i].coefficients.end());
		}

		if (q_accel)
		{
			const vector<Feature> accel_features = delta(delta_features, x_accel_window);
			for (int i = 0; i < frames.size(); ++i)
			{
				mixed_features[i].coefficients.insert(mixed_features[i].coefficients.end(), accel_features[i].coefficients.begin(), accel_features[i].coefficients.end());
			}
		}
	}

	return mixed_features;
}

vector<Feature> ICepstral::delta(const vector<Feature> &features, int W)
{
	vector<Feature> delta = features;

	const double denominator = W * (2.0 * W + 1.0) * (W + 1.0) / 3.0 - pow(W, 2);
	for (int i = W; i < features.size() - W; ++i)
	{
		for (int j = 0; j < features[0].coefficients.size(); ++j)
		{
			double numerator = 0.0;
			for (int k = -W; k <= W; ++k)
			{
				numerator += k * features[k + i].coefficients[j];
			}
			delta[i].coefficients[j] = numerator / denominator;
		}
	}

	return delta;
}
