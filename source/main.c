#include <stdio.h>
#include <3ds.h>

// NES SYSTEM
#include "nesGlobal.h"
#include "nesSystem.h"
#include "nesLoadROM.h"
#include "nes6502.h"
#include "nesPPU.h"
// NES MAPPERS
#include "utils.h"
#include "mmc1.h"	// 1
#include "unrom.h"	// 2
#include "cnrom.h"	// 3
#include "mmc3.h"	// 4
#include "mmc5.h"	// 5
#include "aorom.h"	// 7
#include "mapper79.h"	// 79
// FILESYSTEM, BACKGROUND, ETC
#include "FileSystem.h"
#include "background.h"

u8 	*PPU_Memory;
u8 	*SPRITE_Memory;
u8	frameSkip;
u8	skipFrame;

u16  lastInstruction = 0;
u8   lastOP = 0;
u8   lastA = 0;

u32 PAD1_Data;

bool CPU_Running;
bool PAUSE_Emulation;
bool ENABLE_Background;
bool ENABLE_Sprite;
bool inGame;
bool VSYNC;

int PAD1_ReadCount = 0;

extern u8 memory[65536];

/* It will init all services necessary to Homebrew work ! */
void INIT_3DS() {
	srvInit();	
	fsInit();
	aptInit();
	gfxInit();
	hidInit(NULL);
	gfxSet3D(false);

	PPU_Memory 		    = linearAlloc(16384);
	SPRITE_Memory  	    = linearAlloc(256);
	frameSkip			= 2;
	skipFrame			= 0;



	PAD1_Data			= 0;
	ROM_Size			= 0;

	CPU_Running			= false;
	PAUSE_Emulation		= false;
	ENABLE_Background	= true;
	ENABLE_Sprite		= true;
	inGame				= false;
	VSYNC				= true;


	init_ppu();
}

/* It will init and load the ROM List */
void INIT_FileSystem() {
	fileSystem.inMenu = false;
	NES_LOADROMLIST();
}


/* Finish all services which was used ! */
void EXIT_3DS() {

	/* Free All Linear Allocation */
	if (SRAM_Name != NULL)
        linearFree (SRAM_Name);
	
	if (fileSystem.fileList != NULL)
        linearFree (fileSystem.fileList);

	linearFree (memory);
	linearFree (PPU_Memory);
	linearFree (SPRITE_Memory);

	fsExit();
	hidExit();
	gfxExit();
	aptExit();
	srvExit();
	svcExitProcess();
}

void INIT_EMULATION() {

	if (NES_LoadROM() == -1) {
		
		inGame = false;
	}

	if (MAPPER == 4) 
		mmc3_reset();

	if (SRAM == 1) 
		SRAM_LOADSTATE();

	CPU_Reset();
	RESET_INPUT();
	do_mirror(MIRRORING);

	CPU_Running = true;
}


/* Check if a button is pressed */
void NES_CheckJoypad() {
    u32 keys = ((u32*)0x10000000)[7];

    if (!inGame)
    	inGame = true;

    if(inGame == true){

        if(keys & BUTTON_L1) {
            inGame = false;
        }
    
        if(keys & BUTTON_A)
            SET_INPUT(7);
        else
            CLEAR_INPUT(7);

        if(keys & BUTTON_B)
            SET_INPUT(8);
        else
            CLEAR_INPUT(8);

        if(keys & BUTTON_START)
            SET_INPUT(5);
        else
            CLEAR_INPUT(5);

        if(keys & BUTTON_RIGHT)
            SET_INPUT(4);
        else
            CLEAR_INPUT(4);

        if(keys & BUTTON_LEFT)
            SET_INPUT(3);
        else
            CLEAR_INPUT(3);

        if(keys & BUTTON_UP)
            SET_INPUT(2);
        else
            CLEAR_INPUT(2);

        if(keys & BUTTON_DOWN)
            SET_INPUT(1);
        else
            CLEAR_INPUT(1);

        if(keys & BUTTON_SELECT)
            SET_INPUT(6);
        else
            CLEAR_INPUT(6);
    }
}

void NES_MAINLOOP() {
	APP_STATUS status;

	int scanline = 0;

	while ((status = aptGetStatus()) != APP_EXITING ) {
		
		switch (status) {
			case APP_RUNNING:

				if (!inGame) {
					drawMenu();
					NES_MainMenu();
					drawBuffers();
					gspWaitForVBlank();
				} else {
					ppu_status = 0;
					if (!CPU_Running)
						INIT_EMULATION();

					/* Check FrameSkip */

					if (skipFrame > frameSkip) 
						skipFrame = 0;

					if (skipFrame == 0)
						NES_ColorBackground();

					for (scanline = 0; scanline < 262; scanline++) {
						if (MAPPER == 5) mmc5_hblank(scanline);

						CPU_Execute();

						if (scanline < 240) {
							if (MAPPER == 4) mmc3_hblank(scanline);

							render_scanline(scanline);
						
						} else {
							if (scanline == 241) {
								if (exec_nmi_on_vblank) { NMI(); }

								ppu_status = 0x80;
							}
						}
					}
					
					NES_CheckJoypad();
					
					if (skipFrame == 0)
						drawBuffers();
					
					skipFrame++;

					if (VSYNC)		
						gspWaitForVBlank();

				}
			break;

			case APP_SUSPENDING:
				aptReturnToMenu();
			break;

			case APP_SLEEPMODE:
				aptWaitStatusEvent();
			break;

			default:

			break;
		}

	}

}

