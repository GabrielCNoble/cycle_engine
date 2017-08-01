#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define LIGHT_DIRECTIONAL 4

#define MAX_LIGHTS_PER_CALL 32

#extension GL_ARB_explicit_uniform_location : enable
uniform int sysLightCount;

uniform int sysLightIndexes[MAX_LIGHTS_PER_CALL];

struct sysLightParamsFields
{
	vec3 sysLightColor;
	float sysLightRadius;
	float sysLightSpotCutoff;
	float sysLightSpotCosCutoff;
	float sysLightSpotBlend;
};


#ifdef _GL3A_
#extension GL_ARB_uniform_buffer_object : enable

layout (std140) uniform sysLightParamsUniformBlock
{
	sysLightParamsFields sysLightParams[MAX_LIGHTS_PER_CALL];	
};

#else

layout(std140) uniform sysLightParamsFields sysLightParams[MAX_LIGHTS_PER_CALL];	

#endif


float attenuate_point(vec3 light_vec, float light_radius, float linear_fallof, float quadratic_fallof)
{
	float l = length(light_vec);
	float a = 1.0 / (l * linear_fallof + l * l * quadratic_fallof);
	return a * clamp((light_radius - l) / light_radius, 0.0, 1.0);
}

float attenuate_spot(vec3 light_vec, vec3 spot_direction, float light_distance, float spot_cos_cutoff, float spot_exponent, float linear_fallof, float quadratic_fallof)
{
	float l = length(light_vec);
	float a = 1.0 / (l * linear_fallof + l * l * quadratic_fallof);
	float cos = max(dot(light_vec / l, spot_direction), 0.0);
	
	a *= cos * smoothstep(spot_cos_cutoff, spot_cos_cutoff + float(spot_exponent) / 255.0, cos) * clamp((light_distance - l) / light_distance, 0.0, 1.0);
	return a;
}
