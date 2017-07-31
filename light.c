#include "light.h"
#include "shader.h"
#include "draw.h"
#include "camera.h"
#include "material.h"		/* for material_FloatToColor4_t */
#include "vector.h"
//#include "framebuffer.h"
#include "macros.h"


extern renderer_t renderer;
extern shader_array shader_a;
light_array light_a;
light_array active_light_a;
extern camera_array camera_a;
int max_lights_per_pass;

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

extern int bm_extensions;

extern int uniform_buffer_alignment;
extern int light_params_uniform_buffer_size;
extern int type_offsets[];

void light_CacheGPULight(light_ptr light);

void light_DropGPULight(light_ptr light);

void light_Init()
{
	
	mat3_t m;
	
	int i;
	
	
	/*glGenTextures(1, &_512_extra_map);
	glBindTexture(GL_TEXTURE_2D, _512_extra_map);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
 	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
 	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 512, 512, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);*/
 	
 	glGenFramebuffers(1, &extra_framebuffer);
 	
	//glGetIntegerv(GL_MAX_LIGHTS, &max_lights_per_pass);
	
	light_cache_size = 256;
	cached_light_count = 0;
	free_stack_top = -1;
	free_stack = (int *)malloc(sizeof(int) * light_cache_size);
	
	if(bm_extensions & EXT_UNIFORM_BUFFER_OBJECT)
	{
		glGenBuffers(1, &light_cache);
		glBindBuffer(GL_UNIFORM_BUFFER, light_cache);
		glBufferData(GL_UNIFORM_BUFFER, light_params_uniform_buffer_size * light_cache_size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	
	
	
	
	
	
	
	light_a.shadow_data=NULL;
	light_a.position_data=NULL;
	light_a.extra_data=NULL;
	light_a.params=NULL;
	light_a.light_count=0;
	light_a.stack_top = -1;
	light_ResizeLightArray(&light_a, 8);
	
	affecting_lights.lights = NULL;
	affecting_lights.count = 0;
	light_ResizeAffectingLightList(16);
	
	active_light_a.shadow_data=NULL;
	active_light_a.position_data=NULL;
	active_light_a.extra_data=NULL;
	active_light_a.params=NULL;
	active_light_a.light_count=0;
	light_ResizeLightArray(&active_light_a, 8);
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), -0.5, 1);
	mat4_t_compose(&cube_shadow_mats[0], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[0]);
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 0.5, 1);
	mat4_t_compose(&cube_shadow_mats[1], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[1]);
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(1.0, 0.0, 0.0), -0.5, 1);
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 1.0, 0);
	mat4_t_compose(&cube_shadow_mats[2], &m, vec3(0.0, 0.0, 0.0));
	mat4_t_transpose(&cube_shadow_mats[2]);
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(1.0, 0.0, 0.0), 0.5, 1);
	//mat3_t_rotate(&m, vec3(0.0, 0.0, 1.0), 1.0, 0);
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
	return;
}

void light_ResizeLightArray(light_array *larray, int new_size)
{
	/* TODO: aligned allocation (using a single big chunk of memory maybe?)... */
	//light_world_data *wtemp=calloc(new_size, sizeof(light_world_data));
	//light_local_data *ltemp=calloc(new_size, sizeof(light_local_data));
	light_data2 *stemp=(light_data2 *)calloc(new_size, sizeof(light_data2));
	light_data0 *ctemp=(light_data0 *)calloc(new_size, sizeof(light_data0));
	light_data3 *etemp=(light_data3 *)calloc(new_size, sizeof(light_data3));
	light_data1 *ptemp=(light_data1 *)calloc(new_size, sizeof(light_data1));
	int *temp = (int *)malloc(sizeof(int) * new_size);
	
	if(larray->position_data)
	{
		//memcpy(wtemp, larray->world_data, sizeof(light_world_data)*larray->light_count);
		//memcpy(ltemp, larray->local_data, sizeof(light_local_data)*larray->light_count);
		memcpy(stemp, larray->shadow_data, sizeof(light_data2)*larray->light_count);
		memcpy(ctemp, larray->position_data, sizeof(light_data0)*larray->light_count);
		memcpy(etemp, larray->extra_data, sizeof(light_data3)*larray->light_count);
		memcpy(ptemp, larray->params, sizeof(light_data1)*larray->light_count);
		
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
	larray->shadow_data=stemp;
	larray->position_data=ctemp;
	larray->extra_data=etemp;
	larray->params=ptemp;
	larray->array_size=new_size;
	larray->free_stack = temp;
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
	
	params->spot_e = 255 * spot_blend;
	params->max_shadow_aa_samples = max_shadow_aa_samples;
	
	
	if(shadow_map_res > MAX_SHADOW_MAP_RES) shadow_map_res = MAX_SHADOW_MAP_RES;
	else if(shadow_map_res < MIN_SHADOW_MAP_RES) shadow_map_res = MIN_SHADOW_MAP_RES;
	shadow_map_res = (shadow_map_res + MIN_SHADOW_MAP_RES - 1) & (~(MIN_SHADOW_MAP_RES - 1));
	params->shadow_map_res = shadow_map_res / MIN_SHADOW_MAP_RES;
	
	
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
		if(bm_flags & LIGHT_GENERATE_SHADOWS)
		{
			shadow_data->shadow_map = light_CreateShadowCubeMap(shadow_map_res);
		}
			
		frustum_angle=45.0;		
		position_data->spot_co = 0;
		//position_data->spot_co=0;
	}
	else if(bm_flags&LIGHT_SPOT)
	{
		//position_data->local_position.floats[3]=1.0;
		position_data->local_position.floats[3] = 1.0;
		if(bm_flags&LIGHT_GENERATE_SHADOWS)
		{
			shadow_data->shadow_map=light_CreateShadowMap(shadow_map_res);
		}
			
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



PEWAPI void light_DestroyLight(light_ptr light)
{
	int light_index;
	if(light.position_data)
	{
		light_index = light.position_data->light_index;
		
		light_a.stack_top++;
		light_a.free_stack[light_a.stack_top] = light_index;
		
		if(light_a.position_data[light_index].bm_flags & LIGHT_GENERATE_SHADOWS)
		{
			light_DestroyShadowMap(&light_a.shadow_data[light_index].shadow_map);
		}
		
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

PEWAPI void light_BindLightCache()
{
	//glUniformBlockBinding(shader_a.shaders[renderer.active_shader_index].shader_ID, 1, LIGHT_PARAMS_BINDING);
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache);
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_PARAMS_BINDING, light_cache);
}

PEWAPI void light_UnbindLightCache()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_PARAMS_BINDING, 0);
}

