#include <algorithm>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#include "logger.h"
#include "recognizer.h"
#include "recorder.h"

using namespace std;

int main()
{
	Config config("B:\\record\\random\\");
	config.load("word.config");
	Recognizer *recognizer = nullptr;

	double duration = 0.8;
	string test_filename = "test";
	Recorder recorder(config.folder, "C:\\Program Files (x86)\\sox-14-4-2\\sox.exe");

	string option;
	while (Logger::logger() << "Choose: <Word>: Add word, <1> Train, <2> Test: ", cin >> option)
	{
		if (option == "1")
		{
			if (config.audio_names.empty())
			{
				Logger::logger() << "Recording is required first." << endl;
				continue;
			}

			vector<string> del_patterns = { "*.model*", "*.mfcs", "*.lpcs", "*.codebook", "*.universe" };
			for (int i = 0; i < del_patterns.size(); ++i)
			{
				string filename = config.folder + del_patterns[i];
				Logger::logger() << "Deleting " << filename << endl;
				system(string("DEL " + filename).c_str());
			}
			config.load("word.config");
			if (recognizer != nullptr)
			{
				delete recognizer;
			}
			recognizer = new Recognizer(config);
		}
		else if (option == "2")
		{
			if (recognizer == nullptr)
			{
				Logger::logger() << "Training is required first." << endl;
				continue;
			}

			recorder.record(test_filename, duration);
			int word_index = recognizer->recognize(test_filename);
			Logger::logger() << "The recognised word is: " << config.audio_names[word_index] << endl;
		}
		else
		{
			int index = config.add_word(option);
			string filename = option + "_" + to_string(index);
			recorder.record(filename, duration);
		}
	}
	if (recognizer != nullptr)
	{
		delete recognizer;
	}
}
