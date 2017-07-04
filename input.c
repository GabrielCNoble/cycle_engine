#include "input.h"
#include "console.h"
#include "pew.h"
#include "draw.h"

input_cache input;
extern console_t console;
extern pew_t pew;
extern renderer_t renderer;

float last_mouse_x = 0.0;
float last_mouse_y = 0.0;

SDL_Cursor *ibeam;
SDL_Cursor *arrow;
SDL_Cursor *h_arrows;
SDL_Cursor *v_arrows;
SDL_Cursor *dl_arrows;
SDL_Cursor *dr_arrows;


#ifdef __cplusplus
extern "C"
{
#endif

/*
=============
input_Init
=============
*/
PEWAPI void input_Init()
{
	
	ibeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	h_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	v_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	dl_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	dr_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	
	input.mouse_dx=0.0;
	input.mouse_dy=0.0;
	input.kb_event=(SDL_Event *)calloc(1, sizeof(SDL_Event));
	SDL_WarpMouseInWindow(renderer.window, renderer.screen_width/2, renderer.screen_height/2);
	input.bm_mouse = 0;
	input_GetInput();
	return;
}

/*
=============
input_Finish
=============
*/
PEWAPI void input_Finish()
{
	free(input.kb_event);
	return;
}

/*
=============
input_GetInput
=============
*/
PEWAPI void input_GetInput()
{
	int bm;
	//SDL_Cursor *cursor;
	
	
	
	if(!pew.b_console)
	{
		SDL_PollEvent(input.kb_event);
		input.kb_keys=(Uint8 *)SDL_GetKeyboardState(NULL);
	}
	
	//TwEventSDL(input.kb_event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
	
	bm=SDL_GetMouseState(&input.mouse_x, &input.mouse_y);
	
	input.mouse_y = renderer.screen_height - input.mouse_y;
	
	if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
		input.bm_mouse &= ~MOUSE_LEFT_BUTTON_JUST_CLICKED;
	}
	
	if(input.bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
	{
		input.bm_mouse &= ~MOUSE_RIGHT_BUTTON_JUST_CLICKED;
	}
	
	if(input.bm_mouse & MOUSE_LEFT_BUTTON_JUST_RELEASED)
	{
		input.bm_mouse &= ~MOUSE_LEFT_BUTTON_JUST_RELEASED;
	}
	
	if(input.bm_mouse & MOUSE_RIGHT_BUTTON_JUST_RELEASED)
	{
		input.bm_mouse &= ~MOUSE_RIGHT_BUTTON_JUST_RELEASED;
	}
	
	if(bm & 1)
	{
		if(!(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
		{
			input.bm_mouse |= MOUSE_LEFT_BUTTON_JUST_CLICKED;
		}
		
		input.bm_mouse |= MOUSE_LEFT_BUTTON_CLICKED;
		//TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
	}
	else
	{
		if(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
		{
			input.bm_mouse |= MOUSE_LEFT_BUTTON_JUST_RELEASED;
		}
		input.bm_mouse &= ~MOUSE_LEFT_BUTTON_CLICKED;
		//TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
	}
	
	if(bm & 4)
	{
		if(!(input.bm_mouse & MOUSE_RIGHT_BUTTON_CLICKED))
		{
			input.bm_mouse |= MOUSE_RIGHT_BUTTON_JUST_CLICKED;
		}
		input.bm_mouse |= MOUSE_RIGHT_BUTTON_CLICKED;
		//TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_RIGHT);
	}
	else
	{
		if(input.bm_mouse & MOUSE_RIGHT_BUTTON_CLICKED)
		{
			input.bm_mouse |= MOUSE_RIGHT_BUTTON_JUST_RELEASED;
		}
		input.bm_mouse &= ~MOUSE_RIGHT_BUTTON_CLICKED;
		//TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_RIGHT);
	}
	
	input.normalized_mouse_x=(float)input.mouse_x/(float)renderer.screen_width;
	input.normalized_mouse_y=(float)input.mouse_y/(float)renderer.screen_height;
	
	input.normalized_mouse_x*=2.0;
	input.normalized_mouse_x-=1.0;
	
	input.normalized_mouse_y*=2.0;
	input.normalized_mouse_y-=1.0;
	
	//input.bm_mouse &= ~MOUSE_OVER_WIDGET;
	
	//if(!pew.b_console)
	if(pew.pew_state == PEW_PLAYING)
	{
		SDL_ShowCursor(0);
		SDL_WarpMouseInWindow(renderer.window, renderer.screen_width/2, renderer.screen_height/2);
		input.mouse_dx=input.normalized_mouse_x;
		input.mouse_dy=input.normalized_mouse_y;
	}
	else
	{
		SDL_ShowCursor(1);
		//cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
		//SDL_SetCursor(h_arrows);
		
		input.mouse_dx = input.normalized_mouse_x - last_mouse_x;
		input.mouse_dy = input.normalized_mouse_y - last_mouse_y;
		
		last_mouse_x = input.normalized_mouse_x;
		last_mouse_y = input.normalized_mouse_y;
	}
	
	
	input_GetEsc();
	
	
	
	//printf("%f %f\n", input.normalized_mouse_x, input.normalized_mouse_y);
	//printf("%d\n", input.bm_mouse);
	
	return;
}

PEWAPI void input_SetCursor(int cursor)
{
	switch(cursor)
	{
		case CURSOR_ARROW:
			SDL_SetCursor(arrow);
		break;
		
		case CURSOR_HORIZONTAL_ARROWS:
			SDL_SetCursor(h_arrows);
		break;
		
		case CURSOR_VERTICAL_ARROWS:
			SDL_SetCursor(v_arrows);
		break;
		
		case CURSOR_I_BEAM:
			SDL_SetCursor(ibeam);
		break;
		
		case CURSOR_LEFT_DIAGONAL_ARROWS:
			SDL_SetCursor(dl_arrows);
		break;
		
		case CURSOR_RIGHT_DIAGONAL_ARROWS:
			SDL_SetCursor(dr_arrows);
		break;
	}
}


PEWAPI int input_GetKeyPressed(int key)
{
	return input.kb_keys[key];
}


PEWAPI int input_GetMouseButton(int button)
{
	//return input.bm_mouse&button;
	int i = 0;
	switch(button)
	{
		case SDL_BUTTON_LEFT:
			i = input.bm_mouse & (MOUSE_LEFT_BUTTON_CLICKED | MOUSE_LEFT_BUTTON_JUST_CLICKED | MOUSE_LEFT_BUTTON_JUST_RELEASED);
		break;
		
		case  SDL_BUTTON_MIDDLE:
		
		break;
		
		case SDL_BUTTON_RIGHT:
			i = input.bm_mouse & (MOUSE_RIGHT_BUTTON_CLICKED | MOUSE_RIGHT_BUTTON_JUST_CLICKED | MOUSE_RIGHT_BUTTON_JUST_RELEASED);
		break;
	}
	
	return i;
}



PEWAPI void input_GetEsc()
{
	if(input_GetKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		if(pew.esc_state==ESC_RELEASED)
		{
			pew.esc_state=ESC_JUST_PRESSED;
		}
		else if(pew.esc_state==ESC_JUST_PRESSED)
		{
			pew.esc_state=ESC_HELD_DOWN;
		}
		
		if(pew.esc_state==ESC_JUST_PRESSED)
		{
			if(pew.pew_state==PEW_PAUSED && !pew.b_console)
			{
				pew_Resume();
			}
			else if(pew.pew_state==PEW_PLAYING)
			{
				pew_Pause();
			}
		}
	}
	else
	{
		pew.esc_state=ESC_RELEASED;
	}
}

/*
=============
input_ProcessConsoleInput
=============
*/
/*PEWAPI void input_ProcessConsoleInput()
{
	if(console.bm_status&CONSOLE_KEY_JUST_PRESSED)
	{
		console.bm_status&= ~CONSOLE_KEY_JUST_PRESSED;
	}
	
	if(input.kb_keys[SDL_SCANCODE_F3])
	{
		if(!(console.bm_status&CONSOLE_KEY_DOWN))
		{
			console.bm_status|=CONSOLE_KEY_JUST_PRESSED;
		}
		console.bm_status|=CONSOLE_KEY_DOWN;
	}
	else
	{
		console.bm_status&= ~CONSOLE_KEY_DOWN;
	}
	
	
	if(console.bm_status&CONSOLE_KEY_JUST_PRESSED && !(console.bm_status&CONSOLE_ROLLDOWN || console.bm_status&CONSOLE_ROLLUP))
	{
		console.bm_status&= ~CONSOLE_CAN_CHANGE;
		if(!(console.bm_status&CONSOLE_VISIBLE))
		{
			console.bm_status|=CONSOLE_ROLLDOWN;
		}
		else
		{
			console.bm_status|=CONSOLE_ROLLUP;
		}
	}
	
	if(console.bm_status&CONSOLE_ROLLDOWN)
	{
		console_RollDownConsole();
	}
	else if(console.bm_status&CONSOLE_ROLLUP)
	{
		console_RollUpConsole();
	}
	
	if(console.bm_status&CONSOLE_VISIBLE)pew.pew_status=PEW_PAUSED;
	else pew.pew_status=PEW_PLAYING;
}*/

#ifdef __cplusplus
}
#endif









