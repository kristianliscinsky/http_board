/**
 * ISA 2019 HTTP nastenka
 * Autro: Kristian Liscinsky
 * xlisci01
 */

#ifndef SHELPER_H
#define SHELPER_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <bits/stdc++.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <list>
#include <map>

#define MAIN_DELIMITER  "\r\n\r\n"
#define BUFFER_SIZE 1024
#define ONE_CHAR 1

using namespace std;

//parsing request
struct ParsedRequest parseWholeRequest(string request);
struct ParsedRequest parseHeaderAndRequestLine(string header);
struct ParsedRequest parseRequestLine(string requestLine);
struct ParsedRequest parseHeaderWithCRControl(string header);
struct ParsedRequest parseHeadersLines(string headerLine);
vector <string> customGetLines(string request);
vector <string> bodyHeaderDelimiter(string request);

//parsing url from request
struct ResponseBuilder processRequest(struct ParsedRequest parsedRequest);
struct UrlParse parseGETUrl(string url);
struct UrlParse parsePOSTUrl(string url);
struct UrlParse parseDELETEUrl(string url);
struct UrlParse parsePUTUrl(string url);
bool is_number(const std::string& s);

//functions for handling boards
struct ResponseBuilder boards();//GET
struct ResponseBuilder board_list(string name);//GET
struct ResponseBuilder board_add(string name);//POST
struct ResponseBuilder item_add(string name, string content);//POST
struct ResponseBuilder board_delete(string name);//DELETE
struct ResponseBuilder item_delete(string name, string id);//DELETE
struct ResponseBuilder item_update(string name, string id, string content);//PUT

//creating response
struct ResponseBuilder addReasonPhraseToResponse(struct ResponseBuilder responseBuilder);
string createResponse(struct ResponseBuilder responseBuilder);

//parsing arguments
int parseArguments(int argc, char *argv[]);
//help message
void printHelpServer();

//sending/reading message
string recvMessage(int socket); 
void sendResponse(int socket, string response);
 
struct ParsedRequest{
	bool isRequestOk = true;
	string command;
	string url;
	string version;
	string host;
	string accept;
	string contentType;
	string contentLength;
	string content;
};

struct UrlParse{
	bool isUrlOk = true;
	string board;
	string name;
	string id;
};

struct ResponseBuilder{
	int returnCode;
	string content;
	string reasonPhrase;
};

#endif //SHELPER_H

