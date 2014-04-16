#include "comms.h"
#include <strings.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

//#define debug(buf,len); write(1,buf,len);write(1,"\n",1);
#define debug(buf,len); {};

int main(int argc, char **argv) {
	char inbuf[8196];
	char outbuf[8196];
	int ioffs, ooffs;
	int r, client, sock;
	struct pollfd fds[2];
	int fd;
	FILE *f;

	if (argc < 4) {
		printf("usage: %s filepath [tcp|unix] [port|path]\n", argv[0]);
		return -1;
	}

	fd = open (argv[1], O_RDWR);
	if (fd == -1) {
		perror ("open");
		return -1;
	}
	f = fdopen (fd, "rw");

	sock = strcasecmp(argv[2], "tcp")? init_unix_listen(argv[3])
					 : init_tcp_listen(atoi(argv[3]));

	if ((client = accept(sock, NULL, NULL)) > -1) {
		fds[0].fd = client;
		fds[1].fd = fd;
		fds[0].events = POLLIN | POLLHUP;
		fds[1].events = POLLIN | POLLHUP;

		ioffs = ooffs = 0;

		while (poll(fds, 2, 1) >= 0) {
			if (fds[0].revents & POLLIN) {
				r = read(client, &inbuf[ioffs],
							sizeof(inbuf)-ioffs);

				if (r <= 0) {
					perror("read");
					close (client);
					break;
				}

				ioffs += r;
			} else if (ioffs) {
				write(fd, inbuf, ioffs);
				fsync(fd);

				debug (inbuf, ioffs);

				ioffs = 0;
			}
			
			if (fds[1].revents & POLLIN) {
				r = read (fd, &outbuf[ooffs],
						sizeof(outbuf)-ooffs);
				if (r <= 0) {
					perror ("read");
					return 0;
				}

				ooffs += r;
			} else if (ooffs) {
				write(client, outbuf, ooffs);
				fsync(client);

				debug (outbuf, ooffs);

				ooffs = 0;
			}

			for (r = 0; r < 2; r++)
				if (fds[r].revents & POLLHUP) {
					close (client);
					return 0;
				}
		}

		perror("poll");
		close(client);
		return -1;
	}

	perror ("accept");
	return -1;
}
