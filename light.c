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

//static unsigned int _64_extra_map;
//static unsigned int _128_extra_map;
//static unsigned int _256_extra_map;
//unsigned int _512_extra_map;
//static unsigned int _1024_extra_map;
//static unsigned int _2048_extra_map;


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
 	
	glGetIntegerv(GL_MAX_LIGHTS, &max_lights_per_pass);
	
	
	light_a.shadow_data=NULL;
	light_a.position_data=NULL;
	light_a.extra_data=NULL;
	light_a.params=NULL;
	light_a.light_count=0;
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
	}
	
	//larray->world_data=wtemp;
	//larray->local_data=ltemp;
	larray->shadow_data=stemp;
	larray->position_data=ctemp;
	larray->extra_data=etemp;
	larray->params=ptemp;
	larray->array_size=new_size;
	return;
	
	/*light_t *temp=(light_t *)calloc(new_size, sizeof(light_t));
	if(larray->lights)
	{
		memcpy(temp, larray->lights, sizeof(light_t)*larray->light_count);
		free(larray->lights);
	}
	larray->lights=temp;
	larray->array_size=new_size;
	return;*/
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
/*PEWAPI int light_CreateLightFromData(light_position_data *position_data, light_params *lparams)
{
	int light_index;
	float frustum_angle;
	if(likely(position_data && lparams))
	{
		if(light_a.light_count>=light_a.array_size)
		{
			light_ResizeLightArray(&light_a, light_a.array_size<<1);
		}
		
		
		light_index=light_a.light_count;
		
		
		if(position_data->bm_flags&LIGHT_POINT)
		{
			
			position_data->local_position.floats[3]=1.0;
			if(position_data->bm_flags&LIGHT_GENERATE_SHADOWS)
			{
				light_a.shadow_data[light_index].shadow_map=light_CreateShadowCubeMap(512);
			}
			
			frustum_angle=45.0;		
			position_data->spot_co=0;
		}
		else if(position_data->bm_flags&LIGHT_SPOT)
		{
			position_data->local_position.floats[3]=1.0;
			if(position_data->bm_flags&LIGHT_GENERATE_SHADOWS)
			{
				light_a.shadow_data[light_index].shadow_map=light_CreateShadowMap(512);
			}
			
			frustum_angle=(float)position_data->spot_co;
		}
		else
		{
			position_data->local_position.floats[3]=0.0;
		}
	
		light_a.position_data[light_index].world_orientation=position_data->local_orientation;
		light_a.position_data[light_index].local_orientation=position_data->local_orientation;
		light_a.position_data[light_index].world_position=position_data->local_position;
		light_a.position_data[light_index].local_position=position_data->local_position;
		light_a.position_data[light_index].radius=position_data->radius;
		light_a.position_data[light_index].bm_flags=position_data->bm_flags;
		light_a.position_data[light_index].bm_state=LIGHT_HAS_MOVED;
		light_a.position_data[light_index].spot_co=position_data->spot_co;
		
		if(position_data->bm_flags&LIGHT_POINT)
		{
			light_a.shadow_data[light_index].shadow_map=light_CreateShadowCubeMap(512);
			frustum_angle=90.0;
		}
		else
		{
			light_a.shadow_data[light_index].shadow_map=light_CreateShadowMap(512);
			frustum_angle=(float)position_data->spot_co;
		}
		
		CreatePerspectiveMatrix(&light_a.extra_data[light_index].light_projection_matrix, DegToRad(frustum_angle) , 1.0, 0.1, position_data->radius, light_a.extra_data[light_index].generated_frustums);
		light_a.shadow_data[light_index].znear = light_a.extra_data[light_index].generated_frustums[0].znear;
		light_a.shadow_data[light_index].zfar = light_a.extra_data[light_index].generated_frustums[0].zfar;
		//light_a.shadow_data[light_index].shadow_map=light_CreateShadowMap(512);
		//light_a.shadow_data[light_index].bm_shadow_flags=position_data->bm_flags;
	
		light_a.shadow_data[light_index].l_queue.command_buffers=NULL;
		light_a.shadow_data[light_index].l_queue.count=0;
		light_ResizeLightRenderQueue(&light_a.shadow_data[light_index], 16);


		lparams->assigned_node=scenegraph_AddNode(NODE_LIGHT, light_index, lparams->name);
		lparams->bm_status=0;
		light_a.params[light_index]=*lparams;
		light_a.light_count++;
		
		lparams->assigned_node=NULL;
		return light_index;
		switch(light->type)
		{
			case LIGHT_POINT:
				light->spot_co=180.0;
			case LIGHT_SPOT:
				light->local_position.floats[3]=1.0;
			break;
			
			case LIGHT_DIRECTIONAL:
				light->local_position.floats[3]=0.0;
			break;
		}
		light_index=light_a.light_count;
		light->world_position=light->local_position;
		light->const_fallof=light->radius;
		CreatePerspectiveMatrix(&light->light_projection_matrix, 3.14159265*0.35, 1.0, 0.1, light->radius, NULL);
		light->shadow_map=light_CreateShadowMap(1024);
		light->assigned_node=scenegraph_AddNode(NODE_LIGHT, light_index, light->name);
		light_a.lights[light_a.light_count++]=*light;
		light->assigned_node=NULL;
		
		//return light_a.light_count-1;
	}
}*/

