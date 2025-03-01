#ifndef PrintApp_HPP
#define PrintApp_HPP

#include "utils.hpp"

enum ExitStatus
{
	FAILURE,
	SUCCESS,
};

class Request;

class Server;

class PrintApp
{
	public:
			static std::string getCurrentDateTime();
			static void printEvent(const char* color, ExitStatus status, const char* str, ...);
			static void printErrorCode(const char* color, short& errorCode, short code, const char* str, ...);
			static void printStartServer(Server& server);
};

#endif
