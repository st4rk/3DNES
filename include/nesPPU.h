
#include <stdbool.h>
#include <3ds.h>
#include <3ds/gfx.h>

#include "nesGlobal.h"
#include "nes6502.h"


extern u32 ppu_control1;
extern u32 ppu_control2;
extern u32 ppu_addr;
extern u32 ppu_addr_h;
extern u32 ppu_addr_tmp;
extern u32 ppu_status;
extern u32 ppu_status_tmp;
extern u32 ppu_bgscr_f;

extern int current_scanline;

extern u32 sprite_address;

extern u32 loopyT;
extern u32 loopyV;
extern u32 loopyX;

extern void init_ppu();
extern void show_gfxcache();
extern void write_PPU_Memory(u32 address,u8 data);
extern void render_scanline(int scanline);
extern void render_background(int scanline);
extern void render_sprite(int scanline,bool foreground);
extern void NES_ColorBackground();
extern void do_mirror(int type);