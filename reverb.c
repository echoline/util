#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <thread.h>

#define RATE 44100

int rate;
double decay;
int delay;
Point c;

void
reverb(unsigned char *buf, short *tmp, int len)
{
	short l, r;
	int i;
	int dur = delay;

	for(i = 0; i < len; i += 4){
		l = buf[i+0] | (buf[i+1] << 8);
		r = buf[i+2] | (buf[i+3] << 8);

		l += tmp[(i/2+0+dur)%len] * decay;
		r += tmp[(i/2+1+dur)%len] * decay;

		tmp[(i/2+0)%len] = l;
		tmp[(i/2+1)%len] = r;

		buf[i+0] = l & 0xFF;
		buf[i+1] = (l >> 8) & 0xFF;
		buf[i+2] = r & 0xFF;
		buf[i+3] = (r >> 8) & 0xFF;
	}
}

void
doproc(void *unused)
{
	unsigned char *buf = malloc(rate * 4);
	short *tmp = malloc(rate * 2);
	int r;

	memset(tmp, 0, rate * 2);

	while((r = read(0, buf, rate * 4)) > 0){
		reverb(buf, tmp, r);
		write(1, buf, r);
	}
}

void
redraw(Image *screen)
{
	int x = delay;

	draw(screen, screen->r, display->white, nil, ZP);

	draw(screen, Rect(screen->r.min.x + x - 1, screen->r.min.y,
					screen->r.min.x + x + 1, screen->r.max.y - Dy(screen->r)/2),
				display->black, nil, ZP);

	x = decay * Dx(screen->r);

	draw(screen, Rect(screen->r.min.x + x - 1, screen->r.min.y + Dy(screen->r)/2,
					screen->r.min.x + x + 1, screen->r.max.y),
				display->black, nil, ZP);
}

void
eresized(int new)
{
	if(new && getwindow(display, Refnone) < 0)
		fprint(2,"can't reattach to window");
	c = divpt(subpt(screen->r.max, screen->r.min), 2);
	redraw(screen);
}

void
threadmain(int argc, char **argv)
{
	Mouse m;
	Point xy;
	rate = RATE;
	delay = 0;
	decay = 0.0;

	if (argc > 1)
		rate = atoi(argv[1]);

	if(initdraw(0, 0, "reverb") < 0)
		sysfatal("initdraw: %r");
	einit(Emouse);

	eresized(0);
	proccreate(doproc, nil, mainstacksize);

	for(;;m = emouse()){
		if(m.buttons & 1){
			xy = subpt(m.xy, screen->r.min);

			if (xy.y > c.y)
				decay = xy.x / (double)Dx(screen->r);
			else
				delay = xy.x;
			
			redraw(screen);
		}
	}
}
