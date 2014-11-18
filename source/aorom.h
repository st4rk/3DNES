
void aorom_switch_prg(int bank) {
	int prg_size;
	unsigned int address;

	address = 0x8000;
	prg_size = 32768;


	memcpy(memory + address, ROM_Cache + 16 + (bank  * prg_size), prg_size);
}

void
aorom_access(unsigned int address,unsigned char data)
{

	if(address > 0x7fff && address < 0x10000) {
        if (data & 0x10) {do_mirror(2);} else {do_mirror(3);}
		aorom_switch_prg(data & 0xF);
	}
}
