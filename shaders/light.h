#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define LIGHT_DIRECTIONAL 4

#define MAX_LIGHTS_PER_CALL 16

#extension GL_ARB_explicit_uniform_location : enable
uniform int sysLightCount;

struct sysLightParamsFields
{
	vec3 sysLightColor;
	float sysLightRadius;
};


#ifdef _GL3A_
#extension GL_ARB_uniform_buffer_object : enable

layout (shared) uniform sysLightParamsUniformBlock
{
	sysLightParamsFields sysLightParams[MAX_LIGHTS_PER_CALL];	
};

#else


layout(shared) uniform sysLightParamsFields sysLightParams[MAX_LIGHTS_PER_CALL];	

#endif

