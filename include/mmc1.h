#include "nes6502.h"
// TODO: Write in ASM

int mmc1_reg[4];
int mmc1_seq;
int mmc1_acc;

void mmc1_switch_chr(int bank, int pagesize, int area) {
	int prg_size;
	int chr_size;
	int chr_start;

	unsigned int address;

	prg_size = 16384;

	chr_start = prg_size * PRG;

	if(pagesize == 0) {
		chr_size = 8192;
		address = 0x0000;
	} else if(pagesize == 1) {
		chr_size = 4096;
		if(area == 0) {
			address = 0x0000;
		} else if(area == 1) {
			address = 0x1000;
		 } else {

		}
	} else {

	}

	memcpy(PPU_Memory + address, ROM_Cache + 16 + chr_start + (bank * chr_size), chr_size);
}

void mmc1_access(unsigned int address,unsigned char data) {
	int regnum;

	if(address > 0x7fff && address < 0x10000) {
        if(address > 0x7fff && address < 0xa000) {regnum = 0;}
        if(address > 0x9fff && address < 0xc000) {regnum = 1;}
        if(address > 0xbfff && address < 0xe000) {regnum = 2;}
        if(address > 0xdfff && address < 0x10000) {regnum = 3;}
        if (data & 0x80) {
			mmc1_reg[0] |= 0xC;
			mmc1_acc = mmc1_reg[regnum];
			mmc1_seq = 5;
		} else {
		    if (data & 1) {mmc1_acc |= (1 << mmc1_seq);}
			mmc1_seq++;
		}

		if (mmc1_seq == 5) {
            mmc1_reg[regnum] = mmc1_acc;
            mmc1_seq = 0;
            mmc1_acc = 0;

            //Mirroring
            if (mmc1_reg[0] & 2) {
                do_mirror((mmc1_reg[0] & 0x01) ^ 1);
            } else {
                do_mirror(2);
            }

			//Bank Switch PRG
			if ((mmc1_reg[0] & 0x08) == 0) { //32k PRG
                //Copia um banco de 32k
                memcpy(memory + 0x8000, ROM_Cache + 16 + (maskaddr((mmc1_reg[3] & 15) * 4) * 0x2000), 32768);
            } else { //16k PRG
                if (mmc1_reg[0] & 0x04) {
                    //Copia dois bancos de 16k
                    memcpy(memory + 0xC000, ROM_Cache + 16 + (maskaddr(0xFE) * 0x2000), 16384);
                    memcpy(memory + 0x8000, ROM_Cache + 16 + (maskaddr((mmc1_reg[3] & 15) * 2) * 0x2000), 16384);
                } else {
                    //Copia dois bancos de 16k
                    memcpy(memory + 0x8000, ROM_Cache + 16, 16384);
                    memcpy(memory + 0xC000, ROM_Cache + 16 + (maskaddr((mmc1_reg[3] & 15) * 2)  * 0x2000), 16384);
                }
            }

            //Bank Switch CHR
			if (CHR != 0) {
                if(mmc1_reg[0] & 0x10) { //4k
                    mmc1_switch_chr(mmc1_reg[1], 1, 0);
                    mmc1_switch_chr(mmc1_reg[2], 1, 1);
                } else { //8k
                    mmc1_switch_chr(mmc1_reg[1] / 2, 0, 0);
                }
            }
		}
	}
}
