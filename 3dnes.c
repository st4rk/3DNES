

#include <stdio.h>
#include <string.h>

#include "6502core.h"
#include "macros.h"
#include "3dnes.h"
#include "romloader.h"
#include "ppu.h"
#include "input.h"
#include "functions.h"
#include "test_vsh_shbin.h"

/* included mappers */
#include "utils.h"
#include "mmc1.h"	// 1
#include "unrom.h"	// 2
#include "cnrom.h"	// 3
#include "mmc3.h"	// 4
#include "mmc5.h"	// 5
#include "aorom.h"	// 7
#include "mapper79.h"	// 79
/* ctrulib */
#include <3ds.h>

/* PICA200 Shader */
DVLB_s* shader;

char romfn[256];

/* cache the rom in memory to access the data quickly */
u8 romcache[1048576];
u8 ppu_memory[16384];
u8 sprite_memory[256];

/* JoyPad Data */
u32 pad1_data;
int pad1_readcount = 0;

/* Tick per Line */
int line_ticks = 114;

/* 1 = CPU Running, 0 = CPU Paused */
int CPU_is_running = 1;
int pause_emulation = 0;

/* 1 = Start Menu, 0 = In Game */
int inMenu = 1;

/* 1 = Enable Sprite, 0 = Disable Sprite */
int enable_background = 1;
int enable_sprites = 1;

/* Frameskip Emulation */
u8 frameskip = 0;
u8 skipframe = 0;

/* romName */
char rom_name[48];
/* romSize */
long romlen;

extern int show_menu;


/**************************************************************************************/
								/* GPU Commands and Variables */

u32* gpuOut  = (u32*) 0x1F119400;
u32* gpuDOut = (u32*) 0x1F370800;
u32* gpuCmd;
u32* TopScreenTexture;

extern u32* gxCmdBuf;
extern u32* PPU_TopScreen;



/*************************************************************************************/
								/* File System */
FS_archive sdmcArchive;

#define MAX_SIZE 0x106
#define MAX_SCREEN_FILES 10
#define MAX_FILES 80

int sFile = 0; /* Selected File */
int cFile = 0; /* Cursor File */
int currFile = 0; /* Current File */
char tn_files[MAX_FILES][MAX_SIZE]; /* Total files/names on SD */

/* Used to press key just one time ! */
u8 UKEY_UP = 0;
u8 UKEY_DOWN = 0;
u8 UKEY_LEFT = 0;
u8 UKEY_RIGHT = 0;
u8 UKEY_B = 0;

/* Configuration Menu */
int confiMenu = 0;
int configCursor = 0;


void NES_ROMLIST() {
	
	u16 dirName[512]; 
	Handle romHandle;
	FS_dirent dirStruct;
	FS_path dirPath = FS_makePath(PATH_CHAR, "/3DNES/ROMS");

	sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

	FSUSER_OpenDirectory(NULL, &romHandle, sdmcArchive, dirPath);

	/* Get total of files/directory on 3DS SD */
	int cont = 0;
	while(1) {
		u32 dataRead = 0;
		FSDIR_Read(romHandle, &dataRead, 1, &dirStruct);

		if(dataRead == 0)
			break;

		strcpy(tn_files[cont], dirStruct.name);
		cont++;
	}

	FSDIR_Close(romHandle);
}

void NES_DrawFileList() {
	int i = 0;

	draw_string_c(55, "well, not working yet :/ ");

	draw_select_bar(-67, (cFile * 15) + 53);

	for(i = 0; i < MAX_SCREEN_FILES; i++) {
		draw_string_c(55 + (i * 15), tn_files[i + sFile]);
	}

}

void NES_drawConfiguration() {
	draw_select_bar(-67, (configCursor * 15) + 70);
	draw_string_c(50, "Configuration Menu");
	draw_string_c(73, "FrameSkip: ");
	draw_string_c(88, "Background: ");
	draw_string_c(103,"Sprites: ");
	draw_string_c(118, "Exit and Start Game");



}

