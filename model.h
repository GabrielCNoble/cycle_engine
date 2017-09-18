#ifndef MODEL_H
#define MODEL_H

#include "conf.h"
#include "includes.h"
#include "gpu.h"

#include "gmath/vector.h"
#include "gmath/matrix.h"
#include "gmath/triangle_types.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision\CollisionShapes\btShapeHull.h"



enum MESH_T_FLAGS
{
	MESH_HAS_CHANGED = 1,
	MESH_FULL_COPY = 1 << 1,
	MESH_CACHED = 1 << 2
};

/*typedef struct
{
	float *v_data;
	float *n_data;
	float *t_c_data;
	int vert_count;
	struct face_t *next;
	struct face_t *previous;
}face_t;*/


/*typedef struct
{
	float a[3];
	float b[3];
	triangle_t *a_tri;
	triangle_t *b_tri;
}edge_t;*/

//#pragma pack(1)
typedef struct
{
	vec3_t position;
	vec3_t normal;
	vec3_t tangent;
	vec2_t tex_coord;
}vertex_t;

//#pragma pack(4)


typedef struct
{
	int *indexes;
	int vert_count;
}face_indexes;

typedef struct vertex_data_t
{
	float *position;
	float *normal;
	float *tangent;
	float *tex_coord;
	edge_t *edges;					/* useful for edge decimation and mesh simplification */
	int *indexes;
	char *name;
	int index_count;
	int vertex_count;
	int edge_count;
	struct vertex_data_t *next;
}vertex_data_t;						/* original vertex data from disk */

typedef struct
{
	//float *v_data;
	//float *n_data;
	//float *t_c_data;
	//float *t_data;
	
	vertex_t *vertices;
	
	
	//float *ov_data;
	//float *on_data;
	//float *ot_c_data;
	//float *ot_data;
	//int *i_data;									/* indexes */
	int id;
	
	/* this shouldn't be related with collision. The physics engine should grab the lowest acceptable LOD of
	this vertex data, instead of the mesh_t having data dedicated to it... */
	btConvexHullShape *collision_shape;				/* simplified convex hull */
	//float *b_data;
	int vert_count;
	short draw_mode;
	short flags;
	//unsigned int vertex_buffer;
	//unsigned int index_buffer;
	int vcache_slot_id;	
	int start;							
	//gpu_buffer buffer;					
	char *name;
}mesh_t;	


//typedef struct
//{
//	float limits[6];		/* x_max, x_min, y_max, y_min, z_max, z_min of the forward vector*/
//	int *indexes;			/* indexes that should be used for extraction */
//}extrusion_cache_t;		/* used for quick lookup by the shadow volume generator */


typedef struct
{
	int array_size;
	int mesh_count;
	mesh_t *meshes;
	//char **names;			/* names */
}mesh_array;	


typedef struct mesh_node_t
{
	mesh_t mesh;
	struct mesh_node_t *next;
}mesh_node_t;


#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void model_Init(char *path);

PEWAPI void model_Finish();

PEWAPI void model_ResizeMeshArray(int new_size);

PEWAPI int model_StoreMesh(mesh_t *mesh);

PEWAPI void model_StoreVertexData(char *name, float *position, float *normal, float *tangent, float *tex_coord, edge_t *edges, int *indexes, int index_count, int vertex_count, int edge_count);

PEWAPI vertex_data_t *model_GetVertexData(char *name);

void model_AddMeshNode(mesh_t *mesh);

void model_RemoveMeshNode(char *name);

void model_FlushMeshList();

PEWAPI mesh_t model_GetMesh(char *name, int b_full_copy);

PEWAPI mesh_t *model_GetMeshPtr(char *name);

PEWAPI void model_FreeMesh(mesh_t *mesh);

PEWAPI void model_GetMaxMinsFromVertexData(vertex_t *vertex_data, float *maxmins, int vertex_count);

PEWAPI void model_CalculateTangents(vertex_t *vertices, int vert_count);

PEWAPI void model_LoadModel(char *filename, char *name);

PEWAPI void model_GenerateIcoSphere(float radius, int sub_divs, float **verts, int *face_count);

PEWAPI void model_GenerateIcoSphereMesh(float radius, int sub_divs);

PEWAPI void model_GenerateCone(float length, float angle, int base_vert_count, float **verts, int *face_count);

PEWAPI void model_GenerateConeMesh(float length, float angle, int base_vert_count);

PEWAPI void model_GenerateQuadMesh(float size);

PEWAPI void model_GenerateConvexPoly(float size, int vert_count);

float model_GetFloat(char *file_str, int *cursor);

vec2_t model_GetVec2(char *file_str, int *str_cursor);

vec3_t model_GetVec3(char *file_str, int *str_cursor);

face_indexes model_GetFaceData(char *file_str, int *str_cursor);

void model_TriangulateFace(float **verts, int *vert_count);

void model_TriangulateFaceIndexes(int **indexes, int *vert_count);

void model_LoadMaterialFromWavefront(char *file_name);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_H */










