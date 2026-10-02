/* SDLTRS version Copyright (c): 2006, Mark Grebe */

/* Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/
/*
 * Copyright (C) 1992 Clarendon Hill Software.
 *
 * Permission is granted to any individual or institution to use, copy,
 * or redistribute this software, provided this copyright notice is retained. 
 *
 * This software is provided "as is" without any expressed or implied
 * warranty.  If this software brings on any sort of damage -- physical,
 * monetary, emotional, or brain -- too bad.  You've got no one to blame
 * but yourself. 
 *
 * The software may be modified for your own purposes, but modified versions
 * must retain this notice.
 */

/*
   Modified by Timothy Mann, 1996
   Modified by Mark Grebe, 2006
   Last modified on Wed May 07 09:12:00 MST 2006 by markgrebe
*/

/*#define KBDEBUG 1*/
/*#define JOYDEBUG 1*/
#define KP_JOYSTICK 1         /* emulate joystick with keypad */
/*#define KPNUM_JOYSTICK 1*/  /* emulate joystick with keypad + NumLock */
#define SHIFT_F1_IS_F13 1     /* use if X reports Shift+F1..F8 as F13..F20 */
/*#define SHIFT_F1_IS_F11 1*/ /* use if X reports Shift+F1..F10 as F11..F20 */

#include <SDL3/SDL.h>
#include "z80.h"
#include "trs.h"
#include "trs_sdl_keyboard.h"

/*
 * Key event queue
 */
#define KEY_QUEUE_SIZE	(32)
static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_head;
static int key_queue_entries;

/*
 * TRS-80 key matrix
 */
#define TK(a, b) (((a)<<4)+(b))
#define TK_ADDR(tk) (((tk) >> 4)&0xf)
#define TK_DATA(tk) ((tk)&0xf)
#define TK_DOWN(tk) (((tk)&0x10000) == 0)

#define TK_AtSign       TK(0, 0)  /* @   */
#define TK_A            TK(0, 1)
#define TK_B            TK(0, 2)
#define TK_C            TK(0, 3)
#define TK_D            TK(0, 4)
#define TK_E            TK(0, 5)
#define TK_F            TK(0, 6)
#define TK_G            TK(0, 7)
#define TK_H            TK(1, 0)
#define TK_I            TK(1, 1)
#define TK_J            TK(1, 2)
#define TK_K            TK(1, 3)
#define TK_L            TK(1, 4)
#define TK_M            TK(1, 5)
#define TK_N            TK(1, 6)
#define TK_O            TK(1, 7)
#define TK_P            TK(2, 0)
#define TK_Q            TK(2, 1)
#define TK_R            TK(2, 2)
#define TK_S            TK(2, 3)
#define TK_T            TK(2, 4)
#define TK_U            TK(2, 5)
#define TK_V            TK(2, 6)
#define TK_W            TK(2, 7)
#define TK_X            TK(3, 0)
#define TK_Y            TK(3, 1)
#define TK_Z            TK(3, 2)
#define TK_LeftBracket  TK(3, 3)  /* [ { */ /* not really on keyboard */
#define TK_Backslash    TK(3, 4)  /* \ | */ /* not really on keyboard */
#define TK_RightBracket TK(3, 5)  /* ] } */ /* not really on keyboard */
#define TK_Caret        TK(3, 6)  /* ^ ~ */ /* not really on keyboard */
#define TK_Underscore   TK(3, 7)  /* _   */ /* not really on keyboard */
#define TK_0            TK(4, 0)  /* 0   */
#define TK_1            TK(4, 1)  /* 1 ! */
#define TK_2            TK(4, 2)  /* 2 " */
#define TK_3            TK(4, 3)  /* 3 # */
#define TK_4            TK(4, 4)  /* 4 $ */
#define TK_5            TK(4, 5)  /* 5 % */
#define TK_6            TK(4, 6)  /* 6 & */
#define TK_7            TK(4, 7)  /* 7 ' */
#define TK_8            TK(5, 0)  /* 8 ( */
#define TK_9            TK(5, 1)  /* 9 ) */
#define TK_Colon        TK(5, 2)  /* : * */
#define TK_Semicolon    TK(5, 3)  /* ; + */
#define TK_Comma        TK(5, 4)  /* , < */
#define TK_Minus        TK(5, 5)  /* - = */
#define TK_Period       TK(5, 6)  /* . > */
#define TK_Slash        TK(5, 7)  /* / ? */
#define TK_Enter        TK(6, 0)
#define TK_Clear        TK(6, 1)
#define TK_Break        TK(6, 2)
#define TK_Up           TK(6, 3)
#define TK_Down         TK(6, 4)
#define TK_Left         TK(6, 5)
#define TK_Right        TK(6, 6)
#define TK_Space        TK(6, 7)
#define TK_LeftShift    TK(7, 0)
#define TK_RightShift   TK(7, 1)  /* M3/4 only; both shifts are 7, 0 on M1 */
#define TK_Ctrl         TK(7, 2)  /* M4 only */
#define TK_CapsLock     TK(7, 3)  /* M4 only */
#define TK_F1           TK(7, 4)  /* M4 only */
#define TK_F2           TK(7, 5)  /* M4 only */
#define TK_F3           TK(7, 6)  /* M4 only */
#define TK_Unused       TK(7, 7)

