/*
@ -----------------------------------------------------------------
@  3DNES 6502core - Written by gdkchan and St4rk
@  Date: 27/09/2014
@  3DNES é um emulador de Nintendo Entertainment System para 3DS
@  esse é um projeto open-source, você pode modificar e utilizar
@  os arquivos para estudo, desde que mantenha os devidos créditos
@ -----------------------------------------------------------------
*/


#include <3ds.h>
/* NES Memory */
extern unsigned char memory[65536];

/* Registers to use */

	unsigned char A;
	unsigned char P;
	unsigned char X;
	unsigned char Y;
	unsigned char S;
	int PC;


int EA; //Effective Address
int tick_count;

/* opcode of 6502 */
extern void (*op[0x100])();


extern void IRQ();
extern void NMI();
extern void CPU_reset();
extern void CPU_execute(int cycles);
