#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"

using namespace std;

class Recorder
{
private:
	const static int sample_rate = 16000;
	const string record_command = "\"C:\\Program Files (x86)\\sox-14-4-2\\sox.exe\" -t waveaudio -c 1 -r 16000 -d -t s16 -q";

public:
	static void capture_audio(string filename, double duration)
	{
		string data_filename = filename + ".wav";
		string recording_filename = filename + ".samples";
		string record_command = record_command + " " + data_filename + " trim 0 " + to_string(duration);
		cout << "Recording started, duration: " << duration << endl;
		system(record_command.c_str());

		fstream data(data_filename, ios::in | ios::binary);
		vector<int16_t> amplitudes(ceil(sample_rate * duration));
		data.read((char *)&amplitudes[0], amplitudes.size() * sizeof(amplitudes[0]));

		Utils::set_vector_to_file<int16_t>(amplitudes, recording_filename);
	}
};

/// The entry of the program, it takes user input, calls required functions and outputs the results.
int main()
{
	const double duration = 0.8;
	const int n_utterances = 15;
	const string record_folder = "B:\\record\\";
	const vector<string> audio_names = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };

	cout << "Starting recording session.";
	for (int i = 1; i <= n_utterances; ++i)
	{
		cout << endl << "Utterances phase: " << i << endl;
		for (int j = 0; j < audio_names.size(); ++j)
		{
			cout << endl << "Press enter to start for: " << audio_names[j] && cin.get();
			Recorder::capture_audio(audio_names[j] + "_" + to_string(i), duration);
		}
	}
	cout << endl << "All recordings done" << cin.get();

	return 0;
}
