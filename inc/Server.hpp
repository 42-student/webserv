#ifndef Server_HPP
#define Server_HPP

#include "utils.hpp"

class Location;

class Server
{
	private:
			struct sockaddr_in				_servAddr;
			std::vector<Location>			_locations;
			uint16_t						_port;
			in_addr_t						_host;
			std::string						_servName;
			std::string						_root;
			std::string						_index;
			bool							_autoIndex;
			std::string						_redirect;
			unsigned long					_maxBodySize;
			int								_listenFd;
			std::map<short, std::string>	_errPag;
			
	public:
			Server();
			Server(const Server &other);
			Server &operator=(const Server &rhs);
			~Server();

			void									setupServer();
			void									setLocation(std::string nameLocation, std::vector<std::string> parametr);
			void									setPort(std::string parametr);
			void									setHost(std::string parametr);
			void									setServerName(std::string server_name);
			void									setRoot(std::string root);
			void									setIndex(std::string index);
			void									setAutoindex(std::string autoindex);
			void									setRedirect(std::string redirect);
			void									setClientMaxBodySize(std::string parametr);
			void									setFd(int);
			void									setErrorPages(std::vector<std::string> &parametr);
			void									initErrorPages(void);	
			static std::string						statusCodeString(short);
			const std::vector<Location>				&getLocations();
			const std::vector<Location>::iterator	getLocationKey(std::string key);	
			const uint16_t							&getPort();
			const in_addr_t							&getHost();
			const std::string						&getServerName();
			const std::string						&getRoot();
			const std::string						&getIndex();
			const bool								&getAutoindex();
			const std::string						&getRedirect();
			const size_t							&getClientMaxBodySize();
			int										getFd();
			const std::map<short, std::string>		&getErrorPages();
			const std::string						&getPathErrorPage(short key);
			void									handleLocationDefaults(Location &newLocation, bool flagMaxSize);
			void									handleLocationValidation(int valid);	
			void									handleRootLocation(std::vector<std::string> &param, size_t &i, Location &newLocation);
			void									handleIndexLocation(std::vector<std::string> &param, size_t &i, Location &newLocation);
			void									handleAutoindex(std::vector<std::string> &param, size_t &i, const std::string &path, Location &newLocation, bool &flagAutoIndex);
			void									handleRedirect(std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation);
			void									handleReturn(std::vector<std::string> &param, size_t &i, const std::string &path, Location &newLocation);
			void									handleAlias(std::vector<std::string> &param, size_t &i, const std::string &path, Location &newLocation);
			void									handleMaxBodySize(std::vector<std::string> &param, size_t &i, Location &newLocation, bool &flagMaxSize);
			void									handleAllowMethods(std::vector<std::string> &param, size_t &i, Location &newLocation, bool &flagMethods);
			void									handleCgiExtension(std::vector<std::string> &param, size_t &i, Location &newLocation);
			void									handleCgiPath(std::vector<std::string> &param, size_t &i, Location &newLocation);
			int										isValidLocation(Location &location) const;	
			int										isValidRegularLocation(Location &location) const;
			bool									isValidHost(std::string host) const;
			int										isValidCgiLocation(Location &location) const;
			bool									isValidErrorPages();
			bool									checkLocation() const;	
			static void								checkToken(std::string &parametr);
};

#endif
