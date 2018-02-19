#pragma once

#include <vector>

class AudioProcessor
{
private:
	const int samples_per_frame;
	const int samples_window_overlap;
	const double normalisation_value = 5000.0;
	const double pre_emphasis_factor = 0.95;
	std::vector<double> hamming_coefficients;

	/// Setup hamming coefficients.
	void setup_hamming_window();

	/// Fix DC offset of the signal.
	void dc_offset(std::vector<double> &samples);

	/// Normalise amplitudes of the signal.
	void normalise(std::vector<double> &samples);

	/// Premphasize - boost the higher frequencies.
	void pre_emphasize(std::vector<double> &samples);

	/// Apply hamming window.
	void hamming_window(std::vector<double> &frame);

	/// Divides the segments into frames.
	std::vector<std::vector<double>> framing(const std::vector<double> &samples);

public:
	/// Constructor.
	AudioProcessor(int samples_per_frame, int samples_window_overlap);

	/// Process the samples and return frames.
	std::vector<std::vector<double>> process(const std::vector<double> &samples);
};
