#include "console.h"
#include "input.h"
#include "pew.h"
#include "draw_types.h"

extern console_t console;
extern input_cache input;
extern pew_t pew;
extern console_font font;
extern renderer_t renderer;

static float color_conversion_lookup_table[256];


static char help_no_args[]={"help: type help [command] to get more detailed information about it.\n"};
static char help_enable[]={"enable: command used to enable features of the engine, such wireframe mode, or debug lines.\noptions are:\n    echo: enable console echo\n    debug: enable debug drawing\n    wireframe: turn wireframe mode on\n		render_flag: enable different rendering features\n"};
static char help_disable[]={"disable: command used to disable features of the engine, such wireframe mode, or debug lines.\noptions are:\n    echo: disable console echo\n    debug: disable debug drawing\n    wireframe: turn wireframe mode off\n"};
static char help_echo[]={"echo: repeats whatever proceedes the command echo, including reserved words of the console.\n"};
static char help_render_mode[]={"render_mode: used to set the engine's current render mode. options are:\n    wireframe: draws geometry in wireframe with the same color\n    flat: draw all the geometry with just its diffuse color, without performing light calculations\n    lit: draw geometry with light calculations\n"};
static char help_debug[]={"debug: when enabled, the engine draws lights, AABBs and other debuging lines.\noptions are:\n    disable: disable debug drawing\n    mode1: just light origins are drawn\n    mode2: origin and boundaries of lights are drawn, just like AABBs boundaries\n"};

#define CONSOLE_LINE_WIDTH 65
#define CONSOLE_MAX_ROWS 13
#define CONSOLE_MAX_INPUT_LINE 1023
/* x_pos,
   y_pos,
   input_line_y_pos,
   height (in lines),
   width (in chars) */
static int console_alignment[7][5]=
{
	
	935, -73, -28, 23, 156,	/*1920x1080*/
	780, -60, -22, 19, 130, /*1600x900*/
	665, -45, -16, 16, 111,	/*1366x768*/
	620, -63, -25, 22, 103,	/*1280x1024*/
	620, -55, -24, 21, 103,	/*1280x960*/
	495, -45, -16, 16, 83,	/*1024x768*/
	390, -35, -10, 13, 65,	/*800x600*/
};


