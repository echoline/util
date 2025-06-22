#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "ann.h"

int
annsave(char *filename, Ann *ann)
{
	Weights *W;
	int i, j, k;
	int fd = open(filename, O_WRONLY | O_CREAT, 0644);
	if (fd < 0) {
		perror("create");
		return -1;
	}

	dprintf(fd, "%d\n", ann->n);
	for (i = 0; i < ann->n; i++)
		dprintf(fd, "%d\n", ann->layers[i]->n);

	for (i = 0; i < (ann->n - 1); i++) {
		W = ann->weights[i];
		for (j = 0; j <= W->inputs; j++)
			for (k = 0; k < W->outputs; k++)
				dprintf(fd, "%f\n", W->values[j][k]);
	}

	close(fd);
	return 0;
}

char *retline = nil;

char*
readline(int fd)
{
	static int length;
	static int offset;
	char *epos;
	int r;

	if (retline == nil) {
		length = 0x1000;
		offset = 0;
		retline = malloc(length);
		retline[offset] = '\0';
	}

	r = strlen(retline);
	if (r > 0) {
		r++;
		memmove(retline, &retline[r], length - r);
		length -= r;
		offset = strlen(retline);
	}

	if (length < 0x1000) {
		retline = realloc(retline, length + 0x1000);
		length += 0x1000;
	}

	do {
		epos = strchr(retline, '\n');
		if (epos != nil) {
			*epos = '\0';
			return retline;
		}

		r = read(fd, &retline[offset], length - offset - 1);
		if (r < 0) {
			perror("read");
			exit(-3);
		}
		if (r > 0) {
			offset += r;
			retline[offset] = '\0';
			length += r;
			retline = realloc(retline, length);
		}
	} while(r != 0);

	return nil;
}

char*
sreadline(int fd)
{
	char *ret = readline(fd);
	if (ret == nil) {
		dprintf(2, "error: early end of file\n");
		exit(-4);
	}
	return ret;
}

Ann*
annload(char *filename)
{
	Weights *W;
	Ann *ann = nil;
	char *buf;
	int i, j, k;
	int num_layers, *layers;
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return ann;
	}

	buf = sreadline(fd);
	num_layers = atoi(buf);

	if (num_layers < 2) {
		dprintf(2, "num_layers < 2\n");
		return ann;
	}

	layers = calloc(num_layers, sizeof(int));
	for (i = 0; i < num_layers; i++) {
		buf = sreadline(fd);
		layers[i] = atoi(buf);
	}

	ann = anncreatev(num_layers, layers);
	for (i = 0; i < (ann->n - 1); i++) {
		W = ann->weights[i];
		for (j = 0; j <= W->inputs; j++)
			for (k = 0; k < W->outputs; k++) {
				buf = sreadline(fd);
				W->values[j][k] = atof(buf);
			}
	}

	free(retline);
	retline = nil;

	return ann;
}
