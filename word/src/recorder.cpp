#include "recorder.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include "logger.h"
#include "utils.h"

using namespace std;

Recorder::Recorder(string folder, string sox_path) : folder(folder), sox_path(sox_path)
{
}

void Recorder::record(string filename, double duration)
{
	string data_filename = folder + filename + ".wav";
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
