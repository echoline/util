#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

struct js_event {
	unsigned int time;
	short value;
	unsigned char type;
	unsigned char number;
};

int main() {
	int joystick_fd = open("/dev/fd/0", O_RDONLY | O_NONBLOCK);
	if (joystick_fd < 0)
		return -1;

	struct js_event jse;

	while(1) {
		if (read (joystick_fd, &jse, sizeof(jse)) == sizeof(jse)) {
			if (jse.type == 2) {
				if (jse.number == 1) {
					printf("%02x\n", (unsigned char)((-jse.value/32768.0)*50.0+0x40));
				} else if (jse.number == 3) {
					printf("%02x\n", (unsigned char)((-jse.value/32768.0)*50.0+0xC0));
				}
				fflush(stdout);
			}
		} else {
			usleep(125);
		}
	}
}

