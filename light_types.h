#ifndef LIGHT_TYPES_H
#define LIGHT_TYPES_H


#include "material_types.h"
//#include "scenegraph.h"
#include "scenegraph_types.h"
#include "vector_types.h"
#include "matrix_types.h"

#define MAX_LIGHTS_PER_ENTITY 16

#define LIGHT_ZNEAR 0.001

#define LIGHT_MAX_RADIUS 80.0
#define LIGHT_MIN_RADIUS 1.0


#define MIN_SHADOW_MAP_RES 				32
#define MAX_SHADOW_MAP_RES      		MIN_SHADOW_MAP_RES * 255

#define MIN_VOLUME_SAMPLES 				4
#define MAX_VOLUME_SAMPLES      		64
#define MAX_LIGHT_VOLUME_SCATTERING 	0.1
#define MIN_LIGHT_VOLUME_SCATTERING 	0.000002

#define MAX_LIGHT_ENERGY 				1000.0
#define MIN_LIGHT_ENERGY				0.02

/* TODO (#1#): Avoid unmoving objects to be re-rendered 
	               into shadow maps. Maybe create a big, shared 
	               shadow maps for objects that haven't moved 
	               in two frames, and re-render just objects that 
	               are moving in the light's shadow map. 
				   Shadow mapping would then 
	               access both shadow maps. Other solution 
	               would be to not render cube shadow map 
	               faces where no objects moved recently. */

enum LIGHT_FLAGS
{
	LIGHT_POINT =								1,
	LIGHT_SPOT =								1<<1,
	LIGHT_DIRECTIONAL =							1<<2,
	LIGHT_GENERATE_SHADOWS =					1<<3,
	LIGHT_FULL_STATIC =							1<<4,		/* full static light, which won't ever move. Light maps only. */
	LIGHT_HALF_STATIC =							1<<5,		/* half static light, which won't ever move, light maps for static entities and brushes,
															   but shadow map regeneration for dynamic stuff around.*/
	LIGHT_DYNAMIC =								1<<6,		/* full dynamic light. */
	LIGHT_DRAW_VOLUME =							1<<7,
	LIGHT_REFRESH_SHADOW_MAP = 					1<<8,		/* if a static light has moved (even though it absolutely shouldn't), its shadow map needs to be
													   		refreshed */
	LIGHT_VIEWPOINT_INSIDE_VOLUME = 			1<<9												   
};


enum LIGHT_STATUS
{
	LIGHT_FRUSTUM_CULLED=1,
	LIGHT_OCCLUDED=2
};

enum LIGHT_FALLOF_FUNCS
{
	LIGHT_FALLOF_LINEAR=1,
	LIGHT_FALLOF_NOMRALIZED=2
};

enum LIGHT_CUBESHADOW_FACES
{
	CUBESHADOW_FACE_POSITIVE_X=1,
	CUBESHADOW_FACE_NEGATIVE_X=2,
	CUBESHADOW_FACE_POSITIVE_Y=4,
	CUBESHADOW_FACE_NEGATIVE_Y=8,
	CUBESHADOW_FACE_POSITIVE_Z=16,
	CUBESHADOW_FACE_NEGATIVE_Z=32,
};


typedef struct
{
	//unsigned int shadow_fb;
	unsigned short resolution;	
	unsigned short shadow_map;			/* for spot lights, this will be a GL_TEXTURE_2D. For point lights, will be GL_TEXTURE_CUBE_MAP */
	//unsigned int alpha_map;				/* to render correct translucent objects shadows */
}smap_t;


typedef struct
{
	mat4_t light_projection_matrix;
	mat4_t world_to_light_matrix;
	frustum_t generated_frustum;
	node_t *assigned_node;
	char *name;
}light_data3;
/* light_extra_data */ 

/* TODO: rename those structs so they have a more meaningful name... */
typedef struct
{
	mat3_t world_orientation;
	vec4_t world_position;
	float radius;
	float screen_value;
	int tex_index;					/* texture that can be used both for projection or modulation of the spot light's intensity */
	//short align0;
	/*short smin_x;
	short smin_y;
	short smax_x;
	short smax_y;*/
	
	mat3_t local_orientation;
	vec4_t local_position;
	int align1;
	int align2;
	char bm_state;
	short bm_flags;
	char spot_co;		/* cutoff (in degrees) */
	//char align4;
	//vec3_t align2;
}light_data0;	/* 128 bytes (2 cache lines) */
/* light_position */


typedef struct
{
	//node_t *assigned_node;
	//char *name;
	//color4_t diffuse;
	
	//color4_t specular;
	//unsigned int align1;
	
	//float scattering;				
	//float energy;
	short scattering;
	short energy;
	unsigned short lin_fallof;
	unsigned short sqr_fallof;
	unsigned char shadow_map_res;		/* stored in multiples of the minimum shadow map resolution */
	//unsigned char min_shadow_map_res;
	
	unsigned char r;
	unsigned char g;
	unsigned char b;
	//short max_shadow_map_res;
	//short min_shadow_map_res;
	//float radius; 
	//unsigned short align2;
	unsigned char const_fallof;
	
	//unsigned char spot_co;		/* cutoff (in degrees) */
	unsigned char spot_e;
	
	//unsigned char type;
	//unsigned char bm_status;
	//unsigned char bm_flags;
	
	unsigned char volume_samples;			// used for lod'ing the light volumes 
	//unsigned char min_samples;			// these values are valid only on light_a array
	unsigned char max_shadow_aa_samples;
	//unsigned char align3;				// on active_light_a, max_samples will be overwritten
	//unsigned char align2;				// with the value to be used on that frame, which will
	//unsigned char align3;				// be determined by the apparent size of the light on screen...
	
}light_data1;	/* 16 bytes (half half cache line) */
/* light params */
#include "draw.h"


/* NOTE: a better scheme could be creating an array for shadowmaps, thus leaving them decoupled from the lights. This way, spot
ligths wouldn't have 20 unused bytes inside of it. The light would have just an index to this array.*/

/* reestructure this into something more useful */
typedef struct
{
	mat4_t model_view_projection_matrix;	
	smap_t shadow_map;
	float znear;
	float zfar;
	//int bm_shadow_flags;
}light_data2;
/* light_shadow_data */
 

 
 
/*typedef struct
{
	mat3_t local_orientation;
	vec4_t local_position;
	float align1;
	float align2;
	float align3;
}light_local_data;*/	/* 64 bytes (1 cache line) */




typedef struct
{
	int count;
	unsigned char light_IDs[MAX_LIGHTS_PER_ENTITY];
}affecting_lights_t;

typedef struct
{
	int count;
	int size;
	int stack_top;
	int *free_stack;
	affecting_lights_t *lights;
}affecting_lights_list;


typedef struct light_array
{
	int array_size;
	int light_count;
	//light_world_data *world_data;
	//light_local_data *local_data;
	light_data2 *shadow_data;
	light_data0 *position_data;
	light_data3 *extra_data;
	light_data1 *params;
	//light_t *lights;
}light_array;

typedef struct
{
	/*light_world_data *world_data;
	light_local_data *local_data;*/
	light_data2 *shadow_data;
	light_data0 *position_data;
	light_data3 *extra_data;
	light_data1 *params;
}light_ptr;


typedef struct
{
	unsigned short resolution;
	unsigned short users;
	unsigned int id;
	
	
}shared_shadow_map_t;



#endif /* LIGHT_TYPES_H_ */










