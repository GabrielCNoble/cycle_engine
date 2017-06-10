#include "framebuffer.h"
#include "console.h"

framebuffer_t *cur_fb;

/* TODO: make this function more flexible. This implementation is ridiculous.  */
framebuffer_t framebuffer_CreateFramebuffer(int width, int height, int depth_buffer_type, int color_output_count, ...)
{
	framebuffer_t fb;
	fb.width=width;
	fb.height=height;
	unsigned short *param =(unsigned short *) ((&color_output_count)+1);
	int type;
	int format;
	int internal_format;
	int i;
	unsigned int new_tex;
	glGenFramebuffers(1, &fb.id);
	glGenTextures(1, &fb.z_buffer);
	unsigned int stencil_texture;
	//glGenTextures(1, &fb.color_out1);
	 
	 
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.id);
	glBindTexture(GL_TEXTURE_2D, fb.z_buffer);
	//printf("framebuffer_CreateFramebuffer: %d\n", fb.z_buffer);
	
	switch(depth_buffer_type)
	{
		case GL_DEPTH_STENCIL:
			internal_format = GL_DEPTH_STENCIL;
			format = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_INT_24_8;
		break;
		
		case GL_DEPTH_COMPONENT:
			internal_format = GL_DEPTH_COMPONENT;
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		break;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, fb.width, fb.height, 0, format, type, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.z_buffer, 0);
	if(depth_buffer_type == GL_DEPTH_STENCIL)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fb.z_buffer, 0);
	}
	
	if(i > 3) i = 3;
	else if(i < 0) i = 0;
	
	for(i = 0; i < color_output_count; i++)
	{
		switch(*param)
		{
			
			case GL_LUMINANCE8:
				internal_format = GL_LUMINANCE8;
				format = GL_LUMINANCE;
				type = GL_UNSIGNED_BYTE;
			break;
			
			case GL_LUMINANCE16F_ARB:
				internal_format = GL_LUMINANCE16F_ARB;
				format = GL_LUMINANCE;
				type = GL_FLOAT;
			break;
			
			case GL_RED:
				internal_format = GL_RED;
				
				type = GL_UNSIGNED_BYTE;
			break;
			
			case GL_RG:
				internal_format = GL_RG8;
				type = GL_UNSIGNED_BYTE;
			break;
			
			case GL_RGB:
				internal_format = GL_RGB8;
				format = GL_RGB;
				type = GL_UNSIGNED_BYTE;
			break;
			
			case GL_RGB16F:
				internal_format = GL_RGB16F;
				format = GL_RGB;
				type = GL_UNSIGNED_BYTE;
			break;
			
			case GL_RGBA:
				internal_format = GL_RGBA8;
				format = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
			break;
			
			case GL_RGBA16F:
				internal_format = GL_RGBA16F;
				format = GL_RGBA;
				type = GL_FLOAT;
			break;
			
			case GL_RGBA32F:
				internal_format = GL_RGBA32F;
				format = GL_RGBA;
				type = GL_FLOAT;
			break;
			
			case GL_SRGB:
				internal_format = GL_SRGB;
				format = GL_RGB;
				type = GL_FLOAT;
			break;
			
			case GL_SRGB_ALPHA:
				internal_format = GL_SRGB_ALPHA;
				format = GL_RGBA;
				type = GL_FLOAT;
			break;
			
		}
		
		glGenTextures(1, &new_tex);
		glBindTexture(GL_TEXTURE_2D, new_tex);
		
		//printf("framebuffer_CreateFramebuffer: %d\n", new_tex);
		
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, fb.width, fb.height, 0, format, type, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, new_tex, 0);
		
		switch(i)
		{
			case 0:
				fb.color_out1 = new_tex;
			break;
			
			case 1:
				fb.color_out2 = new_tex;
			break;
			
			case 2:
				fb.color_out3 = new_tex;
			break;
			
			case 3:
				fb.color_out4 = new_tex;
			break;
		}
		param++;
	}
	fb.color_output_count = color_output_count;	
	
	//printf("fb status: %x\n", glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK_LEFT);
	glBindTexture(GL_TEXTURE_2D, 0);

	return fb;
}

void framebuffer_DeleteFramebuffer(framebuffer_t *fb)
{
	if(fb)
	{
		switch(fb->color_output_count)
		{
			case 4:
				glDeleteTextures(1, &fb->color_out4);
			case 3:
				glDeleteTextures(1, &fb->color_out3);
			case 2:
				glDeleteTextures(1, &fb->color_out2);
			case 1:
				glDeleteTextures(1, &fb->color_out1);
			break;			
		}
		
		glDeleteTextures(1, &fb->z_buffer);
		glDeleteFramebuffers(1, &fb->id);
	}
}


