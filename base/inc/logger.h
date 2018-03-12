#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

/// Thanks to https://stackoverflow.com/questions/13978480/using-freopen-to-print-to-file-and-screen/13978705
/// Thanks to https://stackoverflow.com/questions/27375089/what-is-the-easiest-way-to-print-a-variadic-parameter-pack-using-stdostream
namespace Logger
{
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

	template<typename... Args>
	inline void log(Args... args)
	{
		std::stringstream stream;
		unpack(stream, args...);
		std::string message = stream.str() + '\n';

		std::cout << message;
		std::cout.flush();
	}
};
