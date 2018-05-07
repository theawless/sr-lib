#pragma once

#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

namespace Logger
{
	/// Print the args in the given stream.
	template <typename Arg, typename... Args>
	static inline void print(std::ostream& out, Arg&& arg, Args&&... args)
	{
		// https://stackoverflow.com/questions/27375089/what-is-the-easiest-way-to-print-a-variadic-parameter-pack-using-stdostream
		out << std::forward<Arg>(arg);
		using expander = int[];
		(void)expander
		{
			0, (void(out << ' ' << std::forward<Args>(args)), 0)...
		};
	}

	/// Print the args in the standard output.
	template<typename... Args>
	inline void info(Args... args)
	{
		std::stringstream stream;
		print(stream, args...);

		std::cout << stream.str() + '\n';
		std::cout.flush();
	}

	/// Print the args in the log output.
	template<typename... Args>
	inline void log(Args... args)
	{
#ifndef NDEBUG
		std::stringstream stream;
		print(stream, args...);

		std::clog << stream.str() + '\n';
		std::clog.flush();
#endif
	}
};
