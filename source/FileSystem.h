#ifndef FILESYSTEM_H
#define FILESYSTEM_H



#include <3ds.h>
#include <stdbool.h>
#include <string.h>


#define MAX_FILENAME_SIZE 0x106


FS_archive sdmcArchive;

/* File System Structure */
typedef struct FS_MENU {
	/* File System PAD */
	u8 UKEY_UP;
	u8 UKEY_DOWN;
	u8 UKEY_LEFT;
	u8 UKEY_RIGHT;
	u8 UKEY_B;

	/* Selected File */
	int sFile; 
	/* Cursor File */
	int cFile; 
	/* Current File */
	int currFile;

	/* inConfiguration ? */
	bool inMenu;
	/* Configuration Menu Cursor */
	int cConfig;
	/* FileList */
	char *fileList;
	/* totalFiles */
	int totalFiles;
} FS_MENU;


FS_MENU fileSystem;


/* Load Complete ROM LIST */
void NES_LOADROMLIST();
/* Draw All ROM's */
void NES_drawROMLIST();
/* Draw Configuration Menu */
void NES_drawConfigurationMenu();
/* Update Cursor Position */
void NES_ConfigurationMenu();
/* Update Cursor Position */
void NES_CurrentFileUpdate();
/* Load Game on ROM */
void NES_LoadSelectedGame();
/* Main Menu of 3DNES */
void NES_MainMenu();

/* FS_String Conc */
void FS_StringConc(char* dst, char* src1, char* src2);

/* unicode_to_char */
void unicode_to_char(char* dst, unsigned short *src);

#endif