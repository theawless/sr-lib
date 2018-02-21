#pragma once

#include <fstream>
#include <ios>
#include <limits>
#include <string>
#include <sstream>
#include <vector>

namespace Utils
{
	/// Gets the vector from the stream.
	template <typename T>
	inline std::vector<T> get_vector_from_stream(std::stringstream &stream, char delimiter)
	{
		std::vector<T> vec;

		std::string token;
		while (getline(stream, token, delimiter))
		{
			T value;
			std::stringstream(token) >> value;
			vec.push_back(value);
		}

		return vec;
	}

	/// Sets the vector to a string form.
	template <typename T>
	inline std::string get_string_from_vector(const std::vector<T> &vec, char delimiter)
	{
		std::stringstream stream;
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

	/// Gets the matrix from the stream.
	template <typename T>
	inline std::vector<std::vector<T>> get_matrix_from_stream(std::stringstream &stream, char delimiter)
	{
		std::vector<std::vector<T>> mat;

		std::string line;
		while (getline(stream, line, delimiter))
		{
			std::vector<T> vec = get_vector_from_stream<T>(std::stringstream(line), ',');
			mat.push_back(vec);
		}

		return mat;
	}

	/// Sets the matrix to a string form.
	template <typename T>
	inline std::string get_string_from_matrix(const std::vector<std::vector<T>> &mat, char delimiter)
	{
		std::stringstream stream;

		std::string str;
		for (int i = 0; i < mat.size() - 1; ++i)
		{
			std::string str = get_string_from_vector<T>(mat[i], ',');
			stream << str << delimiter;
		}
		str = get_string_from_vector<T>(mat[mat.size() - 1], ',');
		stream << str;

		return stream.str();
	}

	/// Gets the item from the given file.
	template <typename T>
	inline T get_item_from_file(std::string filename)
	{
		std::fstream file(filename, ios::in);

		T value;
		file >> value;

		return value;
	}

	/// Sets the item to the given file.
	template <typename T>
	inline void set_item_to_file(const T &item, std::string filename)
	{
		std::fstream file(filename, ios::out);

		file << item;
	}

	/// Gets the vector from the given file.
	template <typename T>
	inline static std::vector<T> get_vector_from_file(std::string filename)
	{
		std::fstream file = fstream(filename, ios::in);

		char c;
		std::stringstream stream;
		while (file.get(c))
		{
			stream << c;
		}
		std::vector<T> vec = get_vector_from_stream<T>(stream, '\n');

		return vec;
	}

	/// Sets the vector to the given file.
	template <typename T>
	inline static void set_vector_to_file(const std::vector<T> &vec, std::string filename)
	{
		std::fstream file = fstream(filename, ios::out);

		std::string str = get_string_from_vector<T>(vec, '\n');
		file << str;
	}

	/// Gets the matrix from the given file.
	template <typename T>
	inline static std::vector<std::vector<T>> get_matrix_from_file(std::string filename)
	{
		std::fstream file(filename, ios::in);

		char c;
		std::stringstream stream;
		while (file.get(c))
		{
			stream << c;
		}
		std::vector<std::vector<T>> mat = get_matrix_from_stream<T>(stream, '\n');

		return mat;
	}

	/// Sets the matrix to the given file.
	template <typename T>
	inline static void set_matrix_to_file(const std::vector<std::vector<T>> &mat, std::string filename)
	{
		std::fstream file(filename, ios::out);

		std::string str = get_string_from_matrix(mat, '\n');
		file << str;
	}
}
