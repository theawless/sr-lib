#pragma once

#include <vector>

class ICepstral
{
private:
	static constexpr int x_delta_window = 3;
	static constexpr int x_accel_window = 1;

	const bool q_gain;
	const bool q_delta;
	const bool q_accel;

	/// Subclasses will return coefficients for a frame.
	virtual std::vector<double> coefficients(const std::vector<double> &frame) const = 0;

	/// Gets delta vectors with given transgression window.
	static std::vector<std::vector<double>> delta(const std::vector<std::vector<double>> &coefficients, int W);

protected:
	const int n_cepstra;

public:
	/// Constructor.
	ICepstral(int n_cepstral, bool q_gain, bool q_delta, bool q_accel);

	/// Gets the features.
	std::vector<std::vector<double>> features(const std::vector<std::vector<double>> &frames) const;
};
