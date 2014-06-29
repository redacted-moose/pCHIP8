/*
 * chip8.h
 *
 *  Created on: May 11, 2012
 *      Author: Mighty Moose
 */

#ifndef CHIP8_H_
#define CHIP8_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

typedef unsigned char byte;
typedef unsigned short word;
typedef enum {false, true} bool;

typedef struct {
	byte delay_timer;
	byte sound_timer;
	byte* key;
	byte* LCD;
	byte* memory;
	byte* V;

	word I;
	word opcode;
	word pc;
	word sp;
	word* stack;

	bool drawflag;
} chip8;

extern chip8* initializeCHIP8();
extern void emulateCycle(chip8 *ctx);
extern void setKeys(chip8 *ctx);
extern bool anyKeyPressed(chip8 *ctx);
extern void destroy(chip8 *ctx);
int isKeyPressed(int basic_keycode);

bool keyPress;
byte x;
byte y;
word height;
word pixel;
bool key;

extern byte chip8_fontset[80];

#endif /* CHIP8_H_ */
