

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "functions.h"
#include "3dnes.h"
#include "ppu.h"
#include "macros.h"
#include "romloader.h"
#include "palette.h"
#include "6502core.h"

/* FONT, BACKGROUND */
#include "imgdata.h"
#include "background.h"

/* CTR-LIB */
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/APT.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/HID.h>
#include <ctr/svc.h>


/* ppu control registers */
u32 ppu_control1 = 0x00;
u32 ppu_control2 = 0x00;
u32 ppu_latch = 0x00;

/* used to flip between first and second write (0x2006) */
u32 ppu_addr_h = 0x00;

u8 nt[4][0x400];

/* used for scrolling techniques */
u32 loopyT = 0x00;
u32 loopyV = 0x00;
u32 loopyX = 0x00;

u32 ppu_addr = 0x2000;
u32 ppu_addr_tmp = 0x2000;

/* ppu status/temp registers */
u32 ppu_status;
u32 ppu_status_tmp = 0x00;

u32 sprite_address = 0x00;

/* used to flip between first and second write (0x2005) */
u32 ppu_bgscr_f = 0x00;

u8 color_lookup[0x80000];

/* Pow 2 Table */
int pow2[32];

int mirror[4];

/* used to export the current scanline for the debugger */
int current_scanline;

/* 3DS gspGPU */
u8* gspHeap;
u32* gxCmdBuf;

u8 currentBuffer; 
u8* topLeftFramebuffers[2]; /* The 3DS Framebuffer's */

Handle gspEvent, gspSharedMemHandle;


void swapBuffers() {
    u32 regData;
    GSPGPU_ReadHWRegs(NULL, 0x400478, (u32*)&regData, 4);
    regData^=1;
    currentBuffer=regData&1;
    GSPGPU_WriteHWRegs(NULL, 0x400478, (u32*)&regData, 4);
}

void copyBuffer() {
    //copy topleft FB
    u8 copiedBuffer=currentBuffer^1;
    u8* bufAdr=&gspHeap[0x46500*copiedBuffer];
    GSPGPU_FlushDataCache(NULL, bufAdr, 0x46500);

    GX_RequestDma(gxCmdBuf, (u32*)bufAdr, (u32*)topLeftFramebuffers[copiedBuffer], 0x46500);
}


void gspGpuInit() {
    gspInit();

    GSPGPU_AcquireRight(NULL, 0x0);
    GSPGPU_SetLcdForceBlack(NULL, 0x0);

    //grab main left screen framebuffer addresses
    GSPGPU_ReadHWRegs(NULL, 0x400468, (u32*)&topLeftFramebuffers, 8);

    //convert PA to VA (assuming FB in VRAM)
    topLeftFramebuffers[0]+=0x7000000;
    topLeftFramebuffers[1]+=0x7000000;

    //setup our gsp shared mem section
    u8 threadID;
    svc_createEvent(&gspEvent, 0x0);
    GSPGPU_RegisterInterruptRelayQueue(NULL, gspEvent, 0x1, &gspSharedMemHandle, &threadID);
    svc_mapMemoryBlock(gspSharedMemHandle, 0x10002000, 0x3, 0x10000000);

    //map GSP heap
    svc_controlMemory((u32*)&gspHeap, 0x0, 0x0, 0x2000000, 0x10003, 0x3);

    //wait until we can write stuff to it
    svc_waitSynchronization1(gspEvent, 0x55bcb0);

    //GSP shared mem : 0x2779F000
    gxCmdBuf=(u32*)(0x10002000+0x800+threadID*0x200);

    currentBuffer=0;
}

void gspGpuExit() {
    GSPGPU_UnregisterInterruptRelayQueue(NULL);

    //unmap GSP shared mem
    svc_unmapMemoryBlock(gspSharedMemHandle, 0x10002000);
    svc_closeHandle(gspSharedMemHandle);
    svc_closeHandle(gspEvent);
    
    gspExit();

    //free GSP heap
    svc_controlMemory((u32*)&gspHeap, (u32)gspHeap, 0x0, 0x2000000, MEMOP_FREE, 0x0);
}

