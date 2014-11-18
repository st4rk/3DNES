
#include <stdbool.h>
#include <3ds.h>
#include <3ds/gfx.h>

#include "nesGlobal.h"
#include "6502core.h"


u32 ppu_control1;
u32 ppu_control2;
u32 ppu_addr;
u32 ppu_addr_h;
u32 ppu_addr_tmp;
u32 ppu_status;
u32 ppu_status_tmp;
u32 ppu_bgscr_f;

int current_scanline;

u32 sprite_address;

u32 loopyT;
u32 loopyV;
u32 loopyX;

void init_ppu();
void show_gfxcache();
void write_ppu_memory(unsigned int address,unsigned char data);
void render_scanline(int scanline);
void render_background(int scanline);
void render_sprite(int scanline,bool foreground);
void update_screen();
void do_mirror(int type);
void N3DS_DrawPixel();
void N3DS_SwapBuffers();
