// produce waveform for a "beep"
#include <u.h>
#include <libc.h>

void
usage(void)
{
	fprint(2, "usage: %s [-f hz] [-d decay] [-l samples] [-s slide] [-v volume]\n", argv0);
	exits("usage");
}

void
main(int argc, char **argv)
{
	char *buffer;
	short s;
	int i;
	double hz = 440;
	double decay = 0.9998;
	int samples = 11025;
	double slide = 1.0001;
	double volume = 0.33;
	double *table;

	ARGBEGIN{
	case 'f':
		hz = atof(EARGF(usage()));
		break;
	case 'd':
		decay = atof(EARGF(usage()));
		break;
	case 'l':
		samples = atoi(EARGF(usage()));
		break;
	case 's':
		slide = atof(EARGF(usage()));
		break;
	case 'v':
		volume = atof(EARGF(usage()))/100.0;
		break;
	default:
		usage();
	}ARGEND

	volume = volume < 0? 0: volume > 1? 1: volume;

	table = malloc(6283 * sizeof(double));
	for (i = 0; i < 6283; i++) {
		table[i] = cos(i/1000.0);
	}

	buffer = malloc(samples * 4);

	for (i = 0; i < samples; i++) {
		s = table[(int)(fmod(((double)i/44100*2*PI*(hz-pow(slide,i))),6.283)*1000.0)] * 32767 * volume * pow(decay,i);
		buffer[i*4+0] = buffer[i*4+2] = s & 0xFF;
		buffer[i*4+1] = buffer[i*4+3] = (s >> 8) & 0xFF;
	}

	write(1, buffer, samples * 4);
}
