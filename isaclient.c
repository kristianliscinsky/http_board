/**
 * ISA 2019 HTTP nastenka
 * Autro: Kristian Liscinsky
 * xlisci01
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include "chelper.h"
#include <bits/stdc++.h>

using namespace std;

int main (int argc, char * argv[]) {
	struct RequestBuilder requestBuilder  = parsingClient(argc, argv);
	struct HttpRequest request = createHttpRequest(requestBuilder);
	int client_socket = clientCommunication(requestBuilder.port, requestBuilder.host.c_str());

	/*
 	*	sending request to server
 	*/
	int sent = 0;
	int length = request.message.size();
	const char *data = request.message.data();

	while (length > 0) {
		sent = send(client_socket, data, length, 0);
		if (sent < 0) {
			cerr << "Error, sending failed" << endl;
			break;
		}

		data += sent;
		length -= sent;
	}

	/*
 	*	recieving response from server
 	*/
	string response = recvMessage(client_socket);
	string header;
	string body;

	size_t responseDelimitter = response.find("\r\n\r\n"); 
	if (responseDelimitter != string::npos) {
		header = response.substr(0, responseDelimitter);
		body = response.substr(responseDelimitter + 4, response.length() - header.length() - 4);
	} else {
		cerr << "Bad HTTP response format from server" << endl;
		return(EXIT_FAILURE);
	}

	//check if response has mandatory format
	struct ParsedResponse responseStruct = parseHeaderAndStatusLine(header);
	if (!responseStruct.isResponseOk) {
		cerr << "Bad HTTP response format from server" << endl;
		return(EXIT_FAILURE);
	}

	header = header + "\r\n\r\n";
	//header to stderr
	std::cerr << header;
	//body to stdout
	std::cout << body;

	close(client_socket);
	return 0;
}