PEWAPI void light_UpdateGPULight(light_ptr light)
{
	
}

void light_CacheGPULight(light_ptr light)
{
	int index;
	void *p;
	if(free_stack_top >= 0)
	{
		index = free_stack[free_stack_top--];
	}
	else
	{
		index = cached_light_count++;
	}
	
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache);
	p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
	p = (char *)p + light_params_uniform_buffer_size * index;
	
	/*memcpy(p, &light.extra_data->light_projection_matrix.floats[0][0], sizeof(mat4_t));
	
	p = ((char *)p) + type_offsets[OFFSET_MAT4];
	
	((vec4_t *)p)->x = light.position_data->world_orientation.floats[0][0];
	((vec4_t *)p)->y = light.position_data->world_orientation.floats[0][1];
	((vec4_t *)p)->z = light.position_data->world_orientation.floats[0][2];
	((vec4_t *)p)->w = 0.0;
	p = ((char *)p) + type_offsets[OFFSET_VEC4];
	
	((vec4_t *)p)->x = light.position_data->world_orientation.floats[1][0];
	((vec4_t *)p)->y = light.position_data->world_orientation.floats[1][1];
	((vec4_t *)p)->z = light.position_data->world_orientation.floats[1][2];
	((vec4_t *)p)->w = 0.0;
	p = ((char *)p) + type_offsets[OFFSET_VEC4];
	
	((vec4_t *)p)->x = light.position_data->world_orientation.floats[2][0];
	((vec4_t *)p)->y = light.position_data->world_orientation.floats[2][1];
	((vec4_t *)p)->z = light.position_data->world_orientation.floats[2][2];
	((vec4_t *)p)->w = 0.0;
	p = ((char *)p) + type_offsets[OFFSET_VEC4];
	
	((vec4_t *)p)->x = light.position_data->world_position.x;
	((vec4_t *)p)->y = light.position_data->world_position.y;
	((vec4_t *)p)->z = light.position_data->world_position.z;
	((vec4_t *)p)->w = 1.0;
	p = ((char *)p) + sizeof(float) * 3;
		
	*((float *)p) = light.position_data->radius;
	p = ((char *)p) + type_offsets[OFFSET_FLOAT];*/
	
	((vec3_t *)p)->r = (float)light.params->r / 255.0;
	((vec3_t *)p)->g = (float)light.params->g / 255.0;
	((vec3_t *)p)->b = (float)light.params->b / 255.0;
	p = ((char *)p) + sizeof(float) * 3;
	*((float *)p) = light.position_data->radius;	
		

	light.position_data->cache_index = index;
	light.position_data->bm_flags |= LIGHT_CACHED;
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	
}

void light_DropGPULight(light_ptr light)
{
	
	if(light.position_data->bm_flags & LIGHT_CACHED)
	{
		free_stack_top++;
		free_stack[free_stack_top] = light.position_data->cache_index;
		light.position_data->bm_flags &= ~MATERIAL_Cached;
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












