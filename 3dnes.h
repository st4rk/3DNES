#include <3ds.h>

extern char romfn[256];

extern u8 romcache[1048576];

extern u8 ppu_memory[16384];
extern u8 sprite_memory[256];

extern u32 pad1_data;

extern int CPU_is_running;
extern int pause_emulation;
extern u8 skipframe;

/* SRAM Save */
extern void load_state();
extern void save_state();

/* Background and sprites Enable or Disable ? */
extern int enable_background;
extern int enable_sprites;
/* ROM Size */
extern long romlen;
/* readByte */
extern unsigned char memory_read(unsigned int address);
/* writeByte */
extern void write_memory(unsigned int address,unsigned char data);
/* readShort */
extern int memory_read16(int address);
extern int memory_read16zp(int address);
/* Some Stuff */
extern void set_input();
extern void reset_emulation();
extern void quit_emulation();

/* Save State */

extern void saveState(int n);
extern void loadState(int n);