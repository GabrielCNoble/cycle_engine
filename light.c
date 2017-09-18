#include "light.h"
#include "shader.h"
#include "draw.h"
#include "camera.h"
#include "material.h"	
#include "vector.h"
//#include "framebuffer.h"
#include "macros.h"
#include "input.h"
#include "draw_debug.h"


#define SHADOW_MAP_TEXTURE_CIELING 8192

/*enum RENDERER_RESOLUTIONS
{
	RENDERER_1920x1080=0,
	RENDERER_1600x900,
	RENDERER_1366x768,
	RENDERER_1280x1024,
	RENDERER_1280x960,
	RENDERER_1024x768,
	RENDERER_800x600,
};*/

//enum CLUSTER_TEXTURE_RESOLUTIONS
//{
//	CLUSTER_64x36 = 0,					/* 30 x 30 clusters */
	//CLUSTER_64x36,						/* 25 x 25 clusters */
//	CLUSTER_30x32,						/* ~46 x 24 clusters */
//	CLUSTER_64x64,						/* 20 x 16 clusters */
//	CLUSTER_LAST,
//};

enum CLUSTER_RESOLUTIONS
{
	CLUSTER_1920x1080 = 0,
	CLUSTER_1600x900,
	CLUSTER_1366x768,
	CLUSTER_LAST,
};


int cluster_texture_resolutions[CLUSTER_LAST][2] = 
{
	60, 34,
	50, 29,
	43, 24,
};




extern renderer_t renderer;
extern shader_array shader_a;
light_array light_a;
light_array active_light_a;

int *active_light_indexes;
extern camera_array camera_a;
int max_lights_per_pass;

mat4_t *active_light_transforms;

unsigned int *clusters; 

affecting_lights_list affecting_lights;
//extern framebuffer_t shadow_buffer;


static mat4_t light_projection_matrix;

extern screen_tile_list screen_tiles;
extern float color_conversion_lookup_table[256];
 
mat4_t cube_shadow_mats[6];


unsigned int extra_framebuffer;

static int light_cache_size;
static int cached_light_count;
static int free_stack_top;
static int *free_stack;
static unsigned int light_cache;
unsigned int cluster_texture;
unsigned int cluster_ubo;

extern int bm_extensions;

extern int uniform_buffer_alignment;
extern int light_params_uniform_buffer_size;
extern int type_offsets[];



int ssm_cur_width;
int ssm_cur_height;
unsigned int ssm;


int free_chunk_count;
int free_chunk_size;
ks_chunk_t *free_chunks;

int alloc_chunk_count;
int alloc_chunk_size;
ks_chunk_t *alloc_chunks;


int shadow_memory_update_delay = 0;



void light_CacheGPULight(light_ptr light);

void light_DropGPULight(light_ptr light);

void light_Init()
{
	
	mat3_t m;
 	
 	//glGenFramebuffers(1, &extra_framebuffer);
	
	light_cache_size = MAX_ACTIVE_LIGHTS;
	cached_light_count = 0;
	free_stack_top = -1;
	free_stack = (int *)malloc(sizeof(int) * light_cache_size);
	
	if(bm_extensions & EXT_UNIFORM_BUFFER_OBJECT)
	{
		//while(glGetError() != GL_NO_ERROR);
		glGenBuffers(1, &light_cache);
		glBindBuffer(GL_UNIFORM_BUFFER, light_cache);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(gpu_lamp_t) * light_cache_size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		//glGenBuffers(1, &cluster_ubo);
		//glBindBuffer(GL_UNIFORM_BUFFER, cluster_ubo);
		//glBufferData(GL_UNIFORM_BUFFER, sizeof(int) * 4 * cluster_texture_resolutions[renderer.selected_resolution][0] * cluster_texture_resolutions[renderer.selected_resolution][1] * CLUSTER_Z_DIVS, NULL, GL_DYNAMIC_DRAW);
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);
		//printf("%x\n", glGetError());
		
	}
	
	glGenTextures(1, &cluster_texture);
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
	
	//while(glGetError() != GL_NO_ERROR);
	
	glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE32UI_EXT, cluster_texture_resolutions[renderer.selected_resolution][0], cluster_texture_resolutions[renderer.selected_resolution][1], CLUSTER_Z_DIVS, 0, GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_INT, NULL);
	
	//printf("%x\n", glGetError());
	
	glBindTexture(GL_TEXTURE_3D, 0);
	
	
	
	clusters = (unsigned int *)malloc(sizeof(int) * cluster_texture_resolutions[renderer.selected_resolution][0] * cluster_texture_resolutions[renderer.selected_resolution][1] * CLUSTER_Z_DIVS);
	
	
	ssm_cur_width = SHADOW_MAP_MIN_RES * 64;
 	ssm_cur_height = SHADOW_MAP_MIN_RES * 64;
 	
 	//ssm_cur_width = SHADOW_MAP_TEXTURE_CIELING;
 	//ssm_cur_height = SHADOW_MAP_TEXTURE_CIELING;
 	
 	glGenTextures(1, &ssm);
	glBindTexture(GL_TEXTURE_2D, ssm);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
 	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ssm_cur_width, ssm_cur_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
 	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	light_a.shadow_data=NULL;
	light_a.position_data=NULL;
	light_a.extra_data=NULL;
	light_a.params=NULL;
	light_a.light_count=0;
	light_a.stack_top = -1;
	light_ResizeLightArray(&light_a, MAX_ACTIVE_LIGHTS);
	
	//affecting_lights.lights = NULL;
	//affecting_lights.count = 0;
	//light_ResizeAffectingLightList(16);
	
	active_light_a.shadow_data=NULL;
	active_light_a.position_data=NULL;
	active_light_a.extra_data=NULL;
	active_light_a.params=NULL;
	active_light_a.light_count=0;
	light_ResizeLightArray(&active_light_a, MAX_ACTIVE_LIGHTS);
	
	//active_light_indexes = (int *)malloc(sizeof(int) * MAX_ACTIVE_LIGHTS);
	active_light_transforms = (mat4_t *)malloc(sizeof(mat4_t) * MAX_ACTIVE_LIGHTS);
	
	
	
	free_chunk_count = 1;
	free_chunk_size = 128;
	free_chunks = (ks_chunk_t *)malloc(sizeof(ks_chunk_t) * free_chunk_size);
	free_chunks[0].x = 0;
	free_chunks[0].y = 0;
	free_chunks[0].w = ssm_cur_width;
	free_chunks[0].h = ssm_cur_height;
	
	
	alloc_chunk_count = 0;
	alloc_chunk_size = 128;
	alloc_chunks = (ks_chunk_t *)malloc(sizeof(ks_chunk_t) * alloc_chunk_size);
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), -0.5, 1);
	mat4_t_compose(&cube_shadow_mats[0], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[0]);
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 0.5, 1);
	mat4_t_compose(&cube_shadow_mats[1], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[1]);
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(1.0, 0.0, 0.0), 0.5, 1);
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 1.0, 0);
	mat4_t_compose(&cube_shadow_mats[2], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[2]);
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(1.0, 0.0, 0.0), -0.5, 1);
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 1.0, 0);
	mat4_t_compose(&cube_shadow_mats[3], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[3]);
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 1.0, 1);
	mat4_t_compose(&cube_shadow_mats[5], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[5]);
	
	
	
	m=mat3_t_id();
	mat4_t_compose(&cube_shadow_mats[4], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[4]);
	
}

