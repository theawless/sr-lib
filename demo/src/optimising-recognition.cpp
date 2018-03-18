#include <chrono>
#include <limits>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include "config.h"
#include "logger.h"
#include "tester.h"
#include "trainer.h"

using namespace std;

pair<int, int> test(const Parameters &parameters, const vector<pair<string, int>> &saved_words)
{
	Trainer trainer(parameters);
	Tester tester(parameters);

	vector<int> n_hits(parameters.words.size(), 0), n_errs(parameters.words.size(), 0);
	for (int i = 0; i < parameters.words.size(); ++i)
	{
		for (int j = parameters.words[i].second; j < saved_words[i].second; ++j)
		{
			string test_filename = saved_words[i].first + "_" + to_string(j);
			string word = tester.test(test_filename);

			if (word == saved_words[i].first)
			{
				n_hits[i]++;
			}
			if (word == "")
			{
				n_errs[i]++;
			}
		}
	}

	return pair<int, int>(accumulate(n_hits.begin(), n_hits.end(), 0), accumulate(n_errs.begin(), n_errs.end(), 0));
}

int main()
{
	const string folder = "B:\\record\\digit_0.8_2\\";
	Config config(folder + "sr-lib.config");
	WordConfig word_config(folder + "words.config");
	config.load(); word_config.load();

	vector<pair<string, int>> saved_words = word_config.words();
	vector<pair<string, int>> new_words(saved_words);
	for (int i = 0; i < new_words.size(); ++i)
	{
		new_words[i].second *= 0.70;
	}

	// exhaustive search, use fewer options
	vector<int> n_cepstra_values = { 12, 16, 20, 24 };
	vector<int> n_predict_values = { 8, 10, 13, 16 };
	vector<bool> q_gain_values = { false, true };
	vector<bool> q_delta_values = { false, true };
	vector<bool> q_accel_values = { false, true };
	vector<int> x_codebook_values = { 8, 16, 32, 64 };
	vector<int> n_state_values = { 5, 8, 10, 12 };
	vector<int> n_bakis_values = { 1, 2, 3, 4 };
	vector<int> n_retrain_values = { 3, 4, 5 };
	vector<string> cepstral_values = { "mfc", "lpc" };

	int n_hits = numeric_limits<int>::min();

	for (int n_cepstra : n_cepstra_values)
	{
		for (int n_predict : n_predict_values)
		{
			for (bool q_gain : q_gain_values)
			{
				for (bool q_delta : q_delta_values)
				{
					for (bool q_accel : q_accel_values)
					{
						for (int x_codebook : x_codebook_values)
						{
							for (int n_state : n_state_values)
							{
								for (int n_bakis : n_bakis_values)
								{
									for (int n_retrain : n_retrain_values)
									{
										for (string cepstral : cepstral_values)
										{
											if (cepstral == "mfc" && n_predict != n_predict_values[0])
											{
												continue;
											}
											if (!q_delta && q_accel)
											{
												continue;
											}

											config.set_val<int>("n_cepstra", n_cepstra);
											config.set_val<int>("n_predict", n_predict);
											config.set_val<bool>("q_gain", q_gain);
											config.set_val<bool>("q_delta", q_delta);
											config.set_val<bool>("q_accel", q_accel);
											config.set_val<int>("x_codebook", x_codebook);
											config.set_val<int>("n_state", n_state);
											config.set_val<int>("n_bakis", n_bakis);
											config.set_val<int>("n_retrain", n_retrain);
											config.set_val<string>("cepstral", cepstral);

											string temp_dir = folder + "\\__temp__\\";
											string wav_files = folder + "*.wav";
											string config_files = folder + "*.config";
											string all_files = folder + "*.*";
											system(string("mkdir " + temp_dir).c_str());
											system(string("move >nul " + wav_files + " " + temp_dir).c_str());
											system(string("move >nul " + config_files + " " + temp_dir).c_str());
											system(string("erase /q " + all_files).c_str());
											system(string("move >nul " + temp_dir + "* " + folder).c_str());
											system(string("rmdir " + temp_dir).c_str());

											Logger::info
											(
												"Config:", "n_cepstra", n_cepstra, "n_predict", n_predict, "q_gain", q_gain, "q_delta", q_delta, "q_accel", q_accel,
												"x_codebook", x_codebook, "n_state", n_state, "n_bakis", n_bakis, "n_retrain", n_retrain, "cepstral", cepstral
											);
											Parameters parameters(folder, new_words, config);

											chrono::steady_clock::time_point start = chrono::steady_clock::now();
											pair<int, int> result = test(parameters, saved_words);
											chrono::steady_clock::time_point end = chrono::steady_clock::now();

											chrono::seconds time = chrono::duration_cast<chrono::seconds>(end - start);
											Logger::info("Time taken :", time.count(), "n_hits", result.first, "n_errs", result.second);

											if (n_hits < result.first)
											{
												n_hits = result.first;
												config.save();
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	cin.get();
}
