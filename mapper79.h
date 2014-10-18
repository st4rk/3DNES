typedef unsigned char UINT_8;
typedef unsigned int UINT_32;




void mapper79_chr_switch(UINT_32 bank) {
	UINT_32 CHR_SIZE = 8192;
	UINT_8 bank_select = (bank & 0x7); // 0x7 == 0x00000111

	memcpy(ppu_memory, romcache + 16 + (PRG * 16384) + (bank_select * CHR_SIZE), CHR_SIZE);
}


void mapper79_access(UINT_32 address, UINT_8 data) {

	mapper79_chr_switch(data);

}