void init_ppu() {
    //Otimização para renderizar mais rápido
    int b1;
    int b2;
    int X;
    int c;

    for (b1 = 0; b1 < 256; b1++) {
        for (b2 = 0; b2 < 256; b2++) {
            for (X = 0; X < 8; X++) {
                if (b1 & (1 << X)) {c = 1;} else {c = 0;}
                if (b2 & (1 << X)) {c += 2;}
                color_lookup[b1 * 2048 + b2 * 8 + X] = c;
            }
        }
    }

    //Tabela com múltiplos de 2
    int t_var = 1;
    int i;
    for (i = 0; i < 31; i++) {
        if(i == 0)
            pow2[i] = 1;
        else {
            t_var = t_var * 2;
            pow2[i] = t_var;
        }
    }
    pow2[31] = -2147483648;

}

void write_ppu_memory(unsigned int address,unsigned char data) {

    if(address == 0x2000) {
        ppu_addr_tmp = data;

        ppu_control1 = data;

        memory[address] = data;

        loopyT &= 0xf3ff; // (0000110000000000)
        loopyT |= (data & 3) << 10; // (00000011)

        return;
        }

    if(address == 0x2001) {
        ppu_addr_tmp = data;

        ppu_control2 = data;
        memory[address] = data;

        return;
        }

    /* sprite_memory address register */
    if(address == 0x2003) {
        ppu_addr_tmp = data;

        sprite_address = data;
        memory[address] = data;

        return;
    }

    /* sprite_memory i/o register */
    if(address == 0x2004) {
        ppu_addr_tmp = data;

        sprite_memory[sprite_address] = data;
        sprite_address++;

        memory[address] = data;
        return;
    }

    /* vram address register #1 (scrolling) */
    if(address == 0x2005) {
        ppu_addr_tmp = data;

        if(ppu_bgscr_f == 0x00) {
            loopyT &= 0xFFE0; // (0000000000011111)
            loopyT |= (data & 0xF8) >> 3; // (11111000)
            loopyX = data & 0x07; // (00000111)

            ppu_bgscr_f = 0x01;

            memory[address] = data;

            return;
        }

        if(ppu_bgscr_f == 0x01) {
            loopyT &= 0xFC1F; // (0000001111100000)
            loopyT |= (data & 0xF8) << 2; //(0111000000000000)
            loopyT &= 0x8FFF; //(11111000)
            loopyT |= (data & 0x07) << 12; // (00000111)

            ppu_bgscr_f = 0x00;

            memory[address] = data;

            return;
        }
    }

    /* vram address register #2 */
    if(address == 0x2006) {
        ppu_addr_tmp = data;

        /* First write -> Store the high byte 6 bits and clear out the last two */
        if(ppu_addr_h == 0x00) {
            loopyT &= 0x00FF; // (0011111100000000)
            loopyT |= (data & 0x3F) << 8; // (1100000000000000) (00111111)

            ppu_addr_h = 0x01;

            memory[address] = data;

            return;
        }

        /* Second write -> Store the low byte 8 bits */
        if(ppu_addr_h == 0x01) {
            loopyT &= 0xFF00; // (0000000011111111)
            loopyT |= data; // (11111111)
            loopyV = loopyT; // v=t
            ppu_addr = loopyV;

            ppu_addr_h = 0x00;

            memory[address] = data;

            return;
        }
    }

    /* vram i/o register */
    if(address == 0x2007) {
        /* if the vram_write_flag is on, vram writes should ignored */
        if(vram_write_flag) return;

        ppu_addr_tmp = data;

        ppu_memory[ppu_addr] = data;

        /* nametable mirroring */
        if((ppu_addr >= 0x2000) && (ppu_addr <= 0x3EFF)) {
            if (ppu_addr >= 0x3000) {ppu_addr &= 0xEFFF;}
            nt[mirror[(ppu_addr & 0xC00) / 0x400]][ppu_addr & 0x3FF] = data;
        }

        /* palette mirror */
        if(ppu_addr == 0x3f10) {
            ppu_memory[0x3f00] = data;
        }

        ppu_addr_tmp = ppu_addr;

        if(!increment_32) {
            ppu_addr++;
        } else {
            ppu_addr += 0x20;
        }
        memory[address] = data;
        return;
    }

    /* transfer 256 bytes of memory into sprite_memory */
    if(address == 0x4014) {memcpy(sprite_memory, memory + 0x100 * data, 256);}

    return;
}


