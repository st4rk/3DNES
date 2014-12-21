@ -----------------------------------------------------------------
@  3DNES 6502 - Written by St4rk
@  Date: 08/12/2014
@  3DNES é um emulador de Nintendo Entertainment System para 3DS
@  esse é um projeto open-source, você pode modificar e utilizar
@  os arquivos para estudo, desde que mantenha os devidos créditos
@ -----------------------------------------------------------------

.arm

#include "nes6502.inc"

@ --------------------
@ -    Data Region  --
@ --------------------

.data 
.align 4

.global CPU_6502_REG @ 6502 Registers, we will load/save the registers here
CPU_6502_REG:
		.long 0, 0, 0, 0, 0, 0, 0, 0, 0

teste:
		.long 0

.equ TOTAL_CYCLE, 114
@ 
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

.macro ABSY @ absolute, Y  TODO: addr in cycles
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


.macro func_retn
	b end_execute
.endm

.global IRQ
IRQ: @ maskable interrupt
	stmdb sp!, {r0-r12, lr}
	LOAD_6502

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
	.long brk,    ora_indx, ILOP,   ILOP,  ILOP,    ora_zp,  asl_zp,  ILOP, php, ora_imm,  asl_a, ILOP,   ILOP,     ora_abso, asl_abso, ILOP @ 0
	.long bpl,    ora_indy, ILOP,   ILOP,  ILOP,    ora_zpx, asl_zpx, ILOP, clc, ora_absy, ILOP,  ILOP,   ILOP,     ora_absx, asl_absx, ILOP @ 1
	.long jsr,    and_indx, ILOP,   ILOP,  bit_zp,  and_zp,  rol_zp,  ILOP, plp, and_imm,  rol_a, ILOP,   bit_abso, and_abso, rol_abso, ILOP @ 2  
	.long bmi,    and_indy, ILOP,   ILOP,  ILOP,    and_zpx, rol_zpx, ILOP, sec, and_absy, ILOP,  ILOP,   ILOP,     and_absx, rol_absx, ILOP @ 3
	.long rti,    eor_indx, ILOP,   ILOP,  ILOP,    eor_zp,  lsr_zp,  ILOP, pha, eor_imm,  lsr_a, ILOP,   jmp_abso, eor_abso, lsr_abso, ILOP @ 4
	.long bvc,    eor_indy, ILOP,   ILOP,  ILOP,    eor_zpx, lsr_zpx, ILOP, cli, eor_absy, ILOP,  ILOP,   ILOP,     eor_absx, lsr_absx, ILOP @ 5
	.long rts,    adc_indx, ILOP,   ILOP,  ILOP,    adc_zp,  ror_zp,  ILOP, pla, adc_imm,  ror_a, ILOP,   jmp_ind,  adc_abso, ror_abso, ILOP @ 6
	.long bvs,    adc_indy, ILOP,   ILOP,  ILOP,    adc_zpx, ror_zpx, ILOP, sei, adc_absy, ILOP,  ILOP,   ILOP,     adc_absx, ror_absx, ILOP @ 7
	.long ILOP,   sta_indx, ILOP,   ILOP,  sty_zp,  sta_zp,  stx_zp,  ILOP, dey, ILOP,     txa,   ILOP,   sty_abso, sta_abso, stx_abso, ILOP @ 8
	.long bcc,    sta_indy, ILOP,   ILOP,  sty_zpx, sta_zpx, stx_zpy, ILOP, tya, sta_absy, txs,   ILOP,   ILOP,     sta_absx, ILOP,     ILOP @ 9
	.long ldy_imm,lda_indx, ldx_imm,ILOP,  ldy_zp,  lda_zp,  ldx_zp,  ILOP, tay, lda_imm,  tax,   ILOP,   ldy_abso, lda_abso, ldx_abso, ILOP @ A
	.long bcs,    lda_indy, ILOP,   ILOP,  ldy_zpx, lda_zpx, ldx_zpy, ILOP, clv, lda_absy, tsx,   ILOP,   ldy_absx, lda_absx, ldx_absy, ILOP @ B
	.long cpy_imm,cmp_indx, ILOP,   ILOP,  cpy_zp,  cmp_zp,  dec_zp,  ILOP, iny, cmp_imm,  dex,   ILOP,   cpy_abso, cmp_abso, dec_abso, ILOP @ C
	.long bne,    cmp_indy, ILOP,   ILOP,  ILOP,    cmp_zpx, dec_zpx, ILOP, cld, cmp_absy, ILOP,  ILOP,   ILOP,     cmp_absx, dec_absx, ILOP @ D
	.long cpx_imm,sbc_indx, ILOP,   ILOP,  cpx_zp,  sbc_zp,  inc_zp,  ILOP, inx, sbc_imm,  nop,   sbc_imm,cpx_abso, sbc_abso, inc_abso, ILOP @ E
	.long beq,    sbc_indy, ILOP,   ILOP,  ILOP,    sbc_zpx, inc_zpx, ILOP, sed, sbc_absy, nop,   ILOP,   ILOP,     sbc_absx, inc_absx, ILOP @ F 


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
	ldrb r0, [r1, nesPC] @ opcode number
	add nesPC, nesPC, #0x1 @ r0 = memory[nesPC], nesPC++
	ldr r1, =opcodeJumpTable	@ Instruction Fetch
	mov r2, #0x4
	cmp r0, #0x0
	movne r2, r2, LSL r0 @ r2 = r2 << r3
	ldr r3, [r1, r2]
	mov pc, r3

