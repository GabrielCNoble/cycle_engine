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

enum BRUSH_FLAGS
{
	BRUSH_SUBTRACTIVE = 1,
	
};


typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale; 
	int brush_index;
	char *name;
}bmodel_data0_t;


typedef struct
{
	float *verts;
	int *face_indexes;
	int vert_count;
	int handle;
	int start;
	short material_index;
	short type;
	short bm_flags;
}bmodel_data1_t;


typedef struct
{
	bmodel_data0_t *position_data;
	bmodel_data1_t *draw_data;
}bmodel_ptr;


typedef struct
{
	int count;
	int max_brushes;
	bmodel_data0_t *position_data;
	bmodel_data1_t *draw_data;
}brush_list_t;

#ifdef __cplusplus
extern "C"
{
#endif

void brush_Init();

void brush_Finish();

PEWAPI int brush_CreateBrush(char *name, vec3_t position, mat3_t *orientation, vec3_t scale, short type, short material_index, short bm_flags);

PEWAPI int brush_CopyBrush(bmodel_ptr brush, char *name);

void brush_IntersectBrushes(bmodel_ptr brush, bmodel_ptr subtractive_brush);

void brush_UpdateBrush(bmodel_ptr brush);

PEWAPI void brush_DeleteBrush(bmodel_ptr brush);

void brush_ResizeBrushList(int new_size);

/* keeping the brush list sorted is important for command buffer emission. It avoids sorting
by material every frame before putting the visible brushes into the render queue. */
void brush_SortBrushList();

PEWAPI void brush_TranslateBrush(bmodel_ptr brush, vec3_t direction);

PEWAPI void brush_RotateBrush(bmodel_ptr brush, vec3_t axis, float amount);

PEWAPI void brush_ScaleBrush(bmodel_ptr brush, vec3_t axis, float amount);

PEWAPI bmodel_ptr brush_GetBrushByIndex(int index);

#ifdef __cplusplus
}
#endif




#endif









