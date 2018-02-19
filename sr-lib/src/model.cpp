#include "model.h"

#include "utils.h"

using namespace std;

bool Model::empty()
{
	return a.empty();
}

Model Model::bakis(int N, int M)
{
	Model model;

	model.a = vector<vector<double>>(N, vector<double>(N, 0.0));
	for (int i = 0; i < N - 1; ++i)
	{
		model.a[i][i] = 0.9;
		model.a[i][i + 1] = 0.1;
	}
	model.a[N - 1][N - 1] = 1;

	model.b = vector<vector<double>>(N, vector<double>(M, 1.0 / M));

	model.pi = vector<double>(N, 0.0);
	model.pi[0] = 1.0;

	return model;
}

Model Model::merge(const vector<Model> &models)
{
	int M = models[0].b[0].size(), N = models[0].b.size(), Q = models.size();
	Model model;
	model.a = vector<vector<double>>(N, vector<double>(N, 0.0));
	model.b = vector<vector<double>>(N, vector<double>(M, 0.0));
	model.pi = vector<double>(N, 0.0);

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

istream &operator>>(istream &input, Model &model)
{
	char c;
	stringstream stream;

	input.get(); input.get(); input.get();
	while (input.get(c) && c != 'a')
	{
		stream << c;
	}
	model.pi = Utils::get_vector_from_stream<double>(stream, ',');

	input.get(); stream.str(""); stream.clear();
	while (input.get(c) && c != 'b')
	{
		stream << c;
	}
	model.a = Utils::get_matrix_from_stream<double>(stream, '\n');

	input.get(); stream.str(""); stream.clear();
	while (input.get(c))
	{
		stream << c;
	}
	model.b = Utils::get_matrix_from_stream<double>(stream, '\n');

	return input;
}

ostream &operator<<(ostream &output, const Model &model)
{
	output << "pi" << endl;
	output << Utils::get_string_from_vector<double>(model.pi, ',') << endl;
	output << "a" << endl;
	output << Utils::get_string_from_matrix<double>(model.a, '\n') << endl;
	output << "b" << endl;
	output << Utils::get_string_from_matrix<double>(model.b, '\n') << endl;

	return output;
}