void light_Finish()
{
	free(light_a.shadow_data);
	free(light_a.position_data);
	free(light_a.extra_data);
	free(light_a.params);
	
	free(active_light_a.shadow_data);
	free(active_light_a.position_data);
	free(active_light_a.extra_data);
	free(active_light_a.params);
	
	//free(active_light_indexes);
	free(active_light_transforms);
	
	glDeleteTextures(1, &ssm);
	
	free(alloc_chunks);
	free(free_chunks);
	
	return;
}

void light_ResizeLightArray(light_array *larray, int new_size)
{
	/* TODO: aligned allocation (using a single big chunk of memory maybe?)... */
	//light_world_data *wtemp=calloc(new_size, sizeof(light_world_data));
	//light_local_data *ltemp=calloc(new_size, sizeof(light_local_data));
	light_data2 *stemp=(light_data2 *)malloc(new_size * sizeof(light_data2));
	light_data0 *ctemp=(light_data0 *)malloc(new_size * sizeof(light_data0));
	light_data3 *etemp=(light_data3 *)malloc(new_size * sizeof(light_data3));
	light_data1 *ptemp=(light_data1 *)malloc(new_size * sizeof(light_data1));
	int *temp = (int *)malloc(sizeof(int) * new_size);
	
	if(larray->position_data)
	{
		//memcpy(wtemp, larray->world_data, sizeof(light_world_data)*larray->light_count);
		//memcpy(ltemp, larray->local_data, sizeof(light_local_data)*larray->light_count);
		memcpy(stemp, larray->shadow_data, sizeof(light_data2) * larray->array_size);
		memcpy(ctemp, larray->position_data, sizeof(light_data0) * larray->array_size);
		memcpy(etemp, larray->extra_data, sizeof(light_data3) * larray->array_size);
		memcpy(ptemp, larray->params, sizeof(light_data1) * larray->array_size);
		
		//free(larray->world_data);
		//free(larray->local_data);
		free(larray->shadow_data);
		free(larray->position_data);
		free(larray->extra_data);
		free(larray->params);
		free(larray->free_stack);
	}
	
	//larray->world_data=wtemp;
	//larray->local_data=ltemp;
	larray->shadow_data = stemp;
	larray->position_data = ctemp;
	larray->extra_data = etemp;
	larray->params = ptemp;
	larray->array_size = new_size;
	larray->free_stack = temp;
	
	//free(active_light_transforms);
	return;
	
}

void light_ResizeAffectingLightList(int new_size)
{
	affecting_lights_t *a = (affecting_lights_t *)malloc(sizeof(affecting_lights_t) * new_size);
	if(affecting_lights.lights)
	{
		memcpy(a, affecting_lights.lights, sizeof(affecting_lights_t) * affecting_lights.size);
		free(affecting_lights.lights);
	}
	affecting_lights.lights = a;
	affecting_lights.size = new_size;
}

void light_FitLights()
{
	int i;
	int c;
	int j;
	int k;
	int m;

	short ah;
	short bw;
	
	short a0;
	short a1;
	short b0;
	short b1;
	
	short w;
	short h;
	short temp0;
	short temp1;
	short old_w;
	short old_h;
	short old_x;
	short old_y;
	float r[2];
	short aw[2];
	short bh[2];
	int prev;
	int next;
	light_data2 *light;
	light_data0 *light1;
	//int last = free_chunk_count;
	
	//if(w < 1 || h < 1) return -1;
	

	k = active_light_a.light_count;
	w = 0;
	h = 0;
	/*for(m = 0; m < k; m++)
	{
		light1 = &active_light_a.position_data[m];
		light = &active_light_a.shadow_data[m];
		
		if(!(light1->bm_flags & LIGHT_GENERATE_SHADOWS)) continue;
		
		if(light1->bm_flags & LIGHT_POINT)
		{
			w += light->cur_res * 3;
			h += light->cur_res * 2;
		}
		else
		{
			w += light->cur_res;
			h += light->cur_res;
		}
	}
	
	
	light_ResizeShadowMemory(w, h);*/
	
	
	free_chunk_count = 1;
	free_chunks[0].x = 0;
	free_chunks[0].y = 0;
	free_chunks[0].w = ssm_cur_width;
	free_chunks[0].h = ssm_cur_height;
	alloc_chunk_count = 0;
	
	
	for(m = 0; m < k; m++)
	{
		light1 = &active_light_a.position_data[m];
		light = &active_light_a.shadow_data[m];
		
		if(!(light1->bm_flags & LIGHT_GENERATE_SHADOWS)) continue;
		
		w = light->cur_res;
		h = light->cur_res;
		
		if(light1->bm_flags & LIGHT_POINT)
		{
			w *= 3;
			h *= 2;
		}
		c = free_chunk_count;
		
		for(i = 0; i < c; i++)
		{
			if(w <= free_chunks[i].w)
			{
				if(h <= free_chunks[i].h)
				{
					alloc_chunks[alloc_chunk_count].x = free_chunks[i].x;
					alloc_chunks[alloc_chunk_count].y = free_chunks[i].y;
					alloc_chunks[alloc_chunk_count].w = w;
					alloc_chunks[alloc_chunk_count].h = h;
					
					light->x = alloc_chunks[alloc_chunk_count].x;
					light->y = alloc_chunks[alloc_chunk_count].y;
					light->w = alloc_chunks[alloc_chunk_count].w;
					light->h = alloc_chunks[alloc_chunk_count].h;
					
					//printf("%d %d %d %d\n", light->x, light->y, light->w, light->h);
					
					//light->shadow_map_index = active_shadow_map;
					
					
					/*printf("[x: %d  y: %d  w: %d  h: %d]\n\n", shadow_maps[active_shadow_map].alloc_chunks[shadow_maps[active_shadow_map].alloc_chunk_count].x,
															 shadow_maps[active_shadow_map].alloc_chunks[shadow_maps[active_shadow_map].alloc_chunk_count].y,
															 shadow_maps[active_shadow_map].alloc_chunks[shadow_maps[active_shadow_map].alloc_chunk_count].w,
															 shadow_maps[active_shadow_map].alloc_chunks[shadow_maps[active_shadow_map].alloc_chunk_count].h);*/
					
					
					alloc_chunk_count++;				
					
					/* the allocation fits exactly within this chunk. */
					if(w == free_chunks[i].w && h == free_chunks[i].h)
					{
	
						if(free_chunk_count > 1)
						{
							free_chunks[i] = free_chunks[free_chunk_count - 1];
						}
						free_chunk_count--;
						//return;
						break;
						
					}
					
					/* this allocation have one of its dimensions equal 
					to the free chunk, so just shrink it */
					else if(w == free_chunks[i].w)
					{
						free_chunks[i].y += h;
						free_chunks[i].h -= h;
						continue;
						//return;
					}
					else if(h == free_chunks[i].h)
					{
						free_chunks[i].x += w;
						free_chunks[i].w -= w;
						break;
						//return;
					}
					
					old_w = free_chunks[i].w;
					old_h = free_chunks[i].h;
					old_x = free_chunks[i].x;
					old_y = free_chunks[i].y;
					
					/* The requested block has both its dimensions smaller than
					the free block. This means that a new block will be added,
					and the old one will be modified. */
					
					/* the two combinations for each chunk after the cut */
					aw[0] = w;
					aw[1] = old_w;
					ah = old_h - h;
					
					bh[0] = old_h;
					bh[1] = h;
					bw = old_w - w;
					
					/* here, a0 ,a1, b0 and b1
					are swapped to make sure that
					a1 > a0 and b0 > b1, so
					the relation a1b0 > a0b1
					holds. */
					for(j = 0; j < 2; j++)
					{
						a0 = ah;
						a1 = aw[j];
						/* this swap could be made faster if the
						rotate instruction was used, since those are 
						16 bits figures...  */
						if(a1 > a0)
						{
							temp0 = a1;
							a1 = a0;
							a0 = temp0;
						}
						
						b0 = bw;
						b1 = bh[j];
						if(b0 > b1)
						{
							temp0 = b0;
							b0 = b1;
							b1 = temp0;
						}
						/* calculate the ratio for both configurations... */
						r[j] = (float)(a0*b1)/(float)(a1*b0);
					}
					
					free_chunks[i].h = ah;
					free_chunks[i].y = old_y + h;
					free_chunks[i].x = old_x;
					free_chunks[free_chunk_count].w = bw;
					free_chunks[free_chunk_count].y = old_y;
					free_chunks[free_chunk_count].x = old_x + w;
					
					/* and use the one that results in the 
					smallest ratio between the ratio of the 
					chunks' sides. */
					if(r[0] < r[1])
					{
						free_chunks[i].w = aw[0];
						free_chunks[free_chunk_count].h = bh[0];
					}
					else
					{
						free_chunks[i].w = aw[1];
						free_chunks[free_chunk_count].h = bh[1];
					}
					free_chunk_count++;
					break;
					//return;
					
				}
			}
		}
	}
		
}

