in sysMaterial
{
	vec4 base;
	float glossiness;
	float refraction;
};

#define SYS_MAX_LIGHTS 32

in sysLight
{
	vec3 color;
	vec3 forward_vector;
	float energy;
	float radius;
	short volume_samples;
	short shadow_aa_samples;
}sysLights[32];