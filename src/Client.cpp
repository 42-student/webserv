#include "../inc/Client.hpp"

Client::Client()
{
	_lastMessage = time(NULL);
}

Client::Client(const Client &copy)
{
	this->_cliSock = copy._cliSock;
	this->_cliSockAddr = copy._cliSockAddr;
	this->request = copy.request;
	this->response = copy.response;
	this->server = copy.server;
	this->_lastMessage = copy._lastMessage;
}

Client::Client(Server &server)
{
	setServer(server);
	request.setMaxBodySize(server.getClientMaxBodySize());
	_lastMessage = time(NULL);
}

Client &Client::operator=(const Client &copy)
{
	this->_cliSock = copy._cliSock;
	this->_cliSockAddr = copy._cliSockAddr;
	this->request = copy.request;
	this->response = copy.response;
	this->server = copy.server;
	this->_lastMessage = copy._lastMessage;
	return (*this);
}

Client::~Client() {}

void Client::buildResponse()
{
	response.setRequest(this->request);
	response.buildResponse();
}

void Client::updateLastMessageTime()
{
	_lastMessage = time(NULL);
}

void Client::setSocket(int &socket)
{
	_cliSock = socket;
}

void Client::setServer(Server &server)
{
	response.setServer(server);
}

const time_t &Client::getLastMessageTime() const
{
	return (_lastMessage);
}

void Client::clearClient()
{
	response.clear();
	request.clear();
}
