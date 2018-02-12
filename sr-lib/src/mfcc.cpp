#include <mfcc.h>

#include <algorithm>
#include <cmath>
#include <complex>
#include <map>
#include <numeric>
#include <vector>

using namespace std;

class MFCC {

private:
	const double PI = 4 * atan(1.0);
	int fs;
	map<int, std::map<int, complex<double>>> twiddle;
	size_t numCepstra, numFFT, numFFTBins, numFilters;
	double preEmphCoef, lowFreq, highFreq;
	vector<double> frame, powerSpectralCoef, lmfbCoef, mfcc;
	vector<vector<double>> fbank, dct;

private:
	inline double hz2mel(double f) {
		return 2595 * std::log10(1 + f / 700);
	}

	inline double mel2hz(double m) {
		return 700 * (std::pow(10, m / 2595) - 1);
	}

	// Twiddle factor computation
	void compTwiddle(void) {
		const complex<double> J(0, 1);
		for (int N = 2; N <= numFFT; N *= 2) {
			for (int k = 0; k <= N / 2 - 1; k++) {
				twiddle[N][k] = exp(-2 * PI*k / N * J);
			}
		}
	}

	// Cooley-Tukey DIT-FFT recursive function
	vector<complex<double>> fft(vector<complex<double>> x) {
		int N = x.size();
		if (N == 1) {
			return x;
		}

		vector<complex<double>> xe(N / 2, 0), xo(N / 2, 0), Xjo, Xjo2;
		int i;

		// Construct arrays from even and odd indices
		for (i = 0; i < N; i += 2) {
			xe[i / 2] = x[i];
		}
		for (i = 1; i < N; i += 2) {
			xo[(i - 1) / 2] = x[i];
		}

		// Compute N/2-point FFT
		Xjo = fft(xe);
		Xjo2 = fft(xo);
		Xjo.insert(Xjo.end(), Xjo2.begin(), Xjo2.end());

		// Butterfly computations
		for (i = 0; i <= N / 2 - 1; i++) {
			complex<double> t = Xjo[i], tw = twiddle[N][i];
			Xjo[i] = t + tw * Xjo[i + N / 2];
			Xjo[i + N / 2] = t - tw * Xjo[i + N / 2];
		}
		return Xjo;
	}

	// Power spectrum computation
	void computePowerSpec(void) {
		frame.resize(numFFT); // Pads zeros
		vector<complex<double>> framec(frame.begin(), frame.end()); // Complex frame
		vector<complex<double>> fftc = fft(framec);

		for (int i = 0; i < numFFTBins; i++) {
			powerSpectralCoef[i] = pow(abs(fftc[i]), 2);
		}
	}

	// Applying log Mel filterbank (LMFB)
	void applyLMFB(void) {
		lmfbCoef.assign(numFilters, 0);

		for (int i = 0; i < numFilters; i++) {
			// Multiply the filterbank matrix
			for (int j = 0; j < fbank[i].size(); j++) {
				lmfbCoef[i] += fbank[i][j] * powerSpectralCoef[j];
			}
			// Apply Mel-flooring
			if (lmfbCoef[i] < 1.0) {
				lmfbCoef[i] = 1.0;
			}
		}

		// Applying log on amplitude
		for (int i = 0; i < numFilters; i++) {
			lmfbCoef[i] = std::log(lmfbCoef[i]);
		}
	}

	// Computing discrete cosine transform
	void applyDct(void) {
		mfcc.assign(numCepstra + 1, 0);
		for (int i = 0; i <= numCepstra; i++) {
			for (int j = 0; j < numFilters; j++)
				mfcc[i] += dct[i][j] * lmfbCoef[j];
		}
	}

	// Initialisation routines pre-computing and dct matrix
	void initDct(void) {
		int i, j;

		vector<double> v1(numCepstra + 1, 0), v2(numFilters, 0);
		for (i = 0; i <= numCepstra; i++)
			v1[i] = i;
		for (i = 0; i < numFilters; i++)
			v2[i] = i + 0.5;

		dct.reserve(numFilters*(numCepstra + 1));
		double c = sqrt(2.0 / numFilters);
		for (i = 0; i <= numCepstra; i++) {
			vector<double> dtemp;
			for (j = 0; j < numFilters; j++)
				dtemp.push_back(c * cos(PI / numFilters * v1[i] * v2[j]));
			dct.push_back(dtemp);
		}
	}

	// Precompute filterbank
	void initFilterbank() {
		// Convert low and high frequencies to Mel scale
		double lowFreqMel = hz2mel(lowFreq);
		double highFreqMel = hz2mel(highFreq);

		// Calculate filter centre-frequencies
		vector<double> filterCentreFreq;
		filterCentreFreq.reserve(numFilters + 2);
		for (int i = 0; i < numFilters + 2; i++)
			filterCentreFreq.push_back(mel2hz(lowFreqMel + (highFreqMel - lowFreqMel) / (numFilters + 1)*i));

		// Calculate FFT bin frequencies
		vector<double> fftBinFreq;
		fftBinFreq.reserve(numFFTBins);
		for (int i = 0; i < numFFTBins; i++)
			fftBinFreq.push_back(fs / 2.0 / (numFFTBins - 1)*i);

		// Filterbank: Allocate memory
		fbank.reserve(numFilters*numFFTBins);

		// Populate the fbank matrix
		for (int filt = 1; filt <= numFilters; filt++) {
			vector<double> ftemp;
			for (int bin = 0; bin < numFFTBins; bin++) {
				double weight;
				if (fftBinFreq[bin] < filterCentreFreq[filt - 1])
					weight = 0;
				else if (fftBinFreq[bin] <= filterCentreFreq[filt])
					weight = (fftBinFreq[bin] - filterCentreFreq[filt - 1]) / (filterCentreFreq[filt] - filterCentreFreq[filt - 1]);
				else if (fftBinFreq[bin] <= filterCentreFreq[filt + 1])
					weight = (filterCentreFreq[filt + 1] - fftBinFreq[bin]) / (filterCentreFreq[filt + 1] - filterCentreFreq[filt]);
				else
					weight = 0;
				ftemp.push_back(weight);
			}
			fbank.push_back(ftemp);
		}
	}

public:
	// MFCC class constructor
	MFCC(int sampFreq = 16000, int nCep = 12, int numFilt = 40, double lf = 50, double hf = 6500) {
		fs = sampFreq;     // Sampling frequency
		numCepstra = nCep;         // Number of cepstra
		numFFT = 512;          // FFT size
		numFilters = numFilt;      // Number of Mel warped filters
		preEmphCoef = 0.97;         // Pre-emphasis coefficient
		lowFreq = lf;           // Filterbank low frequency cutoff in Hertz
		highFreq = hf;         // Filterbank high frequency cutoff in Hertz

		numFFTBins = numFFT / 2 + 1;
		powerSpectralCoef.assign(numFFTBins, 0);

		initFilterbank();
		initDct();
		compTwiddle();
	}

	// Process each frame and extract MFCC
	std::vector<double> processFrame(const vector<double> &samples, size_t N) {
		frame = samples;
		computePowerSpec();
		applyLMFB();
		applyDct();

		return mfcc;
	}
};

MFCC mfcc = MFCC();

vector<vector<double>> speech_to_coefficients(const vector<vector<double>> &segments) {
	vector<vector<double>> coefficients;

	for (int i = 0; i < segments.size(); ++i) {
		vector<double> C = mfcc.processFrame(segments[0], segments.size());
		coefficients.push_back(C);
	}

	return coefficients;
}
