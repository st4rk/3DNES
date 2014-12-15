@ -----------------------------------------------------------------
@  3DNES 6502 - Written by St4rk
@  Date: 08/12/2014
@  3DNES é um emulador de Nintendo Entertainment System para 3DS
@  esse é um projeto open-source, você pode modificar e utilizar
@  os arquivos para estudo, desde que mantenha os devidos créditos
@ -----------------------------------------------------------------

.arm
.align 4

#include "nes6502.inc"

@ --------------------
@ -    Data Region  --
@ --------------------

.data 
.global CPU_6502_REG @ 6502 Registers, we will load/save the registers here
CPU_6502_REG:
		.long 0, 0, 0, 0, 0, 0, 0, 0, 0

.equ TOTAL_CYCLE, 114

.text

@ ------------------------------------------------
@ - Addresing Mode, Load and Save Registers etc --
@ ------------------------------------------------

.macro LOAD_6502
	ldr		r0, =CPU_6502_REG
	ldmia	r0, {r4-r11} 
.endm


.macro SAVE_6502
	ldr		r0, =CPU_6502_REG
	stmia	r0, {r4-r11}
.endm

.macro IMM @ Immediate 
	mov nesEA, nesPC
	add nesPC, #0x100
.endm

.macro ZP  @ zeroPage
	mov r0, nesPC
	bl memoryRead 
	add nesPC, #0x1
	mov nesEA, r0
.endm

.macro ZPX @ zeroPage X
	mov r0, nesPC
	bl memoryRead
	add nesPC, #0x1
	add nesEA, r0, nesX
.endm

.macro ZPY @ zeroPage Y
	mov r0, nesPC
	bl memoryRead
	add nesPC, #0x1
	add nesEA, r0, nesY
.endm

.macro REL @ relative for branch ops (8 bit immediate value, sign-extended)
	mov r0, nesPC
	bl memoryRead
	add nesPC, #0x1
	mov nesEA, r0
	cmp 	nesEA, #0x80
	sublt 	nesEA, #0x100 @ If (nesEA >= 0x80) nesEA -= 0x100
.endm

.macro ABSO @ absolute
	mov r0, nesPC
	bl memoryRead
	mov nesEA, r0
	
	add nesPC, #0x1
	
	mov r0, nesPC
	bl memoryRead
	orr nesEA, nesEA, r0, ASL #8
	add 	nesPC, #0x2
.endm

.macro ABSX @ absolute, X  TODO: addr in cycles
	mov r0, nesPC
	bl memoryRead
	mov nesEA, r0
	
	add nesPC, #0x1
	
	mov r0, nesPC
	bl memoryRead
	orr nesEA, nesEA, r0, ASL #8
	add nesEA, nesX
	add 	nesPC, #0x2
.endm

.macro ABSXY @ absolute, Y  TODO: addr in cycles
	mov r0, nesPC
	bl memoryRead
	mov nesEA, r0
	
	add nesPC, #0x1
	
	mov r0, nesPC
	bl memoryRead
	orr nesEA, nesEA, r0, ASL #8
	add nesEA, nesY
	add 	nesPC, #0x2
.endm

.macro IND  @ indirect
	mov r0, nesPC
	bl memoryRead
	add nesPC, #0x1
	
	mov r0, nesPC
	bl memoryRead
	orr nesEA, nesEA, r0, ASL #8
	mov r1, nesEA
	
	add nesPC, #0x2
	add r2, r1, #0x1
	orr r1, r2, r1, ASL #8

	mov r0, nesEA
	bl memoryRead
	mov nesEA, r0
	mov r0, r2
	bl memoryRead

	orr nesEA, nesEA, r0, ASL #8
.endm

.macro INDX @ indirect, x
	mov r0, nesPC
	bl memoryRead
	add nesEA, r0, nesX
	add nesPC, #0x1

	mov r0, nesEA
	bl memoryRead
	mov r1, r0

	add r0, nesEA, #0x1	
	bl memoryRead

	orr nesEA, r1, r0, ASL #8
.endm