void light_ResizeShadowMemory(int width, int height)
{
	
	if(width > SHADOW_MAP_TEXTURE_CIELING) width = SHADOW_MAP_TEXTURE_CIELING;
	else if(width < SHADOW_MAP_MIN_RES) width = SHADOW_MAP_MIN_RES;
	
	if(height > SHADOW_MAP_TEXTURE_CIELING) height = SHADOW_MAP_TEXTURE_CIELING;
	else if(height < SHADOW_MAP_MIN_RES) height = SHADOW_MAP_MIN_RES;
	
	width = (width + SHADOW_MAP_MIN_RES - 1) & (~(SHADOW_MAP_MIN_RES - 1));
	height = (height + SHADOW_MAP_MIN_RES - 1) & (~(SHADOW_MAP_MIN_RES - 1));
	
	//printf("w: %d  h: %d\n", ssm_cur_width, ssm_cur_height);
	
	if((ssm_cur_width > width && ssm_cur_height > height) || 
	   (ssm_cur_width == width && ssm_cur_height > height) ||
	   (ssm_cur_width > width && ssm_cur_height == height))
	{
		//if(!frames)
		//{
		shadow_memory_update_delay++;	
		//}
	}
	else if(ssm_cur_width < width || ssm_cur_height < height)
	{
		shadow_memory_update_delay = 150;
	}
	
	/* delay physical shadow map resizing when
	the requested size is smaller then the current... */
	if(shadow_memory_update_delay >= 150)
	{
		shadow_memory_update_delay = 0;
		ssm_cur_width = width;
		ssm_cur_height = height;
		//printf("w: %d  h: %d\n", ssm_cur_width, ssm_cur_height);
		glBindTexture(GL_TEXTURE_2D, ssm);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
}

PEWAPI int light_CreateLight(char *name, int bm_flags, vec4_t position, mat3_t *orientation, vec3_t diffuse_color, float radius, float energy, float spot_angle, float spot_blend, float lin_fallof, float sqrd_fallof, float scattering, int volume_samples,  int shadow_map_res, int max_shadow_aa_samples, int tex_index)
{
	int light_index;
	light_data0 *position_data;
	light_data1 *params;
	light_data2 *shadow_data;
	light_data3 *extra_data;
	light_ptr light;
	int c;
	float frustum_angle;
	
	
	
	if(light_a.stack_top >= 0)
	{
		light_index = light_a.free_stack[light_a.stack_top--];
	}
	else
	{
		if(light_index >= light_a.array_size)
		{
			light_ResizeLightArray(&light_a, light_a.array_size<<1);
		}
		
		light_index = light_a.light_count++;
	}
	
	
	
	
	position_data = &light_a.position_data[light_index];
	params = &light_a.params[light_index];
	shadow_data = &light_a.shadow_data[light_index];
	extra_data = &light_a.extra_data[light_index];
	
	memcpy(&position_data->world_orientation, orientation, sizeof(mat3_t));
	memcpy(&position_data->local_orientation, orientation, sizeof(mat3_t));
	memcpy(&position_data->world_position, &position, sizeof(vec4_t));
	memcpy(&position_data->local_position, &position, sizeof(vec4_t));
	//light_a.position_data[light_index].world_position = position;
	//light_a.position_data[light_index].local_position = position;
	position_data->tex_index = tex_index;
	
	if(radius > LIGHT_MAX_RADIUS) radius = LIGHT_MAX_RADIUS;
	else if(radius < LIGHT_MIN_RADIUS) radius = LIGHT_MIN_RADIUS;
	
	position_data->light_index = light_index;
	position_data->radius = radius;
	position_data->bm_flags = bm_flags;
	//light_a.position_data[light_index].bm_state = LIGHT_HAS_MOVED;
	position_data->spot_co = (char)spot_angle;
	
	if(spot_blend > 1.0) spot_blend = 1.0;
	else if(spot_blend < 0.0) spot_blend = 0.0;
	
	params->spot_e = (unsigned char)0xff * spot_blend;
	params->max_shadow_aa_samples = max_shadow_aa_samples;
	
	if(shadow_map_res > MAX_SHADOW_MAP_RES) shadow_map_res = MAX_SHADOW_MAP_RES;
	else if(shadow_map_res < MIN_SHADOW_MAP_RES) shadow_map_res = MIN_SHADOW_MAP_RES;
	shadow_map_res = (shadow_map_res + MIN_SHADOW_MAP_RES - 1) & (~(MIN_SHADOW_MAP_RES - 1));
	params->shadow_map_res = shadow_map_res / MIN_SHADOW_MAP_RES;
	
	shadow_data->max_res = shadow_map_res;
	shadow_data->min_res = MIN_SHADOW_MAP_RES;
	shadow_data->cur_res = shadow_data->max_res;
	
	
	//printf("%d %d\n", shadow_data->max_res, shadow_data->min_res);
	
	
	if(volume_samples > MAX_VOLUME_SAMPLES) volume_samples = MAX_VOLUME_SAMPLES;
	else if(volume_samples < MIN_VOLUME_SAMPLES) volume_samples = MIN_VOLUME_SAMPLES;
	params->volume_samples = volume_samples;
	
	/*if(max_shadow_map_res < 0) max_shadow_map_res = MIN_SHADOW_MAP_RES;
	else if(max_shadow_map_res > MAX_SHADOW_MAP_RES) max_shadow_map_res = MAX_SHADOW_MAP_RES;
	
	if(min_shadow_map_res < 0) min_shadow_map_res = MIN_SHADOW_MAP_RES;
	else if(min_shadow_map_res > MAX_SHADOW_MAP_RES) min_shadow_map_res = MAX_SHADOW_MAP_RES;*/
	
	/*if(min_shadow_map_res > max_shadow_map_res)
	{
		c = min_shadow_map_res;
		min_shadow_map_res = max_shadow_map_res;
		max_shadow_map_res = c;
	}*/
	
	
	//min_shadow_map_res = (min_shadow_map_res + MIN_SHADOW_MAP_RES - 1) & (~(MIN_SHADOW_MAP_RES - 1));
	
	
	//light_a.params[light_index].min_shadow_map_res = min_shadow_map_res / MIN_SHADOW_MAP_RES;
	
	
	/*if(max_samples < 0) max_samples = MIN_VOLUME_SAMPLES;
	else if(max_samples > MAX_VOLUME_SAMPLES) max_samples = MAX_VOLUME_SAMPLES;
	
	if(min_samples < 0) min_samples = MIN_VOLUME_SAMPLES;
	else if(min_samples > MAX_VOLUME_SAMPLES) min_samples = MAX_VOLUME_SAMPLES;
	
	if(min_samples > max_samples)
	{
		c = min_samples;
		min_samples = max_samples;
		max_samples = c;
	}*/
	
	if(bm_flags&LIGHT_POINT)
	{
			
		//position_data->local_position.floats[3]=1.0;
		position_data->local_position.floats[3] = 1.0;
		/*if(bm_flags & LIGHT_GENERATE_SHADOWS)
		{
			shadow_data->shadow_map = light_CreateShadowCubeMap(shadow_map_res);
		}*/
			
		frustum_angle=45.0;		
		position_data->spot_co = 0;
		//position_data->spot_co=0;
	}
	else if(bm_flags&LIGHT_SPOT)
	{
		//position_data->local_position.floats[3]=1.0;
		position_data->local_position.floats[3] = 1.0;
		/*if(bm_flags&LIGHT_GENERATE_SHADOWS)
		{
			shadow_data->shadow_map=light_CreateShadowMap(shadow_map_res);
		}*/
			
		frustum_angle = (float)position_data->spot_co;
	}
	else
	{
		position_data->local_position.floats[3] = 0.0;
	}
	
	
		

		
	CreatePerspectiveMatrix(&extra_data->light_projection_matrix, DegToRad(frustum_angle), 1.0, LIGHT_ZNEAR, radius / LIGHT_ZNEAR, &extra_data->generated_frustum);
	
	/*if(bm_flags & LIGHT_POINT)
	{
		light_CalculatePointLightFrustums(&light_a.extra_data[light_index].generated_frustums[0], light_a.extra_data[light_index].generated_frustums);
	}*/
	
	shadow_data->znear = extra_data->generated_frustum.znear;
	shadow_data->zfar = extra_data->generated_frustum.zfar;
	
	/* depth imprecision was causing shadow artifacts. The solution was to kick the far clipping plane WAY farther, hence the
	division by LIGHT_ZNEAR (which is smaller than one). Here zfar will be multiplied by LIGHT_ZNEAR to retrieve the correct value,
	necessary to perform accurate frustum culling. */
	extra_data->generated_frustum.zfar *= LIGHT_ZNEAR;
	
	//light_a.params[light_index].diffuse = material_FloatToColor4_t(diffuse_color.floats[0], diffuse_color.floats[1], diffuse_color.floats[2], 1.0);
	params->r = 255 * diffuse_color.r;
	params->g = 255 * diffuse_color.g;
	params->b = 255 * diffuse_color.b;
	//light_a.params[light_index].specular = material_FloatToColor4_t(specular_collor.floats[0], specular_collor.floats[1], specular_collor.floats[2], 1.0);
	
	
	if(lin_fallof > 1.0) lin_fallof = 1.0;
	else if(lin_fallof < 0.0) lin_fallof = 0.0;
	if(sqrd_fallof > 1.0) lin_fallof = 1.0;
	else if(sqrd_fallof < 0.0) lin_fallof = 0.0;
	
	/*if(sqrd_fallof < 0.003 && lin_fallof < 0.003)
	{
		lin_fallof = 1.0 / 255.0;
	}*/
	
	if(scattering > MAX_LIGHT_VOLUME_SCATTERING) scattering = MAX_LIGHT_VOLUME_SCATTERING;
	else if(scattering < MIN_LIGHT_VOLUME_SCATTERING) scattering = MIN_LIGHT_VOLUME_SCATTERING;
	
	if(energy > MAX_LIGHT_ENERGY) energy = MAX_LIGHT_ENERGY;
	else if(energy < MIN_LIGHT_ENERGY) energy = MIN_LIGHT_ENERGY;
	
	
	params->lin_fallof = 0xffff * lin_fallof;
	params->sqr_fallof = 0xffff * sqrd_fallof;
	//light_a.params[light_index].name = strdup(name);
	params->scattering = 0xffff * (scattering / MAX_LIGHT_VOLUME_SCATTERING);
	params->energy = 0xffff * (energy / MAX_LIGHT_ENERGY);
	//light_a.params[light_index].min_samples = min_samples;
	
	//light_a.params[light_index].assigned_node = scenegraph_AddNode(NODE_LIGHT, light_index, name);
	//light_a.params[light_index].bm_status = 0;
	
	extra_data->name = strdup(name);
	extra_data->assigned_node = scenegraph_AddNode(NODE_LIGHT, light_index, -1, name);
	
	light.position_data = position_data;
	light.extra_data = extra_data;
	light.params = params;
	light.shadow_data = shadow_data;
	
	light_CacheGPULight(light);

	//light_a.light_count++;

	return light_index;
}


PEWAPI void light_UpdateLightFrustum(light_ptr light)
{
	float frustum_angle;
	float radius;
	if(light.position_data->bm_flags & LIGHT_GENERATE_SHADOWS)
	{
		if(light.position_data->bm_flags & LIGHT_POINT)
		{
			frustum_angle = 45.0;
		}
		else
		{
			frustum_angle = (float)light.position_data->spot_co;
		}
		
		radius = light.position_data->radius;
		CreatePerspectiveMatrix(&light.extra_data->light_projection_matrix, DegToRad(frustum_angle), 1.0, LIGHT_ZNEAR, radius / LIGHT_ZNEAR, &light.extra_data->generated_frustum);
		
		light.shadow_data->znear = light.extra_data->generated_frustum.znear;
		light.shadow_data->zfar = light.extra_data->generated_frustum.zfar;
		light.extra_data->generated_frustum.zfar *= LIGHT_ZNEAR;
	}
	
}


PEWAPI void light_DestroyLight(light_ptr light)
{
	int light_index;
	if(light.position_data)
	{
		light_index = light.position_data->light_index;
		
		light_a.stack_top++;
		light_a.free_stack[light_a.stack_top] = light_index;
		
		/*if(light_a.position_data[light_index].bm_flags & LIGHT_GENERATE_SHADOWS)
		{
			light_DestroyShadowMap(&light_a.shadow_data[light_index].shadow_map);
		}*/
		
		light.position_data->light_index = -1;
		light_DropGPULight(light);
		
		scenegraph_RemoveNode(light.extra_data->assigned_node, 0);
		
	}
}

PEWAPI void light_DestroyLightByIndex(int light_index)
{
	
}

smap_t light_CreateShadowMap(int resolution)
{
	//while(glGetError()!=GL_NO_ERROR);
	smap_t smap;
	unsigned int temp;
	//glGenFramebuffers(1, &smap.shadow_fb);
 	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, smap.shadow_fb);
 	
	glGenTextures(1, &temp);
	smap.shadow_map = temp;
	//glGenTextures(1, &smap.alpha_map);
	glBindTexture(GL_TEXTURE_2D, smap.shadow_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
 	
 	
	/*glBindTexture(GL_TEXTURE_2D, smap.alpha_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R, resolution, resolution, 0, GL_R, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);*/	


 	glBindTexture(GL_TEXTURE_2D, 0);
 	smap.resolution=resolution;
 	return smap;
}

smap_t light_CreateShadowCubeMap(int resolution)
{
	
	/* for cube shadows, each face has to be bound to the framebuffer right before rendering the 
	shadow map. That's why no more initialisation is done here. */
	smap_t smap;
	int i;
	int j;
	unsigned int temp;
	
	smap.resolution=resolution;
	//glEnable(GL_TEXTURE_CUBE_MAP);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_buffer.id);
	glGenTextures(1, &temp);
	smap.shadow_map = temp;
	glBindTexture(GL_TEXTURE_CUBE_MAP, smap.shadow_map);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
 	
	for(i=0; i<6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, smap.resolution, smap.resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, smap.shadow_map, 0);
	//	glClear(GL_DEPTH_BUFFER_BIT);
	}
	
	
	/*glGenTextures(1, &smap.alpha_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, smap.alpha_map);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
 	
	for(i=0; i<6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB16F, GL_FLOAT, NULL);
	}*/
	
	
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return smap;
	
	
}

