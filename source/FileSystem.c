#include "FileSystem.h"
#include "nesGlobal.h"


u8  ROM_Cache[MAX_ROM_SIZE];
u8 *SRAM_Name;
u64	ROM_Size;
FS_archive sdmcArchive;

extern bool inGame;	
extern bool VSYNC;
extern u8	frameSkip;

void unicodeToChar(char* dst, u16* src) {
    if(!src || !dst)return;
    while(*src)*(dst++)=(*(src++))&0xFF;
    *dst=0x00;
}


/* Load Complete ROM LIST */
void NES_LOADROMLIST() {
	Handle romHandle;
	
	FS_dirent dirStruct;
	FS_path dirPath = FS_makePath(PATH_CHAR, "/3DNES/ROMS");

	/* INIT SDMC ARCHIVE */
	sdmcArchive = (FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);

	FSUSER_OpenDirectory(NULL, &romHandle, sdmcArchive, dirPath);

	/* Get total of files/directory on 3DS SD */
	int cont = 0;

	while(1) {
		u32 dataRead = 0;
		FSDIR_Read(romHandle, &dataRead, 1, &dirStruct);
		if(dataRead == 0) break;
		unicodeToChar(fileSystem.fileList[cont], dirStruct.name);
		cont++;
	}

	/* Save total of files */
	fileSystem.totalFiles = cont;
	
	/*
	TODO: Dynamic File Read
	fileSystem.fileList = linearAlloc(MAX_FILENAME_SIZE * cont);

	FSUSER_OpenDirectory(NULL, &romHandle, sdmcArchive, dirPath);

	cont = 0;

	while(1) {
		u32 dataRead = 0;
		FSDIR_Read(romHandle, &dataRead, 1, &dirStruct);
		if(dataRead == 0) break;
		unicodeToChar(&fileSystem.fileList[MAX_FILENAME_SIZE * cont), dirStruct.name);
		cont++;
	}
	
	*/

	FSDIR_Close(romHandle);
}

/* Draw All ROM's */
void NES_drawROMLIST() {
	int i = 0;

	// TODO: Fix select file bar 
	//draw_select_bar(-67, (fileSystem.cFile * 15) + 53);

	for(i = 0; i < fileSystem.totalFiles; i++) {
		draw_string_c(55 + (i * 15), fileSystem.fileList[i]);
	}

	draw_string(25, (fileSystem.cFile * 15) + 53, "->");

}

/* Draw Configuration Menu */
void NES_drawConfigurationMenu() {
	char gameFPS[18];


	sprintf(gameFPS, "FrameSkip: %d", frameSkip);

	draw_string(10, (fileSystem.cConfig* 15) + 70, "->");
	draw_string_c(50, "Configuration Menu");
	draw_string_c(73, gameFPS);
	
	if (VSYNC)
		draw_string_c(88, "VSYNC: ENABLE");
	else
		draw_string_c(88, "VSYNC: DISABLE");


	draw_string_c(103,"Sprites: ");
	draw_string_c(118, "Exit and Start Game");

}

void FS_StringConc(char* dst, char* src1, char* src2) {
	int i = 0;
    int Size2 = strlen(src1);
    int Size3 = strlen(src2);


    for (i = 0; i < Size2; i++) {
        dst[i] = src1[i];
    }

    for (i = 0; i < Size3; i++) {
        dst[i + Size2] = src2[i];
    }

}

void NES_LoadSelectedGame() {
	u32    bytesRead = 0;
	u32    SRAM_Size = 0;
	u32    ROMDIR_Size = (strlen("/3DNES/ROMS/") + strlen(fileSystem.fileList[fileSystem.currFile]) + 1);
	Handle fileHandle;

	/* Alloc ROM Directory */
	char ROM_DIR[ROMDIR_Size];

	memset(ROM_DIR, 0x0, ROMDIR_Size);
	/* concatenate strings */
	FS_StringConc(ROM_DIR,  "/3DNES/ROMS/", fileSystem.fileList[fileSystem.currFile]);

	/* TODO: FIX IT
	/*if (SRAM_Name != NULL) {
		linearFree(SRAM_Name);
		SRAM_Size = (strlen(fileSystem.fileList[fileSystem.currFile]) - 4);
		SRAM_Name = linearAlloc(SRAM_Size);
		strncpy((char*)SRAM_Name, fileSystem.fileList[fileSystem.currFile], SRAM_Size);
	}
	*/
	
	FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, ROM_DIR), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	FSFILE_GetSize(fileHandle, &ROM_Size);
	
	/* TODO: Fix Dynamic ROM 
	if (ROM_Cache != NULL)
		linearFree(ROM_Cache);

	ROM_Cache = linearAlloc(ROM_Size);
	*/
	FSFILE_Read(fileHandle, &bytesRead, 0x0, (u32*)ROM_Cache, (u32)ROM_Size);
	FSFILE_Close(fileHandle);

	
	/* FREE Allocation */
	linearFree(ROM_DIR);

	/* Start Emulation */
	inGame = true;
}

