

// TODO: Write in ASM

void
cnrom_switch_chr(int bank)
{
	int prg_size;
	int chr_size;

	int chr_start;

	unsigned int address;

	address = 0x0000;

	prg_size = 16384;
	chr_size = 8192;

	chr_start = prg_size * PRG;


	memcpy(PPU_Memory + address, ROM_Cache + 16 + chr_start + (bank * chr_size), chr_size);
}

void
cnrom_access(unsigned int address,unsigned char data)
{
	if(address > 0x7fff && address < 0x10000) {
		cnrom_switch_chr(data & (CHR - 1));
	}
}
