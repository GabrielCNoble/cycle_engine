#include "material.h"
#include "draw.h"
#include "shader.h"
#include "texture.h"

float color_conversion_lookup_table[256];

material_array material_a;
extern renderer_t renderer;
extern texture_array texture_a;
//#define PRINT_INFO

/* init'ed in shader.c */
extern int material_params_uniform_buffer_size;

static int material_path_lenght = 0;
static char material_path[256];




#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void (*material_SetMaterialByIndex)(int material_index);

static void material_SetMaterialByIndexGL3A(int material_index);
static void material_SetMaterialByIndexGL2B(int material_index);

/*
=============
material_Init
=============
*/
PEWAPI void material_Init(char *path)
{
	int i;
	material_a.materials=NULL;
	material_a.material_count=0;
	material_ResizeMaterialArray(16);
	strcpy(material_path, path);
	material_path_lenght = strlen(material_path);
	
	/*for(i=0; i<256; i++)
	{
		color_conversion_lookup_table[i]=(float)i/255.0;
	}*/
	
	if((glBindBufferBase))
	{
		material_SetMaterialByIndex = material_SetMaterialByIndexGL3A; 
	}
	else
	{
		printf("warning: glBindBufferBase not supported. Using compatibility code...\n");
		material_SetMaterialByIndex = material_SetMaterialByIndexGL2B;
	}
	
	return;
}


/*
=============
material_Finish
=============
*/
PEWAPI void material_Finish()
{
	free(material_a.materials);
	return;
}


/*
=============
material_ResizeMaterialArray
=============
*/
PEWAPI void material_ResizeMaterialArray(int new_size)
{
	material_t *temp=(material_t *)calloc(new_size, sizeof(material_t));
	if(material_a.materials)
	{
		memcpy(temp, material_a.materials, sizeof(material_t)*material_a.material_count);
		free(material_a.materials);
	}
	material_a.materials=temp;
	material_a.array_size=new_size;
	
	printf("resize array\n");
	return;
}


/*
=============
material_CreateMaterial
=============
*/
PEWAPI void material_CreateMaterialFromData(material_t *material)
{
	if(material_a.material_count>=material_a.array_size)
	{
		material_ResizeMaterialArray(material_a.array_size<<1);
	}
	material_a.materials[material_a.material_count++]=*material;
	return;
}


