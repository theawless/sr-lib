#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace Utils
{
	/// Gets the vector from the stream.
	template <typename T>
	inline std::vector<T> get_vector_from_stream(std::stringstream &stream, char delimiter);

	/// Sets the vector to a string form.
	template <typename T>
	inline std::string get_string_from_vector(const std::vector<T> &vec, char delimiter);

	/// Gets the matrix from the stream.
	template <typename T>
	inline std::vector<std::vector<T>> get_matrix_from_stream(std::stringstream &stream, char delimiter);

	/// Sets the matrix to a string form.
	template <typename T>
	inline std::string get_string_from_matrix(const std::vector<std::vector<T>> &mat, char delimiter);

	/// Gets the item from the given file.
	template <typename T>
	inline T get_item_from_file(std::string filename);

	/// Sets the item to the given file.
	template <typename T>
	inline void set_item_to_file(const T &item, std::string filename);

	/// Gets the vector from the given file.
	template <typename T>
	inline static std::vector<T> get_vector_from_file(std::string filename);

	/// Sets the vector to the given file.
	template <typename T>
	inline static void set_vector_to_file(const std::vector<T> &vec, std::string filename);

	/// Gets the matrix from the given file.
	template <typename T>
	inline static std::vector<std::vector<T>> get_matrix_from_file(std::string filename);

	/// Sets the matrix to the given file.
	template <typename T>
	inline static void set_matrix_to_file(const std::vector<std::vector<T>> &mat, std::string filename);
};

// For making template functions work.
#include "../src/utils.cpp"