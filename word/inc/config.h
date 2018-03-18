#pragma once

#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "utils.h"

class Config
{
private:
	const std::string filename;

protected:
	std::map<std::string, std::string> dict;

public:
	/// Constructor.
	Config(std::string filename);

	/// Loads the config.
	void load();

	/// Saves the config.
	void save() const;

	/// Gets the value from key.
	template<typename T>
	inline T get_val(std::string key, T fail) const
	{
		if (dict.find(key) == dict.end())
		{
			return fail;
		}

		T val;
		std::stringstream(dict.at(key)) >> val;

		return val;
	}

	/// Sets the value for key.
	template<typename T>
	inline void set_val(std::string key, T val)
	{
		std::stringstream stream;
		stream << val;

		dict[key] = stream.str();
	}
};

class WordConfig : public Config
{
public:
	/// Constructor.
	WordConfig(std::string filename);

	/// Adds word.
	int add(std::string word);

	/// Get words.
	std::vector<std::pair<std::string, int>> words() const;
};

struct Parameters
{
public:
	const std::string folder;
	const std::vector<std::pair<std::string, int>> words;
	const int n_thread;
	const int x_frame;
	const int x_overlap;
	const std::string cepstral;
	const int n_cepstra;
	const int n_predict;
	const bool q_gain;
	const bool q_delta;
	const bool q_accel;
	const int x_codebook;
	const int n_state;
	const int n_bakis;
	const int n_retrain;

	/// Constructor.
	Parameters(std::string folder, const std::vector<std::pair<std::string, int>> &words, const Config &config);
};
