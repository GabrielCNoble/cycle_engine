#extension ARB_uniform_buffer_object : enable
layout(shared) uniform sysMaterialParams
{
	vec4 base;
	float glossiness;
	float metallic;
	float emissive;
	int flags;
};