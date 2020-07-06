/**
 * ISA 2019 HTTP nastenka
 * Autro: Kristian Liscinsky
 * xlisci01
 */

#include <string>
#include <iostream>
#include <string.h>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <bits/stdc++.h>

#include "chelper.h"

#define ONE_CHAR 1
#define BUFFER_SIZE 1024

using namespace std;

void printHelpClient() {
  	cout << "++----------------------------------------------------------------------------------------------++" << endl;
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
	cout << "++ 				Launch client application					++" << endl;
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
	cout << "++ ./isaclient -H <host> -p <port> <command>							++" << endl;
	cout << "++	<command> has following structure:							++" << endl;
	cout << "++	boards -- returns a list of available bulletin boards, one per line			++" << endl;
	cout << "++	board add <name> -- creates a new blank bulletin board named <name> 			++"<< endl;
	cout << "++	board delete <name> -- deletes bulletin board <name> and all its content		++" << endl;
	cout << "++	board list <name> -- displays the content of the bulletin board board <name>		++" << endl;
	cout << "++	item add <name> <content> -- inserts a new post at the end of the bulletin board <name>	++" << endl;
	cout << "++	item delete <name> <id> -- changes the content of post <id> in the bulletin board <name>++" << endl;
	cout << "++	item update <name> <id> <content> -- deletes post <id> from bulletin board <name>	++" << endl;
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
	cout << "++----------------------------------------------------------------------------------------------++" << endl;
}

/********************************************************************************************************
*				PARSING CLIENT ARGUMENTS						*
* ******************************************************************************************************/

struct RequestBuilder parsingClient(int argc, char * argv[]) {
	struct RequestBuilder request;

	//print help
	if (argc == 2 && strcmp(argv[1], "-h") == 0) {
		printHelpClient();
		exit(EXIT_SUCCESS);
	}
	//minimum are 6 arguments
	if (argc < 6) {
		cerr << "Error: Too few arguments." << endl;
		exit(EXIT_FAILURE);
	}

	//////////////////////////////////////////////////////////////////

	//if 2nd argument is not -h, then it must be -H
	if (/*strcmp(argv[2], "-h") == 0 ||*/ strcmp(argv[1], "-H") == 0) {
		request.host = argv[2];
	} else {
		cerr << "Error: Bad arguments, enter -h for help." << endl;
		exit(EXIT_FAILURE);
	}

	//4th argument is -p
	if (strcmp(argv[3], "-p") == 0) {
		request.port = atoi(argv[4]);
	} else {
		cerr << "Error: Bad arguments, enter -h for help." << endl;
		exit(EXIT_FAILURE);
	}
	
	//////////////////////////////////////////////////////////////////
	
	std::string command = argv[5];
	if (command == "boards") {
		if (argc != 6) {
			cerr << "Error: Bad arguments, enter -h for help." << endl;
			exit(EXIT_FAILURE);
		}
		return request = buildBoards(request.port, request.host);
	} else if (command == "board") {
		return request = builderParserBoard(request.port, request.host, argc, argv);
	} else if (command == "item") {
		return request = builderParserItem(request.port, request.host, argc, argv);
	} else {
	  cerr << "Error: Bad arguments, enter -h for help." << endl;
	  exit(EXIT_FAILURE);
	}
}

/**
 * 	Fill structure RequestBuilder for boards command
 */
struct RequestBuilder buildBoards(int port, string host) {
	struct RequestBuilder request;
	request.host = host;
	request.port = port;
	request.command = "GET";
	request.url = "/boards";
	request.host_port = host + ":" + to_string(port);
	//request.accept = "text/plain";
	return request;
}

/**
 * 	Fill structure RequestBuilder for command handling with boards
 */
struct RequestBuilder builderParserBoard(int port, string host, int argc, char * argv[]) {
	if (argc < 7) {
	  cerr << "Error: Bad arguments, enter -h for help." << endl;
	  exit(EXIT_FAILURE);
	}
	
	std::string command = argv[6];
	
