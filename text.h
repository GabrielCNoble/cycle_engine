#ifndef TEXT_H
#define TEXT_H

#include "conf.h"
#include "includes.h"


/* To render a glyph using freetype, the
workflow is as follows:
	- load the font (FT_Face) from disk using FT_New_Face, passing the address of the font variable to the function
	- set the font size using FT_Set_Char_Size
	- get the desired glyph index from the font using FT_Get_Char_Index
	- load the glyph in the FT_Glyph_Slot of the font using FT_Load_Glyph
	- call FT_Render_Glyph to turn the glyph into a bitmap
	- copy the bytes renderer through FT_Face->glyph->bitmap.buffer to another location */
	
	
/* FT_Set_Char_Size set the font's active char size */

/* A FT_Face is a typedef'd derivation of FT_FaceRec (typedef struct FT_FaceRec_*  FT_Face;) */


/*enum FONT_Y_OFFSETS
{
	OFFSET_12_1=2,
	OFFSET_12_2=0,
	OFFSET_16_1=5,
	OFFSET_16_2=3,
	OFFSET_32_1=10,
	OFFSET_32_2=6
};*/

enum FONT_SIZE
{
	FONT_SIZE_4 = 1,
	FONT_SIZE_8 = 2,
	FONT_SIZE_10 = 4,
	FONT_SIZE_12 = 8,
	FONT_SIZE_16 = 16,
	FONT_SIZE_22 = 32,
	FONT_SIZE_24 = 64,
	FONT_SIZE_28 = 128,
	FONT_SIZE_32 = 256
};

//typedef struct text_renderer_t
//{
//	FT_Library font_renderer;
//}text_renderer_t;

/*typedef struct fchar_t
{
	int char_code;
	int width;
	int height;
	int buffer_entry;
}fchar_t;*/

typedef struct font_t
{
	//FT_Face font;
	TTF_Font *font;
	//int max_chars;
	//int char_count;
	//fchar_t *chars;
	unsigned int tex;
	unsigned int size;
	int max_width;
	int max_height;
	unsigned char *buffer;
	unsigned char *texture_buffer;
	char *name;
}font_t;

typedef struct font_array
{
	int array_size;
	int font_count;
	font_t *fonts;
}font_array;

//typedef struct
//{
//	int width;
//	int height;
//	int row_pitch;				/* how much bytes between string rows... */
//	int char_count;				/* char count plus null */
//	unsigned char *start;
//	char *name;					/* the name of this cached string... (maybe use some sort of hash mechanism?) */
//}cached_string_t;				/* for frequently used strings (GUI and stuff...) */

//typedef struct
//{
//	int cache_size;
//	int string_count;
//	cached_string_t *cached_strings;
//}string_cache_t;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void text_Init(char *path);

PEWAPI void text_Finish();

PEWAPI void text_ResizeFontArray(int new_size);

//PEWAPI void text_ExtendFont(font_t *font, int new_size);

PEWAPI void text_LoadFont(char *file_name, char *name);

PEWAPI int text_GetFontIndex(char *name);

//PEWAPI void text_CreateFont(font_t *font);

//PEWAPI void text_CacheString(char *str, font_t *font);

//PEWAPI void text_GetCacheStringIndex(char *name);

PEWAPI void text_UncacheString(int string_index);

PEWAPI void text_ResizeStringCache(int new_size);

#ifdef __cplusplus
}
#endif

#endif /* TEXT_H */














