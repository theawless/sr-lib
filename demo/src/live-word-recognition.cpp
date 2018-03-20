#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "logger.h"
#include "tester.h"
#include "trainer.h"
#include "utils.h"

using namespace std;

class Recorder
{
public:
	/// Constructor.
	Recorder(const string &folder, const string &sox_path) : folder(folder), sox_path(sox_path)
	{
	}

	/// Records the audio with given parameters.
	void record(const string &filename, double duration) const
	{
		const string wav_filename = folder + filename + ".wav";
		const string base_command = '"' + sox_path + '"' + " -t waveaudio -c 1 -r " + to_string(sample_rate) + " -d -q -L ";
		const string full_command = base_command + wav_filename + " trim 0 " + to_string(duration);

		Logger::info("Recording started, duration:", duration);
		system(full_command.c_str());
	}

private:
	static constexpr int sample_rate = 16000;

	const string folder;
	const string sox_path;
};

int main()
{
	const string folder = "B:\\record\\digit_0.8_2\\";

	const string config_filename = folder + "sr-lib.config";
	const string word_config_filename = folder + "words.config";
	const Config config = Utils::get_item_from_file<Config>(config_filename);
	const WordConfig word_config = Utils::get_item_from_file<WordConfig>(word_config_filename);
	
	Logger::info("Training...");
	const unique_ptr<ModelTrainer> model_trainer = ModelTrainer::Builder(folder, word_config.utterances(), config).build();
	const unique_ptr<Tester> tester = Tester::Builder(folder, word_config.words(), config).build();

	const double duration = 0.8;
	const string test_filename = "__test__";
	const Recorder recorder(folder, "C:\\Program Files (x86)\\sox-14-4-2\\sox.exe");

	string ready;
	while (Logger::info("Are you ready to speak? (y/n)"), cin >> ready)
	{
		if (ready != "Y" && ready != "y")
		{
			continue;
		}
		recorder.record(test_filename, duration);

		Logger::info("The recognised word is:", tester->test(test_filename));
	}
}
