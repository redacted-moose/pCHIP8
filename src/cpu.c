/*
 * cpu.c
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

#define X ((opcode & 0x0F00) >> 8)
#define Y ((opcode & 0x00F0) >> 4)
#define N (opcode & 0x000F)
#define NN (opcode & 0x00FF)
#define NNN (opcode & 0x0FFF)

#define WHITE 0xFF
#define BLACK 0x00


void load(const char* path) {
    FILE *romfile = fopen(path, "rb");
    if(romfile == NULL) {
        fprintf(stderr, "File error: %s does not exist?\n", path);
        exit(1);
    }

    // Get file size
    fseek(romfile, 0L, SEEK_END);
    long size = ftell(romfile);
    fseek(romfile, 0L, SEEK_SET);
    fprintf(stdout, "File size is %ld\n", size);

    long result = fread(chip.memory + 0x200, sizeof(byte), size, romfile);
    fprintf(stdout, "result is %ld\n", result);
    if(result != size) {
        fprintf(stderr, "Reading error\n");
        exit(3);
    }

    fclose(romfile);
}

void execute() {
	word opcode = chip.memory[chip.pc] << 8 | chip.memory[chip.pc + 1];

    chip.pc += 2;
    chip.pc %= 0x1000;

	switch (opcode & 0xF000) {
    case 0x0000:
        switch (N) { // 0x0XYN
        case 0x0: // 0x00E0: Clear the screen
            SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
            chip.drawflag = true;
            break;

        case 0xE: // 0x00EE: Return from a subroutine
            chip.sp--;
            chip.pc = chip.stack[chip.sp];
            break;

        default:
            fprintf(stderr, "Unknown opcode [0x0000]: 0x%4X\n", opcode);
            exit(-1);
        }
        break;

    case 0x1000: // 0x1NNN: Jump to immediate 12 bit address
        chip.pc = NNN;
        break;

    case 0x2000: // 0x2NNN: Call immediate 12 bit address
        chip.stack[chip.sp] = chip.pc;
        chip.sp++;
        chip.pc = NNN;
        break;

    case 0x3000: // 0x3XNN: Skip next if V[X] == NN
        chip.pc += (chip.V[X] == NN) ? 2 : 0;
        break;

    case 0x4000: // 0x4XNN: Skip next if V[X] != NN
        chip.pc += (chip.V[X] != NN) ? 2 : 0;
        break;

    case 0x5000: // 0x5XY0: Skip next if V[X] == V[Y]
        chip.pc += (chip.V[X] == chip.V[Y]) ? 2 : 0;
        break;

    case 0x6000: // 0x6XNN: Store NN in V[X]
        chip.V[X] = NN;
        break;

    case 0x7000: // 0x7XNN: V[X] += NN
        chip.V[X] += NN;
        break;

    case 0x8000: // 0x8XYN
        switch (N) {
        case 0x0: // 0x8XY0: V[X] = V[Y]
            chip.V[X] = chip.V[Y];
            break;

        case 0x1: // 0x8XY1: V[X] |= V[Y]
            chip.V[X] |= chip.V[Y];
            break;

        case 0x2: // 0x8XY2: V[X] &= V[Y]
            chip.V[X] &= chip.V[Y];
            break;

        case 0x3: // 0x8XY3: V[X] ^= V[Y]
            chip.V[X] ^= chip.V[Y];
            break;

        case 0x4: // 0x8XY4: add V[Y] to V[X], V[F] set to carry if carry
            chip.V[0xF] = (chip.V[Y] > (0xFF - chip.V[X])) ? 1 : 0;
            chip.V[Y] += chip.V[X];
            break;

        case 0x5: // 0x8XY4: subtract V[Y] from V[X], V[F] set to borrow
            chip.V[0xF] = (chip.V[Y] > chip.V[X]) ? 1 : 0;
            chip.V[X] -= chip.V[Y];
            break;

        case 0x6: // 0x8XY6: V[X] = V[Y] >> 1, V[F] = least significant bit of V[Y]
            chip.V[0xF] = chip.V[Y] & 0x01;
            chip.V[X] = (chip.V[Y] >> 1) & 0x7F;
            break;

        case 0x7: // 0x8XY7: V[X] = V[Y] - V[X], V[F] set to borrow status
            chip.V[0xF] = (chip.V[X] > chip.V[Y]) ? 1 : 0;
            chip.V[X] = chip.V[Y] - chip.V[X];
            break;

        case 0xE: // 0x8XYE: V[X] = V[Y] << 1, V[F] = most significant bit of V[Y]
            chip.V[0xF] = (chip.V[Y] >> 7) & 0x01;
            chip.V[X] = chip.V[Y] << 1;
            break;

        default:
            fprintf(stderr, "Unknown opcode [0x8000]: 0x%4X\n", opcode);
            exit(-1);
        }
        break;

    case 0x9000: // 0x9XY0: Skip next if V[X] != V[Y]
        chip.pc += (chip.V[X] != chip.V[Y]) ? 2 : 0;
        break;

    case 0xA000: // 0xANNN: I = NNN
        chip.I = NNN;
        break;

    case 0xB000: // 0xBNNN: Jump to NNN + V[0]
        chip.pc = NNN + chip.V[0];
        break;

    case 0xC000: // 0xCXNN: V[X] = (random number) & mask
        chip.V[X] = (rand() % 0xFF) & NN;
        break;

    case 0xD000: // 0xDXYN: Draw sprite at position V[X], V[Y], using N (height) bytes of sprite data starting at address in I.  V[F] = 1 if any pixels changed to unset, else 0.
        chip.V[0xF] = 0;
        for (int yline = 0; yline < N; yline++) {
            byte pixel_row = chip.memory[chip.I + yline];
            for (int xline = 0; xline < 8; xline++) {
                byte pixel = (pixel_row >> (7 - xline)) & 0x01;
                byte color = get_pixel(chip.V[X] + xline, chip.V[Y] + yline) ? 0 : 1;
                if (color & pixel) {
                    chip.V[0xF] = 1;
                }
                set_pixel(chip.V[X] + xline, chip.V[Y] + yline, (color ^ pixel) ? BLACK : WHITE);
            }
        }

        chip.drawflag = true;
        break;

    case 0xE000: // 0xEXNN
        switch (NN) {
        case 0x9E: // 0xEX9E: Skip next if key in V[X] is pressed
            check_keys();
            chip.pc += (chip.key[chip.V[X]]) ? 2 : 0;
            break;

        case 0xA1: // 0xEXA1: Skip next if key in V[X] is not pressed
            check_keys();
            chip.pc += (!chip.key[chip.V[X]]) ? 2 : 0;
            break;

        default:
            fprintf(stderr, "Unknown opcode [0xE000]: 0x%4X\n", opcode);
            exit(-1);
        }
        break;

    case 0xF000: // 0xFXNN
        switch (NN) {
        case 0x07: // 0xFX07: Store current value of delay timer in V[X]
            chip.V[X] = chip.delay_timer;
            break;

        case 0x0A: // 0xFX0A: Wait for keypress and store result in V[X]
            while (true) {
                check_keys();
                for (int i = 0; i < 16; i++) {
                    if (chip.key[i]) {
                        chip.V[X] = i;
                        return;
                    }
                }
            }

            break;

        case 0x0015: // 0xFX15: Set delay timer to V[X]
            chip.delay_timer = chip.V[X];
            break;

        case 0x0018: // 0xFX18: Set sound timer to V[X]
            chip.sound_timer = chip.V[X];
            break;

        case 0x001E: // 0xFX1E: I += V[X]
            chip.V[0xF] = (chip.I + chip.V[X] > 0xFFF) ? 1 : 0; // Undocumented carry flag
            chip.I += chip.V[X];
            chip.I %= 0x1000;
            break;

        case 0x0029: // 0xFX29: I = Address of hex character in char map indicated by V[X]
            chip.I = 0x5 * chip.V[X];
            break;

        case 0x0033: // 0xFX33: Store BCD of V[X] at I, I+1, and I+2
            chip.memory[chip.I]     = chip.V[X] / 100;
            chip.memory[chip.I + 1] = (chip.V[X] / 10) % 10;
            chip.memory[chip.I + 2] = (chip.V[X] % 100) % 10;
            break;

        case 0x0055: // 0xFX55: Store V[0 - X] starting at I, I = I + X + 1 when done
            for(int i = 0; i < X; i++) {
                chip.memory[chip.I + i] = chip.V[i];
            }

            chip.I += X + 1;
            break;

        case 0x0065: // 0xFX65: Fill V[0 - X] with values starting at I, I = I + X + 1 when done
            for (int i = 0; i < X; i++) {
                chip.V[i] = chip.memory[chip.I + i];
            }

            chip.I += X + 1;
            break;

        default:
            fprintf(stderr, "Unknown opcode [0xF000]: 0x%4X\n", opcode);
            exit(-1);
        }
        break;

    default:
        fprintf(stderr, "Unknown opcode 0x%04X\n", opcode);
        exit(-1);
	}
}

void check_keys() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: run = false; break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_1: chip.key[0x0] = 1; break;
            case SDLK_2: chip.key[0x1] = 1; break;
            case SDLK_3: chip.key[0x2] = 1; break;
            case SDLK_4: chip.key[0xc] = 1; break;
            case SDLK_q: chip.key[0x4] = 1; break;
            case SDLK_w: chip.key[0x5] = 1; break;
            case SDLK_e: chip.key[0x6] = 1; break;
            case SDLK_r: chip.key[0xd] = 1; break;
            case SDLK_a: chip.key[0x7] = 1; break;
            case SDLK_s: chip.key[0x8] = 1; break;
            case SDLK_d: chip.key[0x9] = 1; break;
            case SDLK_f: chip.key[0xe] = 1; break;
            case SDLK_z: chip.key[0xa] = 1; break;
            case SDLK_x: chip.key[0xb] = 1; break;
            case SDLK_c: chip.key[0xc] = 1; break;
            case SDLK_v: chip.key[0xf] = 1; break;
            default: break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_1: chip.key[0x0] = 0; break;
            case SDLK_2: chip.key[0x1] = 0; break;
            case SDLK_3: chip.key[0x2] = 0; break;
            case SDLK_4: chip.key[0xc] = 0; break;
            case SDLK_q: chip.key[0x4] = 0; break;
            case SDLK_w: chip.key[0x5] = 0; break;
            case SDLK_e: chip.key[0x6] = 0; break;
            case SDLK_r: chip.key[0xd] = 0; break;
            case SDLK_a: chip.key[0x7] = 0; break;
            case SDLK_s: chip.key[0x8] = 0; break;
            case SDLK_d: chip.key[0x9] = 0; break;
            case SDLK_f: chip.key[0xe] = 0; break;
            case SDLK_z: chip.key[0xa] = 0; break;
            case SDLK_x: chip.key[0xb] = 0; break;
            case SDLK_c: chip.key[0xc] = 0; break;
            case SDLK_v: chip.key[0xf] = 0; break;
            default: break;
            }
            break;
        }
    }
}

byte chip8_fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Thanks PierrotLL!
/* int isKeyPressed(int basic_keycode) { */
/* 	const unsigned short* keyboard_register = (unsigned short*)0xA44B0000; */
/* 	int row, col, word, bit; */
/* 	row = basic_keycode%10; */
/* 	col = basic_keycode/10-1; */
/* 	word = row>>1; */
/* 	bit = col + 8*(row&1); */
/* 	return (0 != (keyboard_register[word] & 1<<bit)); */
/* } */
