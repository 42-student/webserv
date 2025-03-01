#include "../inc/PrintApp.hpp"

std::string PrintApp::getCurrentDateTime()
{
	std::time_t now = std::time(NULL);
	std::tm* time_info = std::localtime(&now);
	char date[100];
	std::strftime(date, sizeof(date), "[%d-%m-%Y %H:%M:%S] ", time_info);
	return std::string(date, date + std::strlen(date));
}

void PrintApp::printEvent(const char* color, ExitStatus status, const char* str, ...)
{
	char output[8192];
	va_list args;
	va_start(args, str);
	vsnprintf(output, sizeof(output), str, args);
	va_end(args);
	std::string errorAndExit = color + PrintApp::getCurrentDateTime() + output + RESET;
	if (status == FAILURE)
		throw Error(errorAndExit);
	std::cout << color << PrintApp::getCurrentDateTime() << output << RESET << std::endl;
}

void PrintApp::printErrorCode(const char* color, short& errorCode, short code, const char* str, ...)
{
	char output[8192];
	va_list args;
	va_start(args, str);
	vsnprintf(output, sizeof(output), str, args);
	va_end(args);
	errorCode = code;
	std::cout << color << PrintApp::getCurrentDateTime() << output << RESET << std::endl;
	return;
}

void PrintApp::printStartServer(Server& server)
{
	char buf[INET_ADDRSTRLEN];
	const char* name = server.getServerName().c_str();
	const char* host = inet_ntop(AF_INET, &server.getHost(), buf, INET_ADDRSTRLEN);
	int port = server.getPort();
	std::stringstream p;
	p << port;
	std::string portStr = p.str();
	std::string serverAddr = "http://" + std::string(host) + ":" + portStr;
	std::cout << YELLOW;
	std::cout << "┌─────────────────────────────────────────────────┐" << std::endl;
	std::cout << "│   Server Address: " << serverAddr << std::setw(33 - serverAddr.size()) << "│" << std::right << std::endl;
	std::cout << "│   " << name << ": " << PrintApp::getCurrentDateTime() << std::setw(25 - std::string(name).size()) << "│" << std::right << std::endl;
	std::cout << "└─────────────────────────────────────────────────┘" << std::endl;
	std::cout << RESET;
}