end_execute:
	add nesTick, nesTick, #0x1
	cmp nesTick, #TOTAL_CYCLE
	blt CPU_Loop @ if (nesTick < TOTAL_CYCLE) GoTo CPU_Loop

	sub nesTick, nesTick, #TOTAL_CYCLE

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
	
	add nesPC, #0x1
	add r0, nesStack, #0x100
	mov r1, nesPC, ASR #8
	bl writeMemory
	sub nesStack, #0x1
	add r0, nesStack, #0x100
	add r1, nesPC, #0xFF
	bl writeMemory
	sub nesStack, #0x1
	orr nesF, #sInterruptFlag
	add r0, nesStack, #0x100
	mov r1, nesF
	bl writeMemory
	sub nesStack, #0x1
	orr nesF, #interruptFlag
	ldr nesPC, =(memory+0xFFFF)
	ldr r0,    =(memory+0xFFFE) @ r0   = memory[0xFFFE]
	ldr nesPC, [nesPC]
	ldr r0, [r0]
	and r0, r0, #0xFF00
	orr nesPC, nesPC, r0

	add nesTick, nesTick, #0x7

	func_retn

@ ---------------------- ORA INDX ------------------------------
.ltorg

ora_indx: @ TODO: penalty_op
	

	INDX
	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag 
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x6

	func_retn


@ --------------------- ORA ZP ---------------------------------

ora_zp:
	
	ZP

	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x3

	func_retn


@ -------------------- ASL ZP ---------------------------------

asl_zp: 
	
	ZP

	mov r0, nesEA
	bl memoryRead

	tst r0, #0x80
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, lsl #1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 


	add nesTick, nesTick, #0x5

	func_retn


@ -------------------  PHP -------------------------------------

php:
	

	add r0, nesStack, #0x100
	orr r1, nesF, #sInterruptFlag
	bl writeMemory
	sub nesStack, #0x1

	add nesTick, nesTick, #0x3

	func_retn


@ ------------------ ORA IMM ----------------------------------

ora_imm: @ TODO: penalty_op
	
	IMM 

	mov r0, nesEA
	bl memoryRead
	orr nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x2

	func_retn


@ ----------------- ASL_A   ----------------------------------

asl_a:
	
	tst nesA, #0x80
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov nesA, nesA, lsl #0x1 

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag 
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x2

	func_retn


@ ---------------- ORA ABSO -----------------------------
ora_abso: @ TODO: penalty_op
	
	ABSO

	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag 
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x6

	func_retn

@ ---------------- asl_abso ------------------------------
asl_abso:
	
	ABSO

	mov r0, nesEA
	bl memoryRead

	tst r0, #0x80
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, lsl #1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x6
	
	func_retn
@ -------------- BPL ----------------------------------

bpl:
	
	REL

	tst nesF, #0x80
	beq bpl_eq
	b  bpl_end

bpl_eq:
	mov r0, nesPC
	add r1, nesPC, nesEA
	and r0, r0, #0xFF00
	and r1, r1, #0xFF00

	cmp r0, r1 @ if ((nesPC & 0xFF00) != (nesPC + nesEA) & 0xFF00))
	addne nesTick, #0x2
	addeq nesTick, #0x1

	add nesPC, nesPC, nesEA

bpl_end:
	add nesTick, nesTick, #0x2

	func_retn



@ --------------- ORA INDY ----------------------------

ora_indy: @ TODO: Penalty_OP
	
	INDY

	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag 
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x5

	func_retn

@ -------------- ORA ZPX ------------------------------

ora_zpx:
	
	ZPX

	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x4

	func_retn


@ -------------- ASL ZPX ------------------------------

asl_zpx:
	

	ZPX

	mov r0, nesEA
	bl memoryRead

	tst r0, #0x80
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	
	mov r0, r0, lsl #1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ ------------- CLC ---------------------------------
clc:
	
	bic nesF, nesF, #0x1
	add nesTick, nesTick, #0x2
	func_retn

@ ------------- ORA_ABSY -----------------------------

ora_absy:
	
	ABSY

	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag 
	orrne nesF, nesF, #signFlag 
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x4

	func_retn

@ ---------------- ORA ABSX --------------------------

ora_absx:
	
	ABSX

	mov r0, nesEA
	bl memoryRead
	orr nesA, r0

	cmp nesA, #0x0 
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag 

	add nesTick, nesTick, #0x4

	func_retn

@ --------------- ASL ABSX ---------------------------

asl_absx:
	
	ABSX

	mov r0, nesEA
	bl memoryRead

	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, lsl #1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x7

	func_retn

