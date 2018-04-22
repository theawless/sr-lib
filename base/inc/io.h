#pragma once

#include <iostream>
#include <limits>
#include <string>
#include <sstream>
#include <vector>

namespace IO
{
	/// Get the item from the stream.
	template <typename T>
	inline T get_item_from_stream(std::istream &stream)
	{
		T item;
		stream >> item;

		return item;
	}

	/// Set the item to a string form.
	template <typename T>
	inline std::string get_string_from_item(const T &item)
	{
		std::stringstream stream;
		// maximise precision
		stream.precision(std::numeric_limits<double>::max_digits10);
		stream.setf(std::ios::scientific);
		stream << item;

		return stream.str();
	}

	/// Get the vector from the stream.
	template <typename T>
	inline std::vector<T> get_vector_from_stream(std::istream &stream, char delim = ',')
	{
		std::vector<T> vec;

		std::string token;
		while (getline(stream, token, delim))
		{
			std::stringstream token_stream(token);
			vec.push_back(get_item_from_stream<T>(token_stream));
		}

		return vec;
	}

	/// Set the vector to a string form.
	template <typename T>
	inline std::string get_string_from_vector(const std::vector<T> &vec, char delim = ',')
	{
		if (vec.empty())
		{
			return std::string();
		}

		std::stringstream stream;

		for (int i = 0; i < vec.size() - 1; ++i)
		{
			stream << get_string_from_item<T>(vec[i]) << delim;
		}
		stream << get_string_from_item<T>(vec[vec.size() - 1]);

		return stream.str();
	}

	/// Get the matrix from the stream.
	template <typename T>
	inline std::vector<std::vector<T>> get_matrix_from_stream(std::istream &stream, char delim_token = ',', char delim_line = '\n')
	{
		std::vector<std::vector<T>> mat;

		std::string line;
		while (getline(stream, line, delim_line))
		{
			std::stringstream line_stream(line);
			mat.push_back(get_vector_from_stream<T>(line_stream, delim_token));
		}

		return mat;
	}

	/// Set the matrix to a string form.
	template <typename T>
	inline std::string get_string_from_matrix(const std::vector<std::vector<T>> &mat, char delim_token = ',', char delim_line = '\n')
	{
		if (mat.empty())
		{
			return std::string();
		}

		std::stringstream stream;

		for (int i = 0; i < mat.size() - 1; ++i)
		{
			stream << get_string_from_vector<T>(mat[i], delim_token) << delim_line;
		}
		stream << get_string_from_vector<T>(mat[mat.size() - 1], delim_token);

		return stream.str();
	}
}
