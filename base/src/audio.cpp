#include "audio.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "maths.h"

using namespace std;

void AudioProcessor::setup_hamming_window()
{
	const double pi = 4.0 * atan(1.0);

	for (int i = 0; i < samples_per_frame; ++i)
	{
		hamming_coefficients[i] = 0.54 - (0.46 * cos((2 * pi * i) / (samples_per_frame - 1)));
	}
}

void AudioProcessor::dc_offset(vector<double> &samples)
{
	double sum_amplitude = accumulate(samples.begin(), samples.end(), 0.0);
	double mean_amplitude = sum_amplitude / samples.size();

	for (int i = 0; i < samples.size(); ++i)
	{
		samples[i] -= mean_amplitude;
	}
}

void AudioProcessor::normalise(vector<double> &samples)
{
	double max_amplitude = Maths::maximum_absolute(samples);
	double normalisation_factor = normalisation_value / max_amplitude;

	for (int i = 0; i < samples.size(); ++i)
	{
		samples[i] *= normalisation_factor;
	}
}

void AudioProcessor::pre_emphasize(vector<double> &samples)
{
	for (int i = 1; i < samples.size(); ++i)
	{
		samples[i] -= pre_emphasis_factor * samples[i - 1];
	}
}

void AudioProcessor::hamming_window(vector<double> &frame)
{
	for (int i = 0; i < frame.size(); ++i)
	{
		frame[i] *= hamming_coefficients[i];
	}
}

vector<vector<double>> AudioProcessor::framing(const vector<double> &samples)
{
	vector<vector<double>> frames;

	for (int i = 0; i <= samples.size() - samples_per_frame; i += samples_window_overlap)
	{
		vector<double>::const_iterator left = samples.begin() + i;
		vector<double>::const_iterator right = left + samples_per_frame;
		vector<double> frame(left, right);
		frames.push_back(frame);
	}

	return frames;
}

AudioProcessor::AudioProcessor(int samples_per_frame, int samples_window_overlap) : samples_per_frame(samples_per_frame),
samples_window_overlap(samples_window_overlap), hamming_coefficients(vector<double>(samples_per_frame))
{
	setup_hamming_window();
}

vector<vector<double>> AudioProcessor::process(const vector<double> &unprocessed_samples)
{
	vector<double> samples(unprocessed_samples);

	dc_offset(samples);
	normalise(samples);
	pre_emphasize(samples);
	vector<vector<double>> frames = framing(samples);
	for (int i = 0; i < frames.size(); ++i)
	{
		hamming_window(frames[i]);
	}

	return frames;
}
