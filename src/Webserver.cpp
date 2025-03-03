// #include "../inc/Webserver.hpp"

// Webserver::Webserver() {}

// Webserver::~Webserver() {}

// void Webserver::setupServers(std::vector<Server> servers)
// {
// 	std::cout << "Initializing servers...\n";
// 	_serv = servers;	
// 	for (std::vector<Server>::iterator it = _serv.begin(); it != _serv.end(); ++it)
// 	{
// 		bool isSameServ = false;
// 		for (std::vector<Server>::iterator existingServerIt = _serv.begin(); existingServerIt != it; ++existingServerIt)
// 		{
// 			if (existingServerIt->getHost() == it->getHost() && existingServerIt->getPort() == it->getPort())
// 			{
// 				it->setFd(existingServerIt->getFd());
// 				isSameServ = true;
// 			}
// 		}
// 		if (!isSameServ)
// 			it->setupServer();
// 		PrintApp::printStartServer(*it);
// 	}
// }

// void Webserver::processServerRequests()
// {
// 	fd_set receivedFdCopy;
// 	fd_set writeFdCopy;
// 	int selectStatus;	
// 	_maxFd = 0;
// 	initializeSets();
// 	struct timeval timer;
// 	while (true)
// 	{
// 		timer.tv_sec = 1;
// 		timer.tv_usec = 0;
// 		receivedFdCopy = _inFdSet;
// 		writeFdCopy = _outFdSet;
// 		if ((selectStatus = select(_maxFd + 1, &receivedFdCopy, &writeFdCopy, NULL, &timer)) < 0)
// 			PrintApp::printEvent(RED_BOLD, FAILURE, "Error occurred during select operation. Reason: %s. Closing the connection...", strerror(errno));
// 		for (int i = 0; i <= _maxFd; ++i)
// 		{
// 			if (FD_ISSET(i, &receivedFdCopy) && _servsDict.count(i))
// 				acceptNewConnection(_servsDict.find(i)->second);
// 			else if (FD_ISSET(i, &receivedFdCopy) && _clientsDict.count(i))
// 				readAndProcessRequest(i, _clientsDict[i]);
// 			else if (FD_ISSET(i, &writeFdCopy) && _clientsDict.count(i))
// 			{
// 				int cgi_state = _clientsDict[i].response.getCgiState();
// 				if (cgi_state == 1 && FD_ISSET(_clientsDict[i].response.cgiObj.pipeIn[1], &writeFdCopy))
// 					sendCgiBody(_clientsDict[i], _clientsDict[i].response.cgiObj);
// 				else if (cgi_state == 1 && FD_ISSET(_clientsDict[i].response.cgiObj.pipeOut[0], &receivedFdCopy))
// 					readCgiResponse(_clientsDict[i], _clientsDict[i].response.cgiObj);
// 				else if ((cgi_state == 0 || cgi_state == 2) && FD_ISSET(i, &writeFdCopy))
// 					sendResponse(i, _clientsDict[i]);
// 			}
// 		}
// 		handleClientTimeout();
// 	}
// }

// void Webserver::initializeSets()
// {
// 	FD_ZERO(&_inFdSet);
// 	FD_ZERO(&_outFdSet);
// 	for (std::vector<Server>::iterator it = _serv.begin(); it != _serv.end(); ++it)
// 	{
// 		if (listen(it->getFd(), 512) == -1)
// 			PrintApp::printEvent(RED_BOLD, FAILURE, "Failed to listen on socket. Reason: %s. Closing the connection...", strerror(errno));
// 		if (fcntl(it->getFd(), F_SETFL, O_NONBLOCK) < 0)
// 			PrintApp::printEvent(RED_BOLD, FAILURE, "Error setting non-blocking mode using fcntl. Reason: %s. Closing the connection...", strerror(errno));
// 		addToSet(it->getFd(), _inFdSet);
// 		_servsDict.insert(std::make_pair(it->getFd(), *it));
// 	}
// 	_maxFd = _serv.back().getFd();
// }

// void Webserver::addToSet(const int fd, fd_set &set)
// {
// 	FD_SET(fd, &set);
// 	if (fd > _maxFd)
// 		_maxFd = fd;
// }

// void Webserver::removeFromSet(const int fd, fd_set &fdSet)
// {
// 	FD_CLR(fd, &fdSet);
// 	if (fd == _maxFd)
// 		_maxFd--;
// }

