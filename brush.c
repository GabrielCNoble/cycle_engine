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
		free(brush_list.draw_data->verts);
		gpu_Free(brush_list.draw_data->handle);
	}
	free(brush_list.position_data);
	free(brush_list.draw_data);
}


PEWAPI int brush_CreateBrush(char *name, vec3_t position, mat3_t *orientation, vec3_t scale, short type, short material_index, short bm_flags)
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
	position_data->brush_index = index;
	memcpy(&position_data->orientation, orientation, sizeof(mat3_t));
	 
	
	draw_data->verts = (float *)malloc(size);
	draw_data->vert_count = vert_count;
	draw_data->type = type;
	draw_data->material_index = material_index;
	draw_data->bm_flags = bm_flags;
	
	
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
	
	brush_SortBrushList();
	
	return index;
	
}

PEWAPI int brush_CopyBrush(bmodel_ptr brush, char *name)
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
	
	position_data->name = strdup(name);
	position_data->position = brush.position_data->position;
	position_data->scale = brush.position_data->scale;
	position_data->brush_index = index;
	memcpy(&position_data->orientation, &brush.position_data->orientation, sizeof(mat3_t));
	 
	size = brush.draw_data->vert_count * 6 * sizeof(float); 
	
	vert_count = brush.draw_data->vert_count;
	
	draw_data->verts = (float *)malloc(size);
	draw_data->vert_count = vert_count; 
	draw_data->type = brush.draw_data->type;
	draw_data->material_index = brush.draw_data->material_index;
	draw_data->bm_flags = brush.draw_data->bm_flags;
	
	
	
	for(i = 0; i < vert_count; i++)
	{
		draw_data->verts[i * 6] = brush.draw_data->verts[i * 6];
		draw_data->verts[i * 6 + 1] = brush.draw_data->verts[i * 6 + 1];
		draw_data->verts[i * 6 + 2] = brush.draw_data->verts[i * 6 + 2];
		
		draw_data->verts[3 + i * 6] = brush.draw_data->verts[3 + i * 6];
		draw_data->verts[3 + i * 6 + 1] = brush.draw_data->verts[3 + i * 6 + 1];
		draw_data->verts[3 + i * 6 + 2] = brush.draw_data->verts[3 + i * 6 + 2];
	}	
	
	draw_data->handle = gpu_Alloc(size);
	draw_data->start = gpu_GetAllocStart(draw_data->handle);
	
	gpu_Write(draw_data->handle, 0, draw_data->verts, size, 0);
	
	brush_SortBrushList();
	
	
	return index;
}