/* Fake keycodes with special meanings */
#define TK_NULL                 TK(8, 0)
#define TK_Neutral              TK(8, 1)
#define TK_ForceShift           TK(8, 2)
#define TK_ForceNoShift         TK(8, 3)
#define TK_ForceShiftPersistent TK(8, 4)
#define TK_AllKeysUp            TK(8, 5)
#define TK_Joystick             TK(10,  0)
#define TK_North                TK(10,  1)
#define TK_Northeast            TK(10,  9)
#define TK_East                 TK(10,  8)
#define TK_Southeast            TK(10, 10)
#define TK_South                TK(10,  2)
#define TK_Southwest            TK(10,  5)
#define TK_West                 TK(10,  4)
#define TK_Northwest            TK(10,  6)
#define TK_Fire                 TK(10, 16)

#define JOY_BOUNCE 20000

typedef struct {
	SDL_Keycode key;
	int bit_action;
	int shift_action;
} KeyTable;

/* Keysyms in the extended ASCII range 0x0000 - 0x00ff */

KeyTable ascii_key_table[] = {
	{ SDLK_AT, TK_AtSign, TK_ForceNoShift },
	{ 'a', TK_A, TK_ForceNoShift },
	{ 'A', TK_A,  TK_ForceShift },
	{ 'b', TK_B, TK_ForceNoShift },
	{ 'B', TK_B,  TK_ForceShift },
	{ 'c', TK_C, TK_ForceNoShift },
	{ 'C', TK_C,  TK_ForceShift },
	{ 'd', TK_D, TK_ForceNoShift },
	{ 'D', TK_D,  TK_ForceShift },
	{ 'e', TK_E, TK_ForceNoShift },
	{ 'E', TK_E,  TK_ForceShift },
	{ 'f', TK_F, TK_ForceNoShift },
	{ 'F', TK_F,  TK_ForceShift },
	{ 'g', TK_G, TK_ForceNoShift },
	{ 'G', TK_G,  TK_ForceShift },
	{ 'h', TK_H, TK_ForceNoShift },
	{ 'H', TK_H,  TK_ForceShift },
	{ 'i', TK_I, TK_ForceNoShift },
	{ 'I', TK_I,  TK_ForceShift },
	{ 'j', TK_J, TK_ForceNoShift },
	{ 'J', TK_J,  TK_ForceShift },
	{ 'K', TK_K,  TK_ForceShift },
	{ 'k', TK_K, TK_ForceNoShift },
	{ 'l', TK_L, TK_ForceNoShift },
	{ 'L', TK_L,  TK_ForceShift },
	{ 'm', TK_M, TK_ForceNoShift },
	{ 'M', TK_M,  TK_ForceShift },
	{ 'n', TK_N, TK_ForceNoShift },
	{ 'N', TK_N,  TK_ForceShift },
	{ 'o', TK_O, TK_ForceNoShift },
	{ 'O', TK_O,  TK_ForceShift },
	{ 'p', TK_P, TK_ForceNoShift },
	{ 'P', TK_P,  TK_ForceShift },
	{ 'q', TK_Q, TK_ForceNoShift },
	{ 'Q', TK_Q,  TK_ForceShift },
	{ 'r', TK_R, TK_ForceNoShift },
	{ 'R', TK_R,  TK_ForceShift },
	{ 's', TK_S, TK_ForceNoShift },
	{ 'S', TK_S,  TK_ForceShift },
	{ 't', TK_T, TK_ForceNoShift },
	{ 'T', TK_T,  TK_ForceShift },
	{ 'u', TK_U, TK_ForceNoShift },
	{ 'U', TK_U,  TK_ForceShift },
	{ 'v', TK_V, TK_ForceNoShift },
	{ 'V', TK_V,  TK_ForceShift },
	{ 'w', TK_W, TK_ForceNoShift },
	{ 'W', TK_W,  TK_ForceShift },
	{ 'x', TK_X, TK_ForceNoShift },
	{ 'X', TK_X,  TK_ForceShift },
	{ 'y', TK_Y, TK_ForceNoShift },
	{ 'Y', TK_Y,  TK_ForceShift },
	{ 'z', TK_Z, TK_ForceNoShift },
	{ 'Z', TK_Z,  TK_ForceShift },
	{ SDLK_LEFTBRACKET, TK_LeftBracket, TK_ForceNoShift },
	{ SDLK_LEFTBRACE, TK_LeftBracket, TK_ForceShift },
	{ SDLK_BACKSLASH, TK_Backslash, TK_ForceNoShift },
	{ SDLK_PIPE, TK_Backslash, TK_ForceShift },
	{ SDLK_RIGHTBRACKET, TK_RightBracket, TK_ForceNoShift },
	{ SDLK_RIGHTBRACE, TK_RightBracket, TK_ForceShift },
	{ SDLK_CARET, TK_Caret, TK_ForceNoShift },
	{ SDLK_TILDE, TK_Caret, TK_ForceShift },
	{ SDLK_UNDERSCORE, TK_Underscore, TK_ForceNoShift },
	{ SDLK_0, TK_0, TK_ForceNoShift },
	{ SDLK_1, TK_1, TK_ForceNoShift },
	{ SDLK_EXCLAIM, TK_1, TK_ForceShift },
	{ SDLK_2, TK_2, TK_ForceNoShift },
	{ SDLK_DBLAPOSTROPHE, TK_2, TK_ForceShift },
	{ SDLK_3, TK_3, TK_ForceNoShift },
	{ SDLK_HASH, TK_3, TK_ForceShift },
	{ SDLK_4, TK_4, TK_ForceNoShift },
	{ SDLK_DOLLAR, TK_4, TK_ForceShift },
	{ SDLK_5, TK_5, TK_ForceNoShift },
	{ SDLK_PERCENT, TK_5, TK_ForceShift },
	{ SDLK_6, TK_6, TK_ForceNoShift },
	{ SDLK_AMPERSAND, TK_6, TK_ForceShift },
	{ SDLK_7, TK_7, TK_ForceNoShift },
	{ SDLK_APOSTROPHE, TK_7, TK_ForceShift },
	{ SDLK_8, TK_8, TK_ForceNoShift },
	{ SDLK_LEFTPAREN, TK_8, TK_ForceShift },
	{ SDLK_9, TK_9, TK_ForceNoShift },
	{ SDLK_RIGHTPAREN, TK_9, TK_ForceShift },
	{ SDLK_COLON, TK_Colon, TK_ForceNoShift },
	{ SDLK_ASTERISK, TK_Colon, TK_ForceShift },
	{ SDLK_SEMICOLON, TK_Semicolon, TK_ForceNoShift },
	{ SDLK_PLUS, TK_Semicolon, TK_ForceShift },
	{ SDLK_COMMA, TK_Comma, TK_ForceNoShift },
	{ SDLK_LESS, TK_Comma, TK_ForceShift },
	{ SDLK_MINUS, TK_Minus, TK_ForceNoShift },
	{ SDLK_EQUALS, TK_Minus, TK_ForceShift },
	{ SDLK_PERIOD, TK_Period, TK_ForceNoShift },
	{ SDLK_GREATER, TK_Period, TK_ForceShift },
	{ SDLK_SLASH, TK_Slash, TK_ForceNoShift },
	{ SDLK_QUESTION, TK_Slash, TK_ForceShift },
	{ SDLK_RETURN, TK_Enter, TK_Neutral },
	{ SDLK_CLEAR, TK_Clear, TK_Neutral },
	{ SDLK_HOME, TK_Clear, TK_Neutral },
	{ SDLK_ESCAPE, TK_Break, TK_Neutral },
	{ SDLK_UP, TK_Up, TK_Neutral },
	{ SDLK_DOWN, TK_Down, TK_Neutral },
	{ SDLK_LEFT, TK_Left, TK_Neutral },
	{ SDLK_BACKSPACE, TK_Left, TK_Neutral },
	{ SDLK_RIGHT, TK_Right, TK_Neutral },
	{ SDLK_SPACE, TK_Space, TK_Neutral },
	{ SDLK_LSHIFT, TK_LeftShift, TK_Neutral },
	{ SDLK_RSHIFT, TK_RightShift, TK_Neutral },
	{ SDLK_LCTRL, TK_Ctrl, TK_Neutral },
	{ SDLK_RCTRL, TK_Ctrl, TK_Neutral },
	{ SDLK_CAPSLOCK, TK_CapsLock, TK_Neutral },
	{ SDLK_F1, TK_F1, TK_Neutral },
	{ SDLK_F2, TK_F2, TK_Neutral },
	{ SDLK_F3, TK_F3, TK_Neutral },
	{ SDLK_UNKNOWN, TK_NULL, TK_Neutral },
	{ SDLK_KP_8, TK_North, TK_Neutral },
	{ SDLK_KP_9, TK_Northeast, TK_Neutral },
	{ SDLK_KP_6, TK_East, TK_Neutral },
	{ SDLK_KP_3, TK_Southeast, TK_Neutral },
	{ SDLK_KP_2, TK_South, TK_Neutral },
	{ SDLK_KP_1, TK_Southwest, TK_Neutral },
	{ SDLK_KP_4, TK_West, TK_Neutral },
	{ SDLK_KP_7, TK_Northwest, TK_Neutral },
	{ SDLK_KP_0, TK_Fire, TK_Neutral },
};

