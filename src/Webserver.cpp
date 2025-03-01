#include "../inc/Webserver.hpp"

Webserver::Webserver() {}

Webserver::~Webserver() {}

void Webserver::setupServers(std::vector<Server> servers)
{
	std::cout << "Initializing servers...\n";
	_serv = servers;	
	for (std::vector<Server>::iterator it = _serv.begin(); it != _serv.end(); ++it)
	{
		bool isSameServ = false;
		for (std::vector<Server>::iterator existingServerIt = _serv.begin(); existingServerIt != it; ++existingServerIt)
		{
			if (existingServerIt->getHost() == it->getHost() && existingServerIt->getPort() == it->getPort())
			{
				it->setFd(existingServerIt->getFd());
				isSameServ = true;
			}
		}
		if (!isSameServ)
			it->setupServer();
		PrintApp::printStartServer(*it);
	}
}

void Webserver::processServerRequests()
{
	fd_set receivedFdCopy;
	fd_set writeFdCopy;
	int selectStatus;	
	_maxFd = 0;
	initializeSets();
	struct timeval timer;
	while (true)
	{
		timer.tv_sec = 1;
		timer.tv_usec = 0;
		receivedFdCopy = _inFdSet;
		writeFdCopy = _outFdSet;
		if ((selectStatus = select(_maxFd + 1, &receivedFdCopy, &writeFdCopy, NULL, &timer)) < 0)
			PrintApp::printEvent(RED_BOLD, FAILURE, "Error occurred during select operation. Reason: %s. Closing the connection...", strerror(errno));
		for (int i = 0; i <= _maxFd; ++i)
		{
			if (FD_ISSET(i, &receivedFdCopy) && _servsDict.count(i))
				acceptNewConnection(_servsDict.find(i)->second);
			else if (FD_ISSET(i, &receivedFdCopy) && _clientsDict.count(i))
				readAndProcessRequest(i, _clientsDict[i]);
			else if (FD_ISSET(i, &writeFdCopy) && _clientsDict.count(i))
			{
				int cgi_state = _clientsDict[i].response.getCgiState();
				if (cgi_state == 1 && FD_ISSET(_clientsDict[i].response.cgiObj.pipeIn[1], &writeFdCopy))
					sendCgiBody(_clientsDict[i], _clientsDict[i].response.cgiObj);
				else if (cgi_state == 1 && FD_ISSET(_clientsDict[i].response.cgiObj.pipeOut[0], &receivedFdCopy))
					readCgiResponse(_clientsDict[i], _clientsDict[i].response.cgiObj);
				else if ((cgi_state == 0 || cgi_state == 2) && FD_ISSET(i, &writeFdCopy))
					sendResponse(i, _clientsDict[i]);
			}
		}
		handleClientTimeout();
	}
}

void Webserver::initializeSets()
{
	FD_ZERO(&_inFdSet);
	FD_ZERO(&_outFdSet);
	for (std::vector<Server>::iterator it = _serv.begin(); it != _serv.end(); ++it)
	{
		if (listen(it->getFd(), 512) == -1)
			PrintApp::printEvent(RED_BOLD, FAILURE, "Failed to listen on socket. Reason: %s. Closing the connection...", strerror(errno));
		if (fcntl(it->getFd(), F_SETFL, O_NONBLOCK) < 0)
			PrintApp::printEvent(RED_BOLD, FAILURE, "Error setting non-blocking mode using fcntl. Reason: %s. Closing the connection...", strerror(errno));
		addToSet(it->getFd(), _inFdSet);
		_servsDict.insert(std::make_pair(it->getFd(), *it));
	}
	_maxFd = _serv.back().getFd();
}

void Webserver::addToSet(const int fd, fd_set &set)
{
	FD_SET(fd, &set);
	if (fd > _maxFd)
		_maxFd = fd;
}

