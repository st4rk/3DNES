
#include <stdio.h>

#include "3dnes.h"

int pad1_DOWN;
int pad1_UP;
int pad1_LEFT;
int pad1_RIGHT;
int pad1_START;
int pad1_SELECT;
int pad1_A;
int pad1_B;

void
set_input(int pad_key)
{
	switch(pad_key) {
		/* pad_down */
		case 1:
		pad1_DOWN = 0x01;
		break;

		/* pad_up */
		case 2:
		pad1_UP = 0x01;
		break;

		/* pad_left */
		case 3:
		pad1_LEFT = 0x01;
		break;

		/* pad_right */
		case 4:
		pad1_RIGHT = 0x01;
		break;

		/* pad_start */
		case 5:
		pad1_START = 0x01;
		break;

		/* pad_select */
		case 6:
		pad1_SELECT = 0x01;
		break;

		/* pad_a */
		case 7:
		pad1_A = 0x01;
		break;

		/* pad_b */
		case 8:
		pad1_B = 0x01;
		break;

		default:
		/* never reached */
		break;
	}
}

void
clear_input(int pad_key)
{
	switch(pad_key) {
		/* pad_down */
		case 1:
		pad1_DOWN = 0x40;
		break;

		/* pad_up */
		case 2:
		pad1_UP = 0x40;
		break;

		/* pad_left */
		case 3:
		pad1_LEFT = 0x40;
		break;

		/* pad_right */
		case 4:
		pad1_RIGHT = 0x40;
		break;

		/* pad_start */
		case 5:
		pad1_START = 0x40;
		break;

		/* pad_select */
		case 6:
		pad1_SELECT = 0x40;
		break;

		/* pad_a */
		case 7:
		pad1_A = 0x40;
		break;

		/* pad_b */
		case 8:
		pad1_B = 0x40;
		break;

		default:
		/* never reached */
		break;
	}
}

void
reset_input()
{
	pad1_DOWN = 0x40;
	pad1_UP = 0x40;
	pad1_LEFT = 0x40;
	pad1_RIGHT = 0x40;
	pad1_START = 0x40;
	pad1_SELECT = 0x40;
	pad1_A = 0x40;
	pad1_B = 0x40;

}
