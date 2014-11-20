arm-none-eabi-strip 3DNES.elf
makerom -f cci -o 3DNES.3ds -rsf gw_workaround.rsf -target d -exefslogo -elf 3DNES.elf -icon icon.icn -banner banner.bnr