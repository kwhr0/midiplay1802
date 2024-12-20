#define nofloats
#include "nstdlib.h"
#include "types.h"

#define inline
#define __inline__
#include <ctype.h>
#include <stdlib.h>
#include "ctypes.c"
#include "memcpy.c"
#include "memset.c"
#include "memswap.c"
#include "strncmp.c"
#include "qsort.c"

#define STACK_N		4
#define ENTRY_N		300

typedef struct {
	u16 cluster, index, n;
} Stack;

typedef struct {
	char name[8];
	u16 cluster;
	u32 len;
} Entry;

static Stack stack[STACK_N];
static Entry entry[ENTRY_N];
static u8 depth, meterpos, autokey;
static u16 over;
static u8 level[32];

#include "nstdlib.c"
#include "base.c"
#include "file.c"
#include "snd.c"
#include "midi.c"

static void meter(void) {
	const int W = 64 / SG_N;
	int x, y, v = 3 * level[meterpos] >> 5;
	for (y = 0; y < 24; y++)
		for (x = W * meterpos; x < W * meterpos + W; x++)
			point(x, 31 - y, y < v);
	if (++meterpos >= SG_N) meterpos = 0;
}

static void print_name_s(int d, int y) {
	int i;
	Entry *e = &entry[stack[d].index];
	char *p = e->name;
	locate(0, y);
	reverse = e->len ? 0 : 0xff;
	for (i = 0; i < 8; i++) _putc(*p++);
	reverse = 0;
}

#define copy_cluster(dst, src)\
	(((char *)&dst)[0] = src[27], ((char *)&dst)[1] = src[26])

static int cmp_entry(const void *a, const void *b) {
	return strncmp(((Entry *)a)->name, ((Entry *)b)->name, 8);
}

static void list(void) {
	static char vramsave[VRAMSIZE];
	Stack *s = &stack[depth];
	int i;
	memcpy(VRAM, vramsave, VRAMSIZE);
	while (1) {
		Entry *e;
		char *buf;
		DirOpen(s->cluster);
		for (i = 0; buf = DirRead();)
			if (buf[8] == 'M' && buf[9] == 'I' && buf[10] == 'D' || buf[11] & 0x10) {
				e = &entry[i++];
				memcpy(e->name, buf, 8);
				copy_cluster(e->cluster, buf);
				((char *)&e->len)[0] = 0;
				((char *)&e->len)[1] = buf[30];
				((char *)&e->len)[2] = buf[29];
				((char *)&e->len)[3] = buf[28];
			}
		if (!i) {
			printf("Error: directory is empty.\n");
			exit(1);
		}
		s->n = i;
		qsort(entry, i, sizeof(Entry), cmp_entry);
		while (1) {
			int c;
			print_name_s(depth, depth);
			e = &entry[s->index];
			c = autokey ? autokey : keydown();
			switch (c) {
			case KEY_LEFT:
				if (s->index > 0) {
					s->index--;
					if (autokey) autokey = KEY_RETURN;
				}
				else autokey = 0;
				for (i = depth + 1; i < STACK_N; i++) stack[i].index = 0;
				break;
			case KEY_RIGHT:
				if (s->index < s->n - 1) {
					s->index++;
					if (autokey) autokey = KEY_RETURN;
				}
				else if (autokey) autokey = KEY_UP;
				for (i = depth + 1; i < STACK_N; i++) stack[i].index = 0;
				break;
			case KEY_UP: case KEY_ESCAPE:
				if (depth > 0) {
					locate(0, depth);
					for (i = 0; i < 8; i++) _putc(' ');
					s = &stack[--depth];
					if (autokey) autokey = KEY_RIGHT;
					goto next;
				}
				break;
			case KEY_DOWN: case KEY_RETURN:
				if (e->len) {
					memcpy(vramsave, VRAM, VRAMSIZE);
					printf("START ");
					for (i = 0; i < 8 && e->name[i] > ' '; i++)
						putc(e->name[i]);
					printf(".MID\n");
					FileOpen(e->cluster, e->len);
					autokey = 0;
					return;
				}
				else if (depth < STACK_N) {
					s = &stack[++depth];
					s->cluster = e->cluster;
					goto next;
				}
				break;
			default:
				if (isalnum(c)) {
					c = toupper(c);
					for (i = 0; i < s->n && c > *entry[i].name; i++)
						;
					if (i < s->n) s->index = i;
				}
				break;
			}
		}
next:;
	}
}

int main(void) {
	char *buf;
	SndInit();
	FileInit();
	DirOpen(0);
	while ((buf = DirRead()) && strncmp(buf, "MIDI    ", 8))
		;
	if (!buf) {
		printf("Error: /MIDI not found.\n");
		return 1;
	}
	copy_cluster(stack[0].cluster, buf);
	while (1) {
		int endcount;
		list();
		cls();
		print_name_s(depth, 0);
		MidiInit();
		if (MidiHeader()) break;
		over = 0;
		endcount = 0;
		while (1) {
			int start = inp(TIMING), end, c;
			if (MidiProcess() && !endcount) endcount = 1;
			SndProcess();
			switch (c = keydown()) {
			case KEY_UP: case KEY_ESCAPE:
				autokey = 0;
				goto next;
			case KEY_LEFT: case KEY_RIGHT:
				autokey = c;
				goto next;
			default:
				if (!endcount || ++endcount < 200) break;
				autokey = KEY_RIGHT;
				goto next;
			}
			end = inp(TIMING);
			if (start != end) printf("OVER %d\n", ++over);
			meter();
#if 1
			while (start == (end = inp(TIMING)))
				meter();
#endif
		}
next:;
		SndInit();
	}
	return 0;
}

