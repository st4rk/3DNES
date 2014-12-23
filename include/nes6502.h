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

u8 memory[65536];

extern void IRQ();
extern void NMI();
extern void CPU_Reset();
extern void CPU_Execute();
