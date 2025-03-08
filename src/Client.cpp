#include "../inc/Client.hpp"

Client::Client()
{
	_lastMessage = time(NULL);
	_cgiStartTime = 0;
}

Client::Client(const Client &copy)
{
	_cliSock = copy._cliSock;
	_cliSockAddr = copy._cliSockAddr;
	request = copy.request;
	response = copy.response;
	server = copy.server;
	_lastMessage = copy._lastMessage;
	_cgiStartTime = copy._cgiStartTime;
}

Client::Client(Server &serv)
{
	setServer(serv);
	request.setMaxBodySize(server.getClientMaxBodySize());
	_lastMessage = time(NULL);
	_cgiStartTime = 0;
}

Client &Client::operator=(const Client &copy)
{
	if (this != &copy)
	{
		_cliSock = copy._cliSock;
		_cliSockAddr = copy._cliSockAddr;
		request = copy.request;
		response = copy.response;
		server = copy.server;
		_lastMessage = copy._lastMessage;
		_cgiStartTime = copy._cgiStartTime;
	}
	return (*this);
}

Client::~Client() {}

void Client::buildResponse()
{
	response.setRequest(this->request);
	response.constructResponse();
}

void Client::updateLastMessageTime()
{
	_lastMessage = time(NULL);
}

void Client::setSocket(int &socket)
{
	_cliSock = socket;
}

int Client::getSocket()
{
	return this->_cliSock;
}

void Client::setServer(Server &server)
{
	response.setServer(server);
}

const time_t &Client::getLastMessageTime() const
{
	return this->_lastMessage;
}

void Client::setCgiStartTime(time_t start)
{
	_cgiStartTime = start;
}

time_t Client::getCgiStartTime() const
{
	return this->_cgiStartTime;
}

void Client::clearClient()
{
	response.reset();
	request.clear();
}
