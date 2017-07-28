

uniform int sysLightCount;

#define MAX_LIGHTS_PER_CALL 128


struct sysLightParamsFields
{
	mat3 sysLightOrientation;
	vec3 sysLightPosition;
	vec3 sysLightColor;
	vec2 sysLightShadowMapOrigin;
	float sysLightRadius;
	float sysLightLinearAttenuation;
	float sysLightQuadraticAttenuation;
	float sysLightShadowMapSize;
	int sysLightType;
};


#ifdef _GL3A_
#extension ARB_uniform_buffer_object : enable

layout (shared) uniform sysLightParamsUniformBlock
{
	sysLightParamsFields sysLightParams[MAX_LIGHTS_PER_CALL];	
};

#else

uniform sysLightParamsFields sysLightParams[MAX_LIGHTS_PER_CALL];	

#endif

