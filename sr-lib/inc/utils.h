#pragma once

#include <string>
#include <vector>

#define RECORD_FOLDER ".\\Record\\"
#define DEFAULT_DURATION 0.8

/// Captures the audio for given duration and saves it in the given file.
void capture_audio(std::string filename, double duration);

/// Gets the vector from the given file.
std::vector<double> get_vector_from_txt(std::string filename);

/// Sets the vector to the given file.
void set_vector_to_txt(const std::vector<double> &vec, std::string filename);

/// Gets the matrix from the given file.
std::vector<std::vector<double>> get_matrix_from_csv(std::string filename);

/// Saves the vector of vectors coefficients to a CSV file.
void set_matrix_to_csv(const std::vector<std::vector<double>> &mat, std::string filename);

/// Divides the given vector into chunks of given size.
std::vector<std::vector<double>> get_vector_chunks(const std::vector<double> &unchunked, int chunk_size);

/// Gives a string representation of number with leading zeroes.
std::string pad_number(int num, int max_num);
