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

	public:
			Server		server;
			Request		request;
			Response	response;

			Client();
			Client(const Client &other);
			Client(Server &);
			Client &operator=(const Client &rhs);
			~Client();

			void			buildResponse();
			void			updateLastMessageTime();	
			void			setSocket(int &);
			void			setServer(Server &);
			const time_t	&getLastMessageTime() const;	
			void			clearClient();

			int getSocket();
};

#endif
