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






#define light_SetLightType(x) glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, x)
#define light_SetAreaType(x) glLighti(GL_LIGHT1, GL_SPOT_EXPONENT, x)


/*#define light_CreateSpotLight(name, flags, position, orientation, color, distance, energy, spot_angle, spot_blend, lin_fallof, sqrd_fallof, scattering, volume_samples, shadow_map_res, tex_index) \
        light_CreateLight(name, (flags & (~(LIGHT_POINT | LIGHT_DIRECTIONAL))) | LIGHT_SPOT, position, orientation, color, distance, energy, spot_angle, spot_blend, lin_fallof, sqrd_fallof, scattering, volume_samples, MIN_VOLUME_SAMPLES, shadow_map_res, MIN_SHADOW_MAP_RES, 0, tex_index)


#define light_CreatePointLight(name, flags, position, orientation, color, radius, energy, lin_fallof, sqrd_fallof, scattering, volume_samples, shadow_map_res) \
		light_CreateLight(name, (flags & (~(LIGHT_SPOT | LIGHT_DIRECTIONAL))) | LIGHT_POINT, position, orientation, color, distance, energy, 0, 0.0, lin_fallof, sqrd_fallof, scattering, volume_samples, MIN_VOLUME_SAMPLES, shadow_map_res, MIN_SHADOW_MAP_RES, 0, -1)*/

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

PEWAPI int light_CreateLight(char *name, int bm_flags, vec4_t position, mat3_t *orientation, vec3_t diffuse_color, float radius, float energy, float spot_angle, float spot_blend, float lin_fallof, float sqrd_fallof, float scattering, int volume_samples, int shadow_map_res, int max_shadow_aa_samples, int tex_index);

PEWAPI inline int light_CreatePointLight(char *name, int flags, vec4_t position, mat3_t *orientation, vec3_t color, float radius, float energy, float lin_fallof, float sqrd_fallof, float scattering, int volume_samples, int shadow_map_res);

PEWAPI inline int light_CreateSpotLight(char *name, int flags, vec4_t position, mat3_t *orientation, vec3_t color, float distance, float energy, float spot_angle, float spot_blend, float lin_fallof, float sqrd_fallof, float scattering, int volume_samples, int shadow_map_res, int tex_index);

PEWAPI void light_DestroyLight(light_ptr light);

PEWAPI void light_UpdateGPULight(light_ptr light);

PEWAPI void light_BindLightCache();

PEWAPI void light_UnbindLightCache();

//PEWAPI void light_DestroyLight(light_t *light);

//PEWAPI void light_DestroyLightByIndex(int light_index);

smap_t light_CreateShadowMap(int resolution);

smap_t light_CreateShadowCubeMap(int resolution);

void light_DestroyShadowMap(smap_t *shadow_map);

/* sort by world position */
void light_SortLights();

/* sets lights for access in the shaders. If light_count is bigger than MAX_LIGHTS_PER_ENTITY, 
the adicional lights will be ignored. */
void light_SetLightsByIndex(int *IDs, int light_count);


void light_SetLights(int *IDs, int light_count);

void light_SetLight(int ID);

void light_GetAffectingLights(render_queue *rqueue);

PEWAPI void light_TranslateLight(light_ptr light, vec3_t direction, float amount, int b_set);

PEWAPI void light_RotateLight(light_ptr light, vec3_t axis, float angle, int b_set);


//void light_GetAffectedTiles();


//void light_CalculatePointLightFrustums(frustum_t *generated_frustum, frustum_t *frustums);


//PEWAPI light_t *light_GetLight(char *name);

PEWAPI light_ptr light_GetLight(char *name);

PEWAPI light_ptr light_GetActiveLight(int index);


//PEWAPI light_t *light_GetLightByIndex(int light_index);

PEWAPI light_ptr light_GetLightByIndex(int light_index);

//PEWAPI int light_GetMaxLightCount();

PEWAPI int light_GetLightCount();

PEWAPI inline unsigned char light_FloatToChar(float f);

PEWAPI inline vec3_t light_GetLightForwardVector(light_data0 *pdata);

PEWAPI inline vec3_t light_GetLightUpVector(light_data0 *pdata);

PEWAPI inline vec3_t light_GetLightRightVector(light_data0 *pdata);


#include "light.inl"

#ifdef __cplusplus
}
#endif




#endif /* LIGHT_H */



















