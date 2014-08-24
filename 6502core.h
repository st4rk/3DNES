
#include <ctr/types.h>
 extern unsigned char memory[65536];

 typedef struct {
	unsigned char A;
	unsigned char P;
	unsigned char X;
	unsigned char Y;
	unsigned char S;
	int PC;
} cpu_regs;
cpu_regs R;
int EA; //Effective Address
int tick_count;

extern void IRQ();
extern void NMI();
extern void CPU_reset();
extern void CPU_execute(int cycles);
