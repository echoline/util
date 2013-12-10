#include <stdio.h>

void choose(unsigned int n, unsigned int r) {
	unsigned long long j;
	unsigned long long val = 1;

	if (r == -1) {
		printf ("%llu\t", val);

		for (j = 1; j <= n; j++) {
			val *= n - j + 1;
	       		val /= j;

			printf ("%llu\t", val);
		}

		printf ("\n");

	} else {
		for (j = 1; j <= r && j <= n; j++) {
			val *= n - j + 1;
	       		val /= j;
		}

		printf ("%llu\n", val);
	}
}

int main(int argc, char **argv) {
	unsigned long long n, r;

	if (argc > 2) {
		n = atoi(argv[1]);
		r = atoi(argv[2]);

		choose(n, r);

	} else if (argc > 1) {
		n = atoi(argv[1]);

		choose(n, -1);

	} else for (n = 0; n < 13; n++) {

		choose(n, -1);
	}
}
