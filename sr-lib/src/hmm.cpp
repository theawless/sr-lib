#include "hmm.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <tuple>
#include <vector>

#define OPTIMISE_CONVERGENCE_THRESHOLD 1.001
#define OPTIMISE_CONVERGENCE_MAX_ITERATIONS 50
#define MINIMUM_PROBABILITY 10e-60

using namespace std;

pair<double, vector<vector<double>>> forward(const Model &lambda, const vector<int> &o) {
	int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();

	vector<vector<double>> alpha(T, vector<double>(N, 0.0));
	for (int i = 0; i < N; ++i) {
		alpha[0][i] = lambda.pi[i] * lambda.b[i][o[0]];
	}
	for (int t = 0; t < T - 1; ++t) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				alpha[t + 1][i] += alpha[t][j] * lambda.a[j][i];
			}
			alpha[t + 1][i] *= lambda.b[i][o[t + 1]];
		}
	}

	double P = 0.0;
	for (int i = 0; i < N; ++i) {
		P += alpha[T - 1][i];
	}

	return pair<double, vector<vector<double>>>(P, alpha);
}

pair<double, vector<vector<double>>> forward_scaled(const Model &lambda, const vector<int> &o) {
	int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();
	double C = 0;

	vector<vector<double>> alpha(T, vector<double>(N, 0.0));
	for (int i = 0; i < N; ++i) {
		alpha[0][i] = lambda.pi[i] * lambda.b[i][o[0]];
		C += alpha[0][i];
	}
	for (int i = 0; i < N; ++i) {
		alpha[0][i] /= C;
	}

	for (int t = 0; t < T - 1; ++t) {
		C = 0;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				alpha[t + 1][i] += alpha[t][j] * lambda.a[j][i];
			}
			alpha[t + 1][i] *= lambda.b[i][o[t + 1]];
			C += alpha[t + 1][i];
		}
		for (int i = 0; i < N; ++i) {
			alpha[t + 1][i] /= C;
		}
	}

	double P = 0.0;
	for (int i = 0; i < N; ++i) {
		P += alpha[T - 1][i];
	}

	return pair<double, vector<vector<double>>>(P, alpha);
}

/// Calculates beta values.
static vector<vector<double>> backward(const Model &lambda, const vector<int> &o) {
	int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();

	vector<vector<double>> beta(T, vector<double>(N, 0.0));
	for (int i = 0; i < N; ++i) {
		beta[T - 1][i] = 1;
	}
	for (int t = T - 2; t >= 0; --t) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				beta[t][i] += beta[t + 1][j] * lambda.a[i][j] * lambda.b[j][o[t + 1]];
			}
		}
	}

	return beta;
}

/// Calculates beta values.
static vector<vector<double>> backward_scaled(const Model &lambda, const vector<int> &o) {
	int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();
	double C = 0;

	vector<vector<double>> beta(T, vector<double>(N, 0.0));
	for (int i = 0; i < N; ++i) {
		beta[T - 1][i] = 1;
		C += beta[T - 1][i];
	}
	for (int i = 0; i < N; ++i) {
		beta[T - 1][i] /= C;
	}

	for (int t = T - 2; t >= 0; --t) {
		C = 0;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				beta[t][i] += beta[t + 1][j] * lambda.a[i][j] * lambda.b[j][o[t + 1]];
			}
			C += beta[t][i];
		}
		for (int i = 0; i < N; ++i) {
			beta[t][i] /= C;
		}
	}

	return beta;
}

/// Calculates P* and optimal states.
static pair<double, vector<int>> viterbi(const Model &lambda, const vector<int> &o) {
	int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();

	vector<int> psi(T, 0.0);
	vector<vector<double>> delta(T, vector<double>(N, 0.0));
	for (int i = 0; i < N; ++i) {
		delta[0][i] = lambda.pi[i] * lambda.b[i][o[0]];
	}
	for (int t = 0; t < T - 1; ++t) {
		for (int i = 0; i < N; ++i) {
			delta[t + 1][i] = numeric_limits<double>::min();
			for (int j = 0; j < N; ++j) {
				double current_max_delta = delta[t][j] * lambda.a[j][i];
				if (delta[t + 1][i] < current_max_delta) {
					delta[t + 1][i] = current_max_delta;
					psi[i] = j;
				}
			}
			delta[t + 1][i] *= lambda.b[i][o[t + 1]];
		}
	}

	vector<int> q(T, 0.0);
	q[T - 1] = max_element(delta[T - 1].begin(), delta[T - 1].end()) - delta[T - 1].begin();
	for (int i = T - 2; i >= 0; --i) {
		q[i] = psi[q[i + 1]];
	}
	double P_star = delta[T - 1][q[T - 1]];

	return pair<double, vector<int>>(P_star, q);
}

