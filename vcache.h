#ifndef VCACHE_H
#define VCACHE_H

#include "conf.h"
#include "includes.h"
#include "model.h"

typedef struct vcache_slot_t
{
	mesh_t *mesh;
	int handle;
	int start;
	int cur_refs;
	int lst_refs;
}vcache_slot_t;

typedef struct vcache_stack
{
	int stack_size;
	int top;
	int *stack;
}vcache_stack_t;


void vcache_Init();

void vcache_Finish();

void vcache_CacheMeshData(mesh_t *mesh);

void vcache_DropMeshData(mesh_t *mesh);

void vcache_ExpandCache(int new_size);






#endif /* VERT_CACHE_H */