PEWAPI int light_CreateLight(char *name, int bm_flags, vec4_t position, mat3_t *orientation, vec3_t diffuse_color, float radius, float energy, float spot_angle, float spot_blend, float lin_fallof, float sqrd_fallof, float scattering, int max_samples, int min_samples, int max_shadow_map_res, int min_shadow_map_res, int max_shadow_aa_samples)
{
	int light_index = light_a.light_count;
	int c;
	float frustum_angle;
	if(light_index>=light_a.array_size)
	{
		light_ResizeLightArray(&light_a, light_a.array_size<<1);
	}
	
	memcpy(&light_a.position_data[light_index].world_orientation, orientation, sizeof(mat3_t));
	memcpy(&light_a.position_data[light_index].local_orientation, orientation, sizeof(mat3_t));
	memcpy(&light_a.position_data[light_index].world_position, &position, sizeof(vec4_t));
	memcpy(&light_a.position_data[light_index].local_position, &position, sizeof(vec4_t));
	//light_a.position_data[light_index].world_position = position;
	//light_a.position_data[light_index].local_position = position;
	light_a.position_data[light_index].radius = radius;
	light_a.position_data[light_index].bm_flags = bm_flags;
	//light_a.position_data[light_index].bm_state = LIGHT_HAS_MOVED;
	light_a.position_data[light_index].spot_co = (char)spot_angle;
	
	if(spot_blend > 1.0) spot_blend = 1.0;
	else if(spot_blend < 0.0) spot_blend = 0.0;
	
	light_a.params[light_index].spot_e = 255 * spot_blend;
	light_a.params[light_index].max_shadow_aa_samples = max_shadow_aa_samples;
	
	
	if(max_shadow_map_res < 0) max_shadow_map_res = MIN_SHADOW_MAP_RES;
	else if(max_shadow_map_res > MAX_SHADOW_MAP_RES) max_shadow_map_res = MAX_SHADOW_MAP_RES;
	
	if(min_shadow_map_res < 0) min_shadow_map_res = MIN_SHADOW_MAP_RES;
	else if(min_shadow_map_res > MAX_SHADOW_MAP_RES) min_shadow_map_res = MAX_SHADOW_MAP_RES;
	
	if(min_shadow_map_res > max_shadow_map_res)
	{
		c = min_shadow_map_res;
		min_shadow_map_res = max_shadow_map_res;
		max_shadow_map_res = c;
	}
	
	max_shadow_map_res = (max_shadow_map_res + MIN_SHADOW_MAP_RES - 1) & (~(MIN_SHADOW_MAP_RES - 1));
	min_shadow_map_res = (min_shadow_map_res + MIN_SHADOW_MAP_RES - 1) & (~(MIN_SHADOW_MAP_RES - 1));
	
	light_a.params[light_index].max_shadow_map_res = max_shadow_map_res / MIN_SHADOW_MAP_RES;
	//light_a.params[light_index].min_shadow_map_res = min_shadow_map_res / MIN_SHADOW_MAP_RES;
	
	
	if(max_samples < 0) max_samples = MIN_VOLUME_SAMPLES;
	else if(max_samples > MAX_VOLUME_SAMPLES) max_samples = MAX_VOLUME_SAMPLES;
	
	if(min_samples < 0) min_samples = MIN_VOLUME_SAMPLES;
	else if(min_samples > MAX_VOLUME_SAMPLES) min_samples = MAX_VOLUME_SAMPLES;
	
	if(min_samples > max_samples)
	{
		c = min_samples;
		min_samples = max_samples;
		max_samples = c;
	}
	
	
	
	/* snap values to the closest MIN_SHADOW_MAP_RES multiple */
	
	
	
	
	
	if(bm_flags&LIGHT_POINT)
	{
			
		//position_data->local_position.floats[3]=1.0;
		light_a.position_data[light_index].local_position.floats[3] = 1.0;
		if(bm_flags&LIGHT_GENERATE_SHADOWS)
		{
			light_a.shadow_data[light_index].shadow_map=light_CreateShadowCubeMap(max_shadow_map_res);
		}
		else
		{
			light_a.position_data[light_index].bm_flags &= ~LIGHT_GENERATE_SHADOWS;
		}
			
		frustum_angle=45.0;		
		light_a.position_data[light_index].spot_co = 0;
		//position_data->spot_co=0;
	}
	else if(bm_flags&LIGHT_SPOT)
	{
		//position_data->local_position.floats[3]=1.0;
		light_a.position_data[light_index].local_position.floats[3] = 1.0;
		if(bm_flags&LIGHT_GENERATE_SHADOWS)
		{
			light_a.shadow_data[light_index].shadow_map=light_CreateShadowMap(max_shadow_map_res);
		}
		else
		{
			light_a.position_data[light_index].bm_flags &= ~LIGHT_GENERATE_SHADOWS;
		}
			
		frustum_angle = (float)light_a.position_data[light_index].spot_co;
	}
	else
	{
		light_a.position_data[light_index].local_position.floats[3] = 0.0;
	}
	
	
		

		
	CreatePerspectiveMatrix(&light_a.extra_data[light_index].light_projection_matrix, DegToRad(frustum_angle) , 1.0, 0.001, radius / 0.001, &light_a.extra_data[light_index].generated_frustum);
	
	/*if(bm_flags & LIGHT_POINT)
	{
		light_CalculatePointLightFrustums(&light_a.extra_data[light_index].generated_frustums[0], light_a.extra_data[light_index].generated_frustums);
	}*/
	
	light_a.shadow_data[light_index].znear = light_a.extra_data[light_index].generated_frustum.znear;
	light_a.shadow_data[light_index].zfar = light_a.extra_data[light_index].generated_frustum.zfar;
	
	//light_a.params[light_index].diffuse = material_FloatToColor4_t(diffuse_color.floats[0], diffuse_color.floats[1], diffuse_color.floats[2], 1.0);
	light_a.params[light_index].r = 255 * diffuse_color.r;
	light_a.params[light_index].g = 255 * diffuse_color.g;
	light_a.params[light_index].b = 255 * diffuse_color.b;
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
	
	
	light_a.params[light_index].lin_fallof = 0xffff * lin_fallof;
	light_a.params[light_index].sqr_fallof = 0xffff * sqrd_fallof;
	//light_a.params[light_index].name = strdup(name);
	light_a.params[light_index].scattering = 0xffff * (scattering / MAX_LIGHT_VOLUME_SCATTERING);
	light_a.params[light_index].energy = 0xffff * (energy / MAX_LIGHT_ENERGY);
	light_a.params[light_index].max_samples = max_samples;
	//light_a.params[light_index].min_samples = min_samples;
	
	//light_a.params[light_index].assigned_node = scenegraph_AddNode(NODE_LIGHT, light_index, name);
	//light_a.params[light_index].bm_status = 0;
	
	light_a.extra_data[light_index].name = strdup(name);
	light_a.extra_data[light_index].assigned_node = scenegraph_AddNode(NODE_LIGHT, light_index, -1, name);
	

	light_a.light_count++;

	return light_index;
}