/* Start Game lol */
void NES_StartGame() {
	u32 bytesRead;
	u64 fileSize = 0;
	Handle fileHandle;
	/* Check SRAM, if a game is opened before, then it will save the game */
	if(SRAM == 1)
		write_sav();

	/* ROM_DIR */
	u8 ROM_DIR[strlen("/3DNES/ROMS/") + strlen(tn_files[currFile]) + 1]; 

	/* rom_name to SRAM savefile */
	strcpy(rom_name, tn_files[currFile]);

	/* ROM_DIR = /3DNES/ROMS/HERE ROM NAME.nes */
	sprintf(ROM_DIR, "/3DNES/ROMS/smb3.nes", tn_files[currFile]);

	FS_archive sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

	/* Open File */
	FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, ROM_DIR), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	
	/* Get ROM Size */
	FSFILE_GetSize(fileHandle, &fileSize);

	romlen = fileSize; /* Set Rom Size */

	/* Load ROM in ROM Cache */
	FSFILE_Read(fileHandle, &bytesRead, 0x0, (u32*)romcache,(u32)fileSize);

	FSFILE_Close(fileHandle);

	/* Start Emulation */
	emu_start();
	inMenu = 0;
}


void NES_ConfigurationMenu() { 
	u32 keys = ((u32*)0x10000000)[7];
	
	/* Configuration Menu */
		if(keys & BUTTON_LEFT) {
			if(!UKEY_LEFT) {
				switch(configCursor) {
					case 0:
						frameskip--;
					break;

					case 1:
						enable_background = 0;
					break;

					case 2:
						enable_sprites = 0;
					break;

					default:

					break;
				}

				UKEY_LEFT = 1;
			}
		} else {
			UKEY_LEFT = 0;
		}
	
		if(keys & BUTTON_RIGHT) {
			if(!UKEY_RIGHT) {


				switch(configCursor) {
					case 0:
						frameskip++;
					break;

					case 1:
						enable_background = 1;
					break;

					case 2:
						enable_sprites = 1;
					break;

					default:

					break;
				}


				UKEY_RIGHT = 1;
			}
		} else {
			UKEY_RIGHT = 0;
		}

		if(keys & BUTTON_UP){
			if(!UKEY_UP){
				if(configCursor > 0)
					configCursor--;
				
				UKEY_UP = 1;
			}
		} else {
			UKEY_UP = 0;
		}

		if(keys & BUTTON_DOWN){
			if(!UKEY_DOWN) {

				if(configCursor < 10)
					configCursor++;
				
				UKEY_DOWN = 1;
			}
		} else {
			UKEY_DOWN = 0;
		}


		if(keys & BUTTON_B) {
			if(!UKEY_B) {
				if(configCursor == 3) 
					confiMenu = 0;

				UKEY_B = 1;
			} else {
				UKEY_B = 0;
			}
		}

}

void NES_CurrentFileUpdate() {
	u32 keys = ((u32*)0x10000000)[7];

	if(confiMenu == 0) {
		if(keys & BUTTON_UP){
			if(!UKEY_UP){
				if(sFile > 0) 
					sFile--;
				 else {
					if(cFile > 0)
					   cFile--;
				}

				if(currFile > 0)
					currFile--;
				
				UKEY_UP = 1;
			}
		} else {
			UKEY_UP = 0;
		}

		if(keys & BUTTON_DOWN){
			if(!UKEY_DOWN) {
				if(cFile < MAX_SCREEN_FILES)
					cFile++;
				 else 
					sFile++;
				
				currFile++;
				
				UKEY_DOWN = 1;
			}
		} else {
			UKEY_DOWN = 0;
		}


		if(keys & BUTTON_LEFT) {
			if(!UKEY_LEFT) {
				confiMenu = 1;

				UKEY_LEFT = 1;
			}
		} else {
			UKEY_LEFT = 0;
		}
	

	}


	if(keys & BUTTON_B) {
			if(!UKEY_B) {
				NES_StartGame();

				UKEY_B = 1;
			} else {
				UKEY_B = 0;
			}
		}

}

/*************************************************************************************/
								/* System */

void NES_ScreenShot() {
	u8 FrameBuffer[288000];
	Handle dirHandle;
	Handle ScreenHandle;
	u32 bytesRead;

	//FrameBuffer = getTopFrameBuffer();

	FS_path dirPath=FS_makePath(PATH_CHAR, "/3DNES/");

	sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

	//FSUSER_OpenArchive(NULL, &sdmcArchive);

	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);

	FSUSER_OpenFile(NULL, &ScreenHandle, sdmcArchive, FS_makePath(PATH_CHAR,  "SS.bin"), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
 	
 	FSFILE_Write(ScreenHandle, &bytesRead, 0x0, &FrameBuffer, 288000, 0x10001);

 	FSFILE_Close(ScreenHandle);
}