void render_scanline(int scanline) {
    current_scanline = scanline;

    if (scanline == 0) {
        loopyV = loopyT;
    } else {
        loopyV &= 0xfbe0;
        loopyV |= (loopyT & 0x041f);
    }

    //Desenha Background e sprites
   // if (sprite_on) {render_sprite(scanline, true);} // Disable Speed Issues
    if (background_on && (skipframe == 0)) {render_background(scanline);}
    if (sprite_on) {render_sprite(scanline, false);}

    if((loopyV & 0x7000) == 0x7000) { /* subtile y_offset == 7 */
        loopyV &= 0x8fff; /* subtile y_offset = 0 */
        if((loopyV & 0x03e0) == 0x03a0) { /* nametable line == 29 */
            loopyV ^= 0x0800; /* switch nametables (bit 11) */
            loopyV &= 0xfc1f; /* name table line = 0 */
        } else {
            if((loopyV & 0x03e0) == 0x03e0) { /* nametable line == 31 */
                loopyV &= 0xfc1f; /* name table line = 0 */
            } else {
                loopyV += 0x0020;
            }
        }
    } else {
        loopyV += 0x1000; /* next subtile y_offset */
    }
}

/* Draw Pixel for NES */
void draw_pixel(int x, int y, int nescolor) {
    /* don't fail on attempts to draw outside the screen. */
    if ((x>=256) || (x<0)) {return;}
    if ((y>=240) || (y<0)) {return;}

    u8* bufAdr=&gspHeap[0x46500*currentBuffer];
    y = 240-y;
    x = 72+x;
    u32 v=(y+x*240)*3;

    bufAdr[v]=palette[nescolor].b;
    bufAdr[v+1]=palette[nescolor].g;
    bufAdr[v+2]=palette[nescolor].r;
}


/* draw pixel RGB Format */
void draw_pixel_rgb(int x, int y, u8 r, u8 g, u8 b) {
    /* don't fail on attempts to draw outside the screen. */

    u8* bufAdr=&gspHeap[0x46500*currentBuffer];
    y = 240-y;
    x = 72+x;
    u32 v=(y+x*240)*3;

    bufAdr[v]=b;
    bufAdr[v+1]=g;
    bufAdr[v+2]=r;;
}

/* Draw String */
void draw_string(int sx, int sy, unsigned char str[]) {
    int i;
    for (i = 0; i < strlen(str); i++) {
        int fntnum = (str[i] - 32) & 0xFF;
        int y;
        for (y = 0; y < 8; y++) {
            int currbyte = fonts[(fntnum * 8) + y];

            //Desenha sprite de 1BPP
            int x;
            int mult = 0x80;
            for(x = 0; x < 8; x++) {
                if ((currbyte & mult) == mult) {
                    draw_pixel_rgb(sx + x, sy + y, 0xFF, 0xFF, 0xFF);
                    draw_pixel_rgb(sx + x, sy + y + 1, 0, 0, 0); //Sombra
                }
                mult /= 2;
            }
        }
        sx += 8;
    }
}

