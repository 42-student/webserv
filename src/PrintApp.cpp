#include "../inc/PrintApp.hpp"

std::string PrintApp::getCurrentDateTime()
{
	std::time_t now = std::time(NULL);
	std::tm* time_info = std::localtime(&now);
	char date[24];
	std::strftime(date, sizeof(date), "[%d-%m-%Y %H:%M:%S] ", time_info);
	return std::string(date, date + std::strlen(date));
}

void PrintApp::printEvent(const char* color, ExitStatus status, const char* str, ...)
{
	char output[1024];
	va_list args;
	va_start(args, str);
	vsnprintf(output, sizeof(output), str, args);
	va_end(args);
	std::string timestamp = PrintApp::getCurrentDateTime();
	std::string formattedMessage;
	formattedMessage += color;
	formattedMessage += timestamp;
	formattedMessage += output;
	formattedMessage += RESET;
	if (status == FAILURE)
		throw Error(formattedMessage);
	std::cout << formattedMessage << std::endl;
}

void PrintApp::printErrorCode(const char* color, short& errorCode, short code, const char* str, ...)
{
	char output[1024];
	va_list args;
	va_start(args, str);
	vsnprintf(output, sizeof(output), str, args);
	va_end(args);
	errorCode = code;
	std::cout << color;
	std::cout << PrintApp::getCurrentDateTime();
	std::cout << output;
	std::cout << RESET << std::endl;
}

void PrintApp::printStartServer(Server& server)
{
	char buf[INET_ADDRSTRLEN];
	const char* name = server.getServerName().c_str();
	const char* host = inet_ntop(AF_INET, &server.getHost(), buf, INET_ADDRSTRLEN);
	int port = server.getPort();
	std::ostringstream ossp;
	ossp << port;
	std::string portStr = ossp.str();
	std::string serverAddr = "http://";
	serverAddr += host;
	serverAddr += ":";
	serverAddr += portStr;
	std::cout << YELLOW;
	std::cout << "\n*********************************************" << std::endl;
	std::cout << "*   Server Address: " << serverAddr;
	std::cout << std::setw(25 - static_cast<int>(serverAddr.size())) << " *" << std::endl;
	std::cout << "*   " << name << ": " << PrintApp::getCurrentDateTime();
	std::cout << std::setw(17 - static_cast<int>(std::strlen(name))) << " *" << std::endl;
	std::cout << "*********************************************" << std::endl;
	std::cout << RESET;
}
