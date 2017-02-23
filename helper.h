#ifndef _HELPER_H_
#define _HELPER_H_

/* generic methods for creating sockets */

int make_socket(int type, int *serverfd);

/* client needs to create a UDP socket too */
int make_bind_socket(int port, int type, int *serverfd);

/* this is probably not so generic */
int make_udp_socket(int *fd, int *port);
#endif
