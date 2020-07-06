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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include "shelper.h"

using namespace std;

int main (int argc, char * argv[]) {
	int rc;
	int welcome_socket;
	struct /*sockaddr_in6*/sockaddr_in sa;
	struct /*sockaddr_in6*/sockaddr_in sa_client;
	int port_number = parseArguments(argc, argv);
    
	socklen_t sa_client_len = sizeof(sa_client);
	
	if ((welcome_socket = socket(/*PF_INET6*/AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("ERROR: socket");
		exit(EXIT_FAILURE);
	}

	//set timeout
	struct timeval tv;
	tv.tv_sec = 15;
	setsockopt(welcome_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv,sizeof(struct timeval));	
	
	memset(&sa,0,sizeof(sa));
	/*sa.sin6_family*/sa.sin_family = /*AF_INET6*/AF_INET;
	/*sa.sin6_addr*/sa.sin_addr.s_addr = /*in6addr_any*/htonl(INADDR_ANY);
	/*sa.sin6_port*/sa.sin_port = htons(port_number);	
        
	if ((rc = bind(welcome_socket, (struct sockaddr*)&sa, sizeof(sa))) < 0) {
		perror("ERROR: bind");
		exit(EXIT_FAILURE);		
	}

	if ((listen(welcome_socket, 1)) < 0) {
		perror("ERROR: listen");
		exit(EXIT_FAILURE);				
	}

	while(1){
		int comm_socket = accept(welcome_socket, (struct sockaddr*)&sa_client, &sa_client_len);		
		if (comm_socket > 0) {
			string request = recvMessage(comm_socket);

			ParsedRequest parsedRequest = parseWholeRequest(request);
			ResponseBuilder responseBuilder = processRequest(parsedRequest);
			responseBuilder = addReasonPhraseToResponse(responseBuilder);
			string response = createResponse(responseBuilder);

			//cout << response << endl;
			sendResponse(comm_socket, response);
		}//if (comm_socket > 0)
		close(comm_socket);
	}	
}

