#ifndef DRAW_DEBUG_H
#define DRAW_DEBUG_H

#include "conf.h"
#include "vector.h"
#include "entity.h"

#define DRAW_SCREEN_QUAD_BEGIN 0
#define DRAW_SCREEN_QUAD_COUNT 4


#define DRAW_SPHERE_LOD0_BEGIN 4
#define DRAW_SPHERE_LOD0_COUNT 240

#define DRAW_SPHERE_LOD1_BEGIN 244
#define DRAW_SPHERE_LOD1_COUNT 60

#define DRAW_CONE_LOD0_BEGIN 304
#define DRAW_CONE_LOD0_COUNT 48

#define DRAW_CONE_LOD1_BEGIN 352
#define DRAW_CONE_LOD1_COUNT 30


enum DRAW_TYPE
{
	DRAW_ENTITY_ORIGIN = 1,
	DRAW_ENTITY_AABB,
	DRAW_LIGHT_ORIGIN,
	DRAW_LIGHT_LIMITS,
	DRAW_BONE_ORIGIN,
	DRAW_BONE,
	DRAW_CONSTRAINT,
	DRAW_POINT,						/* this allow general debug drawing, so vertex color will be uploaded in the float buffer */
	DRAW_POINT_HOMOGENEOUS,
	DRAW_LINE,
	DRAW_LINE_HOMOGENEOUS,
	DRAW_TRIANGLE,
	DRAW_MESH,								
	DRAW_OUTLINE, 					/* useful for drawing object outline... */
};


typedef struct debug_draw_t
{
	float *data; 				/* this is the address inside the float buffer. Used only for general debug drawing cmds... */
	int count;
	int type;
}debug_draw_t;


#ifdef _cpluspluc
extern "C"
{
#endif 

void draw_debug_Init();

void draw_debug_Finish();

void draw_debug_Draw();

PEWAPI void draw_debug_DrawTriangle(vec3_t a, vec3_t b, vec3_t c, vec3_t a_color, vec3_t b_color, vec3_t c_color);

PEWAPI void draw_debug_DrawLine(vec3_t from, vec3_t to, vec3_t color, float line_thickness, int b_xray, int homogeneous);

//PEWAPI void draw_debug_Draw2DLine(vec2_t from, vec2_t to, vec3_t color);

PEWAPI void draw_debug_DrawPoint(vec3_t position, vec3_t color, float point_size, int homogenous);

PEWAPI void draw_debug_DrawPointHomogeneous(vec3_t position, vec3_t color, float point_size);



PEWAPI void draw_debug_DrawOutline(vec3_t position, mat3_t *orientation, mesh_t *mesh, vec3_t color, float line_thickness, int b_xray);

//PEWAPI void draw_debug_Draw2DPoint(vec2_t position, vec3_t color, float point_size);

//PEWAPI void draw_debug_DrawPointHomogeneous(vec3_t position, vec3_t color, float point_size);

//PEWAPI void draw_debug_DrawLineHomogeneous(vec3_t a, vec3_t b, vec3_t color);

void draw_debug_DrawEntitiesAABBs();

void draw_debug_DrawLights(int b_draw_limits);

void draw_debug_DrawArmatures();

void draw_debug_Constraints();

void draw_debug_DrawZBuffer();

void draw_debug_DrawNBuffer();

void draw_debug_DrawDBuffer();

void draw_debug_DrawVertexData(vec3_t position, vertex_data_t *data);

#ifdef _cplusplus
}
#endif



#endif /* DRAW_DEBUG_H */
