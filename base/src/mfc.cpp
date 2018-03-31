#include "mfc.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

using namespace std;

MFC::MFC(int n_cepstra, bool q_gain, bool q_delta, bool q_accel) :
	ICepstral(n_cepstra, q_gain, q_delta, q_accel), filter_bank(setup_filter_bank()), dct_matrix(setup_dct_matrix(n_cepstra))
{
}

vector<vector<double>> MFC::setup_filter_bank()
{
	vector<vector<double>> filter_bank(n_filters, vector<double>(n_fft_bins, 0.0));

	// calculate filter centre-frequencies
	vector<double> hz_filter_center(n_filters + 2, 0.0);
	const function<double(double)> hertz2mel = [](double hz) { return 2595 * log10(1 + hz / 700); };
	const function<double(double)> mel2hertz = [](double mel) { return 700 * (pow(10, mel / 2595) - 1); };
	const double low_mel = hertz2mel(hz_low), high_mel = hertz2mel(hz_high);
	for (int i = 0; i < n_filters + 2; ++i)
	{
		hz_filter_center[i] = mel2hertz(low_mel + (high_mel - low_mel) / (n_filters + 1.0) * i);
	}

	// calculate FFT bin frequencies
	vector<double> hz_fft_bin(n_fft_bins, 0.0);
	for (int i = 0; i < n_fft_bins; ++i)
	{
		hz_fft_bin[i] = hz_sampling / 2.0 / (n_fft_bins - 1.0) * i;
	}

	for (int filter = 1; filter <= n_filters; ++filter)
	{
		for (int bin = 0; bin < n_fft_bins; ++bin)
		{
			if (hz_fft_bin[bin] < hz_filter_center[filter - 1])
			{
				filter_bank[filter - 1][bin] = 0.0;
			}
			else if (hz_fft_bin[bin] <= hz_filter_center[filter])
			{
				filter_bank[filter - 1][bin] = (hz_fft_bin[bin] - hz_filter_center[filter - 1]) / (hz_filter_center[filter] - hz_filter_center[filter - 1]);
			}
			else if (hz_fft_bin[bin] <= hz_filter_center[filter + 1])
			{
				filter_bank[filter - 1][bin] = (hz_filter_center[filter + 1] - hz_fft_bin[bin]) / (hz_filter_center[filter + 1] - hz_filter_center[filter]);
			}
		}
	}

	return filter_bank;
}

vector<vector<double>> MFC::setup_dct_matrix(int n_cepstra)
{
	vector<vector<double>> dct_matrix(n_cepstra + 1, vector<double>(n_filters, 0.0));

	const double pi = 4.0 * atan(1.0), c = sqrt(2.0 / n_filters);
	for (int i = 0; i < n_cepstra + 1; ++i)
	{
		for (int j = 0; j < n_filters; ++j)
		{
			dct_matrix[i][j] = c * cos(pi / n_filters * i * (0.5 + j));
		}
	}

	return dct_matrix;
}

Feature MFC::feature(const vector<double> &frame) const
{
	Feature feature;

	const vector<double> P = power_spectrum(frame);
	const vector<double> H = lmfb(P);
	feature.coefficients = dct(H);
	normalise(feature.coefficients);

	return feature;
}

vector<double> MFC::power_spectrum(const vector<double> &frame) const
{
	vector<double> P(n_fft_bins, 0.0);

	vector<complex<double>> complex_frame(frame.begin(), frame.end());
	complex_frame.resize(n_fft, complex<double>());
	fft(complex_frame);
	for (int i = 0; i < n_fft_bins; ++i)
	{
		P[i] = pow(abs(complex_frame[i]), 2);
	}

	return P;
}

void MFC::fft(vector<complex<double>> &x) const
{
	const int N = x.size();

	// DFT
	int n = N;
	const double theta = 4.0 * atan(1.0) / N;
	complex<double> phi(cos(theta), -sin(theta));
	while (n > 1)
	{
		n >>= 1;
		phi *= phi;

		complex<double> R(1.0, 0.0);
		for (int i = 0; i < n; ++i)
		{
			for (int j = i; j < N; j += n * 2)
			{
				const complex<double> t = x[j] - x[j + n];
				x[j] += x[j + n];
				x[j + n] = t * R;
			}
			R *= phi;
		}
	}

	// decimation
	const int m = log2(N);
	for (int i = 0; i < N; ++i)
	{
		// reverse bits
		int j = i;
		j = ((j & 0xaaaaaaaa) >> 1) | ((j & 0x55555555) << 1);
		j = ((j & 0xcccccccc) >> 2) | ((j & 0x33333333) << 2);
		j = ((j & 0xf0f0f0f0) >> 4) | ((j & 0x0f0f0f0f) << 4);
		j = ((j & 0xff00ff00) >> 8) | ((j & 0x00ff00ff) << 8);
		j = ((j >> 16) | (j << 16)) >> (32 - m);

		if (j > i)
		{
			swap(x[i], x[j]);
		}
	}
}

vector<double> MFC::lmfb(const vector<double> &P) const
{
	vector<double> H(n_filters, 0.0);

	for (int i = 0; i < n_filters; ++i)
	{
		for (int j = 0; j < n_fft_bins; ++j)
		{
			H[i] += filter_bank[i][j] * P[j];
		}

		H[i] = max(H[i], 1.0);
		H[i] = log(H[i]);
	}

	return H;
}

vector<double> MFC::dct(const vector<double> &H) const
{
	vector<double> C(n_cepstra + 1, 0.0);

	for (int i = 0; i < n_cepstra + 1; ++i)
	{
		for (int j = 0; j < n_filters; ++j)
		{
			C[i] += dct_matrix[i][j] * H[j];
		}
	}

	return C;
}

void MFC::normalise(vector<double> &C) const
{
	const double mean_C = accumulate(C.begin() + 1, C.end(), 0.0) / (n_cepstra);

	for (int i = 1; i < n_cepstra + 1; ++i)
	{
		C[i] -= mean_C;
	}
}
