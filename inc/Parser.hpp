#ifndef PARSER_HPP
#define PARSER_HPP

#include "utils.hpp"

class Server;

class Parser {
	private:
			std::vector<Server>			_servList;
			std::vector<std::string>	_servConf;
			size_t						_nbOfServ;

	public:
			Parser();
			~Parser();

			int							parsServConf(const std::string &config_file);
			const std::string			extractServConf(std::string &fileContent);
			const std::string			removeComments(std::string &string);
			const std::string			removeSpaces(std::string &string);
			void						splitServers(std::string &content);
			size_t						findStartServ(size_t start, std::string &content);
			size_t						findEndServ(size_t start, std::string &content);
			void						createServFromConf(std::string &config, Server &server);
			std::vector<std::string>	splitParam(std::string inputStr, std::string delimeter);
			void						valServParamSize(const std::vector<std::string> &parameters);
			void						valDupPort(Server &server);
			void						valServScopeChar(const std::string &parameter);
			std::vector<std::string>	parsLocCodes(const std::vector<std::string> &parameters, size_t &i);
			void						valClosingBracket(const std::vector<std::string> &parameter, size_t &i);
			void						valDupHost(Server &server);
			void						valDupRoot(Server &server);
			void						processErrorCodes(const std::vector<std::string> &parameters, size_t &i, std::vector<std::string> &errorCodes);
			void						valDupMaxBodySize(bool isMaxSizeSet);
			void						valDupServName(Server &server);
			void						valDupIndex(Server &server);
			void						valDupRedirect(Server &server);
			void						valDupAutoindex(bool flag_autoindex);
			void						setDefaultServVal(Server &server);
			void						servValidations(Server &server);
			bool						checkIfServDup(Server &currentServer, Server &nextServer);
			void						checkDupServConf();
			std::vector<Server>			getServers();
};

#endif
