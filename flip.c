#include <u.h>
#include <libc.h>

Rune runes[][2] = {
	{L'a',  L'ɐ'},
	{L'b',  L'q'},
	{L'c',  L'ɔ'},
	{L'd',  L'p'},
	{L'e',  L'ǝ'},
	{L'f',  L'ɟ'},
	{L'g',  L'ƃ'},
	{L'h',  L'ɥ'},
	{L'i',  L'ᴉ'},
	{L'j',  L'ɾ'},
	{L'k',  L'ʞ'},
	{L'l',  L'˥'},
	{L'm',  L'ɯ'},
	{L'n',  L'u'},
	{L'o',  L'o'},
	{L'p',  L'd'},
	{L'q',  L'b'},
	{L'r',  L'ɹ'},
	{L's',  L's'},
	{L't',  L'ʇ'},
	{L'u',  L'n'},
	{L'v',  L'ʌ'},
	{L'w',  L'ʍ'},
	{L'x',  L'x'},
	{L'y',  L'ʎ'},
	{L'z',  L'z'},
	{L'A',  L'∀'},
	{L'B',  L'q'},
	{L'C',  L'Ɔ'},
	{L'D',  L'p'},
	{L'E',  L'Ǝ'},
	{L'F',  L'Ⅎ'},
	{L'G',  L'פ'},
	{L'H',  L'H'},
	{L'I',  L'I'},
	{L'J',  L'ſ'},
	{L'K',  L'ʞ'},
	{L'L',  L'˥'},
	{L'M',  L'W'},
	{L'N',  L'N'},
	{L'O',  L'O'},
	{L'P',  L'Ԁ'},
	{L'Q',  L'Q'},
	{L'R',  L'ʁ'},
	{L'S',  L'S'},
	{L'T',  L'┴'},
	{L'U',  L'∩'},
	{L'V',  L'Λ'},
	{L'W',  L'M'},
	{L'X',  L'X'},
	{L'Y',  L'⅄'},
	{L'Z',  L'Z'},
	{L'\'', ','},
	{L'"',  L','},
	{L',',  L'\''},
	{L'.',  L'˙'},
	{L'!',  L'¡'},
	{L'?',  L'¿'},
	{L'_',  L'‾'},
	{L'1',  L'Ɩ'},
	{L'2',  L'ᄅ'},
	{L'3',  L'Ɛ'},
	{L'4',  L'ㄣ'},
	{L'5',  L'ϛ'},
	{L'6',  L'9'},
	{L'7',  L'ㄥ'},
	{L'8',  L'8'},
	{L'9',  L'6'},
	{L'0',  L'0'},
	{L'ɐ',  L'a'},
	{L'ɔ',  L'c'},
	{L'ǝ',  L'e'},
	{L'ɟ',  L'f'},
	{L'ƃ',  L'g'},
	{L'ɥ',  L'h'},
	{L'ᴉ',  L'i'},
	{L'ɾ',  L'j'},
	{L'ʞ',  L'k'},
	{L'˥',  L'l'},
	{L'ɯ',  L'm'},
	{L'ʁ',  L'r'},
	{L'ʇ',  L't'},
	{L'ʌ',  L'v'},
	{L'ʍ',  L'w'},
	{L'ʎ',  L'y'},
	{L'∀',  L'A'},
	{L'Ɔ',  L'C'},
	{L'Ǝ',  L'E'},
	{L'Ⅎ',  L'F'},
	{L'פ',  L'G'},
	{L'ſ',  L'J'},
	{L'Ԁ',  L'P'},
	{L'┴',  L'T'},
	{L'∩',  L'U'},
	{L'Λ',  L'V'},
	{L'⅄',  L'Y'},
	{L'˙',  L'.'},
	{L'¡',  L'!'},
	{L'¿',  L'?'},
	{L'‾',  L'_'},
	{L'Ɩ',  L'1'},
	{L'ᄅ',  L'2'},
	{L'Ɛ',  L'3'},
	{L'ㄣ',  L'4'},
	{L'ϛ',  L'5'},
	{L'ㄥ',  L'7'},
	{L'(',  L')'},
	{L')',  L'('},
	{L'[',  L']'},
	{L']',  L'['},
	{0, 0},
};

Rune
flip(Rune in){
	int i = 0;

	while(runes[i][0]){
		if(runes[i][0] == in)
			return runes[i][1];
		if(runes[i][1] == in)
			return runes[i][0];
		i++;
	}

	return in;
}

void
main(int argc, char **argv){
	Rune rune;
	char buf[8192 + UTFmax];
	char *ptr;
	Rune out[8192 + UTFmax];
	int r, i, j, l;

	while((r = read(0, buf, sizeof buf - UTFmax)) != 0) {
		if (r < 0)
			sysfatal("read: %r");

		buf[r] = '\0';

		ptr = buf;
		while(r > 0){
			while(*ptr == ' ' || *ptr == '	'){
				print("%c", *ptr++);
				r--;
			}

			l = strcspn(ptr, "\r\n");
			j = utfnlen(ptr, l);
			out[j--] = L'\0';
			while(j >= 0){
				i = chartorune(&rune, ptr);
				if(i == 1 && rune == Runeerror)
					sysfatal("invalid utf-8");
				r -= i;
				ptr += i;
				rune = flip(rune);
				out[j--] = rune;
			}

			print("%S", out);
			while(*ptr == '\r' || *ptr == '\n'){
				print("%c", *ptr++);
				r--;
			}
		}
	}
}
