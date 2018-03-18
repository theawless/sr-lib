#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

/// Thanks to https://stackoverflow.com/questions/27375089/what-is-the-easiest-way-to-print-a-variadic-parameter-pack-using-stdostream
namespace Logger
{
	/// Prints the args in the given stream.
	template <typename Arg, typename... Args>
	static inline void unpack(std::ostream& out, Arg&& arg, Args&&... args)
	{
		out << std::forward<Arg>(arg);
		using expander = int[];
		(void)expander
		{
			0, (void(out << ' ' << std::forward<Args>(args)), 0)...
		};
	}

	/// Logs the args in the standard output.
	template<typename... Args>
	inline void info(Args... args)
	{
		std::stringstream stream;
		unpack(stream, args...);

		std::cout << stream.str() + '\n';
		std::cout.flush();
	}

	/// Logs the args in the log output.
	template<typename... Args>
	inline void log(Args... args)
	{
#if DEBUG
		std::stringstream stream;
		unpack(stream, args...);

		std::clog << stream.str() + '\n';
		std::clog.flush();
#endif
	}
};