/*PEWAPI void light_DestroyLight(light_t *light)
{
	
}*/

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
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


void light_SortLights()
{
	return;
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


void light_SetLight(int ID)
{
	vec4_t pos;
	//mat4_t light_model_view_projection_matrix;
	mat4_t camera_to_light_clipspace;
	mat4_t ctransform;
	vec4_t fv;
	float d[3];
	float c[4];
	camera_t *active_camera=camera_GetActiveCamera();
	//mat4_t camera_to_light_clipspace_matrix;
	//mat3_t cam_orientation=camera_a.cameras[renderer.active_camera_index].world_orientation;
	//vec3_t cam_position=camera_a.cameras[renderer.active_camera_index].world_position;
	//mat4_t camera_transform
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(&camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix.floats[0][0]);
	
	if(ID>=0)
	{
		
		//mat3_t_transpose(&cam_orientation);
		
		//mat4_t_mult(&light_model_view_projection_matrix, &active_light_a.world_data[ID].world_to_light_matrix, &active_light_a.world_data[ID].light_projection_matrix);
		//mat4_t_mult(&camera_to_light_clipspace_matrix, &)
		
		
		d[0]=-active_light_a.position_data[ID].world_orientation.floats[2][0];
		d[1]=-active_light_a.position_data[ID].world_orientation.floats[2][1];
		d[2]=-active_light_a.position_data[ID].world_orientation.floats[2][2];
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, d);
		
		/* is this faster? really? */
		c[0]=color_conversion_lookup_table[active_light_a.params[ID].r];
		c[1]=color_conversion_lookup_table[active_light_a.params[ID].g];
		c[2]=color_conversion_lookup_table[active_light_a.params[ID].b];
		c[3]=active_light_a.position_data[ID].radius;
		glLightfv(GL_LIGHT0, GL_DIFFUSE, c);
		
		/*c[0]=color_conversion_lookup_table[active_light_a.params[ID].specular.r];
		c[1]=color_conversion_lookup_table[active_light_a.params[ID].specular.g];
		c[2]=color_conversion_lookup_table[active_light_a.params[ID].specular.b];
		glLightfv(GL_LIGHT0, GL_SPECULAR, c);*/
		
		glLightfv(GL_LIGHT0, GL_POSITION, &active_light_a.position_data[ID].world_position.floats[0]);
		
	
		
		//glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, light_a.lights[IDs[i]].const_fallof);
		//glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, light_a.lights[IDs[i]].lin_fallof);
		//glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, light_a.lights[IDs[i]].sqr_fallof);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, (float)active_light_a.position_data[ID].spot_co);
		glLighti(GL_LIGHT0, GL_SPOT_EXPONENT, (int)active_light_a.params[ID].spot_e);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, (float)active_light_a.params[ID].lin_fallof/255.0);
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, (float)active_light_a.params[ID].sqr_fallof/255.0);
		
		glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, active_light_a.params[ID].scattering);
		glLighti(GL_LIGHT1, GL_SPOT_CUTOFF, active_light_a.params[ID].max_samples);
		//printf("light is using %d samples\n", active_light_a.params[ID].max_samples);
		
		//glPushMatrix();

		//glUniformMatrix4fv(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightModelViewProjectionMatrix], 1, 0, &light_model_view_projection_matrix.floats[0][0]);
		
		//glUniformMatrix4fv(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightModelViewProjectionMatrix], 1, 0, &active_light_a.shadow_data[ID].model_view_projection_matrix.floats[0][0]);
		//shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_LightModelViewProjectionMatrix, &active_light_a.shadow_data[ID].model_view_projection_matrix.floats[0][0]);
		//shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_LightProjectionMatrix, &active_light_a.extra_data[ID].light_projection_matrix.floats[0][0]);
		//shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_LightModelViewMatrix, &active_light_a.extra_data[ID].world_to_light_matrix.floats[0][0]);
		
		shader_SetCurrentShaderUniform1f(UNIFORM_LightZNear, active_light_a.shadow_data[ID].znear);
		shader_SetCurrentShaderUniform1f(UNIFORM_LightZFar, active_light_a.shadow_data[ID].zfar);
		
		

		//glUniform1f(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightLinearFallof], (float)active_light_a.params[ID].lin_fallof/255.0);
		//glUniform1f(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightSquaredFallof], (float)active_light_a.params[ID].sqr_fallof/255.0);
		//glUniform1f(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightRadius], active_light_a.position_data[ID].radius);
		//glUniform1i(shader_a.shaders[renderer.active_shader_index].uniforms[UNIFORM_LightCount], 1);
		//glPopMatrix();
	
	}
	//glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	
	return;
	
}

