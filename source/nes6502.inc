@ -----------------------------------------------------------------
@  3DNES 6502 - Written by St4rk
@  Date: 18/11/2014
@  3DNES é um emulador de Nintendo Entertainment System para 3DS
@  esse é um projeto open-source, você pode modificar e utilizar
@  os arquivos para estudo, desde que mantenha os devidos créditos
@ -----------------------------------------------------------------



@ ---------------    CPU FLAGS    --------------------------------

.equ carryFlag,      0x01
.equ zeroFlag,       0x02
.equ interruptFlag,  0x04
.equ decimalFlag,    0x08
.equ sInterruptFlag, 0x10
.equ unusedFlag,     0x20
.equ overflowFlag,   0x40
.equ signFlag,       0x80 

@ ----------------- CPU REGISTERS --------------------------------


@ TODO: each ARM register has 32 bits, so use A,X,Y in just one register(24 bits, 8 bits unused or other important thing =P)
nesF						.req r4
nesA						.req r5
nesX						.req r6
nesY						.req r7
nesStack					.req r8
nesPC						.req r9  
nesEA						.req r10 
nesTick						.req r11 
nesMemory					.req r12