static int keystate[8] = { 0, };
static int force_shift = TK_Neutral;
static int joystate = 0;
int trs_joystick_num = 0;
int trs_keypad_joystick = TRUE;

/* Avoid changing state too fast so keystrokes aren't lost. */
static tstate_t key_stretch_timeout;
int stretch_amount = STRETCH_AMOUNT;
int trs_kb_bracket_state = 0;

void trs_keyboard_save(FILE *file)
{
  fwrite(&keystate,8,sizeof(int),file);
  fwrite(&force_shift,1,sizeof(int),file);
  fwrite(&joystate,1,sizeof(int),file);
  fwrite(&key_stretch_timeout,1,sizeof(long long),file);
  fwrite(&stretch_amount,1,sizeof(int),file);
  fwrite(&trs_kb_bracket_state,1,sizeof(int),file);
}

void trs_keyboard_load(FILE *file)
{
  fread(&keystate,8,sizeof(int),file);
  fread(&force_shift,1,sizeof(int),file);
  fread(&joystate,1,sizeof(int),file);
  fread(&key_stretch_timeout,1,sizeof(long long),file);
  fread(&stretch_amount,1,sizeof(int),file);
  fread(&trs_kb_bracket_state,1,sizeof(int),file);
}

void trs_kb_reset()
{
  key_stretch_timeout = z80_state.t_count;
}