// void Webserver::acceptNewConnection(Server &serv)
// {
// 	struct sockaddr_in cliSocketAddr;
// 	long cliSocketAddrSize = sizeof(cliSocketAddr);
// 	int cliSocket;
// 	Client newClient(serv);
// 	if ((cliSocket = accept(serv.getFd(), (struct sockaddr *)&cliSocketAddr, (socklen_t *)&cliSocketAddrSize)) == -1)
// 	{
// 		PrintApp::printEvent(RED_BOLD, SUCCESS, "Encountered an error while accepting incoming connection. Reason: %s.", strerror(errno));
// 		return;
// 	}
// 	PrintApp::printEvent(GREEN_BOLD, SUCCESS, "New connection established.");
// 	addToSet(cliSocket, _inFdSet);
// 	if (fcntl(cliSocket, F_SETFL, O_NONBLOCK) < 0)
// 	{
// 		PrintApp::printEvent(RED_BOLD, FAILURE, "Error encountered while setting socket to non-blocking mode using fcntl. Reason: %s. Closing the connection...", strerror(errno));
// 		removeFromSet(cliSocket, _inFdSet);
// 		close(cliSocket);
// 		return;
// 	}	
// 	newClient.setSocket(cliSocket);
// 	if (_clientsDict.count(cliSocket) != 0)
// 		_clientsDict.erase(cliSocket);
// 	_clientsDict.insert(std::make_pair(cliSocket, newClient));
// }

// void Webserver::readAndProcessRequest(const int &i, Client &client)
// {
// 	char buffer[40000];
// 	int bytesRead = read(i, buffer, 40000);
// 	if (bytesRead == 0)
// 	{
// 		PrintApp::printEvent(YELLOW, SUCCESS, "Connection with client %d has been successfully closed.", i);
// 		closeConnection(i);
// 		return;
// 	}
// 	if (bytesRead < 0)
// 	{
// 		PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while reading from file descriptor %d. Reason: %s.", i, strerror(errno));
// 		closeConnection(i);
// 		return;
// 	}
// 	if (bytesRead)
// 	{
// 		client.updateLastMessageTime();
// 		client.request.parseHTTPRequestData(buffer, bytesRead);
// 		memset(buffer, 0, sizeof(buffer));
// 	}
// 	if (client.request.isParsingDone() || client.request.errorCodes())
// 	{
// 		assignServer(client);
// 		if (client.request.errorCodes() != 501)
// 			PrintApp::printEvent(BLUE, SUCCESS, "<%s> \"%s\"", client.request.getMethodStr().c_str(), client.request.getPath().c_str());
// 		client.buildResponse();
// 		if (client.response.getCgiState())
// 		{
// 			handleReqBody(client);
// 			addToSet(client.response.cgiObj.pipeIn[1], _outFdSet);
// 			addToSet(client.response.cgiObj.pipeOut[0], _inFdSet);
// 		}
// 		removeFromSet(i, _inFdSet);
// 		addToSet(i, _outFdSet);
// 	}
// }

// void Webserver::assignServer(Client &client)
// {
// 	for (std::vector<Server>::iterator it = _serv.begin(); it != _serv.end(); ++it)
// 	{
// 		if (checkServ(client, it))
// 			client.setServer(*it);
// 	}
// }

// bool Webserver::checkServ(Client &client, std::vector<Server>::iterator it)
// {
// 	bool s = client.request.getServerName() == it->getServerName();
// 	bool h = client.server.getHost() == it->getHost();
// 	bool p = client.server.getPort() == it->getPort();
// 	return s && h && p;
// }

// void Webserver::handleReqBody(Client &client)
// {
// 	if (client.request.getBody().length() == 0)
// 	{
// 		std::string tmp;
// 		std::fstream file;
// 		(client.response.cgiObj.getCgiPath().c_str());
// 		std::stringstream ss;
// 		ss << file.rdbuf();
// 		tmp = ss.str();
// 		client.request.setBody(tmp);
// 	}
// }

