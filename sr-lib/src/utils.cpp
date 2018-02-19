#include <fstream>
#include <limits>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

namespace Utils
{
	template <typename T>
	vector<T> get_vector_from_stream(stringstream &stream, char delimiter)
	{
		vector<T> vec;

		string token;
		while (getline(stream, token, delimiter))
		{
			T value;
			stringstream(token) >> value;
			vec.push_back(value);
		}

		return vec;
	}

	template <typename T>
	string get_string_from_vector(const vector<T> &vec, char delimiter)
	{
		stringstream stream;
		// maximise precision
		stream.precision(numeric_limits<double>::max_digits10);
		stream.setf(ios::scientific);

		for (int i = 0; i < vec.size() - 1; ++i)
		{
			stream << vec[i] << delimiter;
		}
		stream << vec[vec.size() - 1];

		return stream.str();
	}

	template <typename T>
	vector<vector<T>> get_matrix_from_stream(stringstream &stream, char delimiter)
	{
		vector<vector<T>> mat;

		string line;
		while (getline(stream, line, delimiter))
		{
			vector<T> vec = get_vector_from_stream<T>(stringstream(line), ',');
			mat.push_back(vec);
		}

		return mat;
	}

	template <typename T>
	string get_string_from_matrix(const vector<vector<T>> &mat, char delimiter)
	{
		stringstream stream;

		string str;
		for (int i = 0; i < mat.size() - 1; ++i)
		{
			string str = get_string_from_vector<T>(mat[i], ',');
			stream << str << delimiter;
		}
		str = get_string_from_vector<T>(mat[mat.size() - 1], ',');
		stream << str;

		return stream.str();
	}

	template <typename T>
	T get_item_from_file(string filename)
	{
		fstream file(filename, ios::in);

		T value;
		file >> value;

		return value;
	}

	template <typename T>
	void set_item_to_file(const T &item, string filename)
	{
		fstream file(filename, ios::out);

		file << item;
	}

	template <typename T>
	vector<T> get_vector_from_file(string filename)
	{
		fstream file = fstream(filename, ios::in);

		char c;
		stringstream stream;
		while (file.get(c))
		{
			stream << c;
		}
		vector<T> vec = get_vector_from_stream<T>(stream, '\n');

		return vec;
	}

	template <typename T>
	void set_vector_to_file(const vector<T> &vec, string filename)
	{
		fstream file = fstream(filename, ios::out);

		string str = get_string_from_vector<T>(vec, '\n');
		file << str;
	}

	template <typename T>
	vector<vector<T>> get_matrix_from_file(string filename)
	{
		fstream file(filename, ios::in);

		char c;
		stringstream stream;
		while (file.get(c))
		{
			stream << c;
		}
		vector<vector<T>> mat = get_matrix_from_stream<T>(stream, '\n');

		return mat;
	}

	template <typename T>
	void set_matrix_to_file(const vector<vector<T>> &mat, string filename)
	{
		fstream file(filename, ios::out);

		string str = get_string_from_matrix(mat, '\n');
		file << str;
	}
}
