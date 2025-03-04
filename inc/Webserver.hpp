#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Client.hpp"
#include "Response.hpp"
#include "utils.hpp"

class Webserver
{
	private:
			std::vector<struct pollfd>	_pollFds;
			std::vector<Server>			_serv;
			std::map<int, Server>		_servsDict;
			std::map<int, Client>		_clientsDict;
			int							_maxFd;
			std::string					_cgiOutBuff;

	public:
			Webserver();
			~Webserver();

			void	setupServers(std::vector<Server>);
			void	processServerRequests();
			void	initializePollFds();
			void	addToPoll(int fd, short events);
			void	removeFromPoll(int fd);
			void	acceptNewConnection(Server &);
			void	readAndProcessRequest(const int &, Client &);
			void	assignServer(Client &);
			bool	checkServ(Client &client, std::vector<Server>::iterator it);
			void	handleReqBody(Client &);
			void	sendCgiBody(Client &, Cgi &);
			void	readCgiResponse(Client &, Cgi &);
			void	sendResponse(const int &, Client &);
			void	handleClientTimeout();
			void	closeConnection(const int);
};

#endif
