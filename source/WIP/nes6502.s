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


@ ------------------------------
@ - Memory Read/ Memory Write --
@ ------------------------------
.global memoryRead
memoryRead:
	ldr r2, lr

	cmp 	r0, #0x2000
	movlt 	r12, r0, AND #0x7FF
	ldrlt 	r0, [memory, r12]

	cmp 	r0, #0x7FFF
	ldrgt 	r0, [memory, r0]
	

	

	ldr pc, r2
@ ------------------------------------------------
@ - Addresing Mode, Load and Save Registers etc --
@ ------------------------------------------------

.macro LOAD_6502:
	ldr		r0, =CPU_6502_REG
	ldmia	r0, {r4-r11} 
.end


.macro SAVE_6502:
	ldr		r0, =CPU_6502_REG
	stmia	r0, {r4-r11}
.end

.macro IMM: @ Immediate 
	ldr		nesEA, [memory, nesPC], #2 @ nesEA = memory[nesPC++]
.end


@ TODO: memoryRead in ASM

.macro ZP:  @ zeroPage

.end

.macro ZPX: @ zeroPage X

.end

.macro ZPY: @ zeroPage Y

.end

.macro REL: @ relative for branch ops (8 bit immediate value, sign-extended)
	
	cmp 	nesEA, #0x80
	sublt 	nesEA, #0x100 @ If (nesEA >= 0x80) nesEA -= 0x100
.end

.macro ABSO: @ absolute
	
	add 	nesPC, #0x2
.end

.macro ABSX: @ absolute, X  TODO: addr in cycles

	add 	nesPC, #0x2
.end

.macro ABSXY: @ absolute, Y  TODO: addr in cycles
	
	add 	nesPC, #0x2
.end

.macro IND:  @ indirect
	
	add 	nesPC, #0x2
.end

.macro INDX: @ indirect, x
	

.end


.macro INDY:  @ indirect, y  TODO: addr in cycles
	
.end


@ Interrupt TODO : writeMemory in Assembly


.global IRQ
IRQ: @ maskable interrupt
	stmdb sp!, {r0-r12, lr}
	LOAD_6502 @ Load 6502 Registers

	@ write memory
	sub nesStack, #0x1
	@ write memory
	sub nesStack, #0x1
	orr nesF, sInterruptFlag @ software Interrupt Flag
	@ write memory
	sub nesStack, #0x1
	orr nesF, interruptFlag @ Interrupt
	ldr nesPC, [memory, #0xFFFF] @ nesPC = memory[0xFFFF]
	ldr r0,    [memory, #0xFFFE] @ r0   = memory[0xFFFE]
	orr nesPC, r0, nesPC, ASL #8 @ nesPC = r0 | (nesPC 8 arith shift left);
	add nesTick, #0x7

	SAVE_6502 @ End of operation save 6502 registers
	
	ldmia sp!, {r0-r12, pc}


.global NMI
NMI: @ non-maskable interrupt
	@ write memory
	sub nesStack, #0x1
	@ write memory
	sub nesStack, #0x1
	orr nesF, sInterruptFlag @ software Interrupt flag
	@ write memory
	sub nesStack, #0x1
	orr nesF, interruptFlag @ interrupt
	ldr nesPC, [memory, #0xFFFB] @ nesPC = memory[0xFFFB]
	ldr r0,    [memory, #0xFFFA] @ r0   = memory[0xFFFA]
	orr nesPC, r0, nesPC, ASL #8 @ nesPC = r0 | (nesPC 8 arith shift left);
	add nesTick, #0x7
	SAVE_6502 @ End of operation save 6502 registers
	ldmia sp!, {r0-r12, pc}



@ --------------------
@ - Instruction Set --
@ --------------------

@ ------------------------- BRK --------------------------------

brk:
	add nesPC, #0x1
	@ writeMemory
	sub nesStack, #0x1
	@ writeMemory
	sub nesStack, #0x1
	orr nesF, sInterruptFlag
	@ writeMemory
	sub nesStack, #0x1
	orr nesF, interruptFlag
	ldr nesPC, [memory, #0xFFFF] @ nesPC = memory[0xFFFF]
	ldr r0,    [memory, #0xFFFE] @ r0   = memory[0xFFFE]
	orr nesPC, r0, nesPC, ASL #8 @ nesPC = r0 | (nesPC 8 arith shift left);

@ ---------------------- ORA INDX ------------------------------

ora_indx:




.macro instruFetch:
	ldr r1, [opcodeJumpTable, r0] @ get address 
	bl  r1
.end 

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


@ -----------------------
@ - 6502 Execute/Reset --
@ -----------------------


.global CPU_Execute
CPU_Execute:
	stmdb sp!, {r0-r12, lr}
	LOAD_6502 @ Load All Registers
	b end_execute @ Start CPU_Loop

CPU_Loop:
	ldrb r0, [memory, nesPC], #0x1 @ r0 = memory[nesPC], nesPC++
	instruFetch

end_execute:
	cmp nesTick, TOTAL_CYCLE
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

	ldr nesPC, [memory, #0xFFFD] @ nesPC = memory[0xFFFD]
	ldr r0,    [memory, #0xFFFC] @ r0   = memory[0xFFFC]
	orr nesPC, r0, nesPC, ASL #8 @ nesPC = r0 | (nesPC 8 arith shift left);

	SAVE_6502
	ldmia sp!, {r0-r12, pc}

