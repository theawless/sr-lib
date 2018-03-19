#include "mfc.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

using namespace std;

map<int, map<int, complex<double>>> MFC::setup_twiddle()
{
	map<int, map<int, complex<double>>> twiddle;

	const complex<double> J(0, 1);
	const double pi = 4.0 * atan(1.0);
	for (int N = 2; N <= n_fft; N *= 2)
	{
		for (int k = 0; k <= N / 2 - 1; ++k)
		{
			twiddle[N][k] = exp(-2 * pi / N * k * J);
		}
	}

	return twiddle;
}

vector<vector<double>> MFC::setup_filter_bank()
{
	vector<vector<double>> filter_bank(n_filters, vector<double>(n_fft_bins, 0.0));

	// calculate filter centre-frequencies
	vector<double> hz_filter_center(n_filters + 2, 0.0);
	function<double(double)> hertz2mel = [](double hz) { return 2595 * log10(1 + hz / 700); };
	function<double(double)> mel2hertz = [](double mel) { return 700 * (pow(10, mel / 2595) - 1); };
	const double low_mel = hertz2mel(hz_low);
	const double high_mel = hertz2mel(hz_high);
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

	const double pi = 4.0 * atan(1.0);
	const double c = sqrt(2.0 / n_filters);
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
	const vector<double> P = power_spectrum(frame);
	const vector<double> H = lmfb(P);
	vector<double> C = dct(H);
	normalise(C);

	return Feature{ C };
}

vector<double> MFC::power_spectrum(const vector<double> &frame) const
{
	vector<double> P(n_fft_bins, 0.0);

	vector<complex<double>> complex_frame(frame.begin(), frame.end());
	complex_frame.resize(n_fft, complex<double>(0, 0));
	const vector<complex<double>> transformed_frame = fft(complex_frame);
	for (int i = 0; i < n_fft_bins; ++i)
	{
		P[i] = pow(abs(transformed_frame[i]), 2);
	}

	return P;
}

vector<complex<double>> MFC::fft(const vector<complex<double>> &x) const
{
	const int N = x.size();
	if (N == 1)
	{
		return x;
	}

	vector<complex<double>> xe(N / 2, 0.0), xo(N / 2, 0.0);
	for (int i = 0; i < N; i += 2)
	{
		// even indices
		xe[i / 2] = x[i];
	}
	for (int i = 1; i < N; i += 2)
	{
		// odd indices
		xo[(i - 1) / 2] = x[i];
	}

	// compute N/2-point FFT
	vector<complex<double>> Xjo = fft(xe), Xjo2 = fft(xo);
	Xjo.insert(Xjo.end(), Xjo2.begin(), Xjo2.end());

	// butterfly computations
	for (int i = 0; i <= N / 2 - 1; ++i)
	{
		const complex<double> t = Xjo[i], tw = twiddle.at(N).at(i);
		Xjo[i] = t + tw * Xjo[i + N / 2];
		Xjo[i + N / 2] = t - tw * Xjo[i + N / 2];
	}

	return Xjo;
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

MFC::MFC(int n_cepstra, bool q_gain, bool q_delta, bool q_accel) : ICepstral(n_cepstra, q_gain, q_delta, q_accel),
twiddle(setup_twiddle()), filter_bank(setup_filter_bank()), dct_matrix(setup_dct_matrix(n_cepstra))
{
}