.macro INDY  @ indirect, y  TODO: addr in cycles
	mov r0, nesPC
	bl memoryRead
	add nesEA, r0, #0x1
	add nesPC, #0x1

	orr r1, nesEA, r0, ASL #8 @ data

	bl memoryRead
	mov nesEA, r0
	
	mov r0, r1
	bl memoryRead

	orr nesEA, nesEA, r0, ASL #8
	add nesEA, nesY
.endm

.global IRQ
IRQ: @ maskable interrupt
	stmdb sp!, {r0-r12, lr}
	LOAD_6502 @ Load 6502 Registers

	add nesStack, #0x64 @ nesStack += 100
	lsr nesPC, #0x8 @ nesPC >> 8
	mov r0, nesStack @ r0 = nesStack
	mov r1, nesPC 	 @ r1 = nesStack
	bl writeMemory @ nesStack, nesPC
	sub nesStack, #0x1 @ nesStack--
	mov r0, nesStack @ r0 = nesStack
	bl writeMemory @ nesStack, nesPC
	sub nesStack, #0x1 @ nesStack--
	mov r0, nesStack
	orr nesF, #sInterruptFlag @ software Interrupt Flag
	bl writeMemory @ nesStack, nesPC
	sub nesStack, #0x1
	orr nesF, #interruptFlag @ Interrupt
	ldr nesPC, =(memory+0xFFFF)
	ldr r0,    =(memory+0xFFFE) @ r0   = memory[0xFFFE]
	ldrh nesPC, [nesPC]
	ldrh r0, [r0]
	and r0, r0, #0xFF00
	orr nesPC, nesPC, r0
	add nesTick, #0x7

	SAVE_6502 @ End of operation save 6502 registers
	
	ldmia sp!, {r0-r12, pc}


.global NMI
NMI: @ non-maskable interrupt
	stmdb sp!, {r0-r12, lr}

	LOAD_6502 @ Load 6502 Registers

	add nesStack, #0x64 @ nesStack += 100
	lsr nesPC, #0x8 @ nesPC >> 8
	mov r0, nesStack @ r0 = nesStack
	mov r1, nesPC 	 @ r1 = nesStack
	bl writeMemory @ nesStack, nesPC
	sub nesStack, #0x1 @ nesStack--
	mov r0, nesStack @ r0 = nesStack
	bl writeMemory @ nesStack, nesPC
	sub nesStack, #0x1 @ nesStack--
	mov r0, nesStack
	orr nesF, #sInterruptFlag @ software Interrupt Flag
	bl writeMemory @ nesStack, nesPC
	sub nesStack, #0x1
	orr nesF, #interruptFlag @ Interrupt
	ldr nesPC, =(memory+0xFFFB)
	ldr r0,    =(memory+0xFFFA) @ r0   = memory[0xFFFE]
	ldrh nesPC, [nesPC]
	ldrh r0, [r0]
	and r0, r0, #0xFF00
	orr nesPC, nesPC, r0
	add nesTick, #0x7

	SAVE_6502 @ End of operation save 6502 registers
	
	ldmia sp!, {r0-r12, pc}



@ ----------------------
@ - OPCODE JUMP TABLE --
@ ----------------------

