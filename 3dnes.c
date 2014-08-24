

#include <stdio.h>
#include <string.h>

#include "6502core.h"
#include "macros.h"
#include "3dnes.h"
#include "romloader.h"
#include "ppu.h"
#include "input.h"
#include "functions.h"


/* included mappers */
#include "mappers/utils.h"
#include "mappers/mmc1.h"	// 1
#include "mappers/unrom.h"	// 2
#include "mappers/cnrom.h"	// 3
#include "mappers/mmc3.h"	// 4
#include "mappers/mmc5.h"	// 5
#include "mappers/aorom.h"	// 7
#include "mappers/mapper79.h"	// 79
 
/* CTR-LIB */
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/APT.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/HID.h>
#include <ctr/svc.h>
#include <ctr/FS.h>

char romfn[256];

extern int show_menu;

/* cache the rom in memory to access the data quickly */
unsigned char romcache[1048576];
unsigned char ppu_memory[16384];
unsigned char sprite_memory[256];

/* PAD 1 Data */
unsigned int pad1_data;
int pad1_readcount = 0;

/* Tick per Line */
int line_ticks = 114;

int in_emulation = 1;

int CPU_is_running = 1;
int pause_emulation = 0;

int inMenu = 1;

int enable_background = 1;
int enable_sprites = 1;

unsigned char frameskip = 0;
unsigned char skipframe = 0;

char rom_name[48];

long romlen;
Handle fsuHandle;

u8 in3D = 0;


/*************************************************************************************/
								/* File System */
FS_archive sdmcArchive;

#define MAX_SIZE 48
#define MAX_SCREEN_FILES 10
#define MAX_FILES 80

int sFile = 0; /* Selected File */
int cFile = 0; /* Cursor File */
int currFile = 0; /* Current File */
char tn_files[MAX_FILES][MAX_SIZE]; /* Total files/names on SD */

/* Used to press key just one time ! */
u8 UKEY_UP = 0;
u8 UKEY_DOWN = 0;

void NES_ROMLIST() {
	
	u16 dirName[512]; 
	Handle romHandle;

	FS_path dirPath=FS_makePath(PATH_CHAR, "/3DNES/ROMS/");

	sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

	FSUSER_OpenArchive(fsuHandle, &sdmcArchive);
	FSUSER_OpenDirectory(fsuHandle, &romHandle, sdmcArchive, dirPath);

	/* Get total of files/directory on 3DS SD */
	int cont = 0;
	while(1) {
		u32 dataRead = 0;
		FSDIR_Read(romHandle, &dataRead, 1, dirName);

		if(dataRead == 0)
			break;

		unicodeToChar(tn_files[cont], dirName);
		cont++;
	}

	FSDIR_Close(romHandle);
}

void NES_DrawFileList() {
	int i = 0;

	draw_select_bar(-67, (cFile * 15) + 53);

	for(i = 0; i < MAX_SCREEN_FILES; i++) {
		draw_string_c(55 + (i * 15), tn_files[i + sFile]);
	}

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
	sprintf(ROM_DIR, "/3DNES/ROMS/%s", tn_files[currFile]);

	FS_archive sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

	/* Open File */
	FSUSER_OpenFileDirectly(fsuHandle, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, ROM_DIR), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	
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


void NES_CurrentFileUpdate() {
	u32 keys = ((u32*)0x10000000)[7];

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


	if(keys & BUTTON_B) 
		NES_StartGame();

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

	FSUSER_OpenArchive(fsuHandle, &sdmcArchive);

	FSUSER_OpenDirectory(fsuHandle, &dirHandle, sdmcArchive, dirPath);

	FSUSER_OpenFile(fsuHandle, &ScreenHandle, sdmcArchive, FS_makePath(PATH_CHAR,  "SS.bin"), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
 	
 	FSFILE_Write(ScreenHandle, &bytesRead, 0x0, &FrameBuffer, 288000, 0x10001);

 	FSFILE_Close(ScreenHandle);
}


void NES_Menu() {
	if(inMenu == 1) {
		NES_DrawFileList();
		NES_CurrentFileUpdate();
		updateMenu();
		check_joypad();
	}
}


/*************************************************************************************/
/* exitAPP */
void exitAPP() {
	hidExit();
	gspGpuExit();
	aptExit();
	svc_exitProcess();
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

	FSUSER_OpenArchive(fsuHandle, &sdmcArchive);

	FSUSER_OpenDirectory(fsuHandle, &dirHandle, sdmcArchive, dirPath);
 
	Handle fileHandledump;
	FSUSER_OpenFile(fsuHandle, &fileHandledump, sdmcArchive, FS_makePath(PATH_CHAR,  resultado), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
 	
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

	FSUSER_OpenArchive(fsuHandle, &sdmcArchive);

	FSUSER_OpenDirectory(fsuHandle, &dirHandle, sdmcArchive, dirPath);
 
	Handle fileHandledump;
	FSUSER_OpenFile(fsuHandle, &fileHandledump, sdmcArchive, FS_makePath(PATH_CHAR,  resultado), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
 	
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

void start_emulation() {
	int scanline = 0;
	APP_STATUS status;

	FSUSER_Initialize(fsuHandle);
	sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(fsuHandle, &sdmcArchive);

	NES_ROMLIST();

	while((status=aptGetStatus())!=APP_EXITING) {

		if(status==APP_RUNNING){
				ppu_status = 0;

	        if(skipframe > frameskip)
				skipframe = 0;

			if(inMenu == 1){
				NES_Menu();
			}else {
				for(scanline = 0; scanline < 262; scanline++) { //262 scanlines?
					
			            if (MAPPER == 5) mmc5_hblank(scanline); //MMC5 IRQ
			            CPU_execute(line_ticks);
			            if (scanline < 240) {
			                if (MAPPER == 4) mmc3_hblank(scanline);
			                render_scanline(scanline);
			            } else {
			                if (scanline == 241) {
			                    if(exec_nmi_on_vblank) {NMI();}
			                    ppu_status = 0x80;
			                }
			            }
					}

				
					if(skipframe == 0)
						update_screen();

					skipframe++;

					check_joypad();
			    }

			}
			else if(status == APP_SUSPENDING)
			{
				aptReturnToMenu();
			}
			else if(status == APP_SLEEPMODE)
			{
				aptWaitStatusEvent();
			}
	
	}


	exitAPP();
	in_emulation = 0;
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

int start_emu() {
  	initSrv();
	aptInit(APPID_APPLICATION);
	gspGpuInit();
	hidInit(NULL);
	aptSetupEventHandler();
	srv_getServiceHandle(NULL, &fsuHandle, "fs:USER");


	SRAM = 0; 

	start_emulation();

	return 0;
}
