/**
 * ISA 2019 HTTP nastenka
 * Autro: Kristian Liscinsky
 * xlisci01
 */

#ifndef CHELPER_H
#define CHELPER_H

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

using namespace std;

//for parsing arguments
struct RequestBuilder parsingClient(int argc, char * argv[]);
struct RequestBuilder buildBoards(int port, string host);
struct RequestBuilder builderParserItem(int port, string host, int argc, char * argv[]);
struct RequestBuilder builderParserBoard(int port, string host, int argc, char * argv[]);

//for creating request
struct HttpRequest createHttpRequest(struct RequestBuilder requestBuilder);
struct HttpRequest getHttpRequest(struct RequestBuilder requestBuilder);
struct HttpRequest postHttpRequest(struct RequestBuilder requestBuilder);
struct HttpRequest deleteHttpRequest(struct RequestBuilder requestBuilder);
struct HttpRequest putHttpRequest(struct RequestBuilder requestBuilder);

//for parsing response
struct ParsedResponse parseStatusLine(string statusLine);
struct ParsedResponse parseHeaderWithCRControl(string header);
struct ParsedResponse parseHeadersLines(string headerLine);
struct ParsedResponse parseHeaderAndStatusLine(string headerAndStatusLine);
bool is_number(const std::string& s);
int getContentLength(string response);
string recvMessage(int socket);
vector <string> customGetLines(string request); 

//rest
void printHelpClient();
int clientCommunication(int port_number, const char *server_hostname);

struct HttpRequest {
	std::string message;
};

struct RequestBuilder {
	std::string host;
	int port;

	std::string command;
	std::string url;
	std::string host_port;
	std::string accept;
	std::string contentType;
	int contentLength;
	std::string content;
};

struct ParsedResponse {
	bool isResponseOk = true;
	string version;
	int returnCode;
	string reasonPhrase;
	string date;
	string contentLength;
	string contentType;
};

#endif //CHELPER_H
