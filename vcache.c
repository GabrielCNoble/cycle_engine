#include "vcache.h"
#include "model.h"
#include "macros.h"
#include "gpu.h"


int vcache_count;
int vcache_size;
vcache_slot_t *vert_cache;

int stack_top;
int *stack;


void vcache_Init()
{
	
	vcache_count = 0;
	vcache_size = 0;
	vert_cache = NULL;
	stack_top = -1;
	vcache_ExpandCache(64);
	
	
	return;
}

void vcache_Finish()
{
	free(vert_cache);
	free(stack);
	//glDeleteBuffers(1, &vcache_gpu_buffer);
}

void vcache_CacheMeshData(mesh_t *mesh)
{
	int byte_count;
	int vertex_byte_count = 0;
	int normal_byte_count = 0;
	int tangent_byte_count = 0;
	int tex_coord_byte_count = 0;
	int slot_index;
	int i;
	float *f;
	
	if(likely(mesh))
	{
		/* don't cache already cached stuff... */
		if(mesh->vcache_slot_id >= 0) return;
		
		byte_count = sizeof(float) * 3 * mesh->vert_count;
	
		if(mesh->n_data)
		{
			byte_count += sizeof(float) * 3 * mesh->vert_count;
		}
		if(mesh->t_data)
		{
			byte_count += sizeof(float) * 3 * mesh->vert_count;
		}
		if(mesh->t_c_data)
		{
			byte_count += sizeof(float) * 2 * mesh->vert_count;
		}
		
		
		//f = (float *)malloc(byte_count);
		
		if(stack_top < 0)
		{
			slot_index = vcache_count++;
		}
		else
		{
			slot_index = stack[stack_top--];
		}
		
		vert_cache[slot_index].handle = gpu_Alloc(byte_count);
		vert_cache[slot_index].start = gpu_GetAllocStart(vert_cache[slot_index].handle);
		vert_cache[slot_index].mesh = mesh;
		
		mesh->vcache_slot_id = slot_index;
		mesh->start = vert_cache[slot_index].start;
		gpu_Write(vert_cache[slot_index].handle, 0, mesh->v_data, byte_count, 0);
		mesh->flags |= MESH_CACHED;
		printf("mesh_t %s has been cached\n", mesh->name);
		
		/*gpu_Read(vert_cache[slot_index].handle, 0, f, byte_count, 0);
		
		
		for(i = 0; i < mesh->vert_count; i++)
		{
			printf("v: [%f %f %f]\n", mesh->v_data[i*6], mesh->v_data[i*6 + 1], mesh->v_data[i*6 + 2]);
		}*/
		
		
	}
	return;
}


void vcache_DropMeshData(mesh_t *mesh)
{
	if(mesh)
	{
		//gpu_Free(mesh->vcache_slot_id);
		gpu_Free(vert_cache[mesh->vcache_slot_id].handle);
		stack[++stack_top] = mesh->vcache_slot_id;
		mesh->vcache_slot_id = -1;
		mesh->flags &= ~MESH_CACHED;
		return;
	}
}


void vcache_ExpandCache(int new_size)
{
	vcache_slot_t *t = (vcache_slot_t *)malloc(sizeof(vcache_slot_t) * new_size);
	int *i = (int *)malloc(sizeof(int) * new_size);
	
	/*if(!t)
	{
		printf("null ptr!!!\n\n\n");
	}*/
	
	if(vert_cache)
	{
		memcpy(t, vert_cache, sizeof(vcache_slot_t) * vcache_size);
		free(vert_cache);
		free(stack);
	}
	vert_cache = t;
	stack = i;
	vcache_size = new_size;
}

