void light_GetAffectingLights(render_queue *rqueue)
{
	int i;
	int c;
	int j;
	int k;
	int closest_index;
	vec3_t vec_to_light;
	vec3_t p;
	float v;
	float r;
	//c=light_a.light_count;
	//light_list->count=0;
	affecting_lights_t *a;
	c = affecting_lights.count;
	for(i=0; i < c; i++)
	{
		a = &affecting_lights.lights[i];
		k = a->count;
		for(j = 0; j < k; j++)
		{
			
		}
		/*if(!(light_a.lights[i].bm_status&LIGHT_FRUSTUM_CULLED))
		{
			v=light_a.lights[i].world_position.floats[0];
			if(v<entity->c_maxmins[3]-0.01+entity->world_position.floats[0])v=entity->c_maxmins[3]-0.01+entity->world_position.floats[0];
			if(v>entity->c_maxmins[0]+0.01+entity->world_position.floats[0])v=entity->c_maxmins[0]+0.01+entity->world_position.floats[0];
			p.floats[0]=v;
		
			v=light_a.lights[i].world_position.floats[1];
			if(v<entity->c_maxmins[4]-0.01+entity->world_position.floats[1])v=entity->c_maxmins[4]-0.01+entity->world_position.floats[1];
			if(v>entity->c_maxmins[1]+0.01+entity->world_position.floats[1])v=entity->c_maxmins[1]+0.01+entity->world_position.floats[1];
			p.floats[1]=v;
		
			v=light_a.lights[i].world_position.floats[2];
			if(v<entity->c_maxmins[5]-0.01+entity->world_position.floats[2])v=entity->c_maxmins[5]-0.01+entity->world_position.floats[2];
			if(v>entity->c_maxmins[2]+0.01+entity->world_position.floats[2])v=entity->c_maxmins[2]+0.01+entity->world_position.floats[2];
			p.floats[2]=v;
		
			vec_to_light.floats[0]=light_a.lights[i].world_position.floats[0]-p.floats[0];
			vec_to_light.floats[1]=light_a.lights[i].world_position.floats[1]-p.floats[1];
			vec_to_light.floats[2]=light_a.lights[i].world_position.floats[2]-p.floats[2];
			if(dot3(vec_to_light, vec_to_light)<light_a.lights[i].radius*light_a.lights[i].radius)
			{
				light_list->light_IDs[light_list->count]=i;
				light_list->count++;
			}
		}*/
		
	
	}
	return;
}