void NES_ConfigurationMenu() { 
	u32 keys = ((u32*)0x10000000)[7];
	
	/* Configuration Menu */
		if(keys & BUTTON_LEFT) {
			if(!fileSystem.UKEY_LEFT) {
				switch(fileSystem.cConfig) {
					case 0:
						frameSkip--;
					break;

					case 1:
						VSYNC = false;
					break;

					case 2:
						ENABLE_Sprite = 0;
					break;

					default:

					break;
				}

				fileSystem.UKEY_LEFT = 1;
			}
		} else {
			fileSystem.UKEY_LEFT = 0;
		}
	
		if(keys & BUTTON_RIGHT) {
			if(!fileSystem.UKEY_RIGHT) {
				switch(fileSystem.cConfig) {
					case 0:
						frameSkip++;
					break;

					case 1:
						VSYNC = true;
					break;

					case 2:
						ENABLE_Sprite = 1;
					break;

					default:

					break;
				}


				fileSystem.UKEY_RIGHT = 1;
			}
		} else {
			fileSystem.UKEY_RIGHT = 0;
		}

		if(keys & BUTTON_UP){
			if(!fileSystem.UKEY_UP){
				if(fileSystem.cConfig > 0)
					fileSystem.cConfig--;
				
				fileSystem.UKEY_UP = 1;
			}
		} else {
			fileSystem.UKEY_UP = 0;
		}

		if(keys & BUTTON_DOWN){
			if(!fileSystem.UKEY_DOWN) {

				if(fileSystem.cConfig < 10)
					fileSystem.cConfig++;
				
				fileSystem.UKEY_DOWN = 1;
			}
		} else {
			fileSystem.UKEY_DOWN = 0;
		}


		if(keys & BUTTON_B) {
			if(!fileSystem.UKEY_B) {
				if(fileSystem.cConfig == 3)
					fileSystem.inMenu = 0;

				fileSystem.UKEY_B = 1;
			} else {
				fileSystem.UKEY_B = 0;
			}
		}

}

void NES_CurrentFileUpdate() {
	u32 keys = ((u32*)0x10000000)[7];

	if(fileSystem.inMenu == 0) {
		if(keys & BUTTON_UP){
			if(!fileSystem.UKEY_UP){
				if(fileSystem.sFile > 0) 
					fileSystem.sFile--;
				 else {
					if(fileSystem.cFile > 0)
					   fileSystem.cFile--;
				}

				if(fileSystem.currFile > 0)
					fileSystem.currFile--;
				
				fileSystem.UKEY_UP = 1;
			}
		} else {
			fileSystem.UKEY_UP = 0;
		}

		if(keys & BUTTON_DOWN){
			if(!fileSystem.UKEY_DOWN) {
				if(fileSystem.cFile < fileSystem.totalFiles)
					fileSystem.cFile++;
				 else 
					fileSystem.sFile++;
				
				fileSystem.currFile++;
				
				fileSystem.UKEY_DOWN = 1;
			}
		} else {
			fileSystem.UKEY_DOWN = 0;
		}


		if(keys & BUTTON_LEFT) {
			if(!fileSystem.UKEY_LEFT) {
				fileSystem.inMenu = 1;

				fileSystem.UKEY_LEFT = 1;
			}
		} else {
			fileSystem.UKEY_LEFT = 0;
		}
	

	}


	if(keys & BUTTON_B) {
			if(!fileSystem.UKEY_B) {
				NES_LoadSelectedGame();

				fileSystem.UKEY_B = 1;
			} else {
				fileSystem.UKEY_B = 0;
			}
		}

}


void NES_MainMenu() {
	if (fileSystem.inMenu == 0) {
		NES_drawROMLIST();
		NES_CurrentFileUpdate();
	}
	else {
		NES_drawConfigurationMenu();
		NES_ConfigurationMenu();
	}
}
