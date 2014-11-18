/*
 * LameNES - Nintendo Entertainment System (NES) emulator
 *
 * Copyright (c) 2005, Joey Loman, <joey@lamenes.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * &0x80 = D7
 * &0x40 = D6
 * &0x20 = D5
 * &0x10 = D4
 * &0x08 = D3
 * &0x04 = D2
 * &0x02 = D1
 * &0x01 = D0
 */

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
