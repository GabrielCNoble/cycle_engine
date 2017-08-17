#include "gpu.h"
#include "macros.h"
#include "vector.h"
//#define GPU_HEAP_SIZE 134217728		/* 128 MB heap should be enough... */
#define GPU_HEAP_SIZE 33554432			/* 32 MB */
#define GPU_MIN_ALLOC sizeof(float)			
#define FREE_THRESHOLD 15

//int vcache_size;
//int next_vcache_id;

unsigned int gpu_heap;
int frees;



/*typedef struct
{
	int start;
	int size;
}gpu_head_t;

typedef struct
{
	int size;
	int cursor;
	gpu_head_t *list;
	int free_stack_top;
	int free_stack_size;
	int *free_stack;
}gpu_heap_list_t;*/

gpu_heap_list_t alloc_list;
gpu_heap_list_t free_list;


unsigned int mapped_array_buffer;
int mapped_array_access;
unsigned int mapped_index_buffer;
int mapped_index_access;

unsigned int bound_array_buffer;
unsigned int bound_index_buffer;
/*...*/

/* internal use only... */
void gpu_Defrag();
void gpu_Sort(int left, int right);


void gpu_Init()
{
	//while(glGetError() != GL_NO_ERROR);
	glGenBuffers(1, &gpu_heap);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	glBufferData(GL_ARRAY_BUFFER, GPU_HEAP_SIZE, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//printf("gpu_Init: %x\n\n", glGetError());
	
	mapped_array_buffer = 0;
	mapped_array_access = 0;
	mapped_index_buffer = 0;
	mapped_index_access = 0;
	bound_array_buffer = 0;
	bound_index_buffer = 0;
	
	frees = 0;
	
	alloc_list.cursor = 0;
	alloc_list.size = 32;
	alloc_list.free_stack_top = -1;
	alloc_list.free_stack_size = 32;
	alloc_list.free_stack = (int *)malloc(sizeof(int) * 32);
	alloc_list.list = (gpu_head_t *)malloc(sizeof(gpu_head_t) * 32);
	
	
	free_list.cursor = 1;
	free_list.size = 32;
	free_list.list = (gpu_head_t *)malloc(sizeof(gpu_head_t) * 32);
	
	
	/* the whole heap is free... */
	free_list.list[0].start = 0;
	free_list.list[0].size = GPU_HEAP_SIZE;
	
}

void gpu_Finish()
{
	glDeleteBuffers(1, &gpu_heap);
	free(alloc_list.list);
	free(alloc_list.free_stack);
	free(free_list.list);
	//free(free_list.free_stack);
}

PEWAPI gpu_buffer_t gpu_CreateGPUBuffer(int size, int type, int hint)
{
	gpu_buffer_t buf;
	if(size>0)
	{
		glGenBuffers(1, &buf.buffer_ID);
		glBindBuffer(type, buf.buffer_ID);
		glBufferData(type, size, NULL, hint);
		
		
		buf.size=size;
		buf.hint=hint;
		buf.type=type;
		
		glBindBuffer(type, 0);
		return buf;
	}
	
	buf.buffer_ID=0;
	return buf;
}


PEWAPI void gpu_BindGPUBuffer(gpu_buffer_t *buffer)
{
	glBindBuffer(buffer->type ,buffer->buffer_ID);
	return;
}



PEWAPI void gpu_FillGPUBuffer(gpu_buffer_t *buffer, int size, int count, void *data)
{
	glBindBuffer(buffer->type, buffer->buffer_ID);
	glBufferData(buffer->type, size*count, data, buffer->hint);
	glBindBuffer(buffer->type, 0);
	return;
}


PEWAPI void *gpu_MapGPUBuffer(gpu_buffer_t *buffer, int access)
{
	void *rtrn;
	glBindBuffer(buffer->type, buffer->buffer_ID);
	rtrn=glMapBuffer(buffer->type, access);
	return rtrn;
}


PEWAPI void gpu_UnmapGPUBuffer(gpu_buffer_t *buffer)
{
	glUnmapBuffer(buffer->type);
	return;
}


PEWAPI void gpu_DeleteGPUBuffer(gpu_buffer_t *buffer)
{
	glDeleteBuffers(1, &buffer->buffer_ID);
	return;
}

PEWAPI int gpu_Alloc(int size)
{
	int h = -1;
	register int i;
	int c = free_list.cursor;
	gpu_head_t *t;
	int *q;
	int f;
	int attempt_defrag = 0;
	
	if(alloc_list.cursor >= alloc_list.size)
	{
		t = (gpu_head_t *)malloc(sizeof(gpu_head_t) * (alloc_list.size + 32));
		q = (int *)malloc(sizeof(int) * (alloc_list.size + 32));
		memcpy(t, alloc_list.list, sizeof(gpu_head_t) * alloc_list.size);
		free(alloc_list.list);
		free(alloc_list.free_stack);
		alloc_list.list = t;
		alloc_list.free_stack = q;
		alloc_list.size += 32;
	}
	
	if(likely(size > 0))
	{
		/* round the size up to the closest multiple of the minimum allowed allocation... */
		
		size = (size + GPU_MIN_ALLOC - 1) & (~(GPU_MIN_ALLOC - 1));
		
		//printf("size is %d\n", size);
		_try_again:
		for(i = 0; i < c; i++)
		{
			if(free_list.list[i].size >= size)
			{
				//h = i;
				
				if(alloc_list.free_stack_top > -1)
				{
					h = alloc_list.free_stack[alloc_list.free_stack_top--];
				}
				else
				{
					h = alloc_list.cursor++;
				}
				//h = f;
				
				alloc_list.list[h].size = size;
				alloc_list.list[h].start = free_list.list[i].start;
				
				if(free_list.list[i].size > size)
				{
					/* alloc is smaller than this free block, so just chopp
					a chunk off and set it as alloc'd... */
					
					free_list.list[i].start += size;
					free_list.list[i].size -= size;
				}
				else
				{
					/* free block fits perfectly the request, so just copy its info
					to the alloc_list and pull the last element of the free_list to
					the now vacant position...*/
					
					free_list.list[i].size = -1;
					if(i < free_list.cursor - 1)
					{
						free_list.list[i] = free_list.list[free_list.cursor - 1];
						free_list.cursor--;
					}
				}
				
				break;
			}
		}
		/* the function didn't find a free block that could
		service the request, so try to defrag the heap and
		repeat the search... */
		if(!attempt_defrag && h == -1)
		{
			attempt_defrag = 1;
			gpu_Defrag();
			goto _try_again;
		}
	}
	return h;
}

PEWAPI int gpu_Realloc(int handle, int size)
{
	return handle;
}

PEWAPI void gpu_Free(int handle)
{
	gpu_head_t *t;
	
	if(free_list.cursor >= free_list.size)
	{
		t = (gpu_head_t *)malloc(sizeof(gpu_head_t) * (free_list.size + 32));
		memcpy(t, free_list.list, sizeof(gpu_head_t) * free_list.size);
		free(free_list.list);
		free_list.list = t;
		free_list.size += 32;
	}

	free_list.list[free_list.cursor++] = alloc_list.list[handle];
	alloc_list.free_stack[++alloc_list.free_stack_top] = handle;
	frees++;
	//printf("%d free block heads\n", free_list.cursor);
	if(frees > FREE_THRESHOLD)
	{
		gpu_Defrag();
	}
	
	//printf("%d free block heads\n", free_list.cursor);
	//printf("%d %d\n", free_list.list[0].size, free_list.list[0].start);
}

void gpu_Defrag()
{
	register int i;
	int c = free_list.cursor;
	int k = 0;
	int j;
	int new_count = c;
	gpu_Sort(0, free_list.cursor - 1);
	
	for(i = 1; i < c; i++)
	{
		
		if(free_list.list[k].start + free_list.list[k].size == free_list.list[i].start)
		{
			free_list.list[k].size += free_list.list[i].size;
			free_list.list[i].size = -1;
			new_count--;
		}
		else
		{
			k = i;
		}
	}
	
	for(i = 0; i < c; i++)
	{
		if(free_list.list[i].size < 0)
		{
			for(j = i; j < c; j++)
			{
				if(free_list.list[j].size > -1)
				{
					free_list.list[i] = free_list.list[j];
					free_list.list[j].size = -1;
					i = j - 1;
					break;
				}
			}
		}
	}
	
	free_list.cursor = new_count;
	
	//printf("%d free blocks\n", new_count);
	//printf("%d %d\n", free_list.list[0].size, free_list.list[0].start);
	
	frees = 0;
}

void gpu_Sort(int left, int right)
{
	int i = left;
	int j = right;
	int m = (right + left) / 2;
	gpu_head_t t;
	gpu_head_t q = free_list.list[m];
	while(i <= j)
	{
		for(; free_list.list[i].start < q.start && i < right; i++);
		for(; free_list.list[j].start > q.start && j > left; j--);
		
		if(i <= j)
		{
			t = free_list.list[i];
			free_list.list[i] = free_list.list[j];
			free_list.list[j] = t;
			i++;
			j--;
		} 
	}
	if(j > left) gpu_Sort(left, j);
	if(i < right) gpu_Sort(i, right);
	
	return;
}

PEWAPI void gpu_Read(int handle, int offset, void *buffer, int count, int raw)
{
	register int i;
	int c = count;
	//int start = alloc_list.list[handle].start;
	int start;
	void *p;
	
	if(raw)
	{
		start = handle;
	}
	else
	{
		start = alloc_list.list[handle].start;
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	p = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	p = (char *)p + start + offset; 
	if(!(count % 4))
	{
		c >>= 2;
		
		for(i = 0; i < c; i++)
		{
			*((int *)buffer + i) = *((int *)p + i);
		}
	}
	else if(!(count % 2))
	{
		c >>= 1;
		
		for(i = 0; i < c; i++)
		{
			*((short *)buffer + i) = *((short *)p + i);
		}
	}
	else
	{
		for(i = 0; i < c; i++)
		{
			*((char *)buffer + i) = *((char *)p + i);
		}
	}
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	if(bound_array_buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, bound_array_buffer);
		if(mapped_array_buffer)
		{
			glMapBuffer(GL_ARRAY_BUFFER, mapped_array_access);
		}
	}
}

PEWAPI void gpu_Write(int handle, int offset, void *buffer, int count, int raw)
{
	int start;
	void *p;
	
	if(raw)
	{
		start = handle;
	}
	else
	{
		start = alloc_list.list[handle].start;
	}	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	p = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	p = ((char *)p) + start + offset; 	
	
	asm
	(
		"movl %0, %%edi\n"
		"movl %1, %%esi\n"
		"movl %2, %%ecx\n"
		
		".intel_syntax noprefix\n"
		"test ecx, 3\n"
		"jz _even\n"
		"mov eax, ecx\n"
		"and ecx, 3\n"
		"rep movsb\n"
		"lea ecx, [eax - 3]\n"
		"_even:\n"
		"shr ecx, 2\n"
		"rep movsd\n"
		
		".att_syntax prefix\n"
		:: "rm" (p), "rm" (buffer), "rm" (count)
	);
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	if(bound_array_buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, bound_array_buffer);
		if(mapped_array_buffer)
		{
			glMapBuffer(GL_ARRAY_BUFFER, mapped_array_access);
		}
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}






