#ifndef NESLOADROM_H
#define NESLOADROM_H


#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "nesGlobal.h"
#include "nes6502.h"

u8 PRG;
u8 CHR;
u8 MAPPER;

int RCB;
int OS_MIRROR;
int FS_MIRROR;
int TRAINER;
int SRAM;
int MIRRORING;
int VRAM;

int NES_LoadROM();


#endif
