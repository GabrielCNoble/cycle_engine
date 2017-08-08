#ifndef BRUSH_H
#define BRUSH_H

#include "matrix.h"
#include "vector.h"




enum BRUSH_TYPE
{
	BRUSH_CUBE = 1,
	BRUSH_CONE,
	BRUSH_CYLINDER,
};


typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale; 
	char *name;
}bmodel_data0_t;


typedef struct
{
	float *verts;
	int vert_count;
	int handle;
	int start;
	short material_index;
	short type;
}bmodel_data1_t;


typedef struct
{
	bmodel_data0_t *position_data;
	bmodel_data1_t *draw_data;
}bmodel_ptr_t;


typedef struct
{
	int count;
	int max_brushes;
	bmodel_data0_t *position_data;
	bmodel_data1_t *draw_data;
}brush_list_t;


void brush_Init();

void brush_Finish();

void brush_CreateBrush(char *name, vec3_t position, mat3_t *orientation, vec3_t scale, short type, short material_index);

void brush_UpdateBrush(bmodel_ptr_t brush);

void brush_DeleteBrush(bmodel_ptr_t brush);

void brush_ResizeBrushList(int new_size);

void brush_TranslateBrush(bmodel_ptr_t brush, vec3_t direction);

void brush_RotateBrush(bmodel_ptr_t brush, vec3_t axis, float amount);





#endif









