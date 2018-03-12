#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "utils.h"

struct Config
{
public:
	int N = 5;
	int M = 16;
	int n_retrain = 3;
	int n_thread = 4;

	std::string folder;
	std::vector<std::string> audio_names;
	std::vector<int> n_utterances;

	/// Constructor that loads the config file.
	Config(std::string folder) :
		folder(folder), audio_names(std::vector<std::string>()), n_utterances(std::vector<int>())
	{
	}

	/// Load from filename.
	void load(std::string filename)
	{
		std::vector<std::vector<std::string>> mat = Utils::get_matrix_from_file<std::string>(folder + filename);
		for (int i = 0; i < mat.size(); ++i)
		{
			audio_names.push_back(mat[i][0]);
			n_utterances.push_back(std::stoi(mat[i][1]));
		}
	}

	/// Save the config.
	void save(std::string filename)
	{
		std::vector<std::vector<std::string>> mat;
		for (int i = 0; i < audio_names.size(); ++i)
		{
			std::vector<std::string> vec;
			vec.push_back(audio_names[i]);
			vec.push_back(std::to_string(n_utterances[i]));

			mat.push_back(vec);
		}
		Utils::set_matrix_to_file<std::string>(mat, folder + filename);
	}

	/// Adds the word and returns its index.
	int add_word(std::string word)
	{
		int index = 0;
		std::vector<std::string>::iterator it = find(audio_names.begin(), audio_names.end(), word);
		if (it != audio_names.end())
		{
			index = n_utterances[std::distance(audio_names.begin(), it)]++;
		}
		else
		{
			audio_names.push_back(word);
			n_utterances.push_back(1);
		}

		return index;
	}
};
