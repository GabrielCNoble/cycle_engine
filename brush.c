#include "brush.h"
#include "gpu.h"
#include "macros.h"
#include <stdlib.h>


static float cube_bmodel_verts[] = 
{
	-1.0, 1.0, 1.0,
	-1.0,-1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0, 1.0, 1.0,
	-1.0, 1.0, 1.0,
	
	 1.0, 1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0, 1.0,-1.0,
	 1.0, 1.0, 1.0,
	 
	 1.0, 1.0,-1.0,
	 1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0, 1.0,-1.0,
	 1.0, 1.0,-1.0,
	 
	-1.0, 1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0,-1.0, 1.0,
	-1.0,-1.0, 1.0,
	-1.0, 1.0, 1.0,
	-1.0, 1.0,-1.0,
	
	-1.0, 1.0,-1.0,
	-1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0,
	 1.0, 1.0,-1.0,
	-1.0, 1.0,-1.0,
	
	-1.0,-1.0, 1.0,
	-1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0, 1.0,
	-1.0,-1.0, 1.0,         
	  
};


static float cube_bmodel_normals[] = 
{
   0.0, 0.0, 1.0,
   0.0, 0.0, 1.0,
   0.0, 0.0, 1.0,
   0.0, 0.0, 1.0,
   0.0, 0.0, 1.0,
   0.0, 0.0, 1.0,
   
   1.0, 0.0, 0.0,
   1.0, 0.0, 0.0,
   1.0, 0.0, 0.0,
   1.0, 0.0, 0.0,
   1.0, 0.0, 0.0,
   1.0, 0.0, 0.0,
   
   0.0, 0.0,-1.0,
   0.0, 0.0,-1.0,
   0.0, 0.0,-1.0,
   0.0, 0.0,-1.0,
   0.0, 0.0,-1.0,
   0.0, 0.0,-1.0,
   
  -1.0, 0.0, 0.0,
  -1.0, 0.0, 0.0,
  -1.0, 0.0, 0.0,
  -1.0, 0.0, 0.0,
  -1.0, 0.0, 0.0,
  -1.0, 0.0, 0.0,
  
   0.0, 1.0, 0.0,
   0.0, 1.0, 0.0,
   0.0, 1.0, 0.0,
   0.0, 1.0, 0.0,
   0.0, 1.0, 0.0,
   0.0, 1.0, 0.0,
   
   0.0,-1.0, 0.0,
   0.0,-1.0, 0.0,
   0.0,-1.0, 0.0,
   0.0,-1.0, 0.0,
   0.0,-1.0, 0.0,
   0.0,-1.0, 0.0,
   
};

static int cube_bmodel_size = sizeof(float) * 6 * 36;
static int cube_bmodel_vcount = 36;

//static unsigned int brush_buffer;


brush_list_t brush_list;
brush_list_t visible_brush_list;	/* updated every frame... */