	if (command == "add") {
		if (argc != 8) {
		  cerr << "Error: Bad arguments, enter -h for help." << endl;
		  exit(EXIT_FAILURE);
		}

		string name = argv[7];
		struct RequestBuilder request;
		request.host = host;
		request.port = port;
		request.command = "POST";
		request.url = "/boards/" + name;
		request.host_port = host + ":" + to_string(port);
		//request.accept = "text/plain";
		request.contentType = "text/plain";
		request.contentLength = 0;
		request.content = "";
		return request;
		
	} else if (command == "delete") {
		if (argc != 8) {
		  cerr << "Error: Bad arguments, enter -h for help." << endl;
		  exit(EXIT_FAILURE);
		}

		string name = argv[7];
		struct RequestBuilder request;
		request.host = host;
		request.port = port;
		request.command = "DELETE";
		request.url = "/boards/" + name;
		request.host_port = host + ":" + to_string(port);
		//request.accept = "text/plain";
		return request;
		
	} else if (command == "list") {
		if (argc != 8) {
		  cerr << "Error: Bad arguments, enter -h for help." << endl;
		  exit(EXIT_FAILURE);
		}

		string name = argv[7];
		struct RequestBuilder request;
		request.host = host;
		request.port = port;
		request.command = "GET";
		request.url = "/board/" + name;
		request.host_port = host + ":" + to_string(port);
		//request.accept = "text/plain";
		return request;
		
	} else {
	  cerr << "Error: Bad arguments, enter -h for help." << endl;
	  exit(EXIT_FAILURE);
	}
}

/**
 * 	Fill structure RequestBuilder for command handling with items of boards
 */
struct RequestBuilder builderParserItem(int port, string host, int argc, char * argv[]) {
	if (argc < 7) {
		cerr << "Error: Bad arguments, enter -h for help." << endl;
		exit(EXIT_FAILURE);
	}
	
	string command = argv[6];
	
	if (command == "add") {
		if (argc < 9) {
			cerr << "Error: Bad arguments, enter -h for help." << endl;
			exit(EXIT_FAILURE);
		}
		
		string name = argv[7];
		string content;
		
		for (int i = 8; i < argc; i++) {
			string buffer = argv[i];
			content += buffer + " ";
		}
		//remove last space from content
		content.pop_back();
		
		struct RequestBuilder request;
		
		request.host = host;
		request.port = port;
		request.command = "POST";
		request.url = "/board/" + name;
		request.host_port = host + ":" + to_string(port);
		//request.accept = "text/plain";
		request.contentType = "text/plain";
		request.contentLength = content.size();
		request.content = content;
		return request;
		
	} else if (command == "delete") {
		if (argc != 9) {
		  std::cerr << "Error: Bad arguments, enter -h for help." << std::endl;
		  exit(EXIT_FAILURE);
		}

		string name = argv[7];
		string id = argv[8];
		
		struct RequestBuilder request;
		
		request.host = host;
		request.port = port;
		request.command = "DELETE";
		request.url = "/board/" + name + "/" + id;
		request.host_port = host + ":" + to_string(port);
		//request.accept = "text/plain";
		return request;
		
	} else if (command == "update") {
		if (argc < 10) {
		  std::cerr << "Error: Bad arguments, enter -h for help." << std::endl;
		  exit(EXIT_FAILURE);
		}
		
		string name = argv[7];
		string id = argv[8];
		string content;
		
		for (int i = 9; i < argc; i++) {
			string buffer = argv[i];
			content += buffer + " ";
		}
		//remove last space from content
		content.pop_back();
		
		struct RequestBuilder request;

		request.host = host;
		request.port = port;
		request.command = "PUT";
		request.url = "/board/" + name + "/" + id;
		request.host_port = host + ":" + to_string(port);
		//request.accept = "text/plain";
		request.contentType = "text/plain";
		request.contentLength = content.size();
		request.content = content;
		return request;
		
	} else {
		std::cerr << "Error: Bad arguments, enter -h for help." << std::endl;
		exit(EXIT_FAILURE);
	}
}

/********************************************************************************************************
 *				ESTABLISHING A CONNECTION FOR CLIENT					*
 * ******************************************************************************************************/

/**
 * 	Setting networking part for client communication
 */
int clientCommunication(int port_number, const char *server_hostname) {
    	int client_socket;
    	struct hostent *server;
    	struct sockaddr_in server_address;
    
    	/*	DNS Get sever address by DNS */
    	if ((server = gethostbyname(server_hostname)) == NULL) {
       		fprintf(stderr,"ERROR: no such host as %s\n", server_hostname);
        	exit(EXIT_FAILURE);
    	}

   	 /*	Find IP address and server_address structure initialization */
   	 bzero((char *) &server_address, sizeof(server_address));
   	 server_address.sin_family = AF_INET;
   	 bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
   	 server_address.sin_port = htons(port_number);
    
	/*	Create socket */
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	    {
		perror("ERROR: socket");
		exit(EXIT_FAILURE);
    	}

	//set timeout
	struct timeval tv;
	tv.tv_sec = 15;
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv,sizeof(struct timeval));	

	if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    	{
		perror("ERROR: connect");
		exit(EXIT_FAILURE);        
    	}
    
    	return client_socket;
}

