#include "nesMemory.h"


//TODO: Write nesMemory in Assembly

/* Read a byte */
u8 	memoryRead(u32 addr) {
	/* this is ram or rom so we can return the addr */
	if (addr < 0x2000)
		return memory[addr & 0x7FF];

	if (addr > 0x7FFF)
		return memory[addr];


	/* The addr between 0x200 and 0x500 are for input and ouput */

	if (addr == 0x2002) {
		ppu_status_tmp = ppu_status;

		/* set ppu_status (D7) to 0 (vblank on) */
		ppu_status &= 0x7F;

		/* set ppu_status (D6) to 0 (sprite zero) */
		ppu_status &= 0x1F;

		/* Reset VRAM addr Register #1 */
		ppu_bgscr_f = 0x0;

		/* reset VRAM addr Register #2 */
		ppu_addr_h =  0x0;

		/* return bits 7-4 of unmodifyed ppu_status with bits 3-0 of the ppu_addr_tmp */
		return (ppu_status_tmp & 0xE0) | (ppu_addr_tmp & 0x1F);
	}

	if(addr == 0x2007) {
        int tmp = ppu_addr_tmp;
		ppu_addr_tmp = ppu_addr;

		if(increment_32 == 0) {
			ppu_addr++;
		} else {
			ppu_addr += 0x20;
		}

		return PPU_Memory[tmp];
	}

	/* pAPU data (sound) */
	if(addr == 0x4015) {
		return memory[addr];
	}

	/* joypad1 data */
	if(addr == 0x4016) {
		switch(PAD1_ReadCount) {
			case 0:
			memory[addr] = PAD1_A;
			PAD1_ReadCount++;
			break;

			case 1:
			memory[addr] = PAD1_B;
			PAD1_ReadCount++;
			break;

			case 2:
			memory[addr] = PAD1_SELECT;
			PAD1_ReadCount++;
			break;

			case 3:
			memory[addr] = PAD1_START;
			PAD1_ReadCount++;
			break;

			case 4:
			memory[addr] = PAD1_UP;
			PAD1_ReadCount++;
			break;

			case 5:
			memory[addr] = PAD1_DOWN;
			PAD1_ReadCount++;
			break;

			case 6:
			memory[addr] = PAD1_LEFT;
			PAD1_ReadCount++;
			break;

			case 7:
			memory[addr] = PAD1_RIGHT;
			PAD1_ReadCount = 0;
			break;
		}

		return memory[addr];
	}

	if(addr == 0x4017) {
		return memory[addr];
	}

	if (MAPPER == 5) {
        if((addr == 0x5204) || (addr >= 0x5C00 && addr <= 0x5FFF)) {
            return mmc5_read(addr);
        }
    }

	if((addr > 0x5FFF) && (addr < 0x8000) && (MAPPER == 5)) { //SRAM
        return mmc5_wram[(addr - 0x6000) + (mmc5_wram_page * 8192) + (mmc5_wram_chip * 8192)];
	}


	return memory[addr];
}

/* Read 2 bytes */
int  memoryRead16(u32 addr) {
	return (memory_read(addr) + (memory_read(addr + 1) * 0x100));
}

/* write a byte */
void writeMemory(u32 addr, u8 data) {
/* PPU Video Memory area */
	if(addr > 0x1fff && addr < 0x4000) {
		write_ppu_memory(addr,data);
		return;
	}

	/* Sprite DMA Register */
	if(addr == 0x4014) {
		write_ppu_memory(addr,data);
		tick_count += 512;
		return;
	}

	/* Joypad 1 */
	if(addr == 0x4016) {
		memory[addr] = 0x40;

		return;
	}

	/* Joypad 2 */
	if(addr == 0x4017) {
		memory[addr] = 0x48;
		return;
	}

	/* pAPU Sound Registers */
	if(addr > 0x3fff && addr < 0x4016) {
		memory[addr] = data;
		return;
	}
	
	/* SRAM Registers */
	if(addr > 0x5fff && addr < 0x8000) {
        if(MAPPER == 5) { //SRAM
            mmc5_wram[(addr - 0x6000) + (mmc5_wram_page * 8192) + (mmc5_wram_chip * 8192)] = data;
            return;
        } else {

            memory[addr] = data;
            return;
        }
	}

	/* RAM registers */
	if(addr < 0x2000) {memory[addr & 0x7FF] = data; return;}

	switch (MAPPER) {
		case 1:
			mmc1_access(addr, data);
			return;
		break;

		case 2:
			unrom_access(addr, data);
			return;
		break;

		case 3:
			cnrom_access(addr, data);
			return;
		break;

		case 4:
			mmc3_access(addr, data);
			return;
		break;

		case 5:
			mmc5_access(addr, data);
			return;
		break;

		case 7:
			aorom_access(addr, data);
			return;
		break;

		case 79:
			mapper79_access(addr,data);
			return;
		break;

		default:

		break;
	}

	memory[addr] = data;
}


