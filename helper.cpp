#include "helper.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

int make_socket(int type, int *serverAddr) {
	int retval = 0;

	retval =  socket(AF_INET, type, 0);

	/* socket failed */
	if (retval < 0) {
		puts("Cannot open socket");
		return retval;
	}

	/* socket succeed */
	*serverAddr = retval;
	return retval;
}

int make_bind_socket(int port, int type, int *serverAddr) {

	/* sockaddr_in, sin_family and so on refer to netinet/in.h */
	int retval = 0, i = 0, numTries = 10000;
	int temp_serverAddr;
	struct sockaddr_in server_info = {0};

	retval = make_socket(type, &temp_serverAddr);
	if (retval < 0) {
		return retval;
	}

	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = htonl(INADDR_ANY);

	while (true) {
		server_info.sin_port = htons(port + i);
		retval = bind(temp_serverAddr, (struct sockaddr*) &server_info, sizeof(server_info));

		if (retval >= 0) {
			break;
		}

		/* cannot bind and has reach trying limit */
		if (numTries < 0) {
			puts("Cannot do binding");
			close(temp_serverAddr);
			return retval;
		}

		/* otherwise continue binding with the next port value */
		i++;
		numTries--;
	}

	/* OKAY */
	*serverAddr = temp_serverAddr;
	return retval;
}

int make_udp_socket(int *addr, int *rPort) {
	int retval = 0, i = 0, numTries = 10000;

	*rPort = 1025 + rand() % 10000;

	while(true) {
		rPort += i;
		retval = make_bind_socket(*rPort, SOCK_DGRAM, addr);
		if (retval >= 0) {
			break;
		}
		if (numTries < 0) {
			puts("Exceed max tries for binding the UDP socket to a port");
			return -1;
		}
		i++;
		numTries--;
	}
	return retval;
}
