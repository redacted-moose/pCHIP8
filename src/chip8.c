/*
 * chip8.c
 *
 *  Created on: May 11, 2012
 *      Author: Mighty Moose
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "chip8.h"
#include "cpu.h"

CHIP8_STATE chip;
SDL_Surface* screen;
bool run = true;


void reset() {
    chip.pc = 0x200;
    chip.I = 0;
    chip.sp = 0;

    // Load fontset
    for(int i = 0; i < 80; i++) {
		chip.memory[i] = chip8_fontset[i];
    }

    chip.delay_timer = 0;
    chip.sound_timer = 0;

    // Initialize SDL Context

	SDL_Color colors[256];

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

    // Make a grayscale color palette
	for(int i = 0; i < 256; i++){
		colors[i].r = i;
		colors[i].g = i;
		colors[i].b = i;
	}

    atexit(SDL_Quit);

	screen = SDL_SetVideoMode(GPU_SCREEN_WIDTH, GPU_SCREEN_HEIGHT, 8, SDL_SWSURFACE);

    if (screen == NULL) {
		fprintf(stderr, "Couldn't set %dx%dx8 video mode: %s\n",
				GPU_SCREEN_WIDTH, GPU_SCREEN_HEIGHT, SDL_GetError());
		exit(1);
	}

    // Set a grayscale color palette
	SDL_SetColors(screen, colors, 0, 256);

	SDL_WM_SetCaption("CHIP8", NULL);

}

void set_pixel(int x, int y, Uint32 color) {
    if (SDL_MUSTLOCK(screen)) {
        if (SDL_LockSurface(screen) < 0) {
            fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
            return;
        }
    }

	int bpp = screen->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *) screen->pixels + y * screen->pitch + x * bpp;

	switch (bpp) {
	case 1:
		*p = color;
		break;

	case 2:
		*(Uint16 *) p = color;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (color >> 16) & 0xff;
			p[1] = (color >> 8) & 0xff;
			p[2] = color & 0xff;
		} else {
			p[0] = color & 0xff;
			p[1] = (color >> 8) & 0xff;
			p[2] = (color >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32 *) p = color;
		break;
	}

    if (SDL_MUSTLOCK(screen)) {
        SDL_UnlockSurface(screen);
    }

}

/**
 * Return the pixel value at (x, y)
 */
Uint32 get_pixel(int x, int y)
{
    if (SDL_MUSTLOCK(screen)) {
        if (SDL_LockSurface(screen) < 0) {
            fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
            return 0;
        }
    }

    int bpp = screen->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }

    if (SDL_MUSTLOCK(screen)) {
        SDL_UnlockSurface(screen);
    }
}

int main(int argc, char **argv) {

    reset();

    if (argc == 2) {
        load(argv[1]);
    } else {
        load("rom.c8");
    }

	while (run) {

        Uint32 t1 = SDL_GetTicks();

        int cycles = 0;
        while (10 > cycles) {
            execute();
            cycles++;
            check_keys(chip);
        }

        if (chip.delay_timer > 0) { //Optimization: if (delay_timer)
            chip.delay_timer--;
        }

        if (chip.sound_timer > 0) {
            printf("BEEP!\n"); //To be implemented later
            chip.sound_timer--;
        }

        if (chip.drawflag) {
            SDL_UpdateRect(screen, 0, 0, GPU_SCREEN_WIDTH, GPU_SCREEN_HEIGHT);
        }

        Uint32 t2 = SDL_GetTicks();

        SDL_Delay(17 - (t2 - t1));
	}

    SDL_Quit();

	return 0;
}
