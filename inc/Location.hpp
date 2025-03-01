#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "utils.hpp"

class Location
{
	private:
			std::string					_locPath;
			std::string					_locRoot;
			std::vector<short>			_methods;
			bool						_locAutoIndex;
			std::string					_locIndex;
			std::string					_returns;
			std::string					_alias;
			std::vector<std::string>	_cgiPath;
			std::vector<std::string>	_cgiExt;
			unsigned long				_maxBodySize;
			std::string					_locRedirect;

	public:
			std::map<std::string, std::string>	extPath;

			Location();
			Location(const Location &copy);
			Location &operator=(const Location &copy);
			~Location();

			void										setPath(std::string param);
			void										setRootLocation(std::string param);
			void										setMethods(std::vector<std::string> methods);
			void										setAutoindex(std::string param);
			void										setIndexLocation(std::string param);
			void										setReturn(std::string param);
			void										setAlias(std::string param);
			void										setCgiPath(std::vector<std::string> path);
			void										setCgiExtension(std::vector<std::string> extension);
			void										setMaxBodySize(std::string param);
			void										setMaxBodySize(unsigned long param);
			void										setRedirect(std::string param);
			const std::string							&getPath() const;
			const std::string							&getRootLocation() const;
			const std::vector<short>					&getMethods() const;
			const bool									&getAutoindex() const;
			const std::string							&getIndexLocation() const;
			const std::string							&getReturn() const;
			const std::string							&getAlias() const;
			const std::vector<std::string>				&getCgiPath() const;
			const std::vector<std::string>				&getCgiExtension() const;
			const unsigned long							&getMaxBodySize() const;
			const std::string							&getRedirect() const;
			const std::map<std::string, std::string>	&getExtensionPath() const;
};

#endif
