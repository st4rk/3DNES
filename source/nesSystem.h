#ifndef NESSYSTEM_H
#define NESSYSTEM_H


#include <3ds.h>
#include "nesGlobal.h"
#include "FileSystem.h"
#include "6502core.h"

u32 PAD1_UP;
u32 PAD1_DOWN;
u32 PAD1_LEFT;
u32 PAD1_RIGHT;

u32 PAD1_A;
u32 PAD1_B;

u32 PAD1_START;
u32 PAD1_SELECT;


/* SRAM Save and Load */
void SRAM_LOADSTATE();
void SRAM_SAVESTATE();

/* Emulation Stuff */
void SET_INPUT(u32 padKey);
void CLEAR_INPUT(u32 padKey);
void RESET_INPUT();


/* Save State and Load State */
void SAVE_STATE(int n);
void LOAD_STATE(int n);


#endif