int key_heartbeat = 0;
void trs_kb_heartbeat()
{
  /* Don't hold keys in queue too long */
  key_heartbeat++;
}

void trs_kb_bracket(int shifted)
{
  /* Set the shift state for the emulation of the "[ {", "\ |", 
     "] }", "^ ~", and "_ DEL" keys.  Some Model 4 keyboard drivers
     decode these with [ shifted and { unshifted, etc., while most
     other keyboard drivers either ignore them or decode them with
     [ unshifted and { shifted.  We default to the latter.  Note that
     these keys didn't exist on real machines anyway.
  */
  int i;
  trs_kb_bracket_state = shifted;
  for (i=0x5b; i<=0x5f; i++) {
    ascii_key_table[i].shift_action =
      shifted ? TK_ForceShift : TK_ForceNoShift;
  }
  for (i=0x7b; i<0x7f; i++) {
    ascii_key_table[i].shift_action =
      shifted ? TK_ForceNoShift : TK_ForceShift;
  }
}

/* Emulate joystick with the keypad */
int trs_emulate_joystick(int key_down, int bit_action)
{
  if (bit_action < TK_Joystick) return 0;
  if (key_down) {
    joystate |= (bit_action & 0x1f);
  } else {
    joystate &= ~(bit_action & 0x1f);
  }
  return 1;
}

