#extension ARB_uniform_buffer_object : enable

layout(shared) uniform sysMaterialParams
{
	vec4 sysMaterialBaseColor;
	float sysMaterialGlossiness;
	float sysMaterialMetallic;
	float sysMaterialEmissive;
	int sysMaterialFlags;
};

#define MATERIAL_Wireframe 1<<1
#define MATERIAL_DiffuseTexture 1<<2
#define MATERIAL_NormalTexture 1<<3
#define MATERIAL_GlossTexture 1<<4
#define MATERIAL_MetallicTexture 1<<5
#define MATERIAL_HeightTexture 1<<6
#define MATERIAL_FrontAndBack 1<<7
#define MATERIAL_Translucent = 1<<8
#define MATERIAL_Emissive = 1<<9			
#define MATERIAL_Animated = 1<<10			
#define MATERIAL_Warp = 1<<11