#ifdef __cplusplus
extern "C"
{
#endif

void brush_Init()
{
	brush_list.max_brushes = 16;
	brush_list.count = 0;
	brush_list.position_data = (bmodel_data0_t *)malloc(sizeof(bmodel_data0_t) * brush_list.max_brushes);
	brush_list.draw_data = (bmodel_data1_t *)malloc(sizeof(bmodel_data1_t) * brush_list.max_brushes);
}

void brush_Finish()
{
	int i;
	for(i = 0; i < brush_list.count; i++)
	{
		free(brush_list.position_data[i].name);
		gpu_Free(brush_list.draw_data->handle);
	}
	free(brush_list.position_data);
	free(brush_list.draw_data);
}


void brush_CreateBrush(char *name, vec3_t position, mat3_t *orientation, vec3_t scale, short type, short material_index)
{
	bmodel_data0_t *position_data;
	bmodel_data1_t *draw_data;
	float *vertex_src;
	float *normal_src;
	
	vec3_t v;
	vec3_t p;
	
	int i;
	int size;
	int vert_count;
	int index = brush_list.count++;
	
	if(index >= brush_list.max_brushes)
	{
		brush_ResizeBrushList(brush_list.max_brushes + 16);
	}
	
	
	position_data = &brush_list.position_data[index];
	draw_data = &brush_list.draw_data[index];
	
	switch(type)
	{
		case BRUSH_CUBE:
			size = cube_bmodel_size;
			vert_count = cube_bmodel_vcount;
			vertex_src = cube_bmodel_verts;
			normal_src = cube_bmodel_normals;
		break;
	}
	
	
	position_data->name = strdup(name);
	position_data->position = position;
	position_data->scale = scale;
	memcpy(&position_data->orientation, orientation, sizeof(mat3_t));
	 
	
	draw_data->verts = (float *)malloc(size);
	draw_data->vert_count = vert_count;
	draw_data->type = type;
	draw_data->material_index = material_index;
	
	
	for(i = 0; i < vert_count; i++)
	{
		
		v.x = vertex_src[i * 3] * scale.x;
		v.y = vertex_src[i * 3 + 1] * scale.y;
		v.z = vertex_src[i * 3 + 2] * scale.z;
		
		v = MultiplyVector3(orientation, v);
		
		v.x += position.x;
		v.z += position.y;
		v.y += position.z;
		
		draw_data->verts[i * 6] = v.x;
		draw_data->verts[i * 6 + 1] = v.y;
		draw_data->verts[i * 6 + 2] = v.z;
	}
	
	for(i = 0; i < vert_count; i++)
	{
		v.x = normal_src[i * 3];
		v.y = normal_src[i * 3 + 1];
		v.z = normal_src[i * 3 + 2];
		
		v = MultiplyVector3(orientation, v);
		
		draw_data->verts[3 + i * 6] = v.x;
		draw_data->verts[3 + i * 6 + 1] = v.y;
		draw_data->verts[3 + i * 6 + 2] = v.z;
	}
	
	
	draw_data->handle = gpu_Alloc(size);
	draw_data->start = gpu_GetAllocStart(draw_data->handle);
	
	gpu_Write(draw_data->handle, 0, draw_data->verts, size, 0);
	
}

void brush_UpdateBrush(bmodel_ptr brush)
{
	int last_vert_count = gpu_GetAllocSize(brush.draw_data->handle) / (sizeof(float) * 6);
	int size = brush.draw_data->vert_count * 6 * sizeof(float);
	if(brush.draw_data->vert_count > last_vert_count)
	{
		gpu_Free(brush.draw_data->handle);
		brush.draw_data->handle = gpu_Alloc(size);
		brush.draw_data->start = gpu_GetAllocStart(brush.draw_data->handle);
	}
	gpu_Write(brush.draw_data->handle, 0, brush.draw_data->verts, size, 0);
	
}

void brush_DeleteBrush(bmodel_ptr brush)
{
	
}

void brush_ResizeBrushList(int new_size)
{
	bmodel_data0_t *d0 = (bmodel_data0_t *)malloc(sizeof(bmodel_data0_t) * new_size);
	bmodel_data1_t *d1 = (bmodel_data1_t *)malloc(sizeof(bmodel_data1_t) * new_size);
	
	memcpy(d0, brush_list.position_data, sizeof(bmodel_data0_t) * brush_list.count);
	memcpy(d1, brush_list.draw_data, sizeof(bmodel_data1_t) * brush_list.count);
	free(brush_list.position_data);
	free(brush_list.draw_data);
	
	brush_list.max_brushes = new_size;
	brush_list.position_data = d0;
	brush_list.draw_data = d1;
}

void brush_TranslateBrush(bmodel_ptr brush, vec3_t direction)
{
	int i;
	int c = brush.draw_data->vert_count;
	
	for(i = 0; i < c; i++)
	{
		brush.draw_data->verts[i * 6] += direction.x;
		brush.draw_data->verts[i * 6 + 1] += direction.y;
		brush.draw_data->verts[i * 6 + 2] += direction.z;
	}
	
	brush.position_data->position.x += direction.x;
	brush.position_data->position.y += direction.y;
	brush.position_data->position.z += direction.z;
	
	brush_UpdateBrush(brush);
}

void brush_RotateBrush(bmodel_ptr brush, vec3_t axis, float amount)
{
	
}

PEWAPI bmodel_ptr brush_GetBrushByIndex(int index)
{
	bmodel_ptr b = {NULL, NULL};
	if(index >= 0 && index < brush_list.count)
	{
		b.position_data = &brush_list.position_data[index];
		b.draw_data = &brush_list.draw_data[index];
	}
	return b;
}

#ifdef __cplusplus
}
#endif







