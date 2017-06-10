#ifndef TEXTURE_H
#define TEXTURE_H

#include "conf.h"

#include "includes.h"

typedef struct texture
{
	int width;
	int height;
	int frame_count;
	unsigned int tex_ID;
	char *name;
}texture_t;

typedef struct texture_array
{
	int array_size;
	int texture_count;
	texture_t *textures;
}texture_array;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void texture_Init(char *path);

PEWAPI void texture_Finish();

PEWAPI void texture_PurgeAllTextures();

PEWAPI void texture_ResizeTextureArray(int new_size);

PEWAPI int texture_LoadTexture(char *filename, char *name, int gamma_correct);

PEWAPI texture_t *texture_GetTextureByIndex(int texture_index);

PEWAPI void texture_SetTextureByIndex(int texture_index, int tex_unit, int texture_layer);

PEWAPI int texture_GetTextureIndex(char *texture);

PEWAPI unsigned int texture_GetTextureID(char *texture);

#ifdef __cplusplus
}
#endif




#endif /* TEXTURE_H */