/* Joystick functions called when SDL Joystick events occur */
void trs_joy_button_down(void)
{
  joystate |= (TK_Fire & 0x1f);
}

void trs_joy_button_up(void)
{
  joystate &= ~(TK_Fire & 0x1f);
}

void trs_joy_hat(unsigned char value)
{
  joystate &= (TK_Fire & 0x1f);
  
  switch(value) {
    case SDL_HAT_CENTERED:
      break;
    case SDL_HAT_UP:
      joystate |= (TK_North & 0x1f);
      break;
    case SDL_HAT_RIGHT:
      joystate |= (TK_East & 0x1f);
      break;
    case SDL_HAT_DOWN:
      joystate |= (TK_South & 0x1f);
      break;
    case SDL_HAT_LEFT:
      joystate |= (TK_West & 0x1f);
      break;
    case SDL_HAT_RIGHTUP:
      joystate |= (TK_Northeast & 0x1f);
      break;
    case SDL_HAT_RIGHTDOWN:
      joystate |= (TK_Southeast & 0x1f);
      break;
    case SDL_HAT_LEFTUP:
      joystate |= (TK_Southwest & 0x1f);
      break;
    case SDL_HAT_LEFTDOWN:
      joystate |= (TK_Northwest & 0x1f);
      break;
    }
}

void trs_set_keypad_joystick(void) {
	KeyTable* kt;
	if (trs_keypad_joystick) {
		for( kt=ascii_key_table; kt->key != SDLK_UNKNOWN; kt++ ) {
			if( kt->key == SDLK_KP_8 ) {
				kt->bit_action = TK_North;
			}
			else if( kt->key == SDLK_KP_9 ) {
				kt->bit_action = TK_Northeast;
			}
			else if( kt->key == SDLK_KP_6 ) {
				kt->bit_action = TK_Southeast;
			}
			else if( kt->key == SDLK_KP_3 ) {
				kt->bit_action = TK_East;
			}
			else if( kt->key == SDLK_KP_2 ) {
				kt->bit_action = TK_South;
			}
			else if( kt->key == SDLK_KP_1 ) {
				kt->bit_action = TK_Southwest;
			}
			else if( kt->key == SDLK_KP_4 ) {
				kt->bit_action = TK_West;
			}
			else if( kt->key == SDLK_KP_7 ) {
				kt->bit_action = TK_Northwest;
			}
			else if( kt->key == SDLK_KP_0 ) {
				kt->bit_action = TK_Fire;
			}
		}
	} else {
		for( kt=ascii_key_table; kt->key != SDLK_UNKNOWN; kt++ ) {
			if( kt->key == SDLK_KP_8 ) {
				kt->bit_action = TK_8;
			}
			else if( kt->key == SDLK_KP_9 ) {
				kt->bit_action = TK_9;
			}
			else if( kt->key == SDLK_KP_6 ) {
				kt->bit_action = TK_6;
			}
			else if( kt->key == SDLK_KP_3 ) {
				kt->bit_action = TK_3;
			}
			else if( kt->key == SDLK_KP_2 ) {
				kt->bit_action = TK_2;
			}
			else if( kt->key == SDLK_KP_1 ) {
				kt->bit_action = TK_1;
			}
			else if( kt->key == SDLK_KP_4 ) {
				kt->bit_action = TK_4;
			}
			else if( kt->key == SDLK_KP_7 ) {
				kt->bit_action = TK_7;
			}
			else if( kt->key == SDLK_KP_0 ) {
				kt->bit_action = TK_0;
			}
		}
	}
}