// void Webserver::sendCgiBody(Client &client, Cgi &cgi)
// {
// 	int bytesSent;
// 	std::string &req_body = client.request.getBody();	
// 	if (req_body.empty())
// 		bytesSent = 0;
// 	else if (req_body.length() >= 40000)
// 		bytesSent = write(cgi.pipeIn[1], req_body.c_str(), 40000);
// 	else
// 		bytesSent = write(cgi.pipeIn[1], req_body.c_str(), req_body.length());	
// 	if (bytesSent < 0)
// 	{
// 		PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while sending CGI body. Reason: %s.", strerror(errno));
// 		removeFromSet(cgi.pipeIn[1], _outFdSet);
// 		close(cgi.pipeIn[1]);
// 		close(cgi.pipeOut[1]);
// 		client.response.setErrorResponse(500);
// 	}
// 	else if (bytesSent == 0 || static_cast<size_t>(bytesSent) == req_body.length())
// 	{
// 		removeFromSet(cgi.pipeIn[1], _outFdSet);
// 		close(cgi.pipeIn[1]);
// 		close(cgi.pipeOut[1]);
// 	}
// 	else
// 	{
// 		client.updateLastMessageTime();
// 		req_body = req_body.substr(bytesSent);
// 	}
// }

// void Webserver::readCgiResponse(Client &client, Cgi &cgi)
// {
// 	char buffer[40000 * 2];
// 	int bytesRead = 0;
// 	bytesRead = read(cgi.pipeOut[0], buffer, 40000 * 2);
// 	if (bytesRead == 0)
// 	{
// 		removeFromSet(cgi.pipeOut[0], _inFdSet);
// 		close(cgi.pipeIn[0]);
// 		close(cgi.pipeOut[0]);
// 		int status;
// 		waitpid(cgi.getCgiPid(), &status, 0);
// 		if (WEXITSTATUS(status) != 0)
// 			client.response.setErrorResponse(502);
// 		client.response.setCgiState(2);
// 		if (client.response.responseContent.find("HTTP/1.1") == std::string::npos)
// 			client.response.responseContent.insert(0, "HTTP/1.1 200 OK\r\n");
// 		return;
// 	}
// 	else if (bytesRead < 0)
// 	{
// 		PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while reading from CGI script. Reason: %s.", strerror(errno));
// 		removeFromSet(cgi.pipeOut[0], _inFdSet);
// 		close(cgi.pipeIn[0]);
// 		close(cgi.pipeOut[0]);
// 		client.response.setCgiState(2);
// 		client.response.setErrorResponse(500);
// 		return;
// 	}
// 	else
// 	{
// 		client.updateLastMessageTime();
// 		client.response.responseContent.append(buffer, bytesRead);
// 		memset(buffer, 0, sizeof(buffer));
// 	}
// }

// void Webserver::sendResponse(const int &i, Client &c)
// {
// 	int bytesSent;
// 	std::string response = c.response.getRes();
// 	if (response.length() >= 40000)
// 		bytesSent = write(i, response.c_str(), 40000);
// 	else
// 		bytesSent = write(i, response.c_str(), response.length());
// 	if (bytesSent < 0)
// 	{
// 		PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while sending response. Reason: %s.", strerror(errno));
// 		closeConnection(i);
// 	}
// 	else if (bytesSent == 0 || (size_t)bytesSent == response.length())
// 	{
// 		PrintApp::printEvent(WHITE, SUCCESS, "Status<%d>", c.response.getCode());
// 		if (c.request.isConnectionKeepAlive() == false || c.request.errorCodes() || c.response.getCgiState())
// 		{
// 			PrintApp::printEvent(YELLOW, SUCCESS, "Connection with client %d has been terminated.", i);
// 			closeConnection(i);
// 		}
// 		else
// 		{
// 			removeFromSet(i, _outFdSet);
// 			addToSet(i, _inFdSet);
// 			c.clearClient();
// 		}
// 	}
// 	else
// 	{
// 		c.updateLastMessageTime();
// 		c.response.cutRes(bytesSent);
// 	}
// }

// void Webserver::handleClientTimeout()
// {
// 	for (std::map<int, Client>::iterator it = _clientsDict.begin(); it != _clientsDict.end(); ++it)
// 	{
// 		if (time(NULL) - it->second.getLastMessageTime() > 30)
// 		{
// 			PrintApp::printEvent(YELLOW, SUCCESS, "The connection with Client %d has timed out. Closing the connection...", it->first);
// 			closeConnection(it->first);
// 			return;
// 		}
// 	}
// }