/* Draw Text on the mid */
void draw_string_c(int sy, unsigned char str[]) {
    int sx = (240 / 2) - ((strlen(str) * 8) / 2);
    draw_string(sx, sy, str);
}


/* Render Background */
void render_background(int scanline) {
    
    int hscroll = ((loopyV & 31) << 3) + loopyX;
    int vscroll =  (((loopyV >> 5) & 31) << 3) | ((loopyV / 0x1000) & 7);

    int tilerow = (vscroll >> 3) % 30;
    int tileyoffset = vscroll & 7;

    int ptaddr = 0;
    if(ppu_control1 & 0x10) ptaddr = 0x1000;

    int nt_addr = 0x2000 + (loopyV & 0xC00);
    int nt_num = (nt_addr & 0xC00) / 0x400;

    int tilecount;
    for (tilecount = (hscroll >> 3); tilecount < 32; tilecount++) {
        int tilex = (tilecount << 3) - hscroll + 7;
        int offset;
        if (tilex < 7) {offset = tilex;} else {offset = 7;}
        int tileindex = nt[mirror[nt_num]][tilecount + (tilerow << 5)];
        int lookup = nt[mirror[nt_num]][0x3C0 + (tilecount >> 2) + ((tilerow >> 2) << 3)];
        int bgpal = 0;
        switch ((tilecount & 2) | ((tilerow & 2) << 1)) {
            case 0: bgpal = (lookup << 2) & 12; break;
            case 2: bgpal = lookup  & 12; break;
            case 4: bgpal = (lookup >> 2) & 12; break;
            case 6: bgpal = (lookup >> 4) & 12; break;
        }

        int bgtileoffset = ptaddr + (tileindex << 4);
        
        if (tileyoffset == 0) {
            int tiley;
            for(tiley = 0; tiley < 8; tiley++) { //Desenho padrão de sprite 2BPP 8x8
                unsigned char lowbyte = ppu_memory[bgtileoffset + tiley];
                unsigned char highbyte = ppu_memory[bgtileoffset + tiley + 8];
                int clookup_offset = lowbyte * 0x800 + (highbyte << 3);
                int curcol;
                for(curcol = 0; curcol <= offset; curcol++) {
                    int pcolor = color_lookup[clookup_offset + curcol];
                    if ((pcolor & 0x3) != 0) {
                        draw_pixel(tilex - curcol, scanline, ppu_memory[0x3F00 + (pcolor | bgpal)]);
                    }
                }
                if (tilex <= 61183) {tilex += 256;}
            }
        } else {
            unsigned char lowbyte = ppu_memory[bgtileoffset + tileyoffset];
            unsigned char highbyte = ppu_memory[bgtileoffset + tileyoffset + 8];
            int clookup_offset = lowbyte * 0x800 + (highbyte << 3);
            int curcol;
            for(curcol = 0; curcol <= offset; curcol++) {
                int pcolor = color_lookup[clookup_offset + curcol];
                if ((pcolor & 0x3) != 0) {
                    draw_pixel(tilex - curcol, scanline, ppu_memory[0x3F00 + (pcolor | bgpal)]);
                  }
            }
        }
    }

    nt_addr ^= 0x400;
    nt_num = (nt_addr & 0xC00) / 0x400;

    /* FINISHED HERE */

    for (tilecount = 0; tilecount <= (hscroll >> 3); tilecount++) {
        int tilex = (tilecount << 3) + 256 - hscroll + 7;
        int offset;
        if (tilex > 255) {offset = tilex - 255;} else {offset = 0;}
        int tileindex = nt[mirror[nt_num]][tilecount + (tilerow << 5)];
        int lookup = nt[mirror[nt_num]][0x3C0 + (tilecount >> 2) + ((tilerow >> 2) << 3)];
        int bgpal = 0;
        switch ((tilecount & 2) | ((tilerow & 2) << 1)) {
            case 0: bgpal = (lookup << 2) & 12; break;
            case 2: bgpal = lookup  & 12; break;
            case 4: bgpal = (lookup >> 2) & 12; break;
            case 6: bgpal = (lookup >> 4) & 12; break;
        }
        int bgtileoffset = ptaddr + (tileindex << 4);
        if (tileyoffset == 0) {
            int tiley;
            for(tiley = 0; tiley < 8; tiley++) { //Desenho padrão de sprite 2BPP 8x8
                unsigned char lowbyte = ppu_memory[bgtileoffset + tiley];
                unsigned char highbyte = ppu_memory[bgtileoffset + tiley + 8];
                int clookup_offset = lowbyte * 0x800 + (highbyte << 3);
                int curcol;
                for(curcol = offset; curcol < 8; curcol++) {
                    int pcolor = color_lookup[clookup_offset + curcol];
                    if ((pcolor & 0x3) != 0) {
                        draw_pixel(tilex - curcol, scanline, ppu_memory[0x3F00 + (pcolor | bgpal)]);
                    }
                }
                if (tilex <= 61183) {tilex += 256;}
            }
        } else {
            unsigned char lowbyte = ppu_memory[bgtileoffset + tileyoffset];
            unsigned char highbyte = ppu_memory[bgtileoffset + tileyoffset + 8];
            int clookup_offset = lowbyte * 0x800 + (highbyte << 3);
            int curcol;
            for(curcol = offset; curcol < 8; curcol++) {
                int pcolor = color_lookup[clookup_offset + curcol];
                if ((pcolor & 0x3) != 0) {
                    draw_pixel(tilex - curcol, scanline, ppu_memory[0x3F00 + (pcolor | bgpal)]);
                }
            }
        }
    }
}

