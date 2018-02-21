#pragma once

#include <fstream>
#include <ios>
#include <iostream>
#include <vector>

/// Thanks to https://stackoverflow.com/questions/13978480/using-freopen-to-print-to-file-and-screen/13978705
class Logger
{
private:
	std::vector<std::ofstream> log_files;

public:
	/// Global logger.
	inline static Logger& logger()
	{
		static Logger instance;

		return instance;
	}

	/// Operator implementation.
	template<typename T>
	inline Logger& operator<<(const T& obj)
	{
		std::cout << obj;
		for (int i = 0; i < log_files.size(); ++i)
		{
			log_files[i] << obj;
		}

		return *this;
	}

	inline Logger& operator<<(std::ios_base& (*func)(std::ios_base&))
	{
		std::cout << func;
		for (int i = 0; i < log_files.size(); ++i)
		{
			log_files[i] << func;
		}

		return *this;
	}

	template<typename CharT, typename Traits>
	inline Logger& operator<<(std::basic_ios<CharT, Traits>& (*func)(std::basic_ios<CharT, Traits>&))
	{
		std::cout << func;
		for (int i = 0; i < log_files.size(); ++i)
		{
			log_files[i] << func;
		}

		return *this;
	}

	inline Logger& operator<<(std::ostream& (*func)(std::ostream&))
	{
		std::cout << func;
		for (int i = 0; i < log_files.size(); ++i)
		{
			log_files[i] << func;
		}

		return *this;
	}

	/// Adds log file.
	inline void add_log(std::string filename)
	{
		log_files.push_back(std::ofstream(filename));
	}
};