/// Calculates P* and optimal states.
static pair<double, vector<int>> viterbi_logged(const Model &lambda, const vector<int> &o) {
	int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();

	vector<int> psi(T, 0.0);
	vector<vector<double>> delta(T, vector<double>(N, 0.0));
	for (int i = 0; i < N; ++i) {
		double temp = lambda.pi[i];
		if (temp == 0) {
			temp = MINIMUM_PROBABILITY;
		}
		delta[0][i] = log(temp) + log(lambda.b[i][o[0]]);
	}
	for (int t = 0; t < T - 1; ++t) {
		for (int i = 0; i < N; ++i) {
			delta[t + 1][i] = numeric_limits<double>::min();
			for (int j = 0; j < N; ++j) {
				double current_max_delta = delta[t][j] + log(lambda.a[j][i]);
				if (delta[t + 1][i] < current_max_delta) {
					delta[t + 1][i] = current_max_delta;
					psi[i] = j;
				}
			}
			delta[t + 1][i] += lambda.b[i][o[t + 1]];
		}
	}

	vector<int> q(T, 0.0);
	q[T - 1] = max_element(delta[T - 1].begin(), delta[T - 1].end()) - delta[T - 1].begin();
	for (int i = T - 2; i >= 0; --i) {
		q[i] = psi[q[i + 1]];
	}
	double P_star = delta[T - 1][q[T - 1]];

	return pair<double, vector<int>>(P_star, q);
}

/// Improves Model by using xi and gamma values.
static Model restimate(const Model &lambda, const vector<int> &o) {
	int M = lambda.b[0].size(), N = lambda.b.size(), T = o.size();

	vector<vector<double>> alpha = forward(lambda, o).second;
	vector<vector<double>> beta = backward(lambda, o);
	vector<vector<vector<double>>> xsi(T, vector<vector<double>>(N, vector<double>(N, 0.0)));
	for (int t = 0; t < T - 1; ++t) {
		double denominator = 0.0;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				denominator += alpha[t][i] * lambda.a[i][j] * lambda.b[j][o[t + 1]] * beta[t + 1][j];
			}
		}
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				double numerator = alpha[t][i] * lambda.a[i][j] * lambda.b[j][o[t + 1]] * beta[t + 1][j];
				xsi[t][i][j] = numerator / denominator;
			}
		}
	}

	vector<vector<double>> gamma(T, vector<double>(N, 0.0));
	for (int t = 0; t < T; ++t) {
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				gamma[t][i] += xsi[t][i][j];
			}
		}
	}

	Model new_lambda;

	new_lambda.a = vector<vector<double>>(N, vector<double>(N, 0.0));
	for (int i = 0; i < N; ++i) {
		double denominator = 0.0;
		for (int t = 0; t < T; ++t) {
			denominator += gamma[t][i];
		}
		for (int j = 0; j < N; ++j) {
			double numerator = 0.0;
			for (int t = 0; t < T; ++t) {
				numerator += xsi[t][i][j];
			}
			new_lambda.a[i][j] = numerator / denominator;
		}
	}

	new_lambda.b = vector<vector<double>>(N, vector<double>(M, 0.0));
	for (int i = 0; i < N; ++i) {
		double denominator = 0.0;
		for (int t = 0; t < T; ++t) {
			denominator += gamma[t][i];
		}
		for (int j = 0; j < M; ++j) {
			double numerator = 0.0;
			for (int t = 0; t < T; ++t) {
				if (o[t] == j) {
					numerator += gamma[t][i];
				}
			}
			new_lambda.b[i][j] = numerator / denominator;
		}
	}

	new_lambda.pi = vector<double>(N, 0.0);
	for (int i = 0; i < N; ++i) {
		new_lambda.pi[i] = gamma[0][i];
	}

	return new_lambda;
}

