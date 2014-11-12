
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "6502core.h"

#include "3dnes.h"
#include "input.h"

extern int inMenu;

//Get's last index of _c in char *in
int FindLastOf(char *in, char _c) {
    int ret = 0, last_ret = 0;
    int i;
    for (i = 0; i < strlen(in); i++) {
        if (in[i] == _c)  {
            ret = last_ret;
            last_ret = i;
        }
    }
    return ++ret;
}

//Basic sprintf
u32 cast_int(char* oBuff, u32 num) {
    int i;
    if (num < 9) {
        *oBuff++ = (char)(0x30 + num);
        *oBuff = '\0';
        return 1;
    } else {
        int buffer[15];
        int bSize = 1, lCount = 0;
        buffer[0] = num % 10;
        num -= num % 10;
        while (num > 0) {
            num /= 10;
            buffer[bSize++] = num % 10;
        }
        for (i = bSize - 2; i >= 0; i--) {
            *oBuff++ = (char)(0x30 + buffer[i]);
            lCount++;
        }
        *oBuff = '\0';
        return lCount;
    }
}
u32 cast_str(char *oBuff, char *iBuff) {
    u32 lCount = 0;
    while (*iBuff != '\0') {
        *oBuff++ = *iBuff++;
        lCount++;
    }
    *oBuff = '\0';
    return lCount;
}

#include <stdarg.h>
int sprintf(char *oBuff, const char *iBuff, ...) {
    va_list args;
    va_start(args, 40);
    while (*iBuff != '\0') {
        if (*iBuff != '%') {
            *oBuff++ = *iBuff++;
        } else {
            iBuff++; //skip %
            if(*iBuff == 'd') oBuff += cast_int(oBuff, va_arg(args, u32));
            if(*iBuff == 's') oBuff += cast_str(oBuff, va_arg(args, char*));
            iBuff++;
        }
    }
    *oBuff = '\0';
    va_end(args);
    return 0;
}

void unicodeToChar(char* dst, u16* src)
{
    if(!src || !dst)return;
    while(*src)*(dst++)=(*(src++))&0xFF;
    *dst=0x00;
}


/* Check if a button is pressed */
void check_joypad(){
    u32 keys = ((u32*)0x10000000)[7];

    if(keys & BUTTON_L1) {  
        inMenu = 0;
    }

    if(keys & BUTTON_R1) 
        NES_ScreenShot();

    if(inMenu == 0){
        if(keys & BUTTON_L1) {
            inMenu = 1;
        }
        if(keys & BUTTON_A)
            set_input((char *) 7);
        else
            clear_input((char *) 7);

        if(keys & BUTTON_B)
            set_input((char *) 8);
        else
            clear_input((char *) 8);

        if(keys & BUTTON_START)
            set_input((char *) 5);
        else
            clear_input((char*) 5);

        if(keys & BUTTON_RIGHT)
            set_input((char *) 4);
        else
            clear_input((char *) 4);

        if(keys & BUTTON_LEFT)
            set_input((char *) 3);
        else
            clear_input((char *) 3);

        if(keys & BUTTON_UP)
            set_input((char *) 2);
        else
            clear_input((char *) 2);

        if(keys & BUTTON_DOWN)
            set_input((char *) 1);
        else
            clear_input((char *) 1);

        if(keys & BUTTON_SELECT)
            set_input((char *) 6);
        else
            clear_input((char *) 6);
    }
}
