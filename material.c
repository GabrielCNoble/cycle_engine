#include "material.h"
#include "draw.h"
#include "shader.h"
#include "texture.h"

float color_conversion_lookup_table[256];

material_array material_a;
extern renderer_t renderer;
extern texture_array texture_a;
//#define PRINT_INFO

static int material_path_lenght = 0;
static char material_path[256];

#ifdef __cplusplus
extern "C"
{
#endif

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
	
	for(i=0; i<256; i++)
	{
		color_conversion_lookup_table[i]=(float)i/255.0;
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


PEWAPI void material_CreateMaterial(char *name, short shininess, float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a, float specular_r, float specular_g, float specular_b, float specular_intensity, int bm_flags, tex_info_t *ti)
{
	material_t m;
	color4_t p[2];
	int i;
	
	m.name = name;
	m.shininess = shininess;
	material_FloatToBaseMultiplierPair(diffuse_r, diffuse_g, diffuse_b, diffuse_a, p);
	
	m.diff_color = p[0];
	m.diff_mult = p[1];
	
	p[0] = material_FloatToColor4_t(specular_r, specular_g, specular_b, specular_intensity);
	
	m.spec_color = p[0];
	
	m.diff_tex = -1;
	m.norm_tex = -1;
	m.heig_tex = -1;
	m.spec_tex = -1;
	
	m.bm_flags = bm_flags;
	
 
	if(bm_flags & MATERIAL_DiffuseTexture)
	{
		//for(i = 0; i<ti->diff_tex_count && i < 2; i++)
		//{
		//	m.diff_tex[i] = ti->diff_tex[i];
		//}
		m.diff_tex = ti->diff_tex;
	}
		
	if(bm_flags & MATERIAL_NormalTexture)
	{
		/*for(i = 0; i<ti->norm_tex_count && i < 2; i++)
		{
			m.norm_tex[i] = ti->norm_tex[i];
		}*/
		m.norm_tex = ti->norm_tex;
	}
		
	if(bm_flags & MATERIAL_HeightTexture)
	{
		/*for(i = 0; i<ti->heig_tex_count && i < 2; i++)
		{
			m.heig_tex[i] = ti->heig_tex[i];
		}*/
		m.heig_tex = ti->heig_tex;
	}
	
	/*if(bm_flags & MATERIAL_SpecularTexture)
	{
		m.spec_tex = ti->spec_tex;
		//printf("spec tex: %d\n", m.spec_tex);
	}*/
	
	if(bm_flags & MATERIAL_GlossTexture)
	{
		m.gloss_tex = ti->gloss_tex;
		//printf("spec tex: %d\n", m.spec_tex);
	}
	
	if(bm_flags & MATERIAL_MetallicTexture)
	{
		m.met_tex = ti->met_tex;
		//printf("spec tex: %d\n", m.spec_tex);
	}
	
	if(bm_flags & MATERIAL_Wireframe)
	{
		m.shader_index = shader_GetShaderIndex("wireframe");
		m.shininess = 0;
	}
	else if(bm_flags & (MATERIAL_Shadeless | MATERIAL_Emissive))
	{
		m.shader_index = shader_GetShaderIndex("flat");
	}
	else if(bm_flags & MATERIAL_Translucent)
	{
		m.shader_index = shader_GetShaderIndex("draw_translucent");	
	}
	else
	{
		m.shader_index = shader_GetShaderIndex("lit");
	}

		
	

	
	if(material_a.material_count>=material_a.array_size)
	{
		material_ResizeMaterialArray(material_a.array_size<<1);
	}
	material_a.materials[material_a.material_count++]=m;
	
	//printf("created material %s index %d\n", name, material_a.material_count-1);
	
	//printf("%s   diff: %d|%d  norm: %d|%d\n", name,  m.diff_tex[0], texture_a.textures[m.diff_tex[0]].tex_ID, m.norm_tex[0], texture_a.textures[m.norm_tex[0]].tex_ID);
	
	//printf("material %s has shader index %d\n", m.name, m.shader_index);
	
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
			printf("material %s index %d\n", name, i);
			return i;
		}
	}
	return -1;
}