opcodeJumpTable:
	@      0          1       2      3       4        5       6        7     8       9      A      B       C           D        E        F
	.long brk,    ora_indx, NULL,   NULL,  NULL,    ora_zp,  asl_zp,  NULL, php, ora_imm,  asl_a, NULL,   NULL,     ora_abso, asl_abso, NULL @ 0
	.long bpl,    ora_indy, NULL,   NULL,  NULL,    ora_zpx, asl_zpx, NULL, clc, ora_absy, NULL,  NULL,   NULL,     ora_absx, asl_absx, NULL @ 1
	.long jsr,    and_indx, NULL,   NULL,  bit_zp,  and_zp,  rol_zp,  NULL, plp, and_imm,  rol_a, NULL,   bit_abso, and_abso, rol_abso, NULL @ 2  
	.long bmi,    and_indy, NULL,   NULL,  NULL,    and_zpx, rol_zpx, NULL, sec, and_absy, NULL,  NULL,   NULL,     and_absx, rol_absx, NULL @ 3
	.long rti,    eor_indx, NULL,   NULL,  NULL,    eor_zp,  lsr_zp,  NULL, pha, eor_imm,  lsr_a, NULL,   jmp_abso, eor_abso, lsr_abso, NULL @ 4
	.long bvc,    eor_indy, NULL,   NULL,  NULL,    eor_zpx, lsr_zpx, NULL, cli, eor_absy, NULL,  NULL,   NULL,     eor_absx, lsr_absx, NULL @ 5
	.long rts,    adc_indx, NULL,   NULL,  NULL,    adc_zp,  ror_zp,  NULL, pla, adc_imm,  ror_a, NULL,   jmp_ind,  adc_abso, ror_abso, NULL @ 6
	.long bvs,    adc_indy, NULL,   NULL,  NULL,    adc_zpx, ror_zpx, NULL, sei, adc_absy, NULL,  NULL,   NULL,     adc_absx, ror_absx, NULL @ 7
	.long NULL,   sta_indx, NULL,   NULL,  sty_zp,  sta_zp,  stx_zp,  NULL, dey, NULL,     txa,   NULL,   sty_abso, sta_abso, stx_abso, NULL @ 8
	.long bcc,    sta_indy, NULL,   NULL,  sty_zpx, sta_zpx, stx_zpy, NULL, tya, sta_absy, txs,   NULL,   NULL,     sta_absx, NULL,     NULL @ 9
	.long ldy_imm,lda_indx, ldx_imm,NULL,  ldy_zp,  lda_zp,  ldx_zp,  NULL, tay, lda_imm,  tax,   NULL,   ldy_abso, lda_abso, ldx_abso, NULL @ A
	.long bcs,    lda_indy, NULL,   NULL,  ldy_zpx, lda_zpx, ldx_zpy, NULL, clv, lda_absy, tsx,   NULL,   ldy_absx, lda_absx, ldx_absy, NULL @ B
	.long cpy_imm,cmp_indx, NULL,   NULL,  cpy_zp,  cmp_zp,  dec_zp,  NULL, iny, cmp_imm,  dex,   NULL,   cpy_abso, cmp_abso, dec_abso, NULL @ C
	.long bne,    cmp_indy, NULL,   NULL,  NULL,    cmp_zpx, dec_zpx, NULL, cld, cmp_absy, NULL,  NULL,   NULL,     cmp_absx, dec_absx, NULL @ D
	.long cpx_imm,sbc_indx, NULL,   NULL,  cpx_zp,  sbc_zp,  inc_zp,  NULL, inx, sbc_imm,  nop,   sbc_imm,cpx_abso, sbc_abso, inc_abso, NULL @ E
	.long beq,    sbc_indy, NULL,   NULL,  NULL,    sbc_zpx, inc_zpx, NULL, sed, sbc_absy, nop,   NULL,   NULL,     sbc_absx, inc_absx, NULL @ F 




.macro instruFetch
	ldr r1, =opcodeJumpTable
	ldr r1, [r1, r0] @ get address 
	bl  r1
.endm


@ -----------------------
@ - 6502 Execute/Reset --
@ -----------------------


.global CPU_Execute
CPU_Execute:
	stmdb sp!, {r0-r12, lr}
	LOAD_6502 @ Load All Registers
	b end_execute @ Start CPU_Loop

CPU_Loop:
	ldr r1, =memory
	ldrb r0, [r1, nesPC] 
	add nesPC, #0x1 @ r0 = memory[nesPC], nesPC++
	instruFetch

end_execute:
	cmp nesTick, #TOTAL_CYCLE
	blt CPU_Loop @ if (nesTick < TOTAL_CYCLE) GoTo CPU_Loop

	SAVE_6502
	ldmia sp!, {r0-r12, pc}


.global CPU_Reset
CPU_Reset:
	stmdb sp!, {r0-r12, lr}
	LOAD_6502

	mov nesX, 		#0x0
	mov nesY, 		#0x0
	mov nesA, 		#0x0
	mov nesTick, 	#0x0
	mov nesEA,		#0x0
	mov nesStack,	#0x0
	mov nesF,		#0x0

	ldr nesPC, =(memory+0xFFFD)
	ldr r0,    =(memory+0xFFFC) @ r0   = memory[0xFFFE]
	ldrh nesPC, [nesPC]
	ldrh r0, [r0]

	and r0, r0, #0xFF00
	orr nesPC, nesPC, r0

	SAVE_6502
	ldmia sp!, {r0-r12, pc}



