#include "preprocess.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "maths.h"

using namespace std;

vector<double> Preprocessor::setup_hamming_coefficients(int x_frames)
{
	vector<double> hamming_coefficients(x_frames, 0.54);

	const double pi = 4.0 * atan(1.0);
	for (int i = 0; i < x_frames; ++i)
	{
		hamming_coefficients[i] -= 0.46 * cos(2 * i  * pi / (x_frames - 1));
	}

	return hamming_coefficients;
}

void Preprocessor::dc_offset(vector<double> &samples) const
{
	const double mean_amplitude = accumulate(samples.begin(), samples.end(), 0.0) / samples.size();

	for (int i = 0; i < samples.size(); ++i)
	{
		samples[i] -= mean_amplitude;
	}
}

void Preprocessor::normalise(vector<double> &samples) const
{
	const double normalisation_factor = normalisation_value / Maths::maximum_absolute(samples);

	for (int i = 0; i < samples.size(); ++i)
	{
		samples[i] *= normalisation_factor;
	}
}

void Preprocessor::pre_emphasize(vector<double> &samples) const
{
	for (int i = 1; i < samples.size(); ++i)
	{
		samples[i] -= pre_emphasis_factor * samples[i - 1];
	}
}

vector<vector<double>> Preprocessor::framing(const vector<double> &samples) const
{
	vector<vector<double>> frames;

	for (int i = 0; i <= samples.size() - x_frames; i += x_overlap)
	{
		const vector<double>::const_iterator left = samples.begin() + i;
		const vector<double>::const_iterator right = left + x_frames;
		vector<double> frame(left, right);
		hamming_window(frame);
		frames.push_back(frame);
	}

	return frames;
}

void Preprocessor::hamming_window(vector<double> &frame) const
{
	for (int i = 0; i < x_frames; ++i)
	{
		frame[i] *= hamming_coefficients[i];
	}
}

Preprocessor::Preprocessor(int x_frames, int x_overlap) : x_frames(x_frames), x_overlap(x_overlap), hamming_coefficients(setup_hamming_coefficients(x_frames))
{
}

vector<vector<double>> Preprocessor::process(const vector<double> &unprocessed_samples) const
{
	vector<double> samples = unprocessed_samples;

	dc_offset(samples);
	normalise(samples);
	pre_emphasize(samples);

	return framing(samples);
}