PEWAPI void material_CreateMaterial(char *name, float glossiness, float metallic, vec4_t color, float emissive, int bm_flags, tex_info_t *ti)
{
	material_t *material = &material_a.materials[material_a.material_count++];
	//color4_t p[2];
	int i;
	void *v;
	material->name = name;
	if(glossiness > 1.0) glossiness = 1.0;
	else if(glossiness < 0.0) glossiness = 0.0;
	
	if(metallic > 1.0) metallic = 1.0;
	else if(metallic < 0.0) metallic = 0.0;
	
	if(color.r > 1.0) color.r = 1.0;
	if(color.r < 0.0) color.r = 0.0;
	
	if(color.g > 1.0) color.g = 1.0;
	if(color.g < 0.0) color.g = 0.0;
	
	if(color.b > 1.0) color.b = 1.0;
	if(color.b < 0.0) color.b = 0.0;
	
	if(color.a > 1.0) color.a = 1.0;
	if(color.a < 0.0) color.a = 0.0;
	
	if(emissive > MAX_MATERIAL_EMISSIVE) emissive = MAX_MATERIAL_EMISSIVE;
	else if(emissive < 0.0) emissive = 0.0;
	
	
	material->emissive = (emissive / MAX_MATERIAL_EMISSIVE) * 0xffff;
	material->glossiness = 0xffff * glossiness;
	material->metallic = 0xffff * metallic;
	material->diff_color.r = 0xff * color.r;
	material->diff_color.g = 0xff * color.g;
	material->diff_color.b = 0xff * color.b;
	material->diff_color.a = 0xff * color.a;

	
	material->diff_tex = -1;
	material->norm_tex = -1;
	material->met_tex = -1;
	material->gloss_tex = -1;
	material->heig_tex = -1;
	
	material->bm_flags = bm_flags;
	
	
	glGenBuffers(1, &material->uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, material->uniform_buffer);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 0, material->uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, material_params_uniform_buffer_size, NULL, GL_DYNAMIC_DRAW);
	//glBufferStorage(GL_UNIFORM_BUFFER, material_params_uniform_buffer_size, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	
	shader_UploadMaterialParams(material);
	//v = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
 
	if(bm_flags & MATERIAL_DiffuseTexture)
	{
		material->diff_tex = ti->diff_tex;
	}
		
	if(bm_flags & MATERIAL_NormalTexture)
	{
		material->norm_tex = ti->norm_tex;
	}
		
	if(bm_flags & MATERIAL_HeightTexture)
	{
		material->heig_tex = ti->heig_tex;
	}
	
	if(bm_flags & MATERIAL_GlossTexture)
	{
		material->gloss_tex = ti->gloss_tex;
	}
	
	if(bm_flags & MATERIAL_MetallicTexture)
	{
		material->met_tex = ti->met_tex;
	}
	
	if(bm_flags & MATERIAL_Wireframe)
	{
		material->shader_index = shader_GetShaderIndex("wireframe");
	}
	else if(bm_flags & (MATERIAL_Shadeless | MATERIAL_Emissive))
	{
		material->shader_index = shader_GetShaderIndex("flat");
	}
	else if(bm_flags & MATERIAL_Translucent)
	{
		material->shader_index = shader_GetShaderIndex("draw_translucent");	
	}
	else
	{
		material->shader_index = shader_GetShaderIndex("lit");
	}
	
	
	if(material_a.material_count>=material_a.array_size)
	{
		material_ResizeMaterialArray(material_a.array_size<<1);
	}
	
	
	return;
}


/*
=============
material_LoadMaterial
=============
*/
PEWAPI void material_LoadMaterial(char *filename, char *name)
{
	return;
}


PEWAPI material_t *material_GetMaterialByIndex(int material_index)
{
	return &material_a.materials[material_index];	
}



/*
=============
material_GetMaterialIndex
=============
*/
PEWAPI int material_GetMaterialIndex(char *name)
{
	int i;
	int c;
	c=material_a.material_count;
	for(i=0; i<c; i++)
	{
		if(!strcmp(material_a.materials[i].name, name))
		{
			return i;
		}
	}
	return -1;
}

