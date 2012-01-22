/**
 * \file comms.h
 * \brief TCP/IP and Unix socket code
 */
#ifndef SOCKET_H
#define SOCKET_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>

/**
 * initialize and return a unix or internet socket
 * arg should be of form "/path/to/socket" or "inetserv:2947"
 */
int init_connection(char*);

/**
 *  initialize a listening tcp socket on port specified in arg
 */
int init_tcp_listen(int);

/**
 * initialize a listening unix socket at location specified in arg
 */
int init_unix_listen(char*);

/**
 * accept a new connection shown by activity on listening socket
 */
int accept_connection(int);

#endif