/********************************************************************************************************
 *					CREATING HTTP REQUEST						*
 * ******************************************************************************************************/

/**
*	method for creating HTTP request, if command is invalid, EXIT_FAILURE, HttpRequest otherwise
*	@param RequestBuilder
*	@return struct HttpRequest
*/
struct HttpRequest createHttpRequest(struct RequestBuilder requestBuilder) {
	struct HttpRequest requestInProgress;

	if (requestBuilder.command == "GET") {
		requestInProgress = getHttpRequest(requestBuilder);
	} else if (requestBuilder.command == "POST") {
		requestInProgress = postHttpRequest(requestBuilder);
	} else if (requestBuilder.command == "DELETE") {
		requestInProgress = deleteHttpRequest(requestBuilder);
	} else if (requestBuilder.command == "PUT") {
		requestInProgress = putHttpRequest(requestBuilder);
	} else {
		perror("Invalid operation\n");
			exit(EXIT_FAILURE);
	}

	return requestInProgress;
}

/**
 * 	method for creating HTTP GET Request string
 */
struct HttpRequest getHttpRequest(struct RequestBuilder requestBuilder) {
	struct HttpRequest requestInProgress;
        requestInProgress.message = "GET " + requestBuilder.url + " " + "HTTP/1.1\r\n";
        requestInProgress.message += "Host: " + requestBuilder.host_port + "\r\n";
	//requestInProgress.message += "Accept: " + requestBuilder.accept + "\r\n";
	requestInProgress.message += "Content-Length: " + to_string(requestBuilder.content.size()) + "\r\n";
	requestInProgress.message += "\r\n";
	return requestInProgress;
}

/**
 * 	method for creating HTTP POST Request string
 */
struct HttpRequest postHttpRequest(struct RequestBuilder requestBuilder) {
	struct HttpRequest requestInProgress;
        requestInProgress.message = "POST " + requestBuilder.url + " " + "HTTP/1.1\r\n";
        requestInProgress.message += "Host: " + requestBuilder.host_port + "\r\n";
        //requestInProgress.message += "Accept: " + requestBuilder.accept + "\r\n";
	requestInProgress.message += "Content-Type: " + requestBuilder.contentType + "\r\n";
	requestInProgress.message += "Content-Length: " + to_string(requestBuilder.content.size()) + "\r\n";
        requestInProgress.message += "\r\n";
	requestInProgress.message += requestBuilder.content;
        return requestInProgress;
}

/**
 * 	method for creating HTTP DELETE Request string
 */
struct HttpRequest deleteHttpRequest(struct RequestBuilder requestBuilder) {
        struct HttpRequest requestInProgress;
        requestInProgress.message = "DELETE " + requestBuilder.url + " " + "HTTP/1.1\r\n";
        requestInProgress.message += "Host: " + requestBuilder.host_port + "\r\n";
        //requestInProgress.message += "Accept: " + requestBuilder.accept + "\r\n";
	requestInProgress.message += "Content-Length: " + to_string(requestBuilder.content.size()) + "\r\n";
        requestInProgress.message += "\r\n";
        return requestInProgress;
}

/**
 * 	method for creating HTTP PUT Request string
 */
struct HttpRequest putHttpRequest(struct RequestBuilder requestBuilder) {
        struct HttpRequest requestInProgress;
        requestInProgress.message = "PUT " + requestBuilder.url + " " + "HTTP/1.1\r\n";
        requestInProgress.message += "Host: " + requestBuilder.host_port + "\r\n";
        //requestInProgress.message += "Accept: " + requestBuilder.accept + "\r\n";
        requestInProgress.message += "Content-Type: " + requestBuilder.contentType + "\r\n";
        requestInProgress.message += "Content-Length: " + to_string(requestBuilder.content.size()) + "\r\n";
        requestInProgress.message += "\r\n";
        requestInProgress.message += requestBuilder.content;
        return requestInProgress;
}

/********************************************************************************************************
 *					PARSING HTTP RESPONSE						*
 * ******************************************************************************************************/