/*
=============
material_SetMaterialByIndex
=============
*/
PEWAPI void material_SetMaterialByIndex(int material_index)
{
	float c_color[4];
	if(material_index>=0)
	{
		
		renderer.active_material_index = material_index;
		
		if(material_a.materials[material_index].bm_flags&MATERIAL_Wireframe)
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
		}*/
		
		c_color[0]=color_conversion_lookup_table[material_a.materials[material_index].diff_color.r]*material_a.materials[material_index].diff_mult.r;
		c_color[1]=color_conversion_lookup_table[material_a.materials[material_index].diff_color.g]*material_a.materials[material_index].diff_mult.g;
		c_color[2]=color_conversion_lookup_table[material_a.materials[material_index].diff_color.b]*material_a.materials[material_index].diff_mult.b;
		c_color[3]=color_conversion_lookup_table[material_a.materials[material_index].diff_color.a];
		
		glMaterialfv(GL_FRONT, GL_DIFFUSE, (GLfloat *)&c_color);
	
		/*if(c_color[3]<1.0 && material_a.materials[material_index].bm_flags & MATERIAL_Translucent)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);
		}
		else
		{
			glDisable(GL_BLEND); 
			glDepthMask(GL_TRUE);
		}*/
		
				   /* is this really necessary? */
				   /* is this faster? */
				   /* really? A probably uncached memory access
				   is faster than a division instruction? */
				   
		c_color[0]=color_conversion_lookup_table[material_a.materials[material_index].spec_color.r];
		c_color[1]=color_conversion_lookup_table[material_a.materials[material_index].spec_color.g];
		c_color[2]=color_conversion_lookup_table[material_a.materials[material_index].spec_color.b];
		c_color[3]=color_conversion_lookup_table[material_a.materials[material_index].spec_color.a];
		glMaterialfv(GL_FRONT, GL_SPECULAR, (GLfloat *)&c_color);
		glMateriali(GL_FRONT, GL_SHININESS, material_a.materials[material_index].shininess);
		renderer.active_material_index=material_index;	
		
		//shader_SetCurrentShaderUniform1i(UNIFORM_MaterialFlags, (int)material_a.materials[material_index].bm_flags);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_Shadeless, (int)material_a.materials[material_index].bm_flags & MATERIAL_Shadeless);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_DiffuseTexture, (int)material_a.materials[material_index].bm_flags & MATERIAL_DiffuseTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_NormalTexture, (int)material_a.materials[material_index].bm_flags & MATERIAL_NormalTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_HeightTexture, (int)material_a.materials[material_index].bm_flags & MATERIAL_HeightTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_GlossTexture, (int)material_a.materials[material_index].bm_flags & MATERIAL_GlossTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_MetallicTexture, (int)material_a.materials[material_index].bm_flags & MATERIAL_MetallicTexture);
		shader_SetCurrentShaderUniform1i(UNIFORM_MFLAG_FrontAndBack, (int)material_a.materials[material_index].bm_flags & MATERIAL_FrontAndBack);
		
		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_a.textures[material_a.materials[material_index].diff_tex[0]].tex_ID);
		shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);*/
		
		//if(material_a.materials[material_index].diff_tex[0]!=-1)
		//{

			
			//printf("diff tex %d\n", material_a.materials[material_index].diff_tex[0]);
		if(material_a.materials[material_index].diff_tex!=-1)
		{
			texture_SetTextureByIndex(material_a.materials[material_index].diff_tex, GL_TEXTURE0, 0);
		}
			
		if(material_a.materials[material_index].norm_tex!=-1)
		{
			texture_SetTextureByIndex(material_a.materials[material_index].norm_tex, GL_TEXTURE1, 0);
		}
			
		if(material_a.materials[material_index].heig_tex!=-1)
		{
			texture_SetTextureByIndex(material_a.materials[material_index].heig_tex, GL_TEXTURE2, 0);
		}
			
		if(material_a.materials[material_index].gloss_tex != -1)
		{
			texture_SetTextureByIndex(material_a.materials[material_index].gloss_tex, GL_TEXTURE3, 0);
			//printf("set specular texture\n");
		}
		
		if(material_a.materials[material_index].met_tex != -1)
		{
			texture_SetTextureByIndex(material_a.materials[material_index].met_tex, GL_TEXTURE4, 0);
			//printf("set specular texture\n");
		}
			//shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
		//}
		
		//if(material_a.materials[material_index].shader_index!=renderer.active_shader_index)
		//{
			//shader_SetShaderByIndex(material_a.materials[material_index].shader_index);
		//}
			
	}
	return;
}



/*
=============
material_FloatToColor4_t
=============
*/
PEWAPI color4_t material_FloatToColor4_t(float r, float g, float b, float a)
{
	color4_t color;
	int t;
	
	t=255.0*r;
	/*if(t>255)t=255;*/
	color.r=t&0x000000ff;
	
	t=255.0*g;
	/*if(t>255)t=255;*/
	color.g=t&0x000000ff;
	
	t=255.0*b;
	/*if(t>255)t=255;*/
	color.b=t&0x000000ff;
	
	t=255.0*a;
	/*if(t>255)t=255;*/
	color.a=t&0x000000ff;
	
	return color;
}



/*
=============
material_FloatToBaseMultiplierPair
=============
*/
PEWAPI void material_FloatToBaseMultiplierPair(float r, float g, float b, float a, color4_t *pair)
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
}

PEWAPI void material_SetMaterialDiffuseColor(material_t *material, float r, float g, float b, float a)
{
	color4_t pair[2];
	material_FloatToBaseMultiplierPair(r, g, b, a, pair);
	material->diff_color=pair[0];
	material->diff_mult=pair[1];	
}


#ifdef __cplusplus
}
#endif