@ ---------------------- JSR --------------------------------
jsr:
	
	add r0, nesStack, #0x100
	sub r1, nesPC, #0x1
	mov r1, r1, ASR #8
	bl writeMemory
	sub nesStack, nesStack, #0x1

	add r0, nesStack, #0x100
	sub r1, nesPC, #0x1
	bl writeMemory
	sub nesStack, nesStack, #0x1

	mov nesPC, nesEA

	add nesTick, nesTick, #0x6

	func_retn

@ ---------------------- AND INDX ---------------------------
and_indx:
		
	INDX

	mov r0, nesEA
	bl memoryRead

	and nesA, nesA, r0

	cmp nesEA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesEA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ ---------------------- BIT ZP -----------------------------
bit_zp:
	

	ZP

	mov r0, nesEA
	bl memoryRead

	and r1, r0, nesA

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	tst r0, #overflowFlag
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	add nesTick, nesTick, #0x3

	func_retn

@ ---------------------- AND ZP -----------------------------
and_zp: @ todo penalty_op
	
	ZP

	mov r0, nesEA
	bl memoryRead

	and nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn

@ ---------------------- ROL ZP -----------------------------
rol_zp:
	
	ZP

	mov r0, nesEA
	bl memoryRead

	tst nesF, #0x1
	beq rol_eq

	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSL #0x1
	orr r0, r0, #0x1
	b rol_end

rol_eq:
	
	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSL #0x1

rol_end:
	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn
@ ---------------------- PLP --------------------------------
plp:
	
	add nesStack, nesStack, #0x1
	add r0, nesStack, #0x100
	bl memoryRead
	orr nesF, r0, #0x20

	add nesTick, nesTick, #0x4
	
	func_retn

@ ---------------------- AND IMM ----------------------------
and_imm:
	
	IMM
	
	mov r0, nesEA
	bl memoryRead

	and nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ ---------------------- ROL A ------------------------------
rol_a:
	
	mov r0, nesEA
	bl memoryRead

	tst nesF, #0x1
	beq rol_a_eq

	tst r0, #0x80
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov nesA, nesA, LSL #0x1
	orr nesA, nesA, #0x1
	b rol_a_end

rol_a_eq:
	tst r0, #0x80
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov nesA, nesA, LSL #0x1

rol_a_end:
	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag
	
	add nesTick, nesTick, #0x2

	func_retn

@ ---------------------- BIT ABSO ---------------------------
bit_abso:
	
	ABSO

	mov r0, nesEA
	bl memoryRead

	and r1, r0, nesA

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	tst r0, #overflowFlag
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	add nesTick, nesTick, #0x4

	func_retn

@ ---------------------- AND ABSO ---------------------------
and_abso:
	
	ABSO

	mov r0, nesEA
	bl memoryRead

	and nesA, nesA, r0

	cmp nesEA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesEA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ ---------------------- ROL ABSO ---------------------------
rol_abso:  
	
	ABSO

	mov r0, nesEA
	bl memoryRead

	tst nesF, #0x1
	bne rol_eq_abso

	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSL #0x1
	orr r0, r0, #0x1
	b rol_end_abso

rol_eq_abso:
	
	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSL #0x1

rol_end_abso:
	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ -------------------- BMI ------------------------------
bmi:
	
	REL

	mov r0, nesF
	and r0, r0, #0x80

	cmp r0, #0x80
	beq bmi_eq
	b bmi_end
bmi_eq:

	and r0, nesPC, #0xFF00
	add r1, nesPC, nesEA
	and r1, r1, #0xFF00

	cmp r0, r1
	addne nesTick, #0x2
	addeq nesTick, #0x1
	add nesPC, nesPC, nesEA

bmi_end:
	add nesTick, nesTick, #0x2
	func_retn

@ -------------------- AND INDY -------------------------
and_indy:
	
	INDY

	mov r0, nesEA
	bl memoryRead

	and nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn

@ -------------------- AND ZPX --------------------------
and_zpx:
	
	ZPX

	mov r0, nesEA
	bl memoryRead

	and nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ -------------------- ROL ZPX --------------------------
rol_zpx: 
	
	ZPX

	mov r0, nesEA
	bl memoryRead

	tst nesF, #0x1
	beq rolx_eq

	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSL #0x1
	orr r0, r0, #0x1
	b rolx_end

rolx_eq:
	
	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSL #0x1

rolx_end:
	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ -------------------- SEC ------------------------------
sec:
	
	orr nesF, nesF, #carryFlag
	add nesTick, nesTick, #0x2
	func_retn

@ -------------------- AND ABSY -------------------------
and_absy:
	
	ABSY

	mov r0, nesEA
	bl memoryRead

	and nesA, nesA, r0

	cmp nesEA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesEA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ -------------------- AND ABSX -------------------------
and_absx:
	
	ABSX

	mov r0, nesEA
	bl memoryRead

	and nesA, nesA, r0

	cmp nesEA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesEA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ -------------------- ROL ABSX -------------------------
rol_absx:
	
	ABSX

	mov r0, nesEA
	bl memoryRead

	tst nesF, #0x1
	beq rolx_eq

	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSL #0x1
	orr r0, r0, #0x1
	b rol_xend

rol_xeq:
	
	tst r0, #signFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSL #0x1

rol_xend:
	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x7

	func_retn


