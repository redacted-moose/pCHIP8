/*
 * chip8.c
 *
 *  Created on: May 11, 2012
 *      Author: Mighty Moose
 */

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <display_syscalls.h>
#include <keyboard_syscalls.h>
#include "BFILE_syscalls.h"
#include "chip8.h"
#include "cpu.h"

#define KEY_PRIZM_7 74
#define KEY_PRIZM_8 64
#define KEY_PRIZM_9 54
#define KEY_PRIZM_DEL 44
#define KEY_PRIZM_4 73
#define KEY_PRIZM_5 63
#define KEY_PRIZM_6 53
#define KEY_PRIZM_MULTIPLY 43
#define KEY_PRIZM_1 72
#define KEY_PRIZM_2 62
#define KEY_PRIZM_3  52
#define KEY_PRIZM_PLUS 42
#define KEY_PRIZM_0 71
#define KEY_PRIZM_PERIOD 61
#define KEY_PRIZM_EXP 51
#define KEY_PRIZM_NEGATIVE 41

chip8* initializeCHIP8() {
	int size;

	//chip8 *ctx = (chip8 *)calloc(1, sizeof(chip8));
	chip8 *ctx = (chip8 *)malloc(sizeof(chip8));
	ctx->pc = 0x200;
	ctx->opcode = 0;
	ctx->I = 0;
	ctx->sp = 0;

	//initialize all memory
	/*ctx->memory = (byte *)calloc(4096, sizeof(byte));
	ctx->V = (byte *)calloc(16, sizeof(byte));
	ctx->LCD = (byte *)calloc(64*32, sizeof(byte));
	ctx->stack = (word *)calloc(16, sizeof(word));
	ctx->key = (byte *)calloc(16, sizeof(byte));*/

	ctx->memory = (byte *)malloc(4096*sizeof(byte));
	memset(ctx->memory, 0L, sizeof(ctx->memory));
	ctx->V = (byte *)malloc(16*sizeof(byte));
	memset(ctx->V, 0L, sizeof(ctx->V));
	ctx->LCD = (byte *)malloc(64*32*sizeof(byte));
	memset(ctx->LCD, 0L, sizeof(ctx->LCD));
	ctx->stack = (word *)malloc(16*sizeof(word));
	memset(ctx->stack, 0L, sizeof(ctx->stack));
	ctx->key = (byte *)malloc(16*sizeof(byte));
	memset(ctx->key, 0L, sizeof(ctx->key));

	//load fontset - shouldn't this be at 0x50?
	for(int i = 0; i < 80; ++i)
		ctx->memory[i] = chip8_fontset[i];

	//reset timers
	ctx->delay_timer = 0;
	ctx->sound_timer = 0;

	//load game
	/*fseek(rom, 0L, SEEK_END);
	size = ftell(rom);
	fseek(rom, 0L, SEEK_SET);*/
	size = Bfile_GetFileSize_OS(rom, 0);
	//for(int i = 0; i < size; i++)
		//ctx->memory[i + 512] = fgetc(rom);
	Bfile_ReadFile_OS(rom, ctx->memory + 512, size, 0);

	return ctx;
}

