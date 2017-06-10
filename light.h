#ifndef LIGHT_H
#define LIGHT_H

#pragma once



#include "light_types.h"

#include "conf.h"
#include "includes.h"
#include "matrix.h"
#include "vector.h"
#include "frustum.h"
#include "entity.h"
#include "scenegraph.h"
#include "framebuffer.h"
#include "material.h"
#include "draw_types.h"


#define MIN_SHADOW_MAP_RES 				32
#define MAX_SHADOW_MAP_RES      		MIN_SHADOW_MAP_RES * 255

#define MIN_VOLUME_SAMPLES 				4
#define MAX_VOLUME_SAMPLES      		64
#define MAX_LIGHT_VOLUME_SCATTERING 	0.1
#define MIN_LIGHT_VOLUME_SCATTERING 	0.000002

#define MAX_LIGHT_ENERGY 				1000.0
#define MIN_LIGHT_ENERGY				0.02



#define light_SetLightType(x) glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, x)
#define light_SetAreaType(x) glLighti(GL_LIGHT1, GL_SPOT_EXPONENT, x)


#ifdef __cplusplus
extern "C"
{
#endif

void light_Init();

void light_Finish();

void light_ResizeLightArray(light_array *larray, int new_size);

void light_ResizeAffectingLightList(int new_size);

//PEWAPI int light_CreateLight(light_t *light);

//PEWAPI int light_CreateLightFromData(light_data0 *position_data, light_data1 *lparams);

PEWAPI int light_CreateLight(char *name, int bm_flags, vec4_t position, mat3_t *orientation, vec3_t diffuse_color, float radius, float energy, float spot_angle, float spot_blend, float lin_fallof, float sqrd_fallof, float scattering, int max_samples, int min_samples, int max_shadow_map_res, int min_shadow_map_res, int max_shadow_aa_samples);

//PEWAPI void light_DestroyLight(light_t *light);

//PEWAPI void light_DestroyLightByIndex(int light_index);

smap_t light_CreateShadowMap(int resolution);

smap_t light_CreateShadowCubeMap(int resolution);

/* sort by world position */
void light_SortLights();

/* sets lights for access in the shaders. If light_count is bigger than MAX_LIGHTS_PER_ENTITY, 
the adicional lights will be ignored. */
void light_SetLightsByIndex(int *IDs, int light_count);


void light_SetLights(int *IDs, int light_count);

void light_SetLight(int ID);

void light_GetAffectingLights(render_queue *rqueue);


//void light_GetAffectedTiles();


//void light_CalculatePointLightFrustums(frustum_t *generated_frustum, frustum_t *frustums);


//PEWAPI light_t *light_GetLight(char *name);

PEWAPI light_ptr light_GetLight(char *name);


//PEWAPI light_t *light_GetLightByIndex(int light_index);

PEWAPI light_ptr light_GetLightByIndex(int light_index);

//PEWAPI int light_GetMaxLightCount();

PEWAPI int light_GetLightCount();

PEWAPI static inline unsigned char light_FloatToChar(float f);

PEWAPI static inline vec3_t light_GetLightForwardVector(light_data0 *pdata);

PEWAPI static inline vec3_t light_GetLightUpVector(light_data0 *pdata);

PEWAPI static inline vec3_t light_GetLightRightVector(light_data0 *pdata);

#ifdef __cplusplus
}
#endif


#ifndef LIGHT_INL
#include "light.inl"
#endif


#endif /* LIGHT_H */



















