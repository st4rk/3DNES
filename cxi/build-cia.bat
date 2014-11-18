arm-none-eabi-strip 3DNES.elf
makerom -f cia -o 3DNES.cia -elf 3DNES.elf -rsf build_cia.rsf -icon icon.icn -banner banner.bnr -exefslogo -target t 
pause