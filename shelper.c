/**
 * ISA 2019 HTTP nastenka
 * Autro: Kristian Liscinsky
 * xlisci01
 */

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <list>
#include <map>
#include "shelper.h"

//data structure for saving boards
map <string, list <string>> zoznamNastenok;

/**
 *	Function for printing server help message
 */
void printHelpServer() {
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
	cout << "++ 				Launch server application					++" << endl;
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
	cout << "++ 	./isaserver -p <port>									++" << endl;
	cout << "++		server is listening to port number: <port>					++" << endl;
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
}

/********************************************************************************************************
*				SENDIGN/READING DATA							*
* ******************************************************************************************************/

/**
 * 	Sending response to client
 */
void sendResponse(int socket, string response) {
	int read = 0;
	int length = response.size();
	const char *data = response.data();

	while (length > 0) {
		read = send(socket, data, length, 0);
		if (read < 0) {
			cerr << "Error while sending response to client" << endl;
		}
		data += read;
		length -= read;
	}
}

/**
 * 	Recieving data from cther side
 */
string recvMessage(int socket) {
	char buffer[ONE_CHAR + 1];
	string result;
	//recieve header
	while(true) {
		memset(buffer, 0, ONE_CHAR + 1);
		int read = recv(socket, buffer, ONE_CHAR, 0);
		if (read == 0) break; //reading done
		if (read < 0) {
			cerr << "Error while reading header" << endl;
			break;
		}
		result += buffer;
		if (result.find("\r\n\r\n") != string::npos) break;//end of header
	}

	size_t delimitterPos;
	delimitterPos = result.find("\r\n\r\n");
	//request contains \r\n\r\n if not, then we have whole request in result variable
	if (delimitterPos != string::npos) {
		string testContent = result.substr(0, delimitterPos);
		struct ParsedRequest testingRequest =  parseHeaderAndRequestLine(testContent);

		//request contains Content-Length header an it is number (need to find out because stoi throw exception otherwise)
		if(!testingRequest.contentLength.empty() && is_number(testingRequest.contentLength)) {
			int contentLength = stoi(testingRequest.contentLength);
			char buffer[BUFFER_SIZE+1];
			int total = 0;

			while (total < contentLength) {
				memset(buffer, 0, BUFFER_SIZE + 1);
				int read = recv(socket, buffer, BUFFER_SIZE, 0);
				if (!read) break;//reading done
				if (read < 0) {
					cerr << "Eror while reading content" << endl;
					break;
				}
				total += read;
				result += buffer;
			}
		//if request does not contain Content-Length header, we expect, there is no content
		} else {
			return result;
		}
	}
	return result;
}

/********************************************************************************************************
*				PARSING SERVER ARGUMENTS						*
* ******************************************************************************************************/

/**
 * 	Parse argument of server
 * 	return port number if success
 * 	exit program is fail
 */
int parseArguments(int argc, char *argv[]) {
	if (argc == 2 && strcmp(argv[1], "-h") == 0) {
		printHelpServer();
		exit(0);
	} else if (argc == 3 && strcmp(argv[1], "-p") == 0) {
		if (!is_number(argv[2])) {
			cerr << "Error: port is not a number" << endl;
			exit(-1);
		}
		int port = stoi(argv[2]);
		return port;
	} else {
		cerr << "Error, Wrong argument, try -h for help" << endl;
		exit(EXIT_FAILURE);
	}
}

/********************************************************************************************************
*				CREATING HTTP RESPONSE							*
* ******************************************************************************************************/

/**
 * 	Create HTTP/1.1 response
 */
string createResponse(struct ResponseBuilder responseBuilder) {
	string response;
	response = "HTTP/1.1 " + to_string(responseBuilder.returnCode) + " " + responseBuilder.reasonPhrase + "\r\n";
	
	char str[40];
	time_t now = time(NULL);
	strftime(str, 40, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&now));
	string date = str;
	response += "Date: " + date + "\r\n";
	response += "Content-Type: text/plain\r\n";
	response += "Content-Length: " + to_string(responseBuilder.content.size()) + "\r\n";
	response += "\r\n";

	if (!responseBuilder.content.empty()) {
		response += responseBuilder.content;
	}

	return response;
}

/**
 * 	Add HTTP code to response according convention
 */
struct ResponseBuilder addReasonPhraseToResponse(struct ResponseBuilder responseBuilder) {
	if (responseBuilder.returnCode == 200) {
		responseBuilder.reasonPhrase = "OK";
		return responseBuilder;
	} else if (responseBuilder.returnCode == 201) {
		responseBuilder.reasonPhrase = "Created";
		return responseBuilder;
	} else if (responseBuilder.returnCode == 400) {
		responseBuilder.reasonPhrase = "Bad Request";
		return responseBuilder;
	} else if (responseBuilder.returnCode == 404) {
		responseBuilder.reasonPhrase = "Not Found";
		return responseBuilder;
	} else if (responseBuilder.returnCode == 409) {
		responseBuilder.reasonPhrase = "Conflict";
		return responseBuilder;
	} else {
		return responseBuilder;
	}
}

/********************************************************************************************************
*				HANDLING WITH BOARDS							*
* ******************************************************************************************************/

/**
 * 	GET method to get all available boards
 */
struct ResponseBuilder boards() {
	struct ResponseBuilder responseBuilder;

	if (zoznamNastenok.empty()){
		responseBuilder.returnCode = 200;
		responseBuilder.content = "";
		return responseBuilder;
	}

	for (auto i : zoznamNastenok) {
		responseBuilder.content +=  i.first + '\n';
	}

	int lastIndex = responseBuilder.content.length() - 1;
	char lastChar = responseBuilder.content[lastIndex];
	if (lastChar == '\n') {
		responseBuilder.content = responseBuilder.content.substr(0, responseBuilder.content.length() - 1);
	}

	responseBuilder.returnCode = 200;
	return responseBuilder;
}

/**
 * 	GET method for get content of board <name>
 */
struct ResponseBuilder board_list(string name) {
	struct ResponseBuilder responseBuilder;
	string content;
	map <string, list<string>>::iterator it;

	//if board <name> does not exist
	it = zoznamNastenok.find(name);
	if (it == zoznamNastenok.end()) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Board named " + name + " does not exist";
		return responseBuilder;
	}

	//if board is empty
	if (zoznamNastenok.find(name)->second.empty()) {
		responseBuilder.returnCode = 200;
		responseBuilder.content = "";
		return responseBuilder;
	}

	int place = 1;
	for (auto i : zoznamNastenok.find(name)->second) {
		content += to_string(place) + ". " +  i + '\n';
		place++;
	}

	int lastIndex = content.length() - 1;
	char lastChar = content[lastIndex];
	if (lastChar == '\n') {
		content = content.substr(0, content.length() - 1);
	}

	responseBuilder.returnCode = 200;
	responseBuilder.content = content;
	return responseBuilder;
}

/**
 * 	POST method for creat new board
 */ 
struct ResponseBuilder board_add(string name) {
	struct ResponseBuilder responseBuilder;
	list <string> newList;
	//if board with name <name> already exists
	if(zoznamNastenok.find(name) != zoznamNastenok.end()) {
		responseBuilder.returnCode = 409;
		responseBuilder.content = "Board named " + name + " already exists";
		return responseBuilder;
	}

	zoznamNastenok.insert(pair<string, list<string>>(name, newList));
	responseBuilder.returnCode = 201;
	responseBuilder.content = "Board " + name + " has been added";
	return responseBuilder;
}

/**
 * 	POST method for add content to existing board
 */
struct ResponseBuilder item_add(string name, string content) {
	struct ResponseBuilder responseBuilder;

	//if board with name <name> does not exist
	if(zoznamNastenok.find(name) == zoznamNastenok.end()) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Board named " + name + " does not exist";
		return responseBuilder;
	}

	//no content
	if (content.empty()) {
		responseBuilder.returnCode = 400;
		responseBuilder.content = "Trying to add empty content";
		return responseBuilder;
	}

	zoznamNastenok.find(name)->second.push_back(content);
	responseBuilder.returnCode = 201;
	responseBuilder.content = "Item has been added to the board " + name;
	return responseBuilder;
}

/**
 * 	PUT method for update content of board
 */
struct ResponseBuilder item_update(string name, string id, string content) {
	struct ResponseBuilder responseBuilder;
	if (!is_number(id)) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Item with ID " + id + " does not exist in board " + name;
		return responseBuilder;
	}

	unsigned long idPrispevku = stoul(id);
	
	//if board with name <name> does not exist
	if(zoznamNastenok.find(name) == zoznamNastenok.end()) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Board named " + name + " does not exist";
		return responseBuilder;
	}

	//if item with id <id> does not exist
	if(zoznamNastenok.find(name)->second.size() < idPrispevku) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Item with ID " + id + " does not exist in board " + name;
		return responseBuilder;
	}

	//no content
	if (content.empty()) {
		responseBuilder.returnCode = 400;
		responseBuilder.content = "Trying to update with empty content";
		return responseBuilder;
	}

	//updating
	list<string>::iterator it = zoznamNastenok.find(name)->second.begin();
	advance(it, idPrispevku - 1);
	it = zoznamNastenok.find(name)->second.erase(it);
	zoznamNastenok.find(name)->second.insert(it, content);
	responseBuilder.returnCode = 200;
	responseBuilder.content = "Item with ID " + id + " in board " + name + " has been changed";
	return responseBuilder;
}

/**
 * 	DELETE method for deleting board with name <name>
 */
struct ResponseBuilder board_delete(string name) {
	struct ResponseBuilder responseBuilder;

	//if board with name <name> does not exist
	if (zoznamNastenok.find(name) == zoznamNastenok.end()) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Board named " + name + " does not exist";
		return responseBuilder;
	}

	map<string, list<string>>::iterator it;
	it = zoznamNastenok.find(name);
	zoznamNastenok.erase(it);

	responseBuilder.returnCode = 200;
	responseBuilder.content = "Board " + name + " has been deleted";
	return responseBuilder;
}

/**
 * 	DELETE method for deleting item from board
 */
struct ResponseBuilder item_delete(string name, string id) {
	struct ResponseBuilder responseBuilder;
	if (!is_number(id)) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Item with ID " + id + " does not exist in board " + name;
		return responseBuilder;
	}

	unsigned long idPrispevku = stoul(id);

	//if board with name <name> does not exist
	if (zoznamNastenok.find(name) == zoznamNastenok.end()) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Board named " + name + " does not exist";
		return responseBuilder;
	}

	//if item with id <id> does not exist
	if (zoznamNastenok.find(name)->second.size() < idPrispevku) {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Item with ID " + id + " does not exist in board " + name;
		return responseBuilder;
	}

	list<string>::iterator it = zoznamNastenok.find(name)->second.begin();
	advance(it, idPrispevku - 1);
	zoznamNastenok.find(name)->second.erase(it);

	responseBuilder.returnCode = 200;
	responseBuilder.content = "Item with ID " + id + " has been deleted from board " + name;
	return responseBuilder;
}

/**
 * 	Find out if string is a number
 */
