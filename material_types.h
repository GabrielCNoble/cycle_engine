#ifndef MATERIAL_TYPES_H
#define MATERIAL_TYPES_H

#include "matrix_types.h"
#include "vector_types.h"
#include "frustum_types.h"

#define MAX_MATERIAL_EMISSIVE 100.0

enum MATERIAL_FLAGS
{
	MATERIAL_Shadeless = 1,
	MATERIAL_Wireframe = 1<<1,
	MATERIAL_DiffuseTexture = 1<<2,
	MATERIAL_NormalTexture = 1<<3,
	MATERIAL_GlossTexture = 1<<4,
	MATERIAL_MetallicTexture = 1<<5,
	MATERIAL_HeightTexture = 1<<6,
	MATERIAL_EnvironmentTexture = 1<<7,
	MATERIAL_FrontAndBack = 1<<8,
	MATERIAL_Translucent = 1<<9,
	MATERIAL_Emissive = 1<<10,	
	MATERIAL_Cached = 1 << 11
};

typedef struct color4_t
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
}color4_t;

typedef struct 
{
	short diff_tex;
	short norm_tex;
	short heig_tex;
	short spec_tex;
	short gloss_tex;
	short met_tex;
}tex_info_t;


typedef struct
{
	unsigned int cache_index;						/* index into the material gpu cache */
	color4_t diff_color;
	short shader_index;
	short emissive;									/* this could be unsigned char */
	short glossiness;								/* this could be unsigned char */
	short metallic;									/* this could be unsigned char */
	short bm_flags;
	short diff_tex;		
	short norm_tex;
	short heig_tex;
	short gloss_tex;
	short met_tex;
	char *name;	
			
}material_t;	/* 32 bytes long... */
/* alignment in this machine seems to be 256 bytes... 224 are thrown away for each GPU material... is this really better? */

typedef struct
{
	int array_size;
	int material_count;
	material_t *materials;
}material_array;



#endif /* MATERIAL_TYPES_H */
