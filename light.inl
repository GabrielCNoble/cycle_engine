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


#ifdef __cplusplus
}
#endif



#endif /* LIGHT_INL */
