#include <stdio.h>
#include <ctype.h>

int irc2ansi(int irc) {
	switch(irc) {
	case 0:
		return 7;
	case 1:
		return 0;
	case 2:
		return 4;
	case 3:
		return 2;
	case 4:
		return 1;
	case 6:
		return 5;
	case 8:
		return 3;
	case 9:
		return 2;
	case 10:
	case 11:
	case 12:
		return 6;
	default:
		return 1;
	}
}

int main() {
	unsigned char bold = 0, color = 0;
	int i = 0, r, fg, bg;

	while (1) {
		r = getchar();

		if (r == EOF)
			break;

		if (r == '\002') {
			bold = !bold;

			if (bold)
				printf("\x1b[1m");
			else
				printf("\x1b[0m");
		} else if (r == '\003') {
			if (color > 1) {
				printf("\x1b[0m");
				color = 0;
			} else if (color == 0) {
				color = 1;
				fg = 0;
				bg = 0;
			}
		} else if (color % 3 != 0) {
			if (isdigit(r)) {
				if (color == 1) {
					fg *= 10;
					fg += r - '0';
				} else if (color == 2) {
					bg *= 10;
					bg += r - '0';
				}
 			} else if (r == ',') {
				color = 2;
			} else {
				fg = irc2ansi(fg) + 30;
				if (color == 2) {
					bg = irc2ansi(bg) + 40;
					printf("\x1b[%d;%dm%c", fg, bg, (char)r);
				} else {
					printf("\x1b[%dm%c", fg, (char)r);
				}
				color = 3;
			}
		} else if (r == '\n') {
			color = bold = 0;
			printf("\x1b[0m\n");
		} else
			putchar(r);
	}
}
