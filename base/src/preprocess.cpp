#include "preprocess.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

using namespace std;

Preprocessor::Preprocessor(int x_frame, int x_overlap) : x_frames(x_frame), x_overlap(x_overlap), hamming_coefficients(setup_hamming_coefficients(x_frames))
{
}

vector<vector<double>> Preprocessor::process(const vector<double> &unprocessed_samples) const
{
	vector<vector<double>> frames;

	vector<double> samples = unprocessed_samples;
	dc_offset(samples);
	normalise(samples);
	samples = trim(samples);
	pre_emphasize(samples);
	frames = framing(samples);

	return frames;
}

vector<double> Preprocessor::setup_hamming_coefficients(int x_frame)
{
	vector<double> hamming_coefficients(x_frame, 0.54);

	const double pi = 4.0 * atan(1.0);
	for (int i = 0; i < x_frame; ++i)
	{
		hamming_coefficients[i] -= 0.46 * cos(2 * i  * pi / (x_frame - 1));
	}

	return hamming_coefficients;
}

void Preprocessor::dc_offset(vector<double> &samples) const
{
	const double mean_sample = accumulate(samples.begin(), samples.end(), 0.0) / samples.size();

	for (int i = 0; i < samples.size(); ++i)
	{
		samples[i] -= mean_sample;
	}
}

void Preprocessor::normalise(vector<double> &samples) const
{
	const function<double(const vector<double> &)> maximum_absolute = [](const vector<double> &f)
	{
		return *max_element(f.begin(), f.end(), [](double a, double b) { return abs(a) < abs(b); });
	};
	const double normalisation_factor = normalisation_value / maximum_absolute(samples);

	for (int i = 0; i < samples.size(); ++i)
	{
		samples[i] *= normalisation_factor;
	}
}

vector<double> Preprocessor::trim(const vector<double> &samples) const
{
	vector<double> trimmed_samples;

	const int bg_window = 800, window = 80;
	const vector<double> bg(samples.begin(), samples.begin() + bg_window);
	const double mean_bg = accumulate(bg.begin(), bg.end(), 0.0) / samples.size();
	const double sd_bg = sqrt(accumulate(bg.begin(), bg.end(), 0.0, [mean_bg](double a, double b) { return a + pow(b - mean_bg, 2); }) / bg_window);

	for (int i = 0; i <= samples.size() - window; i += window)
	{
		int voiced = 0;
		for (int j = 0; j < window; ++j)
		{
			double distance = abs(samples[i + j] - mean_bg) / sd_bg;
			voiced += distance > 3.0 ? 1 : -1;
		}
		if (voiced > 0)
		{
			trimmed_samples.insert(trimmed_samples.end(), samples.begin() + i, samples.begin() + i + window);
		}
	}

	return trimmed_samples;
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
