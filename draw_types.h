#ifndef DRAW_TYPES_H
#define DRAW_TYPES_H

#include "includes.h"
#include "matrix_types.h"
#include "vector_types.h"

enum RENDERER_RESOLUTIONS
{
	RENDERER_1920x1080=0,
	RENDERER_1600x900,
	RENDERER_1366x768,
	RENDERER_1280x1024,
	RENDERER_1280x960,
	RENDERER_1024x768,
	RENDERER_800x600,
};

enum RENDER_MODE
{
	RENDER_DRAWMODE_LIT = 1,
	RENDER_DRAWMODE_FLAT,
	RENDER_DRAWMODE_WIREFRAME,
};

enum RENDER_FLAGS
{
	RENDERFLAG_DRAW_LIGHT_VOLUMES=1,
	RENDERFLAG_USE_SHADOW_MAPS=2,
	RENDERFLAG_USE_BLOOM = 4,
};

enum DEBUG_MODE
{
	//DEBUG_DISABLED=0,
	DEBUG_L1,
	DEBUG_L2,
	DEBUG_L3
};

enum DEBUG_DRAW
{
	DEBUG_DISABLED = 0,
	DEBUG_DRAW_ENTITY_ORIGIN = 1,
	DEBUG_DRAW_ENTITY_AABB = 1<<1,
	DEBUG_DRAW_LIGHT_ORIGINS = 1<<2,
	DEBUG_DRAW_LIGHT_LIMITS = 1<<3,
	DEBUG_DRAW_COLLIDERS = 1<<4,
	DEBUG_DRAW_ZBUFFER = 1<<5,
	DEBUG_DRAW_NBUFFER = 1<<6,
	DEBUG_DRAW_DBUFFER = 1<<7,
	DEBUG_DRAW_ARMATURES = 1<<8,
	DEBUG_DRAW_OUTLINES = 1<<9
};


enum INITIALIZATION_MODE
{
	INIT_DETECT=0,
	INIT_FORCE_FULLSCREEN,
	INIT_WINDOWED
};

enum
{
	BLOOM_SMALL_RADIUS = 0,
	BLOOM_MEDIUM_RADIUS,
	BLOOM_LARGE_RADIUS,
	BLOOM_SMALL_ITERATIONS,
	BLOOM_MEDIUM_ITERATIONS,
	BLOOM_LARGE_ITERATIONS,
	BLOOM_INTENSITY,
};


enum COMMAND_BUFFER_TYPE
{
	COMMAND_BUFFER_NORMAL=0,
	COMMAND_BUFFER_DEBUG=1,
	COMMAND_BUFFER_GEOMETRY_DATA,
	COMMAND_BUFFER_CAMERA_DATA,
	COMMAND_BUFFER_LIGHT_DATA,
};

enum COMMAND_BUFFER_VERTEX_ATTRIBUTE
{
	
	CBATTRIBUTE_NORMAL=1<<8,
	CBATTRIBUTE_TEX_COORD=1<<9,
	CBATTRIBUTE_TANGENT=1<<10,
	CBATTRIBUTE_BTANGENT=1<<11,
	CBATTRIBUTE_VOLATILE_DATA = 1<<12					/* skinned meshes data is kind of mutable, so make the renderer get it from a special buffer... */
	
};

enum BLEND_MODE
{
	BLEND_ADDITIVE=1,
	
};



typedef union
{
	mat4_t model_view_matrix;
	struct
	{
		int un0, un1, un2, start;
		int un3, un4, un5, vert_count;
		int un6, un7, lights_index, entity_index;
		int un8, un9, un10;
		short material_index;
		short draw_flags;
		
	};
	//mat4_t last_model_view_matrix;			
}command_buffer_t;

typedef struct
{
	mat4_t model_view_matrix;
	mat4_t last_model_view_matrix;
}command_buffer_128_t;

typedef struct
{
	int queue_size;
	int count;
	int *entity_indexes;
	command_buffer_t *base;
	command_buffer_t *command_buffers;
}render_queue;

typedef struct 
{
	SDL_Window *window;
	SDL_GLContext *context;
	//FT_Library font_renderer;
//	FT_Face font; 
	int screen_width;
	int screen_height;
	int width;
	int height;
	int selected_resolution;
	int active_camera_index;
	int active_shader_index;
	int active_material_index;
	int frame_count;
	int render_mode;
	int renderer_flags;
	float time;

}renderer_t;

typedef struct
{
	float x_max;
	float x_min;
	float y_max;
	float y_min;
	int max_light_count;
	int light_count;
	int *light_IDs;
}screen_tile;

typedef struct
{
	int tile_width;
	int tile_height;
	int tiles_per_row;
	int tiles_per_coloumn;
	int tile_count;
	screen_tile *tiles;
}screen_tile_list;

typedef struct varying_t
{
	short type;
	char *name;
	struct varying_t *next;
}varying_t;





/*typedef struct
{
	mat4_t light_model_view_projection_matrix;
	mat4_t shadow_draw_data;
}shadow_command_buffer_t;


typedef struct
{
	int queue_size;
	int count;
	shadow_command_buffer_t *shadow_cbs;
}shadow_queue;*/



#endif /* DRAW_TYPES_H */


















