/* Mirroring especial */
extern int mirror[4]; //O MMC5 tem um Mirroring especial
extern unsigned char nt[4][0x400]; //O MMC5 pode usar ExRam como NameTable
int sel_nt;

/* RAM Extra */
unsigned char ex_ram[1024]; //RAM Extra dentro do MAPPER de 1KB
unsigned char mmc5_wram[65536]; //64kb de RAM extra em alguns cartuchos

unsigned char mmc5_wram_page;
unsigned char mmc5_wram_chip;

/* PRG/CHR */
unsigned char mmc5_prgsize;
unsigned char mmc5_chrsize;
unsigned char mmc5_gfx_mode;

unsigned char mmc5_chr_page_sprite[8];
unsigned char mmc5_chr_page_background[4];

/* IRQ */
unsigned char mmc5_irq_clear;
unsigned int mmc5_irq_scanline;
unsigned int mmc5_irq_line;
unsigned char mmc5_irq_status;
unsigned char mmc5_irq_enable;

void copynt(int nt_num) {
    int i;
    for (i = 0; i < 1024; i++) {
        nt[nt_num][i] = ex_ram[i];
    }
}

void mmc5_switch_prg(int address, int bank, int prg_size) {
	memcpy(memory + address, romcache + 16 + (maskaddr(bank)  * 0x2000), prg_size);
}

void mmc5_switch_chr(int bank, int page, int chr_size) {
	int prg_size;

	int chr_start;

	unsigned int address;

	address = page * 0x400;

	prg_size = 16384;
	chr_start = prg_size * PRG;

	memcpy(ppu_memory + address, romcache + 16 + chr_start + (bank * 0x400), chr_size);
}

