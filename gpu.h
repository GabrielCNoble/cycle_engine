#ifndef GPU_H
#define GPU_H

#include "conf.h"
#include "includes.h"


typedef struct
{
	int size;
	unsigned int hint;
	unsigned int type;
	unsigned int buffer_ID;
}gpu_buffer_t;

typedef struct
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
}gpu_heap_list_t;


#ifdef __cplusplus
extern "C"
{
#endif

void gpu_Init();

void gpu_Finish();

PEWAPI gpu_buffer_t gpu_CreateGPUBuffer(int size, int type, int hint);

PEWAPI void gpu_BindGPUBuffer(gpu_buffer_t *buffer);

PEWAPI void gpu_FillGPUBuffer(gpu_buffer_t *buffer, int size, int count, void *data);

PEWAPI void *gpu_MapGPUBuffer(gpu_buffer_t *buffer, int access);

PEWAPI void gpu_UnmapGPUBuffer(gpu_buffer_t *buffer);

PEWAPI void gpu_DeleteGPUBuffer(gpu_buffer_t *buffer);

PEWAPI int gpu_Alloc(int size);

PEWAPI int gpu_Realloc(int handle, int size);

PEWAPI void gpu_Free(int handle);

PEWAPI inline int gpu_GetAllocStart(int handle);

PEWAPI inline int gpu_GetAllocSize(int handle);



/* those are intended for high density data transfers, so if you want to transfer a single byte with it, 
you may have a bad day creating a temp buffer just for the sake of it... */
PEWAPI void gpu_Read(int handle, int offset, void *buffer, int count, int raw);

PEWAPI void gpu_Write(int handle, int offset, void *buffer, int count, int raw);



/* those should ALWAYS be used, for the gpu heap management
depens upon info updated by those functions */
PEWAPI inline void gpu_BindBuffer(int target, unsigned int id);

PEWAPI inline void *gpu_MapBuffer(int target, int access);

PEWAPI inline void gpu_UnmapBuffer(int target);





#include "gpu.inl"

#ifdef __cplusplus
}
#endif



#endif /* GPU_H */