/**
 * 	Checking if response is valid
 * 	This function check only Status line and Headers
 * 	Body we do not have to check, because it is always OK
 */
struct ParsedResponse parseHeaderAndStatusLine(string headerAndStatusLine) {
	struct ParsedResponse parsedResponse;
	//string can not be empty
	if (headerAndStatusLine.empty()) {
		//cerr << "Bad request, missing request line and header" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	vector <string> statusLineAndHeader = customGetLines(headerAndStatusLine);
	
	//if Request has not headers
	if (statusLineAndHeader.size() == 1) {
		string statusLine = statusLineAndHeader[0];
		statusLine.push_back('\r');
		parsedResponse = parseStatusLine(statusLine);
		if (!parsedResponse.isResponseOk) {
			//cerr << "Bad request from client, invalid request line" << endl;
			parsedResponse.isResponseOk = false;
			return parsedResponse;
		} else {
			return parsedResponse;
		}
	}

	//if there is any header, size must be greater than 2 (status line + header)
	//other wise it end with exception, becasue we are accessing second element of statusLineAndHeader
	if (statusLineAndHeader.size() < 2) {
		//cerr << "Bad request from client, minimum requires are Request Line and Host" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	//processing request line (first line of response)
	string statusLine = statusLineAndHeader[0];
	if (statusLine.empty()){
		//cerr << "Bad request from client, missing request line" << endl;
		parsedResponse.isResponseOk = false;
		return  parsedResponse;
	} else {
		struct ParsedResponse auxiliaryStruct =  parseStatusLine(statusLine);
		if (!auxiliaryStruct.isResponseOk) {
			//cerr << "Bad request from client, invalid request line" << endl;
			parsedResponse.isResponseOk = false;
			return  parsedResponse;	
		} else {
			parsedResponse.version = auxiliaryStruct.version;
			parsedResponse.returnCode = auxiliaryStruct.returnCode;
			parsedResponse.reasonPhrase = auxiliaryStruct.reasonPhrase;
		}
	}

	//last line of header is processing separate
	for (size_t i = 1; i < statusLineAndHeader.size() - 1; i++) {
		string header = statusLineAndHeader[i];
		if (header.empty()) {
			//cerr << "Bad request from client, missing mandatory header host" << endl;
			parsedResponse.isResponseOk = false;
			return parsedResponse;
		} else {
			struct ParsedResponse auxiliaryStruct = parseHeaderWithCRControl(header);
			if (!auxiliaryStruct.isResponseOk) {
				//cerr << "Bad request from client, invalid header" << endl;
				parsedResponse.isResponseOk = false;
				return  parsedResponse;	
			} else {
				if (auxiliaryStruct.date != "") {
					parsedResponse.date = auxiliaryStruct.date;
				} else if (auxiliaryStruct.contentLength != ""){
					parsedResponse.contentLength = auxiliaryStruct.contentLength;
				} else if (auxiliaryStruct.contentType != ""){
					parsedResponse.contentType = auxiliaryStruct.contentType;
				}
			}
		}
	}

	struct ParsedResponse auxStructForLast = parseHeadersLines(statusLineAndHeader[statusLineAndHeader.size() - 1]);
	
	if (!auxStructForLast.isResponseOk) {
		//cerr << "Bad request" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	//save required headers
	if (auxStructForLast.date != "") {
		parsedResponse.date = auxStructForLast.date;
	} else if (auxStructForLast.contentType != ""){
		parsedResponse.contentType = auxStructForLast.contentType;
	} else if (auxStructForLast.contentLength != "") {
		parsedResponse.contentLength = auxStructForLast.contentLength;
	}
	return parsedResponse;
}

/**
 * 	Function for parsing headers without last of them
 */
struct ParsedResponse parseHeaderWithCRControl(string header) {
	struct ParsedResponse parsedResponse;

	//header can not be empty
	if (header.empty()) {
		//cerr << "Bad request, header is empty" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	//have to end with CR character
	if (header.at(header.length() - 1) != '\r'){
		//cerr << "Bad request, No CR in header" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	//remove CR for better handling
	header.erase(header.size() - 1);
		
	//parsing line
	struct ParsedResponse auxiliaryStruct = parseHeadersLines(header);

	if (!auxiliaryStruct.isResponseOk) {
		//cerr << "Bad request" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	if (!auxiliaryStruct.contentType.empty()) {
		parsedResponse.contentType = auxiliaryStruct.contentType;
	} else if (!auxiliaryStruct.contentLength.empty()) {
		parsedResponse.contentLength = auxiliaryStruct.contentLength;
	} else if (!auxiliaryStruct.date.empty()) {
		parsedResponse.date = auxiliaryStruct.date;
	}
	
	return parsedResponse;
}

/**
 * 	Function for parsing one header line from response
 */
struct ParsedResponse parseHeadersLines(string headerLine) {
	struct ParsedResponse parsedResponse;
	//headers line can not be empty
	if (headerLine.empty()) {
		//cerr << "Bad request, header is empty" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	vector <string> parts;
	size_t position;
	
	if ((position = headerLine.find(": ")) == string::npos) {
		//cerr << "Bad HTTP request from client, bad structure of header" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}
	size_t headerLineLength = headerLine.length();

	parts.push_back(headerLine.substr(0, position));
	parts.push_back(headerLine.substr(position + 2, headerLineLength - 1));

	//header must have name and value
	if (parts[0].empty() || parts[1].empty()) {
		//cerr << "Bad HTTP request from client, bad structure of header" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}
	
	if (parts[0].compare("Content-Type") == 0) {
		parsedResponse.contentType = parts[1];
		return parsedResponse;
	} else if (parts[0].compare("Content-Length") == 0) {
		parsedResponse.contentLength = parts[1];
		return parsedResponse;
	} else if (parts[0].compare("Date") == 0) {
		parsedResponse.date = parts[1];
		return parsedResponse;
	} else {
		return parsedResponse;
	}
}

/**
 * 	Function for parsing first line of response
 * 	Do not have to fullfil struct properly, in case of there are more than 2 spaces
 */
struct ParsedResponse parseStatusLine(string statusLine) {
	struct ParsedResponse parsedResponse;
	//can not be empty
	if (statusLine.empty()) {
		//cerr << "Bad request, Request Line is empty" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	//must end with character CR
	if (statusLine.at(statusLine.length() - 1) != '\r'){
		//cerr << "Bad request, No CR in request line" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	//this is only for request, response can have more spaces, for example HTTP/1.1 404 NOT FOUND\r\n
	//can contains only 2 spaces
/*	int spaceOccurs = count(statusLine.begin(), statusLine.end(), ' ');
	if (spaceOccurs != 2) {
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}
*/
	//delete CR character for better handling with string
	statusLine.erase(statusLine.size() - 1);
	
	vector <string> parts;
	stringstream stream(statusLine);
	string buffer; //empty string

	while(getline(stream, buffer, ' ')) {
		parts.push_back(buffer);
		buffer.clear();
	}
	
	if (parts.size() < 3) {
		//cerr << "Bad request, Not all items in request line" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	string version, returnCode, reasonPhrase;
	version = parts[0];
	returnCode = parts[1];
	reasonPhrase = parts[2];

	//check, if version is ok
	if (version.compare("HTTP/1.1") != 0) {
		//cerr << "Bad request, version is not match with HTTP/1.1" << endl;
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	} else {
		parsedResponse.version = version;
	}

	if (is_number(returnCode)) {
		int code = stoi(returnCode);
		parsedResponse.returnCode = code;
	} else {
		parsedResponse.isResponseOk = false;
		return parsedResponse;
	}

	parsedResponse.reasonPhrase = reasonPhrase;
	return parsedResponse;
}

/*
 *	Split string accoring new line character
 *	Return vector of strings
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

/********************************************************************************************************
 *					RECIEVING HTTP RESPONSE						*
 * ******************************************************************************************************/

/**
 * 	Recieving data from server side
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
		//testContent contains header without \r\n\r\n
		string testContent = result.substr(0, delimitterPos + 1);
		//get content length from header, if miss, return 0
		int readTill = getContentLength(testContent);
		//if readTill <= 0, Content-Length header miss in response
		if(readTill > 0) {
			int contentLength = readTill;
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

/**
 * 	Get Content-Length value from recieved header string
 */
int getContentLength(string response) {
	int result = 0;
	string line;
	vector <string> lines = customGetLines(response);
	for(unsigned int i = 0; i < lines.size(); i++) {
		line = lines[i];
		size_t position = line.find("Content-Length: ");
		if (position != std::string::npos) {
			string help = line.substr(position + 16, line.length() - 1 - position - 16);
			if (is_number(help)) {
				result = stoi(help);
				return result;
			}
		}
	}

	return result;
}

/**
 * 	Find out if string is a number
 */
bool is_number(const std::string& s) {
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

