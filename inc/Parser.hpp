#ifndef PARSER_HPP
#define PARSER_HPP

#include "utils.hpp"

class Server;

class Parser
{
	private:
			std::vector<Server>			_servList;
			std::vector<std::string>	_servConf;
			size_t						_servNbs;

			std::string					trimWhitespace(const std::string& str) const;
			std::string					removeComments(const std::string& str) const;
			void						extractServerBlocks(const std::string& content);
			size_t						findServerStart(size_t pos, const std::string& content) const;
			size_t						findServerEnd(size_t pos, const std::string& content) const;
			Server						parsServConf(const std::string& config);
			std::vector<std::string>	tokenize(const std::string& str, const std::string& delimiters) const;
			void						validateParameterCount(const std::vector<std::string>& params) const;
			void						processDirective(const std::string& directive, size_t& i, const std::vector<std::string>& params, Server& server,
														std::vector<std::string>& errorCodes, int& locationFlag, bool& isAutoindexSet, bool& isMaxSizeSet);
			void						checkDuplicatePort(Server& server);
			void						checkDuplicateHost(Server& server);
			void						checkDuplicateRoot(Server& server);
			void						checkDuplicateMaxBodySize(bool isSet) const;
			void						checkDuplicateServerName(Server& server);
			void						checkDuplicateIndex(Server& server);
			void						checkDuplicateRedirect(Server& server);
			void						checkDuplicateAutoindex(bool isSet) const;
			void						validateScopeChar(const std::string& param) const;
			void						validateClosingBracket(const std::vector<std::string>& params, size_t i) const;
			void						setDefaultServerValues(Server& server);
			void						validateServer(Server& server);
			bool						isDuplicateServer(Server& curr, Server& next);
			void						checkDuplicateServers();
			
	public:
			Parser();
			~Parser();

			int						parseServerConfig(const std::string& filePath);
			std::vector<Server>		getServers() const;
};

#endif
