/* Emulador da CPU 6502 por gdkchan - 2014 */

#include <stdlib.h>
#include <stdbool.h>

#include "3dnes.h"
#include "6502core.h"

bool penalty_op, penalty_addr;

unsigned char memory[65536];

/* Instruções */

void adc() {
    penalty_op = true;
	unsigned char data = memory_read(EA);
	int temp = R.A + data + (R.P & 0x1);
	if (temp > 0xFF) {R.P |= 0x1;} else {R.P &= ~0x1;}
	if ((~(R.A ^ data)) & (R.A ^ temp) & 0x80) {R.P |= 0x40;} else {R.P &= ~0x40;}
	R.A = temp & 0xFF;
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}
void and() {
    penalty_op = true;
	unsigned char data = memory_read(EA);
	R.A &= data;
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void asl_a() {
    if (R.A & 0x80) {R.P |= 0x1;} else {R.P &= ~0x1;}
	R.A <<= 1;
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void asl() {
	unsigned char data = memory_read(EA);
	if (data & 0x80) {R.P |= 0x1;} else {R.P &= ~0x1;}
	data <<= 1;
	write_memory(EA, data);
	if (data) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (data & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void bcc() {
	if (!(R.P & 0x1))
	{
	    if ((R.PC & 0xFF00) != ((R.PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		R.PC += EA;
	}
}

void bcs() {
	if ((R.P & 0x1) == 0x1)
	{
	    if ((R.PC & 0xFF00) != ((R.PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		R.PC += EA;
	}
}

void beq() {
	if ((R.P & 0x2) == 0x2)
	{
	    if ((R.PC & 0xFF00) != ((R.PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		R.PC += EA;
	}
}

void bit() {
	unsigned char data = memory_read(EA);
	if (!(data & R.A)) {R.P |= 0x2;} else {R.P &= ~0x2;}
	if (data & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
	if (data & 0x40) {R.P |= 0x40;} else {R.P &= ~0x40;}
}

void bmi() {
	if ((R.P & 0x80) == 0x80)
	{
	    if ((R.PC & 0xFF00) != ((R.PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		R.PC += EA;
	}
}

void bne() {
	if (!(R.P & 0x2))
	{
        if ((R.PC & 0xFF00) != ((R.PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		R.PC += EA;
	}
}

void bpl() {
	if (!(R.P & 0x80))
	{
	    if ((R.PC & 0xFF00) != ((R.PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		R.PC += EA;
	}
}

void brk() {
	R.PC ++;
    write_memory(R.S + 0x100, R.PC >> 8);
	R.S = (R.S - 1) & 0xFF;
    write_memory(R.S + 0x100, R.PC & 0xFF);
	R.S = (R.S - 1) & 0xFF;
	R.P |= 0x10;
    write_memory(R.S + 0x100, R.P);
	R.S = (R.S - 1) & 0xFF;
	R.P |= 0x4;
	R.PC = memory[0xFFFE] + (memory[0xFFFF] * 0x100);
}

void bvc() {
	if (!(R.P & 0x40))
	{
	    if ((R.PC & 0xFF00) != ((R.PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		R.PC += EA;
	}
}

void bvs() {
	if ((R.P & 0x40) == 0x40)
	{
	    if ((R.PC & 0xFF00) != ((R.PC + EA) & 0xFF00)) {tick_count += 2;} else {tick_count += 1;}
		R.PC += EA;
	}
}

void cmp() {
    penalty_op = true;
	unsigned char data = memory_read(EA);
	int temp = R.A - data;
	if (!(temp & 0x8000)) {R.P |= 0x1;} else {R.P &= ~0x1;}
	if (temp) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (temp & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void cpx() {
	unsigned char data = memory_read(EA);
	int temp = R.X - data;
	if (!(temp & 0x8000)) {R.P |= 0x1;} else {R.P &= ~0x1;}
	if (temp) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (temp & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void cpy() {
	unsigned char data = memory_read(EA);
	int temp = R.Y - data;
	if (!(temp & 0x8000)) {R.P |= 0x1;} else {R.P &= ~0x1;}
	if (temp) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (temp & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void dec() {
	write_memory(EA, (memory_read(EA) - 1) & 0xFF);
	unsigned char data = memory_read(EA);
    if (data) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (data & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void dex() {
	R.X = (R.X - 1) & 0xFF;
	if (R.X) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.X & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void dey() {
	R.Y = (R.Y - 1) & 0xFF;
	if (R.Y) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.Y & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void eor() {
    penalty_op = true;
	unsigned char data = memory_read(EA);
	R.A ^= data;
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void inc() {
	write_memory(EA, (memory_read(EA) + 1) & 0xFF);
	unsigned char data = memory_read(EA);
	if (data) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (data & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void inx() {
	R.X = (R.X + 1) & 0xFF;
	if (R.X) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.X & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void iny() {
	R.Y = (R.Y + 1) & 0xFF;
	if (R.Y) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.Y & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void jsr() {
    write_memory(R.S + 0x100, (R.PC - 1) >> 8);
	R.S = (R.S - 1) & 0xFF;
    write_memory(R.S + 0x100, (R.PC - 1) & 0xFF);
	R.S = (R.S - 1) & 0xFF;
	R.PC = EA;
}

void lda() {
    penalty_op = true;
	R.A = memory_read(EA);
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void ldx() {
    penalty_op = true;
	R.X = memory_read(EA);
	if (R.X) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.X & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void ldy() {
    penalty_op = true;
	R.Y = memory_read(EA);
	if (R.Y) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.Y & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void lsr_a() {
	if (R.A & 0x1) {R.P |= 0x1;} else {R.P &= ~0x1;}
	R.A >>= 1;
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void lsr() {
	unsigned char data = memory_read(EA);
	if (data & 0x1) {R.P |= 0x1;} else {R.P &= ~0x1;}
	data >>= 1;
	write_memory(EA, data);
	if (data) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (data & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void ora() {
    penalty_op = true;
	unsigned char data = memory_read(EA);
	R.A |= data;
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void pha() {
    write_memory(R.S + 0x100, R.A);
	R.S = (R.S - 1) & 0xFF;
}

void php() {
    write_memory(R.S + 0x100, R.P | 0x10);
	R.S = (R.S - 1) & 0xFF;
}

void pla() {
    R.S = (R.S + 1) & 0xFF;
	R.A = memory_read(R.S + 0x100);
    if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void plp() {
    R.S = (R.S + 1) & 0xFF;
	R.P = memory_read(R.S + 0x100) | 0x20;
}

void rol_a() {
	if (R.P & 0x1)
	{
		if (R.A & 0x80) {R.P |= 0x1;} else {R.P &= ~0x1;}
		R.A = (R.A << 1) | 0x1;
	}
	else
	{
		if (R.A & 0x80) {R.P |= 0x1;} else {R.P &= ~0x1;}
		R.A <<= 1;
	}
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void rol() {
	unsigned char data = memory_read(EA);
	if (R.P & 0x1)
	{
	    if (data & 0x80) {R.P |= 0x1;} else {R.P &= ~0x1;}
		data = (data << 1) | 0x1;
	}
	else
	{
	    if (data & 0x80) {R.P |= 0x1;} else {R.P &= ~0x1;}
		data <<= 1;
	}
	write_memory(EA, data);
	if (data) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (data & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void ror_a() {
	if (R.P & 0x1)
	{
	    if (R.A & 0x1) {R.P |= 0x1;} else {R.P &= ~0x1;}
		R.A = (R.A >> 1) | 0x80;
	}
	else
	{
	    if (R.A & 0x1) {R.P |= 0x1;} else {R.P &= ~0x1;}
		R.A >>= 1;
	}
	if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void ror() {
	unsigned char data = memory_read(EA);
	if (R.P & 0x1)
	{
		if (data & 0x1) {R.P |= 0x1;} else {R.P &= ~0x1;}
		data = (data >> 1) | 0x80;
	}
	else
	{
	    if (data & 0x1) {R.P |= 0x1;} else {R.P &= ~0x1;}
		data >>= 1;
	}
	write_memory(EA, data);
	if (data) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (data & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void rti() {
    R.S = (R.S + 1) & 0xFF;
	R.P = memory_read(R.S + 0x100) | 0x20;
	R.S = (R.S + 1) & 0xFF;
	R.PC = memory_read(R.S + 0x100);
	R.S = (R.S + 1) & 0xFF;
	R.PC |= (memory_read(R.S + 0x100) * 0x100);
}

void rts() {
    R.S = (R.S + 1) & 0xFF;
	R.PC = memory_read(R.S + 0x100);
	R.S = (R.S + 1) & 0xFF;
	R.PC |= (memory_read(R.S + 0x100) * 0x100);
	R.PC++;
}

void sbc() {
    penalty_op = true;
	unsigned char data = memory_read(EA) ^ 0xFF;
	int temp = R.A + data + (R.P & 0x1);
	if (temp > 0xFF) {R.P |= 0x1;} else {R.P &= ~0x1;}
	if ((~(R.A ^ data)) & (R.A ^ temp) & 0x80) {R.P |= 0x40;} else {R.P &= ~0x40;}
	R.A = temp & 0xFF;
    if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void sec() {
	R.P |= 0x1;
}

void sed() {
	R.P |= 0x8;
}

void sei() {
	R.P |= 0x4;
}

void sta() {
	write_memory(EA, R.A);
}

void stx() {
	write_memory(EA, R.X);
}

void sty() {
	write_memory(EA, R.Y);
}

void tax() {
	R.X = R.A;
    if (R.X) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.X & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void tay() {
	R.Y = R.A;
    if (R.Y) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.Y & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void tsx() {
	R.X = R.S;
    if (R.X) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.X & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void txa() {
	R.A = R.X;
    if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

void txs() {
	R.S = R.X;
}

void tya() {
	R.A = R.Y;
    if (R.A) {R.P &= ~0x2;} else {R.P |= 0x2;}
	if (R.A & 0x80) {R.P |= 0x80;} else {R.P &= ~0x80;}
}

/* Modos de endereçamento */

void imm() { //immediate
	EA = R.PC++;
}

void zp() { //zero-page
	EA = memory_read(R.PC++);
}

void zpx() { //zero-page,X
	EA = (memory_read(R.PC++) + R.X) & 0xFF;
}

void zpy() { //zero-page,Y
	EA = (memory_read(R.PC++) + R.Y) & 0xFF;
}

void rel() { //relative for branch ops (8-bit immediate value, sign-extended)
	EA = memory_read(R.PC++);
	if (EA >= 0x80) {EA -= 0x100;}
}

void abso() { //absolute
	EA = memory_read(R.PC) + (memory_read(R.PC + 1) * 0x100);
	R.PC += 2;
}

void absx() { //absolute,X
    penalty_addr = true;
	EA =memory_read(R.PC) + (memory_read(R.PC + 1) * 0x100) + R.X;
	R.PC += 2;
}

void absy() { //absolute,Y
    penalty_addr = true;
	EA = memory_read(R.PC) + (memory_read(R.PC + 1) * 0x100) + R.Y;
	R.PC += 2;
}

void ind() { //indirect
	int temp = memory_read(R.PC) + (memory_read(R.PC + 1) * 0x100);
	R.PC += 2;
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	EA = memory_read(temp) + (memory_read(data) * 0x100);
}

void indx() { //(indirect,X)
    int temp = memory_read(R.PC++) + R.X;
	EA = memory_read(temp & 0xFF) + (memory_read((temp + 1) & 0xFF) * 0x100);
}

void indy() { //(indirect),Y
    penalty_addr = true;
	int temp = memory_read(R.PC++);
	int data = (temp & 0xFF00) | ((temp + 1) & 0xFF); //ZP Wrap
	data = memory_read(temp) + (memory_read(data) * 0x100);
	EA = data + R.Y;
}

/* Interrupts */

//Nota: Break está junto com as instruções

void IRQ() {
	//Maskable Interrupt
    write_memory(R.S + 0x100, R.PC >> 8);
	R.S = (R.S - 1) & 0xFF;
	write_memory(R.S + 0x100, R.PC & 0xFF);
	R.S = (R.S - 1) & 0xFF;
	R.P |= 0x10;
	write_memory(R.S + 0x100, R.P);
	R.S = (R.S - 1) & 0xFF;
	R.P |= 0x4;
	R.PC = memory[0xFFFE] + (memory[0xFFFF] * 0x100);
	tick_count += 7;
}


void NMI() {
	//Non-Maskable Interrupt
    write_memory(R.S + 0x100, R.PC >> 8);
	R.S = (R.S - 1) & 0xFF;
	write_memory(R.S + 0x100, R.PC & 0xFF);
	R.S = (R.S - 1) & 0xFF;
	R.P |= 0x10;
    write_memory(R.S + 0x100, R.P);
	R.S = (R.S - 1) & 0xFF;
	R.P |= 0x4;
	R.PC = memory[0xFFFA] + (memory[0xFFFB] * 0x100);
	tick_count += 7;
}

/* Emulação da CPU (Reset/Inicia) */

void CPU_reset() {
    R.A = 0;
    R.X = 0;
    R.Y = 0;
    R.S = 0xFF;
    R.P = 0x20;
	R.PC = memory[0xFFFC] + (memory[0xFFFD] * 0x100);
}

void CPU_execute(int cycles) {
	unsigned char opcode;

	while(tick_count < cycles) {
        penalty_op = false; penalty_addr = false;
		opcode=memory[R.PC++];

		switch(opcode) {
			/* ADC  -  Add to Accumulator with Carry */
            case 0x69: imm(); adc(); tick_count += 2; break;
            case 0x65: zp(); adc(); tick_count += 3; break;
            case 0x75: zpx(); adc(); tick_count += 4; break;
            case 0x6D: abso(); adc(); tick_count += 4; break;
            case 0x7D: absx(); adc(); tick_count += 4; break;
            case 0x79: absy(); adc(); tick_count += 4; break;
            case 0x61: indx(); adc(); tick_count += 6; break;
            case 0x71: indy(); adc(); tick_count += 4; break;

            /* AND  -  AND Memory with Accumulator */
            case 0x29: imm(); and(); tick_count += 2; break;
            case 0x25: zp(); and(); tick_count += 3; break;
            case 0x35: zpx(); and(); tick_count += 4; break;
            case 0x2D: abso(); and(); tick_count += 4; break;
            case 0x3D: absx(); and(); tick_count += 4; break;
            case 0x39: absy(); and(); tick_count += 4; break;
            case 0x21: indx(); and(); tick_count += 6; break;
            case 0x31: indy(); and(); tick_count += 5; break;

            /* ASL  -  Arithmatic Shift Left */
            case 0x0A: asl_a(); tick_count += 2; break;
            case 0x06: zp(); asl(); tick_count += 5; break;
            case 0x16: zpx(); asl(); tick_count += 6; break;
            case 0x0E: abso(); asl(); tick_count += 6; break;
            case 0x1E: absx(); asl(); tick_count += 7; break;

            /* BCC  -  Branch on Carry Clear */
            case 0x90: rel(); bcc(); tick_count += 2; break;

            /* BCS  -  Branch on Carry Set */
            case 0xB0: rel(); bcs(); tick_count += 2; break;

            /* BEQ  -  Branch Zero Set */
            case 0xF0: rel(); beq(); tick_count += 2; break;

            /* BIT  -  Test Bits in Memory with Accumulator */
            case 0x24: zp(); bit(); tick_count += 3; break;
            case 0x2C: abso(); bit(); tick_count += 4; break;

            /* BMI  -  Branch on Result Minus */
            case 0x30: rel(); bmi(); tick_count += 2; break;

            /* BNE  -  Branch on Z reset */
            case 0xD0: rel(); bne(); tick_count += 2; break;

            /* BPL  -  Branch on Result Plus (or Positive) */
            case 0x10: rel(); bpl(); tick_count += 2; break;

            /* BRK  -  Force a Break */
            case 0x00: brk(); tick_count += 7; break;

            /* BVC  -  Branch on Overflow Clear */
            case 0x50: rel(); bvc(); tick_count += 2; break;

            /* BVS  -  Branch on Overflow Set */
            case 0x70: rel(); bvs(); tick_count += 2; break;

            /* CLC  -  Clear Carry Flag */
            case 0x18: R.P &= ~0x1; tick_count += 2; break;

            /* CLD  -  Clear Decimal Mode */
            case 0xD8: R.P &= ~0x8; tick_count += 2; break;

            /* CLI  -  Clear Interrupt Disable */
            case 0x58: R.P &= ~0x4; tick_count += 2; break;

            /* CLV  -  Clear Overflow Flag */
            case 0xB8: R.P &= ~0x40; tick_count += 2; break;

            /* CMP  -  Compare Memory and Accumulator */
            case 0xC9: imm(); cmp(); tick_count += 2; break;
            case 0xC5: zp(); cmp(); tick_count += 3; break;
            case 0xD5: zpx(); cmp(); tick_count += 4; break;
            case 0xCD: abso(); cmp(); tick_count += 4; break;
            case 0xDD: absx(); cmp(); tick_count += 4; break;
            case 0xD9: absy(); cmp(); tick_count += 4; break;
            case 0xC1: indx(); cmp(); tick_count += 6; break;
            case 0xD1: indy(); cmp(); tick_count += 5; break;

            /* CPX  -  Compare Memory and X register */
            case 0xE0: imm(); cpx(); tick_count += 2; break;
            case 0xE4: zp(); cpx(); tick_count += 3; break;
            case 0xEC: abso(); cpx(); tick_count += 4; break;

            /* CPY  -  Compare Memory and Y register */
            case 0xC0: imm(); cpy(); tick_count += 2; break;
            case 0xC4: zp(); cpy(); tick_count += 3; break;
            case 0xCC: abso(); cpy(); tick_count += 4; break;

            /* DEC  -  Decrement Memory by One */
            case 0xC6: zp(); dec(); tick_count += 5; break;
            case 0xD6: zpx(); dec(); tick_count += 6; break;
            case 0xCE: abso(); dec(); tick_count += 6; break;
            case 0xDE: absx(); dec(); tick_count += 7; break;

            /* DEX  -  Decrement X */
            case 0xCA: dex(); tick_count += 2; break;

            /* DEY  -  Decrement Y */
            case 0x88: dey(); tick_count += 2; break;

            /* EOR  -  Exclusive-OR Memory with Accumulator */
            case 0x49: imm(); eor(); tick_count += 2; break;
            case 0x45: zp(); eor(); tick_count += 3; break;
            case 0x55: zpx(); eor(); tick_count += 4; break;
            case 0x4D: abso(); eor(); tick_count += 4; break;
            case 0x5D: absx(); eor(); tick_count += 4; break;
            case 0x59: absy(); eor(); tick_count += 4; break;
            case 0x41: indx(); eor(); tick_count += 6; break;
            case 0x51: indy(); eor(); tick_count += 5; break;

            /* INC  -  Increment Memory by one */
            case 0xE6: zp(); inc(); tick_count += 5; break;
            case 0xF6: zpx(); inc(); tick_count += 6; break;
            case 0xEE: abso(); inc(); tick_count += 6; break;
            case 0xFE: absx(); inc(); tick_count += 7; break;

            /* INX  -  Increment X by one */
            case 0xE8: inx(); tick_count += 2; break;

            /* INY  -  Increment Y by one */
            case 0xC8: iny(); tick_count += 2; break;

            /* JMP - Jump */
            case 0x4c: abso(); R.PC = EA; tick_count += 3; break;
            case 0x6c: ind(); R.PC = EA; tick_count += 5; break;

            /* JSR - Jump to subroutine */
            case 0x20: abso(); jsr(); tick_count += 6; break;

            /* LDA - Load Accumulator with memory */
            case 0xA9: imm(); lda(); tick_count += 2; break;
            case 0xA5: zp(); lda(); tick_count += 3; break;
            case 0xB5: zpx(); lda(); tick_count += 4; break;
            case 0xAD: abso(); lda(); tick_count += 4; break;
            case 0xBD: absx(); lda(); tick_count += 4; break;
            case 0xB9: absy(); lda(); tick_count += 4; break;
            case 0xA1: indx(); lda(); tick_count += 6; break;
            case 0xB1: indy(); lda(); tick_count += 5; break;

            /* LDX - Load X with Memory */
            case 0xA2: imm(); ldx(); tick_count += 2; break;
            case 0xA6: zp(); ldx(); tick_count += 3; break;
            case 0xB6: zpy(); ldx(); tick_count += 4; break;
            case 0xAE: abso(); ldx(); tick_count += 4; break;
            case 0xBE: absy(); ldx(); tick_count += 4; break;

            /* LDY - Load Y with Memory */
            case 0xA0: imm(); ldy(); tick_count += 2; break;
            case 0xA4: zp(); ldy(); tick_count += 3; break;
            case 0xB4: zpx(); ldy(); tick_count += 4; break;
            case 0xAC: abso(); ldy(); tick_count += 4; break;
            case 0xBC: absx(); ldy(); tick_count += 4; break;

            /* LSR  -  Logical Shift Right */
            case 0x4A: lsr_a(); tick_count += 2; break;
            case 0x46: zp(); lsr(); tick_count += 5; break;
            case 0x56: zpx(); lsr(); tick_count += 6; break;
            case 0x4E: abso(); lsr(); tick_count += 6; break;
            case 0x5E: absx(); lsr(); tick_count += 7; break;

            /* NOP - No Operation */
            case 0xEA: tick_count += 2; break;

            /* ORA  -  OR Memory with Accumulator */
            case 0x09: imm(); ora(); tick_count += 2; break;
            case 0x05: zp(); ora(); tick_count += 3; break;
            case 0x15: zpx(); ora(); tick_count += 4; break;
            case 0x0D: abso(); ora(); tick_count += 4; break;
            case 0x1D: absx(); ora(); tick_count += 4; break;
            case 0x19: absy(); ora(); tick_count += 4; break;
            case 0x01: indx(); ora(); tick_count += 6; break;
            case 0x11: indy(); ora(); tick_count += 5; break;

            /* PHA  -  Push Accumulator on Stack */
            case 0x48: pha(); tick_count += 3; break;

            /* PHP  -  Push Processor Status on Stack */
            case 0x08: php(); tick_count += 3; break;

            /* PLA  -  Pull Accumulator from Stack */
            case 0x68: pla(); tick_count += 4; break;

            /* PLP  -  Pull Processor Status from Stack */
            case 0x28: plp(); tick_count += 4; break;

            /* ROL  -  Rotate Left */
            case 0x2A: rol_a(); tick_count += 2; break;
            case 0x26: zp(); rol(); tick_count += 5; break;
            case 0x36: zpx(); rol(); tick_count += 6; break;
            case 0x2E: abso(); rol(); tick_count += 6; break;
            case 0x3E: absx(); rol(); tick_count += 7; break;

            /* ROR  -  Rotate Right */
            case 0x6A: ror_a(); tick_count += 2; break;
            case 0x66: zp(); ror(); tick_count += 5; break;
            case 0x76: zpx(); ror(); tick_count += 6; break;
            case 0x6E: abso(); ror(); tick_count += 6; break;
            case 0x7E: absx(); ror(); tick_count += 7; break;

            /* RTI  -  Return from Interrupt */
            case 0x40: rti(); tick_count += 6; break;

            /* RTS  -  Return from Subroutine */
            case 0x60: rts(); tick_count += 6; break;

            /* SBC  -  Subtract from Accumulator with Carry */
            case 0xE9: case 0xEB: imm(); sbc(); tick_count += 2; break; //OBS: 0xEB não oficial
            case 0xE5: zp(); sbc(); tick_count += 3; break;
            case 0xF5: zpx(); sbc(); tick_count += 4; break;
            case 0xED: abso(); sbc(); tick_count += 4; break;
            case 0xFD: absx(); sbc(); tick_count += 4; break;
            case 0xF9: absy(); sbc(); tick_count += 4; break;
            case 0xE1: indx(); sbc(); tick_count += 6; break;
            case 0xF1: indy(); sbc(); tick_count += 5; break;

            /* SEC  -  Set Carry Flag */
            case 0x38: sec(); tick_count += 2; break;

            /* SED  -  Set Decimal Mode */
            case 0xF8: sed(); tick_count += 2; break;

            /* SEI - Set Interrupt Disable */
            case 0x78: sei(); tick_count += 2; break;

            /* STA - Store Accumulator in Memory */
            case 0x85: zp(); sta(); tick_count += 3; break;
            case 0x95: zpx(); sta(); tick_count += 4; break;
            case 0x8D: abso(); sta(); tick_count += 4; break;
            case 0x9D: absx(); sta(); tick_count += 5; break;
            case 0x99: absy(); sta(); tick_count += 5; break;
            case 0x81: indx(); sta(); tick_count += 6; break;
            case 0x91: indy(); sta(); tick_count += 6; break;

            /* STX - Store X in Memory */
            case 0x86: zp(); stx(); tick_count += 3; break;
            case 0x96: zpy(); stx(); tick_count += 4; break;
            case 0x8E: abso(); stx(); tick_count += 4; break;

            /* STY - Store Y in Memory */
            case 0x84: zp(); sty(); tick_count += 3; break;
            case 0x94: zpx(); sty(); tick_count += 4; break;
            case 0x8C: abso(); sty(); tick_count += 4; break;

            /* TAX  -  Transfer Accumulator to X */
            case 0xAA: tax(); tick_count += 2; break;

            /* TAY  -  Transfer Accumulator to Y */
            case 0xA8: tay(); tick_count += 2; break;

            /* TSX  -  Transfer Stack to X */
            case 0xBA: tsx(); tick_count += 2; break;

            /* TXA  -  Transfer X to Accumulator */
            case 0x8A: txa(); tick_count += 2; break;

            /* TXS  -  Transfer X to Stack */
            case 0x9A: txs(); tick_count += 2; break;

            /* TYA  -  Transfer Y to Accumulator */
            case 0x98: tya(); tick_count += 2; break;

            /* Não oficial */
            /* LAX */
            case 0xA7: zp(); lda(); ldx(); tick_count += 3; break;
            case 0xB7: zpy(); lda(); ldx(); tick_count += 4; break;
            case 0xAF: abso(); lda(); ldx(); tick_count += 4; break;
            case 0xBF: absy(); lda(); ldx(); tick_count += 4; break;
            case 0xA3: indx(); lda(); ldx(); tick_count += 6; break;
            case 0xB3: indy(); lda(); ldx(); tick_count += 5; break;

            /* SAX */
            case 0x87: zp(); sta(); stx(); write_memory(EA, R.A & R.X); tick_count += 3; break;
            case 0x97: zpy(); sta(); stx(); write_memory(EA, R.A & R.X); tick_count += 4; break;
            case 0x8F: abso(); sta(); stx(); write_memory(EA, R.A & R.X); tick_count += 4; break;
            case 0x83: indx(); sta(); stx(); write_memory(EA, R.A & R.X); tick_count += 6; break;

            /* DCP */
            case 0xC7: zp(); dec(); cmp(); tick_count += 5; break;
            case 0xD7: zpx(); dec(); cmp(); tick_count += 6; break;
            case 0xCF: abso(); dec(); cmp(); tick_count += 6; break;
            case 0xDF: absx(); dec(); cmp(); tick_count += 7; break;
            case 0xDB: absy(); dec(); cmp(); tick_count += 7; break;
            case 0xC3: indx(); dec(); cmp(); tick_count += 8; break;
            case 0xD3: indy(); dec(); cmp(); tick_count += 8; break;

            /* ISB */
            case 0xE7: zp(); inc(); sbc(); tick_count += 5; break;
            case 0xF7: zpx(); inc(); sbc(); tick_count += 6; break;
            case 0xEF: abso(); inc(); sbc(); tick_count += 6; break;
            case 0xFF: absx(); inc(); sbc(); tick_count += 7; break;
            case 0xFB: absy(); inc(); sbc(); tick_count += 7; break;
            case 0xE3: indx(); inc(); sbc(); tick_count += 8; break;
            case 0xF3: indy(); inc(); sbc(); tick_count += 8; break;

            /* SLO */
            case 0x7: zp(); asl(); ora(); tick_count += 5; break;
            case 0x17: zpx(); asl(); ora(); tick_count += 6; break;
            case 0xF: abso(); asl(); ora(); tick_count += 6; break;
            case 0x1F: absx(); asl(); ora(); tick_count += 7; break;
            case 0x1B: absy(); asl(); ora(); tick_count += 7; break;
            case 0x3: indx(); asl(); ora(); tick_count += 8; break;
            case 0x13: indy(); asl(); ora(); tick_count += 8; break;

            /* RLA */
            case 0x27: zp(); rol(); and(); tick_count += 5; break;
            case 0x37: zpx(); rol(); and(); tick_count += 6; break;
            case 0x2F: abso(); rol(); and(); tick_count += 6; break;
            case 0x3F: absx(); rol(); and(); tick_count += 7; break;
            case 0x3B: absy(); rol(); and(); tick_count += 7; break;
            case 0x23: indx(); rol(); and(); tick_count += 8; break;
            case 0x33: indy(); rol(); and(); tick_count += 8; break;

            /* SRE */
            case 0x47: zp(); lsr(); eor(); tick_count += 5; break;
            case 0x57: zpx(); lsr(); eor(); tick_count += 6; break;
            case 0x4F: abso(); lsr(); eor(); tick_count += 6; break;
            case 0x5F: absx(); lsr(); eor(); tick_count += 7; break;
            case 0x5B: absy(); lsr(); eor(); tick_count += 7; break;
            case 0x43: indx(); lsr(); eor(); tick_count += 8; break;
            case 0x53: indy(); lsr(); eor(); tick_count += 8; break;

            /* RRA */
            case 0x67: zp(); ror(); adc(); tick_count += 5; break;
            case 0x77: zpx(); ror(); adc(); tick_count += 6; break;
            case 0x6F: abso(); ror(); adc(); tick_count += 6; break;
            case 0x7F: absx(); ror(); adc(); tick_count += 7; break;
            case 0x7B: absy(); ror(); adc(); tick_count += 7; break;
            case 0x63: indx(); ror(); adc(); tick_count += 8; break;
            case 0x73: indy(); ror(); adc(); tick_count += 8; break;

            case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA: tick_count += 2; break; //NOP
            case 0x80: case 0x82: case 0x89: case 0xC2: case 0xE2: R.PC++; tick_count += 2; break; //DOP (2 cyc)
            case 0x4: case 0x44: case 0x64:  R.PC++; tick_count += 3; break; //DOP (3 cyc)
            case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4: R.PC++; tick_count += 4; break; //DOP (4 cyc)
            case 0xC: case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC: R.PC+=2; tick_count += 4; break; //TOP
		}

		if (penalty_op && penalty_addr) tick_count++;
	}

    tick_count -= cycles;
}
