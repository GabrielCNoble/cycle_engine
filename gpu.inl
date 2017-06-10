#ifndef GPU_INL
#define GPU_INL

#include "conf.h"
#include "includes.h"
#include "gpu.h"


extern unsigned int mapped_array_buffer;
extern int mapped_array_access;
extern unsigned int mapped_index_buffer;
extern int mapped_index_access;

extern unsigned int bound_array_buffer;
extern unsigned int bound_index_buffer;

extern gpu_heap_list_t alloc_list;
extern gpu_heap_list_t free_list;

PEWAPI void gpu_BindBuffer(int target, unsigned int id)
{
	switch(target)
	{
		case GL_ARRAY_BUFFER:
			bound_array_buffer = id;
		break;
			
		case GL_ELEMENT_ARRAY_BUFFER:
			bound_index_buffer = id;
		break;
	}
	
	glBindBuffer(target, id);
}

PEWAPI void *gpu_MapBuffer(int target, int access)
{
	void *r = glMapBuffer(target, access);
	if(r)
	{
		switch(target)
		{
			case GL_ARRAY_BUFFER:
				mapped_array_buffer = bound_array_buffer;
				mapped_array_access = access;
			break;
			
			case GL_ELEMENT_ARRAY_BUFFER:
				mapped_index_buffer = bound_index_buffer;
				mapped_index_access = access;
			break;
		}
	}
	return r;
}

PEWAPI void gpu_UnmapBuffer(int target)
{
	switch(target)
	{
		case GL_ARRAY_BUFFER:
			mapped_array_buffer = 0;
			mapped_array_access = 0;
		break;
			
		case GL_ELEMENT_ARRAY_BUFFER:
			mapped_index_buffer = 0;
			mapped_index_access = 0;
		break;
	}
	
	glUnmapBuffer(target);
}

PEWAPI int gpu_GetAllocStart(int handle)
{
	if(handle >= 0)
	{
		return alloc_list.list[handle].start;
	}
}



#endif 