void trs_open_joystick(void)
{
  static SDL_Joystick *open_joy = NULL;
  int num_joysticks = 0;
  
  SDL_GetJoysticks(&num_joysticks);
  
  if (open_joy != NULL) {
    SDL_CloseJoystick(open_joy);
    open_joy = NULL;
 }

  if ((trs_joystick_num != -1) &&
      (trs_joystick_num <= (num_joysticks -1))) {
      open_joy = SDL_OpenJoystick(trs_joystick_num);
  }
  else
    trs_joystick_num = -1;
}

void trs_joy_axis(unsigned char axis, short value)
{
  int dir;
  
  if (value < -JOY_BOUNCE)
    dir = -1;
  else if (value > JOY_BOUNCE)
    dir = 1;
  else
    dir = 0;
    
  if (axis == 0) {
    switch (dir) {
      case -1:
        joystate |= (TK_West & 0x1f);
        joystate &= ~(TK_East & 0x1f);
        break;
      case 0:
        joystate &= ~((TK_West | TK_East) & 0x1f);
        break;
      case 1:
        joystate |= (TK_East & 0x1f);
        joystate &= ~(TK_West & 0x1f);
        break;
    }
  }
  else if (axis == 1) {
    switch (dir) {
      case -1:
        joystate |= (TK_North & 0x1f);
        joystate &= ~(TK_South & 0x1f);
        break;
      case 0:
        joystate &= ~((TK_North | TK_South) & 0x1f);
        break;
      case 1:
        joystate |= (TK_South & 0x1f);
        joystate &= ~(TK_North & 0x1f);
        break;
    }
  }
}

int trs_joystick_in()
{
#if JOYDEBUG
  debug("joy %02x ", joystate);
#endif
  return ~joystate;
}

void trs_xlate_keysym(int keysym, int key_down) {
	KeyTable* kt;
	static int shift_action = TK_Neutral;

	if( !keysym && !key_down ) {
		//force all keys up 
		queue_key(TK_AllKeysUp);
		shift_action = TK_Neutral;
		return;
	}
	
	for( kt=ascii_key_table; kt->key != SDLK_UNKNOWN; kt++ ) {
		if( kt->key == (SDL_Keycode)keysym ) {
			break;
		}
	}

	if (kt->bit_action == TK_NULL) return;
	if (trs_emulate_joystick(key_down, kt->bit_action)) return;

	if (key_down) {
		if( shift_action != TK_ForceShiftPersistent && shift_action != kt->shift_action ) {
			shift_action = kt->shift_action;
			queue_key(shift_action);
		}
		queue_key(kt->bit_action);
	} else {
		queue_key(kt->bit_action | 0x10000);
		if (shift_action != TK_Neutral && shift_action == kt->shift_action) {
			shift_action = TK_Neutral;
			queue_key(shift_action);
		}
	}
}

static void change_keystate(int action)
{
    int key_down;
    int i;
#ifdef KBDEBUG
    debug("change_keystate: action 0x%x\n", action);
#endif

    switch (action) {
      case TK_AllKeysUp:
	/* force all keys up */
	for (i=0; i<7; i++) {
	    keystate[i] = 0;
	}
	force_shift = TK_Neutral;
	break;

      case TK_Neutral:
      case TK_ForceShift:
      case TK_ForceNoShift:
      case TK_ForceShiftPersistent:
	force_shift = action;
	break;

      default:
	key_down = TK_DOWN(action);
	if (key_down) {
	    keystate[TK_ADDR(action)] |= (1 << TK_DATA(action));
	} else {
	    keystate[TK_ADDR(action)] &= ~(1 << TK_DATA(action));
	}
    }
}

