#ifndef NESGLOBAL_H
#define NESGLOBAL_H

#include <stdbool.h>
#include <3ds.h>


/***********************************************
** All global defines will be declared here   **
************************************************/

#define BUTTON_A 1
#define BUTTON_B 2
#define BUTTON_UP 64
#define BUTTON_LEFT 32
#define BUTTON_DOWN 128
#define BUTTON_RIGHT 16
#define BUTTON_X 1024
#define BUTTON_Y 2048
#define BUTTON_L1 512
#define BUTTON_R1 256
#define BUTTON_START 8
#define BUTTON_SELECT 4

/* memory[0x2000] */
#define exec_nmi_on_vblank	(ppu_control1 & 0x80) /* 1 = Generate VBlank NMI */
#define sprite_16		(ppu_control1 & 0x20) /* 1 = Sprites 8x16/8x8 */
#define background_addr_hi	(ppu_control1 & 0x10) /* 1 = BG pattern adr $0000/$1000 */
#define sprite_addr_hi		(ppu_control1 & 0x08) /* 1 = Sprite pattern adr $0000/$1000 */
#define increment_32		(ppu_control1 & 0x04) /* 1 = auto increment 1/32 */

/* memory[0x2001] */
#define sprite_on		(ppu_control2 & 0x10) /* 1 = Show sprite */
#define background_on		(ppu_control2 & 0x08) /* 1 = Show background */
#define sprite_clipping_off	(ppu_control2 & 0x04) /* 1 = 1 = No clipping */
#define background_clipping_off	(ppu_control2 & 0x02) /* 1 = 1 = No clipping */
#define monochrome_on		(ppu_control2 & 0x01) /* 1 = Display monochrome */

/* memory[0x2002] */
#define vblank_on		(ppu_status & 0x80) /* 1 = In VBlank */
#define sprite_zero		(ppu_status & 0x40) /* 1 = PPU has hit Sprite #0 */
#define scanline_sprite_count	(ppu_status & 0x20) /* 1 = More than 8 sprites on current scanline */
#define vram_write_flag		(ppu_status & 0x10) /* 1 = Writes to VRAM are ignored */


/***********************************************
** All global variables will be declared here **
************************************************/

extern u8 					*ROM_Cache;
extern u8 					*PPU_Memory;
extern u8 					*SPRITE_Memory;
extern u8					*SRAM_Name;
extern u8					frameSkip;
extern u8					skipFrame;
extern u8					ROM_Title[128];

extern u32 					PAD1_Data;
extern u32					ROM_Size;
extern u32				    line_ticks;

extern bool					CPU_Running;
extern bool 				PAUSE_Emulation;
extern bool					ENABLE_Background;
extern bool					ENABLE_Sprite;
extern bool					inGame;

#endif