/* render sprites 16x16 8x8 */
void render_sprite(int scanline, bool foreground) {
    int currspr;
    for (currspr = 252; currspr >= 0; currspr-=4) {
        int spry = sprite_memory[currspr] + 1;
        int tileindex = sprite_memory[currspr + 1];
        int attr = sprite_memory[currspr + 2];
        int sprx = sprite_memory[currspr + 3];

        int ptaddr = 0;
        if(sprite_addr_hi) {ptaddr = 0x1000;}
        int tileh;
        if (sprite_16) {tileh = 16;} else {tileh = 8;}

     //   bool ontop = attr & 32;
        int sprpal = 16 + ((attr & 3) << 2);
        bool vflip = attr & 128;
        bool hflip = attr & 64;

        int pcolor;
        //&& (ontop == foreground)
        if ((spry <= scanline) && (spry + tileh > scanline)) {
            if (!sprite_16) { //8x8
                int scan_to_draw;
                if (!vflip) {scan_to_draw = scanline - spry;} else {scan_to_draw = spry + 7 - scanline;}

                int sproffset = ptaddr + (tileindex * 16);
                unsigned char lowbyte = ppu_memory[sproffset + scan_to_draw];
                unsigned char highbyte = ppu_memory[sproffset + scan_to_draw + 8];
                int currpix;
                if (!hflip) {
                    sprx += 7;
                    for (currpix = 7; currpix >= 0; currpix--) {
                        if (lowbyte & pow2[currpix]) pcolor = 1; else pcolor = 0;
                        if (highbyte & pow2[currpix]) pcolor += 2;
                        if ((pcolor & 0x3) != 0) {
                            draw_pixel(sprx - currpix, scanline, ppu_memory[0x3F00 + (pcolor | sprpal)]);
                            if (currspr == 0) {ppu_status |= 0x40;} //Sprite 0 Hit Flag
                        }
                    }
                } else {
                    for (currpix = 0; currpix < 8; currpix++) {
                        if (lowbyte & pow2[currpix]) pcolor = 1; else pcolor = 0;
                        if (highbyte & pow2[currpix]) pcolor += 2;
                        if ((pcolor & 0x3) != 0) {
                            draw_pixel(sprx + currpix, scanline, ppu_memory[0x3F00 + (pcolor | sprpal)]);
                            if (currspr == 0) {ppu_status |= 0x40;} //Sprite 0 Hit Flag
                        }
                    }
                }

            } else { //8x16
                int scan_to_draw;
                if (!vflip) {scan_to_draw = scanline - spry;} else {scan_to_draw = spry + 15 - scanline;}
                int sproffset;
                if (scan_to_draw < 8) {
                    if (tileindex % 2 == 0) {sproffset = (tileindex << 4);} else {sproffset = 0x1000 + ((tileindex - 1) << 4);}
                } else {
                    scan_to_draw -= 8;
                    if (tileindex % 2 == 0) {sproffset = ((tileindex + 1) << 4);} else {sproffset = 0x1000 + (tileindex << 4);}
                }
                unsigned char lowbyte = ppu_memory[sproffset + scan_to_draw];
                unsigned char highbyte = ppu_memory[sproffset + scan_to_draw + 8];
                int currpix;
                if (!hflip) {
                    sprx += 7;
                    for (currpix = 7; currpix >= 0; currpix--) {
                        if (lowbyte & pow2[currpix]) pcolor = 1; else pcolor = 0;
                        if (highbyte & pow2[currpix]) pcolor += 2;
                        if ((pcolor & 0x3) != 0) {
                            draw_pixel(sprx - currpix, scanline, ppu_memory[0x3F00 + (pcolor | sprpal)]);
                            if (currspr == 0) {ppu_status |= 0x40;} //Sprite 0 Hit Flag
                        }
                    }
                } else {
                    for (currpix = 0; currpix < 8; currpix++) {
                        if (lowbyte & pow2[currpix]) pcolor = 1; else pcolor = 0;
                        if (highbyte & pow2[currpix]) pcolor += 2;
                        if ((pcolor & 0x3) != 0) {
                            draw_pixel(sprx + currpix, scanline, ppu_memory[0x3F00 + (pcolor | sprpal)]);
                            if (currspr == 0) {ppu_status |= 0x40;} //Sprite 0 Hit Flag
                        }
                    }
                }
            }
        }
    }
}

