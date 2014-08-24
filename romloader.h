

extern unsigned char PRG;
extern unsigned char CHR;

extern unsigned char MAPPER;

extern int OS_MIRROR;
extern int FS_MIRROR;
extern int TRAINER;
extern int SRAM;
extern int MIRRORING;
extern int VRAM;

extern int analyze_header(char *romfn);
extern int load_rom(char *romfn);