@ -------------------- RTI -------------------------
rti:
	
	add nesStack, nesStack, #0x1
	add r0, nesStack, #0x100
	bl memoryRead

	orr nesF, r0, #0x20

	add nesStack, nesStack, #0x1
	add r0, nesStack, #0x100
	bl memoryRead
	add nesStack, nesStack, #0x1

	add r0, nesStack, #0x100
	bl memoryRead

	and r0, r0, #0xFF00
	orr nesPC, nesPC, r0

	add nesTick, nesTick, #0x6

	func_retn

@ -------------------- EOR INDX -------------------------
eor_indx: @ TODO: penalty_op
	
	INDX

	mov r0, nesEA
	bl memoryRead

	eor nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ -------------------- EOR ZP -------------------------
eor_zp: @ TODO: penalty OP
	
	ZP

	mov r0, nesEA
	bl memoryRead

	eor nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	func_retn
@ -------------------- LSR ZP -------------------------
lsr_zp: 
	
	ZP

	mov r0, nesEA
	bl memoryRead

	tst r0, #0x1
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSR #0x1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	func_retn
@ -------------------- PHA -------------------------
pha:
	
	add r0, nesStack, #0x100
	mov r1, nesA
	bl writeMemory
	sub nesStack, nesStack, #0x1

	add nesTick, nesTick, #0x3

	func_retn

@ -------------------- EOR IMM -------------------------
eor_imm: @ TODO: Penalty_OP  
	
	IMM

	mov r0, nesEA
	bl memoryRead

	eor nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ -------------------- LSR A -------------------------
lsr_a:
	
	tst nesA, #0x1
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov nesA, nesA, ASR #0x1

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ -------------------- JMP ABSO -------------------------
jmp_abso: 
	
	ABSO

	mov nesPC, nesEA

	add nesTick, nesTick, #0x3

	func_retn

@ -------------------- EOR ABSO -------------------------
eor_abso: 
	
	ABSO

	mov r0, nesEA
	bl memoryRead

	eor nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ -------------------- LSR ABSO-------------------------
lsr_abso:
	
	ABSO

	mov r0, nesEA
	bl memoryRead

	tst r0, #0x1
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSR #0x1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn
@ --------------------   BVC   -------------------------
bvc:
	
	REL

	tst nesF, #overflowFlag
	beq bvc_fun
	b bvc_end

bvc_fun:
	and r0, nesPC, #0xFF00
	add r1, nesPC, nesEA
	and r1, r1, #0xFF00
	cmp r0, r1
	addne nesTick, nesTick, #0x2
	addeq nesTick, nesTick, #0x1
	add nesPC, nesPC, nesEA

bvc_end:
	add nesTick, nesTick, #0x2
	func_retn

@ -------------------- EOR INDY -------------------------
eor_indy: @ TODO: Penalty_Addr
	
	INDY

	mov r0, nesEA
	bl memoryRead

	eor nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn

@ -------------------- EOR ZPX -------------------------
eor_zpx: 
	
	ZPX

	mov r0, nesEA
	bl memoryRead

	eor nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ -------------------- LSR ZPX -------------------------
lsr_zpx: 
	
	ZPX

	mov r0, nesEA
	bl memoryRead

	tst r0, #0x1
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSR #0x1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn
@ --------------------   CLI   -------------------------
cli:	
	
	bic nesF, nesF, #interruptFlag
	add nesTick, nesTick, #0x2

	func_retn
@ -------------------- EOR ABSY -------------------------
eor_absy:
	
	ABSY

	mov r0, nesEA
	bl memoryRead

	eor nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ -------------------- EOR ABSX -------------------------
eor_absx: 
	
	ABSX

	mov r0, nesEA
	bl memoryRead

	eor nesA, nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ -------------------- LSR ABSX -------------------------
lsr_absx:
	
	ABSX

	mov r0, nesEA
	bl memoryRead

	tst r0, #0x1
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag
	mov r0, r0, LSR #0x1

	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #0x80
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x7

	func_retn

@ ------------------ RTS -------------------------
rts:
	

	add nesStack, nesStack, #0x1
	add r0, nesStack, #0x100
	bl memoryRead
	mov nesPC, r0
	add nesStack, nesStack, #0x1
	add r0, nesStack, #0x100
	bl memoryRead

	and r0, r0, #0xFF00
	orr nesPC, nesPC, r0
	add nesPC, nesPC, #0x1
	add nesTick, nesTick, #0x6

	func_retn

@ ------------------ ADC INDX --------------------
adc_indx: @ TODO PENALTY_OP
	
	INDX
	
	mov r0, nesEA
	bl memoryRead

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ ------------------ ADC ZP ----------------------
adc_zp:
	
	ZP
	
	mov r0, nesEA
	bl memoryRead

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn
@ ------------------ ROR ZP ----------------------
ror_zp:  
	
	ZP

	mov r0, nesEA
	bl memoryRead

	tst nesF, #carryFlag
	bne ror_zp_ne

	tst r0, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, ASR #0x1

	b ror_zp_end
ror_zp_ne:

	tst r0, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, ASR #0x1
	orr r0, r0, #signFlag

