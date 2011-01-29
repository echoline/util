/* this program logs into TOC and constructs,
 * then sends FLAP packets from standard in.
 * packets recieved are stripped of the header
 * and printed to standard out.
 *
 * http://eli.neoturbine.net
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <limits.h>
#include <poll.h>
#include <stdbool.h>

#define HOST "toc.oscar.aol.com"
//#define HOST "localhost"
#define PORT 9898 
#define AUTH_HOST "login.oscar.aol.com"
#define AUTH_PORT 5190

unsigned int sequence = UINT_MAX;

struct signon {
	unsigned int version;
	unsigned short tlv;
	unsigned short sn_len;
	char sn[80];
};

struct flap {
	unsigned char magic;
	unsigned char frame;
	unsigned short seq;
	unsigned short len;
};

void flap_to_socket(int socket, struct flap *header, void *buf, int len) {
	char *packet = malloc((sizeof(struct flap)+len+1)*sizeof(char));
	int slen = 0, tmp;

	header->seq = htons(++sequence & 0xffff);;
	header->len = htons(len);
	memcpy(packet, header, sizeof(struct flap));
	slen += sizeof(struct flap);
	memcpy(&packet[slen], buf, len);
	slen += len;
	tmp = len = 0;
        while ((tmp = write(socket,packet + len,slen - len)) < (slen - len)) {
		if (tmp < 1) {
	                perror("write");
        	        exit(0);
		}
		len += tmp;
        }
	//printf("%u: sent:%d len:%d data:%s\n", sequence, slen, len, buf);
}

char *roast(const char *password) {
	int i, len = strlen(password);
	char *ret = malloc((3+(2*len))*sizeof(char));
	char *roastring = "Tic/Toc";
	char byte[3];

	strcpy(ret, "0x");
	for (i = 0; i < len; i++) {
		snprintf(byte, 3, "%02x", password[i]^roastring[i%7]);
		strncat(ret, byte, 3);
	}

	return ret;
}

int toc_signon(int sock, struct flap *header, char **argv) {
	char *roastedpass = roast(argv[2]);
	char buf[4096];
	int rlen;
	struct signon signon;

	if(!write(sock, "FLAPON\r\n\r\n", 10)) { /* Client sends "FLAPON\r\n\r\n" */
		perror("write");
		exit(0);
	}

	if ((rlen = read(sock, buf, sizeof(buf)) < 1)) { /* TOC sends Client FLAP SIGNON */
		perror("read");
		exit(0);
	}

	//sleep(1);
	header->magic = '*';
	header->frame = 1;
	header->seq = htons(0);
	signon.version = htonl(1);
	signon.tlv = htons(1);
	signon.sn_len = htons(strlen(argv[1]));
	strncpy(signon.sn, argv[1], 80);
	flap_to_socket(sock, header, &signon, strlen(argv[1]) + 8); /* Client sends TOC FLAP SIGNON */

	//sleep(1);
	header->frame = 2;
	snprintf(buf, 4096, "toc2_login %s %d %s %s english TIC:TEST 160 US \"\" \"\" 3 0 30303 -kentucky -utf8 %d", AUTH_HOST, AUTH_PORT, argv[1], roastedpass, 7696 * argv[1][0] * argv[2][0]);
	free(roastedpass);
	flap_to_socket(sock, header, buf, strlen(buf) + 1); /* Client sends TOC "toc_signon" message */

	/* if login fails TOC drops client's connection
	 * else TOC sends client SIGN_ON reply */
	if ((rlen = read(sock, buf, sizeof(buf)) < 1)) {
		perror("read");
		exit(0);
	}
	if (strncasecmp(&buf[6], "SIGN_ON", 7)) {
		fprintf(stderr, "SIGN_ON not received.\n");
		exit(0);
	}

	//sleep(1);
	strcpy(buf, "toc_init_done");
	flap_to_socket(sock, header, buf, strlen(buf) + 1);
}

int main(int argc, char **argv) {
	int sock;
	struct hostent *host;
	struct sockaddr_in sa;
	char *buf = malloc(4096);
	int rlen, i;
	struct flap send_header;
	struct flap recv_header;
	struct pollfd *pfds = malloc(2 * sizeof(struct pollfd));
	bool flag;

	if (argc != 3) {
		fprintf(stderr, "usage: %s screenname password\n", argv[0]);
		return 0;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 0;
	}
	
	if (!(host = gethostbyname(HOST))) {
		fprintf(stderr, "unable to resolve host " HOST "\n");
		return 0;
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr = *((struct in_addr *)host->h_addr);
	memset(&(sa.sin_zero), '\0', 8);
	
	if (connect(sock, (struct sockaddr*)&sa, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		return 0;
	}

	toc_signon(sock, &send_header, argv);

	pfds[0].fd = 0;
	pfds[0].events = POLLIN;
	pfds[1].fd = sock;
	pfds[1].events = POLLIN;

	while(poll(pfds, 2, -1) > 0) {
		flag = true;
		if (pfds[0].revents & POLLIN) {
			flag = false;
			buf = malloc(4097);
			if (fgets(buf, 4096, stdin) == NULL) {
				perror("exiting");
				return 1;
			}
			buf[strcspn(buf, "\n")] = '\0';
			//fprintf(stderr, "%s\n", buf);
			flap_to_socket(sock, &send_header, buf, strlen(buf) + 1);
			free(buf);
		}
		if (pfds[1].revents & POLLIN) {
			flag = false;
			buf = malloc(7);
			rlen = 0;
			while (rlen < 6) {
				if ((i = read(sock, buf + rlen, 6 - rlen)) < 1) {
					perror("read1");
					return 0;
				}
				rlen += i;
			}
			memcpy(&recv_header, buf, 6);
			free(buf);
			rlen = 0;
			//printf("*%d:", ntohs(recv_header.len));
			buf = malloc(ntohs(recv_header.len)+1);
			while (rlen < ntohs(recv_header.len)) {
				if ((i = read(sock, buf + rlen, ntohs(recv_header.len) - rlen)) < 1) {
					perror("read2");
					return 0;
				}
				rlen += i;
			}
			buf[rlen] = '\0';
			printf("%s\n", buf);
			//fprintf(stderr, "%s\n", buf);
			fflush(stdout);
			free(buf);
		}
		if (flag == true) {
			fprintf(stderr, "no read from poll");
		}
	}
	perror("poll");
}

