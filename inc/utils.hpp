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
#include <poll.h>
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
#define CYAN "\033[0;36m"

#define STATE_REQUEST_LINE 0
#define STATE_REQUEST_LINE_POST_PUT 1
#define STATE_REQUEST_LINE_METHOD 2
#define STATE_REQUEST_LINE_FIRST_SPACE 3
#define STATE_REQUEST_LINE_URI_PATH_SLASH 4
#define STATE_REQUEST_LINE_URI_PATH 5
#define STATE_REQUEST_LINE_URI_QUERY 6
#define STATE_REQUEST_LINE_URI_FRAGMENT 7
#define STATE_REQUEST_LINE_VER 8
#define STATE_REQUEST_LINE_HT 9
#define STATE_REQUEST_LINE_HTT 10
#define STATE_REQUEST_LINE_HTTP 11
#define STATE_REQUEST_LINE_HTTP_SLASH 12
#define STATE_REQUEST_LINE_MAJOR 13
#define STATE_REQUEST_LINE_DOT 14
#define STATE_REQUEST_LINE_MINOR 15
#define STATE_REQUEST_LINE_CR 16
#define STATE_REQUEST_LINE_LF 17
#define STATE_FIELD_NAME_START 18
#define STATE_FIELDS_END 19
#define STATE_FIELD_NAME 20
#define STATE_FIELD_VALUE 21
#define STATE_FIELD_VALUE_END 22
#define STATE_CHUNKED_LENGTH_BEGIN 23
#define STATE_CHUNKED_LENGTH 24
#define STATE_CHUNKED_IGNORE 25
#define STATE_CHUNKED_LENGTH_CR 26
#define STATE_CHUNKED_LENGTH_LF 27
#define STATE_CHUNKED_DATA 28
#define STATE_CHUNKED_DATA_CR 29
#define STATE_CHUNKED_DATA_LF 30
#define STATE_CHUNKED_END_CR 31
#define STATE_CHUNKED_END_LF 32
#define STATE_MESSAGE_BODY 33
#define STATE_PARSING_DONE 34

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
