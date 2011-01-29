/**
 * program to mix together fifos specified by args with standard input, all
 * into standard output
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#define ERRORMSG "please create missing fifo with `mkfifo %s'\n"

int main(int argc, char **argv) {
	int ret, i;
	struct pollfd *pfds;
	char buf[4096];
	bool flag;

	if (argc < 2) {
		fprintf(stderr, "usage: %s fifo(s)\n", argv[0]);
		return -1;
	}
	/* there are as many pfds as the number of args, plus one for stdin,
	 * so use the number of args plus the command name */
	pfds = malloc(sizeof(struct pollfd) * argc);

	/* set up the pfds for stdin and all the arguments to the program */
	pfds[0].fd = 0;
	pfds[0].events = POLLIN;
	for (i = 1; i < argc; i++) {
		pfds[i].fd = open(argv[i], O_RDONLY);
		pfds[i].events = POLLIN;
		if (pfds[i].fd == -1) {
			fprintf(stderr, ERRORMSG, argv[i]);
			return -1;
		}
	}

	/* main loop waits each time for a valid pollfd */
	while(poll(pfds, argc, -1) > 0) {
		flag = true;
		// check stdin
		if (pfds[0].revents & POLLIN) {
			flag = false;
			// read stdin
			ret = read(pfds[0].fd, buf, 4096);
			buf[ret] = '\0';
			// exit on error or end
			if (ret < 1) {
				fprintf(stderr, "exiting\n");
				for (i = 0; i < argc; i++)
					close(pfds[i].fd);
				return 1;
			}
			// or print read buffer
			printf("%s", buf);
			//fprintf(stderr, "%s", buf);
			fflush(stdout);
		}
		// loop through other fds
		for (i = 1; i < argc; i++) {
			// check current fd
			if (pfds[i].revents & POLLIN) {
				flag = false;
				// read it
				ret = read(pfds[i].fd, buf, 4096);
				buf[ret] = '\0';
				// reopen on error or end
				if (ret < 1) {
					close(pfds[i].fd);
					pfds[i].fd = open(argv[i],
							O_RDONLY);
				}
				// print read buffer
				printf("%s", buf);
			//	fprintf(stderr, "%s", buf);
				fflush(stdout);
			}
		}
		if (flag) {
			fprintf(stderr, "no activity from poll\n");
			return -1;
		}
	}
	perror("poll");
}
