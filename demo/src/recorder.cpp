#include <iostream>
#include <string>
#include <vector>

#include "utils.h"

#define DURATION 2
#define N_UTTERANCES 15
#define RECORD_FOLDER "B:\\record\\digit\\"
#define RECORD_FILENAME RECORD_FOLDER "140101002"

using namespace std;

vector<string> audio_names = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/// The entry of the program, it takes user input, calls required functions and outputs the results.
int main() {
	cout << "Starting recording session.";
	for (int i = 1; i <= N_UTTERANCES; ++i) {
		string utterance = pad_number(i, N_UTTERANCES);
		cout << endl << "Utterances phase: " << utterance << endl;
		for (int j = 0; j < audio_names.size(); ++j) {
			cout << endl << "Press enter to start for: " << audio_names[j] && cin.get();
			capture_audio(string(RECORD_FILENAME) + "_" + audio_names[j] + "_" + utterance, DURATION);
		}
	}
	cout << endl << "All recordings done" << cin.get();

	return 0;
}