void light_DestroyShadowMap(smap_t *shadow_map)
{
	unsigned int id;
	if(shadow_map)
	{
		id = shadow_map->shadow_map;
		glDeleteTextures(1, &id);
	}
}


void light_SortLights()
{
	return;
}

void light_UploadLightIndexes()
{
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	
	glBindTexture(GL_TEXTURE_3D, 0);
	
	//glUniform1iv(shader_a.shaders[renderer.active_shader_index].sysLightIndexes, active_light_a.light_count, active_light_indexes);
	//glUniform1i(shader_a.shaders[renderer.active_shader_index].sysLightCount, active_light_a.light_count);
}

void light_UploadLightTransforms()
{
	int i;
	int c = active_light_a.light_count;
	int light_index;
	gpu_lamp_t *lamp;
	
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache);
	lamp = (gpu_lamp_t *)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);

	
	for(i = 0; i < c; i++)
	{
		light_index = active_light_a.position_data[i].cache_index;
		//memcpy(&lamp[light_index], &active_light_transforms[i], sizeof(mat4_t));
		
		/* Updating this cache every frame could lead to bad performance... */
		
		
		lamp[light_index].sysLightRightVector.x = active_light_transforms[i].floats[0][0];
		lamp[light_index].sysLightRightVector.y = active_light_transforms[i].floats[0][1];
		lamp[light_index].sysLightRightVector.z = active_light_transforms[i].floats[0][2];
		lamp[light_index].sysLightRightVector.w = active_light_transforms[i].floats[0][3];
		
		lamp[light_index].sysLightUpVector.x = active_light_transforms[i].floats[1][0];
		lamp[light_index].sysLightUpVector.y = active_light_transforms[i].floats[1][1];
		lamp[light_index].sysLightUpVector.z = active_light_transforms[i].floats[1][2];
		lamp[light_index].sysLightUpVector.w = active_light_transforms[i].floats[1][3];
		
		lamp[light_index].sysLightForwardVector.x = active_light_transforms[i].floats[2][0];
		lamp[light_index].sysLightForwardVector.y = active_light_transforms[i].floats[2][1];
		lamp[light_index].sysLightForwardVector.z = active_light_transforms[i].floats[2][2];
		lamp[light_index].sysLightForwardVector.w = active_light_transforms[i].floats[2][3];
		
		lamp[light_index].sysLightPosition.x = active_light_transforms[i].floats[3][0];
		lamp[light_index].sysLightPosition.y = active_light_transforms[i].floats[3][1];
		lamp[light_index].sysLightPosition.z = active_light_transforms[i].floats[3][2];
		lamp[light_index].sysLightPosition.w = active_light_transforms[i].floats[3][3];
		
		
		lamp[light_index].sysLightModelViewProjectionMatrix = active_light_a.shadow_data[i].model_view_projection_matrix;
		lamp[light_index].sysLightShadowX = (float)active_light_a.shadow_data[i].x / (float)ssm_cur_width;
		lamp[light_index].sysLightShadowY = (float)active_light_a.shadow_data[i].y / (float)ssm_cur_height;
		lamp[light_index].sysLightShadowSize = (float)active_light_a.shadow_data[i].w / (float)ssm_cur_width;
		
		
		//printf("%f %f %f\n", lamp[light_index].sysLightShadowX, lamp[light_index].sysLightShadowY, lamp[light_index].sysLightShadowSize);
		
		
		/* size * 3 for point lights... */
		//lamp[light_index].sysLightShadowSize = (float)light.shadow_data->w;
		
		
		/*printf("%f %f %f %f\n", lamp[light_index].sysLightPosition.x, 
								lamp[light_index].sysLightPosition.y,
								lamp[light_index].sysLightPosition.z,
								lamp[light_index].sysLightPosition.w);*/
	}
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
}

