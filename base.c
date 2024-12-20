#include "font8x8/font8x8_basic.h"

#define VRAM			((char *)0xff00)
#define VRAMSIZE		0x100
#define locate(x, y)	(cursX = (x & 7), cursY = (y & 3))
#define cls()			memset(VRAM, 0, VRAMSIZE)

static u8 cursX, cursY, reverse;

void point(int x, int y, int f) {
	static const u8 m[] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 };
	char *p;
	x &= 0x3f;
	y &= 0x1f;
	p = VRAM + (y << 3) + (x >> 3);
	x &= 7;
	if (f) *p |= m[x];
	else *p &= ~m[x];
}

void _putc(char c) {
	int i;
	char *src, *dst;
	if (c & 0x80) return;
	src = font8x8_basic[c];
	dst = VRAM + (cursY << 6) + cursX;
	for (i = 0; i < 8; i++)
		dst[i << 3] = src[i] ^ reverse;
	if (++cursX >= 8) {
		cursX = 0;
		if (++cursY >= 4) cursY = 0;
	}
}

int keydown(void) {
	static u8 k0, t0, cnt, rep;
	u8 k = inp(KEYBOARD), r, t;
	if (k0 != k) {
		k0 = k;
		cnt = k ? 40 : 0;
		rep = 128;
		return k;
	}
	if (cnt) {
		t = inp(TIMING);
		if (t0 != t) {
			t0 = t;
			if (!--cnt) {
				cnt = rep >> 4;
				if (rep > 16) rep--;
				return k;
			}
		}
	}
	return 0;
}

