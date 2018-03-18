#include <string>
#include <vector>

#include "logger.h"
#include "tester.h"
#include "trainer.h"

using namespace std;

class Recorder
{
private:
	static constexpr int sample_rate = 16000;

	const string folder;
	const string sox_path;

public:
	/// Constructor.
	Recorder(string folder, string sox_path) : folder(folder), sox_path(sox_path)
	{
	}

	/// Records the audio with given parameters.
	void record(string filename, double duration)
	{
		string wav_filename = folder + filename + ".wav";

		string base_command = '"' + sox_path + '"' + " -t waveaudio -c 1 -r " + to_string(sample_rate) + " -d -q -L ";
		string full_command = base_command + wav_filename + " trim 0 " + to_string(duration);
		Logger::info("Recording started, duration:", duration);
		system(full_command.c_str());
	}
};

int main()
{
	const string folder = "B:\\record\\digit_0.8_2\\";
	Config config(folder + "sr-lib.config");
	WordConfig word_config(folder + "words.config");
	config.load(); word_config.load();

	Parameters parameters(folder, word_config.words(), config);
	Trainer trainer(parameters);
	Tester tester(parameters);

	double duration = 0.8;
	string test_filename = "__test__";
	Recorder recorder(folder, "C:\\Program Files (x86)\\sox-14-4-2\\sox.exe");

	string ready;
	while (Logger::info("Are you ready to speak? (y/n)"), cin >> ready)
	{
		if (ready != "Y" && ready != "y")
		{
			continue;
		}

		recorder.record(test_filename, duration);

		Logger::info("The recognised word is:", tester.test(test_filename));
	}
	cin.get();
}
