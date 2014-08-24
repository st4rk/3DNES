typedef unsigned char UINT_8;
typedef unsigned int UINT_32;

void mapper79_prg_switch(UINT_32 bank) {
	UINT_32 PRG_SIZE = 32768;
	UINT_32 PRG_ADDRESS = 0x8000;
	UINT_8 bank_select = (bank & 0x8); // 0x8 == 0x0001000

	memcpy(memory + PRG_ADDRESS, romcache + 16 + (bank_select  * PRG_SIZE), PRG_SIZE);
}



void mapper79_chr_switch(UINT_32 bank) {
	UINT_32 CHR_SIZE = 8192;
	UINT_32 CHR_ADDRESS = 0x0;
	UINT_8 bank_select = (bank & 0x7); // 0x7 == 0x00000111

	memcpy(ppu_memory + CHR_ADDRESS, romcache + 16 + (PRG * 16384) + (bank_select * CHR_SIZE), CHR_SIZE);
}


void mapper79_access(UINT_32 address, UINT_8 data) {
	/* Select CHR or PRG */

	mapper79_chr_switch(data);

	/* if(address > 0x7FFF && address < 0x10000) // 0x7 == 0x00000111
		mapper79_prg_switch(data); */


	/* if(address > 0x0000 && address < 0x2000)
		mapper79_chr_switch(data); */

}