ror_zp_end:
	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn

@ ------------------ PLA -------------------------
pla:
	
	add nesStack, nesStack, #0x1
	add r0, nesStack, #0x100
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ ------------------ ADC IMM ---------------------
adc_imm:  
	
	IMM
	
	mov r0, nesEA
	bl memoryRead

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ ------------------ ROR_A -----------------------
ror_a:
	
	tst nesF, #carryFlag
	bne ror_a_ne

	tst nesA, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag	

	mov nesA, nesA, ASR #0x1

	b ror_zp_end
ror_a_ne:

	tst nesA, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov nesA, nesA, ASR #0x1
	orr nesA, nesA, #signFlag

ror_a_end:

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ ------------------ JMP IND ---------------------
jmp_ind: 
	 
	IND

	mov nesPC, nesEA
	add nesTick, nesTick, #0x3

	func_retn

@ ------------------ ADC ABSO --------------------
adc_abso: 
	
	ABSO
	
	mov r0, nesEA
	bl memoryRead

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ ------------------ ROR ABSO --------------------
ror_abso: 
	
	ABSO

	mov r0, nesEA
	bl memoryRead

	tst nesF, #carryFlag
	bne ror_abso_ne

	tst r0, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, ASR #0x1

	b ror_abso_end
ror_abso_ne:

	tst r0, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, ASR #0x1
	orr r0, r0, #signFlag

ror_abso_end:
	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn
@ ----------------- BVS -------------------
bvs:
	
	REL

	mov r0, nesF
	and r0, r0, #overflowFlag

	tst r0, #overflowFlag
	bne bvs_ne
	b bvs_end

bvs_ne:
	mov r0, nesPC
	and r0, r0, #0xFF00
	add r1, nesEA, nesPC
	and r1, r1, #0xFF00

	cmp r1, r0
	addne nesTick, nesTick, #0x2
	addeq nesTick, nesTick, #0x1
	add nesPC, nesPC, nesEA

bvs_end:
	add nesTick, nesTick, #0x2
	func_retn

@ ----------------- ADC INDY --------------
adc_indy:
	
	INDY
	
	mov r0, nesEA
	bl memoryRead

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ----------------- ADC ZPX ---------------
adc_zpx:
	
	ZPX
	
	mov r0, nesEA
	bl memoryRead

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ----------------- ROR ZPX ---------------
ror_zpx:
	
	ABSO

	mov r0, nesEA
	bl memoryRead

	tst nesF, #carryFlag
	bne ror_zpx_ne

	tst r0, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, ASR #0x1

	b ror_abso_end
ror_zpx_ne:

	tst r0, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, ASR #0x1
	orr r0, r0, #signFlag

ror_zpx_end:
	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn
@ ----------------- SEI -------------------
sei: 
	
	orr nesF, nesF, #interruptFlag
	add nesTick, nesTick, #0x2

	func_retn

@ ----------------- ADC ABSY --------------
adc_absy: 
	
	ABSY
	
	mov r0, nesEA
	bl memoryRead

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ----------------- ADC ABSX --------------
adc_absx:
	
	ABSX
	
	mov r0, nesEA
	bl memoryRead

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ----------------- ROR ABSX --------------
ror_absx: @ penalty_addr
	
	ABSX

	mov r0, nesEA
	bl memoryRead

	tst nesF, #carryFlag
	bne ror_absx_ne

	tst r0, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, ASR #0x1

	b ror_abso_end
ror_absx_ne:

	tst r0, #carryFlag
	orrne nesF, nesF, #carryFlag
	biceq nesF, nesF, #carryFlag

	mov r0, r0, ASR #0x1
	orr r0, r0, #signFlag

ror_absx_end:
	mov r1, r0
	mov r0, nesEA
	bl writeMemory

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x7

	func_retn

@ ----------------- STA INDX -------------------
sta_indx:
	
	INDX

	mov r0, nesEA
	mov r1, nesA
	bl writeMemory
	add nesTick, nesTick, #0x6
	func_retn
@ ----------------- STY ZP ---------------------
sty_zp:
	
	ZP

	mov r0, nesEA
	mov r1, nesY
	bl writeMemory
	add nesTick, nesTick, #0x4
	func_retn

@ ----------------- STA ZP ---------------------
sta_zp:
	
	ZP

	mov r0, nesEA
	mov r1, nesA
	bl writeMemory
	add nesTick, nesTick, #0x3
	func_retn
@ ----------------- STX ZP ---------------------
stx_zp:
	
	ZP

	mov r0, nesEA
	mov r1, nesX
	bl writeMemory
	add nesTick, nesTick, #0x3
	func_retn

@ ----------------- DEY ------------------------
dey: 
	
	sub nesY, nesY, #0x1
	cmp nesY, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesY, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag
	add nesTick, nesTick, #0x2

	func_retn

@ ----------------- TXA ------------------------
txa:
	
	mov nesA, nesX

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ ----------------- STY ABSO -------------------
sty_abso:
	
	ABSO

	mov r0, nesEA
	mov r1, nesY
	bl writeMemory

	add nesTick, nesTick, #0x4
	func_retn