void framebuffer_SetFramebufferOutput(framebuffer_t *framebuffer, unsigned int color_output, unsigned int output_ID)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer->id);
	switch(color_output)
	{
		case GL_COLOR_ATTACHMENT0:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_ID, 0);
			framebuffer->color_out1=output_ID;
		break;
		
		case GL_COLOR_ATTACHMENT1:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, output_ID, 0);
			framebuffer->color_out2=output_ID;
		break;
		
		case GL_COLOR_ATTACHMENT2:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, output_ID, 0);
			framebuffer->color_out3=output_ID;
		break;
	}
	
	return;
}


void framebuffer_BindFramebuffer(framebuffer_t *framebuffer)
{
	int color_attachments[]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	if(framebuffer)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer->id);
		glViewport(0, 0, framebuffer->width, framebuffer->height);
		switch(framebuffer->color_output_count)
		{
			case 1:
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
			break;
			
			case 2:
				glDrawBuffers(2, (unsigned int *)color_attachments);
			break;
			
			case 3:
				glDrawBuffers(3, (unsigned int *)color_attachments);
			break;
			
			case 4:
				glDrawBuffers(4, (unsigned int *)color_attachments);
			break;
		}
		
		cur_fb = framebuffer;
		
		return;
	}
	else
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDrawBuffer(GL_LEFT);
		
		cur_fb = NULL;
	}
	return;
}


void framebuffer_ResizeFramebuffer(framebuffer_t *framebuffer, int width, int height)
{
	short internal_format;
	short type;
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer->id);
	glBindTexture(GL_TEXTURE_2D, framebuffer->z_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8 , width, height, 0, GL_DEPTH_COMPONENT24, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer->z_buffer, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebuffer->z_buffer, 0);
	
	framebuffer->width=width;
	framebuffer->height=height;
	
	switch(framebuffer->color_output_count)
	{
		case 3:
			if(framebuffer->bm_color_output_flags&COLOR2_FLOAT)
			{
				internal_format=GL_RGBA32F;
				type=GL_FLOAT;
			}
			else
			{
				internal_format=GL_RGBA8;
				type=GL_UNSIGNED_BYTE;
			}
			glGenTextures(1, &framebuffer->color_out3);
			glBindTexture(GL_TEXTURE_2D, framebuffer->color_out3);
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, width, 0, GL_RGBA, type, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, framebuffer->color_out3, 0);
		case 2:
			if(framebuffer->bm_color_output_flags&COLOR1_FLOAT)
			{
				internal_format=GL_RGBA32F;
				type=GL_FLOAT;
			}
			else
			{
				internal_format=GL_RGBA8;
				type=GL_UNSIGNED_BYTE;
			}
			glGenTextures(1, &framebuffer->color_out2);
			glBindTexture(GL_TEXTURE_2D, framebuffer->color_out2);
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, width, 0, GL_RGBA, type, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer->color_out2, 0);
		case 1:
			if(framebuffer->bm_color_output_flags&COLOR0_FLOAT)
			{
				internal_format=GL_RGBA32F;
				type=GL_FLOAT;
			}
			else
			{
				internal_format=GL_RGBA16;
				type=GL_UNSIGNED_SHORT;
			}
			glGenTextures(1, &framebuffer->color_out1);
			glBindTexture(GL_TEXTURE_2D, framebuffer->color_out1);
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, width, 0, GL_RGBA, type, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->color_out1, 0);
		break;
	}
}

/*
==============
framebuffer_CopyFramebuffer

pretty unsafe function. Don't allow swizzle of the channels (yet),
and don't check for matching color outputs of the buffers. Supposed to
be used internally only.
==============
*/
void framebuffer_CopyFramebuffer(framebuffer_t *dst, framebuffer_t *src, int bm_copy_flags)
{
	int c[3];
	int q = 0;
	int filter;
	int mask = 0;
	
	int prev_dbuffer;
	int prev_rbuffer;
	
	glGetIntegerv(GL_DRAW_FRAMEBUFFER, &prev_dbuffer);
	glGetIntegerv(GL_READ_FRAMEBUFFER, &prev_rbuffer);
	
	
	
	if(bm_copy_flags & COPY_COLOR0)
	{
		c[q++] = GL_COLOR_ATTACHMENT0;
	}
	if(bm_copy_flags & COPY_COLOR1)
	{
		c[q++] = GL_COLOR_ATTACHMENT1;
	}
	if(bm_copy_flags & COPY_COLOR2)
	{
		c[q++] = GL_COLOR_ATTACHMENT2;
	}
	
	if(q)
	{
		mask |= GL_COLOR_BUFFER_BIT;
	}
	else
	{
		c[q++] = GL_NONE;
	}
	if(bm_copy_flags & COPY_DEPTH)
	{
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	
	if(bm_copy_flags & COPY_FILTER_LINEAR)
	{
		filter = GL_LINEAR;
	}
	else
	{
		filter = GL_NEAREST;
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->id);
	glDrawBuffers(q, (unsigned int *)c);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src->id);
	
	glBlitFramebuffer(0, 0, src->width, src->height, 0, 0, dst->width, dst->height, mask, filter);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_dbuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, prev_rbuffer);
	
}
























