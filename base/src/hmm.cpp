#include "hmm.h"

#include <algorithm>
#include <cmath>

#include "logger.h"

using namespace std;

HMM::HMM(const Model &lambda) :
	lambda(lambda)
{
}

Model HMM::optimise(const vector<int> &o)
{
	int iteration = 0;
	double old_P_star, P_star;

	tweak();
	P_star = viterbi(o).first;
	do
	{
		iteration += 1;
		old_P_star = P_star;
		Logger::log("Restimate lambda: iteration:", iteration, "P* is:", P_star);

		restimate(o);
		tweak();
		P_star = viterbi(o).first;
	} while (P_star / old_P_star > convergence_threshold && iteration < convergence_max_iterations);

	return lambda;
}

pair<double, vector<vector<double>>> HMM::forward(const vector<int> &o) const
{
	const int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();
	pair<double, vector<vector<double>>> alpha(0.0, vector<vector<double>>(T, vector<double>(N, 0.0)));

	vector<double> C(T, 0.0);
	for (int i = 0; i < N; ++i)
	{
		alpha.second[0][i] = lambda.pi[i] * lambda.b[i][o[0]];
		C[0] += alpha.second[0][i];
	}
	C[0] = 1 / C[0];
	for (int i = 0; i < N; ++i)
	{
		alpha.second[0][i] *= C[0];
	}
	for (int t = 0; t < T - 1; ++t)
	{
		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j < N; ++j)
			{
				alpha.second[t + 1][i] += alpha.second[t][j] * lambda.a[j][i];
			}
			alpha.second[t + 1][i] *= lambda.b[i][o[t + 1]];
			C[t + 1] += alpha.second[t + 1][i];
		}
		C[t + 1] = 1 / C[t + 1];
		for (int i = 0; i < N; ++i)
		{
			alpha.second[t + 1][i] *= C[t + 1];
		}
	}

	for (int t = 0; t < T; ++t)
	{
		alpha.first -= log(C[t]);
	}

	return alpha;
}

void HMM::tweak()
{
	const int M = lambda.b[0].size(), N = lambda.b.size();

	for (int i = 0; i < N; ++i)
	{
		double dummy = minimum_probability;
		for (int j = 0; j < N; ++j)
		{
			if (lambda.a[i][j] != 0)
			{
				dummy = min(dummy, lambda.a[i][j] / 10.0);
			}
		}
		int count = 0;
		for (int j = 0; j < N; ++j)
		{
			if (lambda.a[i][j] == 0.0)
			{
				lambda.a[i][j] = dummy;
				count++;
			}
		}
		int max_j = 0;
		for (int j = 0; j < N; ++j)
		{
			if (lambda.a[i][j] > lambda.a[i][max_j])
			{
				max_j = j;
			}
		}
		lambda.a[i][max_j] -= count * dummy;
	}

	for (int i = 0; i < N; ++i)
	{
		double dummy = minimum_probability;
		for (int j = 0; j < M; ++j)
		{
			if (lambda.b[i][j] != 0)
			{
				dummy = min(dummy, lambda.b[i][j] / 10.0);
			}
		}
		int count = 0;
		for (int j = 0; j < M; ++j)
		{
			if (lambda.b[i][j] == 0.0)
			{
				lambda.b[i][j] = dummy;
				count++;
			}
		}
		int max_j = 0;
		for (int j = 0; j < M; ++j)
		{
			if (lambda.b[i][j] > lambda.b[i][max_j])
			{
				max_j = j;
			}
		}
		lambda.b[i][max_j] -= count * dummy;
	}
}