void light_AssignLightsToClusters()
{
	int i;
	int c = active_light_a.light_count;
	camera_t *active_camera = camera_GetActiveCamera();
	
	int j;
	
	int x;
	int y;
	int z;
	
	//mat4_t *projection_matrix = &active_camera->projection_matrix;
	
	vec4_t corners[8];
	vec3_t light_origin;
	vec3_t cam_vec;
	vec3_t p;
	vec2_t ac;
	vec2_t lb;
	vec2_t rb;
	
	float light_radius;
	
	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
	float ntop = active_camera->frustum.top;
	float nright = active_camera->frustum.right;
	
	float x_max;
	float x_min;
	float y_max;
	float y_min;
	float d;
	float t;
	float denom;
	float si;
	float co;
	float k;
	float l;
	
	int cluster_min_x;
	int cluster_min_y;
	int cluster_min_z;
	int cluster_max_x;
	int cluster_max_y;
	int cluster_max_z;
	
	int layer_size = cluster_texture_resolutions[renderer.selected_resolution][1] * cluster_texture_resolutions[renderer.selected_resolution][0];
	int row_size = cluster_texture_resolutions[renderer.selected_resolution][0];
		
	
	denom = log(1.0 + (2.0 * tan(0.68)) / CLUSTER_SIZE);
	
	
	for(z = 0; z < CLUSTER_Z_DIVS; z++)
	{
		for(y = 0; y < cluster_texture_resolutions[renderer.selected_resolution][1]; y++)
		{
			for(x = 0; x < cluster_texture_resolutions[renderer.selected_resolution][0]; x++)
			{
				clusters[z * layer_size + y * row_size + x] = 0;
			}
				
		}
	}
	
	
	/*for(y = 0; y < cluster_texture_resolutions[renderer.selected_resolution][1]; y++)
	{
		for(x = 0; x < cluster_texture_resolutions[renderer.selected_resolution][0]; x++)
		{
			clusters[y * row_size + x] = 0;
		}
			
	}*/

	
	
	
	
	for(i = 0; i < c; i++)
	{
		
		//light_origin = active_light_a.position_data[i].world_position.vec3;
		light_origin = vec3(active_light_transforms[i].floats[3][0], active_light_transforms[i].floats[3][1], active_light_transforms[i].floats[3][2]);
		light_radius = active_light_a.position_data[i].radius;
		
		/* light is completely behind near plane, so don't
		to anything (this case is already handled by the
		frustum culling code, but whatever...) */
		/*if(light_origin.z - light_radius > nznear)
		{
			
		}
		else*/
		
			
			//if(dot3(light_origin, light_origin) >= light_radius * light_radius)
			//{
		ac.x = light_origin.x;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
		l = light_radius * light_radius;
		k = nznear - ac.y;
		k = sqrt(light_radius * light_radius - k * k);
				
		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);
					
					
			si = light_radius / d;
			co = t / d;	
					
			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;
					
					
			if(rb.y > nznear)
			{				
				rb.x = ac.x + k;
				rb.y = nznear;
			}
					
			if(lb.y > nznear)
			{				
				lb.x = ac.x - k;
				lb.y = nznear;
			}
					
			x_min = ((nznear / nright) * lb.x) / lb.y;
			x_max = ((nznear / nright) * rb.x) / rb.y;
					
			if(x_min < -1.0) x_min = -1.0;
			if(x_max > 1.0) x_max = 1.0;
				
					
		}
		else
		{
			x_min = -1.0;
			x_max = 1.0;
		}
				
				
			/*	draw_debug_DrawPoint(vec3(x_min, 0.0, -0.5), vec3(0.0, 1.0, 0.0), 12.0, 1);
				draw_debug_DrawPoint(vec3(x_max, 0.0, -0.5), vec3(0.0, 1.0, 0.0), 12.0, 1);*/
				
				
				
				
		ac.x = light_origin.y;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
				//l = light_radius * light_radius;
				
		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);
					//k = nznear - ac.y;
					//k = sqrt(light_radius * light_radius - k * k);
					
			si = light_radius / d;
			co = t / d;	
					
			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;
					
					
			if(rb.y > nznear)
			{				
				rb.x = ac.x + k;
				rb.y = nznear;
			}
					
			if(lb.y > nznear)
			{				
				lb.x = ac.x - k;
				lb.y = nznear;
			}
					
			y_min = ((nznear / ntop) * lb.x) / lb.y;
			y_max = ((nznear / ntop) * rb.x) / rb.y;
					
			if(y_min < -1.0) y_min = -1.0;
			if(y_max > 1.0) y_max = 1.0;
				
				
		}
		else
		{
			y_min = -1.0;
			y_max = 1.0;
		}
			
			
		/*draw_debug_DrawPoint(vec3(x_min, y_max, -0.5), vec3(0.0, 0.0, 1.0), 12.0, 1);
		draw_debug_DrawPoint(vec3(x_min, y_min, -0.5), vec3(0.0, 0.0, 1.0), 12.0, 1);
		draw_debug_DrawPoint(vec3(x_max, y_min, -0.5), vec3(0.0, 0.0, 1.0), 12.0, 1);
		draw_debug_DrawPoint(vec3(x_max, y_max, -0.5), vec3(0.0, 0.0, 1.0), 12.0, 1);*/
		/*draw_debug_DrawLine(vec3(x_min, y_max, -0.25), vec3(x_min, y_min, -0.25), vec3(0.0, 1.0, 0.0), 1.0, 0, 1, 0);
		draw_debug_DrawLine(vec3(x_min, y_min, -0.25), vec3(x_max, y_min, -0.25), vec3(0.0, 1.0, 0.0), 1.0, 0, 1, 0);
		draw_debug_DrawLine(vec3(x_max, y_min, -0.25), vec3(x_max, y_max, -0.25), vec3(0.0, 1.0, 0.0), 1.0, 0, 1, 0);
		draw_debug_DrawLine(vec3(x_max, y_max, -0.25), vec3(x_min, y_max, -0.25), vec3(0.0, 1.0, 0.0), 1.0, 0, 1, 0);*/
			
			
		x_min = x_min * 0.5 + 0.5;
		y_min = y_min * 0.5 + 0.5;
		
		x_max = x_max * 0.5 + 0.5;
		y_max = y_max * 0.5 + 0.5;
			
			
		cluster_min_x = (renderer.width * x_min) / CLUSTER_SIZE;
		cluster_min_y = (renderer.height * y_min) / CLUSTER_SIZE;
			
		cluster_max_x = (renderer.width * x_max) / CLUSTER_SIZE;
		cluster_max_y = (renderer.height * y_max) / CLUSTER_SIZE;
			
		cluster_min_z = int(log(-(light_origin.z + light_radius) / -nznear) / denom) / CLUSTER_Z_DIVS;
		cluster_max_z = int(log(-(light_origin.z - light_radius) / -nznear) / denom) / CLUSTER_Z_DIVS;
			
		if(cluster_min_z < 0) cluster_min_z = 0;
		if(cluster_max_z > CLUSTER_Z_DIVS) cluster_max_z = CLUSTER_Z_DIVS;
			
		//printf("%d %d\n", cluster_min_z, cluster_max_z);
		
		j = 1 << active_light_a.position_data[i].cache_index;
			
		for(z = cluster_min_z; z <= cluster_max_z; z++)
		{	
			for(y = cluster_min_y; y <= cluster_max_y; y++)
			{
				
				
				for(x = cluster_min_x; x <= cluster_max_x; x++)
				{
					clusters[z * layer_size + y * row_size + x] |= j;
				}
			}
		}
			
			//printf("%d %d\n", cluster_min_z, cluster_max_z);
			
			
		/*for(y = cluster_min_y; y <= cluster_max_y; y++)
		{
			for(x = cluster_min_x; x <= cluster_max_x; x++)
			{
				clusters[y * row_size + x * 4] |= j;
			}
				
		}*/
			
			 
			
	//	}
		
	}
	
	//while(glGetError() != GL_NO_ERROR);
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	//glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, cluster_texture_resolutions[renderer.selected_resolution][0], 
											   cluster_texture_resolutions[renderer.selected_resolution][1], 
											   CLUSTER_Z_DIVS, GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_INT, clusters);
											
	//glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);										
	glBindTexture(GL_TEXTURE_3D, 0);
	
	//printf("%x\n", glGetError());											
	
}

