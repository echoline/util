#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ann.h"

void
usage(char **argv)
{
	dprintf(2, "usage: %s [-train] filename [num_layers num_input_layer ... num_output_layer]\n", argv[0]);
	exit(-1);
}

void
main(int argc, char **argv)
{
	Ann *ann;
	char *filename;
	int train;
	int num_layers = 0;
	int *layers = nil;
	int i;
	char *line;
	double *input;
	double *output = nil;
	double *runoutput;
	int ninput;
	int noutput;
	int offset;
	double f;
	int trainline;
	int nline;
	struct stat statbuf;

	train = 0;

	if (argc < 2)
		usage(argv);

	filename = argv[1];

	if (argv[1][0] == '-' && argv[1][1] == 't') {
		if (argc < 3)
			usage(argv);

		train = 1;
		filename = argv[2];
	}

	ann = nil;
	if (stat(filename, &statbuf) == 0) {
		ann = annload(filename);
		if (ann == nil)
			exit(-5);
	}

	if (argc >= (train + 3)) {
		num_layers = atoi(argv[train + 2]);

		if (num_layers < 2 || argc != (train + 3 + num_layers))
			usage(argv);

		layers = calloc(num_layers, sizeof(int));

		for (i = 0; i < num_layers; i++)
			layers[i] = atoi(argv[train + 3 + i]);
	}

	if (num_layers > 0) {
		if (ann != nil) {
			if (ann->n != num_layers) {
				dprintf(2, "num_layers: %d != %d\n", ann->n, num_layers);
				exit(-6);
			}

			for (i = 0; i < num_layers; i++) {
				if (layers[i] != ann->layers[i]->n) {
					dprintf(2, "num_layer_%d: %d != %d\n", i, layers[i], ann->layers[i]->n);
					exit(-7);
				}
			}
		} else {
			ann = anncreatev(num_layers, layers);
			if (ann == nil)
				exit(-8);
		}
	}

	if (ann == nil) {
		dprintf(2, "file not found: %s\n", filename);
		exit(-9);
	}

	ninput = ann->layers[0]->n;
	noutput = ann->layers[ann->n - 1]->n;
	input = calloc(ninput, sizeof(double));
	if (train == 1)
		output = calloc(noutput, sizeof(double));

	trainline = 0;
	nline = ninput;

	do {
		int i = 0;
		while ((line = readline(0)) != nil) {
			do {
				if (strlen(line) == 0)
					break;
				while(isspace(*line))
					line++;
				if (strlen(line) == 0)
					break;
				offset = 0;
				while (isdigit(line[offset]) || line[offset] == '.' || line[offset] == '-')
					offset++;
				if (!isspace(line[offset]) && line[offset] != '\0') {
					dprintf(2, "input error: %s\n", line);
					exit(-10);
				}
				f = atof(line);
				if (trainline == 0) {
					input[i] = f;
					i++;
				} else {
					output[i] = f;
					i++;
				}
				line = &line[offset];
				while(isspace(*line))
					line++;
			} while(i < nline && strlen(line) > 0);

			if (i == nline) {
				if (trainline == 0) {
					runoutput = annrun(ann, input);
					for (i = 0; i < noutput; i++)
/*						if (runoutput[i] == 0.0)
							printf("0%c", (i == (noutput-1))? '\n': ' ');
						else if (runoutput[i] == 1.0)
							printf("1%c", (i == (noutput-1))? '\n': ' ');
						else */
							printf("%f%c", runoutput[i], (i == (noutput-1))? '\n': ' ');
					free(runoutput);
				}

				if (train == 1) {
					if (trainline == 0) {
						trainline = 1;
						nline = noutput;
					} else {
						anntrain(ann, input, output);
						trainline = 0;
						nline = ninput;
					}
				}
				i = 0;
			}
		}
	} while(line != nil);

	if (train == 1 && annsave(filename, ann) != 0)
		exit(-11);

	exit(0);
}