/* Update and Clear the background */
void update_screen() {
    int nescolor = ppu_memory[0x3f00];

    
    swapBuffers();
    copyBuffer();
    u8* bufAdr=&gspHeap[0x46500*currentBuffer];
 
    memset(bufAdr + 51840, palette[nescolor].b << 16 | palette[nescolor].g << 8 | palette[nescolor].r, 186160);
	
}

/* It will clear screen when the game Start */
void clearScreen() {
    swapBuffers();
    copyBuffer();
    u8* bufAdr=&gspHeap[0x46500*currentBuffer];
    memset(bufAdr, 0, 288000);
}

/* update menu image */
void updateMenu() {
    swapBuffers();
    copyBuffer();

    u8* bufAdr=&gspHeap[0x46500*currentBuffer];

    memcpy(bufAdr, imagem,288000);
}


u8 getTopFrameBuffer() {
    u8* bufAdr=&gspHeap[0x46500 * currentBuffer];

    return bufAdr;
}


void do_mirror(int type) {
    switch (type) {
        case 0: mirror[0] = 0; mirror[1] = 0; mirror[2] = 1; mirror[3] = 1; break; //H
        case 1: mirror[0] = 0; mirror[1] = 1; mirror[2] = 0; mirror[3] = 1; break; //V
        case 2: mirror[0] = 0; mirror[1] = 0; mirror[2] = 0; mirror[3] = 0; break; //OS1
        case 3: mirror[0] = 1; mirror[1] = 1; mirror[2] = 1; mirror[3] = 1; break; //OS2
        case 4: mirror[0] = 0; mirror[1] = 1; mirror[2] = 2; mirror[3] = 3; break; //FS
    }
}

