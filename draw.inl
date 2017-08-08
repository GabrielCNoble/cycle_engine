#ifndef DRAW_INL
#define DRAW_INL

#include "draw_types.h"
#include "macros.h"
#include "conf.h"

extern void (*draw_DrawFrameFunc)();

extern render_queue render_q;
extern render_queue t_render_q;
extern render_queue shadow_q;
extern brush_render_queue_t brush_render_queue;

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

inline void draw_ResizeBrushRenderQueue(int new_size)
{
	int *a = (int *)malloc(sizeof(int) * new_size);
	int *b = (int *)malloc(sizeof(int) * new_size);
	
	memcpy(a, brush_render_queue.count, sizeof(int) * brush_render_queue.max_command_buffer);
	memcpy(b, brush_render_queue.start, sizeof(int) * brush_render_queue.max_command_buffer);
	
	free(brush_render_queue.count);
	free(brush_render_queue.start);
	
	brush_render_queue.count = a;
	brush_render_queue.start = b;
	
	brush_render_queue.max_command_buffer = new_size;
}

inline int draw_DispatchBrushCommandBuffer(int start, int count, short material_index)
{
	int index = brush_render_queue.command_buffer_count;
	
	if(index >= brush_render_queue.max_command_buffer)
	{
		draw_ResizeBrushRenderQueue(index + 16);
	}
	
	brush_render_queue.count[index] = count;
	brush_render_queue.start[index] = start;
	
	brush_render_queue.command_buffer_count++;
	
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
