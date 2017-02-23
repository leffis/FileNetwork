#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "helper.h"
#include <string>

/** set up the server address structure */
static void make_server_info(int ip, int port, struct sockaddr_in *server_info) {
	assert(server_info != NULL);
	memset(server_info, 0, sizeof(struct sockaddr_in));
	server_info->sin_family = AF_INET;
	server_info->sin_port = htons(port);
	memcpy(&server_info->sin_addr, &ip, sizeof(int));
}

/** Turn hostname into its IP address */
static int map_hostname(char *hostname, int *ip) {
	struct hostent* server_entity;

	assert(hostname != NULL);
	assert(ip != NULL);

	server_entity =gethostbyname(hostname);
	if (server_entity == NULL) {
		puts("Cannot resolve hostname");
		return -1;
	}

	memcpy(ip, server_entity->h_addr, server_entity->h_length);
	return 0;
}

/** Send the request to server */
static int start_negotiation(int serverAddr, char* request) {
	int count;
	char buf[256];

	snprintf(buf, 256, "%s", request);

	count = write(serverAddr, buf, sizeof(buf));
	if (count <= 0) {
		puts("Cannot send the request to the server");
		return -1;
	}

	return 0;
}

/** Get the udp port after the negotiation has started */
static int get_udp_port(int serverAddr, int *udp_port) {
	int count;
	char buf[5];

	assert(udp_port != NULL);

	count = read(serverAddr, buf, sizeof(buf) -1);
	if (count <= 0) {
		puts("Cannot read from the server");
		return -1;
	}

	/* set the port number here */
	*udp_port = ntohl(*(int*)buf);
	return 0;
}

static int getFile(int serverAddr, int ip, int port, char *msg, char *dest) {

	/* send message and recieve results */
	struct sockaddr_in server_info;
	int udp_socketfd;

	assert(msg != NULL);

	make_server_info(ip, port, &server_info);
	make_bind_socket(0, SOCK_DGRAM, &udp_socketfd);

	/* send request filename */
	sendto(udp_socketfd, msg, sizeof(msg), 0, (struct sockaddr*) &server_info, sizeof(struct sockaddr_in));
    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen(dest, "ab"); 
    if(fp == NULL) {
        printf("Error opening file");
        return 1;
    }
    printf("opened %s\n", dest);
    /* Receive data in chunks of 256 bytes */
    int bytesReceived = 0;
    char recvBuff[256];
    memset(recvBuff, '0', sizeof(recvBuff));

    while((bytesReceived = read(serverAddr, recvBuff, 256)) > 0) {
		printf("received %d bytes\n", bytesReceived);
        fwrite(recvBuff, 1, bytesReceived, fp);
    }

  	printf("remote file received at %s.\n", dest); 

	return 0;
}

static int putFile(int serverAddr, int ip, int port, char *msg, char *dest) {
	struct sockaddr_in server_info;
	int udp_socketfd;

	assert(msg != NULL);

	make_server_info(ip, port, &server_info);
	make_bind_socket(0, SOCK_DGRAM, &udp_socketfd);

	/* send request fliename */
	sendto(udp_socketfd, dest, sizeof(dest), 0, (struct sockaddr*) &server_info, sizeof(struct sockaddr_in));

	/* send the file to server now */
    FILE *fp = fopen(msg, "rb");
    if(fp==NULL) {
        puts("File opern error.");
        return 1;
    }
        /* Read data from file and send it */
    while(true) {
        /* First read file in chunks of 256 bytes */
        unsigned char buff[256]={0};
        break;
        int nread = fread(buff,1,256,fp);
        printf("Bytes read %d \n", nread);        

        /* If read was success, send data. */
        if(nread > 0) {
            printf("Sending \n");
            write(serverAddr, buff, nread);
        }

        /* There is something tricky going on with read .. 
         Either there was error, or we reached end of file. */
        if (nread < 256) {
            if (feof(fp))
                printf("End of file\n");
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
    puts("file uploaded.");

	return 0;
}

static int make_tcp_connection(int ip, int port, int *serverAddr) {

	int temp_serverAddr;
	struct sockaddr_in server_info = {0};

	assert(serverAddr != NULL);

	if (make_socket(SOCK_STREAM, &temp_serverAddr) < 0) {
		puts("socket create failed");
		return -1;
	}

	make_server_info(ip, port, &server_info);

	if (connect(temp_serverAddr, (struct sockaddr *)&server_info, sizeof(server_info)) < 0) {
		puts("Cannot connect to the server");
		close(temp_serverAddr);
		return -1;
	}

	*serverAddr = temp_serverAddr;
	return 0;
}

int main(int argc, char **argv) {
	int serverAddr, ip, udp_port;

	if (argc != 6) {
		puts("ERROR argument, should be: request host n_port txt1 txt2");
		exit(-1);
	}

	/* create tcp connection */
	if (map_hostname(argv[2], &ip) < 0
	        || make_tcp_connection(ip, atoi(argv[3]), &serverAddr) < 0) {
		return -1;
	}

	/* negotiate the udp port */
	if (start_negotiation(serverAddr, argv[1]) < 0
	        || get_udp_port(serverAddr, &udp_port) < 0) {
		close(serverAddr);
		return -1;
	}

	/* we're done with the tcp negotiation */
	//close(serverAddr);
	//std::string files[2] = {argv[4], argv[5]};
	if (argv[1] == std::string("put")) {

		return putFile(serverAddr, ip, udp_port, argv[4], argv[5]);
	} else if (argv[1] == std::string("get")) {
		return getFile(serverAddr, ip, udp_port, argv[4], argv[5]);
	}
	puts("ERROR: invalid request. Only expect request put or get.");
	return 0;
}