@ ----------------- STA ABSO -------------------
sta_abso:
	
	ABSO

	mov r0, nesEA
	mov r1, nesA
	bl writeMemory

	add nesTick, nesTick, #0x4
	func_retn
@ ----------------- STX ABSO -------------------
stx_abso:
	
	ABSO

	mov r0, nesEA
	mov r1, nesX
	bl writeMemory

	add nesTick, nesTick, #0x4

	func_retn

@ ---------------- BCC --------------------------
bcc:
	
	REL


	tst nesF, #carryFlag
	beq bcc_eq
	b bcc_end

bcc_eq:
	mov r0, nesPC
	add r1, nesPC, nesEA
	and r0, #0xFF00
	and r1, #0xFF00

	cmp r0, r1
	addne nesTick, nesTick, #0x2
	addeq nesTick, nesTick, #0x1
	add nesPC, nesPC, nesEA

bcc_end:
	add nesTick, nesTick, #0x2
	func_retn

@ ---------------- STA INDY ---------------------
sta_indy: @ TODO: Penalty_addr
	@push {r0-r3, lr}
	INDY

	mov r0, nesEA
	mov r1, nesA
	bl writeMemory
	add nesTick, nesTick, #0x6

	func_retn
	@pop {r0-r3, pc}

@ ---------------- STY ZPX ----------------------
sty_zpx:
	
	ZPX

	mov r0, nesEA
	mov r1, nesY
	bl writeMemory
	add nesTick, nesTick, #0x4

	func_retn

@ ---------------- STA ZPX ----------------------
sta_zpx:
	
	ZPX

	mov r0, nesEA
	mov r1, nesA
	bl writeMemory

	add nesTick, nesTick, #0x4

	func_retn

@ ---------------- STX ZPX ----------------------
stx_zpy:
	
	ZPY

	mov r0, nesEA
	mov r1, nesX
	bl writeMemory

	add nesTick, nesTick, #0x4

	func_retn

@ ---------------- TYA --------------------------
tya:
	
	mov nesA, nesY

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ --------------- STA ABSY ----------------------
sta_absy:
	
	ABSY

	mov r0, nesEA
	mov r1, nesA
	bl writeMemory
	add nesTick, nesTick, #0x5

	func_retn

@ --------------- TXS ---------------------------
txs:
	
	mov nesStack, nesX
	add nesTick, nesTick, #0x2

	func_retn

@ -------------- STA ABSX -----------------------
sta_absx:
	
	ABSX

	mov r0, nesEA
	mov r1, nesA
	bl writeMemory
	add nesTick, nesTick, #0x5

	func_retn

@ -------------- LDY IMM ------------------------
ldy_imm: @ penalty_op
	
	IMM

	mov r0, nesEA
	bl memoryRead
	mov nesY, r0

	cmp nesY, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesY, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ -------------- LDA INDX -----------------------
lda_indx:
	
	INDX

	mov r0, nesEA
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn
@ -------------- LDX IMM ------------------------
ldx_imm:
	
	IMM

	mov r0, nesEA
	bl memoryRead
	mov nesX, r0

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn
@ -------------- LDY ZP -------------------------
ldy_zp:
	
	ZP

	mov r0, nesEA
	bl memoryRead
	mov nesY, r0

	cmp nesY, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesY, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn
@ -------------- LDA ZP -------------------------
lda_zp:
	
	ZP

	mov r0, nesEA
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn


@ -------------- LDX ZP -------------------------
ldx_zp:
	
	ZP

	mov r0, nesEA
	bl memoryRead
	mov nesX, r0

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn
@ -------------- TAY ----------------------------
tay:
	
	mov nesY, nesA

	cmp nesY, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesY, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ -------------- LDA IMM ------------------------
lda_imm: @ TODO: PENALTY OP
	
	IMM
	mov r0, nesEA
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ -------------- TAX ----------------------------
tax:
	
	mov nesX, nesA

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn
@ -------------- LDY ABSO -----------------------
ldy_abso:
	
	ABSO
	mov r0, nesEA
	bl memoryRead
	mov nesY, r0

	cmp nesY, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesY, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ -------------- LDA ABSO -----------------------
lda_abso:
	
	ABSO
	mov r0, nesEA
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ -------------- LDX ABSO -----------------------
ldx_abso:
	
	ABSO
	mov r0, nesEA
	bl memoryRead
	mov nesX, r0

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ --------------------- BCS ---------------------------
bcs:   
	
	REL
	
	tst nesF, #carryFlag
	bne bcs_ne
	b bcs_end

bcs_ne:
	mov r0, nesPC
	add r1, nesPC, nesEA
	and r0, #0xFF00
	and r1, #0xFF00

	cmp r0, r1
	addne nesTick, nesTick, #0x2
	addeq nesTick, nesTick, #0x1
	add nesPC, nesPC, nesEA

bcs_end:
	add nesTick, nesTick, #0x2
	func_retn

@ --------------------- LDA INDY ----------------------
lda_indy:
	
	INDY
	mov r0, nesEA
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn

