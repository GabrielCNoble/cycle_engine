struct sysOffsetFields
{
	mat4 sysMat4Field;
	mat3 sysMat3Field;
	mat2 sysMat2Field;
	vec4 sysVec4Field;
	vec3 sysVec3Field;
	vec2 sysVec2Field;
	float sysFloatField;
	int sysIntField;
	//short sysShortField;
	//short sysLastField;
};

#ifdef _GL3A_
#extension ARB_uniform_buffer_object : enable

layout(std140) uniform sysOffsetQueryUniformBlock
{
	sysOffsetFields sysOffsets;
};

#else

layout(std140) uniform sysOffsetFields sysOffsets;

#endif


