﻿#include "model.h"

#include <sstream>

#include "io.h"

using namespace std;

Model::Builder::Builder(int N, int M, int step) :
	N(N), M(M), step(step)
{
}

Model Model::Builder::bakis() const
{
	Model model{ vector<vector<double>>(N, vector<double>(N, 0.0)), vector<vector<double>>(N, vector<double>(M, 1.0 / M)), vector<double>(N, 0.0) };

	for (int i = 0; i < N - step; ++i)
	{
		model.a[i][i] = 1 - step * 0.1;
		for (int j = 0; j < step; ++j)
		{
			model.a[i][i + j + 1] = 0.1;
		}
	}
	for (int i = N - step; i < N; ++i)
	{
		model.a[i][i] = 1 - (N - 1 - i) * 0.1;
		for (int j = 0; j < N - 1 - i; ++j)
		{
			model.a[i][i + j + 1] = 0.1;
		}
	}
	model.pi[0] = 1.0;

	return model;
}

Model Model::Builder::merge(const vector<Model> &models) const
{
	Model model{ vector<vector<double>>(N, vector<double>(N, 0.0)), vector<vector<double>>(N, vector<double>(M, 0.0)), vector<double>(N, 0.0) };

	int Q = models.size();
	for (int i = 0; i < Q; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			for (int k = 0; k < N; ++k)
			{
				model.a[j][k] += models[i].a[j][k];
			}
		}
		for (int j = 0; j < N; ++j)
		{
			for (int k = 0; k < M; ++k)
			{
				model.b[j][k] += models[i].b[j][k];
			}
		}
		for (int j = 0; j < N; ++j)
		{
			model.pi[j] += models[i].pi[j];
		}
	}

	for (int j = 0; j < N; ++j)
	{
		for (int k = 0; k < N; ++k)
		{
			model.a[j][k] /= Q;
		}
	}
	for (int j = 0; j < N; ++j)
	{
		for (int k = 0; k < M; ++k)
		{
			model.b[j][k] /= Q;
		}
	}
	for (int i = 0; i < N; ++i)
	{
		model.pi[i] /= Q;
	}

	return model;
}

bool Model::empty() const
{
	return a.empty() || b.empty() || pi.empty();
}

istream &operator>>(istream &input, Model &model)
{
	string line;
	stringstream stream;

	// pi
	std::getline(input, line);
	std::getline(input, line);
	stream << line;
	model.pi = IO::get_vector_from_stream<double>(stream);

	// a
	std::getline(input, line);
	stream = std::stringstream();
	while (std::getline(input, line) && line != "b")
	{
		stream << line << '\n';
	}
	model.a = IO::get_matrix_from_stream<double>(stream);

	// b
	stream = std::stringstream();
	while (std::getline(input, line))
	{
		stream << line << '\n';
	}
	model.b = IO::get_matrix_from_stream<double>(stream);

	return input;
}

ostream &operator<<(ostream &output, const Model &model)
{
	output << "pi" << '\n';
	output << IO::get_string_from_vector<double>(model.pi) << '\n';
	output << "a" << '\n';
	output << IO::get_string_from_matrix<double>(model.a) << '\n';
	output << "b" << '\n';
	output << IO::get_string_from_matrix<double>(model.b) << '\n';

	return output;
}
