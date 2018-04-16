#pragma once

#include <fstream>
#include <ios>
#include <limits>
#include <string>
#include <sstream>
#include <vector>

namespace Utils
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

	/// Get the item from the given file.
	template <typename T>
	inline T get_item_from_file(const std::string &filename)
	{
		std::ifstream stream(filename, std::ios::binary);

		return get_item_from_stream<T>(stream);
	}

	/// Set the item to the given file.
	template <typename T>
	inline void set_item_to_file(const T &item, const std::string &filename)
	{
		std::ofstream stream(filename, std::ios::binary);

		stream << get_string_from_item<T>(item);
	}

	/// Get the vector from the given file.
	template <typename T>
	inline std::vector<T> get_vector_from_file(const std::string &filename, char delim = '\n')
	{
		std::ifstream stream(filename, std::ios::binary);

		return get_vector_from_stream<T>(stream, delim);
	}

	/// Set the vector to the given file.
	template <typename T>
	inline void set_vector_to_file(const std::vector<T> &vec, const std::string &filename, char delim = '\n')
	{
		std::ofstream stream(filename, std::ios::binary);

		stream << get_string_from_vector<T>(vec, delim);
	}

	/// Get the matrix from the given file.
	template <typename T>
	inline std::vector<std::vector<T>> get_matrix_from_file(const std::string &filename, char delim_token = ',', char delim_line = '\n')
	{
		std::ifstream stream(filename, std::ios::binary);

		return get_matrix_from_stream<T>(stream, delim_token, delim_line);
	}

	/// Set the matrix to the given file.
	template <typename T>
	inline void set_matrix_to_file(const std::vector<std::vector<T>> &mat, const std::string &filename, char delim_token = ',', char delim_line = '\n')
	{
		std::ofstream stream(filename, std::ios::binary);

		stream << get_string_from_matrix(mat, delim_token, delim_line);
	}
}
