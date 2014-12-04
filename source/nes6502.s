@ -----------------------------------------------------------------
@  3DNES 6502 - Written by St4rk
@  Date: 18/11/2014
@  3DNES é um emulador de Nintendo Entertainment System para 3DS
@  esse é um projeto open-source, você pode modificar e utilizar
@  os arquivos para estudo, desde que mantenha os devidos créditos
@ -----------------------------------------------------------------

.arm
.align 2


@ --------------------
@ -    Data Region  --
@ --------------------







@ --------------------
@ - Addressing Mode --
@ --------------------




@ -----------------------
@ - 6502 Execute/Reset --
@ -----------------------


.global CPU_Execute @ CPU_Execute(cycles)
.type   CPU_Execute STT_FUNC
CPU_Execute:


.global CPU_Reset
.type   CPU_Reset   STT_FUNC
CPU_Reset:


@ --------------------
@ - CPU INTERRUPT   --
@ --------------------


.global IRQ
.type   IRQ  STT_FUNC
IRQ:




.global NMI
.type   NMI STT_FUNC
NMI:




