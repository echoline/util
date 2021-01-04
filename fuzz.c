#include <u.h>
#include <libc.h>
#include <fcall.h>

void
usage(char *argv0)
{
	fprint(2, "usage:\n\t%s seednumber\n", argv0);
	exits("usage");
}

void
randfcall(uchar *ap, uint nap)
{
	int n;

	ap[0] ^= 0x1; // Tmsg/Rmsg

	for(n = rand() % nap; n < nap; n += (rand() % nap)) {
		ap[n] = rand() % 0x100;
	}
}

void
main(int argc, char **argv)
{
	Fcall fcall;
	int r;
	uchar *buf;

	if (argc < 2)
		usage(argv[0]);

	buf = malloc(0x10000);

	srand(atoi(argv[1]));

	while((r = read9pmsg(0, buf, 0x10000)) > 0) {
		randfcall(buf, r);
		r = convS2M(&fcall, buf, 0x10000);
		if(write(1, buf, r) != r)
			sysfatal("write: %r");
	}

	sysfatal("read9pmsg: %r");
}
