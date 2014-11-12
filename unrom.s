@ -----------------------------------------------------------------
@  3DNES unrom - Written by St4rk
@  Date: 27/09/2014
@  3DNES é um emulador de Nintendo Entertainment System para 3DS
@  esse é um projeto open-source, você pode modificar e utilizar
@  os arquivos para estudo, desde que mantenha os devidos créditos
@ -----------------------------------------------------------------

.arm
.align 2
.global unrom_access
.type unrom_access STT_FUNC
unrom_access: @ unrom_acess(adress, data)
	push     {r0-r5, lr}

	ldr      r2, =0x7fff
	cmp      r0, r2
	cmpgt    r0, #0x10000
	blt      unrom_switch_prg
	b        unrom_end
	

unrom_switch_prg:
	

	@ r1 = bank	
	@ address
	ldr      r4, =0x8000
	@ bank * prg_size
	ldr      r2, =0x4000
	mul      r3, r2, r1
	@ memory + address
	ldr      r0, =memory
	add      r0, r0, r4
	@ romcache + 16	+ (bank * prg_size)
	ldr      r1, =romcache+16
	add      r1, r1, r3
	@ memcpy (memory, romcache, prg_size)

unrom_memcpy:

	ldr      r3, [r1], #4
	str      r3, [r0], #4
	subs     r2, r2, #4
	bge      unrom_memcpy
	 
unrom_end:
	pop       {r0-r5, pc}