void Webserver::removeFromSet(const int fd, fd_set &fdSet)
{
	FD_CLR(fd, &fdSet);
	if (fd == _maxFd)
		_maxFd--;
}

void Webserver::acceptNewConnection(Server &serv)
{
	struct sockaddr_in cliSocketAddr;
	long cliSocketAddrSize = sizeof(cliSocketAddr);
	int cliSocket;
	Client newClient(serv);
	if ((cliSocket = accept(serv.getFd(), (struct sockaddr *)&cliSocketAddr, (socklen_t *)&cliSocketAddrSize)) == -1)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Encountered an error while accepting incoming connection. Reason: %s.", strerror(errno));
		return;
	}
	PrintApp::printEvent(GREEN_BOLD, SUCCESS, "New connection established.");
	addToSet(cliSocket, _inFdSet);
	if (fcntl(cliSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		PrintApp::printEvent(RED_BOLD, FAILURE, "Error encountered while setting socket to non-blocking mode using fcntl. Reason: %s. Closing the connection...", strerror(errno));
		removeFromSet(cliSocket, _inFdSet);
		close(cliSocket);
		return;
	}	
	newClient.setSocket(cliSocket);
	if (_clientsDict.count(cliSocket) != 0)
		_clientsDict.erase(cliSocket);
	_clientsDict.insert(std::make_pair(cliSocket, newClient));
}

void Webserver::readAndProcessRequest(const int &i, Client &client)
{
	char buffer[40000];
	int bytesRead = read(i, buffer, 40000);
	if (bytesRead == 0)
	{
		PrintApp::printEvent(YELLOW, SUCCESS, "Connection with client %d has been successfully closed.", i);
		closeConnection(i);
		return;
	}
	if (bytesRead < 0)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while reading from file descriptor %d. Reason: %s.", i, strerror(errno));
		closeConnection(i);
		return;
	}
	if (bytesRead)
	{
		client.updateLastMessageTime();
		client.request.parseHTTPRequestData(buffer, bytesRead);
		memset(buffer, 0, sizeof(buffer));
	}
	if (client.request.isParsingDone() || client.request.errorCodes())
	{
		assignServer(client);
		if (client.request.errorCodes() != 501)
			PrintApp::printEvent(BLUE, SUCCESS, "<%s> \"%s\"", client.request.getMethodStr().c_str(), client.request.getPath().c_str());
		client.buildResponse();
		if (client.response.getCgiState())
		{
			handleReqBody(client);
			addToSet(client.response.cgiObj.pipeIn[1], _outFdSet);
			addToSet(client.response.cgiObj.pipeOut[0], _inFdSet);
		}
		removeFromSet(i, _inFdSet);
		addToSet(i, _outFdSet);
	}
}

void Webserver::assignServer(Client &client)
{
	for (std::vector<Server>::iterator it = _serv.begin(); it != _serv.end(); ++it)
	{
		if (checkServ(client, it))
			client.setServer(*it);
	}
}

bool Webserver::checkServ(Client &client, std::vector<Server>::iterator it)
{
	bool s = client.request.getServerName() == it->getServerName();
	bool h = client.server.getHost() == it->getHost();
	bool p = client.server.getPort() == it->getPort();
	return s && h && p;
}

void Webserver::handleReqBody(Client &client)
{
	if (client.request.getBody().length() == 0)
	{
		std::string tmp;
		std::fstream file;
		(client.response.cgiObj.getCgiPath().c_str());
		std::stringstream ss;
		ss << file.rdbuf();
		tmp = ss.str();
		client.request.setBody(tmp);
	}
}