void light_GetAffectedTiles()
{
	/*int i;
	int c;
	int j;
	int k;
	camera_t *camera=camera_GetActiveCamera();
	c=light_a.light_count;
	vec4_t w_position;
	mat4_t model_view_projection_matrix;
	
	float x_max;
	float y_max;
	float x_min;
	float y_min;
	
	mat4_t_mult(&model_view_projection_matrix, &camera->world_to_camera_matrix, &camera->projection_matrix);
	draw_ClearScreenTiles();
	for(i=0; i<c; i++)
	{
		w_position=light_a.lights[i].world_position;
		w_position=MultiplyVector4(&model_view_projection_matrix, w_position);
		
		w_position.floats[0]/=w_position.floats[3];	
		w_position.floats[1]/=w_position.floats[3];
		w_position.floats[2]/=w_position.floats[3];
		
		w_position.floats[0]=(w_position.floats[0]/2.0)+0.5;
		w_position.floats[1]=(w_position.floats[1]/2.0)+0.5;
		w_position.floats[2]=(w_position.floats[2]/2.0)+0.5;
		
		j=(int)(renderer.screen_width*w_position.floats[0])/screen_tiles.tile_width;
		k=(int)(renderer.screen_height*w_position.floats[1])/screen_tiles.tile_height;
		
		if(j<0)j=0;
		else if(j>=screen_tiles.tiles_per_row)j=screen_tiles.tiles_per_row-1;
		if(k<0)k=0;
		else if(k>=screen_tiles.tiles_per_coloumn)k=screen_tiles.tiles_per_coloumn-1;

		
		if(w_position.floats[0]>screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].x_min && 
		   w_position.floats[0]<screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].x_max)
		{
			if(w_position.floats[1]>screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].y_min && 
		   	   w_position.floats[1]<screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].y_max)
			{
				screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].light_count++;
			}
		
		}
		
	}*/
}