/*
=============
material_SetMaterialByIndexGL3A (OpenGL 3.0 or above)
=============
*/
static void material_SetMaterialByIndexGL3A(int material_index)
{
	float c_color[4];
	float emissive;
	int bm_flags asm("edi\n");
	material_t *material asm("esi\n");
	if(material_index>=0)
	{
		renderer.active_material_index = material_index;	
		material = &material_a.materials[material_index];
		bm_flags = material->bm_flags;
		
		renderer.active_material_index = material_index;
		
		glBindBufferBase(GL_UNIFORM_BUFFER, MATERIAL_PARAMS_BINDING, material->uniform_buffer);
		
		if(bm_flags&MATERIAL_Wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	
		if(bm_flags & MATERIAL_DiffuseTexture)
		{
			texture_SetTextureByIndex(material->diff_tex, GL_TEXTURE0, 0);
		}
		
		if(bm_flags & MATERIAL_NormalTexture)
		{
			texture_SetTextureByIndex(material->norm_tex, GL_TEXTURE1, 0);
		}

		if(bm_flags & MATERIAL_HeightTexture)
		{
			texture_SetTextureByIndex(material->heig_tex, GL_TEXTURE2, 0);
		}

		if(bm_flags & MATERIAL_GlossTexture)
		{
			texture_SetTextureByIndex(material->gloss_tex, GL_TEXTURE3, 0);
		}

		if(bm_flags & MATERIAL_MetallicTexture)
		{
			texture_SetTextureByIndex(material->met_tex, GL_TEXTURE4, 0);
		}
			
	}
	return;
}

/*
=============
material_SetMaterialByIndexGL2B (OpenGL 2.0 or bellow)
=============
*/
static void material_SetMaterialByIndexGL2B(int material_index)
{
	float c_color[4];
	float emissive;
	int bm_flags asm("edi\n");
	material_t *material asm("esi\n");
	if(material_index>=0)
	{
		renderer.active_material_index = material_index;	
		material = &material_a.materials[material_index];
		bm_flags = material->bm_flags;
		
		renderer.active_material_index = material_index;

		if(bm_flags&MATERIAL_Wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		c_color[0] = (float)material->diff_color.r / 255.0;
		c_color[1] = (float)material->diff_color.g / 255.0;
		c_color[2] = (float)material->diff_color.b / 255.0;
		c_color[3] = (float)material->diff_color.a / 255.0;
		
		if(bm_flags & MATERIAL_Emissive)
		{
			emissive = ((float)material->emissive / (float)(0xffff)) * MAX_MATERIAL_EMISSIVE;
			c_color[0] *= 1.0 + emissive;
			c_color[1] *= 1.0 + emissive;
			c_color[2] *= 1.0 + emissive;
		}
		
		glMaterialfv(GL_FRONT, GL_DIFFUSE, (GLfloat *)&c_color);
		
		
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_Shadeless, (int)bm_flags & MATERIAL_Shadeless);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_DiffuseTexture, (int)bm_flags & MATERIAL_DiffuseTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_NormalTexture, (int)bm_flags & MATERIAL_NormalTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_HeightTexture, (int)bm_flags & MATERIAL_HeightTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_GlossTexture, (int)bm_flags & MATERIAL_GlossTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_MetallicTexture, (int)bm_flags & MATERIAL_MetallicTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_FrontAndBack, (int)bm_flags & MATERIAL_FrontAndBack);

		if(bm_flags & MATERIAL_DiffuseTexture)
		{
			texture_SetTextureByIndex(material->diff_tex, GL_TEXTURE0, 0);
		}

		if(bm_flags & MATERIAL_NormalTexture)
		{
			texture_SetTextureByIndex(material->norm_tex, GL_TEXTURE1, 0);
		}

		if(bm_flags & MATERIAL_HeightTexture)
		{
			texture_SetTextureByIndex(material->heig_tex, GL_TEXTURE2, 0);
		}

		if(bm_flags & MATERIAL_GlossTexture)
		{
			texture_SetTextureByIndex(material->gloss_tex, GL_TEXTURE3, 0);
		}

		if(bm_flags & MATERIAL_MetallicTexture)
		{
			texture_SetTextureByIndex(material->met_tex, GL_TEXTURE4, 0);
		}
			
	}
	return;
}