PEWAPI void light_BindLightCache()
{
	//glUniformBlockBinding(shader_a.shaders[renderer.active_shader_index].shader_ID, 1, LIGHT_PARAMS_BINDING);
	//glBindBuffer(GL_UNIFORM_BUFFER, light_cache);
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_PARAMS_BINDING, light_cache);
}

PEWAPI void light_UnbindLightCache()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_PARAMS_BINDING, 0);
}

PEWAPI void light_UpdateGPULight(light_ptr light)
{
	int index;
	void *p;
	gpu_lamp_t *lamp;
	
	if(!(light.position_data->bm_flags & LIGHT_CACHED))
	{
		if(free_stack_top >= 0)
		{
			index = free_stack[free_stack_top--];
		}
		else
		{
			index = cached_light_count++;
		}
		
		light.position_data->cache_index = index;
		light.position_data->bm_flags |= LIGHT_CACHED;
		
	}
	else
	{
		index = light.position_data->cache_index;
	}
	
	
	
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache);
	
	lamp = (gpu_lamp_t *)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	lamp += index;
	
	
	lamp->sysLightModelViewProjectionMatrix = light.shadow_data->model_view_projection_matrix;
	
	lamp->sysLightColor.r = (float)light.params->r / 255.0;
	lamp->sysLightColor.g = (float)light.params->g / 255.0;
	lamp->sysLightColor.b = (float)light.params->b / 255.0;
	lamp->sysLightColor.a = 1.0;
	
	lamp->sysLightLinearFallof = (float)light.params->lin_fallof/0xffff;
	lamp->sysLightSquaredFallof = (float)light.params->sqr_fallof/0xffff;

	lamp->sysLightRadius = light.position_data->radius;
	lamp->sysLightSpotCutoff = (float)light.position_data->spot_co;
	lamp->sysLightSpotCosCutoff = cos(DegToRad(((float)light.position_data->spot_co)));	
	lamp->sysLightSpotBlend = (float)light.params->spot_e / 255.0;
	
	lamp->sysLightShadowX = (float)light.shadow_data->x / (float)ssm_cur_width;
	lamp->sysLightShadowY = (float)light.shadow_data->y / (float)ssm_cur_height;
	
	/* size * 3 for point lights... */
	lamp->sysLightShadowSize = (float)light.shadow_data->w / (float)ssm_cur_width;
	
	lamp->sysLightType = light.position_data->bm_flags & (LIGHT_POINT | LIGHT_SPOT | LIGHT_DIRECTIONAL);
	
	lamp->sysLightZNear = light.shadow_data->znear;
	lamp->sysLightZFar = light.shadow_data->zfar;
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void light_CacheGPULight(light_ptr light)
{
	int index;
	void *p;
	gpu_lamp_t *lamp;
	if(free_stack_top >= 0)
	{
		index = free_stack[free_stack_top--];
	}
	else
	{
		index = cached_light_count++;
	}
	
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache);
	lamp = (gpu_lamp_t *)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	lamp += index;
	//p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
	//lamp = (gpu_lamp_t *)((char *)p + sizeof(gpu_lamp_t) * index);
	
	lamp->sysLightModelViewProjectionMatrix = light.shadow_data->model_view_projection_matrix;
	
	lamp->sysLightColor.r = (float)light.params->r / 255.0;
	lamp->sysLightColor.g = (float)light.params->g / 255.0;
	lamp->sysLightColor.b = (float)light.params->b / 255.0;
	lamp->sysLightColor.a = 1.0;
	
	lamp->sysLightLinearFallof = (float)light.params->lin_fallof/0xffff;
	lamp->sysLightSquaredFallof = (float)light.params->sqr_fallof/0xffff;
	
	lamp->sysLightRadius = light.position_data->radius;
	lamp->sysLightSpotCutoff = (float)light.position_data->spot_co;
	lamp->sysLightSpotCosCutoff = cos(DegToRad(((float)light.position_data->spot_co)));	
	lamp->sysLightSpotBlend = (float)light.params->spot_e / 255.0;
	
	lamp->sysLightShadowX = (float)light.shadow_data->x / (float)ssm_cur_width;
	lamp->sysLightShadowY = (float)light.shadow_data->y / (float)ssm_cur_height;
	
	/* size * 3 for point lights... */
	lamp->sysLightShadowSize = (float)light.shadow_data->w / (float)ssm_cur_width;
	
	//printf("%d %d %d\n", lamp->sysLightShadowX, lamp->sysLightShadowY, lamp->sysLightShadowSize);
	
	
	lamp->sysLightType = light.position_data->bm_flags & (LIGHT_POINT | LIGHT_SPOT | LIGHT_DIRECTIONAL);
	
	lamp->sysLightZNear = light.shadow_data->znear;
	lamp->sysLightZFar = light.shadow_data->zfar;
	
	light.position_data->cache_index = index;
	light.position_data->bm_flags |= LIGHT_CACHED;
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
}

