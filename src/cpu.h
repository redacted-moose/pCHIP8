/*
 * cpu.h
 *
 *  Created on: May 11, 2012
 *      Author: Mighty Moose
 */

#ifndef CPU_H_
#define CPU_H_

#include "chip8.h"

typedef struct {
	byte delay_timer;
	byte sound_timer;
	byte key[16];
	byte memory[0x1000];
	byte V[16];

	word I;
	word pc;
	word sp;
	word stack[16];

	bool drawflag;
} CHIP8_STATE;

extern CHIP8_STATE chip;

extern void load(const char* path);
extern void execute();
extern void check_keys();
extern bool anyKeyPressed();
/* int isKeyPressed(int basic_keycode); */

extern byte chip8_fontset[80];

#endif /* CPU_H_ */
