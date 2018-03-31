#pragma once

#include <iostream>
#include <vector>

struct Feature
{
public:
	std::vector<double> coefficients;

	/// Find the distance with another feature.
	double distance(const Feature &feature) const;

	/// Operators for loading and saving.
	friend std::istream &operator>>(std::istream &input, Feature &feature);
	friend std::ostream &operator<<(std::ostream &output, const Feature &feature);
};

class ICepstral
{
public:
	/// Constructor.
	ICepstral(int n_cepstral, bool q_gain, bool q_delta, bool q_accel);

	/// Get the features.
	std::vector<Feature> features(const std::vector<std::vector<double>> &frames) const;

protected:
	const int n_cepstra;

private:
	static constexpr int x_delta_window = 3;
	static constexpr int x_accel_window = 1;

	const bool q_gain;
	const bool q_delta;
	const bool q_accel;

	/// Subclasses will return coefficients for a frame.
	virtual Feature feature(const std::vector<double> &frame) const = 0;

	/// Get delta features with given transgression window.
	static std::vector<Feature> delta(const std::vector<Feature> &features, int W);
};