void light_DropGPULight(light_ptr light)
{
	
	if(light.position_data->bm_flags & LIGHT_CACHED)
	{
		free_stack_top++;
		free_stack[free_stack_top] = light.position_data->cache_index;
		light.position_data->bm_flags &= ~LIGHT_CACHED;
	}
	
}

void light_SetLightsByIndex(int *IDs, int light_count)
{
	/*register int i;
	register int c;
	float energy;
	float radius;
	float d[3];
	mat4_t m;
	mat4_t transform;
	vec3_t pos;
	c=light_count;
	
	if(c>max_lights_per_pass)
	{
		c=max_lights_per_pass;

	}
	
	glMatrixMode(GL_MODELVIEW);

	glLoadMatrixf(&camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix.floats[0][0]);

	glUniform1i(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightCount], light_count);

	for(i=0; i<c; i++)
	{

		
		d[0]=light_a.lights[IDs[i]].world_orientation.floats[0][2];
		d[1]=light_a.lights[IDs[i]].world_orientation.floats[1][2];
		d[2]=light_a.lights[IDs[i]].world_orientation.floats[2][2];
		
		glLightfv(GL_LIGHT0+i, GL_POSITION, &light_a.lights[IDs[i]].world_position.floats[0]);
		glLightfv(GL_LIGHT0+i, GL_DIFFUSE, &light_a.lights[IDs[i]].diffuse.floats[0]);
		glLightfv(GL_LIGHT0+i, GL_SPECULAR, &light_a.lights[IDs[i]].specular.floats[0]);
		glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, d);
		glLightf(GL_LIGHT0+i, GL_CONSTANT_ATTENUATION, light_a.lights[IDs[i]].const_fallof);
		glLightf(GL_LIGHT0+i, GL_LINEAR_ATTENUATION, light_a.lights[IDs[i]].lin_fallof);
		glLightf(GL_LIGHT0+i, GL_QUADRATIC_ATTENUATION, light_a.lights[IDs[i]].sqr_fallof);
		glLightf(GL_LIGHT0+i, GL_SPOT_CUTOFF, light_a.lights[IDs[i]].spot_co);
		glLighti(GL_LIGHT0+i, GL_SPOT_EXPONENT, (int)light_a.lights[IDs[i]].spot_e);

	}
	glPopMatrix();*/
	return;
}