void light_CalculatePointLightFrustums(frustum_t *generated_frustum, frustum_t *frustums)
{
	
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

void light_CullLights()
{
	/*register int i;
	register int c;
	int s;
	vec3_t l_origin;
	plane_t top;
	plane_t bottom;
	plane_t right;
	plane_t left;
	plane_t near;
	plane_t far;
	
	float distance;
	camera_t *active_camera=camera_GetActiveCamera();
	vec3_t fvec=camera_GetCameraWorldForwardVector(active_camera);
	vec3_t uvec=camera_GetCameraWorldUpVector(active_camera);
	vec3_t rvec=camera_GetCameraWorldRightVector(active_camera);
	
	vec3_t cpos=active_camera->world_position;
	
	vec3_t ftl;
	vec3_t fbl;
	vec3_t ftr;
	vec3_t fbr;
	
	vec3_t ntl;
	vec3_t nbl;
	vec3_t ntr;
	vec3_t nbr;
	
	vec3_t fc;
	vec3_t nc;
	float nzfar=-active_camera->frustum.zfar;
	float nznear=-active_camera->frustum.znear;
	
	float ftop=(nzfar*active_camera->frustum.top)/nznear;
	float ntop=active_camera->frustum.top;

	float fright=(nzfar*active_camera->frustum.right)/nznear;
	float nright=active_camera->frustum.right;

	
	nc=add3(cpos, mul3(fvec, nznear));
	fc=add3(cpos, mul3(fvec, nzfar));

	ftl.floats[0]=fc.floats[0] - rvec.floats[0]*fright + uvec.floats[0]*ftop;
	ftl.floats[1]=fc.floats[1] - rvec.floats[1]*fright + uvec.floats[1]*ftop;
	ftl.floats[2]=fc.floats[2] - rvec.floats[2]*fright + uvec.floats[2]*ftop;
	
	ftr.floats[0]=fc.floats[0] + rvec.floats[0]*fright + uvec.floats[0]*ftop;
	ftr.floats[1]=fc.floats[1] + rvec.floats[1]*fright + uvec.floats[1]*ftop;
	ftr.floats[2]=fc.floats[2] + rvec.floats[2]*fright + uvec.floats[2]*ftop;
	
	fbl.floats[0]=fc.floats[0] - rvec.floats[0]*fright - uvec.floats[0]*ftop;
	fbl.floats[1]=fc.floats[1] - rvec.floats[1]*fright - uvec.floats[1]*ftop;
	fbl.floats[2]=fc.floats[2] - rvec.floats[2]*fright - uvec.floats[2]*ftop;
	
	fbr.floats[0]=fc.floats[0] + rvec.floats[0]*fright - uvec.floats[0]*ftop;
	fbr.floats[1]=fc.floats[1] + rvec.floats[1]*fright - uvec.floats[1]*ftop;
	fbr.floats[2]=fc.floats[2] + rvec.floats[2]*fright - uvec.floats[2]*ftop;
	
	
	ntl.floats[0]=nc.floats[0] - rvec.floats[0]*nright + uvec.floats[0]*ntop;
	ntl.floats[1]=nc.floats[1] - rvec.floats[1]*nright + uvec.floats[1]*ntop;
	ntl.floats[2]=nc.floats[2] - rvec.floats[2]*nright + uvec.floats[2]*ntop;
	
	nbl.floats[0]=nc.floats[0] - rvec.floats[0]*nright - uvec.floats[0]*ntop;
	nbl.floats[1]=nc.floats[1] - rvec.floats[1]*nright - uvec.floats[1]*ntop;
	nbl.floats[2]=nc.floats[2] - rvec.floats[2]*nright - uvec.floats[2]*ntop;
	
	ntr.floats[0]=nc.floats[0] + rvec.floats[0]*nright + uvec.floats[0]*ntop;
	ntr.floats[1]=nc.floats[1] + rvec.floats[1]*nright + uvec.floats[1]*ntop;
	ntr.floats[2]=nc.floats[2] + rvec.floats[2]*nright + uvec.floats[2]*ntop;


	c=light_a.light_count;

	top=ComputePlane(cpos, ftl, ftr);
	bottom=ComputePlane(cpos, fbr, fbl);
	left=ComputePlane(cpos, fbl, ftl);
	right=ComputePlane(cpos, ftr, fbr);
	near=ComputePlane(ntl, ntr, nbl);
	far=ComputePlane(ftl, fbr, ftr);
	
	active_light_a.light_count=0;
	
	for(i=0; i<c; i++)
	{
		//light_a.lights[i].bm_status|= LIGHT_FRUSTUM_CULLED;
		
		l_origin=vec4vec3(light_a.world_data[i].world_position);
		


		distance=dot3(l_origin, top.normal) - top.d;
		if(distance<-light_a.params[i].radius) continue;


		distance=dot3(l_origin, bottom.normal) - bottom.d;
		if(distance<-light_a.params[i].radius) continue;


		distance=dot3(l_origin, left.normal) - left.d;
		if(distance<-light_a.params[i].radius) continue;


		distance=dot3(l_origin, right.normal) - right.d;
		if(distance<-light_a.params[i].radius) continue;


		distance=dot3(l_origin, near.normal) - near.d;
		if(distance<-light_a.params[i].radius) continue;
		
	
		distance=dot3(l_origin, far.normal) - far.d;
		if(distance<-light_a.params[i].radius) continue;
		//printf("light inside frustum\n");
		if(active_light_a.light_count<active_light_a.array_size)
		{
			//active_light_a.lights[active_light_a.light_count++]=light_a.lights[i];
			active_light_a.world_data[active_light_a.light_count]=light_a.world_data[i];
			active_light_a.local_data[active_light_a.light_count]=light_a.local_data[i];
			active_light_a.params[active_light_a.light_count]=light_a.params[i];
			active_light_a.light_count++;
		}
		else
		{
			light_ResizeLightArray(&active_light_a, active_light_a.array_size<<1);
			active_light_a.world_data[active_light_a.light_count]=light_a.world_data[i];
			active_light_a.local_data[active_light_a.light_count]=light_a.local_data[i];
			active_light_a.params[active_light_a.light_count]=light_a.params[i];
			active_light_a.light_count++;
			
			//active_light_a.lights[active_light_a.light_count++]=light_a.lights[i];
		}
		
		
		//light_a.lights[i].bm_status&= ~LIGHT_FRUSTUM_CULLED;
		
		


	}
	//printf("\n");
	//printf("%d\n", active_light_a.light_count);*/
}


/*void light_ResizeLightRenderQueue(light_shadow_data *lshadow_data, int new_size)
{
	command_buffer_t *ctemp;
	if(lshadow_data)
	{
		ctemp=calloc(new_size, sizeof(command_buffer_t));
		if(lshadow_data->l_queue.command_buffers)
		{
			memcpy(ctemp, lshadow_data->l_queue.command_buffers, sizeof(command_buffer_t)*lshadow_data->l_queue.count);
			free(lshadow_data->l_queue.command_buffers);
		}
		lshadow_data->l_queue.command_buffers=ctemp;
		lshadow_data->l_queue.queue_size=new_size;
	}
	
	return;
}*/

/*PEWAPI int light_GetMaxLightCount()
{
	return max_lights_per_pass;
}*/


PEWAPI int light_GetLightCount()
{
	return light_a.light_count;
}


/*PEWAPI unsigned char light_FloatToChar(float f)
{
	int c;
	c=255*f;
	if(c>255)c=255;
	else if(c<0) c=0;
	return c;
}*/

/*PEWAPI vec3_t light_GetLightForwardVector(light_position_data *pdata)
{
	vec3_t v;
	v.floats[0]=pdata->world_orientation.floats[2][0];
	v.floats[1]=pdata->world_orientation.floats[2][1];
	v.floats[2]=pdata->world_orientation.floats[2][2];
	return v;
}

PEWAPI vec3_t light_GetLightUpVector(light_position_data *pdata)
{
	vec3_t v;
	v.floats[0]=pdata->world_orientation.floats[1][0];
	v.floats[1]=pdata->world_orientation.floats[1][1];
	v.floats[2]=pdata->world_orientation.floats[1][2];
	return v;
}

PEWAPI vec3_t light_GetLightRightVector(light_position_data *pdata)
{
	vec3_t v;
	v.floats[0]=pdata->world_orientation.floats[0][0];
	v.floats[1]=pdata->world_orientation.floats[0][1];
	v.floats[2]=pdata->world_orientation.floats[0][2];
	return v;
}*/



















