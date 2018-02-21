#include <iostream>
#include <string>
#include <vector>

#include "logger.h"
#include "recognizer.h"
#include "recorder.h"

using namespace std;

int main()
{
	Config config("word.config", "B:/record/digit_0.8_2/");
	Recognizer recognizer(config);
	recognizer.setup();

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

		int word_index = recognizer.recognize(test_filename);
		Logger::logger() << "The recognised word is: " << config.audio_names[word_index] << endl;
	}
}
