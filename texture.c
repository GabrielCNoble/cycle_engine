#include "texture.h"
#include "shader.h"
#include "draw.h"
#include "console.h"
#include "soil/SOIL.h"


texture_array texture_a;
extern shader_array shader_a;
extern renderer_t renderer;

int texture_path_len = 0;
static char texture_path[256];

#define MFT_MAX_FRAME_PATH 128

#ifdef __cplusplus
extern "C"
{
#endif
/*
=============
texture_Init
=============
*/
PEWAPI void texture_Init(char *path)
{
	texture_a.textures=NULL;
	texture_a.texture_count=0;
	texture_ResizeTextureArray(16);
	strcpy(texture_path, path);
	texture_path_len = strlen(texture_path);
	return;
}


/*
=============
texture_Finish
=============
*/
PEWAPI void texture_Finish()
{
	texture_PurgeAllTextures();
	free(texture_a.textures);
}


/*
=============
texture_PurgeAllTextures
=============
*/
PEWAPI void texture_PurgeAllTextures()
{
	register int i;
	register int c;
	c=texture_a.texture_count;
	
	for(i=c-1; i>=0; i--)
	{
		glDeleteTextures(1, (const GLuint *)&texture_a.textures[i].tex_ID);
		texture_a.texture_count--;
	}
}


/*
=============
texture_ResizeTextureArray
=============
*/
PEWAPI void texture_ResizeTextureArray(int new_size)
{
	texture_t *temp=(texture_t *)calloc(new_size, sizeof(texture_t));
	if(texture_a.textures)
	{
		memcpy(temp, texture_a.textures, sizeof(texture_t)*texture_a.texture_count);
		free(texture_a.textures);
	}
	texture_a.textures=temp;
	texture_a.array_size=new_size;
	return;
}


/*
=============
texture_LoadTexture
=============
*/
/* TODO: better support for npot lenght textures... also, treat byte alignment... */
PEWAPI int texture_LoadTexture(char *filename, char *name, int gamma_correct)
{
	GLuint tex_ID;
	unsigned char *pixels;
	unsigned char *frame;
	FILE *mft_file=NULL;
	char *mft_string=NULL;
	int mft_string_size;
	int mft_string_index;
	int mft_frame_count;
	int mft_frame_width;
	int mft_frame_height;
	int aux_index;
	char mft_frame_count_str[20];					/* a 20 digit number shoud be more than enough */
	char mft_frame_path[MFT_MAX_FRAME_PATH];			/* 128 char path might be enough */
	int channels;
	int internal_format;
	int format;
	int w_h_c;
	int w;
	int h;
	int i;
	int j;
	int k;
	char full_path[256];
	float f;
	strcpy(full_path, texture_path);
	strcat(full_path, filename);
	/*while(filename[i]!='.')i++;
	i++;
	if(filename[i]=='m' && filename[i+1]=='f' && filename[i+2]=='t')
	{
		if(!(mft_file=fopen(filename, "r")))
		{
			console_Print(MESSAGE_ERROR, "couldnt open multi frame texture [%s]\n", name);
			return -1;
		}
		
		mft_string_size=0;
		while(!feof(mft_file))
		{
			fgetc(mft_file);
			mft_string_size++;
		}
		if(mft_string_size<2)
		{
			console_Print(MESSAGE_ERROR, "multi frame texture file [%s] is invalid\n", name);
			fclose(mft_file);
			return -1;
		}
		rewind(mft_file);
		mft_string=(char *)malloc(mft_string_size+1);
		
		mft_string_index=0;
		while(!feof(mft_file))
		{
			*(mft_string+mft_string_index)=fgetc(mft_file);
			mft_string_index++;
		}
		*(mft_string + mft_string_index-1)='\0';
		
		mft_string_index=0;
		

		while((mft_string[mft_string_index]==' ' || mft_string[mft_string_index]=='\n') && mft_string[mft_string_index]!='\0') mft_string_index++;
		
		if(
			mft_string[mft_string_index]=='F' && 
			mft_string[mft_string_index+1]=='R' &&
		   	mft_string[mft_string_index+2]=='A' &&
		   	mft_string[mft_string_index+3]=='M' &&
		   	mft_string[mft_string_index+4]=='E' &&
		   	mft_string[mft_string_index+5]=='S' &&
		   	mft_string[mft_string_index+6]==' '
		  )
		{
		 	mft_string_index+=7;
			while((mft_string[mft_string_index]==' ' || mft_string[mft_string_index]=='\n') && mft_string[mft_string_index]!='\0') mft_string_index++;
			
			if(mft_string[mft_string_index]<47 || mft_string[mft_string_index]>58)
			{
				console_Print(MESSAGE_ERROR, "multi frame texture file [%s] is invalid\n", name);
				fclose(mft_file);
				free(mft_string);
				return -1;
			}
			
			aux_index=0;
			while((mft_string[mft_string_index]>='0' && mft_string[mft_string_index]<='9') && mft_string[mft_string_index]!='\0' && aux_index<19)
			{
				mft_frame_count_str[aux_index++]=mft_string[mft_string_index++];
			}
			
			mft_frame_count_str[aux_index]='\0';
			mft_frame_count=atoi(mft_frame_count_str);
			if(mft_frame_count==0)
			{
				console_Print(MESSAGE_ERROR, "multi frame texture file [%s] has no frames\n", name);
				return -1;
			}
		}
		printf("%d\n", mft_frame_count);
		
		while((mft_string[mft_string_index]==' ' || mft_string[mft_string_index]=='\n') && mft_string[mft_string_index]!='\0') mft_string_index++;
		if(
			mft_string[mft_string_index]=='B' &&
			mft_string[mft_string_index+1]=='E' &&
			mft_string[mft_string_index+2]=='G' &&
			mft_string[mft_string_index+3]=='I' &&
			mft_string[mft_string_index+4]=='N'
		  )
		{
			mft_string_index+=5;
			while((mft_string[mft_string_index]==' ' || mft_string[mft_string_index]=='\n') && mft_string[mft_string_index]!='\0') mft_string_index++;
		
			aux_index=0;
			while(!(mft_string[mft_string_index]==' ' || mft_string[mft_string_index]=='\n') && mft_string[mft_string_index]!='\0' && aux_index<MFT_MAX_FRAME_PATH-1)
			{
				mft_frame_path[aux_index++]=mft_string[mft_string_index++];
			}
			mft_frame_path[aux_index]='\0';
			
			printf("%s\n", mft_frame_path);
			
			frame=SOIL_load_image(mft_frame_path, &w, &h, &channels, 4);
			if(!frame)
			{
				console_Print(MESSAGE_ERROR, "couldnt load first frame of [%s]\n", name);
				fclose(mft_file);
				return -1;
			}

			
			pixels=(unsigned char *)calloc(channels*w*h*mft_frame_count, 1);

			if(!pixels)
			{
				console_Print(MESSAGE_ERROR, "not enough memory to create mft [%s]\n", name);
				fclose(mft_file);
				return -1;
			}
			w_h_c=channels*w*h;
			memcpy(pixels, frame, w_h_c);
			free(frame);
			
			for(k=1; k<mft_frame_count; k++)
			{
				while((mft_string[mft_string_index]==' ' || mft_string[mft_string_index]=='\n') && mft_string[mft_string_index]!='\0') mft_string_index++;
				if(mft_string[mft_string_index]=='\0') break;
		
				aux_index=0;
				while(!(mft_string[mft_string_index]==' ' || mft_string[mft_string_index]=='\n') && mft_string[mft_string_index]!='\0' && aux_index<MFT_MAX_FRAME_PATH-1)
				{
					mft_frame_path[aux_index++]=mft_string[mft_string_index++];
				}
				mft_frame_path[aux_index]='\0';
				printf("%s\n", mft_frame_path);
				frame=SOIL_load_image(mft_frame_path, &w, &h, &channels, 4);
				if(!frame)
				{
					console_Print(MESSAGE_ERROR, "couldnt load subsequent frames of [%s]\n", name);
					fclose(mft_file);
					free(pixels);
					printf("frames loaded: %d\n", k);
					return -1;
				}
				memcpy(pixels + k*w_h_c, frame, w_h_c);
				free(frame);
				
			}
			
		}
		
		free(mft_string);
		fclose(mft_file);
		console_Print(MESSAGE_NORMAL, "multiframe texture [%s] loaded\n", name);
	}
	else
	{*/
		//mft_frame_count=1;
		pixels=SOIL_load_image(full_path, &w, &h, &channels, 0);
		if(!pixels)
		{
			console_Print(MESSAGE_ERROR, "couldnt load texture [%s]\n", name);
			return -1;
		}
		console_Print(MESSAGE_NORMAL, "texture [%s] loaded\n", name);
	/*}*/
	
	
	
	
	/*if(gamma_correct)
	{
		for(i = 0; i < h; i++)
		{
			for(j = 0; j < w; j++)
			{
				printf("[");
				for(k = 0; k < channels; k++)
				{
					f = (float)pixels[(i * w * channels) + (j * channels) + k];
					f = pow(f, 2.2);
					pixels[(i * w * channels) + (j * channels) + k] = f;
					printf("%d ", pixels[(i * w * channels) + (j * channels) + k]);
				}
				printf("] ");
			}
			printf("\n");
		}
	}*/
	

	glGenTextures(1, &tex_ID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_ID);
	
	//printf("texture_LoadTexture: %d\n", tex_ID);
	
	
	switch(channels)
	{
		case 1:
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			format = GL_LUMINANCE;
			internal_format = GL_LUMINANCE8;
		break;
		
		case 2:
			glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
			format = GL_RG;
			internal_format = GL_RG8;
		break;
		
		case 3:
			glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
			format=GL_RGB;
			if(gamma_correct)
			{
				internal_format = GL_SRGB;
			}
			else
			{
				internal_format=GL_RGB8;
			}
			
		break;
		
		default:
		case 4:
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			format=GL_RGBA;
			if(gamma_correct)
			{
				internal_format = GL_SRGB_ALPHA;
			}
			else
			{
				internal_format=GL_RGBA8;
			}
			
		break;
	}
	
	while(glGetError()!= GL_NO_ERROR);
	//glTexImage3D(GL_TEXTURE_2D,0, internal_format, w, h, mft_frame_count, 0, format, GL_UNSIGNED_BYTE, pixels);
	//glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	//printf("%x\n", glGetError());
	
	if(texture_a.texture_count>=texture_a.array_size)
	{
		texture_ResizeTextureArray(texture_a.array_size+10);
	}
	
	texture_a.textures[texture_a.texture_count].frame_count=mft_frame_count;
	texture_a.textures[texture_a.texture_count].height=h;
	texture_a.textures[texture_a.texture_count].width=w;
	texture_a.textures[texture_a.texture_count].tex_ID=tex_ID;
	texture_a.textures[texture_a.texture_count++].name=name;
	
	free(pixels);
	return texture_a.texture_count-1;

}



PEWAPI texture_t *texture_GetTextureByIndex(int texture_index)
{
	return &texture_a.textures[texture_index];
}


/*
=============
texture_SetTextureByIndex
=============
*/
PEWAPI void texture_SetTextureByIndex(int texture_index, int tex_unit, int texture_layer)
{
	
	//printf("set texture %d\n", texture_index);

	int t;
	
	//while(glGetError()!=GL_NO_ERROR);

	int uniform;
	int u_value;

	switch(tex_unit)
	{
		case GL_TEXTURE0:
			uniform = UNIFORM_TextureSampler0;
			u_value = 0;
		break;
		
		case GL_TEXTURE1:
			uniform = UNIFORM_TextureSampler1;
			u_value = 1;
		break;
		
		case GL_TEXTURE2:
			uniform = UNIFORM_TextureSampler2;
			u_value = 2;
		break;
		
		case GL_TEXTURE3:
			uniform = UNIFORM_TextureSampler3;
			u_value = 3;
		break;
		
		case GL_TEXTURE4:
			uniform = UNIFORM_TextureSampler4;
			u_value = 4;
		break;
		
		default:
			return;
	}
	
	glActiveTexture(tex_unit);
	glBindTexture(GL_TEXTURE_2D, texture_a.textures[texture_index].tex_ID);
	shader_SetCurrentShaderUniform1i(uniform, u_value);
	
	return;
}


/*
=============
texture_GetTextureIndex
=============
*/
PEWAPI int texture_GetTextureIndex(char *texture)
{
	int i;
	int c;
	c=texture_a.texture_count;
	for(i=0; i<c; i++)
	{
		if(!strcmp(texture, texture_a.textures[i].name))
		{
			return i;
		}
	}
	return -1;
}



/*
=============
texture_GetTextureID
=============
*/
PEWAPI unsigned int texture_GetTextureID(char *texture)
{
	register int i;
	register int c;
	c=texture_a.texture_count;
	for(i=0; i<c; i++)
	{
		if(!strcmp(texture, texture_a.textures[i].name))
		{
			return texture_a.textures[i].tex_ID;
		}
	}
	return 0;
}

#ifdef __cplusplus
}
#endif










