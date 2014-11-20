#include "nesPPU.h"

/* FONT, BACKGROUND */
#include "gfx/imgdata.h"
#include "gfx/background.h"
#include "gfx/bar.h"

#include "palette.h"

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

int mirror[4];

/* used to export the current scanline for the debugger */
int current_scanline;


/* Screen FrameBuffer */
u8* topLeftFrameBuffer 	= 0;
u8* topRightFrameBuffer = 0;
u8* bottomFrameBuffer   = 0;

void drawBuffers() {
    gfxFlushBuffers();
    gfxSwapBuffers();

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


    topLeftFrameBuffer  = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    topRightFrameBuffer = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
   	bottomFrameBuffer   = gfxGetFramebuffer(GFX_BOTTOM, GFX_BOTTOM, NULL, NULL);

}

void write_PPU_Memory(unsigned int address,unsigned char data) {

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

    /* SPRITE_Memory address register */
    if(address == 0x2003) {
        ppu_addr_tmp = data;

        sprite_address = data;
        memory[address] = data;

        return;
    }

    /* SPRITE_Memory i/o register */
    if(address == 0x2004) {
        ppu_addr_tmp = data;

        SPRITE_Memory[sprite_address] = data;
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

        PPU_Memory[ppu_addr] = data;

        /* nametable mirroring */
        if((ppu_addr >= 0x2000) && (ppu_addr <= 0x3EFF)) {
            if (ppu_addr >= 0x3000) {ppu_addr &= 0xEFFF;}
            nt[mirror[(ppu_addr & 0xC00) / 0x400]][ppu_addr & 0x3FF] = data;
        }

        /* palette mirror */
        if(ppu_addr == 0x3f10) {
            PPU_Memory[0x3f00] = data;
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

    /* transfer 256 bytes of memory into SPRITE_Memory */
    if(address == 0x4014) {memcpy(SPRITE_Memory, memory + 0x100 * data, 256);}

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
    if (background_on && (skipFrame == 0)) {render_background(scanline);}
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

    u8* framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    y = 240-y;
    x = 72+x;
    u32 v=(y+x*240)*3;
   
    framebuffer[v]=palette[nescolor].b;
    framebuffer[v+1]=palette[nescolor].g;
    framebuffer[v+2]=palette[nescolor].r;

}


/* draw pixel RGB Format */
void draw_pixel_rgb(int x, int y, u8 r, u8 g, u8 b) {


    y = 240-y;
    x = 72+x;
    u32 v=(y+x*240)*3;

    u8* topLeftFB=gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

    topLeftFB[v]	=b;
    topLeftFB[v+1]	=g;
    topLeftFB[v+2]	=r;

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


void draw_image_24bpp(int sx, int sy, int w, int h, char img[]) {
    int x, y, i;
    i = 0;
    for (x = 0; x < w; x++) {
        for (y = h; y > 0; y--) {
            if (img[i] + img[i + 1] + img[i + 2] != 0) { //Trata preto como transparente
                draw_pixel_rgb(sx + x, sy + y, img[i + 2], img[i + 1], img[i]);
            }
            i += 3;
        }
    }
}

/* Draw Select Bar */

void draw_select_bar(int x, int y) {
  //  draw_image_24bpp(x, y, 392, 12, select_bar);
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
    int fPix = 0;
    int tilecount;
    for (tilecount = (hscroll >> 3); tilecount < 32; tilecount++) {
        int tilex = (tilecount << 3) - hscroll + 7;
                    if (tilex >= 7 && tilex <= 262) {
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
                                            unsigned char lowbyte = PPU_Memory[bgtileoffset + tiley];
                                            unsigned char highbyte = PPU_Memory[bgtileoffset + tiley + 8];
                                       
                                            int curcol;
                                            for(curcol = 0; curcol <= offset; curcol++) {
                                                    int pcolor = 0;
                                                    if(curcol == 0)  
                                                        fPix = 1;
                                                    else
                                                        fPix = (2 << (curcol - 1));
                                                    
                                                    if (lowbyte & fPix) pcolor = 1; else pcolor = 0;
                                                    if (highbyte & fPix) {pcolor += 2;}
                                                    if ((pcolor & 0x3) != 0) {
                                                            draw_pixel(tilex - curcol, scanline, PPU_Memory[0x3F00 + (pcolor | bgpal)]);
                                                    }
                                            }
                                            if (tilex <= 61183) {tilex += 256;}
                                    }
                            } else {
                                    unsigned char lowbyte = PPU_Memory[bgtileoffset + tileyoffset];
                                    unsigned char highbyte = PPU_Memory[bgtileoffset + tileyoffset + 8];
                               
                                    int curcol;
                                    for(curcol = 0; curcol <= offset; curcol++) {
                                            int pcolor = 0;
                                            if(curcol == 0)  
                                                fPix = 1;
                                            else
                                                fPix = (2 << (curcol - 1));
                                            
                                            if (lowbyte & fPix) pcolor = 1; else pcolor = 0;
                                            if (highbyte & fPix) {pcolor += 2;}
                                            if ((pcolor & 0x3) != 0) {
                                                    draw_pixel(tilex - curcol, scanline, PPU_Memory[0x3F00 + (pcolor | bgpal)]);
                                              }
                                    }
                            }
                    }
    }
 
    nt_addr ^= 0x400;
    nt_num = (nt_addr & 0xC00) / 0x400;
 
    /* FINISHED HERE */
 
    for (tilecount = 0; tilecount <= (hscroll >> 3); tilecount++) {
        int tilex = (tilecount << 3) + 256 - hscroll + 7;
                    if (tilex >= 7 && tilex <= 262) {
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
                                            unsigned char lowbyte = PPU_Memory[bgtileoffset + tiley];
                                            unsigned char highbyte = PPU_Memory[bgtileoffset + tiley + 8];
                                       
                                            int curcol;
                                            for(curcol = offset; curcol < 8; curcol++) {
                                                    int pcolor = 0;
                                                    if(curcol == 0)  
                                                        fPix = 1;
                                                    else
                                                        fPix = (2 << (curcol - 1));
                                                    
                                                    if (lowbyte & fPix) pcolor = 1; else pcolor = 0;
                                                    if (highbyte & fPix) {pcolor += 2;}
                                                    if ((pcolor & 0x3) != 0) {
                                                            draw_pixel(tilex - curcol, scanline, PPU_Memory[0x3F00 + (pcolor | bgpal)]);
                                                    }
                                            }
                                            if (tilex <= 61183) {tilex += 256;}
                                    }
                            } else {
                                    unsigned char lowbyte = PPU_Memory[bgtileoffset + tileyoffset];
                                    unsigned char highbyte = PPU_Memory[bgtileoffset + tileyoffset + 8];
                               
                                    int curcol;
                                    for(curcol = offset; curcol < 8; curcol++) {
                                            int pcolor = 0;
                                            if(curcol == 0)  
                                                fPix = 1;
                                            else
                                                fPix = (2 << (curcol - 1));
                                            
                                            if (lowbyte & fPix) pcolor = 1; else pcolor = 0;
                                            if (highbyte & fPix) {pcolor += 2;}
                                            if ((pcolor & 0x3) != 0) {
                                                    draw_pixel(tilex - curcol, scanline, PPU_Memory[0x3F00 + (pcolor | bgpal)]);
                                            }
                                    }
                            }
                    }
    }
}

