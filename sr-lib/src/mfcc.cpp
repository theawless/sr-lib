#include <mfcc.h>

#include <algorithm>
#include <cmath>
#include <numeric>

using namespace std;

double MFCC::hertz2mel(double hz)
{
	return 2595 * log10(1 + hz / 700);
}

double MFCC::mel2hertz(double mel)
{
	return 700 * (pow(10, mel / 2595) - 1);
}

void MFCC::setup_twiddle()
{
	const complex<double> J(0, 1);

	for (int N = 2; N <= n_fft; N *= 2)
	{
		for (int k = 0; k <= N / 2 - 1; ++k)
		{
			twiddle[N][k] = exp(-2 * PI * k / N * J);
		}
	}
}

void MFCC::setup_filterbank()
{
	double low_mel = hertz2mel(hz_low);
	double high_mel = hertz2mel(hz_high);

	// Calculate filter centre-frequencies
	vector<double> hz_filter_center(n_filters + 2, 0.0);
	for (int i = 0; i < n_filters + 2; ++i)
	{
		hz_filter_center[i] = mel2hertz(low_mel + (high_mel - low_mel) / (n_filters + 1) * i);
	}

	// Calculate FFT bin frequencies
	vector<double> hz_fft_bin(n_fft_bins, 0.0);
	for (int i = 0; i < n_fft_bins; ++i)
	{
		hz_fft_bin[i] = hz_sampling / 2.0 / (n_fft_bins - 1) * i;
	}

	for (int filter = 1; filter <= n_filters; ++filter)
	{
		vector<double> row;
		for (int bin = 0; bin < n_fft_bins; ++bin)
		{
			double weight;
			if (hz_fft_bin[bin] < hz_filter_center[filter - 1])
			{
				weight = 0;
			}
			else if (hz_fft_bin[bin] <= hz_filter_center[filter])
			{
				weight = (hz_fft_bin[bin] - hz_filter_center[filter - 1]) / (hz_filter_center[filter] - hz_filter_center[filter - 1]);
			}
			else if (hz_fft_bin[bin] <= hz_filter_center[filter + 1])
			{
				weight = (hz_filter_center[filter + 1] - hz_fft_bin[bin]) / (hz_filter_center[filter + 1] - hz_filter_center[filter]);
			}
			else
			{
				weight = 0;
			}
			row.push_back(weight);
		}
		filter_bank.push_back(row);
	}
}

void MFCC::setup_dct()
{
	double c = sqrt(2.0 / n_filters);
	vector<double> v1(n_cepstra + 1, 0.0), v2(n_filters, 0.0);

	for (int i = 0; i <= n_cepstra; ++i)
	{
		v1[i] = i;
	}
	for (int i = 0; i < n_filters; ++i)
	{
		v2[i] = i + 0.5;
	}
	for (int i = 0; i <= n_cepstra; ++i)
	{
		vector<double> row;
		for (int j = 0; j < n_filters; j++)
		{
			row.push_back(c * cos(PI / n_filters * v1[i] * v2[j]));
		}
		dct_matrix.push_back(row);
	}
}

vector<complex<double>> MFCC::fft(const vector<complex<double>> &x)
{
	int N = x.size();
	if (N == 1)
	{
		return x;
	}
	vector<complex<double>> xe(N / 2, 0.0), xo(N / 2, 0.0), Xjo, Xjo2;

	// Construct arrays from even and odd indices
	for (int i = 0; i < N; i += 2)
	{
		xe[i / 2] = x[i];
	}
	for (int i = 1; i < N; i += 2)
	{
		xo[(i - 1) / 2] = x[i];
	}

	// Compute N/2-point FFT
	Xjo = fft(xe);
	Xjo2 = fft(xo);
	Xjo.insert(Xjo.end(), Xjo2.begin(), Xjo2.end());

	// Butterfly computations
	for (int i = 0; i <= N / 2 - 1; ++i)
	{
		complex<double> t = Xjo[i], tw = twiddle[N][i];
		Xjo[i] = t + tw * Xjo[i + N / 2];
		Xjo[i + N / 2] = t - tw * Xjo[i + N / 2];
	}

	return Xjo;
}

vector<double> MFCC::power_spectrum(const vector<double> &r_frame)
{
	vector<double> P(n_fft_bins, 0.0);
	vector<complex<double>> frame(r_frame.begin(), r_frame.end());
	frame.resize(n_fft, complex<double>(0, 0));
	vector<complex<double>> transformed_frame = fft(frame);

	for (int i = 0; i < n_fft_bins; ++i)
	{
		P[i] = pow(abs(transformed_frame[i]), 2);
	}

	return P;
}

vector<double> MFCC::lmfb(const vector<double> &P)
{
	vector<double> H(n_filters, 0.0);

	for (int i = 0; i < n_filters; ++i)
	{
		for (int j = 0; j < filter_bank[i].size(); ++j)
		{
			H[i] += filter_bank[i][j] * P[j];
		}
		H[i] = max(H[i], 1.0);
	}
	for (int i = 0; i < n_filters; i++)
	{
		H[i] = log(H[i]);
	}

	return H;
}

void MFCC::normalise(vector<double> &C)
{
	double sum_amplitude = accumulate(C.begin(), C.end(), 0.0);
	double mean_amplitude = sum_amplitude / C.size();

	for (int i = 0; i < C.size(); ++i)
	{
		C[i] -= mean_amplitude;
	}
}

vector<double> MFCC::dct(const vector<double> &H)
{
	vector<double> C(n_cepstra + 1, 0.0);

	for (int i = 0; i <= n_cepstra; ++i)
	{
		for (int j = 0; j < n_filters; ++j)
		{
			C[i] += dct_matrix[i][j] * H[j];
		}
	}

	return vector<double>(C.begin() + 1, C.end());
}

MFCC::MFCC()
{
	setup_twiddle();
	setup_filterbank();
	setup_dct();
}

vector<double> MFCC::mfcc(const vector<double> &frame)
{
	vector<double> P = power_spectrum(frame);
	vector<double> H = lmfb(P);
	vector<double> C = dct(H);
	normalise(C);

	return C;
}

vector<vector<double>> MFCC::mfcc(const vector<vector<double>> &frames)
{
	vector<vector<double>> mfccs;

	for (int i = 0; i < frames.size(); ++i)
	{
		mfccs.push_back(mfcc(frames[i]));
	}

	return mfccs;
}
