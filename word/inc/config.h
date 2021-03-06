#pragma once

#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "io.h"

/// Config keys
/// q_cache      (bool):    whether cached training files should be used
/// n_thread     (int):     number of threads used for parallel execution
/// q_trim       (bool):    whether the samples should be trimmed for background noise
/// x_frame      (int):     number of samples in a frame
/// x_overlap    (int):     number of samples to be overlapped while framing
/// cepstra      (string):  "mfc" or "lpc" variants of feature generation
/// n_cepstra    (int):     number of features
/// q_gain       (bool):    whether gain term should be added to features
/// q_delta      (bool):    whether delta terms should be added to features
/// q_accel      (bool):    whether accel terms should be added to features
/// x_codebook   (int):     size of codebook
/// n_state      (int):     number of states in HMM
/// n_bakis      (int):     connentedness of initial bakis model for HMM
/// n_retrain    (int):     number of times each model should be trained
/// n_gram       (int):     number of previous words to be considered for prediction
/// q_dfa        (bool):    command based word prediction or probability based
/// gram_weight  (double):  linear weight for the final scoring with recognition result
/// cutoff_score (double):  cutoff for final score

struct Config
{
protected:
	std::map<std::string, std::string> dict;

public:
	/// Get the value from key.
	template<typename T>
	inline T get_val(const std::string &key, const T &fail) const
	{
		T val = fail;

		if (dict.find(key) != dict.end())
		{
			std::stringstream stream(dict.at(key));
			stream << std::boolalpha;
			val = IO::get_item_from_stream<T>(stream);
		}

		return val;
	}

	/// Set the value for key.
	template<typename T>
	inline void set_val(const std::string &key, const T &val)
	{
		dict[key] = IO::get_string_from_item<T>(val);
	}

	/// Operator for loading.
	friend std::istream &operator>>(std::istream &input, Config &config)
	{
		std::vector<std::vector<std::string>> mat = IO::get_matrix_from_stream<std::string>(input, '=');

		for (int i = 0; i < mat.size(); ++i)
		{
			config.dict[mat[i][0]] = mat[i][1];
		}

		return input;
	}

	/// Operator for saving.
	friend std::ostream &operator<<(std::ostream &output, const Config &config)
	{
		std::vector<std::vector<std::string>> mat;

		for (std::map<std::string, std::string>::const_iterator it = config.dict.begin(); it != config.dict.end(); ++it)
		{
			std::vector<std::string> vec{ it->first, it->second };
			mat.push_back(vec);
		}
		output << IO::get_string_from_matrix<std::string>(mat, '=');

		return output;
	}
};