@ --------------------- LDA ZPX -----------------------
ldy_zpx:
	
	ZPX
	mov r0, nesEA
	bl memoryRead
	mov nesY, r0

	cmp nesY, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesY, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- LDA ZPX -----------------------
lda_zpx: 
	
	ZPX
	mov r0, nesEA
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ --------------------- LDX ZPY -----------------------
ldx_zpy: 
	
	ZPY
	mov r0, nesEA
	bl memoryRead
	mov nesX, r0

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- CLV ---------------------------
clv:
	
	bic nesF, nesF, #overflowFlag
	add nesTick, nesTick, #0x2
	func_retn

@ --------------------- LDA ABSY ----------------------
lda_absy: @ TODO: penalty_op
	
	ABSY
	mov r0, nesEA
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ --------------------- TSX ---------------------------
tsx:  
	
	mov nesX, nesStack

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn
@ --------------------- LDY ABSX ----------------------
ldy_absx: @ TODO: penalty_op
	
	ABSX
	mov r0, nesEA
	bl memoryRead
	mov nesY, r0

	cmp nesY, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesY, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- LDA ABSX ----------------------
lda_absx: @ TODO: penalty_op
	
	ABSX
	mov r0, nesEA
	bl memoryRead
	mov nesA, r0

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- LDX ABSY ----------------------
ldx_absy: @ TODO: penalty_op
	
	ABSY
	mov r0, nesEA
	bl memoryRead
	mov nesX, r0

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn

@ --------------------- CPY IMM ----------------------
cpy_imm: @ TODO: Penalty_Op
	
	IMM
	mov r0, nesEA
	bl memoryRead
	sub r1, nesY, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn
@ --------------------- CMP INDX ---------------------
cmp_indx:
	
	INDX

	mov r0, nesEA
	bl memoryRead
	sub r1, nesA, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn
@ --------------------- CPY ZP -----------------------
cpy_zp:
	
	ZP
	mov r0, nesEA
	bl memoryRead
	sub r1, nesY, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn
@ --------------------- CMP ZP -----------------------
cmp_zp:
	
	ZP

	mov r0, nesEA
	bl memoryRead
	sub r1, nesA, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn
@ --------------------- DEC ZP -----------------------
dec_zp:
	
	ZP
	mov r0, nesEA
	bl memoryRead
	sub r1, r0, #0x1
	mov r0, nesEA
	bl writeMemory
	bl memoryRead

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn

@ --------------------- INY --------------------------
iny:
	
	add nesY, nesY, #0x1

	cmp nesY, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesY, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ --------------------- CMP IMM ----------------------
cmp_imm: @ TODO: Penalty_op
	
	IMM

	mov r0, nesEA
	bl memoryRead
	sub r1, nesA, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn
@ --------------------- DEX --------------------------
dex:
	
	sub nesX, nesX, #0x1

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn
@ --------------------- CPY ABSO ---------------------
cpy_abso:
	
	ABSO
	mov r0, nesEA
	bl memoryRead
	sub r1, nesY, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- CMP ABSO ---------------------
cmp_abso: @ TODO: Penalty_op
	
	ABSO

	mov r0, nesEA
	bl memoryRead
	sub r1, nesA, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- DEC ABSO ---------------------
dec_abso:
	
	ABSO
	mov r0, nesEA
	bl memoryRead
	sub r1, r0, #0x1
	mov r0, nesEA
	bl writeMemory
	bl memoryRead

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ --------------------- BNE ------------------------
bne:    
	
	REL

	tst nesF, #zeroFlag
	beq bne_eq
	b bne_end

bne_eq:
	and r0, nesPC, #0xFF00
	add r1, nesPC, nesEA
	and r1, r1, #0xFF00

	cmp r1, r0
	addne nesTick, nesTick, #0x2
	addeq nesTick, nesTick, #0x1

	add nesPC, nesPC, nesEA

	bne_end:
	add nesTick, nesTick, #0x2
	func_retn

@ --------------------- CMP INDY -------------------
cmp_indy: 
	
	INDY

	mov r0, nesEA
	bl memoryRead
	sub r1, nesA, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn
@ --------------------- CMP ZPX --------------------
cmp_zpx: @ TODO: Penalty_op
	
	ZPX

	mov r0, nesEA
	bl memoryRead
	sub r1, nesA, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- DEC ZPX --------------------
dec_zpx: 
	
	ZPX
	mov r0, nesEA
	bl memoryRead
	sub r1, r0, #0x1
	mov r0, nesEA
	bl writeMemory
	bl memoryRead

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn
@ --------------------- CLD ------------------------
cld:
	
	bic nesF, nesF, #decimalFlag
	add nesTick, nesTick, #0x2
	func_retn

@ --------------------- CMP ABSY -------------------
cmp_absy: @ penalty_addr
	
	ABSY

	mov r0, nesEA
	bl memoryRead
	sub r1, nesA, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- CMP ABSX -------------------
cmp_absx: @ penalty_addr
	
	ABSX

	mov r0, nesEA
	bl memoryRead
	sub r1, nesA, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ --------------------- DEC ABSX -------------------
dec_absx: 
	
	ABSX
	mov r0, nesEA
	bl memoryRead
	sub r1, r0, #0x1
	mov r0, nesEA
	bl writeMemory
	bl memoryRead

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x7

	func_retn

