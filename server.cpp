#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "helper.h"
#include <string>

/* Complete the UDP port negotiation by openning a socket & sending back
 the port number, then complete client's put request. */
static int replyUportAndPut(int serverAddr, int clientAddr) {
	int count, port, udp_socketfd;
	char port_buf[256] = {0};

	make_udp_socket(&udp_socketfd, &port);
	port = htonl(port);
	memcpy(port_buf, &port, sizeof(int));

	count = write(clientAddr, port_buf, sizeof(port_buf));
	if (count <= 0) {
		puts("Cannot write the udp port to the client");
		close(udp_socketfd);
		return -1;
	}

	printf("Reply with port: %d\n", ntohl(*(int*)port_buf));

	char buf[256] = {0};
	struct sockaddr_in client_info;
	socklen_t info_len = sizeof(struct sockaddr_in);
	/* get the filename client request to store */
	recvfrom(udp_socketfd, buf, 256, 0, (struct sockaddr*) &client_info, &info_len);
	printf("receiving %s\n", buf);
    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen(buf, "ab"); 
    if(fp == NULL) {
        printf("Error opening file");
        return 1;
    }
    printf("opened %s\n", buf);
    /* Receive data in chunks of 256 bytes */
    int bytesReceived = 0;
    char recvBuff[256];
    memset(recvBuff, '0', sizeof(recvBuff));

    while((bytesReceived = read(clientAddr, recvBuff, 256)) > 0) {
		printf("received %d bytes\n", bytesReceived);
        fwrite(recvBuff, 1, bytesReceived, fp);
    }

	puts("upload succeed");

	return 0;
}

/* Complete the UDP port negotiation by openning a socket & sending back
 the port number, then complete client's get request. */
static int replyUportAndGet(int serverAddr, int clientAddr) {
	int count, port, udp_socketfd;
	char port_buf[256] = {0};

	make_udp_socket(&udp_socketfd, &port);
	port = htonl(port);
	memcpy(port_buf, &port, sizeof(int));

	count = write(clientAddr, port_buf, sizeof(port_buf));
	if (count <= 0) {
		puts("Cannot write the udp port to the client");
		close(udp_socketfd);
		return -1;
	}

	printf("Reply with port: %d\n", ntohl(*(int*)port_buf));

	char buf[256] = {0};
	struct sockaddr_in client_info;
	socklen_t info_len = sizeof(struct sockaddr_in);

	recvfrom(udp_socketfd, buf, 256, 0, (struct sockaddr*) &client_info, &info_len);

	/* get the filename client requested, send to client now */

        //clientAddr = accept(serverAddr, (struct sockaddr*)NULL ,NULL);
	printf("sending %s\n", buf);
        /* Open the file that we wish to transfer */
        FILE *fp = fopen(buf,"rb");
        if(fp == NULL) {
            puts("File opern error");
            return 1;   
        }   

        /* Read data from file and send it */
        while(true) {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[256]={0};
            int nread = fread(buff,1,256,fp);

            /* If read was success, send data. */
            if(nread > 0) {
                printf("Sending \n");
                write(clientAddr, buff, nread);
            }
            /*
             * There is something tricky going on with read .. 
             * Either there was error, or we reached end of file.
             */
            if (nread < 256)
            {
                if (feof(fp))
                    printf("End of file\n");
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }
        }
    return 0;
}

/* Handle requests (in our case, it would be the number 13) */
static int handle_requests(int serverAddr) {
	int clientAddr, count, request_num;
	char buf[256];

	clientAddr = accept(serverAddr, NULL, NULL);

	if (clientAddr < 0) {
		return -1;
	}

	puts("Client connected");

	memset(buf, 0, 256);
	/* read one character g(103) or q(112) */
	count = read(clientAddr, buf, 1);
	if (count <= 0) {
		puts("Cannot read client request");
		close (clientAddr);
		return -1;
	}

	/* get and process request number */
	request_num = buf[0];

	if (request_num == 112) {
		puts("client request put");
		if (replyUportAndPut(serverAddr, clientAddr) < 0) {
			puts("put failed");
			close (clientAddr);
			return -1;
		}		
	}
	else if (request_num == 103) {	
		puts("client request get");
		if (replyUportAndGet(serverAddr, clientAddr) < 0) {
			puts("get failed");
			close (clientAddr);
			return -1;
		}
	}
	else {
		puts("Got invalid request, only put and get pls.");
		close (clientAddr);
		return -1;
	}

	puts("Done. Wating next client");
	close (clientAddr);
	return 0;
}

int main(int argc, char **argv) {
	int serverAddr, rPort;

	/* set a random port number that greater than 1024 */
	srand (time(NULL));
	rPort = 1025 + rand() % 10000;

	/* make and bind to a socket */
	if (make_bind_socket(rPort, SOCK_STREAM, &serverAddr) < 0) {
		return -1;
	}

	/* listen to the socket using listen(int, int) from sys/socket.h */
	if (listen(serverAddr, 10) == -1) {
		puts("listen failed.");
		close(serverAddr);
		return -1;
	}

	printf("server listening on uPort=%d\n", rPort);

	while (true) {
		handle_requests(serverAddr);
	}

	/* should not reach */
	puts("server unexpected return.");
	return -1;
}
