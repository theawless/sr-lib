#pragma once

#include <array>
#include <iostream>
#include <string>
#include <vector>

/// http://soundfile.sapp.org/doc/WaveFormat/
/// Works with only PCM 16 bit uncompressed little endian wav files.
struct Wav
{
public:

	/// Get the samples.
	template <typename T>
	inline std::vector<T> samples() const
	{
		std::vector<T> samples;

		// data should be initialized so that it does not have garbage size
		for (int i = 0; i < data.size(); ++i)
		{
			samples.push_back(0x8000 & data[i] ? static_cast<int>((0x7FFF & data[i])) - 0x8000 : data[i]);
		}

		return samples;
	}

	/// Operator for loading.
	friend std::istream &operator>>(std::istream &input, Wav &wav)
	{
		input.read(reinterpret_cast<char *>(&wav.chunk_id[0]), wav.chunk_id.size() * sizeof(wav.chunk_id[0]));
		input.read(reinterpret_cast<char *>(&wav.chunk_size), sizeof(wav.chunk_size));
		input.read(reinterpret_cast<char *>(&wav.format[0]), wav.format.size() * sizeof(wav.format[0]));

		input.read(reinterpret_cast<char *>(&wav.subchunk1_id[0]), wav.subchunk1_id.size() * sizeof(wav.subchunk1_id[0]));
		input.read(reinterpret_cast<char *>(&wav.subchunk1_size), sizeof(wav.subchunk1_size));
		input.read(reinterpret_cast<char *>(&wav.audio_format), sizeof(wav.audio_format));
		input.read(reinterpret_cast<char *>(&wav.num_channels), sizeof(wav.num_channels));
		input.read(reinterpret_cast<char *>(&wav.sample_rate), sizeof(wav.sample_rate));
		input.read(reinterpret_cast<char *>(&wav.byte_rate), sizeof(wav.byte_rate));
		input.read(reinterpret_cast<char *>(&wav.block_align), sizeof(wav.block_align));
		input.read(reinterpret_cast<char *>(&wav.bits_per_sample), sizeof(wav.bits_per_sample));

		input.read(reinterpret_cast<char *>(&wav.subchunk2_id[0]), wav.subchunk2_id.size() * sizeof(wav.subchunk2_id[0]));
		input.read(reinterpret_cast<char *>(&wav.subchunk2_size), sizeof(wav.subchunk2_size));

		wav.data.resize(wav.subchunk2_size / 2);
		input.read(reinterpret_cast<char *>(&wav.data[0]), wav.data.size() * sizeof(wav.data[0]));

		return input;
	}

private:
	// RIFF
	std::array<char, 4> chunk_id;
	uint32_t chunk_size;
	std::array<char, 4> format;

	// fmt
	std::array<char, 4> subchunk1_id;
	uint32_t subchunk1_size;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;

	// data
	std::array<char, 4> subchunk2_id;
	uint32_t subchunk2_size;
	std::vector<uint16_t> data = std::vector<uint16_t>();
};
