#include <iostream>
#include <string>
#include <vector>

#include "logger.h"
#include "tester.h"
#include "trainer.h"

using namespace std;

class Recorder
{
private:
	const static int sample_rate = 16000;
	string folder;
	string sox_path;

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
		Logger::log("Recording started, duration:", duration);
		system(full_command.c_str());
	}
};

int main()
{
	Config config("B:\\record\\digit_0.8_2\\");
	config.load("sr-lib.config");
	Trainer trainer(config);
	Tester tester(config);

	double duration = 0.8;
	string test_filename = "test";
	Recorder recorder(config.folder, "C:\\Program Files (x86)\\sox-14-4-2\\sox.exe");

	string ready;
	while (Logger::log("Are you ready to speak? (y/n)"), cin >> ready)
	{
		if (ready != "Y" && ready != "y")
		{
			continue;
		}

		recorder.record(test_filename, duration);

		int word_index = tester.test(test_filename);
		Logger::log("The recognised word is:", word_index == -1 ? "###" : config.audio_names[word_index]);
	}
}