// void Webserver::closeConnection(const int fd)
// {
// 	if (FD_ISSET(fd, &_outFdSet))
// 		removeFromSet(fd, _outFdSet);
// 	if (FD_ISSET(fd, &_inFdSet))
// 		removeFromSet(fd, _inFdSet);
// 	close(fd);
// 	_clientsDict.erase(fd);
// }



#include "../inc/Webserver.hpp"

Webserver::Webserver() : _maxFd(0) {}

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
    int pollStatus;
    initializePollFds();

    while (true)
    {
        pollStatus = poll(&_pollFds[0], _pollFds.size(), 1000); // 1000ms timeout
        if (pollStatus < 0)
            PrintApp::printEvent(RED_BOLD, FAILURE, "Error occurred during poll operation. Reason: %s. Closing the connection...", strerror(errno));
        if (pollStatus == 0)
        {
            handleClientTimeout();
            continue;
        }

        // Process events in a controlled order to mimic select() behavior
        std::vector<struct pollfd> currentFds = _pollFds; // Copy to avoid modification issues during iteration
        for (size_t i = 0; i < currentFds.size(); ++i)
        {
            int fd = currentFds[i].fd;
            if (currentFds[i].revents & POLLIN)
            {
                if (_servsDict.count(fd))
                    acceptNewConnection(_servsDict.find(fd)->second);
                else if (_clientsDict.count(fd))
                {
                    Client& client = _clientsDict[fd];
                    if (client.response.getCgiState() == 1 && fd == client.response.cgiObj.pipeOut[0])
                        readCgiResponse(client, client.response.cgiObj);
                    else
                        readAndProcessRequest(fd, client);
                }
            }
            if (currentFds[i].revents & POLLOUT)
            {
                if (_clientsDict.count(fd))
                {
                    Client& client = _clientsDict[fd];
                    int cgi_state = client.response.getCgiState();
                    if (cgi_state == 1 && fd == client.response.cgiObj.pipeIn[1])
                        sendCgiBody(client, client.response.cgiObj);
                    else if (cgi_state == 0 || cgi_state == 2)
                        sendResponse(fd, client);
                }
            }
            if (currentFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                if (_clientsDict.count(fd))
                {
                    Client& client = _clientsDict[fd];
                    if (client.response.getCgiState() == 1 && (fd == client.response.cgiObj.pipeIn[1] || fd == client.response.cgiObj.pipeOut[0]))
                    {
                        // Handle CGI pipe errors gracefully
                        client.response.setErrorResponse(500);
                        client.response.setCgiState(2);
                        removeFromPoll(fd);
                        close(fd);
                    }
                    else
                        closeConnection(fd);
                }
            }
        }
        handleClientTimeout();
    }
}

void Webserver::initializePollFds()
{
    _pollFds.clear();
    for (std::vector<Server>::iterator it = _serv.begin(); it != _serv.end(); ++it)
    {
        if (listen(it->getFd(), 512) == -1)
            PrintApp::printEvent(RED_BOLD, FAILURE, "Failed to listen on socket. Reason: %s. Closing the connection...", strerror(errno));
        if (fcntl(it->getFd(), F_SETFL, O_NONBLOCK) < 0)
            PrintApp::printEvent(RED_BOLD, FAILURE, "Error setting non-blocking mode using fcntl. Reason: %s. Closing the connection...", strerror(errno));
        addToPoll(it->getFd(), POLLIN);
        _servsDict.insert(std::make_pair(it->getFd(), *it));
    }
    _maxFd = _serv.back().getFd();
}

void Webserver::addToPoll(int fd, short events)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    _pollFds.push_back(pfd);
    if (fd > _maxFd)
        _maxFd = fd;
}

void Webserver::removeFromPoll(int fd)
{
    for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _pollFds.erase(it);
            break;
        }
    }
    if (fd == _maxFd)
    {
        _maxFd = 0;
        for (size_t i = 0; i < _pollFds.size(); ++i)
            if (_pollFds[i].fd > _maxFd)
                _maxFd = _pollFds[i].fd;
    }
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
    addToPoll(cliSocket, POLLIN);
    if (fcntl(cliSocket, F_SETFL, O_NONBLOCK) < 0)
    {
        PrintApp::printEvent(RED_BOLD, FAILURE, "Error encountered while setting socket to non-blocking mode using fcntl. Reason: %s. Closing the connection...", strerror(errno));
        removeFromPoll(cliSocket);
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
            addToPoll(client.response.cgiObj.pipeIn[1], POLLOUT);
            addToPoll(client.response.cgiObj.pipeOut[0], POLLIN);
            removeFromPoll(i); // Remove client FD from POLLIN until CGI is done
        }
        else
        {
            removeFromPoll(i);
            addToPoll(i, POLLOUT);
        }
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

