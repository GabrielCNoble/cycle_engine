#ifndef CONSOLE_H
#define CONSOLE_H

#include "includes.h"
#include "material.h"

enum CONSOLE_STATUS
{
	CONSOLE_VISIBLE=1,
	CONSOLE_ROLLUP=2,
	CONSOLE_ROLLDOWN=4,
	CONSOLE_KEY_DOWN=8,
	CONSOLE_KEY_JUST_PRESSED=16,
	CONSOLE_CAN_CHANGE=32,
	CONSOLE_WRAPPED_AROUND=64
};

enum CONSOLE_FLAGS
{
	CONSOLE_NO_ECHO=1,
	CONSOLE_WAIT_INPUT=2,				/* default input is lockless */
};

enum ESC_PRESSED_STATE
{
	ESC_JUST_PRESSED,
	ESC_HELD_DOWN,
	ESC_RELEASED
};

enum MESSAGE_TYPE
{
	MESSAGE_FREE,
	MESSAGE_NORMAL,
	MESSAGE_WARNING,
	MESSAGE_ERROR
};

typedef struct char_t
{
	char char_code;
	char type;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	char align;
}char_t;

typedef struct console_font_char_t
{
	char char_code;
	unsigned short *start;
}console_font_char_t;

typedef struct console_font
{
	color4_t current_color;
	console_font_char_t *chars;
	unsigned short *font_bytes;
}console_font;

typedef struct console_t
{
	float x;
	float y;
	float width;
	float height;
	color4_t text_color;
	int bm_status;
	int bm_flags;
	int scrolled_lines;					/* how many lines up the console was scrolled */
	int line_width;
	int max_lines;
	int cursor_x;
	int cursor_y;
	int input_line_cursor;
	char *input_line;
	char_t *console_text_buffer;		/* text buffer */
	unsigned short *console_buffer;		/* console screen */
}console_t;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void console_Init();

PEWAPI void console_Finish();

PEWAPI void console_ProcessConsole();

PEWAPI void console_RollDownConsole();

PEWAPI void console_RollUpConsole();

PEWAPI char console_GetConsoleInput();

PEWAPI void console_ProcessConsoleInput();

PEWAPI void console_PassParam(char *param_str);

PEWAPI int console_TestToken(char *token, char *str);

PEWAPI void console_Enable(char *param_str);

PEWAPI void console_Disable(char *param_str);

PEWAPI void console_Help(char *param_str);

PEWAPI void console_ClearConsoleBuffer();

PEWAPI void console_ClearConsoleInputLine();

PEWAPI void console_Print(int message_type, char *str, ...);

PEWAPI void console_SetTextColor(char r, char g, char b);

PEWAPI void console_BlitTextBuffer();




#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_H */











