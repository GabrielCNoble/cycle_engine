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
	int font_index = font_a.font_count;
	font_t *font;
	
	if(font_index >= font_a.array_size)
	{
		text_ResizeFontArray(font_a.array_size + 16);
	}
	
	
	font = &font_a.fonts[font_index];
	
	
	
	strcpy(full_path, font_path);
	strcat(full_path, file_name);
	
	if(!(font->font = TTF_OpenFont(file_name, 0)))
	{
		return;
	}
	
	font->name = strdup(name);
	
	/*int i;
	int j;
	int k;
	int l;
	int w;
	int h;
	int m;
	int n;
	int max_w;
	int max_h;
	int min_w;
	int min_h;
	int d_w;
	int d_h;
	int a;
	int b;
	int offset;
	int size = 0;
	int glyph_index = 0;
	int char_count = 0;
	unsigned char *temp;
	font_t f;
	char full_path[512];
	strcpy(full_path, font_path);
	strcat(full_path, file_name);
	
	
	if(!FT_New_Face(text_renderer, full_path, 0, &f.font))
	{
		for(i = 0; i < 9; i++)
		{
			strcpy(full_path, name);
			if(bm_sizes & 1)
			{
				switch(i)
				{
					case 0:
						strcat(full_path, "_4");
						size = 4;
					break;
					
					case 1:
						strcat(full_path, "_8");
						size = 8;
					break;
					
					case 2:
						strcat(full_path, "_10");
						size = 10;
					break;
					
					case 3:
						strcat(full_path, "_12");
						size = 12;
					break;
					
					case 4:
						strcat(full_path, "_16");
						size = 16;
					break;
					
					case 5:
						strcat(full_path, "_22");
						size = 22;
					break;
					
					case 6:
						strcat(full_path, "_24");
						size = 24;
					break;
					
					case 7:
						strcat(full_path, "_28");
						size = 28;
					break;
					
					case 8:
						strcat(full_path, "_32");
						size = 32;
					break;
					
					default:
						size = 0;
						bm_sizes >>= 1;
					break;
				}
				
				if(!size)
				{
					continue;
				}
				
				if(font_a.font_count >= font_a.array_size)
				{
					text_ResizeFontArray(font_a.array_size + 8);
				}
				
				f.name = strdup(full_path);
				f.size = size;
				temp = (unsigned char *)malloc(f.font->num_glyphs * (f.font->bbox.xMax - f.font->bbox.xMin) * (f.font->bbox.yMax - f.font->bbox.yMin));
				//f.buffer = (unsigned char *)malloc(f.font->num_glyphs * (f.font->bbox.xMax - f.font->bbox.xMin) * (f.font->bbox.yMax - f.font->bbox.yMin));
				//f.buffer = NULL;
				f.chars = (fchar_t *)malloc(sizeof(fchar_t) * f.font->num_glyphs);
				f.char_count = 0;
				FT_Set_Char_Size(f.font, size * 1000, size * 1000, 0, 0);
				offset = 0;
				
				max_w = 0;
				max_h = 0;
				min_w = 100000000;
				min_h = 100000000;
				for(j = '!'; j <= '~' ; j++)
				{
					glyph_index = FT_Get_Char_Index(f.font, j);
					FT_Load_Glyph(f.font, glyph_index, FT_LOAD_DEFAULT);
					FT_Render_Glyph(f.font->glyph, FT_RENDER_MODE_NORMAL);
					
					
					w = f.font->glyph->bitmap.pitch;
					h = f.font->glyph->bitmap.rows;
					
					printf("load %c   %d %d\n", j, w, h);
					
					if(w > max_w) max_w = w;
					if(h > max_h) max_h = h;
					
					if(w < min_w) min_w = w;
					if(h < min_h) min_h = h;
					
					//temp = (unsigned char *)malloc(f.font->glyph->bitmap.pitch * f.font->glyph->bitmap.rows + offset);
					//if(f.buffer)
					//{
					//	memcpy(temp, f.buffer, offset);
					//	free(f.buffer);
					//}
					//f.buffer = temp;
					//for(k = 0; k < f.font->glyph->bitmap.rows * f.font->glyph->bitmap.pitch; k++)
					//for(k = f.font->glyph->bitmap.rows * f.font->glyph->bitmap.pitch - 1; k >= 0; k--);
					//{
					//	f.buffer[offset + k] = f.font->glyph->bitmap.buffer[k];
				//	}
					
					
					
					//printf("%d %d\n", w, h);
			
					for(k = h - 1 ; k >= 0; k--)
					{
						for(l = 0; l < w; l++)
						{
							temp[offset + k * w + l] = f.font->glyph->bitmap.buffer[(h - k - 1) * w + l];
						}
					}
					
					f.chars[f.char_count].buffer_entry = offset;
					f.chars[f.char_count].width = f.font->glyph->bitmap.pitch;
					f.chars[f.char_count].height = f.font->glyph->bitmap.rows;
					f.chars[f.char_count].char_code = j;
					f.char_count ++;
					offset += w * h;	
					//offset += k;
				}
				
				printf("%d %d  %d %d\n", max_w, max_h, min_w, min_h);
				
				//temp = f.buffer;
				//f.buffer = temp;
				f.buffer = (unsigned char *)malloc(f.char_count * max_w * max_h * 4);
				
				//f.texture_buffer = (unsigned char*) malloc(f.char_count * (f.font->bbox.xMax - f.font->bbox.xMin) * (f.font->bbox.yMax - f.font->bbox.yMin) * 4);
				
				offset = 0;
				
				f.max_width = max_w;
				f.max_height = max_h;
				
				
				for(n = 0; n < max_h; n++)
				{
					for(k = 0; k < f.char_count; k++)
					{
						for(m = 0; m < max_w; m++)
						{
							f.buffer[((n * f.char_count + k) * max_w + m) * 4	 ] = 127;
							f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 1] = 127;
							f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 2] = 200;
							f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 3] = 127;
						}
					}
				}
				
				a = 0;
				b = 0;
				
				for(n = 0; n < max_h; n++)
				{
					
					if(b) a++;
					
					
					for(k = 0; k < f.char_count; k++)
					{
						d_h = (max_h - f.chars[k].height) / 2;
						d_w = (max_w - f.chars[k].width) / 2;
						
						b = 0;
						
						if(n < d_h || n > f.chars[k].height + d_h)
						{
							for(m = 0; m < max_w; m++)
							{
								f.buffer[((n * f.char_count + k) * max_w + m) * 4	 ] = 200;
								f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 1] = 127;
								f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 2] = 127;
								f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 3] = 127;
							}
						}
						else
						{
							for(m = 0; m < max_w; m++)
							{
								if(m < d_w || m > f.chars[k].width + d_w)
								{
									f.buffer[((n * f.char_count + k) * max_w + m) * 4	 ] = 127;
									f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 1] = 200;
									f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 2] = 127;
									f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 3] = 127;
								}
								else
								{
									f.buffer[((n * f.char_count + k) * max_w + m) * 4	 ] = temp[f.chars[k].buffer_entry + f.chars[k].width * a + b];
									f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 1] = temp[f.chars[k].buffer_entry + f.chars[k].width * a + b];
									f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 2] = temp[f.chars[k].buffer_entry + f.chars[k].width * a + b];
									f.buffer[((n * f.char_count + k) * max_w + m) * 4 + 3] = temp[f.chars[k].buffer_entry + f.chars[k].width * a + b];
									b++;

						
								}	
							}
						}
					}
				}
						
				glGenTextures(1, &f.tex);
				glBindTexture(GL_TEXTURE_2D, f.tex);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				
				while(glGetError()!=GL_NO_ERROR);
				
				w = max_w * f.char_count;
				h = max_h;
				
				printf("%d %d\n", w, h);
				
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8192, 8192, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, max_w * f.char_count, max_h, GL_RGBA, GL_UNSIGNED_BYTE, f.buffer);
				

				
				printf("%x\n", glGetError());
				
				font_a.fonts[font_a.font_count++] = f;
				
			}
			
			bm_sizes >>= 1;
		}
		
	}*/
	
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










