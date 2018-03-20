#pragma once

#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "utils.h"

struct Config
{
protected:
	std::map<std::string, std::string> dict;

public:
	/// Gets the value from key.
	template<typename T>
	inline T get_val(const std::string &key, T fail) const
	{
		if (dict.find(key) == dict.end())
		{
			return fail;
		}
		std::stringstream stream(dict.at(key));

		return Utils::get_item_from_stream<T>(stream);
	}

	/// Sets the value for key.
	template<typename T>
	inline void set_val(const std::string &key, const T &val)
	{
		dict[key] = Utils::get_string_from_item<T>(val);
	}

	/// Operators for loading and saving.
	friend std::istream &operator>>(std::istream &input, Config &config);
	friend std::ostream &operator<<(std::ostream &output, const Config &config);
};

struct WordConfig : public Config
{
public:
	/// Adds word or utterance.
	int add(const std::string &word);

	/// Get words.
	std::vector<std::string> words() const;

	/// Get utterances.
	std::vector<std::pair<std::string, int>> utterances() const;
};