void mmc5_access(unsigned int address,unsigned char data) {
    switch (address) {
        case 0x5100: mmc5_prgsize = data & 3; break;
        case 0x5101: mmc5_chrsize = data & 3; break;
        case 0x5104: mmc5_gfx_mode = data & 3; break; //OBS: Apenas um modo suportado
        case 0x5105: //Mirroring do MMC5
            mirror[0] = data & 1;
            mirror[1] = (data & 4) >> 2;
            mirror[2] = (data & 0x10) >> 4;
            mirror[3] = (data & 0x40) >> 6;

            if (mmc5_gfx_mode == 0) { //Modo de usar ExRam como NameTable
                if (data & 2) {sel_nt = 0;}
                if (data & 8) {sel_nt = 1;}
                if (data & 0x20) {sel_nt = 2;}
                if (data & 0x80) {sel_nt = 3;}
            }
        break;
        case 0x5113:
            mmc5_wram_page = data & 3;
            mmc5_wram_chip = (data & 4) >> 2;
        break;
        case 0x5114:
        case 0x5115:
        case 0x5116:
        case 0x5117:
            if (data & 0x80) {
                switch(address & 7) {
                    case 4: if (mmc5_prgsize == 3) mmc5_switch_prg(0x8000, data & 0x7F, 8192); break;
                    case 5:
                        if ((mmc5_prgsize == 1) || (mmc5_prgsize == 2)) { //16k 8
                            mmc5_switch_prg(0x8000, data & 0x7F, 16384);
                        } else { //8k A
                            mmc5_switch_prg(0xA000, data & 0x7F, 8192);
                        }
                    break;
                    case 6: if ((mmc5_prgsize == 2) || (mmc5_prgsize == 3)) mmc5_switch_prg(0xC000, data & 0x7F, 8192); break;
                    case 7:
                        switch (mmc5_prgsize) {
                            case 0: mmc5_switch_prg(0x8000, data & 0x7F, 32768); break;
                            case 1: mmc5_switch_prg(0xC000, data & 0x7F, 16384); break;
                            case 2: mmc5_switch_prg(0xE000, data & 0x7F, 8192); break;
                        }
                    break;
                }
            } else { //Usa SRAM como... PRG-RAM?
                switch(address & 7) {
                    case 4: break;
                    case 5:
                        if ((mmc5_prgsize == 1) || (mmc5_prgsize == 2)) {
                            //???
                        } else { //METAL SLADER GLORY ESCREVE AQUI !!!!!!!!!!!!
                            //???
                            //printf("mmc5 wram\n");
                            memcpy(mmc5_wram + (mmc5_wram_page * 8192) + (mmc5_wram_chip * 8192), romcache + 16 + (maskaddr(data & 0x7F)  * 0x2000), 8192);
                            //mmc5_switch_prg(0x8000, data & 0x7F, 16384); break;
                        }
                    break;
                    case 6: break;
                }
            }
        break;
        case 0x5120: //CHR Sprite
        case 0x5121:
        case 0x5122:
        case 0x5123:
        case 0x5124:
        case 0x5125:
        case 0x5126:
        case 0x5127:
            mmc5_chr_page_sprite[address & 7] = data;

            switch(mmc5_chrsize) {
                case 0: mmc5_switch_chr(mmc5_chr_page_sprite[7], 0, 8192); break;
                case 1:
                    mmc5_switch_chr(mmc5_chr_page_sprite[3], 0, 4096);
                    mmc5_switch_chr(mmc5_chr_page_sprite[7], 4, 4096);
                break;
                case 2:
                    mmc5_switch_chr(mmc5_chr_page_sprite[1], 0, 2048);
                    mmc5_switch_chr(mmc5_chr_page_sprite[3], 2, 2048);
                    mmc5_switch_chr(mmc5_chr_page_sprite[5], 4, 2048);
                    mmc5_switch_chr(mmc5_chr_page_sprite[7], 6, 2048);
                break;
                case 3:
                    mmc5_switch_chr(mmc5_chr_page_sprite[0], 0, 1024);
                    mmc5_switch_chr(mmc5_chr_page_sprite[1], 1, 1024);
                    mmc5_switch_chr(mmc5_chr_page_sprite[2], 2, 1024);
                    mmc5_switch_chr(mmc5_chr_page_sprite[3], 3, 1024);
                    mmc5_switch_chr(mmc5_chr_page_sprite[4], 4, 1024);
                    mmc5_switch_chr(mmc5_chr_page_sprite[5], 5, 1024);
                    mmc5_switch_chr(mmc5_chr_page_sprite[6], 6, 1024);
                    mmc5_switch_chr(mmc5_chr_page_sprite[7], 7, 1024);
                break;
            }
        break;
        case 0x5128: //CHR Background
        case 0x5129:
        case 0x512A:
        case 0x512B:
            mmc5_chr_page_background[address & 3] = data;

            switch(mmc5_chrsize) {
                case 1: mmc5_switch_chr(mmc5_chr_page_background[3], 0, 8192); break;
                case 3:
                    mmc5_switch_chr(mmc5_chr_page_background[0], 4, 1024);
                    mmc5_switch_chr(mmc5_chr_page_background[1], 5, 1024);
                    mmc5_switch_chr(mmc5_chr_page_background[2], 6, 1024);
                    mmc5_switch_chr(mmc5_chr_page_background[3], 7, 1024);
                break;
            }
        break;
        case 0x5203: mmc5_irq_line = data; break;
        case 0x5204: mmc5_irq_enable = data; break;
    }

    if (address >= 0x5C00 && address <= 0x5FFF) {
            ex_ram[address - 0x5C00] = data;
            if (mmc5_gfx_mode == 0) { //Modo de usar ExRam como NameTable
                copynt(sel_nt);
            }
    }
}

unsigned char mmc5_read(unsigned int address) {
    unsigned char tmp = 0;
    switch (address) {
        case 0x5204:
            tmp = mmc5_irq_status;
            mmc5_irq_status = 0;
            mmc5_irq_status &= 0x80;
            return tmp;
        break;
    }

    if ((address >= 0x5C00 && address <= 0x5FFF) && (mmc5_gfx_mode >= 2)) {return ex_ram[address - 0x5C00];}

    return(0);
}

void mmc5_hblank(int scanline) {
    if (scanline < 240) {
        mmc5_irq_scanline++;
        mmc5_irq_status |= 0x40;
        mmc5_irq_clear = 0;
    }

    if (mmc5_irq_scanline == mmc5_irq_line) {
        mmc5_irq_status |= 0x80;
    }
    if (++mmc5_irq_clear > 2) {
        mmc5_irq_scanline = 0;
        mmc5_irq_status &= ~0x80;
        mmc5_irq_status &= ~0x40;
    }

    if ((mmc5_irq_enable & 0x80) && (mmc5_irq_status & 0x80) && (mmc5_irq_status & 0x40)) IRQ();
}