/*
=============
material_SetMaterialByIndex
=============
*/
/*PEWAPI void material_SetMaterialByIndex(int material_index)
{
	float c_color[4];
	float emissive;
	int bm_flags asm("edi\n");
	material_t *material asm("esi\n");
	if(material_index>=0)
	{
		renderer.active_material_index = material_index;	
		material = &material_a.materials[material_index];
		bm_flags = material->bm_flags;
		
		renderer.active_material_index = material_index;

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, material->uniform_buffer);

		
		if(bm_flags&MATERIAL_Wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		
		/*if(material_a.materials[material_index].bm_flags & MATERIAL_FrontAndBack)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
		}

		
		c_color[0] = (float)material->diff_color.r / 255.0;
		c_color[1] = (float)material->diff_color.g / 255.0;
		c_color[2] = (float)material->diff_color.b / 255.0;
		c_color[3] = (float)material->diff_color.a / 255.0;
		
		if(bm_flags & MATERIAL_Emissive)
		{
			emissive = ((float)material->emissive / (float)(0xffff)) * MAX_MATERIAL_EMISSIVE;
			c_color[0] *= 1.0 + emissive;
			c_color[1] *= 1.0 + emissive;
			c_color[2] *= 1.0 + emissive;
		}
		
		glMaterialfv(GL_FRONT, GL_DIFFUSE, (GLfloat *)&c_color);

		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_Shadeless, (int)bm_flags & MATERIAL_Shadeless);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_DiffuseTexture, (int)bm_flags & MATERIAL_DiffuseTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_NormalTexture, (int)bm_flags & MATERIAL_NormalTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_HeightTexture, (int)bm_flags & MATERIAL_HeightTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_GlossTexture, (int)bm_flags & MATERIAL_GlossTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_MetallicTexture, (int)bm_flags & MATERIAL_MetallicTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_FrontAndBack, (int)bm_flags & MATERIAL_FrontAndBack);
		
		//if(material_a.materials[material_index].diff_tex!=-1)
		if(bm_flags & MATERIAL_DiffuseTexture)
		{
			texture_SetTextureByIndex(material->diff_tex, GL_TEXTURE0, 0);
		}
			
		//if(material_a.materials[material_index].norm_tex!=-1)
		if(bm_flags & MATERIAL_NormalTexture)
		{
			texture_SetTextureByIndex(material->norm_tex, GL_TEXTURE1, 0);
		}
			
		//if(material_a.materials[material_index].heig_tex!=-1)
		if(bm_flags & MATERIAL_HeightTexture)
		{
			texture_SetTextureByIndex(material->heig_tex, GL_TEXTURE2, 0);
		}
		
			
		//if(material_a.materials[material_index].gloss_tex != -1)
		if(bm_flags & MATERIAL_GlossTexture)
		{
			texture_SetTextureByIndex(material->gloss_tex, GL_TEXTURE3, 0);
		}
		
		//if(material_a.materials[material_index].met_tex != -1)
		if(bm_flags & MATERIAL_MetallicTexture)
		{
			texture_SetTextureByIndex(material->met_tex, GL_TEXTURE4, 0);
		}
			
	}
	return;
}*/



/*
=============
material_FloatToColor4_t
=============
*/
/*PEWAPI color4_t material_FloatToColor4_t(float r, float g, float b, float a)
{
	color4_t color;
	int t;
	
	t=255.0*r;
	color.r=t&0x000000ff;
	
	t=255.0*g;
	color.g=t&0x000000ff;
	
	t=255.0*b;
	color.b=t&0x000000ff;
	
	t=255.0*a;

	color.a=t&0x000000ff;
	
	return color;
}
*/


/*
=============
material_FloatToBaseMultiplierPair
=============
*/
/*PEWAPI void material_FloatToBaseMultiplierPair(float r, float g, float b, float a, color4_t *pair)
{
	int base;
	int multiplier;
	
	base=255.0*r;
	multiplier=(int)(r-0.000000001)+1;
	pair[0].r=base/multiplier;
	pair[1].r=multiplier;
	
	base=255.0*g;
	multiplier=(int)(g-0.000000001)+1;
	pair[0].g=base/multiplier;
	pair[1].g=multiplier;
	
	base=255.0*b;
	multiplier=(int)(b-0.000000001)+1;
	pair[0].b=base/multiplier;
	pair[1].b=multiplier;
	
	base=255.0*a;
	multiplier=(int)(a-0.000000001)+1;
	pair[0].a=base/multiplier;
	pair[1].a=multiplier;
	
	return;
}*/

/*PEWAPI void material_SetMaterialDiffuseColor(material_t *material, float r, float g, float b, float a)
{
	color4_t pair[2];
	material_FloatToBaseMultiplierPair(r, g, b, a, pair);
	material->diff_color=pair[0];
	material->diff_mult=pair[1];	
}*/


#ifdef __cplusplus
}
#endif