void NES_Menu() {
	if(inMenu == 1) {

		if(confiMenu == 1) {
			updateMenu();
			NES_drawConfiguration();
			NES_ConfigurationMenu();
			check_joypad();
		} else {
			updateMenu();
			NES_DrawFileList();
			NES_CurrentFileUpdate();
			check_joypad();
		}
		
	}
}


/*************************************************************************************/

/* exitAPP */
void exitAPP() {
	/* Free Allocation */
	end_ppu();
	linearFree (gpuCmd);
	linearFree (TopScreenTexture);

	fsExit();
	hidExit();
	gfxExit();
	aptExit();
	srvExit();
	svcExitProcess();
}

/* SRAM Load */
void open_sav() {
	Handle dirHandle;
	u32 bytesRead;
	
	/* Resultado have the size of Path + GameName */
	u8 resultado[strlen(rom_name) + strlen("/3DNES/SAVES/") + 1]; 
	/* Resultado = /3DNES/ROMS/HERE ROM NAME.nes */
	sprintf( resultado, "/3DNES/SAVES/%s", rom_name);
	FS_path dirPath=FS_makePath(PATH_CHAR, "/3DNES/saves/");

	sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

	//FSUSER_OpenArchive(NULL, &sdmcArchive);

	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
 
	Handle fileHandledump;
	FSUSER_OpenFile(NULL, &fileHandledump, sdmcArchive, FS_makePath(PATH_CHAR,  resultado), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
 	
	/* Load ROM in ROM Cache */
	FSFILE_Read(fileHandledump, &bytesRead, 0x0, (u32*)&memory[0x6000],(u32)8192);

 	FSFILE_Close(fileHandledump);


}

/* SRAM Save */
void write_sav() {
	Handle dirHandle;
	u32 bytesRead;
	
	/* Resultado have the size of Path + GameName */
	u8 resultado[strlen(rom_name) + strlen("/3DNES/SAVES/") + 1]; 
	/* Resultado = /3DNES/ROMS/HERE ROM NAME.nes */
	sprintf( resultado, "/3DNES/SAVES/%s", rom_name);
	FS_path dirPath=FS_makePath(PATH_CHAR, "/3DNES/saves/");

	sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

	//FSUSER_OpenArchive(NULL, &sdmcArchive);

	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
 
	Handle fileHandledump;
	FSUSER_OpenFile(NULL, &fileHandledump, sdmcArchive, FS_makePath(PATH_CHAR,  resultado), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
 	
 	FSFILE_Write(fileHandledump, &bytesRead, 0x0, &memory[0x6000], 8192, 0x10001);

 	FSFILE_Close(fileHandledump);
 	

}

void exit_warning(char* warning_msg) {
	inMenu = 1; /* Back to Menu */

	draw_string_c(100, warning_msg);
}

/* readByte */
unsigned char memory_read(unsigned int address) {
	/* this is ram or rom so we can return the address */
	if(address < 0x2000)
		return memory[address & 0x7FF];

    if(address > 0x7FFF)
		return memory[address];

/* the addresses between 0x2000 and 0x5000 are for input/ouput */
	if(address == 0x2002) {
		ppu_status_tmp = ppu_status;

		/* set ppu_status (D7) to 0 (vblank_on) */
		ppu_status &= 0x7F;

		/* set ppu_status (D6) to 0 (sprite_zero) */
		ppu_status &= 0x1F;

		/* reset VRAM Address Register #1 */
		ppu_bgscr_f = 0x00;

		/* reset VRAM Address Register #2 */
		ppu_addr_h = 0x00;

		/* return bits 7-4 of unmodifyed ppu_status with bits 3-0 of the ppu_addr_tmp */
		return (ppu_status_tmp & 0xE0) | (ppu_addr_tmp & 0x1F);
	}


	if(address == 0x2007) {
        int tmp;
		tmp = ppu_addr_tmp;
		ppu_addr_tmp = ppu_addr;

		if(increment_32 == 0) {
			ppu_addr++;
		} else {
			ppu_addr += 0x20;
		}

		return ppu_memory[tmp];
	}

	/* pAPU data (sound) */
	if(address == 0x4015) {
		return memory[address];
	}

	/* joypad1 data */
	if(address == 0x4016) {
		switch(pad1_readcount) {
			case 0:
			memory[address] = pad1_A;
			pad1_readcount++;
			break;

			case 1:
			memory[address] = pad1_B;
			pad1_readcount++;
			break;

			case 2:
			memory[address] = pad1_SELECT;
			pad1_readcount++;
			break;

			case 3:
			memory[address] = pad1_START;
			pad1_readcount++;
			break;

			case 4:
			memory[address] = pad1_UP;
			pad1_readcount++;
			break;

			case 5:
			memory[address] = pad1_DOWN;
			pad1_readcount++;
			break;

			case 6:
			memory[address] = pad1_LEFT;
			pad1_readcount++;
			break;

			case 7:
			memory[address] = pad1_RIGHT;
			pad1_readcount = 0;
			break;
		}

		return memory[address];
	}

	if(address == 0x4017) {
		return memory[address];
	}

	if (MAPPER == 5) {
        if((address == 0x5204) || (address >= 0x5C00 && address <= 0x5FFF)) {
            return mmc5_read(address);
        }
    }

	if((address > 0x5FFF) && (address < 0x8000) && (MAPPER == 5)) { //SRAM
        return mmc5_wram[(address - 0x6000) + (mmc5_wram_page * 8192) + (mmc5_wram_chip * 8192)];
	}


	return memory[address];
}

int memory_read16(int address)
{
	return (memory_read(address) + (memory_read(address + 1) * 0x100));
}

int memory_read16zp(int address)
{
	return (memory_read(address & 0xFF) + (memory_read((address + 1) & 0xFF) * 0x100));
}

/* writeByte */
void write_memory(unsigned int address,unsigned char data) {
/* PPU Video Memory area */
	if(address > 0x1fff && address < 0x4000) {
		write_ppu_memory(address,data);
		return;
	}

	/* Sprite DMA Register */
	if(address == 0x4014) {
		write_ppu_memory(address,data);
		tick_count += 512;
		return;
	}

	/* Joypad 1 */
	if(address == 0x4016) {
		memory[address] = 0x40;

		return;
	}

	/* Joypad 2 */
	if(address == 0x4017) {
		memory[address] = 0x48;
		return;
	}

	/* pAPU Sound Registers */
	if(address > 0x3fff && address < 0x4016) {
		memory[address] = data;
		return;
	}
	
	/* SRAM Registers */
	if(address > 0x5fff && address < 0x8000) {
        if(MAPPER == 5) { //SRAM
            mmc5_wram[(address - 0x6000) + (mmc5_wram_page * 8192) + (mmc5_wram_chip * 8192)] = data;
            return;
        } else {

            memory[address] = data;
            return;
        }
	}

	/* RAM registers */
	if(address < 0x2000) {memory[address & 0x7FF] = data; return;}

	switch (MAPPER) {
		case 1:
			mmc1_access(address, data);
			return;
		break;

		case 2:
			unrom_access(address, data);
			return;
		break;

		case 3:
			cnrom_access(address, data);
			return;
		break;

		case 4:
			mmc3_access(address, data);
			return;
		break;

		case 5:
			mmc5_access(address, data);
			return;
		break;

		case 7:
			aorom_access(address, data);
			return;
		break;

		case 79:
			mapper79_access(address,data);
			return;
		break;

		default:

		break;
	}

	memory[address] = data;
}


void emu_start() {
    if(analyze_header(romfn) == 1) {
   		
	}


	if(load_rom(romfn) == 1) {

	}

	if(MAPPER == 4)
		mmc3_reset();

	if(SRAM == 1) {
		open_sav();
	
	}

	/* Reset 6502 CPU */
	CPU_reset();
	
	/* Reset INPUT's, Init the PPU and MIRROR */
	reset_input();
	init_ppu();
	do_mirror(MIRRORING);

}


void renderScreen() {
    /*
    There is a so many things i need understand here, so probably there is wrong code 
    or useless code here 
    */

    /* General Setup */
    GPU_SetViewport  ((u32*)osConvertVirtToPhys((u32)gpuDOut),(u32*)osConvertVirtToPhys((u32)gpuOut),0 ,0 ,240 * 2, 400);

    GPU_DepthRange    (-1.0f, 0.0f);
    GPU_SetFaceCulling(GPU_CULL_BACK_CCW);
    GPU_SetStencilTest(false, GPU_ALWAYS, 0x00);
    GPU_SetDepthTest  (true, GPU_GREATER, 0x1F);

    /* unknown */
    GPUCMD_AddSingleParam(0x00010062, 0x00000000); //param always 0x0 according to code
    GPUCMD_AddSingleParam(0x000F0118, 0x00000000);

    /* Setup Shader */
    SHDR_UseProgram(shader, 0x0);

    /* unknown 2 */
    GPUCMD_AddSingleParam(0x000F0100, 0x00E40100);
    GPUCMD_AddSingleParam(0x000F0101, 0x01010000);
    GPUCMD_AddSingleParam(0x000F0104, 0x00000010);

    /* Texturing Stuff */
    GPUCMD_AddSingleParam(0x0002006F, 0x00000100);
    GPUCMD_AddSingleParam(0x000F0080, 0x00011001); //enables/disables texturing

    /* TextEnv */
    GPU_SetTexEnv(3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000);
    GPU_SetTexEnv(4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000);
    GPU_SetTexEnv(5, GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
        GPU_TEVOPERANDS(0,0,0), GPU_TEVOPERANDS(0,0,0), GPU_MODULATE, GPU_MODULATE, 0xFFFFFFFF);

    /* Texturing Stuff */
    GPU_SetTexture((u32*)osConvertVirtToPhys((u32)TopScreenTexture), 256, 256, 0x6, GPU_RGBA8);

    GPU_DrawArray(GPU_TRIANGLES, 2 * 3);
}

void mainLoop() {
	int scanline = 0;
	APP_STATUS status;
	
	u32 gpuCmdSize = 0x40000;
	gpuCmd = (u32*) linearAlloc (gpuCmdSize * 4);

	GPU_Reset(gxCmdBuf, gpuCmd, gpuCmdSize);

	/* Alloc TopScreen Texture */

	TopScreenTexture = (u32*) linearAlloc(256 * 512 * 3);

	sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);

	NES_ROMLIST();

	/* Shader Instruction */
	shader = SHDR_ParseSHBIN((u32*)test_vsh_shbin, test_vsh_shbin_size);

	GX_SetMemoryFill(gxCmdBuf, (u32*)gpuOut, 0x404040FF, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
	gfxSwapBuffersGpu();
	NES_StartGame();

	while((status=aptGetStatus())!=APP_EXITING) {
		
		if(status==APP_RUNNING){
			ppu_status = 0;

	        if(skipframe > frameskip)
				skipframe = 0;

				GPUCMD_SetBuffer(gpuCmd, gpuCmdSize, 0);

		/*	if(inMenu == 1){
				NES_Menu();
			}else {
		*/
				for(scanline = 0; scanline < 262; scanline++) { //262 scanlines?

			            if (MAPPER == 5) mmc5_hblank(scanline); //MMC5 IRQ
			            CPU_execute(line_ticks);
			            if (scanline < 240) {
			                if (MAPPER == 4) mmc3_hblank(scanline);
			                render_scanline(scanline);
			            } else {
			                if (scanline == 241) {
			                    if(exec_nmi_on_vblank) { NMI(); }
			                    ppu_status = 0x80;
			                }
			            }
					}


				skipframe++;

				check_joypad();

				GX_SetDisplayTransfer(gxCmdBuf, (u32*)PPU_TopScreen, 0x02000100, (u32*)TopScreenTexture, 0x02000100, 0x3302);
				gspWaitForPPF();
				
				renderScreen();
				GSPGPU_FlushDataCache(NULL, (u8*)PPU_TopScreen, 256 * 512 * 3);

				GPUCMD_Finalize();
				GPUCMD_Run(gxCmdBuf);
				gspWaitForP3D();
				
				GX_SetDisplayTransfer(gxCmdBuf, (u32*)gpuOut, 0x019001E0, (u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0x019001E0, 0x01001000);
				gspWaitForPPF();

				/* Work like memset */
				GX_SetMemoryFill(gxCmdBuf, (u32*)gpuOut, 0x253040FF, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
				gspWaitForPSC0();

			} else if(status == APP_SUSPENDING) {
				aptReturnToMenu();
			} else if(status == APP_SLEEPMODE) {
				aptWaitStatusEvent();
			}

		gspWaitForVBlank();
	}


	exitAPP();
	return 0;
}


void reset_emulation() {
	if(load_rom(romfn) == 1) {

	}

	if(MAPPER == 4)
		mmc3_reset();

	CPU_reset();

	reset_input();

	start_emulation();
}

int start_emulation() {
	srvInit();	
	fsInit();
	aptInit();
	gfxInit();
	hidInit(NULL);
	aptSetupEventHandler();

	/* Initialize GPU */
	GPU_Init(NULL);


	/* Clear SRAM */
	SRAM = 0; 

	mainLoop();

	return 0;
}
