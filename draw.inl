#ifndef DRAW_INL
#define DRAW_INL

#include "draw_types.h"
#include "macros.h"
#include "conf.h"

extern void (*draw_DrawFrameFunc)();

extern render_queue render_q;
extern render_queue t_render_q;
extern render_queue shadow_q;

extern int draw_calls;
extern int texture_binds;
extern int shader_swaps;

#ifdef __cplusplus
extern "C"
{
#endif



PEWAPI void draw_ResetRenderQueue()
{
	render_q.count=0;
	t_render_q.count = 0;
	return;
}


PEWAPI void draw_ResetShadowQueue()
{
	shadow_q.count=0;
	return;
}


PEWAPI void draw_ResizeRenderQueue(render_queue *r_queue, int new_size)
{
	
	command_buffer_t *base=(command_buffer_t *)calloc(new_size+2, sizeof(command_buffer_t));
	command_buffer_t *temp;
	*(int *)&temp=(((int)base)+63)& ~63;
	int *itemp = (int *)malloc(sizeof(int) * new_size);
	
	if(likely(r_queue->base))
	{
		memcpy(temp, r_queue->command_buffers, sizeof(command_buffer_t)*r_queue->count);
		free(r_queue->base);
		free(r_queue->entity_indexes);
	}
	r_queue->entity_indexes = itemp;
	r_queue->base = base;
	r_queue->command_buffers = temp;
	r_queue->queue_size = new_size;
	
	return;
}

void draw_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
	glDrawArrays(mode, first, count);
	draw_calls++;
}

void draw_BindTexture(GLenum target, GLuint texture)
{
	glBindTexture(target, texture);
	texture_binds++;
}

void draw_UseProgram(GLuint program)
{
	glUseProgram(program);
	shader_swaps++;
}

/*PEWAPI void draw_DrawFrame()
{
	draw_DrawFrameFunc();
}*/

#ifdef __cplusplus
}
#endif





#endif /* DRAW_INL */
