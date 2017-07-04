

uniform int sysLightCount;

#define MAX_LIGHTS_PER_CALL 128

#ifdef _GL3A_

#extension ARB_uniform_buffer_object : enable

layout (shared) uniform sysLightParams
{
	vec3 sysLightPosition[MAX_LIGHTS_PER_CALL];
	float sysLightRadius[MAX_LIGHTS_PER_CALL];
	float sysLightLinearAttenuation[MAX_LIGHTS_PER_CALL];
	float sysLightQuadraticAttenuation[MAX_LIGHTS_PER_CALL];
	int sysLightType[MAX_LIGHTS_PER_CALL];	
};

#endif


#ifdef _GL2B_

uniform vec3 sysLightPosition[MAX_LIGHTS_PER_CALL];
uniform float sysLightRadius[MAX_LIGHTS_PER_CALL];
uniform float sysLightLinearAttenuation[MAX_LIGHTS_PER_CALL];
uniform float sysLightQuadraticAttenuation[MAX_LIGHTS_PER_CALL];
uniform int sysLightType[MAX_LIGHTS_PER_CALL];

#endif

