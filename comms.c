/**
 * \file comms.c
 * \brief TCP/IP and Unix socket code
 */
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include "comms.h"

int init_connection(char* addr) {
	struct sockaddr_in inet_addr;
	struct sockaddr_un unix_addr;
	struct hostent *host_addr;
	char *address = strdup(addr);
	char *ptr = strchr(address, ':');
	int i;
	int sock;

	if (!ptr) {
		if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			free(address);
			perror("socket");
			return -1;
		}
		unix_addr.sun_family = AF_UNIX;
		strcpy(unix_addr.sun_path, address);
		i = strlen(unix_addr.sun_path) + sizeof(unix_addr.sun_family);
		if ((connect(sock, (struct sockaddr *)&unix_addr, i) == -1)) {
			free(address);
			perror("connect");
			return -1;
		}
	} else {
		*ptr = '\0';
		ptr++;
		i = atoi(ptr);
		if ((host_addr = gethostbyname(address)) == NULL) {
			free(address);
			perror(address);
			return -1;
		}
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			free(address);
			perror("socket");
			return -1;
		}
		inet_addr.sin_family = AF_INET;
		inet_addr.sin_port = htons(i);
		inet_addr.sin_addr = *((struct in_addr*) host_addr->h_addr);
		memset(&(inet_addr.sin_zero), '\0', 8); 
		if ((connect(sock, (struct sockaddr *)&inet_addr,
			    sizeof(struct sockaddr))) == -1) {
			free(address);
			perror("connect");
			return -1;
		}
	}
	free(address);
	return sock;
}

int init_tcp_listen(int port) {
	struct sockaddr_in inet_addr;
	int one = 1;
	int listener;

	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
		perror("setsockopt");
		return -1;
	}
	inet_addr.sin_family = AF_INET ;
	inet_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_addr.sin_port = htons(port);
	if (bind(listener, (struct sockaddr*)&inet_addr, sizeof(struct sockaddr)) < 0) {
		perror("bind") ;
		return -1;
	}
	if (listen(listener, 10) == -1) {
		perror("listen");
		return -1;
	}

	return listener;
}
	
int init_unix_listen(char *address) {
	struct sockaddr_un unix_addr;
	int one = 1;
	int listener;

	if ((listener = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
		perror("setsockopt");
		return -1;
	}
	unix_addr.sun_family = AF_UNIX;
	strncpy(unix_addr.sun_path, address,
                   sizeof(unix_addr.sun_path) - 1);
	if (bind(listener, (struct sockaddr*)&unix_addr, sizeof(struct sockaddr_un)) < 0) {
		perror("bind") ;
		return -1;
	}
	if (listen(listener, 10) == -1) {
		perror("listen");
		return -1;
	}

	return listener;
}

int accept_connection(int listener) {
	struct sockaddr_in saddr;
	socklen_t saddrlen = sizeof(struct sockaddr_in);
	int sock;

	if ((sock = accept(listener, (struct sockaddr*)&saddr, &saddrlen)) < 0) {
		perror("accept");
		return -1;
	}
	
	return sock;
}
