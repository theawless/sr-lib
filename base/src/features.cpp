#include "features.h"

#include <cmath>

using namespace std;

vector<vector<double>> ICepstral::delta(const vector<vector<double>> &coefficients, int W)
{
	vector<vector<double>> delta(coefficients);

	const double denominator = W * (2.0 * W + 1.0) * (W + 1.0) / 3.0 - pow(W, 2);
	for (int i = W; i < coefficients.size() - W; ++i)
	{
		for (int j = 0; j < coefficients[0].size(); ++j)
		{
			double numerator = 0.0;
			for (int k = -W; k <= W; ++k)
			{
				numerator += k * coefficients[k + i][j];
			}
			delta[i][j] = numerator / denominator;
		}
	}

	return delta;
}

ICepstral::ICepstral(int n_cepstra, bool q_gain, bool q_delta, bool q_accel)
	:n_cepstra(n_cepstra), q_gain(q_gain), q_delta(q_delta), q_accel(q_accel)
{
}

vector<vector<double>> ICepstral::features(const vector<vector<double>> &frames) const
{
	vector<vector<double>> mixed_features(frames.size(), vector<double>());

	vector<vector<double>> coeffs;
	const int offset = q_gain ? 0 : 1;
	for (int i = 0; i < frames.size(); ++i)
	{
		const vector<double> frame_coeffs = coefficients(frames[i]);
		coeffs.push_back(vector<double>(frame_coeffs.begin() + offset, frame_coeffs.end()));
		mixed_features[i].insert(mixed_features[i].end(), coeffs.back().begin(), coeffs.back().end());
	}

	if (q_delta)
	{
		const vector<vector<double>> delta_coeffs = delta(coeffs, x_delta_window);
		for (int i = 0; i < frames.size(); ++i)
		{
			mixed_features[i].insert(mixed_features[i].end(), delta_coeffs[i].begin(), delta_coeffs[i].end());
		}

		if (q_accel)
		{
			const vector<vector<double>> accel_coeffs = delta(delta_coeffs, x_accel_window);
			for (int i = 0; i < frames.size(); ++i)
			{
				mixed_features[i].insert(mixed_features[i].end(), accel_coeffs[i].begin(), accel_coeffs[i].end());
			}
		}
	}

	return mixed_features;
}
