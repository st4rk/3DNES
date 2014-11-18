#include "nesSystem.h"


/* SRAM Save and Load */
void SRAM_LOADSTATE() {
	Handle dirHandle;
	u32 bytesRead;
	
	u8 resultado[strlen(SRAM_Name) + strlen("/3DNES/SAVES/") + 1];

	FS_StringConc( resultado, (u8*)"/3DNES/SAVES/", SRAM_Name);
	FS_path dirPath=FS_makePath(PATH_CHAR, "/3DNES/saves/");

	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
 
	Handle fileHandledump;
	FSUSER_OpenFile(NULL, &fileHandledump, sdmcArchive, FS_makePath(PATH_CHAR,  resultado), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
 	
	/* Load ROM in ROM Cache */
	FSFILE_Read(fileHandledump, &bytesRead, 0x0, (u32*)&memory[0x6000],(u32)8192);

 	FSFILE_Close(fileHandledump);
}

void SRAM_SAVESTATE() {
	Handle dirHandle;
	u32 bytesRead;
	
	u8 resultado[strlen(SRAM_Name) + strlen("/3DNES/SAVES/") + 1];

	FS_StringConc( resultado, (u8*)"/3DNES/SAVES/", SRAM_Name);
	FS_path dirPath=FS_makePath(PATH_CHAR, "/3DNES/saves/");

	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
 
	Handle fileHandledump;
	FSUSER_OpenFile(NULL, &fileHandledump, sdmcArchive, FS_makePath(PATH_CHAR,  resultado), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
 	
 	FSFILE_Write(fileHandledump, &bytesRead, 0x0, &memory[0x6000], 8192, 0x10001);

 	FSFILE_Close(fileHandledump);
}

/* Emulation Stuff */
void SET_INPUT(u32 padKey) {
	switch(padKey) {
			/* pad_down */
			case 1:
			PAD1_DOWN = 0x01;
			break;

			/* pad_up */
			case 2:
			PAD1_UP = 0x01;
			break;

			/* pad_left */
			case 3:
			PAD1_LEFT = 0x01;
			break;

			/* pad_right */
			case 4:
			PAD1_RIGHT = 0x01;
			break;

			/* pad_start */
			case 5:
			PAD1_START = 0x01;
			break;

			/* pad_select */
			case 6:
			PAD1_SELECT = 0x01;
			break;

			/* pad_a */
			case 7:
			PAD1_A = 0x01;
			break;

			/* pad_b */
			case 8:
			PAD1_B = 0x01;
			break;

			default:
			
			break;
	}

}

void RESET_INPUT() {
	PAD1_DOWN = 0x40;
	PAD1_UP = 0x40;
	PAD1_LEFT = 0x40;
	PAD1_RIGHT = 0x40;
	PAD1_START = 0x40;
	PAD1_SELECT = 0x40;
	PAD1_A = 0x40;
	PAD1_B = 0x40;
}


void CLEAR_INPUT(u32 padKey) {
	switch(padKey) {
		/* pad_down */
		case 1:
		PAD1_DOWN = 0x40;
		break;

		/* pad_up */
		case 2:
		PAD1_UP = 0x40;
		break;

		/* pad_left */
		case 3:
		PAD1_LEFT = 0x40;
		break;

		/* pad_right */
		case 4:
		PAD1_RIGHT = 0x40;
		break;

		/* pad_start */
		case 5:
		PAD1_START = 0x40;
		break;

		/* pad_select */
		case 6:
		PAD1_SELECT = 0x40;
		break;

		/* pad_a */
		case 7:
		PAD1_A = 0x40;
		break;

		/* pad_b */
		case 8:
		PAD1_B = 0x40;
		break;

		default:
		/* never reached */
		break;
	}
}


/* Save State and Load State */
// TODO : Save State of course 
void SAVE_STATE(int n) {

}

void LOAD_STATE(int n) {

}