void light_SetLights(int *IDs, int light_count)
{
	/*register int i;
	register int c;
	float energy;
	float d[3];
	mat4_t m;
	mat4_t transform;
	vec4_t pos[128];
	vec3_t dir[128];
	vec3_t color[128];
	float lfallof[128];
	float sqrdfallof[128];
	float radius[128];
	c=light_count;
	
	if(c)
	{
		for(i=0; i<c; i++)
		{
			pos[i]=MultiplyVector4(&camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix, active_light_a.lights[IDs[i]].world_position);
		}
	
		for(i=0; i<c; i++)
		{
			lfallof[i]=active_light_a.lights[IDs[i]].lin_fallof;
		}
	
		for(i=0; i<c; i++)
		{
			sqrdfallof[i]=active_light_a.lights[IDs[i]].sqr_fallof;
		}
	
		for(i=0; i<c; i++)
		{
			radius[i]=active_light_a.lights[IDs[i]].radius;
		}
	
		glUniform4fv(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightPosition], c, &pos[0].floats[0]);
		glUniform1fv(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightLinearFallof], c, lfallof);
		glUniform1fv(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightSquaredFallof], c, sqrdfallof);
		glUniform1fv(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightRadius], c, radius);
	
	}
	glUniform1i(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightCount], light_count);
	
	return;*/
}

PEWAPI void light_TranslateLight(light_ptr light, vec3_t direction, float amount, int b_set)
{
	mat4_t transform;
	if(!b_set)
	{
		light.position_data->local_position.floats[0] += direction.floats[0] * amount;
		light.position_data->local_position.floats[1] += direction.floats[1] * amount;
		light.position_data->local_position.floats[2] += direction.floats[2] * amount;
	}
	else
	{
		light.position_data->local_position.floats[0] = direction.floats[0] * amount;
		light.position_data->local_position.floats[1] = direction.floats[1] * amount;
		light.position_data->local_position.floats[2] = direction.floats[2] * amount;
	}	
}

PEWAPI void light_RotateLight(light_ptr light, vec3_t axis, float angle, int b_set)
{
	mat3_t r;
	r = light.position_data->local_orientation;
	mat3_t_rotate(&r, axis, angle, b_set);
	memcpy(&light.position_data->local_orientation, &r, sizeof(mat3_t));
}

PEWAPI light_ptr light_GetLight(char *name)
{
	register int i;
	register int c;
	c=light_a.light_count;
	light_ptr lptr={NULL, NULL, NULL};
	for(i=0; i<c; i++)
	{
		if(!strcmp(name, light_a.extra_data[i].name))
		{
			//return &light_a.lights[i];
			lptr.shadow_data=&light_a.shadow_data[i];
			lptr.position_data=&light_a.position_data[i];
			lptr.extra_data=&light_a.extra_data[i];
			lptr.params=&light_a.params[i];
			break;
		}
	}
	return lptr;
}

PEWAPI light_ptr light_GetActiveLight(int index)
{
	light_ptr lptr = {NULL, NULL, NULL, NULL};
	
	if(index >= 0 && index < active_light_a.light_count)
	{
		lptr.position_data = &active_light_a.position_data[index];
		lptr.params = &active_light_a.params[index];
		lptr.extra_data = &active_light_a.extra_data[index];
		lptr.shadow_data = &active_light_a.shadow_data[index];
	}
	
	return lptr;
}

PEWAPI light_ptr light_GetLightByIndex(int light_index)
{
	light_ptr lptr={NULL, NULL, NULL};
	if(light_index>=0)
	{
		//return &light_a.lights[light_index];
		lptr.shadow_data=&light_a.shadow_data[light_index];
		lptr.position_data=&light_a.position_data[light_index];
		lptr.extra_data=&light_a.extra_data[light_index];
		lptr.params=&light_a.params[light_index];
	}
	return lptr;
}


PEWAPI int light_GetLightCount()
{
	return light_a.light_count;
}












