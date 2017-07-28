

struct sysMaterialFields
{
	vec4 sysMaterialBaseColor;
	float sysMaterialGlossiness;
	float sysMaterialMetallic;
	float sysMaterialEmissive;
	int sysMaterialFlags;
};


#ifdef _GL2B_
#extension ARB_explicit_uniform_location : enable

uniform sysMaterialFields sysMaterialParams;

	
#else
#extension ARB_uniform_buffer_object : enable

layout(location = 0, shared) uniform sysMaterialParamsUniformBlock
{
	sysMaterialFields sysMaterialParams;
};

#endif

#define  MATERIAL_Shadeless 1
#define  MATERIAL_Wireframe 1<<1
#define  MATERIAL_DiffuseTexture 1<<2
#define  MATERIAL_NormalTexture 1<<3
#define  MATERIAL_GlossTexture 1<<4
#define  MATERIAL_MetallicTexture 1<<5
#define  MATERIAL_HeightTexture 1<<6
#define  MATERIAL_EnvironmentTexture 1<<7
#define  MATERIAL_FrontAndBack 1<<8
#define  MATERIAL_Translucent 1<<9
#define  MATERIAL_Emissive 1<<10


