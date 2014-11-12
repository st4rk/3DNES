/*
 * LameNES - Nintendo Entertainment System (NES) emulator
 *
 * Copyright (c) 2005, Joey Loman, <joey@lamenes.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * mmc3.h - NES Mapper 4: MMC3
 */

unsigned char mmc3_cmd;

int mmc3_prg_bank0 = 0;
int mmc3_prg_bank1 = 0;

int mmc3_prg_page = 0;

int mmc3_chr_xor = 0;

int mmc3_irq_counter = 0;
int mmc3_irq_latch;
int mmc3_irq_control0;
int mmc3_irq_control1;
int mmc3_irq_enable = 0;

int mmc3_prg_addr;

void
mmc3_reset()
{
	memcpy(memory + 0xa000, romcache + 16, 8192);
}

void
mmc3_switch_prg(unsigned int address, int bank)
{
	int prg_size = 8192;
	memcpy(memory + address, romcache + 16 + (bank * prg_size), prg_size);
}

void
mmc3_switch_chr(unsigned int address, int bank, int pagecount)
{
    if (CHR != 0) {
        int prg_size;
        int chr_size;
        int chr_start;

        prg_size = 16384;
        chr_size = 1024;

        chr_start = prg_size * PRG;

        memcpy(ppu_memory + address, romcache + 16 + chr_start + (bank * chr_size), chr_size * pagecount);
    }
}

void
mmc3_access(unsigned int address,unsigned char data)
{
	switch(address) {
		case 0x8000:
            /* store command */
            mmc3_cmd = data & 7;
            mmc3_prg_addr = data & 0x40;

            /* check for chr swapping */
            if(data & 0x80) {
                mmc3_chr_xor = 1;
            } else {
                mmc3_chr_xor = 0;
            }
		break;
		case 0x8001:
            /* exec command (bit 0-2)*/
            switch(mmc3_cmd){
                case 0:
                    if(mmc3_chr_xor == 0) {
                        mmc3_switch_chr(0x0000, data, 2);
                    } else {
                        mmc3_switch_chr(0x1000, data, 2);
                    }
                break;

                case 1:
                    if(mmc3_chr_xor == 0) {
                        mmc3_switch_chr(0x0800, data, 2);
                    } else {
                        mmc3_switch_chr(0x1800, data, 2);
                    }
                break;

                case 2:
                    if(mmc3_chr_xor == 0) {
                        mmc3_switch_chr(0x1000, data, 1);
                    } else {
                        mmc3_switch_chr(0x0000, data, 1);
                    }
                break;

                case 3:
                    if(mmc3_chr_xor == 0) {
                        mmc3_switch_chr(0x1400, data, 1);
                    } else {
                        mmc3_switch_chr(0x400, data, 1);
                    }
                break;

                case 4:
                    if(mmc3_chr_xor == 0) {
                        mmc3_switch_chr(0x1800, data, 1);
                    } else {
                        mmc3_switch_chr(0x800, data, 1);
                    }
                break;

                case 5:
                    if(mmc3_chr_xor == 0) {
                        mmc3_switch_chr(0x1c00, data, 1);
                    } else {
                        mmc3_switch_chr(0x0c00, data, 1);
                    }
                break;

                case 6: mmc3_prg_bank0 = data; break;
                case 7: mmc3_prg_bank1 = data; break;
            }

            if ((mmc3_cmd == 6) || (mmc3_cmd == 7)) {
                if(mmc3_prg_addr) {
                    mmc3_switch_prg(0x8000,(PRG << 1) - 2);
                    mmc3_switch_prg(0xa000,mmc3_prg_bank1);
                    mmc3_switch_prg(0xc000,mmc3_prg_bank0);
                    mmc3_switch_prg(0xe000,(PRG << 1) - 1);
                } else {
                    mmc3_switch_prg(0x8000,mmc3_prg_bank0);
                    mmc3_switch_prg(0xa000,mmc3_prg_bank1);
                    mmc3_switch_prg(0xc000,(PRG << 1) - 2);
                    mmc3_switch_prg(0xe000,(PRG << 1) - 1);
                }
            }
        break;
		case 0xa000:
            /* set horizontal/vertical mirroring */
            if(data & 0x01) {
                /* set to vertical */
                //MIRRORING = 1;
                do_mirror(0);
            } else {
                /* set to horizontal */
                //MIRRORING = 0;
                do_mirror(1);
            }
        break;
		case 0xa001: if(data) SRAM = 1; break;
		case 0xc000:
            /* set IRQ counter */
            mmc3_irq_counter = data;
        break;
		case 0xc001:
            /* set IRQ tmp latch */
            mmc3_irq_latch = data;
        break;
		case 0xe000:
            mmc3_irq_counter = mmc3_irq_latch;
            mmc3_irq_enable = 0;
		break;
		case 0xe001: mmc3_irq_enable = 1; break;
	}
}

void mmc3_hblank(int scanline) {
    if (scanline == 0) {
        mmc3_irq_counter = mmc3_irq_latch;
    } else {
        if(mmc3_irq_enable && (ppu_control2 & 0x18)) { //MMC3 IRQ
            mmc3_irq_counter = (mmc3_irq_counter - 1) & 0xFF;
            if(mmc3_irq_counter == 0) {
                tick_count = 0; //[!!!] Workaround: Conserta partes da tela balançando em SMB3, Kirby
                IRQ();
                mmc3_irq_counter = mmc3_irq_latch;
            }
        }
    }
}