static int kb_mem_value(int address)
{
    int i, bitpos, data = 0;

    for (i=0, bitpos=1; i<7; i++, bitpos<<=1) {
	if (address & bitpos) {
	    data |= keystate[i];
	}
    }
    if (address & 0x80) {
	int tmp = keystate[7];
	if (trs_model == 1) {
	    if (force_shift == TK_ForceNoShift) {
		/* deactivate shift key */
		tmp &= ~1;
	    } else if (force_shift != TK_Neutral) {
		/* activate shift key */
		tmp |= 1;
	    }
	} else {
	    if (force_shift == TK_ForceNoShift) {
		/* deactivate both shift keys */
		tmp &= ~3;
	    } else if (force_shift != TK_Neutral) {
		/* if no shift keys are down, activate left shift key */
		if ((tmp & 3) == 0) tmp |= 1;
	    }
	}
	data |= tmp;
    }
    return data;
}

int trs_kb_mem_read(int address)
{
    int key = -1;
    //int i, wait;
    //static int recursion = 0;
    //static int timesseen;

    /* Prevent endless recursive calls to this routine (by mem_read_word
       below) if REG_SP happens to point to keyboard memory. */
    //if (recursion) return 0;

    /* Avoid delaying key state changes in queue for too long */
    if (key_heartbeat > 2) {
      do {
	key = trs_next_key();
	if (key >= 0) {
	  change_keystate(key);
	  //timesseen = 1;
	}
      } while (key >= 0);
    }

    /* After each key state change, impose a timeout before the next one
       so that the Z-80 program doesn't miss any by polling too rarely,
       and so that we don't tickle the bugs in some common TRS-80 keyboard
       drivers that strike if two keys change simultaneously */
    if (key_stretch_timeout - z80_state.t_count > TSTATE_T_MID) {

	/* Check if we are in the system keyboard driver, called from
	   the wait-for-input routine.  If so, and there are no
	   keystrokes queued, and the current state has been seen by
	   at least 16 such reads, then trs_next_key will pause the
	   process to avoid burning host CPU needlessly.

	   The test below works on both Model I and III and is
	   insensitive to what keyboard driver is being used, as long
	   as it is called through the wait-for-key routine at ROM
	   address 0x0049 and has not pushed too much on the stack yet
	   when it first reads from the key matrix.  The search is
	   needed (at least) for NEWDOS80, which pushes 2 extra bytes
	   on the stack.  */
	
	//wait = 0;
	//if (timesseen++ >= 16) {
	//  recursion = 1;
	//  for (i=0; i<=4; i+=2) {
	//    if (mem_read_word(REG_SP + 2 + i) == 0x4015) {
	//      wait = mem_read_word(REG_SP + 10 + i) == 0x004c;
	//      break;
	//    }
	//  }
	//  recursion = 0;
	//}
	/* Get the next key */
	key = trs_next_key();
	key_stretch_timeout = z80_state.t_count + stretch_amount;
    }

    if (key >= 0) {
      change_keystate(key);
      //timesseen = 1;
    }
    key_heartbeat = 0;
    return kb_mem_value(address);
}

void clear_key_queue()
{
  key_queue_head = 0;
  key_queue_entries = 0;
#if QDEBUG
    debug("clear_key_queue\n");
#endif
}

void queue_key(int state)
{
  key_queue[(key_queue_head + key_queue_entries) % KEY_QUEUE_SIZE] = state;
#if QDEBUG
  debug("queue_key 0x%x\n", state);
#endif
  if (key_queue_entries < KEY_QUEUE_SIZE) {
    key_queue_entries++;
  } else {
#if QDEBUG
    debug("queue_key overflow\n");
#endif
  }
}

int dequeue_key()
{
  int rval = -1;

  if(key_queue_entries > 0)
    {
      rval = key_queue[key_queue_head];
      key_queue_head = (key_queue_head + 1) % KEY_QUEUE_SIZE;
      key_queue_entries--;
#if QDEBUG
      debug("dequeue_key 0x%x\n", rval);
#endif
    }
  return rval;
}

int trs_next_key() {
	return dequeue_key();
}
