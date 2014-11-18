#ifndef NESMEMORY_H
#define NESMEMORY_H


#include <3ds.h>


/* Read a byte */
extern u8 	memoryRead(u32 addr);
/* Read 2 bytes */
extern int  memoryRead16(u32 addr);
/* write a byte */
extern void writeMemory(u32 addr, u8 data);

#endif