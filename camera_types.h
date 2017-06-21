#ifndef CAMERA_TYPES_H
#define CAMERA_TYPES_H

#include "scenegraph.h"
#include "matrix_types.h"
#include "vector_types.h"
#include "frustum_types.h"
#include "framebuffer.h"


typedef struct
{
	int offset;
	int count;
}cluster_ll_t;

typedef struct
{
	vec3_t max;
	vec3_t min;
}cluster_aabb_t;

typedef struct
{
	cluster_ll_t *light_lists;
	cluster_aabb_t *aabbs;
}cluster_list_t;

typedef struct camera_t
{
	mat4_t projection_matrix;
	mat4_t world_to_camera_matrix;
	mat3_t local_orientation;
	mat3_t world_orientation;
	framebuffer_t framebuffer;
	frustum_t frustum;
	vec3_t local_position;
	vec3_t world_position;
	node_t *assigned_node;
	float zoom; 
	float exposure;
	int width;
	int height;
	int camera_index;
	char *name;
}camera_t;


typedef struct 
{
	int array_size;
	int camera_count;
	camera_t *cameras;
	cluster_list_t *clusters;
	//cluster_ll_t **clusters_light_lists;
	//cluster_aabb_t **clusters_aabbs;
}camera_array;





#endif /* CAMERA_TYPES_H */
