#include <stdio.h>

#include <3ds.h>
#include "nesGlobal.h"
#include "nesSystem.h"
#include "nesLoadROM.h"
#include "6502core.h"
#include "nesPPU.h"
#include "nesMemory.h"

#include "FileSystem.h"



extern FS_archive sdmcArchive;

/* It will init all services necessary to Homebrew work ! */
void INIT_3DS() {
	srvInit();	
	fsInit();
	aptInit();
	gfxInit();
	hidInit(NULL);
	aptSetupEventHandler();

	ROM_Cache 			= NULL;
	PPU_Memory 		    = linearAlloc(16384);
	SPRITE_Memory  	    = linearAlloc(256);
	frameSkip			= 0;



	PAD1_Data			= 0;
	ROM_Size			= 0;

	CPU_Running			= false;
	PAUSE_Emulation		= false;
	ENABLE_Background	= true;
	ENABLE_Sprite		= true;
	inGame				= false;

}

/* It will init and load the ROM List */
void INIT_FileSystem() {
	/* INIT SDMC ARCHIVE */
	sdmcArchive = (FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);

	NES_LOADROMLIST();
}


/* Finish all services which was used ! */
void EXIT_3DS() {

	/* Free All Linear Allocation */
	linearFree (ROM_Cache);
	linearFree (PPU_Memory);
	linearFree (SPRITE_Memory);
	linearFree (SRAM_Name);

	fsExit();
	hidExit();
	gfxExit();
	aptExit();
	srvExit();
	svcExitProcess();
}

void INIT_EMULATION() {
	if (NES_LoadROM() == -1) {
		/* An error ocurred and the game can't start */
	}


	if (MAPPER == 4) 
		mmc3_reset();

	if (SRAM == 1) 
		SRAM_LOADSTATE();


	CPU_reset();
	RESET_INPUT();
	init_ppu();
	do_mirror(MIRRORING);

	CPU_Running = true;

}


/* Check if a button is pressed */
void NES_CheckJoypad() {
    u32 keys = ((u32*)0x10000000)[7];

    if(keys & BUTTON_L1) {  
        inGame = 1;
    }

    if(inGame == 1){

        if(keys & BUTTON_L1) {
            inGame = 0;
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
					NES_drawROMLIST();
					NES_drawConfigurationMenu();
				} else {

					if (!CPU_Running)
						INIT_EMULATION();

					/* Check FrameSkip */

					if (skipFrame > frameSkip) 
						skipFrame = 0;


					for (scanline = 0; scanline < 262; scanline++) {
						if (MAPPER == 5) mmc5_hblank(scanline);

						CPU_execute(line_ticks);

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


					skipFrame++;

					NES_CheckJoypad();

				}
			break;

			case APP_SUSPENDING:
				aptReturnToMenu();
			break;

			case APP_SLEEPMODE:
				aptWaitStatusEvent();
			break;

		}
	}

}




int main() {
	

	INIT_3DS();
	INIT_FileSystem();

	NES_MAINLOOP();

	EXIT_3DS();
	return 0;
}
