#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <fstream>
#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "Cgi.hpp"
#include "ConfigFile.hpp"
#include "Location.hpp"
#include "PrintApp.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Server.hpp"

#define RED_BOLD "\033[31;1m"
#define GREEN_BOLD "\033[32;1m"
#define BLUE "\033[34;10m"
#define RED "\033[0;31m"
#define YELLOW "\033[33;10m"
#define ORANGE "\033[38;5;208m"
#define WHITE "\033[38;10m"
#define RESET "\033[0m"

#define MAX_URI_LENGTH 4096
#define MAX_CONTENT_LENGTH 31457280

template <typename T>
std::string toString(const T val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

inline int oops(std::string errorMessage)
{
	std::cerr << RED;
	std::cerr << errorMessage << std::endl;
	std::cerr << RESET;
	return 1;
}

class Error : public std::runtime_error
{
	public:
		Error(const std::string &errorMessage) : std::runtime_error(formatErrorMessage(errorMessage)) {}

	private:
		std::string formatErrorMessage(const std::string &errorMessage) const
		{
			if (errorMessage.empty())
				return "";
			return "\033[31m" + errorMessage + "\033[0m";
		}
};

#endif
