/*
@ -----------------------------------------------------------------
@  3DNES 6502core - Written by gdkchan and St4rk
@  Date: 27/09/2014
@  3DNES é um emulador de Nintendo Entertainment System para 3DS
@  esse é um projeto open-source, você pode modificar e utilizar
@  os arquivos para estudo, desde que mantenha os devidos créditos
@ -----------------------------------------------------------------
*/


#include <stdlib.h>
#include <stdbool.h>

#include "nesMemory.h"
#include "nesGlobal.h"
#include "6502core.h"

bool penalty_op, penalty_addr;

unsigned char memory[65536];

/* Instruções */

void adc_imm() {
	EA = PC++;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void adc_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void adc_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void adc_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void adc_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void adc_absy() {
	penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void adc_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void adc_indy() {
 	penalty_addr = true;
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void and_imm() {
	EA = PC++;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void and_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void and_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void and_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void and_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void and_absy() {
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void and_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void and_indy() {
    penalty_addr = true;
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A &= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}


void asl_a() {
    if (A & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	A <<= 1;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void asl_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	data <<= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void asl_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	unsigned char data = memoryRead(EA);
	if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	data <<= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void asl_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	data <<= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void asl_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
	data <<= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void bcc() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if (!(P & 0x1))
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void bcs() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if ((P & 0x1) == 0x1)
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void beq() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if ((P & 0x2) == 0x2)
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void bit_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (!(data & A)) {P |= 0x2;} else {P &= ~0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	if (data & 0x40) {P |= 0x40;} else {P &= ~0x40;}
	tick_count += 3;
}

void bit_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (!(data & A)) {P |= 0x2;} else {P &= ~0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	if (data & 0x40) {P |= 0x40;} else {P &= ~0x40;}
	tick_count += 4;
}

void bmi() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if ((P & 0x80) == 0x80)
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void bne() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if (!(P & 0x2))
	{
        if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	
	tick_count += 2;
}

void bpl() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if (!(P & 0x80))
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void brk() {

	PC ++;
    writeMemory(S + 0x100, PC >> 8);
	S = (S - 1) & 0xFF;
    writeMemory(S + 0x100, PC & 0xFF);
	S = (S - 1) & 0xFF;
	P |= 0x10;
    writeMemory(S + 0x100, P);
	S = (S - 1) & 0xFF;
	P |= 0x4;
	PC = memory[0xFFFE] + (memory[0xFFFF] * 0x100);
	tick_count += 7;
}

void bvc() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if (!(P & 0x40))
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void bvs() {
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}

	if ((P & 0x40) == 0x40)
	{
	    if ((PC & 0xFF00) != ((PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		PC += EA;
	}
	tick_count += 2;
}

void clc() { P &=~0x1;  tick_count += 2; }
void cld() { P &=~0x8;  tick_count += 2; }
void cli() { P &=~0x4;  tick_count += 2; }
void clv() { P &=~0x40; tick_count += 2; }


void cmp_imm() {
	EA = PC++;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}


void cmp_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void cmp_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cmp_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cmp_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cmp_absy() {
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	int temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cmp_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void cmp_indy() {
    penalty_addr = true;
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	temp = A - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void cpx_imm() {
	EA = PC++;
	unsigned char data = memoryRead(EA);
	int temp = X - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void cpx_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	int temp = X - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void cpx_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	int temp = X - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void cpy_imm() {
	EA = PC++;
	unsigned char data = memoryRead(EA);
	int temp = Y - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}


void cpy_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	int temp = Y - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void cpy_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	int temp = Y - data;
	if (!(temp & 0x8000)) {P |= 0x1;} else {P &= ~0x1;}
	if (temp) {P &= ~0x2;} else {P |= 0x2;}
	if (temp & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void dec_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, (memoryRead(EA) - 1) & 0xFF);
	unsigned char data = memoryRead(EA);
    if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void dec_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	writeMemory(EA, (memoryRead(EA) - 1) & 0xFF);
	unsigned char data = memoryRead(EA);
    if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void dec_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	writeMemory(EA, (memoryRead(EA) - 1) & 0xFF);
	unsigned char data = memoryRead(EA);
    if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void dec_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	writeMemory(EA, (memoryRead(EA) - 1) & 0xFF);
	unsigned char data = memoryRead(EA);
    if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void dex() {
	X = (X - 1) & 0xFF;
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void dey() {
	Y = (Y - 1) & 0xFF;
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void eor_imm() {
	EA = PC++;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void eor_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void eor_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void eor_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void eor_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void eor_absy() {
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void eor_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void eor_indy() {
    penalty_addr = true;
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A ^= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void inc_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, (memoryRead(EA) + 1) & 0xFF);
	unsigned char data = memoryRead(EA);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void inc_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	writeMemory(EA, (memoryRead(EA) + 1) & 0xFF);
	unsigned char data = memoryRead(EA);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void inc_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;

	writeMemory(EA, (memoryRead(EA) + 1) & 0xFF);
	unsigned char data = memoryRead(EA);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void inc_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;

	writeMemory(EA, (memoryRead(EA) + 1) & 0xFF);
	unsigned char data = memoryRead(EA);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void inx() {
	X = (X + 1) & 0xFF;
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void iny() {
	Y = (Y + 1) & 0xFF;
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void jmp_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	PC = EA; tick_count += 3;
}

void jmp_ind() {
	int temp = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	EA = memoryRead(temp) + (memoryRead(data) * 0x100);
	PC = EA; tick_count += 5;
}

void jsr() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;

    writeMemory(S + 0x100, (PC - 1) >> 8);
	S = (S - 1) & 0xFF;
    writeMemory(S + 0x100, (PC - 1) & 0xFF);
	S = (S - 1) & 0xFF;
	PC = EA;
	tick_count += 6;
}

void lda_imm() {
	EA = PC++;
    penalty_op = true;
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}


void lda_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
    
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void lda_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    penalty_op = true;
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void lda_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void lda_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    penalty_op = true;
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void lda_absy() {
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    penalty_op = true;
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void lda_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    penalty_op = true;
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void lda_indy() {
    penalty_addr = true;
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    penalty_op = true;
	A = memoryRead(EA);
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void ldx_imm() {
	EA = PC++;
    penalty_op = true;
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void ldx_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void ldx_zpy() {
	EA = (memoryRead(PC++) + Y) & 0xFF;
    penalty_op = true;
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldx_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldx_absy() {
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    penalty_op = true;
	X = memoryRead(EA);
	if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldy_imm() {
	EA = PC++;
    penalty_op = true;
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void ldy_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void ldy_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    penalty_op = true;
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldy_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void ldy_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    penalty_op = true;
	Y = memoryRead(EA);
	if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void lsr_a() {
	if (A & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	A >>= 1;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void lsr_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	data >>= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void lsr_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	unsigned char data = memoryRead(EA);
	if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	data >>= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void lsr_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	data >>= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void lsr_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
	data >>= 1;
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void nop() { tick_count += 2; }


void ora_imm() {
	EA = PC++;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}


void ora_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}


void ora_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void ora_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void ora_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void ora_absy() {
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}


void ora_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void ora_indy() {
    penalty_addr = true;
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    penalty_op = true;
	unsigned char data = memoryRead(EA);
	A |= data;
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void pha() {
    writeMemory(S + 0x100, A);
	S = (S - 1) & 0xFF;
	tick_count += 3;
}

void php() {
    writeMemory(S + 0x100, P | 0x10);
	S = (S - 1) & 0xFF;
	tick_count += 3;
}

void pla() {
    S = (S + 1) & 0xFF;
	A = memoryRead(S + 0x100);
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void plp() {
    S = (S + 1) & 0xFF;
	P = memoryRead(S + 0x100) | 0x20;
	tick_count += 4;
}

void rol_a() {
	if (P & 0x1)
	{
		if (A & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		A = (A << 1) | 0x1;
	}
	else
	{
		if (A & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		A <<= 1;
	}
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void rol_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data <<= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void rol_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data <<= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void rol_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data <<= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void rol_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {P |= 0x1;} else {P &= ~0x1;}
		data <<= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;	
}

void ror_a() {
	if (P & 0x1)
	{
	    if (A & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		A = (A >> 1) | 0x80;
	}
	else
	{
	    if (A & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		A >>= 1;
	}
	if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void ror_zp() {
	EA = memoryRead(PC++);
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
		if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data >>= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}


void ror_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
		if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data >>= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void ror_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
		if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data >>= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}


void ror_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	unsigned char data = memoryRead(EA);
	if (P & 0x1)
	{
		if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {P |= 0x1;} else {P &= ~0x1;}
		data >>= 1;
	}
	writeMemory(EA, data);
	if (data) {P &= ~0x2;} else {P |= 0x2;}
	if (data & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 7;
}

void rti() {
    S = (S + 1) & 0xFF;
	P = memoryRead(S + 0x100) | 0x20;
	S = (S + 1) & 0xFF;
	PC = memoryRead(S + 0x100);
	S = (S + 1) & 0xFF;
	PC |= (memoryRead(S + 0x100) * 0x100);
	tick_count += 6;
}

void rts() {
    S = (S + 1) & 0xFF;
	PC = memoryRead(S + 0x100);
	S = (S + 1) & 0xFF;
	PC |= (memoryRead(S + 0x100) * 0x100);
	PC++;
	tick_count += 6;
}

void sbc_imm() {
	EA = PC++;
    penalty_op = true;
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void sbc_zp() {
	EA = memoryRead(PC++);
    penalty_op = true;
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 3;
}

void sbc_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
    penalty_op = true;
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void sbc_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void sbc_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void sbc_absy() {
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
    penalty_op = true;
	unsigned char data = memoryRead(EA) ^ 0xFF;
	int temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 4;
}

void sbc_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
    penalty_op = true;
	unsigned char data = memoryRead(EA) ^ 0xFF;
	temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 6;
}

void sbc_indy() {
    penalty_addr = true;
	int temp = memoryRead(PC++);
	int data2 = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data2 = memoryRead(temp) + (memoryRead(data2) * 0x100);
	EA = data2 + Y;
    penalty_op = true;
	unsigned char data = memoryRead(EA) ^ 0xFF;
	temp = A + data + (P & 0x1);
	if (temp > 0xFF) {P |= 0x1;} else {P &= ~0x1;}
	if ((~(A ^ data)) & (A ^ temp) & 0x80) {P |= 0x40;} else {P &= ~0x40;}
	A = temp & 0xFF;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 5;
}

void sec() {
	P |= 0x1;
	tick_count += 2; 
}

void sed() {
	P |= 0x8;
	tick_count += 2; 
}

void sei() {
	P |= 0x4;
	tick_count += 2; 
}

void sta_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, A);
	tick_count += 3;
}

void sta_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	writeMemory(EA, A);
	tick_count += 4;
}


void sta_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	writeMemory(EA, A);
	tick_count += 4;
}


void sta_absx() {
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
	writeMemory(EA, A);
	tick_count += 5;
}


void sta_absy() {
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
	writeMemory(EA, A);
	tick_count += 5;
}

void sta_indx() {
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
	writeMemory(EA, A);
	tick_count += 6;
}

void sta_indy() {
    penalty_addr = true;
	int temp = memoryRead(PC++);
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data = memoryRead(temp) + (memoryRead(data) * 0x100);
	EA = data + Y;
	writeMemory(EA, A);
	tick_count += 6;
}


void stx_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, X);
	tick_count += 3;
}

void stx_zpy() {
	EA = (memoryRead(PC++) + Y) & 0xFF;
	writeMemory(EA, X);
	tick_count += 4;
}

void stx_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	writeMemory(EA, X);
	tick_count += 4;
}

void sty_zp() {
	EA = memoryRead(PC++);
	writeMemory(EA, Y);
	tick_count += 3;
}

void sty_zpx() {
	EA = (memoryRead(PC++) + X) & 0xFF;
	writeMemory(EA, Y);
	tick_count += 4;
}

void sty_abso() {
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	writeMemory(EA, Y);
	tick_count += 4;
}

void tax() {
	X = A;
    if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void tay() {
	Y = A;
    if (Y) {P &= ~0x2;} else {P |= 0x2;}
	if (Y & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void tsx() {
	X = S;
    if (X) {P &= ~0x2;} else {P |= 0x2;}
	if (X & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void txa() {
	A = X;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}

void txs() {
	S = X;
	tick_count += 2;
}

void tya() {
	A = Y;
    if (A) {P &= ~0x2;} else {P |= 0x2;}
	if (A & 0x80) {P |= 0x80;} else {P &= ~0x80;}
	tick_count += 2;
}



/* Modos de endereçamento */

void imm() { //immediate
	EA = PC++;
}

void zp() { //zero-page
	EA = memoryRead(PC++);
}

void zpx() { //zero-page,X
	EA = (memoryRead(PC++) + X) & 0xFF;
}

void zpy() { //zero-page,Y
	EA = (memoryRead(PC++) + Y) & 0xFF;
}

void rel() { //relative for branch ops (8-bit immediate value, sign-extended)
	EA = memoryRead(PC++);
	if (EA >= 0x80) {EA -= 0x100;}
}

void abso() { //absolute
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
}

void absx() { //absolute,X
    penalty_addr = true;
	EA =memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + X;
	PC += 2;
}

void absy() { //absolute,Y
    penalty_addr = true;
	EA = memoryRead(PC) + (memoryRead(PC + 1) * 0x100) + Y;
	PC += 2;
}

void ind() { //indirect
	int temp = memoryRead(PC) + (memoryRead(PC + 1) * 0x100);
	PC += 2;
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	EA = memoryRead(temp) + (memoryRead(data) * 0x100);
}

void indx() { //(indirect,X)
    int temp = memoryRead(PC++) + X;
	EA = memoryRead(temp & 0xFF) + (memoryRead((temp + 1) & 0xFF) * 0x100);
}

void indy() { //(indirect),Y
    penalty_addr = true;
	int temp = memoryRead(PC++);
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data = memoryRead(temp) + (memoryRead(data) * 0x100);
	EA = data + Y;
}

/* Interrupts */

//Nota: Break está junto com as instruções

void IRQ() {
	//Maskable Interrupt
    writeMemory(S + 0x100, PC >> 8);
	S = (S - 1) & 0xFF;
	writeMemory(S + 0x100, PC & 0xFF);
	S = (S - 1) & 0xFF;
	P |= 0x10;
	writeMemory(S + 0x100, P);
	S = (S - 1) & 0xFF;
	P |= 0x4;
	PC = memory[0xFFFE] + (memory[0xFFFF] * 0x100);
	tick_count += 7;
}


void NMI() {
	//Non-Maskable Interrupt
    writeMemory(S + 0x100, PC >> 8);
	S = (S - 1) & 0xFF;
	writeMemory(S + 0x100, PC & 0xFF);
	S = (S - 1) & 0xFF;
	P |= 0x10;
    writeMemory(S + 0x100, P);
	S = (S - 1) & 0xFF;
	P |= 0x4;
	PC = memory[0xFFFA] + (memory[0xFFFB] * 0x100);
	tick_count += 7;
}


void (*op[0x100])() = {
																/* Set Function Pointer Vector */
	/*       0          1      2       3      4         5        6       7     8      9        A     B       C           D         E       F    */
			brk,    ora_indx, NULL,   NULL,  NULL,    ora_zp,  asl_zp,  NULL, php, ora_imm,  asl_a, NULL,   NULL,     ora_abso, asl_abso, NULL, // 0
			bpl,    ora_indy, NULL,   NULL,  NULL,    ora_zpx, asl_zpx, NULL, clc, ora_absy, NULL,  NULL,   NULL,     ora_absx, asl_absx, NULL, // 1
			jsr,    and_indx, NULL,   NULL,  bit_zp,  and_zp,  rol_zp,  NULL, plp, and_imm,  rol_a, NULL,   bit_abso, and_abso, rol_abso, NULL, // 2  
			bmi,    and_indy, NULL,   NULL,  NULL,    and_zpx, rol_zpx, NULL, sec, and_absy, NULL,  NULL,   NULL,     and_absx, rol_absx, NULL, // 3
		    rti,    eor_indx, NULL,   NULL,  NULL,    eor_zp,  lsr_zp,  NULL, pha, eor_imm,  lsr_a, NULL,   jmp_abso, eor_abso, lsr_abso, NULL, // 4
			bvc,    eor_indy, NULL,   NULL,  NULL,    eor_zpx, lsr_zpx, NULL, cli, eor_absy, NULL,  NULL,   NULL,     eor_absx, lsr_absx, NULL, // 5
			rts,    adc_indx, NULL,   NULL,  NULL,    adc_zp,  ror_zp,  NULL, pla, adc_imm,  ror_a, NULL,   jmp_ind,  adc_abso, ror_abso, NULL, // 6
			bvs,    adc_indy, NULL,   NULL,  NULL,    adc_zpx, ror_zpx, NULL, sei, adc_absy, NULL,  NULL,   NULL,     adc_absx, ror_absx, NULL, // 7
			NULL,   sta_indx, NULL,   NULL,  sty_zp,  sta_zp,  stx_zp,  NULL, dey, NULL,     txa,   NULL,   sty_abso, sta_abso, stx_abso, NULL, // 8
			bcc,    sta_indy, NULL,   NULL,  sty_zpx, sta_zpx, stx_zpy, NULL, tya, sta_absy, txs,   NULL,   NULL,     sta_absx, NULL,     NULL, // 9
			ldy_imm,lda_indx, ldx_imm,NULL,  ldy_zp,  lda_zp,  ldx_zp,  NULL, tay, lda_imm,  tax,   NULL,   ldy_abso, lda_abso, ldx_abso, NULL, // A
			bcs,    lda_indy, NULL,   NULL,  ldy_zpx, lda_zpx, ldx_zpy, NULL, clv, lda_absy, tsx,   NULL,   ldy_absx, lda_absx, ldx_absy, NULL, // B
			cpy_imm,cmp_indx, NULL,   NULL,  cpy_zp,  cmp_zp,  dec_zp,  NULL, iny, cmp_imm,  dex,   NULL,   cpy_abso, cmp_abso, dec_abso, NULL, // C
			bne,    cmp_indy, NULL,   NULL,  NULL,    cmp_zpx, dec_zpx, NULL, cld, cmp_absy, NULL,  NULL,   NULL,     cmp_absx, dec_absx, NULL, // D
			cpx_imm,sbc_indx, NULL,   NULL,  cpx_zp,  sbc_zp,  inc_zp,  NULL, inx, sbc_imm,  nop,   sbc_imm,cpx_abso, sbc_abso, inc_abso, NULL, // E
			beq,    sbc_indy, NULL,   NULL,  NULL,    sbc_zpx, inc_zpx, NULL, sed, sbc_absy, nop,   NULL,   NULL,     sbc_absx, inc_absx, NULL};// F 


/* Emulação da CPU (Reset/Inicia) */

void CPU_reset() {
    A = 0;
    X = 0;
    Y = 0;
    S = 0xFF;
    P = 0x20;
	PC = memory[0xFFFC] + (memory[0xFFFD] * 0x100);

}

void CPU_execute(int cycles) {
	unsigned char opcode;
	
	while(tick_count < cycles) {
        penalty_op = false; penalty_addr = false;
		opcode=memory[PC++];

		(*op[opcode])();


		if (penalty_op && penalty_addr) tick_count++;
	}

    tick_count -= cycles;
}
