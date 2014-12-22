#include "nesLoadROM.h"

extern u8 memory[65536];

u8 	ROM_Header[15];
u8	ROM_Title[128];

int NES_LoadROM() {
	int i = 0;

	memcpy (ROM_Header, ROM_Cache, 15);

	/* Check if is a valid NES ROM */
	if ((ROM_Header[0] != 'N' || (ROM_Header[1] != 'E') || (ROM_Header[2] != 'S'))) {
		// TODO: Err Msg
		return -1;
	}

	for (i = 8; i < 15; i++) {
		if ((ROM_Header[i] != 0x0) && (ROM_Header[i] != 0xFF)) {
			// TODO: Err Msg
			return -1;
		}
	}


	/* Load PRG, CHR, MAPPER, RCB etc */
	PRG    = ROM_Header[4];
	CHR    = ROM_Header[5];
	MAPPER = (ROM_Header[6] >> 4) | (ROM_Header[7] & 0x0F);
	RCB    = (ROM_Header[6] - ((ROM_Header[6] >> 4) << 4));

	switch (RCB) {
		case 0x00:
			/* horizontal mirroring only */
			MIRRORING = 0;
			SRAM = 0;
			TRAINER = 0;
			FS_MIRROR = 0;
		break;

		case 0x01:
			/* vertical mirroring only */
			MIRRORING = 1;
			SRAM = 0;
			TRAINER = 0;
			FS_MIRROR = 0;
		break;

		case 0x02:
			/* horizontal mirroring and sram enabled */
			MIRRORING = 0;
			SRAM = 1;
			TRAINER = 0;
			FS_MIRROR = 0;
		break;

		case 0x03:
			/* vertical mirroring and sram enabled */
			MIRRORING = 1;
			SRAM = 1;
			TRAINER = 0;
			FS_MIRROR = 0;
		break;

		case 0x04:
			/* horizontal mirroring and trainer on */
			MIRRORING = 0;
			SRAM = 0;
			TRAINER = 1;
			FS_MIRROR = 0;
		break;

		case 0x05:
			/* vertical mirroring and trainer on */
			MIRRORING = 1;
			SRAM = 0;
			TRAINER = 1;
			FS_MIRROR = 0;
		break;

		case 0x06:
			/* horizontal mirroring, sram enabled and trainer on */
			MIRRORING = 0;
			SRAM = 1;
			TRAINER = 1;
			FS_MIRROR = 0;
		break;

		case 0x07:
			/* vertical mirroring, sram enabled and trainer on */
			MIRRORING = 1;
			SRAM = 1;
			TRAINER = 1;
			FS_MIRROR = 0;
		break;

		case 0x08:
			/* horizontal mirroring and four screen vram on */
			MIRRORING = 0;
			SRAM = 0;
			TRAINER = 0;
			FS_MIRROR = 1;
		break;

		case 0x09:
			/* vertical mirroring and four screen vram on */
			MIRRORING = 1;
			SRAM = 0;
			TRAINER = 0;
			FS_MIRROR = 1;
		break;

		case 0x0A:
			/* horizontal mirroring, sram enabled and four screen vram on */
			MIRRORING = 0;
			SRAM = 1;
			TRAINER = 0;
			FS_MIRROR = 1;
		break;

		case 0x0B:
			/* vertical mirroring, sram enabled and four screen vram on */
			MIRRORING = 1;
			SRAM = 1;
			TRAINER = 0;
			FS_MIRROR = 1;
		break;

		case 0x0C:
			/* horizontal mirroring, trainer on and four screen vram on */
			MIRRORING = 0;
			SRAM = 0;
			TRAINER = 1;
			FS_MIRROR = 1;
		break;

		case 0x0D:
			/* vertical mirroring, trainer on and four screen vram on */
			MIRRORING = 1;
			SRAM = 0;
			TRAINER = 1;
			FS_MIRROR = 1;
		break;

		case 0x0E:
			/* horizontal mirroring, sram enabled, trainer on and four screen vram on */
			MIRRORING = 0;
			SRAM = 1;
			TRAINER = 1;
			FS_MIRROR = 1;
		break;

		case 0x0F:
			/* vertical mirroring, sram enabled, trainer on and four screen vram on */
			MIRRORING = 1;
			SRAM = 1;
			TRAINER = 1;
			FS_MIRROR = 1;
		break;

		default:
			// TODO: Err Msg
			return -1;
		break;

	}


	/* load prg data in memory */
	if(PRG == 0x01) {
		/* map 16kb in mirror mode */
		memcpy(memory + 0x8000, ROM_Cache + 16, 16384);
		memcpy(memory + 0xC000, ROM_Cache + 16, 16384);
	} else {
		/* map 2x 16kb the first one into 8000 and the last one into c000 */
		memcpy(memory + 0x8000, ROM_Cache + 16, 16384);
		memcpy(memory + 0xC000, ROM_Cache + 16 + ((PRG - 1) * 16384), 16384);
	}
	/* load chr data in ppu memory */
	if(CHR != 0x00) {
		memcpy(PPU_Memory, ROM_Cache + 16 + (PRG * 16384), 8192);

		/* fetch title from last 128 bytes */
		memcpy(ROM_Title, ROM_Cache + 16 + (PRG * 16384) + 8192, 128);

	}

	return 0;
}