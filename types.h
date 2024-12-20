#ifndef _TYPES_H_
#define _TYPES_H_

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef signed char s8;
typedef signed short s16;
typedef signed long s32;

#define KEY_RETURN	13
#define KEY_ESCAPE	27
#define KEY_RIGHT	28
#define KEY_LEFT	29
#define KEY_UP		30
#define KEY_DOWN	31

// input
#define STREAM		1
#define TIMING		2
#define KEYBOARD	3

// output
#define SECTOR0		1
#define SECTOR1		2
#define SECTOR2		3
#define PSGADR		4
#define PSGDATA		6

#define INTERVAL	10667L		// uS

#endif
