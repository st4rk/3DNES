#ifndef NESMEMORY_H
#define NESMEMORY_H


#include <3ds.h>
#include "nesGlobal.h"
#include "nesPPU.h"
#include "nesSystem.h"
#include "nesLoadROM.h"
#include "6502core.h"

#include "utils.h"
#include "mmc1.h"	// 1
#include "unrom.h"	// 2
#include "cnrom.h"	// 3
#include "mmc3.h"	// 4
#include "mmc5.h"	// 5
#include "aorom.h"	// 7
#include "mapper79.h"	// 79

int PAD1_ReadCount = 0;


/* Read a byte */
u8 	memoryRead(u32 addr);
/* Read 2 bytes */
int  memoryRead16(u32 addr);
/* write a byte */
void writeMemory(u32 addr, u8 data);

#endif
