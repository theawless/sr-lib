#pragma once

#include <vector>

namespace std {
	enum Phoneme {
		Unvoiced,
		Voiced,
		Silence,
		Unknown,
	};
}

/// Segment the speech and detect phoneme types.
std::vector<std::Phoneme> tag_phonemes(const std::vector<std::vector<double>> &segments, const std::vector<double> &noise);
