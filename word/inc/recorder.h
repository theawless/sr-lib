#pragma once

#include <string>

class Recorder
{
private:
	const static int sample_rate = 16000;
	std::string folder;
	std::string sox_path;

public:
	/// Constructor.
	Recorder(std::string folder, std::string sox_path);

	/// Records the audio with given parameters.
	void record(std::string filename, double duration);
};
