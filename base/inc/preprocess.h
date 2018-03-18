#pragma once

#include <vector>

class Preprocessor
{
private:
	static constexpr double normalisation_value = 5000.0;
	static constexpr double pre_emphasis_factor = 0.95;

	const int x_frames;
	const int x_overlap;
	const std::vector<double> hamming_coefficients;

	/// Setup hamming coefficients.
	static std::vector<double> setup_hamming_coefficients(int x_frames);

	/// Fix DC offset of the signal.
	void dc_offset(std::vector<double> &samples) const;

	/// Normalise amplitudes of the signal.
	void normalise(std::vector<double> &samples) const;

	/// Premphasize - boost the higher frequencies.
	void pre_emphasize(std::vector<double> &samples) const;

	/// Divides the segments into frames.
	std::vector<std::vector<double>> framing(const std::vector<double> &samples) const;

	/// Apply hamming window.
	void hamming_window(std::vector<double> &frame) const;

public:
	/// Constructor.
	Preprocessor(int x_frames, int x_overlap);

	/// Process the samples and return frames.
	std::vector<std::vector<double>> process(const std::vector<double> &samples) const;
};
