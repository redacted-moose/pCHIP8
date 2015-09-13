/*
 * cpu.c
 *
 *  Created on: May 11, 2012
 *      Author: Mighty Moose
 */

//#include "os.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <display_syscalls.h>
#include <keyboard_syscalls.h>
#include "BFILE_syscalls.h"
#include "chip8.h"
#include "cpu.h"

#define KEY_PRIZM_MENU 48

/*inline void setPixel(int x, int y, int color)
{
  unsigned char* p = (unsigned char*)(SCREEN_BASE_ADDRESS + ((x >> 1) + (y << 7) + (y << 5)));
  *p = (x & 1) ? ((*p & 0xF0) | color) : ((*p & 0x0F) | (color << 4));
}*/

/*inline int getPixel(int x, int y)
{
  unsigned char* p = (unsigned char*)(SCREEN_BASE_ADDRESS + ((x >> 1) + (y << 7) + (y << 5)));
  return ((x & 1) ? (*p & 0x0F) : (*p >> 4));
}*/

int main(int argc, char **argv) {

	unsigned char filename[] = {'\\','\\','f','l','s','0','\\','r','o','m','.','c','8'};
	short unsigned int dest[26];

	Bdisp_AllClr_VRAM();
	Bdisp_PutDisp_DD();

	Bfile_StrToName_ncpy(dest, filename, 13);
	rom = Bfile_OpenFile_OS(dest, 0);

	if (rom < 0) {
        //printf("Failed to open rom image - check if it exists...\n");
        return -1;
	}

	chip = initializeCHIP8();

	Bfile_CloseFile_OS(rom);

	do {
		//printf("Starting loop\n");
		//printf("PC = 0x%X\n", chip->pc);
		//printf("Opcode = 0x%X\n", chip->opcode);
		//printf("SP = %d", chip->sp);

		emulateCycle(chip);

		//printf("Is drawflag set?\n");

		if (chip->drawflag) {
			//printf("true\n");

			for (int row = 0; row < 32; row++) {
				for (int col = 0; col < 64; col++) {
					for (int rowoffs = 0; rowoffs < 6; rowoffs++) {
						for (int coloffs = 0; coloffs < 6; coloffs++) {
							Bdisp_SetPoint_VRAM(6*col + coloffs, 6*row + rowoffs, (0x0F * (chip->LCD[64*row + col])) ^ 0x0F);
						}
					}
					//setPixel(col, row, (0x0F * (chip->LCD[64*row + col])) ^ 0x0F);
					//Bdisp_SetPoint_VRAM(col, row, (0x0F * (chip->LCD[64*row + col])) ^ 0x0F);
				}
			}
			Bdisp_PutDisp_DD();
			chip->drawflag = false;
		}
		//printf("false\nChecking for pressed keys...\n");
		setKeys(chip);
		//printf("Time to start over!\n");
	} while (!isKeyPressed(KEY_PRIZM_MENU));

	destroy(chip);

	//TCT_Local_Control_Interrupts(intmask);

	return 0;
}