void emulateCycle(chip8 *ctx) {
	ctx->opcode = ctx->memory[ctx->pc] << 8 | ctx->memory[ctx->pc + 1];

	//printf("Opcode = %X", ctx->opcode);

	switch (ctx->opcode & 0xF000) {
		case 0x0000:
			switch (ctx->opcode & 0x000F) {
				case 0x0000: // 0x00E0: Clears the screen
					//printf("0x00E0: Clear screen\n");
					memset(ctx->LCD, 0, 32*64);
					ctx->drawflag = true;
					ctx->pc += 2;
				break;

				case 0x000E: // 0x00EE: Returns from a subroutine
					//printf("0x00EE: Return from subroutine\n");
					--(ctx->sp);
					ctx->pc = ctx->stack[ctx->sp];
					ctx->pc += 2;
				break;

				default:
					//printf("Unknown opcode [0x0000]: 0x%X\n", ctx->opcode);
				break;
			}
		break;

		case 0x1000:
			//printf("Jump to 0x%03X", ctx->opcode & 0x0FFF);
			ctx->pc = ctx->opcode & 0x0FFF;
		break;

		case 0x2000:
			//printf("Call to 0x%03X", ctx->opcode & 0x0FFF);
			ctx->stack[ctx->sp] = ctx->pc;
			++(ctx->sp);
			ctx->pc = (ctx->opcode) & 0x0FFF;
		break;

		case 0x3000:
			//printf("Skip next if ");
			if (ctx->V[(ctx->opcode & 0x0F00) >> 8] == (ctx->opcode & 0x00FF))
				ctx->pc += 2;
			ctx->pc += 2;
		break;

		case 0x4000:
			if (ctx->V[(ctx->opcode & 0x0F00) >> 8] != (ctx->opcode & 0x00FF))
				ctx->pc += 2;
			ctx->pc += 2;
		break;

		case 0x5000:
			if (ctx->V[(ctx->opcode & 0x0F00) >> 8] == ctx->V[(ctx->opcode & 0x00F0) >> 4])
				ctx->pc += 2;
			ctx->pc += 2;
		break;

		case 0x6000:
			ctx->V[(ctx->opcode & 0x0F00) >> 8] = (ctx->opcode & 0x00FF);
			ctx->pc += 2;
		break;

		case 0x7000:
			ctx->V[(ctx->opcode & 0x0F00) >> 8] += (ctx->opcode & 0x00FF);
			ctx->pc += 2;
		break;

		case 0x8000:
			switch (ctx->opcode & 0x000F) {
				case 0x0000:
					ctx->V[(ctx->opcode & 0x0F00) >> 8] = ctx->V[(ctx->opcode & 0x00F0) >> 4];
					ctx->pc += 2;
				break;

				case 0x0001:
					ctx->V[(ctx->opcode & 0x0F00) >> 8] |= ctx->V[(ctx->opcode & 0x00F0) >> 4];
					ctx->pc += 2;
				break;

				case 0x0002:
					ctx->V[(ctx->opcode & 0x0F00) >> 8] &= ctx->V[(ctx->opcode & 0x00F0) >> 4];
					ctx->pc += 2;
				break;

				case 0x0003:
					ctx->V[(ctx->opcode & 0x0F00) >> 8] ^= ctx->V[(ctx->opcode & 0x00F0) >> 4];
					ctx->pc += 2;
				break;

				case 0x0004:
					if (ctx->V[(ctx->opcode & 0x00F0) >> 4] > (0xFF - ctx->V[(ctx->opcode & 0x0F00) >> 8]))
						ctx->V[0xF] = 1; // Set as carry
					else
						ctx->V[0xF] = 0;
					ctx->V[(ctx->opcode & 0x00F0) >> 4] += ctx->V[(ctx->opcode & 0x0F00) >> 8];
					ctx->pc += 2;
				break;

				case 0x0005:
					if (ctx->V[(ctx->opcode & 0x0F00) >> 8] > ctx->V[(ctx->opcode & 0x00F0) >> 4])
						ctx->V[0xF] = 1; // Should be opposite?
					else
						ctx->V[0xF] = 0;
					ctx->V[(ctx->opcode & 0x0F00) >> 8] -= ctx->V[(ctx->opcode & 0x00F0) >> 4];
					ctx->pc += 2;
				break;

				case 0x0006:
					ctx->V[0xF] = ctx->V[(ctx->opcode & 0x0F00) >> 8] & 0x01;
					ctx->V[(ctx->opcode & 0x0F00) >> 8] >>= 1;
					ctx->pc += 2;
				break;

				case 0x0007:
					if (ctx->V[(ctx->opcode & 0x00F0) >> 4] > ctx->V[(ctx->opcode & 0x0F00) >> 8])
						ctx->V[0xF] = 1; // Should be opposite?
					else
						ctx->V[0xF] = 0;
					ctx->V[(ctx->opcode & 0x00F0) >> 4] = ctx->V[(ctx->opcode & 0x0F00) >> 8] - ctx->V[(ctx->opcode & 0x00F0) >> 4];
					ctx->pc += 2;
				break;

				case 0x000E:
					ctx->V[0xF] = ctx->V[(ctx->opcode & 0x0F00) >> 8] >> 7;
					ctx->V[(ctx->opcode & 0x0F00) >> 8] <<= 1;
					ctx->pc += 2;
				break;

				default:
					//printf("Unknown opcode [0x8000]: 0x%X\n", ctx->opcode);
				break;
			}
		break;

		case 0x9000:
			if (ctx->V[(ctx->opcode & 0x0F00) >> 8] != ctx->V[(ctx->opcode & 0x00F0) >> 4])
				ctx->pc += 2;
			ctx->pc += 2;
		break;

		case 0xA000:
			ctx->I = ctx->opcode & 0x0FFF;
			ctx->pc += 2;
		break;

		case 0xB000:
			ctx->pc = (ctx->opcode & 0x0FFF) + ctx->V[0];
			ctx->pc += 2;
		break;

		case 0xC000:
			ctx->V[(ctx->opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (ctx->opcode & 0x00FF);
			ctx->pc += 2;
		break;

		case 0xD000:
			x = ctx->V[(ctx->opcode & 0x0F00) >> 8];
			y = ctx->V[(ctx->opcode & 0x00F0) >> 4];
			height = (ctx->opcode) & 0x000F;

			ctx->V[0xF] = 0;
			for (int yline = 0; yline < height; yline++) {
				pixel = ctx->memory[ctx->I + yline];
				for (int xline = 0; xline < 8; xline++) {
					if (pixel & (0x80 >> xline)) {
						if (ctx->LCD[(x + xline +((y + yline) * 64))] == 1)
							ctx->V[0xF] = 1;
						ctx->LCD[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			ctx->drawflag = true;
			ctx->pc += 2;
		break;

		case 0xE000:
			switch (ctx->opcode & 0x00FF) {
				case 0x009E:
					if (ctx->key[ctx->V[(ctx->opcode & 0x0F00) >> 8]] != 0)
						ctx->pc += 2;
					ctx->pc +=2;
				break;

				case 0x00A1:
					if (ctx->key[ctx->V[(ctx->opcode & 0x0F00) >> 8]] == 0)
						ctx->pc += 2;
					ctx->pc += 2;
				break;

				default:
					//printf("Unknown opcode [0xE000]: 0x%X\n", ctx->opcode);
				break;
			}
		break;

		case 0xF000:
			switch (ctx->opcode & 0x00FF) {
				case 0x0007:
					ctx->V[(ctx->opcode & 0x0F00) >> 8] = ctx->delay_timer;
					ctx->pc += 2;
				break;

				case 0x000A:
					keyPress = false;

					for (int i = 0; i < 16; ++i) {
						if (ctx->key[i]) {
							ctx->V[(ctx->opcode & 0x0F00) >> 8] = i;
							keyPress = true;
						}
					}

					if (!keyPress)
						return;

					ctx->pc += 2;
				break;

				case 0x0015:
					ctx->delay_timer = ctx->V[(ctx->opcode & 0x0F00) >> 8];
					ctx->pc += 2;
				break;

				case 0x0018:
					ctx->sound_timer = ctx->V[(ctx->opcode & 0x0F00) >> 8];
					ctx->pc += 2;
				break;

				case 0x001E:
					if (ctx->I + ctx->V[(ctx->opcode & 0x0F00) >> 8] > 0xFFF) // Undocumented carry flag feature
						ctx->V[0xF] = 1; // Set carry flag
					else
						ctx->V[0xF] = 0; // Reset carry flag
					ctx->I += ctx->V[(ctx->opcode & 0x0F00) >> 8];
					ctx->pc += 2;
				break;

				case 0x0029:
					ctx->I = 0x5 * ctx->V[(ctx->opcode & 0x0F00) >> 8];
					ctx->pc += 2;
				break;

				case 0x0033:
					ctx->memory[ctx->I]     = ctx->V[(ctx->opcode & 0x0F00) >> 8] / 100;
					ctx->memory[ctx->I + 1] = (ctx->V[(ctx->opcode & 0x0F00) >> 8] / 10) % 10;
					ctx->memory[ctx->I + 2] = (ctx->V[(ctx->opcode & 0x0F00) >> 8] % 100) % 10;
					ctx->pc += 2;
				break;

				case 0x0055:
					for(int i = 0; i < ((ctx->opcode & 0x0F00) >> 8); ++i)
						ctx->memory[ctx->I + i] = ctx->V[i];

					ctx->I += ((ctx->opcode & 0x0F00) >> 8) + 1;
					ctx->pc += 2;
				break;

				case 0x0065:
					for (int i = 0; i < ((ctx->opcode & 0x0F00) >> 8); ++i)
						ctx->V[i] = ctx->memory[ctx->I + i];

					ctx->I += ((ctx->opcode & 0x0F00) >> 8) + 1;
					ctx->pc += 2;
				break;

				default:
					//printf("Unknown opcode [0xF000]: 0x%X\n", ctx->opcode);
				break;
			}
		break;

		default:
			//printf("Unknown opcode 0x%04X\n", ctx->opcode);
		break;
	}

	if (ctx->delay_timer > 0) //Optimization: if (delay_timer)
		--(ctx->delay_timer);

	if (ctx->sound_timer > 0) {
		if (ctx->sound_timer == 1)
			printf("BEEP!"); //To be implemented later
		--(ctx->sound_timer);
	}
}

bool anyKeyPressed(chip8 *ctx) {
	setKeys(ctx);
	for (int i = 0; i < 16; i++) {
		if (ctx->key[i])
			return ctx->key[i];
	}
	return 0;
}

void setKeys(chip8 *ctx) {
	/*if (isKeyPressed(KEY_NSPIRE_7))
		ctx->key[0x1] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_8))
		ctx->key[0x2] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_9))
		ctx->key[0x3] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_4))
		ctx->key[0x4] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_5))
		ctx->key[0x5] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_6))
		ctx->key[0x6] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_1))
		ctx->key[0x7] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_2))
		ctx->key[0x8] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_3))
		ctx->key[0x9] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_0))
		ctx->key[0xA] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_PERIOD))
		ctx->key[0] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_NEGATIVE))
		ctx->key[0xB] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_MULTIPLY))
		ctx->key[0xC] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_MINUS))
		ctx->key[0xD] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_PLUS))
		ctx->key[0xE] = TRUE;
	if (isKeyPressed(KEY_NSPIRE_ENTER))
		ctx->key[0xF] = TRUE;*/

	ctx->key[0x1] = (isKeyPressed(KEY_PRIZM_7))? true: false;
	ctx->key[0x2] = (isKeyPressed(KEY_PRIZM_8))? true: false;
	ctx->key[0x3] = (isKeyPressed(KEY_PRIZM_9))? true: false;
	ctx->key[0x4] = (isKeyPressed(KEY_PRIZM_4))? true: false;
	ctx->key[0x5] = (isKeyPressed(KEY_PRIZM_5))? true: false;
	ctx->key[0x6] = (isKeyPressed(KEY_PRIZM_6))? true: false;
	ctx->key[0x7] = (isKeyPressed(KEY_PRIZM_1))? true: false;
	ctx->key[0x8] = (isKeyPressed(KEY_PRIZM_2))? true: false;
	ctx->key[0x9] = (isKeyPressed(KEY_PRIZM_3))? true: false;
	ctx->key[0xA] = (isKeyPressed(KEY_PRIZM_0))? true: false;
	ctx->key[0] = (isKeyPressed(KEY_PRIZM_PERIOD))? true: false;
	ctx->key[0xB] = (isKeyPressed(KEY_PRIZM_EXP))? true: false;
	ctx->key[0xC] = (isKeyPressed(KEY_PRIZM_DEL))? true: false;
	ctx->key[0xD] = (isKeyPressed(KEY_PRIZM_MULTIPLY))? true: false;
	ctx->key[0xE] = (isKeyPressed(KEY_PRIZM_PLUS))? true: false;
	ctx->key[0xF] = (isKeyPressed(KEY_PRIZM_NEGATIVE))? true: false;
}

void destroy(chip8 *ctx) {
	free(ctx->memory);
	free(ctx->LCD);
	free(ctx->V);
	free(ctx->key);
	free(ctx->stack);
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
int isKeyPressed(int basic_keycode) {
	const unsigned short* keyboard_register = (unsigned short*)0xA44B0000;
	int row, col, word, bit;
	row = basic_keycode%10;
	col = basic_keycode/10-1;
	word = row>>1;
	bit = col + 8*(row&1);
	return (0 != (keyboard_register[word] & 1<<bit));
}
