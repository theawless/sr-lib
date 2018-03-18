#include <array>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

using namespace std;

/// Thanks to http://soundfile.sapp.org/doc/WaveFormat/
/// Works with only PCM 16 bit uncompressed little endian wav files.
class Wav
{
private:
	// RIFF
	array<char, 4> chunk_id;
	uint32_t chunk_size;
	array<char, 4> format;

	// fmt
	array<char, 4> subchunk1_id;
	uint32_t subchunk1_size;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;

	// data
	array<char, 4> subchunk2_id;
	uint32_t subchunk2_size;
	std::vector<uint16_t> data;

public:
	/// Constructor.
	inline Wav(std::string filename)
	{
		std::ifstream file(filename, std::ios::binary);

		file.read(reinterpret_cast<char *>(&chunk_id[0]), chunk_id.size() * sizeof(chunk_id[0]));
		file.read(reinterpret_cast<char *>(&chunk_size), sizeof(chunk_size));
		file.read(reinterpret_cast<char *>(&format[0]), format.size() * sizeof(format[0]));

		file.read(reinterpret_cast<char *>(&subchunk1_id[0]), subchunk1_id.size() * sizeof(subchunk1_id[0]));
		file.read(reinterpret_cast<char *>(&subchunk1_size), sizeof(subchunk1_size));
		file.read(reinterpret_cast<char *>(&audio_format), sizeof(audio_format));
		file.read(reinterpret_cast<char *>(&num_channels), sizeof(num_channels));
		file.read(reinterpret_cast<char *>(&sample_rate), sizeof(sample_rate));
		file.read(reinterpret_cast<char *>(&byte_rate), sizeof(byte_rate));
		file.read(reinterpret_cast<char *>(&block_align), sizeof(block_align));
		file.read(reinterpret_cast<char *>(&bits_per_sample), sizeof(bits_per_sample));

		file.read(reinterpret_cast<char *>(&subchunk2_id[0]), subchunk2_id.size() * sizeof(subchunk2_id[0]));
		file.read(reinterpret_cast<char *>(&subchunk2_size), sizeof(subchunk2_size));

		data.resize(subchunk2_size / 2);
		file.read(reinterpret_cast<char *>(&data[0]), data.size() * sizeof(data[0]));
	}

	/// Gets the samples as type.
	template <typename T>
	inline std::vector<T> samples() const
	{
		std::vector<T> samples;
		for (int i = 0; i < data.size(); ++i)
		{
			samples.push_back(0x8000 & data[i] ? static_cast<int>((0x7FFF & data[i])) - 0x8000 : data[i]);
		}

		return samples;
	}
};
