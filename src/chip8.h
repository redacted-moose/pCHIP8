/*
 * chip8.h
 *
 *  Created on: May 11, 2012
 *      Author: Mighty Moose
 */

#ifndef CHIP8_H_
#define CHIP8_H_

#include <stdint.h>
#include "SDL/SDL.h"

typedef uint8_t byte;
typedef uint16_t word;
typedef enum {false, true} bool;

extern SDL_Surface* screen;
extern bool run;

#define GPU_SCREEN_HEIGHT 32
#define GPU_SCREEN_WIDTH 64

extern void set_pixel(int x, int y, Uint32 color);
extern Uint32 get_pixel(int x, int y);

#endif /* CHIP8_H_ */
