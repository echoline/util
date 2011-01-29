/**
 * program to tack on a prefix and suffix to each line of stdin, and put 
 * the result into standard output
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

fuck(char *s) {
	printf("%s\n", s);
	exit(1);
}

int main(int argc, char **argv) {
	char buf[4096];
	char prefix[80] = "";
	char suffix[80] = "";
	int arg = 1;
	while (arg < argc) {
		if (!strcmp(argv[arg], "-p"))
			if (++arg < argc)
				strncpy(prefix, argv[arg], 79);
			else
				fuck("no parameter for -p");
		else if (!strcmp(argv[arg], "-s"))
			if (++arg < argc)
				strncpy(suffix, argv[arg], 79);
			else
				fuck("no parameter for -s");
		else {
			fuck("usage: prefixer [-p prefix] [-s suffix]");
		}
	}
	while (fgets(buf, 80, stdin)) {
		buf[strcspn(buf, "\n")] = '\0';
		if (buf[0] == '/') {
			if (!strncasecmp(buf, "/p", 2)) {
				if (buf[2] == ' ')
					strncpy(prefix, buf + 3, 80);
				else {
					fprintf(stderr, "%s\n", prefix);
					fflush(stderr);
				}
			} else if (!strncasecmp(buf, "/s", 2)) {
				if (buf[2] == ' ')
					strncpy(suffix, buf + 3, 80);
				else {
					fprintf(stderr, "%s\n", suffix);
					fflush(stderr);
				}
			} else if (!strncasecmp(buf, "/h", 2)) {
				fprintf(stderr, "\t/[message]\tsend message raw to server\n");
				fprintf(stderr, "\t/p [prefix]\tset prefix to prepend to normal message\n");
				fprintf(stderr, "\t/p\t\tdisplay current prefix\n");
				fprintf(stderr, "\t/s [suffix]\tsuffix to append to normal message\n");
				fprintf(stderr, "\t/s\t\tdisplay current suffix\n");
				fprintf(stderr, "\t/h\t\tthis help\n");
			} else {
				printf("%s\n", buf + 1);
				fflush(stdout);
			}
		} else {
			printf("%s%s%s\n", prefix, buf, suffix);
			fflush(stdout);
		}
	}
	fflush(stdout);
}
