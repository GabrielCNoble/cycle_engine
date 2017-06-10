#ifndef SHADER_INL
#define SHADER_INL

#include "draw_types.h"
#include "shader_types.h"


extern renderer_t renderer;
extern shader_array shader_a;

#define PARANOID


#ifdef __cplusplus
extern "C"
{
#endif 



PEWAPI shader_t *shader_GetActiveShader()
{
	return &shader_a.shaders[renderer.active_shader_index];
}

PEWAPI void shader_SetCurrentShaderUniform1i(int uniform, int value)
{
	if(renderer.active_shader_index >= 0)
		glUniform1i(shader_a.shaders[renderer.active_shader_index].default_uniforms[uniform], value);
	#ifdef PARANOID
	else
		printf("invalid active shader!\n");
	#endif
		
		
}

PEWAPI void shader_SetCurrentShaderUniform1f(int uniform, float value)
{
	if(renderer.active_shader_index >= 0)
		glUniform1f(shader_a.shaders[renderer.active_shader_index].default_uniforms[uniform], value);
	#ifdef PARANOID
	else
		printf("invalid active shader!\n");
	#endif	
}

PEWAPI void shader_SetCurrentShaderUniform4fv(int uniform, float *value)
{
	if(renderer.active_shader_index >= 0)
		glUniform4fv(shader_a.shaders[renderer.active_shader_index].default_uniforms[uniform], 1, value);
	#ifdef PARANOID
	else
		printf("invalid active shader!\n");
	#endif	
}

PEWAPI void shader_SetCurrentShaderUniformMatrix4fv(int uniform, float *value)
{
	if(renderer.active_shader_index >= 0)
		glUniformMatrix4fv(shader_a.shaders[renderer.active_shader_index].default_uniforms[uniform], 1, 0, value);
	#ifdef PARANOID
	else
		printf("invalid active shader!\n");
	#endif	
}

PEWAPI void shader_SetCurrentShaderVertexAttribArray(int attrib)
{
	int vertex_attrib_array;
	int size;
	switch(attrib)
	{
		case ATTRIBUTE_vPosition:
			vertex_attrib_array=shader_a.shaders[renderer.active_shader_index].v_position;
			size = 3;
		break;
		
		case ATTRIBUTE_vNormal:
			vertex_attrib_array=shader_a.shaders[renderer.active_shader_index].v_normal;
			size = 3;
		break;
		
		case ATTRIBUTE_vTangent:
			vertex_attrib_array = shader_a.shaders[renderer.active_shader_index].v_tangent;
			size = 3;
		break;
		
		case ATTRIBUTE_vTexCoord:
			vertex_attrib_array=shader_a.shaders[renderer.active_shader_index].v_tcoord;
			size = 2;
		break;	
	}
	
	glEnableVertexAttribArray(vertex_attrib_array);
	glVertexAttribPointer(vertex_attrib_array, size, GL_FLOAT, GL_FALSE, 0, NULL);
	
	return;
}
#ifdef __cplusplus
}
#endif 

#endif /* SHADER_INL */