Model bakis(int N, int M) {
	Model lambda;

	lambda.a = vector<vector<double>>(N, vector<double>(N, 0.0));
	for (int i = 0; i < N - 1; ++i) {
		lambda.a[i][i] = 0.9;
		lambda.a[i][i + 1] = 0.1;
	}
	lambda.a[N - 1][N - 1] = 1;

	lambda.b = vector<vector<double>>(N, vector<double>(M, 1.0 / M));

	lambda.pi = vector<double>(N, 0.0);
	lambda.pi[0] = 1.0;

	return lambda;
}

Model merge(const vector<Model> &lambdas) {
	int M = lambdas[0].b[0].size(), N = lambdas[0].b.size(), Q = lambdas.size();
	Model lambda;
	lambda.a = vector<vector<double>>(N, vector<double>(N, 0.0));
	lambda.b = vector<vector<double>>(N, vector<double>(M, 0.0));
	lambda.pi = vector<double>(N, 0.0);

	for (int i = 0; i < Q; ++i) {
		for (int j = 0; j < N; ++j) {
			for (int k = 0; k < N; ++k) {
				lambda.a[j][k] += lambdas[i].a[j][k];
			}
		}
		for (int j = 0; j < N; ++j) {
			for (int k = 0; k < M; ++k) {
				lambda.b[j][k] += lambdas[i].b[j][k];
			}
		}
		for (int j = 0; j < N; ++j) {
			lambda.pi[j] += lambdas[i].pi[j];
		}
	}

	for (int j = 0; j < N; ++j) {
		for (int k = 0; k < N; ++k) {
			lambda.a[j][k] /= Q;
		}
	}
	for (int j = 0; j < N; ++j) {
		for (int k = 0; k < M; ++k) {
			lambda.b[j][k] /= Q;
		}
	}
	for (int i = 0; i < N; ++i) {
		lambda.pi[i] /= Q;
	}

	return lambda;
}

/// Tweak values of lambda.
static Model tweak(const Model &old_lambda) {
	Model lambda(old_lambda);
	int M = lambda.b[0].size(), N = lambda.b.size();

	for (int i = 0; i < N; ++i) {
		double dummy = MINIMUM_PROBABILITY;
		for (int j = 0; j < N; ++j) {
			if (lambda.a[i][j] != 0) {
				dummy = min(dummy, lambda.a[i][j] / 10);
			}
		}

		int count = 0;
		for (int j = 0; j < N; ++j) {
			if (lambda.a[i][j] == 0) {
				lambda.a[i][j] = dummy;
				count++;
			}
		}

		int max_j = 0;
		for (int j = 0; j < N; ++j) {
			if (lambda.a[i][j] > lambda.a[i][max_j]) {
				max_j = j;
			}
		}
		lambda.a[i][max_j] -= count * dummy;
	}

	for (int i = 0; i < N; ++i) {
		double dummy = MINIMUM_PROBABILITY;
		for (int j = 0; j < M; ++j) {
			if (lambda.b[i][j] != 0) {
				dummy = min(dummy, lambda.b[i][j] / 10);
			}
		}

		int count = 0;
		for (int j = 0; j < M; ++j) {
			if (lambda.b[i][j] == 0) {
				lambda.b[i][j] = dummy;
				count++;
			}
		}

		int max_j = 0;
		for (int j = 0; j < M; ++j) {
			if (lambda.b[i][j] > lambda.b[i][max_j]) {
				max_j = j;
			}
		}
		lambda.b[i][max_j] -= count * dummy;
	}

	return lambda;
}

Model optimise(const Model &old_lambda, const vector<int> &o) {
	double old_P_star;
	int iteration = 0;
	Model lambda = tweak(old_lambda);
	double P_star = viterbi_logged(lambda, o).first;

	do {
		iteration += 1;
		old_P_star = P_star;
		cout << "Restimate lambda: iteration: " << iteration << ", P* is: " << P_star << endl;

		lambda = restimate(lambda, o);
		lambda = tweak(lambda);
		P_star = viterbi_logged(lambda, o).first;
	} while (P_star / old_P_star > OPTIMISE_CONVERGENCE_THRESHOLD && iteration < OPTIMISE_CONVERGENCE_MAX_ITERATIONS);

	return lambda;
}
