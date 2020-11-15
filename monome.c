#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <thread.h>

Image *back;

Rectangle buttons[16][16];
char buttonstate[16][16];
double tones[16];

#define ROOT2 1.05946309436
#define RATE 44100

int rate;

void
redraw(Image *screen)
{
	int i, j;

	draw(screen, screen->r, back, nil, ZP);

	for(i = 0; i < 16; i++)
		for(j = 0; j < 16; j++)
			draw(screen, rectaddpt(buttons[i][j], screen->r.min),
				buttonstate[i][j]? display->white: display->black,
				nil, ZP);
}

void
resize(void)
{
	int fd = open("/dev/wctl", OWRITE);
	if(fd >= 0){
		fprint(fd, "resize -dx 650 -dy 650\n");
		close(fd);
	}
}

void
eresized(int new)
{
	if(new && getwindow(display, Refnone) < 0)
		fprint(2,"can't reattach to window");
	resize();
	redraw(screen);
}

void
audioproc(void *unused)
{
	int i, j, step, dur;
	double tone, sum;
	short out;
	char *buf;

	dur = rate/4;
	buf = malloc(dur * 2);

	for(;;){
		for(j = 0; j < 16; j++){
			for(step = 0; step < dur; step++){
				sum = 0.0;
				for(i = 0; i < 16; i++){
					if(buttonstate[j][i]){
						tone = tones[16 - i - 1];
						sum += cos(tone * ((double)step/rate) * 2*PI);
					}
				}
				out = (short)(sum * 2047.0 * pow(0.9999, step/10.0));
				buf[step*2+0] = out & 0xFF;
				buf[step*2+1] = (out >> 8) & 0xFF;
			}
			write(1, buf, dur*2);
		}
	}

	free(buf);
}

void
threadmain(int argc, char **argv)
{
	int i, j, k;
	Mouse m;
	Point xy;

	rate = RATE;
	if (argc > 1)
		rate = atoi(argv[1]);

	if(initdraw(0, 0, "monome") < 0)
		sysfatal("initdraw: %r");
	einit(Emouse);

	back = allocimage(display, Rect(0,0,1,1), CMAP8, 1, 0x777777FF);
	for(i = 0; i < 16; i++) {
		memset(buttonstate[i], '\0', 16);
		tones[i] = 220;
		for(k = 0; k < i; k++)
			tones[i] *= ROOT2;
		for(j = 0; j < 16; j++)
			buttons[i][j] = insetrect(Rect(i*40, j*40, (i+1)*40, (j+1)*40), 3);
	}
	eresized(0);

	proccreate(audioproc, nil, 8192*8);

	for(;;m = emouse()){
		if(m.buttons & 1){
			xy = subpt(m.xy, screen->r.min);
			for(i = 0; i < 16; i++)
				for(j = 0; j < 16; j++)
					if(ptinrect(xy, buttons[i][j])){
						buttonstate[i][j] = !buttonstate[i][j];
						break;
					}
			redraw(screen);
		}
	}
}