static unsigned short c_font[]=
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//espaço
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0600, 0x0600, 0x0000, 0x0000, 0x0600, 0x0600, 0x0600, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0F00, 0x0600, 0x0600, 0x0000,		// !
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0880, 0x1980, 0x1980, 0x0000, 0x0000, 0x0000,		// "
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3300, 0x3300, 0x3300, 0x7FC0, 0x1980, 0x1980, 0x0CC0, 0x0CC0, 0x3FF0, 0x0660, 0x0660, 0x0660, 0x0000, 0x0000, 0x0000,		// #
	0x0000, 0x0000, 0x0000, 0x0600, 0x0600, 0x0F00, 0x1F80, 0x39C0, 0x31C0, 0x0380, 0x0700, 0x0E00, 0x1C00, 0x38C0, 0x39C0, 0x1F80, 0x0F00, 0x0600, 0x0600, 0x0000,		// $
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3000, 0x3000, 0x18C0, 0x18C0, 0x0C00, 0x0C00, 0x0600, 0x0600, 0x0300, 0x0300, 0x3180, 0x3180, 0x00C0, 0x00C0, 0x0000,		// %
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1F80, 0x3F80, 0x71C0, 0x61E0, 0x63E0, 0x6700, 0x6E00, 0x7C00, 0x7000, 0x7C00, 0x6E00, 0x6600, 0x7E00, 0x3C00, 0x0000, 	// &
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0300, 0x0600, 0x0600, 0x0700, 0x0700, 0x0700, 0x0000,		// ' not used
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0300, 0x0F00, 0x0E00, 0x0C00, 0x1C00, 0x1800, 0x1800, 0x1800, 0x1800, 0x1C00, 0x0C00, 0x0E00, 0x0F00, 0x0300, 0x0000,		// (
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0C00, 0x0E00, 0x0700, 0x0300, 0x0380, 0x0180, 0x0180, 0x0180, 0x0180, 0x0380, 0x0300, 0x0700, 0x0E00, 0x0C00, 0x0000,		// )
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		// *
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	// +
	0x0000, 0x0000, 0x1000, 0x1800, 0x1C00, 0x0C00, 0x0C00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	// ,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0C00, 0x0C00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	// .
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3000, 0x3000, 0x1800, 0x1800, 0x0C00, 0x0C00, 0x0600, 0x0600, 0x0300, 0x0300, 0x0180, 0x0180, 0x00C0, 0x00C0, 0x0000, 	// /
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1F00, 0x7FC0, 0x60C0, 0xE060, 0xF060, 0xD860, 0xCC60, 0xC660, 0xC360, 0xC1E0, 0xC0E0, 0x60C0, 0x7FC0, 0x1F00, 0x0000, 	//0
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7F80, 0x7F80, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x7C00, 0x7C00, 0x1C00, 0x0C00, 0x0000, 	//1
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x7000, 0x3800, 0x1C00, 0x0E00, 0x0700, 0x0380, 0x01C0, 0xC0E0, 0xC060, 0xE0E0, 0x7FC0, 0x3F80, 0x0000,  	//2
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1F80, 0x3FC0, 0x70E0, 0x6060, 0x0060, 0x00E0, 0x07C0, 0x07C0, 0x00E0, 0x0060, 0x6060, 0x70E0, 0x3FC0, 0x1F80, 0x0000,     //3
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x03F0, 0x03F0, 0x00C0, 0x00C0, 0x3FF0, 0x3FF0, 0x30C0, 0x38C0, 0x1CC0, 0x0EC0, 0x07C0, 0x03C0, 0x01C0, 0x00C0, 0x0000,		//4
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0FC0, 0x1FE0, 0x3870, 0x3030, 0x0030, 0x0030, 0x0070, 0x1FE0, 0x3FC0, 0x3000, 0x3000, 0x3000, 0x3FE0, 0x3FE0, 0x0000,		//5
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0F80, 0x1FC0, 0x38E0, 0x3060, 0x3060, 0x38E0, 0x3FC0, 0x3F80, 0x3000, 0x3000, 0x3800, 0x1C00, 0x0F80, 0x0780, 0x0000,		//6
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x0E00, 0x0700, 0x0380, 0x01C0, 0x00C0, 0x60C0, 0x60C0, 0x7FC0, 0x7FC0, 0x0000,		//7
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1F00, 0x3F80, 0x71C0, 0x71C0, 0x71C0, 0x73C0, 0x3F80, 0x1F00, 0x3980, 0x71C0, 0x71C0, 0x71C0, 0x3F80, 0x1F00, 0x0000,		//8
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1E00, 0x1F00, 0x0380, 0x01C0, 0x00C0, 0x1FC0, 0x3FC0, 0x71C0, 0x60C0, 0x60C0, 0x60C0, 0x71C0, 0x3F80, 0x1F00, 0x0000,		//9
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0600, 0x0600, 0x0000, 0x0000, 0x0000, 0x0000, 0x0600, 0x0600, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//:
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//;
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//<
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//=
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//>
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//?
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//@
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x60C0, 0x60C0, 0x60C0, 0x60C0, 0x7FC0, 0x7FC0, 0x60C0, 0x60C0, 0x60C0, 0x71C0, 0x3B80, 0x1F00, 0x0E00, 0x0400, 0x0000,		//A
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7F00, 0x7F80, 0x31C0, 0x30C0, 0x30C0, 0x31C0, 0x3F80, 0x3F80, 0x31C0, 0x30C0, 0x30C0, 0x31C0, 0x7F80, 0x7F00, 0x0000,		//B
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//C
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//D
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//E
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//F
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//G
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//H
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//I
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//J
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//K
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//L
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//M
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//N
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//O
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//P
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//Q
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//R
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//S
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//T
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//U
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//V
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//W
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//X
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//Y
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//Z
	0x0000, 0x0000, 0x1F00, 0x1F00, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1F00, 0x1F00, 0x0000, 	//[
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x1F00, 0x1F00, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x1F00, 0x1F00, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7FE0, 0x7FE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//_
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3FC0, 0x7FC0, 0x60C0, 0x60C0, 0x3FC0, 0x1FC0, 0x00C0, 0x3FC0, 0x1F80, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//a
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7F00, 0x7F80, 0x61C0, 0x60C0, 0x60C0, 0x60C0, 0x71C0, 0x7F80, 0x6F00, 0x6000, 0x6000, 0x6000, 0x6000, 0x6000, 0x0000, 	//b
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1F00, 0x3F80, 0x70C0, 0x6000, 0x6000, 0x6000, 0x70C0, 0x3F80, 0x1F00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//c
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1FC0, 0x3FC0, 0x70C0, 0x60C0, 0x60C0, 0x60C0, 0x71C0, 0x3FC0, 0x1EC0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x0000, 	//d
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1F00, 0x3F80, 0x7000, 0x6000, 0x7F80, 0x7FC0, 0x70C0, 0x3F80, 0x1F00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//e
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x7FC0, 0x7FC0, 0x1800, 0x1800, 0x1800, 0x1C00, 0x0FE0, 0x07E0, 0x0000, 	//f
	0x0000, 0x0000, 0x0000, 0x3F00, 0x3F80, 0x01C0, 0x00C0, 0x1EC0, 0x3FC0, 0x71C0, 0x60C0, 0x70C0, 0x3FC0, 0x1FC0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,		//g
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x60C0, 0x60C0, 0x60C0, 0x60C0, 0x60C0, 0x60C0, 0x71C0, 0x7F80, 0x6F00, 0x6000, 0x6000, 0x6000, 0x6000, 0x6000, 0x0000, 	//h
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3FC0, 0x3FC0, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x1E00, 0x1E00, 0x0000, 0x0000, 0x0600, 0x0600, 0x0000, 0x0000, 	//i
	0x0000, 0x1F80, 0x3FC0, 0x30C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x0FC0, 0x0FC0, 0x0000, 0x0000, 0x00C0, 0x00C0, 0x0000, 0x0000, 	//j
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x30C0, 0x31C0, 0x3380, 0x3700, 0x3E00, 0x3E00, 0x3E00, 0x3700, 0x3380, 0x3180, 0x3000, 0x3000, 0x3000, 0x3000, 0x0000, 	//k
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3FC0, 0x3FC0, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x3E00, 0x3E00, 0x0000, 0x0000, 	//l
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6660, 0x6660, 0x6660, 0x6660, 0x6660, 0x6660, 0x7FE0, 0x7FC0, 0x5980, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//m
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x30C0, 0x30C0, 0x30C0, 0x30C0, 0x30C0, 0x30C0, 0x31C0, 0x3F80, 0x3F00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//n
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1F00, 0x3F80, 0x71C0, 0x60C0, 0x60C0, 0x60C0, 0x71C0, 0x3F80, 0x1F00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//o
	0x0000, 0x6000, 0x6000, 0x6000, 0x6000, 0x6F00, 0x7F80, 0x71C0, 0x60C0, 0x60C0, 0x60C0, 0x61C0, 0x7F80, 0x7F00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//p
	0x0000, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x1EC0, 0x3FC0, 0x71C0, 0x60C0, 0x60C0, 0x60C0, 0x70C0, 0x3FC0, 0x1FC0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//q
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3000, 0x3000, 0x3000, 0x3000, 0x3000, 0x3000, 0x38C0, 0x3FC0, 0x3780, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//r
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1F80, 0x3FC0, 0x00C0, 0x00C0, 0x1FC0, 0x3F80, 0x3000, 0x3F80, 0x1F00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//s
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0FC0, 0x1FC0, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x7F00, 0x7F00, 0x1800, 0x1800, 0x1800, 0x0000, 0x0000, 0x0000, 	//t
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1EC0, 0x3FC0, 0x71C0, 0x60C0, 0x60C0, 0x60C0, 0x60C0, 0x60C0, 0x60C0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//u
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0600, 0x0F00, 0x0F00, 0x1980, 0x1980, 0x30C0, 0x30C0, 0x6060, 0x6060, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//v
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1080, 0x39C0, 0x3FC0, 0x6F60, 0x6660, 0x6660, 0x6660, 0x6660, 0x6660, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//w
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x60C0, 0x71C0, 0x3B80, 0x1F00, 0x0E00, 0x1F00, 0x3B80, 0x71C0, 0x60C0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//x
	0x0000, 0x1800, 0x0C00, 0x0C00, 0x0600, 0x0600, 0x0F00, 0x0F00, 0x1F80, 0x1980, 0x30C0, 0x30C0, 0x6060, 0x6060, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//y
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7FC0, 0x7FC0, 0x3000, 0x1800, 0x0C00, 0x0600, 0x0300, 0x7F80, 0x7FC0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 	//z
	0x0000, 0x0000, 0x0700, 0x0F00, 0x1C00, 0x1800, 0x1800, 0x1800, 0x1800, 0x3800, 0x7000, 0x3800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1C00, 0x0F00, 0x0700, 0x0000, 	//{
	0x0000, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0600, 0x0000, 	//|
	0x0000, 0x0000, 0x7000, 0x7800, 0x1C00, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x0E00, 0x0700, 0x0E00, 0x0C00, 0x0C00, 0x0C00, 0x0C00, 0x1C00, 0x7800, 0x7000, 0x0000, 	//}
};