// TODO 
// Write memoryRead and memoryWrite in Assembly
// i belive which it will improve the speed
u8 	memoryRead(u32 addr) {
	/* this is ram or rom so we can return the addr */
	if (addr < 0x2000)
		return memory[addr & 0x7FF];

	if (addr > 0x7FFF)
		return memory[addr];


	/* The addr between 0x200 and 0x500 are for input and ouput */

	if (addr == 0x2002) {
		ppu_status_tmp = ppu_status;

		/* set ppu_status (D7) to 0 (vblank on) */
		ppu_status &= 0x7F;

		/* set ppu_status (D6) to 0 (sprite zero) */
		ppu_status &= 0x1F;

		/* Reset VRAM addr Register #1 */
		ppu_bgscr_f = 0x0;

		/* reset VRAM addr Register #2 */
		ppu_addr_h =  0x0;

		/* return bits 7-4 of unmodifyed ppu_status with bits 3-0 of the ppu_addr_tmp */
		return (ppu_status_tmp);
	}

	if(addr == 0x2007) {
        int tmp = ppu_addr_tmp;
		ppu_addr_tmp = ppu_addr;

		if(increment_32 == 0) {
			ppu_addr++;
		} else {
			ppu_addr += 0x20;
		}

		return PPU_Memory[tmp];
	}

	/* pAPU data (sound) */
	if(addr == 0x4015) {
		return memory[addr];
	}

	/* joypad1 data */
	if(addr == 0x4016) {
		switch(PAD1_ReadCount) {
			case 0:
			memory[addr] = PAD1_A;
			PAD1_ReadCount++;
			break;

			case 1:
			memory[addr] = PAD1_B;
			PAD1_ReadCount++;
			break;

			case 2:
			memory[addr] = PAD1_SELECT;
			PAD1_ReadCount++;
			break;

			case 3:
			memory[addr] = PAD1_START;
			PAD1_ReadCount++;
			break;

			case 4:
			memory[addr] = PAD1_UP;
			PAD1_ReadCount++;
			break;

			case 5:
			memory[addr] = PAD1_DOWN;
			PAD1_ReadCount++;
			break;

			case 6:
			memory[addr] = PAD1_LEFT;
			PAD1_ReadCount++;
			break;

			case 7:
			memory[addr] = PAD1_RIGHT;
			PAD1_ReadCount = 0;
			break;
		}

		return memory[addr];
	}

	if(addr == 0x4017) {
		return memory[addr];
	}

	if (MAPPER == 5) {
        if((addr == 0x5204) || (addr >= 0x5C00 && addr <= 0x5FFF)) {
            return mmc5_read(addr);
        }
    }

	if((addr > 0x5FFF) && (addr < 0x8000) && (MAPPER == 5)) { //SRAM
        return mmc5_wram[(addr - 0x6000) + (mmc5_wram_page * 8192) + (mmc5_wram_chip * 8192)];
	}


	return memory[addr];
}

/* Read 2 bytes */
int  memoryRead16(u32 addr) {
	return (memoryRead(addr) + (memoryRead(addr + 1) * 0x100));
}

/* write a byte */
void writeMemory(u32 addr, u8 data) {
	/* PPU Video Memory area */
	if(addr > 0x1fff && addr < 0x4000) {
		write_PPU_Memory(addr,data);
		return;
	}

	/* Sprite DMA Register */
	if(addr == 0x4014) {
		write_PPU_Memory(addr,data);
		// TODO: Fix Sprite DMA Tick
		//nesTick += 512;
		return;
	}

	/* Joypad 1 */
	if(addr == 0x4016) {
		memory[addr] = 0x40;

		return;
	}

	/* Joypad 2 */
	if(addr == 0x4017) {
		memory[addr] = 0x48;
		return;
	}

	/* pAPU Sound Registers */
	if(addr > 0x3fff && addr < 0x4016) {
		memory[addr] = data;
		return;
	}
	
	/* SRAM Registers */
	if(addr > 0x5fff && addr < 0x8000) {
        if(MAPPER == 5) { //SRAM
            mmc5_wram[(addr - 0x6000) + (mmc5_wram_page * 8192) + (mmc5_wram_chip * 8192)] = data;
            return;
        } else {

            memory[addr] = data;
            return;
        }
	}

	/* RAM registers */
	if(addr < 0x2000) {memory[addr & 0x7FF] = data; return;}

	switch (MAPPER) {
		case 1:
			mmc1_access(addr, data);
			return;
		break;

		case 2:
			unrom_access(addr, data);
			return;
		break;

		case 3:
			cnrom_access(addr, data);
			return;
		break;

		case 4:
			mmc3_access(addr, data);
			return;
		break;

		case 5:
			mmc5_access(addr, data);
			return;
		break;

		case 7:
			aorom_access(addr, data);
			return;
		break;

		case 79:
			mapper79_access(addr,data);
			return;
		break;

		default:

		break;
	}

	memory[addr] = data;
}

int main() {
	
	INIT_3DS();
	INIT_FileSystem();

	NES_MAINLOOP();

	EXIT_3DS();
	return 0;
}