/* render sprites 16x16 8x8 */
void render_sprite(int scanline, bool foreground) {
    int currspr;
    for (currspr = 252; currspr >= 0; currspr-=4) {
        int spry = SPRITE_Memory[currspr] + 1;
        int tileindex = SPRITE_Memory[currspr + 1];
        int attr = SPRITE_Memory[currspr + 2];
        int sprx = SPRITE_Memory[currspr + 3];
        register int fPix = 0;
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
                unsigned char lowbyte = PPU_Memory[sproffset + scan_to_draw];
                unsigned char highbyte = PPU_Memory[sproffset + scan_to_draw + 8];
                int currpix;
                if (!hflip) {
                    sprx += 7;
                    for (currpix = 7; currpix >= 0; currpix--) {
                        if(currpix == 0)  
                            fPix = 1;
                        else
                            fPix = (2 << (currpix - 1));

                        if (lowbyte & fPix) pcolor = 1; else pcolor = 0;
                        if (highbyte & fPix) pcolor += 2;
                        if ((pcolor & 0x3) != 0) {
                            draw_pixel(sprx - currpix, scanline, PPU_Memory[0x3F00 + (pcolor | sprpal)]);
                            if (currspr == 0) {ppu_status |= 0x40;} //Sprite 0 Hit Flag
                        }
                    }
                } else {
                    for (currpix = 0; currpix < 8; currpix++) {
                        if(currpix == 0)  
                            fPix = 1;
                        else
                            fPix = (2 << (currpix - 1));

                        if (lowbyte & fPix) pcolor = 1; else pcolor = 0;
                        if (highbyte & fPix) pcolor += 2;
                        if ((pcolor & 0x3) != 0) {
                            draw_pixel(sprx + currpix, scanline, PPU_Memory[0x3F00 + (pcolor | sprpal)]);
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
                unsigned char lowbyte = PPU_Memory[sproffset + scan_to_draw];
                unsigned char highbyte = PPU_Memory[sproffset + scan_to_draw + 8];
                int currpix;
                if (!hflip) {
                    sprx += 7;
                    for (currpix = 7; currpix >= 0; currpix--) {
                        if(currpix == 0)  
                            fPix = 1;
                        else
                            fPix = (2 << (currpix - 1));

                        if (lowbyte & fPix) pcolor = 1; else pcolor = 0;
                        if (highbyte & fPix) pcolor += 2;
                        if ((pcolor & 0x3) != 0) {
                            draw_pixel(sprx - currpix, scanline, PPU_Memory[0x3F00 + (pcolor | sprpal)]);
                            if (currspr == 0) {ppu_status |= 0x40;} //Sprite 0 Hit Flag
                        }
                    }
                } else {
                    for (currpix = 0; currpix < 8; currpix++) {
                        if(currpix == 0)  
                            fPix = 1;
                        else
                            fPix = (2 << (currpix - 1));
                        

                        if (lowbyte & fPix) pcolor = 1; else pcolor = 0;
                        if (highbyte & fPix) pcolor += 2;
                        if ((pcolor & 0x3) != 0) {
                            draw_pixel(sprx + currpix, scanline, PPU_Memory[0x3F00 + (pcolor | sprpal)]);
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
    int nescolor = PPU_Memory[0x3f00];
    int x;


  /* DISABLE FOR TEST 
    for(x = 51840; x < 236160; x+=3){
        bufAdr[x]=palette[nescolor].b;
        bufAdr[x+1]=palette[nescolor].g;
        bufAdr[x+2]=palette[nescolor].r;
    }
   */
  
}

/* update menu image */
void updateMenu() {
    u8* bufAdr=gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    memcpy(bufAdr, imagem, 0x46500);
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

