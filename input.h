#ifndef INPUT_H
#define INPUT_H

#include "conf.h"
#include "includes.h"


enum MOUSE_FLAGS
{
	MOUSE_LEFT_BUTTON_CLICKED = 1,
	MOUSE_LEFT_BUTTON_JUST_CLICKED = 1 << 1,
	MOUSE_LEFT_BUTTON_JUST_RELEASED = 1 << 2,
	MOUSE_RIGHT_BUTTON_CLICKED = 1 << 3,
	MOUSE_RIGHT_BUTTON_JUST_CLICKED = 1 << 4,
	MOUSE_RIGHT_BUTTON_JUST_RELEASED = 1 << 5,
	MOUSE_OVER_WIDGET = 1 << 6
	
};

enum CURSOR
{
	CURSOR_ARROW,
	CURSOR_HORIZONTAL_ARROWS,
	CURSOR_VERTICAL_ARROWS,
	CURSOR_I_BEAM,
	CURSOR_LEFT_DIAGONAL_ARROWS,
	CURSOR_RIGHT_DIAGONAL_ARROWS
};

typedef struct
{
	Uint8 *kb_keys;
	SDL_Event *kb_event;
	int bm_mouse;
	int mouse_x;
	int mouse_y;
	float normalized_mouse_x;
	float normalized_mouse_y;
	float mouse_dx;
	float mouse_dy;
}input_cache;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void input_Init();

PEWAPI void input_Finish();

PEWAPI void input_GetInput();

PEWAPI void input_SetCursor(int cursor);

PEWAPI int input_GetKeyPressed(int key);

PEWAPI int input_GetMouseButton(int button);

PEWAPI void input_GetEsc();



/*PEWAPI void input_ProcessConsoleInput();*/

#ifdef __cplusplus
}
#endif



#endif /* INPUT_H */
