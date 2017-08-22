#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define LIGHT_DIRECTIONAL 4

#define MAX_ACTIVE_LIGHTS 32
#define CLUSTER_SIZE 32
#define CLUSTER_Z_DIVS 16


uniform int sysLightCount;
uniform int sysLightIndex;
uniform usampler3D sysClusterTexture;

struct sysLightParamsFields
{
	vec4 sysLightRightVector;
	vec4 sysLightUpVector;
	vec4 sysLightForwardVector;
	vec4 sysLightPosition;
	vec4 sysLightColor;
	
	float sysLightRadius;
	float sysLightSpotCutoff;
	float sysLightSpotCosCutoff;
	float sysLightSpotBlend;
	
	float sysLightShadowV;
	float sysLightShadowU;
	float sysLightShadowSize;
	int sysLightType;
};


#ifdef _GL3A_
#extension GL_ARB_uniform_buffer_object : enable

layout (std140) uniform sysLightParamsUniformBlock
{
	sysLightParamsFields sysLightParams[MAX_ACTIVE_LIGHTS];	
};

#else

layout(std140) uniform sysLightParamsFields sysLightParams[MAX_ACTIVE_LIGHTS];	

#endif


float sysAttenuatePoint(vec3 light_vec, float light_radius, float linear_fallof, float quadratic_fallof)
{
	float l = length(light_vec);
	float a = 1.0 / (l * linear_fallof + l * l * quadratic_fallof);
	return a * clamp((light_radius - l) / light_radius, 0.0, 1.0);
}

float sysAttenuateSpot(vec3 light_vec, vec3 spot_direction, float light_distance, float spot_cos_cutoff, float spot_exponent, float linear_fallof, float quadratic_fallof)
{
	float l = length(light_vec);
	float a = 1.0 / (l * linear_fallof + l * l * quadratic_fallof);
	float cos = max(dot(light_vec / l, spot_direction), 0.0);
	
	a *= cos * smoothstep(spot_cos_cutoff, spot_cos_cutoff + float(spot_exponent) / 255.0, cos) * clamp((light_distance - l) / light_distance, 0.0, 1.0);
	return a;
}

float sysSample2DShadowMap(float x, float y, float w, float h)
{
	return 1.0;
}

float sysSample3DShadowMap(float x, float y, float w, float h)
{
	return 1.0;
}

ivec3 sysGetCluster(float x_coord, float y_coord, float view_z, float z_near)
{
	ivec3 pos;
	pos.x = int(x_coord) / CLUSTER_SIZE;
	pos.y = int(y_coord) / CLUSTER_SIZE;
	pos.z = int(log(-view_z / -z_near) / log(1.0 + (2.0 * tan(0.68)) / CLUSTER_SIZE)) / CLUSTER_Z_DIVS;
	
	pos.z = clamp(pos.z, 0, CLUSTER_Z_DIVS);
	
	return pos;
}







