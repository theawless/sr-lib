#include "config.h"

#include <thread>

using namespace std;

Config::Config(string filename) : filename(filename)
{
}

void Config::load()
{
	vector<vector<string>> mat = Utils::get_matrix_from_file<string>(filename, '=');

	for (int i = 0; i < mat.size(); ++i)
	{
		dict[mat[i][0]] = mat[i][1];
	}
}

void Config::save() const
{
	vector<vector<string>> mat;
	for (map<string, string>::const_iterator it = dict.begin(); it != dict.end(); ++it)
	{
		vector<string> vec{ it->first, it->second };
		mat.push_back(vec);
	}

	Utils::set_matrix_to_file<string>(mat, filename, '=');
}

WordConfig::WordConfig(string filename) : Config(filename)
{
}

int WordConfig::add(string word)
{
	const int current = get_val<int>(word, 0);
	set_val<int>(word, current + 1);

	return current;
}

vector<pair<string, int>> WordConfig::words() const
{
	vector<pair<string, int>> vec;
	for (map<string, string>::const_iterator it = dict.begin(); it != dict.end(); ++it)
	{
		vec.push_back(pair<string, int>(it->first, stoi(it->second)));
	}

	return vec;
}

Parameters::Parameters(string folder, const vector<pair<string, int>> &words, const Config &config) : folder(folder), words(words),
n_thread(config.get_val<int>("n_thread", 4 * std::thread::hardware_concurrency())),
x_frame(config.get_val<int>("x_frame", 300)), x_overlap(config.get_val<int>("x_overlap", 80)),
cepstral(config.get_val<string>("cepstral", "mfc")), n_cepstra(config.get_val<int>("n_cepstra", 12)), n_predict(config.get_val<int>("n_predict", 12)),
q_gain(config.get_val<bool>("q_gain", false)), q_delta(config.get_val<bool>("q_delta", false)), q_accel(config.get_val<bool>("q_accel", false)),
x_codebook(config.get_val<int>("x_codebook", 16)), n_state(config.get_val<int>("n_state", 5)),
n_bakis(config.get_val<int>("n_bakis", 1)), n_retrain(config.get_val<int>("n_retrain", 3))
{
}
