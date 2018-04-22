#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "io.h"

namespace FileIO
{
	/// Get the item from the given file.
	template <typename T>
	inline T get_item_from_file(const std::string &filename)
	{
		std::ifstream stream(filename, std::ios::binary);

		return IO::get_item_from_stream<T>(stream);
	}

	/// Set the item to the given file.
	template <typename T>
	inline void set_item_to_file(const T &item, const std::string &filename)
	{
		std::ofstream stream(filename, std::ios::binary);

		stream << IO::get_string_from_item<T>(item);
	}

	/// Get the vector from the given file.
	template <typename T>
	inline std::vector<T> get_vector_from_file(const std::string &filename, char delim = '\n')
	{
		std::ifstream stream(filename, std::ios::binary);

		return IO::get_vector_from_stream<T>(stream, delim);
	}

	/// Set the vector to the given file.
	template <typename T>
	inline void set_vector_to_file(const std::vector<T> &vec, const std::string &filename, char delim = '\n')
	{
		std::ofstream stream(filename, std::ios::binary);

		stream << IO::get_string_from_vector<T>(vec, delim);
	}

	/// Get the matrix from the given file.
	template <typename T>
	inline std::vector<std::vector<T>> get_matrix_from_file(const std::string &filename, char delim_token = ',', char delim_line = '\n')
	{
		std::ifstream stream(filename, std::ios::binary);

		return IO::get_matrix_from_stream<T>(stream, delim_token, delim_line);
	}

	/// Set the matrix to the given file.
	template <typename T>
	inline void set_matrix_to_file(const std::vector<std::vector<T>> &mat, const std::string &filename, char delim_token = ',', char delim_line = '\n')
	{
		std::ofstream stream(filename, std::ios::binary);

		stream << IO::get_string_from_matrix(mat, delim_token, delim_line);
	}
}
