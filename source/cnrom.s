@ -----------------------------------------------------------------
@  3DNES unrom - Written by St4rk
@  Date: 18/11/2014
@  3DNES é um emulador de Nintendo Entertainment System para 3DS
@  esse é um projeto open-source, você pode modificar e utilizar
@  os arquivos para estudo, desde que mantenha os devidos créditos
@ -----------------------------------------------------------------

.arm
.align 2
.global cnrom_access
.type cnrom_access STT_FUNC
cnrom_access: @ void cnrom_acess(address, data)
	push 		{r0-r5, lr}
	ldr 		r2, =0x7FFF
	cmp 		r0, r2
	cmpgt 		r0, #0x10000
	blt 		cnrom_switch_prg
	b  			cnrom_end


cnrom_switch_prg:
	@ data & (CHR - 1)
	ldr r2, =CHR
	sub r2, #0x1
	and r1, r1, r2

	@ address  = 0x0000 
	@ r2 prg_size  = 16384 = 0x4000
 	@ r3 chr_size  = 8192 =  0x2000
 	@ r4 chr_start = prg_size * PRG;
	ldr r2, =0x4000
	ldr r3, =0x2000
	ldr r4, =PRG
	@ r4 = chr_start
	mul r4, r2, r4

	@ memcpy(PPU_Memory + address,  
	@ ROM_Cache + 16 + chr_start + (bank * chr_size), chr_size);
	
	@ ROM_Cache + 16
	ldr r0, =ROM_Cache+16
	@ ROM_Cache + chr_start
	add r0, r0, r4
	@  (Bank * chr_size)
	mul r4, r1, r3
	@ ROM_Cache + (bank * chr_Size)
	add r0, r0, r4
	@ PPU_Memory
	ldr r1, =PPU_Memory

cnrom_memcpy: @ memcpy(r1, r0, r3)
	ldr   r2, [r0], #4
	str   r2, [r1], #4
	subs  r2, r2, #4
	bge   cnrom_memcpy


cnrom_end:
	pop			{r0-r5, pc}
