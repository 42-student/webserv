#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "utils.hpp"

class Client
{
	private:
			int					_cliSock;
			struct sockaddr_in	_cliSockAddr;
			time_t				_lastMessage;
			time_t				_cgiStartTime;

	public:
			Server		server;
			Request		request;
			Response	response;

			Client();
			Client(const Client &other);
			Client(Server &serv);
			Client &operator=(const Client &rhs);
			~Client();

			void			buildResponse();
			void			updateLastMessageTime();	
			void			setSocket(int &);
			int				getSocket();
			void			setServer(Server &);
			const time_t	&getLastMessageTime() const;
			void			setCgiStartTime(time_t start);
			time_t			getCgiStartTime() const;
			void			clearClient();
};

#endif