@ --------------------
@ - Instruction Set --
@ --------------------

@ ------------------------- BRK --------------------------------

brk:
	mov r1, lr
	
	add nesPC, #0x1
	@ writeMemory
	sub nesStack, #0x1
	@ writeMemory
	sub nesStack, #0x1
	orr nesF, #sInterruptFlag
	@ writeMemory
	sub nesStack, #0x1
	orr nesF, #interruptFlag
	ldr nesPC, =(memory+0xFFFF)
	ldr r0,    =(memory+0xFFFE) @ r0   = memory[0xFFFE]
	ldrh nesPC, [nesPC]
	ldrh r0, [r0]
	and r0, r0, #0xFF00
	orr nesPC, nesPC, r0

	add nesTick, #0x7
	mov pc, r1 

@ ---------------------- ORA INDX ------------------------------

ora_indx: @ TODO: penalty_op
	mov r12, lr

	INDX
	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, #0x80
	biceq nesF, nesF, #signFlag 

	add nesTick, #0x6

	mov pc, r12


@ --------------------- ORA ZP ---------------------------------

ora_zp:
	mov r12, lr
	ZP

	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, #signFlag
	biceq nesF, nesF, #signFlag 


	add nesTick, #0x3

	mov pc, r12


@ -------------------- ASL ZP ---------------------------------

asl_zp: 
	mov r12, lr
	ZP

	mov r0, nesEA
	bl memoryRead

	tst r0, #signFlag
	orrne nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, lsl #1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, #signFlag

	tst r0, #signFlag
	orrne nesF, #signFlag
	biceq nesF, nesF, #signFlag


	add nesTick, #0x5

	mov pc, r12


@ -------------------  PHP -------------------------------------

php:
	mov r12, lr

	add r0, nesStack, #0x100
	orr r1, nesF, #sInterruptFlag
	bl writeMemory
	sub nesStack, #0x1

	add nesTick, #0x3

	mov pc, r12


@ ------------------ ORA IMM ----------------------------------

ora_imm: @ TODO: penalty_op
	mov r12, lr
	IMM 

	mov r0, nesEA
	bl readMemory
	orr nesA, nesA, r0

	add nesTick, #0x2

	mov pc, r12


@ ----------------- ASL_A   ----------------------------------

asl_a:
	mov r12, lr

	tst nesA, #0x80
	orrne nesP, #carryFlag
	biceq nesP, nesP, #carryFlag

	mov nesA, nesA, lsl #0x1 

	cmp nesA, #0x0
	bicne nesP, nesP, #zeroFlag
	orreq nesP, #zeroFlag

	tst nesA, #0x80
	orrne nesP, #signFlag
	biceq nesP, nesP, #signFlag

	mov pc, r12


@ ---------------- ORA ABSO -----------------------------
ora_abso: @ TODO: penalty_op
	mov r12, lr
	ABSO

	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, #0x80
	biceq nesF, nesF, #signFlag 

	add nesTick, #0x6

	mov pc, r12

@ ---------------- asl_abso ------------------------------
asl_abso:
	mov r12, lr

	ABSO

	mov r0, nesEA
	bl memoryRead

	tst r0, #signFlag
	orrne nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, lsl #1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, #signFlag

	tst r0, #signFlag
	orrne nesF, #signFlag
	biceq nesF, nesF, #signFlag


	add nesTick, #0x6

	mov pc, r12

@ -------------- BPL ----------------------------------

bpl:


@ --------------- ORA INDY ----------------------------

ora_indy:


@ -------------- ORA ZPX ------------------------------

ora_zpx:


@ -------------- ASL ZPX ------------------------------

asl_zpx:


@ ------------- CLC ---------------------------------

clc:

@ ------------- ORA_ABSY -----------------------------

ora_absy:


@ ---------------- ORA ABSX --------------------------

ora_absx:


@ --------------- ASL ABSX ---------------------------

asl_absx:
