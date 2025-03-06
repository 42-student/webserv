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

			void					setupServer();
			void					setPort(const std::string& param);
			void					setHost(const std::string& param);
			void					setServerName(const std::string& servName);
			void					setRoot(const std::string& root);
			void					setIndex(const std::string& index);
			void					setAutoindex(const std::string& autoindex);
			void					setRedirect(const std::string& redirect);
			void					setClientMaxBodySize(const std::string& param);
			void					setFd(int fd);
			void					setErrorPages(const std::vector<std::string>& param);
			void					setLocation(const std::string& path, const std::vector<std::string>& param);
			void					initErrorPages();
			std::string				checkToken(const std::string& param);
			void					handleRootLocation(const std::vector<std::string>& param, size_t& i, Location& newLocation);
			void					handleIndexLocation(const std::vector<std::string>& param, size_t& i, Location& newLocation);
			void					handleAutoindex(const std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation, bool& flagAutoIndex);
			void					handleRedirect(const std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation);
			void					handleReturn(const std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation);
			void					handleAlias(const std::vector<std::string>& param, size_t& i, const std::string& path, Location& newLocation);
			void					handleMaxBodySize(const std::vector<std::string>& param, size_t& i, Location& newLocation, bool& flagMaxSize);
			void					handleAllowMethods(const std::vector<std::string>& param, size_t& i, Location& newLocation, bool& flagMethods);
			void					handleCgiExtension(const std::vector<std::string>& param, size_t& i, Location& newLocation);
			void					handleCgiPath(const std::vector<std::string>& param, size_t& i, Location& newLocation);
			const					std::vector<Location>& getLocations();
			const					uint16_t& getPort();
			const					in_addr_t& getHost();
			const					std::string& getServerName();
			const					std::string& getRoot();
			const					std::string& getIndex();
			const					bool& getAutoindex();
			const					std::string& getRedirect();
			const					size_t& getClientMaxBodySize();
			int						getFd();
			const					std::map<short, std::string>& getErrorPages();
			const					std::vector<Location>::iterator getLocationKey(std::string key);
			const					std::string& getPathErrorPage(short key);
			void					handleLocationDefaults(Location& newLocation, bool flagMaxSize);
			void					handleLocationValidation(int valid);
			int						isValidLocation(Location& location) const;
			int						isValidRegularLocation(Location& location) const;
			bool					isValidHost(std::string host) const;
			int						isValidCgiLocation(Location& location) const;
			bool					isValidErrorPages();
			bool					checkLocation() const;
			static std::string		statusCodeString(short statusCode);
			void					validateFile(const std::string& path);
			void					validateDirectory(const std::string& path);
			int						parseInt(const std::string& str, int min, int max);
			unsigned long			parseULong(const std::string& str, unsigned long max);
};

#endif
