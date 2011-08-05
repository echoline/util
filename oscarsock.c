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

#define AUTH_HOST "login.oscar.aol.com"
#define AUTH_PORT 5190
#define CLIENT_ID_STRING "oscarsock.2011"

unsigned int sequence = UINT_MAX;

struct tlv {
	unsigned short type;
	unsigned short length;
	unsigned char value[0x10000];
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

	//header->seq = htons(++sequence & 0xffff);;
	//header->len = htons(len);
	//memcpy(packet, header, sizeof(struct flap));
	packet[0] = header->magic;
	packet[1] = header->frame;
	packet[2] = (header->seq & 0xff00) >> 8;
	packet[3] = (header->seq & 0xff);
	packet[4] = (header->seq & 0xff00) >> 8;
	packet[5] = (header->seq & 0xff);
	slen += 6;
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

void roast(char *password) {
	int i, len = strlen(password);
	char roastring[] = {0xF3, 0x26, 0x81, 0xC4, 0x39, 0x86, 0xDB, 0x92, 0x71, 0xA3, 0xB9, 0xE6, 0x53, 0x7A, 0x95, 0x7C};

	for (i = 0; i < len; i++) {
		password[i]^=roastring[i%16];
	}
}

void make_tlv_str(unsigned char *buf, int type, int length, char *value) {
	int i;

	buf[0] = (type & 0xff00) >> 8;
	buf[1] = (type & 0xff);
	buf[2] = (length & 0xff00) >> 8;
	buf[3] = (length & 0xff);

	for (i = 0; i < length; i++)
		buf[4+i] = value[i];
}

void make_tlv_ushort(unsigned char *buf, int type, unsigned short value) {
	buf[0] = (type & 0xff00) >> 8;
	buf[1] = (type & 0xff);
	buf[2] = 0;
	buf[3] = 2;
	buf[4] = (value & 0xff00) >> 8;
	buf[5] = (value & 0xff);
}

void make_tlv_uint(unsigned char *buf, int type, unsigned int value) {
	buf[0] = (type & 0xff00) >> 8;
	buf[1] = (type & 0xff);
	buf[2] = 0;
	buf[3] = 4;
	buf[4] = (value & 0xff000000) >> 24;
	buf[5] = (value & 0xff0000) >> 16;
	buf[6] = (value & 0xff00) >> 8;
	buf[7] = (value & 0xff);
}

int oscar_signon(int sock, struct flap *header, char **argv) {
	int pwlen = strlen(argv[2]);
	char buf[0x10000] = "";
	int rlen;
	int len = 0;

	roast(argv[2]);

	if ((rlen = read(sock, buf, sizeof(buf)) < 1)) {
		perror("read");
		exit(0);
	}

	header->magic = '*';
	header->frame = 1;
	header->seq = htons(0);
	
	buf[len++] = 0;
	buf[len++] = 0;
	buf[len++] = 0;
	buf[len++] = 1;
	make_tlv_str(&buf[len], 1, strlen(argv[1]), argv[1]);
	len += 4 + strlen(argv[1]);
	make_tlv_str(&buf[len], 2, pwlen, argv[2]);
	len += 4 + pwlen;
	make_tlv_str(&buf[len], 3, strlen(CLIENT_ID_STRING), CLIENT_ID_STRING);
	len += 4 + strlen(CLIENT_ID_STRING);
	make_tlv_ushort(&buf[len], 0x16, 0x010A);
	len += 6;
	make_tlv_ushort(&buf[len], 0x17, 4);
	len += 6;
	make_tlv_ushort(&buf[len], 0x18, 'A');
	len += 6;
	make_tlv_ushort(&buf[len], 0x19, 1);
	len += 6;
	make_tlv_ushort(&buf[len], 0x1A, 0x0CD1);
	len += 6;
	make_tlv_uint(&buf[len], 0x14, 'U');
	len += 8;
	make_tlv_str(&buf[len], 0xf, 2, "en");
	len += 6;
	make_tlv_str(&buf[len], 0xe, 2, "us");
	len += 6;

	flap_to_socket(sock, header, buf, len);
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
		return -1;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}
	
	if (!(host = gethostbyname(AUTH_HOST))) {
		fprintf(stderr, "unable to resolve host " AUTH_HOST "\n");
		return -1;
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(AUTH_PORT);
	sa.sin_addr = *((struct in_addr *)host->h_addr);
	memset(&(sa.sin_zero), '\0', 8);
	
	if (connect(sock, (struct sockaddr*)&sa, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		return -1;
	}

//	toc_signon(sock, &send_header, argv);
	if (oscar_signon(sock, &send_header, argv) == -1)
	{
		fprintf(stderr, "signon failure");
		return -1;
	}

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