#ifdef __cplusplus
extern "C"
{
#endif

/*
=============
console_Init
=============
*/
PEWAPI void console_Init()
{
	char msb;
	char lsb;
	int i;
	int k;
	console.width=2.0;
	console.height=1.0;
	console.x=0.0;
	console.y=2.0;
	console.bm_status=0;
	console.bm_flags=0;
	console.input_line_cursor=0;
	console.input_line=(char *)calloc(1024, 1);
	console.console_text_buffer=(char_t*)calloc(32768, sizeof(char_t));
	console.console_buffer=(unsigned short *)calloc(32768, sizeof(short)*20);
	console.line_width=console_alignment[renderer.selected_resolution][4];
	console.max_lines=console_alignment[renderer.selected_resolution][3];
	//printf("%d %d %d\n", console.line_width, console.max_lines, renderer.selected_resolution);
	
	console_ClearConsoleBuffer();
	console_SetTextColor(255,255,255);
	
	font.chars=(console_font_char_t *)calloc(94, sizeof(console_font_char_t));
	font.font_bytes=(unsigned short *)calloc(94, sizeof(short)*20);
	
	for(i=0; i<20*94; i++)
	{
		msb=*(((unsigned char *)&c_font[i])+1);
		lsb=*(((unsigned char *)&c_font[i]));
		*(((unsigned char *)&c_font[i]))=msb;
		*(((unsigned char *)&c_font[i])+1)=lsb;	

	}
	
	for(i=0; i<256; i++)
	{
		color_conversion_lookup_table[i]=(float)i/255.0;
	}
	
	for(i=0; i<94; i++)
	{
		font.chars[i].char_code=' '+i;
		font.chars[i].start=font.font_bytes+(i*20);
		for(k=0; k<20; k++)
		{
			font.chars[i].start[k]=c_font[k + i*20];
		}
	}
	
	//console_Print(MESSAGE_NORMAL, "window created with resolution [%dx%d]\n", renderer.width, renderer.height);
	
}


/*
=============
console_Finish
=============
*/
PEWAPI void console_Finish()
{
	free(console.console_text_buffer);
	free(console.console_buffer);
	free(console.input_line);
	//free(text_texture);
}



/*
=============
console_ProcessConsole
=============
*/
PEWAPI void console_ProcessConsole()
{
	char byte;
	int last_state;
	
	if(console.bm_status&CONSOLE_KEY_JUST_PRESSED)
	{
		console.bm_status&= ~CONSOLE_KEY_JUST_PRESSED;
	}
	
	if(input.kb_keys[SDL_GetScancodeFromKey(SDLK_QUOTE)])
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
	
	if(console_GetConsoleInput()==96)
	{
		console.bm_status|=CONSOLE_KEY_JUST_PRESSED;
	}
	
	if(console.bm_status&CONSOLE_KEY_JUST_PRESSED && !(console.bm_status&CONSOLE_ROLLDOWN || console.bm_status&CONSOLE_ROLLUP))
	{
		console.bm_status&= ~CONSOLE_CAN_CHANGE;
		
		if(!(console.bm_status&CONSOLE_VISIBLE))
		{
			console.bm_status|=CONSOLE_ROLLDOWN;
			last_state=pew.pew_state;
			pew_Pause();
			if(!(last_state&PEW_PAUSED))
			{
				pew.pew_state|=PEW_PAUSED_BY_CONSOLE;
			}
			pew.b_console=1;

		}
		else
		{
			console.bm_status|=CONSOLE_ROLLUP;
			if(pew.pew_state&PEW_PAUSED_BY_CONSOLE)
			{
				pew_Resume();
			}
			pew.b_console=0;
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

	
	/*if(pew.pew_state!=PEW_EXIT)
	{
		if(console.bm_status&CONSOLE_VISIBLE)
		{
			pew_Pause();
			pew.b_console=1;
		}
		else
		{
			pew_Resume();
			pew.b_console=0;
		}
	}*/
}

/*
=============
console_RollDownConsole
=============
*/
PEWAPI void console_RollDownConsole()
{
	if(!(console.bm_status&CONSOLE_ROLLUP))
	{
		console.bm_status|=CONSOLE_ROLLDOWN;
		console.bm_status|=CONSOLE_VISIBLE;
		//SDL_StartTextInput();
		console.y-=0.003*pew.ti.ms_elapsed;
		if(console.y<0.51)
		{
			console.y=0.51;
			console.bm_status&= ~CONSOLE_ROLLDOWN;
		}
	}
}


/*
=============
console_RollUpConsole
=============
*/
PEWAPI void console_RollUpConsole()
{
	if(!(console.bm_status&CONSOLE_ROLLDOWN))
	{
		console.y+=0.003*pew.ti.ms_elapsed;
		console.bm_status|=CONSOLE_ROLLUP;
		if(console.y>2.0)
		{
			console.y=2.0;
			console.bm_status&= ~CONSOLE_ROLLUP;
			console.bm_status&= ~CONSOLE_VISIBLE;
			//SDL_StopTextInput();
		}
	}
}



/*
=============
console_GetConsoleInput
=============
*/
PEWAPI char console_GetConsoleInput()
{
	unsigned char read_byte=0;
	unsigned char mods=0;
	SDL_Event e;

	while(SDL_PollEvent(&e))
	{
		
		mods=0;
		if(e.key.keysym.mod&KMOD_LSHIFT || e.key.keysym.mod&KMOD_RSHIFT || e.key.keysym.mod&KMOD_CAPS)
		{
			mods=32;
		}
		
	
		if(console.bm_status&CONSOLE_VISIBLE && !(console.bm_status&CONSOLE_ROLLUP || console.bm_status&CONSOLE_ROLLDOWN))
		{
			if(e.key.type==SDL_KEYDOWN)
			{
				switch(e.key.keysym.sym)
				{
					
						case SDLK_QUOTE:
							if(mods)
							{
								read_byte='"';
							}
							else
							{
								return 96;
							}
						break;

						case SDLK_BACKSPACE:
							read_byte=8;
						break;

						case SDLK_PAGEUP:
							if(console.cursor_y-console.scrolled_lines>=console.max_lines)console.scrolled_lines++;
							return 0;

						case SDLK_PAGEDOWN:
							if(console.scrolled_lines>0)console.scrolled_lines--;
							return 0;

						case SDLK_SPACE:
							read_byte=' ';
						break;

						case SDLK_HASH:
							read_byte='#';
						break;

						case SDLK_PERCENT:
							read_byte='%';
						break;

						case SDLK_DOLLAR:
							read_byte='$';
						break;

						case SDLK_AMPERSAND:
							read_byte='&';
						break;

						case SDLK_SLASH:
							read_byte='/';
						break;

						case SDLK_0:
							if(mods){
								read_byte=')';
							}
							else{
								read_byte='0';
							}
						break;

						case SDLK_1:
							if(mods){
								read_byte='!';
							}
							else{
								read_byte='1';
							}
						break;

						case SDLK_2:
							if(mods){
								read_byte='@';
							}
							else{
								read_byte='2';
							}
						break;

						case SDLK_3:
							if(mods){
								read_byte='#';
							}
							else{
								read_byte='3';
							}
						break;

						case SDLK_4:
							if(mods){
								read_byte='$';
							}
							else{
								read_byte='4';
							}
						break;

						case SDLK_5:
							if(mods){
								read_byte='%';
							}
							else{
								read_byte='5';
							}
						break;

						case SDLK_6:
							read_byte='6';
						break;

						case SDLK_7:
							if(mods){
								read_byte='&';
							}
							else{
								read_byte='7';
							}
						break;

						case SDLK_8:
							if(mods){
								read_byte='*';
							}
							else{
								read_byte='8';
							}
						break;

						case SDLK_9:
							if(mods){
								read_byte='(';
							}
							else{
								read_byte='9';
							}
						break;
						
						case SDLK_SEMICOLON:
							if(mods)
							{
								read_byte=':';
							}
							else
							{
								
							}
							
						break;

						case SDLK_PERIOD:
							read_byte='.';
						break;

						case SDLK_COMMA:
							read_byte=',';
						break;

						case SDLK_LEFTBRACKET:
							read_byte='[';
						break;

						case SDLK_RIGHTBRACKET:
							read_byte=']';
						break;

						case 45:
							read_byte='_';
						break;

						case SDLK_a:
							read_byte='a'-mods;
						break;

						case SDLK_b:
							read_byte='b'-mods;
						break;

						case SDLK_c:
							read_byte='c'-mods;
						break;

						case SDLK_d:
							read_byte='d'-mods;
						break;

						case SDLK_e:
							read_byte='e'-mods;
						break;

						case SDLK_f:
							read_byte='f'-mods;
						break;

						case SDLK_g:
							read_byte='g'-mods;
						break;

						case SDLK_h:
							read_byte='h'-mods;
						break;

						case SDLK_i:
							read_byte='i'-mods;
						break;

						case SDLK_j:
							read_byte='j'-mods;
						break;

						case SDLK_k:
							read_byte='k'-mods;
						break;

						case SDLK_l:
							read_byte='l'-mods;
						break;

						case SDLK_m:
							read_byte='m'-mods;
						break;

						case SDLK_n:
							read_byte='n'-mods;
						break;

						case SDLK_o:
							read_byte='o'-mods;
						break;

						case SDLK_p:
							read_byte='p'-mods;
						break;

						case SDLK_q:
							read_byte='q'-mods;
						break;

						case SDLK_r:
							read_byte='r'-mods;
						break;

						case SDLK_s:
							read_byte='s'-mods;
						break;

						case SDLK_t:
							read_byte='t'-mods;
						break;

						case SDLK_u:
							read_byte='u'-mods;
						break;

						case SDLK_v:
							read_byte='v'-mods;
						break;

						case SDLK_w:
							read_byte='w'-mods;
						break;

						case SDLK_x:
							read_byte='x'-mods;
						break;

						case SDLK_y:
							read_byte='y'-mods;
						break;

						case SDLK_z:
							read_byte='z'-mods;
						break;
						
						case SDLK_RETURN:
							read_byte=13;
						break;

					}
				
				}
					
			
		}
	}

	if(read_byte)
	{
		if(read_byte==8 && console.input_line_cursor>0)
		{
			console.input_line_cursor--;
			return 0;
		}
		
		if(read_byte==13)
		{
			console_ProcessConsoleInput();
		}
		else if(read_byte!=8)
		{
			console.input_line[console.input_line_cursor++]=read_byte;
			/*console.input_line[console.input_line_cursor]='\0';
			console.input_line[console.input_line_cursor+1]='\0';*/
		}
		
	}


	//console_Print(MESSAGE_NORMAL, "a");
	return read_byte;
}



/*
=============
console_ProcessConsoleInput
=============
*/
PEWAPI void console_ProcessConsoleInput()
{
	register int i=0;
	int r;
	int g;
	int b;
	int k;
	char c[4];
	while(console.input_line[i]==' ')i++;
	
	if(console.input_line[i]=='c' && 
	   console.input_line[i+1]=='l' &&
	   console.input_line[i+2]=='e' &&
	   console.input_line[i+3]=='a' &&
	   console.input_line[i+4]=='r'
	  )
	{
		console_ClearConsoleBuffer();	
		console_ClearConsoleInputLine();
	}
	
	else if(console.input_line[i]=='e' && 
			console.input_line[i+1]=='n' &&
			console.input_line[i+2]=='a' &&
			console.input_line[i+3]=='b' &&
			console.input_line[i+4]=='l' &&
			console.input_line[i+5]=='e' &&
			(console.input_line[i+6]=='\0' || console.input_line[i+6]==' '))
	{
		i+=6;
		console_Enable(console.input_line+i);
		console_ClearConsoleInputLine();
	}
	
	else if(console.input_line[i]=='d' && 
			console.input_line[i+1]=='i' && 
			console.input_line[i+2]=='s' &&
			console.input_line[i+3]=='a' &&
			console.input_line[i+4]=='b' &&
			console.input_line[i+5]=='l' &&
			console.input_line[i+6]=='e' &&
			(console.input_line[i+7]=='\0' || console.input_line[i+7]==' '))
	{
		i+=7;
		console_Disable(console.input_line+i);
		console_ClearConsoleInputLine();  
	}
		
	
	else if(console.input_line[i]=='r' && 
		    console.input_line[i+1]=='e' &&
			console.input_line[i+2]=='n' &&
			console.input_line[i+3]=='d' &&
			console.input_line[i+4]=='e' &&
			console.input_line[i+5]=='r' &&
			console.input_line[i+6]=='_' &&
			console.input_line[i+7]=='m' &&
			console.input_line[i+8]=='o' &&
			console.input_line[i+9]=='d' &&
			console.input_line[i+10]=='e' &&
			(console.input_line[i+11]=='\0' || console.input_line[i+11]==' '))
	{
		
		i+=11;
		while(console.input_line[i]==' ')i++;
		
		if(console.input_line[i]=='\0')
		{
			switch(renderer.render_mode)
			{
				case RENDER_DRAWMODE_WIREFRAME:
					console_Print(MESSAGE_NORMAL, "current render mode is [rendermode_wireframe]\n");
				break;
				
				case RENDER_DRAWMODE_FLAT:
					console_Print(MESSAGE_NORMAL, "current render mode is [rendermode_flat]\n");
				break;
				
				/*case RENDERMODE_UNLIT:
					console_Print(MESSAGE_NORMAL, "current render mode is [renderemode_unlit]\n");	
				break;*/
				
				case RENDER_DRAWMODE_LIT:
					console_Print(MESSAGE_NORMAL, "current render mode is [rendermode_lit]\n");
				break;
				
			}
			
		}
		
		if(console.input_line[i]=='l' && 
		   console.input_line[i+1]=='i' &&
		   console.input_line[i+2]=='t' &&
		  (console.input_line[i+3]=='\0' || console.input_line[i+3]==' '))
		{
			draw_SetRenderDrawMode(RENDER_DRAWMODE_LIT);
		}
		/*else if(console.input_line[i]=='u' && 
				console.input_line[i+1]=='n' && 
				console.input_line[i+2]=='l' && 
		   		console.input_line[i+3]=='i' &&
		   		console.input_line[i+4]=='t' &&
		  		(console.input_line[i+5]=='\0' || console.input_line[i+5]==' '))
		{
			draw_SetRenderDrawMode(RENDER_DRAWMODE_UNLIT);
		}*/
		
		else if(console.input_line[i]=='w' && 
		    	console.input_line[i+1]=='i' &&
				console.input_line[i+2]=='r' &&
				console.input_line[i+3]=='e' &&
				console.input_line[i+4]=='f' &&
				console.input_line[i+5]=='r' &&
				console.input_line[i+6]=='a' &&
				console.input_line[i+7]=='m' &&
				console.input_line[i+8]=='e' &&
				(console.input_line[i+9]=='\0' || console.input_line[i+9]==' '))
		{
			draw_SetRenderDrawMode(RENDER_DRAWMODE_WIREFRAME);
		}
		
		else if(console.input_line[i]=='f' && 
		    	console.input_line[i+1]=='l' &&
				console.input_line[i+2]=='a' &&
				console.input_line[i+3]=='t' &&
				(console.input_line[i+4]=='\0' || console.input_line[i+4]==' '))
		{
			draw_SetRenderDrawMode(RENDER_DRAWMODE_FLAT);
		}
		
		console_ClearConsoleInputLine();
		
	}
	
	else if(console.input_line[i]=='h' &&
			console.input_line[i+1]=='e' &&
			console.input_line[i+2]=='l' &&
			console.input_line[i+3]=='p' &&
			(console.input_line[i+4]==' ' || console.input_line[i+4]=='\0'))
	{
		i+=4;
		console_Help(console.input_line+i);
		//console_Print(MESSAGE_ERROR, "hah what a joke\n");
		//console_Print(MESSAGE_NORMAL, help);
		console_ClearConsoleInputLine();
	}
	else if(console.input_line[i]=='e' && 
	   		console.input_line[i+1]=='c' &&
	   		console.input_line[i+2]=='h' &&
	   		console.input_line[i+3]=='o' &&
	   		(console.input_line[i+4]==' ' || console.input_line[i+4]=='\0')
	  	   )
	{
		console.input_line[console.input_line_cursor++]='\n';
		console.input_line[console.input_line_cursor]='\0';	
		console_Print(MESSAGE_FREE, console.input_line+5+i);
		console.input_line_cursor=0;
		console.input_line[console.input_line_cursor]='\0';
	}
	else if(console.input_line[i]=='e' && 
	   		console.input_line[i+1]=='x' &&
	   		console.input_line[i+2]=='i' &&
	   		console.input_line[i+3]=='t' &&
	   		(console.input_line[i+4]==' ' || console.input_line[i+4]=='\0')
	  	   )
	{
		//pew.pew_status=PEW_EXIT;
		pew_SetPewState(PEW_EXIT);
	}
	else if(console.input_line[i]=='t' && 
			console.input_line[i+1]=='e' &&
			console.input_line[i+2]=='x' &&
			console.input_line[i+3]=='t' &&
			console.input_line[i+4]=='_' &&
			console.input_line[i+5]=='c' &&
			console.input_line[i+6]=='o' &&
			console.input_line[i+7]=='l' &&
			console.input_line[i+8]=='o' &&
			console.input_line[i+9]=='r' && 
			(console.input_line[i+10]==' ' || console.input_line[i+10]=='\0')
	       )
	{	
		k=i+11;
		
		while(console.input_line[k]==' ')k++;
		
		if(console.input_line[k]>'/' && console.input_line[k]<':')
		{
			c[0]=console.input_line[k];
			k++;
			if(console.input_line[k]>'/' && console.input_line[k]<':')
			{
				c[1]=console.input_line[k];
				k++;
				
				if(console.input_line[k]>'/' && console.input_line[k]<':')
				{
					c[2]=console.input_line[k];
					c[3]='\0';
					k++;
				}
				else c[2]='\0';
			}
			else c[1]='\0';
		}
		else
		{
			console_ClearConsoleBuffer();
			console_ClearConsoleInputLine();
			return;
		}

		r=atoi(c);
		if(r>255 || r<0)
		{
			console_ClearConsoleBuffer();
			console_ClearConsoleInputLine();
			return;
		}
		
		while(console.input_line[k]==' ')k++;
		
		if(console.input_line[k]>'/' && console.input_line[k]<':')
		{
			c[0]=console.input_line[k];
			k++;
			if(console.input_line[k]>'/' && console.input_line[k]<':')
			{
				c[1]=console.input_line[k];
				k++;
				
				if(console.input_line[k]>'/' && console.input_line[k]<':')
				{
					c[2]=console.input_line[k];
					c[3]='\0';
					k++;
				}
				else c[2]='\0';
			}
			else c[1]='\0';
		}
		else
		{	
			console_ClearConsoleBuffer();
			console_ClearConsoleInputLine();
			return;
		}

		g=atoi(c);
		if(g>255 || g<0)
		{
			console_ClearConsoleBuffer();
			console_ClearConsoleInputLine();
			return;
		}
		
		while(console.input_line[k]==' ')k++;
		
		if(console.input_line[k]>'/' && console.input_line[k]<':')
		{
			c[0]=console.input_line[k];
			k++;
			if(console.input_line[k]>'/' && console.input_line[k]<':')
			{
				c[1]=console.input_line[k];
				k++;
				
				if(console.input_line[k]>'/' && console.input_line[k]<':')
				{
					c[2]=console.input_line[k];
					c[3]='\0';
					k++;
				}
				else c[2]='\0';
			}
			else c[1]='\0';
		}
		else
		{
			console_ClearConsoleBuffer();
			console_ClearConsoleInputLine();
			return;
		}

		b=atoi(c);
		if(b>255 || b<0)
		{
			console_ClearConsoleBuffer();
			console_ClearConsoleInputLine();
			return;
		}

		console_SetTextColor(r, g, b);
		//console_ClearConsoleBuffer();
		console_ClearConsoleInputLine();

	}
	
	else
	{
		
		console.input_line[console.input_line_cursor++]='\n';
		console.input_line[console.input_line_cursor]='\0';	
		if(!(console.bm_flags&CONSOLE_NO_ECHO)) console_Print(MESSAGE_FREE, console.input_line+i);
		console_ClearConsoleInputLine();
	}
}



PEWAPI void console_PassParam(char *param_str)
{
	int i=0;
	while(console.input_line_cursor<1022 && param_str[i])
	{
		console.input_line[console.input_line_cursor++]=param_str[i++];
	}
	console.input_line[console.input_line_cursor]='\0';
	console_ProcessConsoleInput();
}

PEWAPI int console_TestToken(char *token, char *str)
{
	int i = 0;
	while(token[i])
	{
		if(token[i] != str[i])
		{
			return 0;
		}
		i++;
	}
	
	if(str[i] == ' ' || str[i] == '\0')
	{
		return 1;
	}
	return 0;

}


PEWAPI void console_Enable(char *param_str)
{
	int i=0;
	while(param_str[i]==' ')i++;
	if(param_str[i]=='\0')
	{
		console_Print(MESSAGE_NORMAL, "enable: command used to enable different features of the engine.\n");
	}

	/*else if(console_TestToken("debug", param_str + i))
	{
		i+=5;
		while(param_str[i]==' ')i++;
		if(param_str[i]=='L')
		{
			i+=1;
			switch(param_str[i])
			{
				case '1':
					draw_SetDebugLevel(DEBUG_L1);
				break;
						
				case '2':
					draw_SetDebugLevel(DEBUG_L2);
				break;
						
				case '3':
					draw_SetDebugLevel(DEBUG_L3);
				break;
			}
		}
	}*/

	else if(console_TestToken("echo", param_str + i))
	{
		i+=4;
		console.bm_flags&= ~CONSOLE_NO_ECHO;
	}

	else if(console_TestToken("wireframe", param_str + i))
	{
		i+=9;
		draw_SetRenderDrawMode(RENDER_DRAWMODE_WIREFRAME);
	}
	
	else if(console_TestToken("render_flag", param_str + i))
	{
		i+=11;
		while(param_str[i]==' ')i++;
		
		if(console_TestToken("use_shadow_maps", param_str + i))
		{
			i+=15;
			draw_SetRenderFlags(renderer.renderer_flags|RENDERFLAG_USE_SHADOW_MAPS);
		}

		else if(console_TestToken("use_light_volumes", param_str + i))
		{
			i+=18;
			draw_SetRenderFlags(renderer.renderer_flags|RENDERFLAG_DRAW_LIGHT_VOLUMES);
		}
		
		else if(console_TestToken("use_bloom", param_str + i))   		
		{
			i+=9;
			draw_SetRenderFlags(renderer.renderer_flags|RENDERFLAG_USE_BLOOM);
		}
		
	}
	else if(console_TestToken("debug_flag", param_str + i))
	{
		i += 10;
		
		while(param_str[i] == ' ' && param_str[i] != '\0') i++;
		if(console_TestToken("draw_armatures", param_str + i))
		{
			draw_SetDebugFlag(DEBUG_DRAW_ARMATURES);
		}
		else if(console_TestToken("draw_light_origins", param_str + i))
		{
			draw_SetDebugFlag(DEBUG_DRAW_LIGHT_ORIGINS);
		}
		else if(console_TestToken("draw_light_limits", param_str + i))
		{
			draw_SetDebugFlag(DEBUG_DRAW_LIGHT_LIMITS);
		}
		else if(console_TestToken("draw_z_buffer", param_str + i))
		{
			draw_SetDebugFlag(DEBUG_DRAW_ZBUFFER);
		}
		else if(console_TestToken("draw_n_buffer", param_str + i))
		{
			draw_SetDebugFlag(DEBUG_DRAW_NBUFFER);
		}
		else if(console_TestToken("draw_entity_origin", param_str + i))
		{
			draw_SetDebugFlag(DEBUG_DRAW_ENTITY_ORIGIN);
		}
		else if(console_TestToken("draw_entity_aabb", param_str + i))
		{
			draw_SetDebugFlag(DEBUG_DRAW_ENTITY_AABB);
		}
	}
	
}

PEWAPI void console_Disable(char *param_str)
{
	int i=0;
	while(param_str[i]==' ')i++;
	if(param_str[i]=='\0')
	{
		console_Print(MESSAGE_NORMAL, "disable: command used to disable different features of the engine.\n");
	}
	
	else if(console_TestToken("debug", param_str + i))
	{
		i+=5;
		draw_SetDebugFlag(DEBUG_DISABLED);
	}
	
	else if(console_TestToken("echo", param_str + i))
	{
		i+=4;
		console.bm_flags|= CONSOLE_NO_ECHO;
	}
	
	else if(console_TestToken("wireframe", param_str + i))
	{
		i+=9;
		draw_SetRenderDrawMode(RENDER_DRAWMODE_LIT);
	}
	
	else if(console_TestToken("render_flag", param_str + i))
	{
		i+=11;
		while(param_str[i]==' ')i++;
		
		if(console_TestToken("use_shadow_maps", param_str + i))
		{
			i+=15;
			draw_SetRenderFlags(renderer.renderer_flags&(~RENDERFLAG_USE_SHADOW_MAPS));
		}
		
		else if(console_TestToken("use_light_volumes", param_str + i))
		{
			i+=18;
			draw_SetRenderFlags(renderer.renderer_flags&(~RENDERFLAG_DRAW_LIGHT_VOLUMES));
		}
		
		else if(console_TestToken("use_bloom", param_str + i))  
		{
			i+=9;
			draw_SetRenderFlags(renderer.renderer_flags& (~RENDERFLAG_USE_BLOOM));
		}
		
	}
	else if(console_TestToken("debug_flag", param_str + i))
	{
		i += 10;
		while(param_str[i] == ' ' && param_str[i] != '\0') i++;
		if(console_TestToken("draw_armatures", param_str + i))
		{
			draw_ResetDebugFlag(DEBUG_DRAW_ARMATURES);
		}
		else if(console_TestToken("draw_light_origins", param_str + i))
		{
			draw_ResetDebugFlag(DEBUG_DRAW_LIGHT_ORIGINS);
		}
		else if(console_TestToken("draw_light_limits", param_str + i))
		{
			draw_ResetDebugFlag(DEBUG_DRAW_LIGHT_LIMITS);
		}
		else if(console_TestToken("draw_z_buffer", param_str + i))
		{
			draw_ResetDebugFlag(DEBUG_DRAW_ZBUFFER);
		}
		else if(console_TestToken("draw_n_buffer", param_str + i))
		{
			draw_ResetDebugFlag(DEBUG_DRAW_NBUFFER);
		}
		else if(console_TestToken("draw_entity_origin", param_str + i))
		{
			draw_ResetDebugFlag(DEBUG_DRAW_ENTITY_ORIGIN);
		}
		else if(console_TestToken("draw_entity_aabb", param_str + i))
		{
			draw_ResetDebugFlag(DEBUG_DRAW_ENTITY_AABB);
		}
	}
}


PEWAPI void console_Help(char *param_str)
{
	int i=0;
	while(param_str[i]==' ')i++;
	if(param_str[i]=='\0')
	{
		console_Print(MESSAGE_NORMAL, help_no_args);
	}
	else if(param_str[i]=='d' && 
			param_str[i+1]=='e' && 
			param_str[i+2]=='b' && 
			param_str[i+3]=='u' && 
			param_str[i+4]=='g' && 
			(param_str[i+5]=='\0' ||param_str[i+5]==' '))
	{
		i+=5;
		console_Print(MESSAGE_NORMAL, help_debug);
	}
	else if(param_str[i]=='e' && 
			param_str[i+1]=='c' && 
			param_str[i+2]=='h' && 
			param_str[i+3]=='o' &&  
			(param_str[i+4]=='\0' ||param_str[i+4]==' '))
	{
		i+=4;
		console_Print(MESSAGE_NORMAL, help_echo);
	}
	
	else if(param_str[i]=='e' && 
			param_str[i+1]=='n' && 
			param_str[i+2]=='a' &&
			param_str[i+3]=='b' &&
			param_str[i+4]=='l' &&
			param_str[i+5]=='e' &&
			(param_str[i+6]=='\0' || param_str[i+6]==' '))
	{
		i+=6;
		console_Print(MESSAGE_NORMAL, help_enable);
	}
	
	else if(param_str[i]=='d' && 
			param_str[i+1]=='i' && 
			param_str[i+2]=='s' &&
			param_str[i+3]=='a' &&
			param_str[i+4]=='b' &&
			param_str[i+5]=='l' &&
			param_str[i+6]=='e' &&
			(param_str[i+7]=='\0' || param_str[i+7]==' '))
	{
		i+=7;
		console_Print(MESSAGE_NORMAL, help_disable);
	}
	else if(param_str[i]=='r' && 
		    param_str[i+1]=='e' &&
			param_str[i+2]=='n' &&
			param_str[i+3]=='d' &&
			param_str[i+4]=='e' &&
			param_str[i+5]=='r' &&
			param_str[i+6]=='_' &&
			param_str[i+7]=='m' &&
			param_str[i+8]=='o' &&
			param_str[i+9]=='d' &&
			param_str[i+10]=='e' &&
			(param_str[i+11]=='\0' || param_str[i+11]==' '))
	{
		i+=11;
		console_Print(MESSAGE_NORMAL, help_render_mode);
	}
	
}

/*
=============
console_ClearConsoleBuffer
=============
*/
PEWAPI void console_ClearConsoleBuffer()
{
	console.cursor_x=0;
	console.cursor_y=0;
	console.scrolled_lines=0;
	return;
}


/*
=============
console_ClearConsoleInputLine
=============
*/
PEWAPI void console_ClearConsoleInputLine()
{
	register int i;
	i=console.input_line_cursor;
	for(i=console.input_line_cursor-1; i>=0; i--)
	{
		console.input_line[i]='\0';
	}
	console.input_line_cursor=0;
	/*console.input_line[console.input_line_cursor]='\0';*/
	return;
}

/*ODDAPI*/

/*
=============
console_Print
=============
*/
/* TODO: implement console wrap-around */
PEWAPI void console_Print(int message_type, char *str, ...)
{
	unsigned int param;
	float fparam;
	int param_str_len;
	int param_str_index;
	int param_byte_offset=0;
	register int i;
	register int c;
	register int k;
	c=strlen(str)+1;
	char_t ch;
	char byte;
	const char *err="error: ";
	const char *warn="warning: ";
	const char *ptr;
	char n_str[128];
	int decimal_point;
	int sign;
	switch(message_type)
	{
		case MESSAGE_ERROR:
			ch.type=MESSAGE_ERROR;
			ptr=err;
			goto PRINT;
		case MESSAGE_WARNING:
			ch.type=MESSAGE_WARNING;	
			ptr=warn;
			
			PRINT:
			for(k=0; ptr[k]; k++)
			{
				ch.char_code=ptr[k];
				console.console_text_buffer[console.cursor_x + console.cursor_y*console.line_width]=ch;
				console.cursor_x++;
				if(console.cursor_x>console.line_width-1)
				{
					console.cursor_y++;
					console.cursor_x=0;
				}
			}
		break;

	}
	
	for(i=0; i<c; i++)
	{
		if(str[i]=='%')
		{
			//i++;
			if(str[i+1]!='f')
			{
				asm volatile
			   (
			   		".intel_syntax noprefix\n"
					"lea %%eax, 16[ebp][%[byte_offset]]\n"	
			   		"mov %%eax, DWORD PTR [%%eax]\n"
			   		"mov %[p], %%eax\n"
			   		".att_syntax\n"
			   		: [p] "=rm" (param) : [byte_offset] "r" (param_byte_offset) :"eax"
			   );
			}
			else
			{
				/*asm
			   (
			   		".intel_syntax noprefix\n"
					"lea %%eax, 16[ebp][%[byte_offset]]\n"	
			   		"mov %%eax, DWORD PTR [%%eax]\n"
			   		"mov %[p], %%eax\n"
			   		".att_syntax\n"
			   		: [p] "=rm" (fparam) : [byte_offset] "r" (param_byte_offset)
			   );
			   printf("%f\n", fparam);*/
				//param=0;
				fparam=0.0;
			}
			
			
			param_byte_offset+=4;
			
			switch(str[i+1])
			{
				case 's':
					param_str_len=strlen((char *)param)+1;   
					ch.type=message_type;
				break;
				
				
				case 'd':
					itoa(param, n_str, 10);
					param_str_len=strlen(n_str)+1;
					param=(int )n_str;
					ch.type=message_type;
				break;
				
				case 'f':
				
					param=(int)fcvt(fparam, 6, &decimal_point, &sign);
					param_str_len=strlen((char *)param)+1;
					if(fparam<1.0)
					{
						
						for(k=0; k<param_str_len && k<128; k++)
						{
							n_str[k+1]=((char *)param)[k];
						}
						n_str[0]='0';
						param_str_len++;
						decimal_point++;
					}
					
					else
					{
						for(k=0; k<param_str_len && k<128; k++)
						{
							n_str[k]=((char *)param)[k];
						}
					}

					//free(param);
					param=(int)n_str;
					
					for(k=param_str_len-1;k<128 && k>=decimal_point; k--)
					{
						((char *)param)[k+1]=((char *)param)[k];
					}
					((char *)param)[k+1]='.';
					param_str_len++;
					
				break;
				
				default:
					goto SKIP_PARAM;
				
			}

			for(param_str_index=0; param_str_index<param_str_len; param_str_index++)
			{
				ch.char_code=((char *)param)[param_str_index];
				console.console_text_buffer[console.cursor_x + console.cursor_y*console.line_width]=ch;
				console.cursor_x++;
				if(console.cursor_x>console.line_width-1)
				{
					console.cursor_y++;
					console.cursor_x=0;
				}
			}
			i+=2;
			console.cursor_x--;
		}
		SKIP_PARAM:
		
		if(str[i]==10)
		{
			/* bad way to do this, but for now it will do... */
			ch.char_code=32;
			for(; console.cursor_x<console.line_width; console.cursor_x++)
			{
				console.console_text_buffer[console.cursor_x + console.cursor_y*console.line_width]=ch;
			}
			console.cursor_y++;
			console.cursor_x=0;
			continue;
		}
		else if(str[i]==13)
		{
			//i+=2;
			//printf("%d\n", str[i]);
			console.cursor_x=0;
			continue;
		}
		else ch.char_code=str[i];
		ch.type=message_type;
		ch.r=console.text_color.r;
		ch.g=console.text_color.g;
		ch.b=console.text_color.b;
		
		console.console_text_buffer[console.cursor_x + console.cursor_y*console.line_width]=ch;
		console.cursor_x++;
		if(console.cursor_x>console.line_width-1)
		{
			console.cursor_y++;
			console.cursor_x=0;
		}
	}
	console.cursor_x--;

}


/*
=============
console_SetTextColor
=============
*/
PEWAPI void console_SetTextColor(char r, char g, char b)
{
	console.text_color.r=r;
	console.text_color.g=g;
	console.text_color.b=b;
}



/*
=============
console_BlitTextBuffer
=============
*/
/* TODO: implement console wrap-around */
PEWAPI void console_BlitTextBuffer()
{
	register int j;
	int ch;
	char_t *str=console.console_text_buffer;
	int x=0;
	int y=console.cursor_y-console.scrolled_lines; 
	int str_len=console.cursor_x + console.cursor_y*console.line_width;
	static int cursor_blink=0;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for(j=0; j<str_len-console.scrolled_lines*console.line_width; j++)
	{
		ch=str[j].char_code-32;
		if(ch==-22)
		{
			x=0;
			y--;
			continue;
		}
		switch(str[j].type)
		{
			case MESSAGE_NORMAL:
				glColor3f(0.0, 1.0, 0.0);
				glRasterPos3f(0.0, -0.5, -0.11);
			break;
			
			case MESSAGE_WARNING:
				glColor3f(1.0, 1.0, 0.0);
				glRasterPos3f(0.0, -0.5, -0.11);
			break;
			
			case MESSAGE_ERROR:
				glColor3f(1.0, 0.0, 0.0);
				glRasterPos3f(0.0, -0.5, -0.11);
			break;
			
			case MESSAGE_FREE:
				glColor3f(color_conversion_lookup_table[str[j].r], color_conversion_lookup_table[str[j].g], color_conversion_lookup_table[str[j].b]);
				glRasterPos3f(0.0, -0.5, -0.11);
			break;
		}
		glBitmap(12, 20, console_alignment[renderer.selected_resolution][0]-(x*12), console_alignment[renderer.selected_resolution][1]-(y*20), 0, 0, (unsigned char *) font.chars[ch].start);
		x++;
		if(x>console.line_width-1)
		{
			x=0;
			y--;
		}
	}
	
	glColor3f(0.6, 0.6, 0.6);
	glRasterPos3f(0.0, -0.5, -0.11);
	
	for(j=0; j<console.input_line_cursor; j++)
	{
		ch=console.input_line[j]-32;
		glBitmap(12, 20, console_alignment[renderer.selected_resolution][0], console_alignment[renderer.selected_resolution][2], 12, 0, (unsigned char *)font.chars[ch].start);
	}
	
	cursor_blink++;
	if(cursor_blink<30)
	{
		glColor3f(0.5, 0.5, 0.5);
		glRasterPos3f(0.0, -0.5, -0.11);
		glBitmap(12, 20, console_alignment[renderer.selected_resolution][0]-(console.input_line_cursor*12), console_alignment[renderer.selected_resolution][2], 0, 0, (unsigned char *) font.chars['|'-32].start);
	}
	if(cursor_blink>60)cursor_blink=0;
}



#ifdef __cplusplus
}
#endif

















