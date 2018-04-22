#pragma once

#include <vector>

/// Digital Processing of Speech Signals - Lawrence Rabiner, R.W. Schafer.
class Preprocessor
{
public:
	/// Constructor.
	Preprocessor(bool q_trim, int x_frame, int x_overlap);

	/// Process the samples and return frames.
	std::vector<std::vector<double>> process(const std::vector<double> &samples) const;

private:
	static constexpr double normalisation_value = 5000.0;
	static constexpr double pre_emphasis_factor = 0.95;
	static constexpr int x_bg_window = 800;
	static constexpr int x_trim_window = 80;

	const std::vector<double> hamming_coefficients;
	const bool q_trim;
	const int x_frame;
	const int x_overlap;

	/// Setup hamming coefficients.
	static std::vector<double> setup_hamming_coefficients(int x_frame);

	/// Fix DC offset of the signal.
	void dc_offset(std::vector<double> &samples) const;

	/// Normalise the signal.
	void normalise(std::vector<double> &samples) const;

	/// Trim the signal.
	std::vector<double> trim(const std::vector<double> &samples) const;

	/// Premphasize - boost the higher frequencies.
	void pre_emphasize(std::vector<double> &samples) const;

	/// Divide the segments into frames.
	std::vector<std::vector<double>> framing(const std::vector<double> &samples) const;

	/// Apply hamming window.
	void hamming_window(std::vector<double> &frame) const;
};
