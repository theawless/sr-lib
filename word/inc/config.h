#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "utils.h"

struct Config
{
public:
	int N = 5;
	int M = 16;
	int n_retrain = 3;

	std::string folder;
	std::vector<std::string> audio_names;
	std::vector<int> n_utterances;

	/// Constructor.
	Config()
	{
	}

	/// Loads the config file.
	Config(const std::string filename, const std::string folder) :
		folder(folder), audio_names(std::vector<std::string>()), n_utterances(std::vector<int>())
	{
		std::vector<std::vector<std::string>> config = Utils::get_matrix_from_file<std::string>(folder + filename);
		for (int i = 0; i < config.size(); ++i)
		{
			audio_names.push_back(config[i][0]);
			n_utterances.push_back(std::stod(config[i][1]));
		}
	}
};
