#ifndef SHADER_TYPES_H
#define SHADER_TYPES_H

enum ATTRIBUTES
{
	ATTRIBUTE_vPosition=0,
	ATTRIBUTE_vNormal,
	ATTRIBUTE_vTangent,
	//ATTRIBUTE_vBTangent,
	ATTRIBUTE_vTexCoord,
};

enum UNIFORMS
{
	UNIFORM_Time=0,
	UNIFORM_RenderTargetWidth,
	UNIFORM_RenderTargetHeight,
	UNIFORM_ShadowMapSize,
	UNIFORM_TextureSampler0,
	UNIFORM_TextureSampler1,
	UNIFORM_TextureSampler2,
	UNIFORM_TextureSampler3,
	UNIFORM_TextureSampler4,
	UNIFORM_TextureSamplerCube0,
	UNIFORM_TextureSamplerCube1,
	UNIFORM_TextureSamplerCube2,
	UNIFORM_TextureSamplerCube3,
	UNIFORM_TextureSamplerCube4,
	UNIFORM_DepthSampler,
	UNIFORM_2DShadowSampler,
	UNIFORM_3DShadowSampler,
	UNIFORM_TextureLayer0,
	UNIFORM_ZNear,
	UNIFORM_ZFar,
	UNIFORM_LightZNear,
	UNIFORM_LightZFar,
	UNIFORM_LightType,
	UNIFORM_LightCount,
	UNIFORM_MaterialFlags,
	UNIFORM_MFLAG_Shadeless,
	UNIFORM_MFLAG_DiffuseTexture,
	UNIFORM_MFLAG_NormalTexture,
	UNIFORM_MFLAG_GlossTexture,
	UNIFORM_MFLAG_MetallicTexture,
	UNIFORM_MFLAG_HeightTexture,
	UNIFORM_MFLAG_FrontAndBack,
	UNIFORM_BloomRadius,
	UNIFORM_BloomIntensity,
	UNIFORM_Exposure,
	UNIFORM_RenderDrawMode,
	//UNIFORM_LightId,
	//UNIFORM_LightLinearFallof,
	//UNIFORM_LightSquaredFallof,
	//UNIFORM_LightPosition,
	//UNIFORM_LightDirection,
	//UNIFORM_LightColor,
	//UNIFORM_LightRadius,
	UNIFORM_CameraToWorldMatrix,
	UNIFORM_WorldToLightMatrix,
	UNIFORM_CameraToLightProjectionMatrix,
	UNIFORM_LightProjectionMatrix,
	UNIFORM_LightModelViewMatrix,
	//UNIFORM_LastModelViewMatrix,
	//UNIFORM_EntityIndex,
	/*UNIFORM_MatrixSlot0,
	UNIFORM_MatrixSlot1,
	UNIFORM_MatrixSlot2,
	UNIFORM_MatrixSlot3,*/
	
	//UNIFORM_ViewPos,
	//UNIFORM_ViewVec,
	UNIFORM_CameraProjectionMatrix,
	
	UNIFORM_Last						/* DO NOT REMOVE THIS... */
};

enum LIGHT_UNIFORMS
{
	UNIFORM_LIGHT_LightPosition = UNIFORM_Last,
	UNIFORM_LIGHT_LightOrientation,
	UNIFORM_LIGHT_LightColor,
	UNIFORM_LIGHT_LightRadius,
	UNIFORM_LIGHT_LightType,
	UNIFORM_LIGHT_LinearFallof,
	UNIFORM_LIGHT_QuadraticFalloc,
	UNIFORM_LIGHT_SpotCutoff,
	UNIFORM_LIGHT_SpotBlend,
	UNIFORM_LIGHT_LightProjectionMatrix,
	UNIFORM_LIGHT_LightModelViewMatrix,
	UNIFORM_LIGHT_CameraToLightProjectionMatrix,
	UNIFORM_LIGHT_ZNear,
	UNIFORM_LIGHT_ZFar,
	UNIFORM_LIGHT_Last,
};

enum MATERIAL_UNIFORMS
{
	UNIFORM_MATERIAL_Shadeless = UNIFORM_LIGHT_Last,
	UNIFORM_MATERIAL_DiffuseTexture,
	UNIFORM_MATERIAL_NormalTexture,
	UNIFORM_MATERIAL_SpecularTexture,
	UNIFORM_MATERIAL_HeightTexture,
	UNIFORM_MATERIAL_FrontAndBack,
	
};

enum SHADER_FLAGS
{
	SHADER_CAPTURE_VERTEX = 1,
	SHADER_CAPTURE_NORMAL = 1 << 1,
	SHADER_CAPTURE_TANGENT = 1 << 2,
	SHADER_CAPTURE_TEXCOORD = 1 << 3
};

#define MATERIAL_PARAMS_BINDING 0
#define MATERIAL_PARAMS_MAX_NAME_LEN 64
#define MATERIAL_PARAMS_FIELDS 5

#define LIGHT_PARAMS_BINDING 1



typedef struct
{
	int v_position;
	int v_normal;
	int v_tangent;
	int v_tcoord;
	unsigned int shader_ID;
	unsigned char *default_uniforms;
	char *name;
	short flags;
}shader_t;

typedef struct
{
	int array_size;
	int shader_count;
	int stack_top;
	int *free_stack;
	shader_t *shaders;
}shader_array;

typedef struct define_t
{
	char *str;
	struct define_t *next;
	union
	{
		int ival;
		int fval;
	};
}define_t;










#endif /* SHADER_TYPES_H */