void brush_IntersectBrushes(bmodel_ptr brush, bmodel_ptr subtractive_brush)
{
	
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
	int index;
	int last;
	if(brush.position_data)
	{
		free(brush.position_data->name);
		free(brush.draw_data->verts);
		gpu_Free(brush.draw_data->handle);
		
		index = brush.position_data->brush_index;
		last = brush_list.count - 1;
		
		if(index < last)
		{	
			brush_list.position_data[index] = brush_list.position_data[last];
			brush_list.draw_data[index] = brush_list.draw_data[last];
			brush_list.position_data[index].brush_index = index;
		}
		brush_list.count--;
		
		brush_SortBrushList();
		
	}
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

void brush_QuickSort(int left, int right)
{
	int l = left;
	int r = right;
	int i;
	int j;
	bmodel_ptr b;
	bmodel_data0_t position_data;
	bmodel_data1_t draw_data;
	
	int m = (l + r) / 2;
	
	b = brush_GetBrushByIndex(m);
	i = left;
	j = right;
	do
	{
		for(;i < right && brush_list.draw_data[i].material_index < b.draw_data->material_index; i++);
		for(; j > left && brush_list.draw_data[j].material_index > b.draw_data->material_index; j--);
		
		if(i <= j)
		{
			
			draw_data = brush_list.draw_data[i];
			position_data = brush_list.position_data[i];
			
			brush_list.draw_data[i] = brush_list.draw_data[j];
			brush_list.position_data[i] = brush_list.position_data[j];
			brush_list.position_data[i].brush_index = i;
		
			brush_list.draw_data[j] = draw_data;
			brush_list.position_data[j] = position_data;
			brush_list.position_data[j].brush_index = j;
			
			i++;
			j--;
		}
	}while(i <= j);
	
	if(j > left) brush_QuickSort(left, j);
	if(i < right) brush_QuickSort(i, right);
}

void brush_SortBrushList()
{
	
	brush_QuickSort(0, brush_list.count - 1);
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
	int i;
	int c = brush.draw_data->vert_count;
	vec3_t v;
	vec3_t p;
	vec3_t n;
	
	mat3_t rotation;
	mat3_t old_rotation;
	
	
	mat3_t_rotate(&rotation, axis, amount, 1);
	
	memcpy(&old_rotation.floats[0][0], &brush.position_data->orientation.floats[0][0], sizeof(mat3_t));
	
	mat3_t_mult(&brush.position_data->orientation, &old_rotation, &rotation);
	
	for(i = 0; i < c; i++)
	{
		
		v.x = brush.draw_data->verts[i * 6];
		v.y = brush.draw_data->verts[i * 6 + 1];
		v.z = brush.draw_data->verts[i * 6 + 2];
		
		n.x = brush.draw_data->verts[3 + i * 6];
		n.y = brush.draw_data->verts[3 + i * 6 + 1];
		n.z = brush.draw_data->verts[3 + i * 6 + 2];
		
		v.x -= brush.position_data->position.x;
		v.y -= brush.position_data->position.y;
		v.z -= brush.position_data->position.z;
		
		p.x = v.x * rotation.floats[0][0] + 
		      v.y * rotation.floats[1][0] + 
		      v.z * rotation.floats[2][0];
		      
		p.y = v.x * rotation.floats[0][1] + 
		      v.y * rotation.floats[1][1] + 
		      v.z * rotation.floats[2][1];
			  
		p.z = v.x * rotation.floats[0][2] + 
		      v.y * rotation.floats[1][2] + 
		      v.z * rotation.floats[2][2];	        
		
		brush.draw_data->verts[i * 6] = p.x + brush.position_data->position.x;
		brush.draw_data->verts[i * 6 + 1] = p.y + brush.position_data->position.y;
		brush.draw_data->verts[i * 6 + 2] = p.z + brush.position_data->position.z;
		
		
		p.x = n.x * rotation.floats[0][0] + 
		      n.y * rotation.floats[1][0] + 
		      n.z * rotation.floats[2][0];
		      
		p.y = n.x * rotation.floats[0][1] + 
		      n.y * rotation.floats[1][1] + 
		      n.z * rotation.floats[2][1];
			  
		p.z = n.x * rotation.floats[0][2] + 
		      n.y * rotation.floats[1][2] + 
		      n.z * rotation.floats[2][2];	        
		
		brush.draw_data->verts[3 + i * 6] = p.x;
		brush.draw_data->verts[3 + i * 6 + 1] = p.y;
		brush.draw_data->verts[3 + i * 6 + 2] = p.z;
	}
	
	brush_UpdateBrush(brush);
	
}

PEWAPI void brush_ScaleBrush(bmodel_ptr brush, vec3_t axis, float amount)
{
	int i;
	int c = brush.draw_data->vert_count;
	
	vec3_t prev_scale;
	vec3_t new_scale;
	vec3_t translation;
	vec3_t v;
	vec3_t p;
	
	mat3_t inverse_rotation;

	
	prev_scale = brush.position_data->scale;
	
	translation.x = brush.position_data->position.x;
	translation.y = brush.position_data->position.y;
	translation.z = brush.position_data->position.z;
	
	brush.position_data->scale.x += axis.x * amount;
	brush.position_data->scale.y += axis.y * amount;
	brush.position_data->scale.z += axis.z * amount;
	
	
	//prev_scale = MultiplyVector3(&brush.position_data->orientation, prev_scale);
	//new_scale = MultiplyVector3(&brush.position_data->orientation, brush.position_data->scale);
	
	memcpy(&inverse_rotation.floats[0][0], &brush.position_data->orientation.floats[0][0], sizeof(mat3_t));
	
	mat3_t_transpose(&inverse_rotation);
	
	new_scale = brush.position_data->scale;
	
	for(i = 0; i < c; i++)
	{
		
		v.x = brush.draw_data->verts[i * 6] - translation.x;
		v.y = brush.draw_data->verts[i * 6 + 1] - translation.y;
		v.z = brush.draw_data->verts[i * 6 + 2] - translation.z;
		
		v = MultiplyVector3(&inverse_rotation, v);
		
		v.x *= (new_scale.x / prev_scale.x);
		v.y *= (new_scale.y / prev_scale.y);
		v.z *= (new_scale.z / prev_scale.z);
		
		v = MultiplyVector3(&brush.position_data->orientation, v);
		
		brush.draw_data->verts[i * 6] = v.x  + translation.x;
		brush.draw_data->verts[i * 6 + 1] = v.y + translation.y;
		brush.draw_data->verts[i * 6 + 2] = v.z + translation.z;
	}
	
	brush_UpdateBrush(brush);
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







