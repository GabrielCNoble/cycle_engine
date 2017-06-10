#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "conf.h"
#include "includes.h"

typedef struct
{
	unsigned int color_out1;
	unsigned int color_out2;
	unsigned int color_out3;
	unsigned int color_out4;
	unsigned int z_buffer;
	unsigned int id;
	short color_output_count;
	short bm_color_output_flags;
	short width;
	short height;
}framebuffer_t;

enum OUTPUT_TYPES
{
	COLOR0_FLOAT=1,
	COLOR1_FLOAT=2,
	COLOR2_FLOAT=4
};

enum COPY_FLAGS
{
	COPY_COLOR0 = 1,
	COPY_COLOR1 = 1<<1,
	COPY_COLOR2 = 1<<2,
	COPY_DEPTH = 1<<3,
	COPY_FILTER_LINEAR = 1<<4				/* if this flag is not present, copy will use GL_NEAREST */
};


//typedef struct
//{
//	int width;
//	int height;
//	unsigned int g_buffer;		/* geometry buffer (diffuse + texture) */
//	unsigned int n_buffer;		/* normals */
//	unsigned int s_buffer;		/* specular textures */
//	unsigned int z_buffer;		/* depth */
//	unsigned int id;
//}deferred_framebuffer_t;


framebuffer_t framebuffer_CreateFramebuffer(int width, int height, int depth_buffer_type, int color_output_count, ...);

void framebuffer_DeleteFramebuffer(framebuffer_t *fb);

void framebuffer_SetFramebufferOutput(framebuffer_t *framebuffer, unsigned int color_output, unsigned int output_ID);

void framebuffer_BindFramebuffer(framebuffer_t *framebuffer);

void framebuffer_ResizeFramebuffer(framebuffer_t *framebuffer, int width, int height);

void framebuffer_CopyFramebuffer(framebuffer_t *dst, framebuffer_t *src, int bm_copy_flags);


//void framebuffer_t *framebuffer_GetCurrentFramebuffer();

/*deffered_framebuffer_t framebuffer_CreateDeferredFramebuffer(int width, int height);

void framebuffer_BindDeferredFramebuffer(deferred_framebuffer_t *framebuffer);*/


#endif /* FRAMEBUFFER_H */