@ ------------------- CPX IMM ---------------------
cpx_imm:
	
	IMM

	mov r0, nesEA
	bl memoryRead
	sub r1, nesX, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn

@ ------------------- SBC INDX --------------------
sbc_indx: 
	
	IMM
	
	mov r0, nesEA
	bl memoryRead
	eor r0, r0, #0xFF

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ ------------------- CPX ZP ----------------------
cpx_zp:  
	
	ZP

	mov r0, nesEA
	bl memoryRead
	sub r1, nesX, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn

@ ------------------- SBC ZP ----------------------
sbc_zp:  
	
	ZP
	
	mov r0, nesEA
	bl memoryRead
	eor r0, r0, #0xFF

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x3

	func_retn
@ ------------------- INC ZP ----------------------
inc_zp: 
	
	ZP

	mov r0, nesEA
	bl memoryRead
	add r1, r0, #0x1
	mov r0, nesEA
	bl writeMemory
	bl memoryRead

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn

@ ------------------- INDX ------------------------
inx:
	
	add nesX, nesX, #0x1

	cmp nesX, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesX, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2
	func_retn

@ ------------------- SBC IMM ---------------------
sbc_imm:  
	
	IMM
	
	mov r0, nesEA
	bl memoryRead
	eor r0, r0, #0xFF

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x2

	func_retn
@ ------------------- NOP -------------------------
nop:
	
	add nesTick, nesTick, #0x2
	func_retn

@ ------------------- CPX ABSO --------------------
cpx_abso: 
	
	ABSO

	mov r0, nesEA
	bl memoryRead
	sub r1, nesX, r0

	tst r1, #0x8000
	orreq nesF, nesF, #carryFlag
	bicne nesF, nesF, #carryFlag

	cmp r1, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r1, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ------------------- SBC ABSO --------------------
sbc_abso: 
	
	ABSO
	
	mov r0, nesEA
	bl memoryRead
	eor r0, r0, #0xFF

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ------------------- INC ABSO --------------------
inc_abso:
	
	ABSO

	mov r0, nesEA
	bl memoryRead
	add r1, r0, #0x1
	mov r0, nesEA
	bl writeMemory
	bl memoryRead

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ ------------------- BEQ -------------------------
beq:  
	
	REL

	tst nesF, #zeroFlag
	bne beq_ne
	b beq_end

beq_ne:
	and r0, nesPC, #0xFF00
	add r1, nesPC, nesEA
	and r1, r1, #0xFF00

	cmp r1, r0
	addne nesTick, nesTick, #0x2
	addeq nesTick, nesTick, #0x1

	add nesPC, nesPC, nesEA

	beq_end:
	add nesTick, nesTick, #0x2
	func_retn
@ ------------------- SBC INDY --------------------
sbc_indy: 
	
	INDY
	
	mov r0, nesEA
	bl memoryRead
	eor r0, r0, #0xFF

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x5

	func_retn
@ ------------------- SBC ZPX ---------------------
sbc_zpx:
	
	ZPX
	
	mov r0, nesEA
	bl memoryRead
	eor r0, r0, #0xFF

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ------------------- INC ZPX ---------------------
inc_zpx:
	
	ZPX

	mov r0, nesEA
	bl memoryRead
	add r1, r0, #0x1
	mov r0, nesEA
	bl writeMemory
	bl memoryRead

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x6

	func_retn

@ ------------------- SED -------------------------
sed:
	
	orr nesF, nesF, #decimalFlag
	add nesTick, nesTick, #0x2

	func_retn

@ ------------------- SBC ABSY --------------------
sbc_absy:
	
	ABSY
	
	mov r0, nesEA
	bl memoryRead
	eor r0, r0, #0xFF

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ------------------- SBC ABSX --------------------
sbc_absx: 
	
	ABSX
	
	mov r0, nesEA
	bl memoryRead
	eor r0, r0, #0xFF

	and r1, nesF, #carryFlag
	add r2, nesA, r0
	add r2, r2, r1

	cmp r2, #0xFF
	orrgt nesF, nesF, #carryFlag
	bicle nesF, nesF, #carryFlag

	eor r3, nesA, r0
	mvn r3, r3

	eor r4, nesA, r2
	and r4, r4, #0x80

	tst r3, r4
	orrne nesF, nesF, #overflowFlag
	biceq nesF, nesF, #overflowFlag

	mov nesA, r2

	cmp nesA, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst nesA, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x4

	func_retn
@ ------------------- INC ABSX --------------------
inc_absx:
	
	ABSX

	mov r0, nesEA
	bl memoryRead
	add r1, r0, #0x1
	mov r0, nesEA
	bl writeMemory
	bl memoryRead

	cmp r0, #0x0
	bicne nesF, nesF, #zeroFlag
	orreq nesF, nesF, #zeroFlag

	tst r0, #signFlag
	orrne nesF, nesF, #signFlag
	biceq nesF, nesF, #signFlag

	add nesTick, nesTick, #0x7

	func_retn


@ ----------------- ILEGAL OPCODE ---------------------

ILOP:
	
	func_retn