bool is_number(const std::string& s) {
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

/********************************************************************************************************
*				PARSING REQUEST								*
* ******************************************************************************************************/

/**
 * 	Function for parsing request and processing information from request
 */
struct ResponseBuilder processRequest(struct ParsedRequest parsedRequest) {
	struct ResponseBuilder responseBuilder;

	if (parsedRequest.command == "GET") {
		struct UrlParse url = parseGETUrl(parsedRequest.url);
		if (url.isUrlOk) {
			if (url.board == "boards") {
				responseBuilder = boards();
				return responseBuilder;
			} else if (url.board == "board") {
				responseBuilder = board_list(url.name);
				return responseBuilder;
			} else {
				responseBuilder.returnCode = 404;
				responseBuilder.content = "Url in request is unknown";
				return responseBuilder;
			}
		} else {
			responseBuilder.returnCode = 404;
			responseBuilder.content = "Not found";
			return responseBuilder;
		}
	} else if (parsedRequest.command == "POST") {

		struct UrlParse url = parsePOSTUrl(parsedRequest.url);
		if (url.isUrlOk) {
			if (url.board == "boards") {
				responseBuilder = board_add(url.name);
				return responseBuilder;
			} else if (url.board == "board") {
				//PUT or POST above item which has content-length = 0 returns 400
				if (parsedRequest.content.empty()) {
					responseBuilder.returnCode = 400;
					responseBuilder.content = "Trying to add empty content";
					return responseBuilder;
				}

				responseBuilder = item_add(url.name, parsedRequest.content);
				return responseBuilder;
			} else {
				responseBuilder.returnCode = 404;
				responseBuilder.content = "Url in request is unknown";
				return responseBuilder;
			}
		} else {
			responseBuilder.returnCode = 404;
			responseBuilder.content = "Not found";
			return responseBuilder;
		}
	} else if (parsedRequest.command == "DELETE") {
		struct UrlParse url = parseDELETEUrl(parsedRequest.url);
		if (url.isUrlOk) {
			if (url.board == "boards") {
				responseBuilder = board_delete(url.name);
				return responseBuilder;
			} else if (url.board == "board") {
				responseBuilder = item_delete(url.name, url.id);
				return responseBuilder;
			} else {
				responseBuilder.returnCode = 404;
				responseBuilder.content = "Url in request is unknown";
				return responseBuilder;
			}
		} else {
			responseBuilder.returnCode = 404;
			responseBuilder.content = "Not found";
			return responseBuilder;
		}
	} else if (parsedRequest.command == "PUT") {
		struct UrlParse url = parsePUTUrl(parsedRequest.url);
		if (url.isUrlOk) {
			///PUT or POST above item which has content-length = 0 returns 400
			if (parsedRequest.content.empty()) {
				responseBuilder.returnCode = 400;
				responseBuilder.content = "Trying to update with empty content";
				return responseBuilder;
			}
			
			responseBuilder = item_update(url.name, url.id, parsedRequest.content);
			return responseBuilder;
		} else {
			responseBuilder.returnCode = 404;
			responseBuilder.content = "Not found";
			return responseBuilder;
		}
	} else {
		responseBuilder.returnCode = 404;
		responseBuilder.content = "Not found";
		return responseBuilder;
	}
}

/********************************************************************************************************
*				PARSING URL FROM REQUEST						*
* ******************************************************************************************************/

/**
 * 	Parse url if command is GET
 */
struct UrlParse parseGETUrl(string url) {
	struct UrlParse urlStruct;
	//cant contains 1 or 2 slashes
	int slashOccurs = count(url.begin(), url.end(), '/');
	if (slashOccurs != 1 && slashOccurs != 2) {
		urlStruct.isUrlOk = false;
		return urlStruct;
	}

	if (url == "/boards") {
		urlStruct.board = "boards";
		return urlStruct;
	} else if (url.substr(0, 7) == "/board/") {
		string name = url.substr(7, url.length() - 1);
		//maybe it will be good to check, that name has not slash (it is forbidden)
		urlStruct.board = "board";
		urlStruct.name = name;
		return urlStruct;
	} else {
		urlStruct.isUrlOk = false;
		return urlStruct;
	}
}

/**
 * 	Parse url if command is POST
 */
struct UrlParse parsePOSTUrl(string url) {
	struct UrlParse urlStruct;
	//must contains 2 slashes
	size_t slashOccurs = count(url.begin(), url.end(), '/');
	if (slashOccurs != 2) {
		urlStruct.isUrlOk = false;
		return urlStruct;
	}

	if (url.substr(0, 8) == "/boards/") {
		string name = url.substr(8, url.length() - 1);
		//maybe it will be good to check, that name has not slash (it is forbidden)
		urlStruct.board = "boards";
		urlStruct.name = name;
		return urlStruct;
	} else if (url.substr(0, 7) == "/board/") {
		string name = url.substr(7, url.length() - 1);
		urlStruct.board = "board";
		urlStruct.name = name;
		return urlStruct;
	} else {
		urlStruct.isUrlOk = false;
		return urlStruct;
	}
}

/**
 * 	Parse url if command is DELETE
 */
struct UrlParse parseDELETEUrl(string url) {
	struct UrlParse urlStruct;
	//can have 2 or 3 slashes
	size_t slashOccurs = count(url.begin(), url.end(), '/');
	if (slashOccurs != 2 && slashOccurs != 3) {
		urlStruct.isUrlOk = false;
		return urlStruct;
	}

	if (url.substr(0, 8) == "/boards/") {
		string name = url.substr(8, url.length() - 1);
		//maybe it will be good to check, that name has not slash (it is forbidden)
		urlStruct.board = "boards";
		urlStruct.name = name;
		return urlStruct;
	} else if (url.substr(0, 7) == "/board/") {
		string zvysok = url.substr(7, url.length() - 1);
		size_t found = zvysok.find('/');
		
		if (found != string::npos) {
			//til 1st slash
			urlStruct.name = zvysok.substr(0, found);
			//after 1st slash til end
			urlStruct.id = zvysok.substr(found + 1, zvysok.length() - 1);
			urlStruct.board = "board";
			return urlStruct;
		} else {
			urlStruct.isUrlOk = false;
			return urlStruct;
		}
	} else {
		urlStruct.isUrlOk = false;
		return urlStruct;
	}
}

/**
 * 	Parse url if command is PUT
 */
struct UrlParse parsePUTUrl(string url) {
	struct UrlParse urlStruct;
	//must have 3 slashes
	size_t slashOccurs = count(url.begin(), url.end(), '/');
	if (slashOccurs != 3) {
		urlStruct.isUrlOk = false;
		return urlStruct;
	}

	if (url.substr(0, 7) == "/board/") {
		string zvysok = url.substr(7, url.length() - 1);
		size_t found = zvysok.find('/');
		
		if (found != string::npos) {
			//til 1st slash
			urlStruct.name = zvysok.substr(0, found);
			//after 1st slash til end
			urlStruct.id = zvysok.substr(found + 1, zvysok.length() - 1);
			urlStruct.board = "board";
			return urlStruct;
		} else {
			urlStruct.isUrlOk = false;
			return urlStruct;
		}
	} else {
		urlStruct.isUrlOk = false;
		return urlStruct;
	}
}

/********************************************************************************************************
*				PARSING REQUEST FROM CLIENT						*
* ******************************************************************************************************/

/**
 * 	Function for parsing whole request
 */
struct ParsedRequest parseWholeRequest(string request) {
	struct ParsedRequest parsedRequest;
	struct ParsedRequest headerAndRQLine; 
	vector <string> bodyAndHead = bodyHeaderDelimiter(request);

	if (bodyAndHead.size() != 2) {
		//cerr << "Bad request from client" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}
	
	headerAndRQLine =  parseHeaderAndRequestLine(bodyAndHead[0]);
	if (!headerAndRQLine.isRequestOk) {
		//cerr << "Bad request from client" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	} else {
		parsedRequest.command = headerAndRQLine.command;
		parsedRequest.url = headerAndRQLine.url;
		parsedRequest.version = headerAndRQLine.version;
		parsedRequest.host = headerAndRQLine.host;
		parsedRequest.accept = headerAndRQLine.accept;
		parsedRequest.contentType = headerAndRQLine.contentType;
		parsedRequest.contentLength = headerAndRQLine.contentLength;
	}
	parsedRequest.content = bodyAndHead[1];
	return parsedRequest;
}

/*
 * Function returns 2 strings (header and body)
 * In case of bad request, return one empty string
 */
vector <string> bodyHeaderDelimiter(string request) {
	size_t position;
	size_t requestLength = request.length();
	vector <string> result;

	if (request.empty()) {
		//cerr << "Bad request, request is empty";
		result.clear();
		return result;
	}
	
	if ((position = request.find(MAIN_DELIMITER)) == string::npos) {
		//cerr << "Bad HTTP request from client" << endl;
		result.clear();
		return result;
	}

	result.push_back(request.substr(0, position));
	result.push_back(request.substr(position + strlen(MAIN_DELIMITER), requestLength- 1));
	return result;
}

/**
 * 	Function for parsing Header and Request Line of request
 */
struct ParsedRequest parseHeaderAndRequestLine(string headerAndRQLine) {
	struct ParsedRequest parsedRequest;
	//string can not be empty
	if (headerAndRQLine.empty()) {
		//cerr << "Bad request, missing request line and header" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}

	vector <string> requestLineAndHeader = customGetLines(headerAndRQLine);
	
	//if Request has not headers
	if (requestLineAndHeader.size() == 1) {
		string requestLine = requestLineAndHeader[0];
		requestLine.push_back('\r');
		parsedRequest = parseRequestLine(requestLine);
		if (!parsedRequest.isRequestOk) {
			//cerr << "Bad request from client, invalid request line" << endl;
			parsedRequest.isRequestOk = false;
			return parsedRequest;
		} else {
			return parsedRequest;
		}
	}

	//if there is any header, size must be greater than 2 (request line + header)
	//other wise it end with exception, becasue we are accessing second element of requestLineAndHeader	
	if (requestLineAndHeader.size() < 2) {
		//cerr << "Bad request from client, minimum requires are Request Line and Host" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}
	
	//processing request line (1st line of request)
	string requestLine = requestLineAndHeader[0];
	if (requestLine.empty()){
		//cerr << "Bad request from client, missing request line" << endl;
		parsedRequest.isRequestOk = false;
		return  parsedRequest;
	} else {
		struct ParsedRequest auxiliaryStruct =  parseRequestLine(requestLine);
		if (!auxiliaryStruct.isRequestOk) {
			//cerr << "Bad request from client, invalid request line" << endl;
			parsedRequest.isRequestOk = false;
			return  parsedRequest;	
		} else {
			parsedRequest.command = auxiliaryStruct.command;
			parsedRequest.url = auxiliaryStruct.url;
			parsedRequest.version = auxiliaryStruct.version;
		}
	}

	//last line of header is processing separate
	for (size_t i = 1; i < requestLineAndHeader.size() - 1; i++) {
		string header = requestLineAndHeader[i];
		if (header.empty()) {
			//cerr << "Bad request from client, missing mandatory header host" << endl;
			parsedRequest.isRequestOk = false;
			return parsedRequest;
		} else {
			struct ParsedRequest auxiliaryStruct = parseHeaderWithCRControl(header);
			if (!auxiliaryStruct.isRequestOk) {
				//cerr << "Bad request from client, invalid header" << endl;
				parsedRequest.isRequestOk = false;
				return  parsedRequest;	
			} else {
				if (auxiliaryStruct.host != "") {
					parsedRequest.host = auxiliaryStruct.host;
				} else if (auxiliaryStruct.accept != ""){
					parsedRequest.accept = auxiliaryStruct.accept;
				} else if (auxiliaryStruct.contentType != ""){
					parsedRequest.contentType = auxiliaryStruct.contentType;
				} else if (auxiliaryStruct.contentLength != "") {
					parsedRequest.contentLength = auxiliaryStruct.contentLength;
				}
			}
		}
	}

	//in most  header can be only one space (SP), but there are headers also with more spaces, for instance Date header
	//so it will be probably better without it
/*
	if (count(requestLineAndHeader[requestLineAndHeader.size() - 1].begin(), requestLineAndHeader[requestLineAndHeader.size() - 1].end(), ' ') != 1) {
		//cerr << "Bad request, Header is invalid (contains more than 1 spaces)" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}
*/
	struct ParsedRequest auxStructForLast = parseHeadersLines(requestLineAndHeader[requestLineAndHeader.size() - 1]);
	
	if (!auxStructForLast.isRequestOk) {
		//cerr << "Bad request" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}

	//save required headers
	if (auxStructForLast.host != "") {
		parsedRequest.host = auxStructForLast.host;
	} else if (auxStructForLast.accept != ""){
		parsedRequest.accept = auxStructForLast.accept;
	} else if (auxStructForLast.contentType != ""){
		parsedRequest.contentType = auxStructForLast.contentType;
	} else if (auxStructForLast.contentLength != "") {
		parsedRequest.contentLength = auxStructForLast.contentLength;
	}
	return parsedRequest;
}

/**
 * 	Function for parsing headers, which end with \r character
 */
struct ParsedRequest parseHeaderWithCRControl(string header) {
	struct ParsedRequest parsedRequest;

	//header can not be empty
	if (header.empty()) {
		//cerr << "Bad request, header is empty" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}

	//have to end with CR character
	if (header.at(header.length() - 1) != '\r'){
		//cerr << "Bad request, No CR in header" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}

	//remove CR for better handling
	header.erase(header.size() - 1);
		
	//only one space can be in header (SP character)
/*	if (count(header.begin(), header.end(), ' ') != 1) {
		//cerr << "Bad request, Header is invalid (contains more than 1 spaces)" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}
*/
	//parsing line
	struct ParsedRequest auxiliaryStruct = parseHeadersLines(header);

	if (!auxiliaryStruct.isRequestOk) {
		//cerr << "Bad request" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}

	//save required headers
	if (!auxiliaryStruct.host.empty()) {
		parsedRequest.host = auxiliaryStruct.host;
	} else if (!auxiliaryStruct.accept.empty()){
		parsedRequest.accept = auxiliaryStruct.accept;
	} else if (!auxiliaryStruct.contentType.empty()){
		parsedRequest.contentType = auxiliaryStruct.contentType;
	} else if (!auxiliaryStruct.contentLength.empty()) {
		parsedRequest.contentLength = auxiliaryStruct.contentLength;
	}
	
	return parsedRequest;
}

/*
 * Get one line of header
 * Split line according string ": "
 * return structure ParsedRequest with filled header
 * filled only selected headers
 **/
struct ParsedRequest parseHeadersLines(string headerLine){
	struct ParsedRequest parsedRequest;
	//headers line can not be empty
	if (headerLine.empty()) {
		//cerr << "Bad request, header is empty" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}

	vector <string> parts;
	size_t position;
	
	if ((position = headerLine.find(": ")) == string::npos) {
		//cerr << "Bad HTTP request from client, bad structure of header" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}
	size_t headerLineLength = headerLine.length();

	parts.push_back(headerLine.substr(0, position));
	parts.push_back(headerLine.substr(position + 2, headerLineLength - 1));

	//header must have name and value
	if (parts[0].empty() || parts[1].empty()) {
		//cerr << "Bad HTTP request from client, bad structure of header" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}
	
	if (parts[0].compare("Host") == 0) {
		parsedRequest.host = parts[1];
		return parsedRequest;
	} else if (parts[0].compare("Accept") == 0) {
		parsedRequest.accept = parts[1];
		return parsedRequest;
	} else if (parts[0].compare("Content-Type") == 0) {
		parsedRequest.contentType = parts[1];
		return parsedRequest;
	} else if (parts[0].compare("Content-Length") == 0) {
		parsedRequest.contentLength = parts[1];
		return parsedRequest;
	} else {
		return parsedRequest;
	}
}

/**
 * 	Function for parsing request line (first line of request)
 */
struct ParsedRequest parseRequestLine(string requestLine) {
	struct ParsedRequest parsedRequest;
	//can not be empty
	if (requestLine.empty()) {
		//cerr << "Bad request, Request Line is empty" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}

	//must end with character CR
	if (requestLine.at(requestLine.length() - 1) != '\r'){
		//cerr << "Bad request, No CR in request line" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}

	//can contains only 2 spaces
	int spaceOccurs = count(requestLine.begin(), requestLine.end(), ' ');
	if (spaceOccurs != 2) {
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}


	//delete CR character for better handling with string
	requestLine.erase(requestLine.size() - 1);
	
	vector <string> parts;
	stringstream stream(requestLine);
	string buffer; //empty string

	while(getline(stream, buffer, ' ')) {
		parts.push_back(buffer);
		buffer.clear();
	}
	
	if (parts.size() != 3) {
		//cerr << "Bad request, Not all items in request line" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}
	string command, url, version;
	command = parts[0];
	url = parts[1];
	version = parts[2];
	
	//check, if version is ok
	if (version.compare("HTTP/1.1") != 0) {
		//cerr << "Bad request, version is not match with HTTP/1.1" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	} else {
		parsedRequest.version = version;
	}
	
	parsedRequest.url = url;

	if (command.compare("GET") == 0) {
		parsedRequest.command = command;
		return parsedRequest;
	} else if (command.compare("POST") == 0) {
		parsedRequest.command = command;
		return parsedRequest;
	} else if (command.compare("PUT") == 0) {
		parsedRequest.command = command;
		return parsedRequest;
	} else if (command.compare("DELETE") == 0) {
		parsedRequest.command = command;
		return parsedRequest;
	} else {
		//cerr << "Bad request, command is wrong" << endl;
		parsedRequest.isRequestOk = false;
		return parsedRequest;
	}
}

/*
 *	Split string accoring new line character
 *	Return vecor of strings
 */
vector <string> customGetLines(string request) {
	stringstream stream(request);
	vector <string> result;
	string buffer; //this creates empty string

	while(getline(stream, buffer, '\n')) {
		result.push_back(buffer);
		buffer.clear();
	}

	return result;
}

