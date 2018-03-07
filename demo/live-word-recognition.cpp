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
		string data_filename = folder + filename + ".audio";
		string recording_filename = folder + filename + ".samples";

		string base_command = '"' + sox_path + '"' + " -t waveaudio -c 1 -r " + to_string(sample_rate) + " -d -t s16 -q ";
		string full_command = base_command + data_filename + " trim 0 " + to_string(duration);
		Logger::logger() << "Recording started, duration: " << duration << endl;
		system(full_command.c_str());

		fstream data(data_filename, ios::in | ios::binary);
		vector<int16_t> amplitudes(ceil(sample_rate * duration));
		data.read((char *)&amplitudes[0], amplitudes.size() * sizeof(amplitudes[0]));
		Utils::set_vector_to_file<int16_t>(amplitudes, recording_filename);
	}
};

int main()
{
	Config config("B:\\record\\digit_0.8_2\\");
	config.load("word.config");
	Trainer trainer(config);
	Tester tester(config);

	double duration = 0.8;
	string test_filename = "test";
	Recorder recorder(config.folder, "C:\\Program Files (x86)\\sox-14-4-2\\sox.exe");

	string ready;
	while (Logger::logger() << "Are you ready to speak? (y/n) ", cin >> ready)
	{
		if (ready != "Y" && ready != "y")
		{
			continue;
		}

		recorder.record(test_filename, duration);

		int word_index = tester.test(test_filename);
		Logger::logger() << "The recognised word is: " << config.audio_names[word_index] << endl;
	}
}
