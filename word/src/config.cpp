#include "config.h"

#include <thread>

using namespace std;

istream &operator>>(istream &input, Config &config)
{
	vector<vector<string>> mat = Utils::get_matrix_from_stream<string>(input, '=');

	for (int i = 0; i < mat.size(); ++i)
	{
		config.dict[mat[i][0]] = mat[i][1];
	}

	return input;
}

ostream &operator<<(ostream &output, const Config &config)
{
	vector<vector<string>> mat;

	for (map<string, string>::const_iterator it = config.dict.begin(); it != config.dict.end(); ++it)
	{
		vector<string> vec{ it->first, it->second };
		mat.push_back(vec);
	}
	output << Utils::get_string_from_matrix<string>(mat, '=');

	return output;
}

int WordConfig::add(const string &word)
{
	const int current = get_val<int>(word, 0);
	set_val<int>(word, current + 1);

	return current;
}

vector<string> WordConfig::words() const
{
	vector<string> vec;

	for (map<string, string>::const_iterator it = dict.begin(); it != dict.end(); ++it)
	{
		vec.push_back(it->first);
	}

	return vec;
}

vector<pair<string, int>> WordConfig::utterances() const
{
	vector<pair<string, int>> vec;

	for (map<string, string>::const_iterator it = dict.begin(); it != dict.end(); ++it)
	{
		vec.push_back(pair<string, int>(it->first, stoi(it->second)));
	}

	return vec;
}
