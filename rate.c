#include <u.h>
#include <libc.h>

void
main(int argc, char **argv)
{
	int rate = atoi(argv[1]);
	char *buf = malloc(rate);

	for(;;){
		if((readn(0, buf, rate)) < rate)
			break;
		write(1, buf, rate);
		sleep(235);
	}
}