void Webserver::sendCgiBody(Client &client, Cgi &cgi)
{
	int bytesSent;
	std::string &req_body = client.request.getBody();	
	if (req_body.empty())
		bytesSent = 0;
	else if (req_body.length() >= 40000)
		bytesSent = write(cgi.pipeIn[1], req_body.c_str(), 40000);
	else
		bytesSent = write(cgi.pipeIn[1], req_body.c_str(), req_body.length());	
	if (bytesSent < 0)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while sending CGI body. Reason: %s.", strerror(errno));
		removeFromSet(cgi.pipeIn[1], _outFdSet);
		close(cgi.pipeIn[1]);
		close(cgi.pipeOut[1]);
		client.response.setErrorResponse(500);
	}
	else if (bytesSent == 0 || static_cast<size_t>(bytesSent) == req_body.length())
	{
		removeFromSet(cgi.pipeIn[1], _outFdSet);
		close(cgi.pipeIn[1]);
		close(cgi.pipeOut[1]);
	}
	else
	{
		client.updateLastMessageTime();
		req_body = req_body.substr(bytesSent);
	}
}

void Webserver::readCgiResponse(Client &client, Cgi &cgi)
{
	char buffer[40000 * 2];
	int bytesRead = 0;
	bytesRead = read(cgi.pipeOut[0], buffer, 40000 * 2);
	if (bytesRead == 0)
	{
		removeFromSet(cgi.pipeOut[0], _inFdSet);
		close(cgi.pipeIn[0]);
		close(cgi.pipeOut[0]);
		int status;
		waitpid(cgi.getCgiPid(), &status, 0);
		if (WEXITSTATUS(status) != 0)
			client.response.setErrorResponse(502);
		client.response.setCgiState(2);
		if (client.response.responseContent.find("HTTP/1.1") == std::string::npos)
			client.response.responseContent.insert(0, "HTTP/1.1 200 OK\r\n");
		return;
	}
	else if (bytesRead < 0)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while reading from CGI script. Reason: %s.", strerror(errno));
		removeFromSet(cgi.pipeOut[0], _inFdSet);
		close(cgi.pipeIn[0]);
		close(cgi.pipeOut[0]);
		client.response.setCgiState(2);
		client.response.setErrorResponse(500);
		return;
	}
	else
	{
		client.updateLastMessageTime();
		client.response.responseContent.append(buffer, bytesRead);
		memset(buffer, 0, sizeof(buffer));
	}
}

void Webserver::sendResponse(const int &i, Client &c)
{
	int bytesSent;
	std::string response = c.response.getRes();
	if (response.length() >= 40000)
		bytesSent = write(i, response.c_str(), 40000);
	else
		bytesSent = write(i, response.c_str(), response.length());
	if (bytesSent < 0)
	{
		PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while sending response. Reason: %s.", strerror(errno));
		closeConnection(i);
	}
	else if (bytesSent == 0 || (size_t)bytesSent == response.length())
	{
		PrintApp::printEvent(WHITE, SUCCESS, "Status<%d>", c.response.getCode());
		if (c.request.isConnectionKeepAlive() == false || c.request.errorCodes() || c.response.getCgiState())
		{
			PrintApp::printEvent(YELLOW, SUCCESS, "Connection with client %d has been terminated.", i);
			closeConnection(i);
		}
		else
		{
			removeFromSet(i, _outFdSet);
			addToSet(i, _inFdSet);
			c.clearClient();
		}
	}
	else
	{
		c.updateLastMessageTime();
		c.response.cutRes(bytesSent);
	}
}

void Webserver::handleClientTimeout()
{
	for (std::map<int, Client>::iterator it = _clientsDict.begin(); it != _clientsDict.end(); ++it)
	{
		if (time(NULL) - it->second.getLastMessageTime() > 30)
		{
			PrintApp::printEvent(YELLOW, SUCCESS, "The connection with Client %d has timed out. Closing the connection...", it->first);
			closeConnection(it->first);
			return;
		}
	}
}

void Webserver::closeConnection(const int fd)
{
	if (FD_ISSET(fd, &_outFdSet))
		removeFromSet(fd, _outFdSet);
	if (FD_ISSET(fd, &_inFdSet))
		removeFromSet(fd, _inFdSet);
	close(fd);
	_clientsDict.erase(fd);
}
