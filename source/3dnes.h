
extern char romfn[256];

extern unsigned char romcache[1048576];

extern unsigned char ppu_memory[16384];
extern unsigned char sprite_memory[256];

extern unsigned int pad1_data;

extern int CPU_is_running;
extern int pause_emulation;
extern unsigned char skipframe;

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