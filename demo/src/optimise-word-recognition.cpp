#include <chrono>
#include <memory>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include "config.h"
#include "logger.h"
#include "tester.h"
#include "trainer.h"
#include "utils.h"

using namespace std;

const string folder = "B:\\record\\digit_0.8_2\\";

pair<int, int> test(const Config &config)
{
	const string word_config_filename = folder + "words.config";
	const WordConfig word_config = Utils::get_item_from_file<WordConfig>(word_config_filename);

	const vector<pair<string, int>> all_utterances = word_config.utterances();
	vector<pair<string, int>> train_utterances(all_utterances);
	for (int i = 0; i < train_utterances.size(); ++i)
	{
		train_utterances[i].second *= 0.70;
	}

	const unique_ptr<ModelTrainer> model_trainer = ModelTrainer::Builder(folder, train_utterances, config).build();
	const unique_ptr<Tester> tester = Tester::Builder(folder, word_config.words(), config).build();

	vector<int> n_hits(all_utterances.size(), 0), n_errs(all_utterances.size(), 0);
	for (int i = 0; i < all_utterances.size(); ++i)
	{
		for (int j = train_utterances[i].second; j < all_utterances[i].second; ++j)
		{
			const string test_filename = all_utterances[i].first + "_" + to_string(j);
			const string word = tester->test(test_filename);

			if (word == all_utterances[i].first)
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

void clean()
{
	const string temp_dir = folder + "\\__temp__\\";
	const string wav_files = folder + "*.wav";
	const string config_files = folder + "*.config";
	const string all_files = folder + "*.*";
	system(string("mkdir " + temp_dir).c_str());
	system(string("move >nul " + wav_files + " " + temp_dir).c_str());
	system(string("move >nul " + config_files + " " + temp_dir).c_str());
	system(string("erase /q " + all_files).c_str());
	system(string("move >nul " + temp_dir + "* " + folder).c_str());
	system(string("rmdir " + temp_dir).c_str());
}

int main()
{
	// exhaustive search, use fewer options
	vector<string> cepstral_values = { "mfc", "lpc" };
	vector<int> n_cepstra_values = { 12, 16, 20, 24 };
	vector<int> n_predict_values = { 8, 10, 13, 16 };
	vector<bool> q_gain_values = { false, true };
	vector<bool> q_delta_values = { false, true };
	vector<bool> q_accel_values = { false, true };
	vector<int> x_codebook_values = { 8, 16, 32, 64 };
	vector<int> n_state_values = { 5, 8, 10, 12 };
	vector<int> n_bakis_values = { 1, 2, 3, 4 };
	vector<int> n_retrain_values = { 3, 4, 5 };

	int n_hits = numeric_limits<int>::min();

	for (const string &cepstral : cepstral_values)
	{
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
											if (cepstral == "mfc" && n_predict != n_predict_values[0])
											{
												continue;
											}
											if (!q_delta && q_accel)
											{
												continue;
											}

											Config config;
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

											Logger::info("Config:", "n_cepstra", n_cepstra, "n_predict", n_predict, "q_gain", q_gain, "q_delta", q_delta, "q_accel", q_accel, "x_codebook", x_codebook, "n_state", n_state, "n_bakis", n_bakis, "n_retrain", n_retrain, "cepstral", cepstral);

											chrono::steady_clock::time_point start = chrono::steady_clock::now();
											pair<int, int> result = test(config);
											chrono::steady_clock::time_point end = chrono::steady_clock::now();

											chrono::seconds time = chrono::duration_cast<chrono::seconds>(end - start);
											Logger::info("Time taken :", time.count(), "n_hits", result.first, "n_errs", result.second);

											if (n_hits < result.first)
											{
												n_hits = result.first;
												Utils::set_item_to_file<Config>(config, folder + "sr-lib.config");
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
