#include "text.h"
#include "log.h"
static FT_Library text_renderer;
font_array font_a;

static int font_path_len = 0;
static char font_path[256];

#ifdef __cplusplus
extern "C"
{
#endif

/*
=============
void text_Init
=============
*/
PEWAPI void text_Init(char *path)
{
	if(TTF_Init())
	{
		log_LogMessage("SDL_ttf: error initializing!");
		exit(-3);
	}
	
	strcpy(font_path, path);
	font_path_len = strlen(font_path);
	font_a.fonts=NULL;
	font_a.font_count=0;
	text_ResizeFontArray(8);
}


/*
=============
void text_Finish
=============
*/
PEWAPI void text_Finish()
{
	/*int i;
	int c;

	c=font_a.font_count;
	for(i=0; i<c; i++)
	{
		free(font_a.fonts[i].chars);
		free(font_a.fonts[i].buffer);
	}
	free(font_a.fonts);
	
	FT_Done_FreeType(text_renderer);*/
}


/*
=============
void text_ResizeFontArray
=============
*/
PEWAPI void text_ResizeFontArray(int new_size)
{
	font_t *temp=(font_t *)calloc(new_size, sizeof(font_t));
	if(font_a.fonts)
	{
		memcpy(temp, font_a.fonts, sizeof(font_t)*font_a.font_count);
		free(font_a.fonts);
	}
	font_a.fonts=temp;
	font_a.array_size=new_size;
	return;
}


/*
=============
static void text_ExtendFont
=============
*/
/*PEWAPI void text_ExtendFont(font_t *font, int new_size)
{
	fchar_t *temp=(fchar_t *)calloc(new_size, sizeof(fchar_t));
	if(font->chars)
	{
		memcpy(temp, font->chars, sizeof(fchar_t)*font->char_count);
		free(font->chars);
	}
	font->chars=temp;
	font->max_chars=new_size;
}*/


/*
=============
void text_LoadFont
=============
*/
/* TODO: Generalize more this function. Make it able to load all the chars that exist
inside the file, not just a hardcoded set. */
PEWAPI void text_LoadFont(char *file_name, char *name)
{
	
	char full_path[256];
	int font_index = font_a.font_count++;
	font_t *font;
	
	if(font_index >= font_a.array_size)
	{
		text_ResizeFontArray(font_a.array_size + 16);
	}
	
	
	font = &font_a.fonts[font_index];
	
	
	
	strcpy(full_path, font_path);
	strcat(full_path, file_name);
	
	//printf("%s\n", full_path);
	
	if(!(font->font = TTF_OpenFont(full_path, 32)))
	{
		printf("couldn't open font. %s\n", TTF_GetError());
		return;
	}
	TTF_SetFontHinting(font->font, TTF_HINTING_NORMAL);
	TTF_SetFontStyle(font->font, TTF_STYLE_NORMAL);
	//printf("%s\n", TTF_GetError());
	font->name = strdup(name);
	
}


/*
=============
void text_GetFontIndex
=============
*/
PEWAPI int text_GetFontIndex(char *name)
{
	register int i;
	register int c;
	c=font_a.font_count;
	for(i=0; i<c; i++)
	{
		if(!strcmp(name, font_a.fonts[i].name))
		{
			return i;
		}
	}
	return -1;
}


/*
=============
void text_CreateFont
=============
*/
/*PEWAPI void text_CreateFont(font_t *font)
{
	if(font)
	{
		if(font_a.font_count>=font_a.array_size)
		{
			text_ResizeFontArray(font_a.array_size+8);
		}
		font_a.fonts[font_a.font_count++]=*font;
	}
	
}*/

#ifdef __cplusplus
}
#endif