// void Webserver::handleReqBody(Client &client)
// {
//     if (client.request.getBody().length() == 0)
//     {
//         std::string tmp;
//         std::fstream file;
//         (client.response.cgiObj.getCgiPath().c_str()); // Assuming this is meant to open a file, but seems incomplete
//         std::stringstream ss;
//         ss << file.rdbuf();
//         tmp = ss.str();
//         client.request.setBody(tmp);
//     }
// }

void Webserver::handleReqBody(Client &client)
{
    if (client.request.getBody().length() == 0 && client.request.getHttpMethod() == POST) // Assuming POST needs a body
    {
        std::string tmp;
        std::fstream file(client.response.cgiObj.getCgiPath().c_str(), std::ios::in);
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            tmp = ss.str();
            file.close();
            client.request.setBody(tmp);
        }
        else
        {
            PrintApp::printEvent(RED_BOLD, SUCCESS, "Failed to open CGI script file for default body.");
            client.response.setErrorResponse(500);
            client.response.setCgiState(2); // Skip CGI if file can’t be read
        }
    }
}

// void Webserver::sendCgiBody(Client &client, Cgi &cgi)
// {
//     int bytesSent;
//     std::string &req_body = client.request.getBody();
//     if (req_body.empty())
//         bytesSent = 0;
//     else if (req_body.length() >= 40000)
//         bytesSent = write(cgi.pipeIn[1], req_body.c_str(), 40000);
//     else
//         bytesSent = write(cgi.pipeIn[1], req_body.c_str(), req_body.length());
//     if (bytesSent < 0)
//     {
//         PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while sending CGI body. Reason: %s.", strerror(errno));
//         removeFromPoll(cgi.pipeIn[1]);
//         close(cgi.pipeIn[1]);
//         close(cgi.pipeOut[1]);
//         client.response.setErrorResponse(500);
//         client.response.setCgiState(2);
//     }
//     else if (bytesSent == 0 || static_cast<size_t>(bytesSent) == req_body.length())
//     {
//         removeFromPoll(cgi.pipeIn[1]);
//         close(cgi.pipeIn[1]);
//         close(cgi.pipeOut[1]);
//     }
//     else
//     {
//         client.updateLastMessageTime();
//         req_body = req_body.substr(bytesSent);
//     }
// }

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
        removeFromPoll(cgi.pipeIn[1]);
        close(cgi.pipeIn[1]);
        client.response.setErrorResponse(500);
        client.response.setCgiState(2);
    }
    else if (bytesSent == 0 || static_cast<size_t>(bytesSent) == req_body.length())
    {
        removeFromPoll(cgi.pipeIn[1]);
        close(cgi.pipeIn[1]);
    }
    else
    {
        client.updateLastMessageTime();
        req_body = req_body.substr(bytesSent);
    }
}

// void Webserver::readCgiResponse(Client &client, Cgi &cgi)
// {
//     char buffer[40000 * 2];
//     int bytesRead = read(cgi.pipeOut[0], buffer, 40000 * 2);
//     if (bytesRead == 0)
//     {
//         removeFromPoll(cgi.pipeOut[0]);
//         close(cgi.pipeIn[0]);
//         close(cgi.pipeOut[0]);
//         int status;
//         waitpid(cgi.getCgiPid(), &status, 0);
//         if (WEXITSTATUS(status) != 0)
//             client.response.setErrorResponse(502);
//         client.response.setCgiState(2);
//         if (client.response.responseContent.find("HTTP/1.1") == std::string::npos)
//             client.response.responseContent.insert(0, "HTTP/1.1 200 OK\r\n");
//         addToPoll(client.getSocket(), POLLOUT); // Re-add client FD for sending response
//     }
//     else if (bytesRead < 0)
//     {
//         PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while reading from CGI script. Reason: %s.", strerror(errno));
//         removeFromPoll(cgi.pipeOut[0]);
//         close(cgi.pipeIn[0]);
//         close(cgi.pipeOut[0]);
//         client.response.setCgiState(2);
//         client.response.setErrorResponse(500);
//         addToPoll(client.getSocket(), POLLOUT);
//     }
//     else
//     {
//         client.updateLastMessageTime();
//         client.response.responseContent.append(buffer, bytesRead);
//         memset(buffer, 0, sizeof(buffer));
//     }
// }

