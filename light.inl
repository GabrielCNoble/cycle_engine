#ifndef LIGHT_INL
#define LIGHT_INL

#include "vector_types.h"
#include "light_types.h"


#ifdef __cplusplus
extern "C"
{
#endif 

PEWAPI unsigned char light_FloatToChar(float f)
{
	int c;
	c=255*f;
	if(c>255)c=255;
	else if(c<0) c=0;
	return c;
}


PEWAPI vec3_t light_GetLightForwardVector(light_data0 *pdata)
{
	vec3_t v;
	v.floats[0]=pdata->world_orientation.floats[2][0];
	v.floats[1]=pdata->world_orientation.floats[2][1];
	v.floats[2]=pdata->world_orientation.floats[2][2];
	return v;
}

PEWAPI vec3_t light_GetLightUpVector(light_data0 *pdata)
{
	vec3_t v;
	v.floats[0]=pdata->world_orientation.floats[1][0];
	v.floats[1]=pdata->world_orientation.floats[1][1];
	v.floats[2]=pdata->world_orientation.floats[1][2];
	return v;
}

PEWAPI vec3_t light_GetLightRightVector(light_data0 *pdata)
{
	vec3_t v;
	v.floats[0]=pdata->world_orientation.floats[0][0];
	v.floats[1]=pdata->world_orientation.floats[0][1];
	v.floats[2]=pdata->world_orientation.floats[0][2];
	return v;
}

PEWAPI int light_CreatePointLight(char *name, int flags, vec4_t position, mat3_t *orientation, vec3_t color, float radius, float energy, float lin_fallof, float sqrd_fallof, float scattering, int volume_samples, int shadow_map_res)
{
	return light_CreateLight(name, (flags & (~(LIGHT_SPOT | LIGHT_DIRECTIONAL))) | LIGHT_POINT, position, orientation, color, radius, energy, 0.0, 0.0, lin_fallof, sqrd_fallof, scattering, volume_samples, shadow_map_res, 0, -1);
}

PEWAPI static inline int light_CreateSpotLight(char *name, int flags, vec4_t position, mat3_t *orientation, vec3_t color, float distance, float energy, float spot_angle, float spot_blend, float lin_fallof, float sqrd_fallof, float scattering, int volume_samples, int shadow_map_res, int tex_index)
{
	return light_CreateLight(name, (flags & (~(LIGHT_POINT | LIGHT_DIRECTIONAL))) | LIGHT_SPOT, position, orientation, color, distance, energy, spot_angle, spot_blend, lin_fallof, sqrd_fallof, scattering, volume_samples, shadow_map_res, 0, tex_index);
}


#ifdef __cplusplus
}
#endif



#endif /* LIGHT_INL */
