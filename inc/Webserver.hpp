// #ifndef WEBSERVER_HPP
// #define WEBSERVER_HPP

// #include "Client.hpp"
// #include "Response.hpp"
// #include "utils.hpp"

// class Webserver
// {
// 	private:
// 			std::vector<Server>		_serv;
// 			std::map<int, Server>	_servsDict;
// 			std::map<int, Client>	_clientsDict;
// 			fd_set					_inFdSet;
// 			fd_set					_outFdSet;
// 			int						_maxFd;

			
// 	public:
// 			Webserver();
// 			~Webserver();
			
// 			void	setupServers(std::vector<Server>);
// 			void	processServerRequests();
// 			void	initializeSets();
// 			void	addToSet(const int, fd_set &);
// 			void	removeFromSet(const int, fd_set &);	
// 			void	acceptNewConnection(Server &);
// 			void	readAndProcessRequest(const int &, Client &);
// 			void	assignServer(Client &);
// 			bool	checkServ(Client &client, std::vector<Server>::iterator it);
// 			void	handleReqBody(Client &);
// 			void	sendCgiBody(Client &, Cgi &);
// 			void	readCgiResponse(Client &, Cgi &);
// 			void	sendResponse(const int &, Client &);
// 			void	handleClientTimeout();
// 			void	closeConnection(const int);

// 		};

// #endif



#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Client.hpp"
#include "Response.hpp"
#include "utils.hpp"
#include <vector>
#include <map>
#include <poll.h> // Include for poll()

class Webserver
{
private:
    std::vector<Server>     _serv;
    std::map<int, Server>   _servsDict;
    std::map<int, Client>   _clientsDict;
    std::vector<struct pollfd> _pollFds; // Vector for pollfd structures
    int                     _maxFd;       // Still useful for tracking max fd

public:
    Webserver();
    ~Webserver();

    void setupServers(std::vector<Server>);
    void processServerRequests();
    void initializePollFds();               // Replaces initializeSets
    void addToPoll(int fd, short events);   // Replaces addToSet
    void removeFromPoll(int fd);            // Replaces removeFromSet
    void acceptNewConnection(Server &);
    void readAndProcessRequest(const int &, Client &);
    void assignServer(Client &);
    bool checkServ(Client &client, std::vector<Server>::iterator it);
    void handleReqBody(Client &);
    void sendCgiBody(Client &, Cgi &);
    void readCgiResponse(Client &, Cgi &);
    void sendResponse(const int &, Client &);
    void handleClientTimeout();
    void closeConnection(const int);
};

#endif