pair<double, vector<int>> HMM::viterbi(const vector<int> &o) const
{
	const int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();
	pair<double, vector<int>> q(0.0, vector<int>(T, 0));

	vector<int> psi(T, 0);
	vector<vector<double>> delta(T, vector<double>(N, 0.0));
	for (int i = 0; i < N; ++i)
	{
		double temp = lambda.pi[i];
		if (temp == 0.0)
		{
			temp = minimum_probability;
		}
		delta[0][i] = log(temp) + log(lambda.b[i][o[0]]);
	}
	for (int t = 0; t < T - 1; ++t)
	{
		for (int i = 0; i < N; ++i)
		{
			delta[t + 1][i] = numeric_limits<double>::min();
			for (int j = 0; j < N; ++j)
			{
				double current_max_delta = delta[t][j] + log(lambda.a[j][i]);
				if (delta[t + 1][i] < current_max_delta)
				{
					delta[t + 1][i] = current_max_delta;
					psi[i] = j;
				}
			}
			delta[t + 1][i] += lambda.b[i][o[t + 1]];
		}
	}

	q.second[T - 1] = max_element(delta[T - 1].begin(), delta[T - 1].end()) - delta[T - 1].begin();
	q.first = delta[T - 1][q.second[T - 1]];
	for (int i = T - 2; i >= 0; --i)
	{
		q.second[i] = psi[q.second[i + 1]];
	}

	return q;
}

vector<vector<double>> HMM::backward(const vector<int> &o) const
{
	const int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();
	vector<vector<double>> beta(T, vector<double>(N, 0.0));

	vector<double> C(T, 0.0);
	for (int i = 0; i < N; ++i)
	{
		beta[T - 1][i] = 1;
		C[0] += beta[T - 1][i];
	}
	C[0] = 1 / C[0];
	for (int i = 0; i < N; ++i)
	{
		beta[T - 1][i] *= C[0];
	}
	for (int t = T - 2; t >= 0; --t)
	{
		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j < N; ++j)
			{
				beta[t][i] += beta[t + 1][j] * lambda.a[i][j] * lambda.b[j][o[t + 1]];
			}
			C[t + 1] += beta[t][i];
		}
		C[t + 1] = 1 / C[t + 1];
		for (int i = 0; i < N; ++i)
		{
			beta[t][i] *= C[t + 1];
		}
	}

	return beta;
}

void HMM::restimate(const vector<int> &o)
{
	const int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();

	const vector<vector<double>> alpha = forward(o).second;
	const vector<vector<double>> beta = backward(o);
	vector<vector<vector<double>>> xsi(T, vector<vector<double>>(N, vector<double>(N, 0.0)));
	for (int t = 0; t < T - 1; ++t)
	{
		double denominator = 0.0;
		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j < N; ++j)
			{
				denominator += alpha[t][i] * lambda.a[i][j] * lambda.b[j][o[t + 1]] * beta[t + 1][j];
			}
		}
		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j < N; ++j)
			{
				double numerator = alpha[t][i] * lambda.a[i][j] * lambda.b[j][o[t + 1]] * beta[t + 1][j];
				xsi[t][i][j] = numerator / denominator;
			}
		}
	}

	vector<vector<double>> gamma(T, vector<double>(N, 0.0));
	for (int t = 0; t < T; ++t)
	{
		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j < N; ++j)
			{
				gamma[t][i] += xsi[t][i][j];
			}
		}
	}

	for (int i = 0; i < N; ++i)
	{
		double denominator = 0.0;
		for (int t = 0; t < T; ++t)
		{
			denominator += gamma[t][i];
		}
		for (int j = 0; j < N; ++j)
		{
			double numerator = 0.0;
			for (int t = 0; t < T; ++t)
			{
				numerator += xsi[t][i][j];
			}
			lambda.a[i][j] = numerator / denominator;
		}
	}

	for (int i = 0; i < N; ++i)
	{
		double denominator = 0.0;
		for (int t = 0; t < T; ++t)
		{
			denominator += gamma[t][i];
		}
		for (int j = 0; j < M; ++j)
		{
			double numerator = 0.0;
			for (int t = 0; t < T; ++t)
			{
				if (o[t] == j)
				{
					numerator += gamma[t][i];
				}
			}
			lambda.b[i][j] = numerator / denominator;
		}
	}

	for (int i = 0; i < N; ++i)
	{
		lambda.pi[i] = gamma[0][i];
	}
}