// void Webserver::readCgiResponse(Client &client, Cgi &cgi)
// {
//     char buffer[40000 * 2];
//     int bytesRead = read(cgi.pipeOut[0], buffer, 40000 * 2);
//     if (bytesRead == 0)
//     {
//         removeFromPoll(cgi.pipeOut[0]);
//         close(cgi.pipeOut[0]);
//         int status;
//         waitpid(cgi.getCgiPid(), &status, 0);
//         if (WEXITSTATUS(status) != 0)
//             client.response.setErrorResponse(502);
//         client.response.setCgiState(2);
//         if (client.response.responseContent.find("HTTP/1.1") == std::string::npos)
//             client.response.responseContent.insert(0, "HTTP/1.1 200 OK\r\n");
//         addToPoll(client.getSocket(), POLLOUT);
//     }
//     else if (bytesRead < 0)
//     {
//         PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while reading from CGI script. Reason: %s.", strerror(errno));
//         removeFromPoll(cgi.pipeOut[0]);
//         close(cgi.pipeOut[0]);
//         client.response.setCgiState(2);
//         client.response.setErrorResponse(500);
//         addToPoll(client.getSocket(), POLLOUT);
//     }
//     else
//     {
//         client.updateLastMessageTime();
//         client.response.responseContent.append(buffer, bytesRead);
//         memset(buffer, 0, sizeof(buffer));
//     }
// }

void Webserver::readCgiResponse(Client &client, Cgi &cgi)
{
    char buffer[40000 * 2];
    int bytesRead = read(cgi.pipeOut[0], buffer, 40000 * 2);
    if (bytesRead == 0)
    {
        // Check if CGI process has exited
        int status;
        pid_t result = waitpid(cgi.getCgiPid(), &status, WNOHANG);
        if (result == cgi.getCgiPid())
        {
            removeFromPoll(cgi.pipeOut[0]);
            close(cgi.pipeOut[0]);
            if (WEXITSTATUS(status) != 0)
                client.response.setErrorResponse(502);
            client.response.setCgiState(2);
            if (client.response.responseContent.find("HTTP/1.1") == std::string::npos)
                client.response.responseContent.insert(0, "HTTP/1.1 200 OK\r\n");
            addToPoll(client.getSocket(), POLLOUT);
        }
    }
    else if (bytesRead < 0)
    {
        PrintApp::printEvent(RED_BOLD, SUCCESS, "Error occurred while reading from CGI script. Reason: %s.", strerror(errno));
        removeFromPoll(cgi.pipeOut[0]);
        close(cgi.pipeOut[0]);
        client.response.setCgiState(2);
        client.response.setErrorResponse(500);
        addToPoll(client.getSocket(), POLLOUT);
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
    else if (bytesSent == 0 || static_cast<size_t>(bytesSent) == response.length())
    {
        PrintApp::printEvent(WHITE, SUCCESS, "Status<%d>", c.response.getCode());
        if (c.request.isConnectionKeepAlive() == false || c.request.errorCodes() || c.response.getCgiState())
        {
            PrintApp::printEvent(YELLOW, SUCCESS, "Connection with client %d has been terminated.", i);
            closeConnection(i);
        }
        else
        {
            removeFromPoll(i);
            addToPoll(i, POLLIN);
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
    for (std::map<int, Client>::iterator it = _clientsDict.begin(); it != _clientsDict.end();)
    {
        if (time(NULL) - it->second.getLastMessageTime() > 30)
        {
            PrintApp::printEvent(YELLOW, SUCCESS, "The connection with Client %d has timed out. Closing the connection...", it->first);
            int fd = it->first;
            ++it; // Increment before closing to avoid iterator invalidation
            closeConnection(fd);
        }
        else
            ++it;
    }
}

void Webserver::closeConnection(const int fd)
{
    removeFromPoll(fd);
    close(fd);
    _clientsDict.erase(fd);
}
