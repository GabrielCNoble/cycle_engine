#include "shader.h"
#include "draw.h"
#include "console.h"
#include "camera.h"
#include "file.h"

#define MAX_SHADER_COMPILE_TRIES 10
//#define PRINT_INFO

static int shader_path_len;
static char shader_path[256];

static char attrib_names[][32]={"vPosition", 
								"vNormal", 
								"vTangent",
							   "vTexCoord"};
							   	
static char capture_varyings[3][10] = {"_vcap_", 
									   "_ncap_", 
									   "_tcap_"};

static char vertex_capture_string[] = "varying vec3 _vcap_;";
static char normal_capture_string[] = "varying vec3 _ncap_;";
static char tangent_capture_string[] = "varying vec3 _tcap_;";


#define UNIFORM_COUNT 38
static char *uniforms[64] = {"sysTime",
							 "sysRenderTargetWidth",
							 "sysRenderTargetHeight",
							 "sysShadowMapSize",
							 "sysTextureSampler0",
							 "sysTextureSampler1",
							 "sysTextureSampler2",
							 "sysTextureSampler3",
							 "sysTextureSampler4",
							 "sysTextureSamplerCube0",
							 "sysTextureSamplerCube1",
							 "sysTextureSamplerCube2",
							 "sysTextureSamplerCube3",
							 "sysTextureSamplerCube4",
							 "sysDepthSampler",
							 "sys2DShadowSampler",
							 "sys3DShadowSampler",
							 "sysTextureLayer0",
							 "sysZNear",
							 "sysZFar",
							 "sysLightZNear",
							 "sysLightZFar",
							 "sysLightType",
							 "sysLightCount",
							 "sysMaterialFlags",
							 "sysFlagShadeless",
							 "sysFlagDiffuseTexture",
							 "sysFlagNormalTexture",
							 "sysFlagGlossTexture",
							 "sysFlagMetallicTexture",
							 "sysFlagHeightTexture",
							 "sysFlagFrontAndBack",
						 	 "sysBloomRadius",
							 "sysBloomIntensity",
							 "sysExposure",
							 "sysRenderDrawMode",
							 "sysCameraToWorldMatrix",
							 "sysWorldToLightMatrix",
							 "sysCameraToLightProjectionMatrix",
							 "sysLightProjectionMatrix",
							 "sysLightModelViewMatrix",
							 "sysCameraProjectionMatrix"};


#define UNIFORM_LIGHT_COUNT	14		
static char *light_uniforms[64] = {"sysLightPosition",
								   "sysLightOrientation",
								   "sysLightColor",
								   "sysLightRadius",
								   "sysLightType",
								   "sysLightLinearFallof",
								   "sysLightQuadraticFallof",
								   "sysLightSpotCutoff",
								   "sysLightSpotBlend",
								   "sysLightProjectionMatrix",
								   "sysLightModelViewMatrix",
								   "sysLightCameraToLightProjectionMatrix",
								   "sysLightZNear",
								   "sysLightZFar"};

extern renderer_t renderer;
shader_array shader_a;

extern int screen_quad_shader_index;
extern int z_prepass_shader_index;
extern int deferred_process_shader_index;
extern int composite_shader_index;
extern int wireframe_shader_index;
extern int flat_shader_index;
extern int lit_shader_index;
extern int draw_translucent_shader_index;
extern int blend_translucent_shader_index;
extern int smap_shader_index;
extern int plvol_shader_index;
extern int slvol_shader_index;
extern int bl_shader_index;
extern int gb_shader_index;
extern int extract_intensity_shader_index;
extern int bloom_blur_shader_index;
extern int bilateral_blur_shader_index;
extern int draw_buffer_shader_index;
extern int draw_z_buffer_shader_index;
extern int intensity0_shader_index;
extern int intensity1_shader_index;

static int capture_count;
static varying_t *vroot = NULL;
static varying_t *vlast;


static varying_t *froot = NULL;
static varying_t *flast;
//static char **capture;

#ifdef __cplusplus
extern "C"
{
#endif


/*
=============
shader_Init
=============
*/
PEWAPI void shader_Init(char *path)
{
	shader_a.shaders=NULL;
	shader_a.shader_count=0;
	shader_ResizeShaderArray(16);
	
	strcpy(shader_path, path);
	shader_path_len = strlen(shader_path);
	
	screen_quad_shader_index=shader_LoadShader("screen_quad_vert.txt", "screen_quad_frag.txt", "screen_quad");
	z_prepass_shader_index = shader_LoadShader("z_prepass_vert.txt", "z_prepass_frag.txt", "z_prepass");
	composite_shader_index=shader_LoadShader("composite_vert.txt", "composite_frag.txt", "composite");
	lit_shader_index = shader_LoadShader("lit_vert.txt", "lit_frag.txt", "lit");
	draw_translucent_shader_index = shader_LoadShader("draw_translucent_vert.txt", "draw_translucent_frag.txt", "draw_translucent");
	blend_translucent_shader_index = shader_LoadShader("blend_translucent_vert.txt", "blend_translucent_frag.txt", "blend_translucent");
	deferred_process_shader_index=shader_LoadShader("deferred_process_vert.txt", "deferred_process_frag.txt", "deferred_process");
	wireframe_shader_index=shader_LoadShader("wireframe_vert.txt", "wireframe_frag.txt", "wireframe");
	flat_shader_index=shader_LoadShader("flat_vert.txt", "flat_frag.txt", "flat");
	smap_shader_index=shader_LoadShader("smap_vert.txt", "smap_frag.txt", "smap");
	plvol_shader_index=shader_LoadShader("volumetric_point_light_vert.txt", "volumetric_point_light_frag.txt", "volumetric_point_light");
	slvol_shader_index=shader_LoadShader("volumetric_spot_light_vert.txt", "volumetric_spot_light_frag.txt", "volumetric_spot_light");
	bl_shader_index=shader_LoadShader("bilateral_blur_vert.txt", "bilateral_blur_frag.txt", "bilateral_blur");
	gb_shader_index = shader_LoadShader("gaussian_blur_vert.txt", "gaussian_blur_frag.txt", "gaussian_blur");
	extract_intensity_shader_index = shader_LoadShader("extract_intensity_vert.txt", "extract_intensity_frag.txt", "extract_intensity");
	bloom_blur_shader_index = shader_LoadShader("bloom_blur_vert.txt", "bloom_blur_frag.txt", "bloom_blur");
	draw_buffer_shader_index = shader_LoadShader("debug_draw_buffer_vert.txt", "debug_draw_buffer_frag.txt", "draw_buffer");
	draw_z_buffer_shader_index = shader_LoadShader("debug_draw_z_buffer_vert.txt", "debug_draw_z_buffer_frag.txt", "draw_z_buffer");
	
	intensity0_shader_index = shader_LoadShader("intensity_vert.txt", "intensity0_frag.txt", "intensity0");
	intensity1_shader_index = shader_LoadShader("intensity_vert.txt", "intensity1_frag.txt", "intensity1");
	
	//shader_LoadShader("skinner_simple_vert.txt", "skinner_simple_frag.txt", "skinner");
	
	//shader_LoadShader("lit_texture_no_bump_vert.txt", "lit_texture_no_bump_frag.txt", "lit_texture_no_bump");
	//shader_LoadShader("lit_texture_bump_vert.txt", "lit_texture_bump_frag.txt", "lit_texture_bump");

	
	return;
}


/*
=============
shader_Finish
=============
*/
PEWAPI void shader_Finish()
{
	int i;
	for(i = 0; i < shader_a.shader_count; i++)
	{
		free(shader_a.shaders[i].default_uniforms);
	}
	printf("a\n");
	free(shader_a.shaders);
	printf("b\n");
	return;
}


/*
=============
shader_GetShaderFileSize
=============
*/
PEWAPI int shader_GetShaderFileSize(FILE *shader_file)
{
	unsigned int byte_count=0;
	while(!feof(shader_file))
	{
		fgetc(shader_file);
		byte_count++;
	}
	rewind(shader_file);
	return byte_count + 1;
}


/*
=============
shader_GetShaderString
=============
*/
PEWAPI char *shader_GetShaderString(FILE *shader_file)
{
	//printf("p0\n");
	int i=0;
	int s;
	char b;
	char *s_str;
	s=shader_GetShaderFileSize(shader_file);
	
	s_str=(char *)malloc(s+2);
	
	while(1)
	{
		b = fgetc(shader_file);
		if(!feof(shader_file))
		{
			s_str[i++] = b;
		}
		else
		{
			break;
		}
		
	}
	
	s_str[i] = '\0';

	/*while(!feof(shader_file))
	{
		*(s_str+i++)=fgetc(shader_file);
	}
	*(s_str+i-1)='\0';*/
	
	//printf("p1\n");
	
	return s_str;
}


/*
=============
shader_LoadShader
=============
*/
PEWAPI int shader_LoadShader(char *vertex_shader_name, char *fragment_shader_name, char *name)
{
	char *v_shader_str=NULL;
	char *f_shader_str=NULL;
	char *error_str=NULL;
	FILE *vertex_shader_file;
	FILE *fragment_shader_file;
	file_t q;
	int i;
	int t;
	int attribs[32];
	GLuint v_shader_obj;
	GLuint f_shader_obj;
	GLuint shader_prog;
	int sha_compile_tries=0;
	int b_compiled;
	int flags = 0;
	int h;
	char vfull_path[256];
	char ffull_path[256];
	strcpy(vfull_path, shader_path);
	strcpy(ffull_path, shader_path);
	strcat(vfull_path, vertex_shader_name);
	strcat(ffull_path, fragment_shader_name);
	int capture_count = 0;
	char *capture[3];

	
	/*if(!(vertex_shader_file=fopen(vfull_path, "r")))
	{
		console_Print(MESSAGE_ERROR, "couldn't open file [%s]!\n", vertex_shader_name);
		return -1;
	}
	v_shader_str=shader_GetShaderString(vertex_shader_file);*/
	
	//v_shader_str = file_LoadFile(vfull_path, 0);
	q = file_LoadFile(vfull_path, 0);
	v_shader_str = q.buf;
	
	if(!v_shader_str)
	{
		console_Print(MESSAGE_ERROR, "couldn't open file [%s]!\n", vertex_shader_name);
		return -1;
	}
	
	
	if(shader_Preprocess(&v_shader_str, &flags))
	{
		console_Print(MESSAGE_ERROR, "vertex shader [%s] has problematic preprocessor directives!\n", vertex_shader_name);
		printf("vertex shader [%s] has problematic preprocessor directives!\n", vertex_shader_name);
		return -1;
	}
	//printf("flags is %d\n", flags);
	if(flags & SHADER_CAPTURE_VERTEX)
	{
		capture[capture_count] = strdup(capture_varyings[0]);
		capture_count++;
	}
	if(flags & SHADER_CAPTURE_NORMAL)
	{
		capture[capture_count] = strdup(capture_varyings[1]);
		capture_count++;
	}
	if(flags & SHADER_CAPTURE_TANGENT)
	{
		capture[capture_count] = strdup(capture_varyings[2]);
		capture_count++;
	}
	

	
	/*if(!(fragment_shader_file=fopen(ffull_path, "r")))
	{
		//printf("couldn't open file [%s]!\n", fragment_shader_name);
		console_Print(MESSAGE_ERROR, "couldn't open file [%s]!\n", fragment_shader_name);
		return -1;
	}
	
	f_shader_str=shader_GetShaderString(fragment_shader_file);*/
	
	//f_shader_str = file_LoadFile(ffull_path, 0);
	q = file_LoadFile(ffull_path, 0);
	f_shader_str = q.buf;
	if(!f_shader_str)
	{
		console_Print(MESSAGE_ERROR, "couldn't open file [%s]!\n", fragment_shader_name);
		return -1;
	}
	
	if(shader_Preprocess(&f_shader_str, &flags))
	{
		console_Print(MESSAGE_ERROR, "fragment shader [%s] has problematic preprocessor directives!\n", fragment_shader_name);
		return -1;
	}
	//printf("a-2");
	
	v_shader_obj=glCreateShader(GL_VERTEX_SHADER);
	b_compiled=0;
	
	//printf("a-1");

	glShaderSource(v_shader_obj, 1, (const GLchar **)&v_shader_str, NULL);
	glCompileShader(v_shader_obj);
	glGetShaderiv(v_shader_obj, GL_COMPILE_STATUS, &b_compiled);

	if(!b_compiled)
	{
		error_str=(char *)calloc(2048, 1);
		console_Print(MESSAGE_ERROR, "[%s] vertex shader compilation failed!\nerror dump in external console\n", name);
		glGetShaderInfoLog(v_shader_obj, 2048, NULL, error_str);
		printf("Error dump:\n%s", error_str);
		free(error_str);
		glDeleteShader(v_shader_obj);
		return -1;
	}
	//else console_Print(MESSAGE_NORMAL, "[%s] vertex shader compilation sucessful\n", name);

	b_compiled=0;
	f_shader_obj=glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(f_shader_obj, 1, (const GLchar **)&f_shader_str, NULL);
	glCompileShader(f_shader_obj);
	glGetShaderiv(f_shader_obj, GL_COMPILE_STATUS, &b_compiled);
	
	//printf("a0");

	if(!b_compiled)
	{
		error_str=(char *)malloc(2048);
		glGetShaderInfoLog(f_shader_obj, 2048, NULL, error_str);
		console_Print(MESSAGE_ERROR, "[%s] fragment shader compilation failed!\nerror dump in external console\n", name);
		printf("Error dump:\n%s", error_str);
		//printf("[%s] fragment shader compilation failed!\n", name);
		//printf("The fragment shader didn't compile. Not my fault.\nError dump:\n%s", error_str);
		free(error_str);
		glDeleteShader(f_shader_obj);
		glDeleteShader(v_shader_obj);
		return -1;
	}
	//else console_Print(MESSAGE_NORMAL, "[%s] fragment shader compilation sucessful\n", name);
	shader_prog=glCreateProgram();
	
	
	glAttachShader(shader_prog, v_shader_obj);
	glAttachShader(shader_prog, f_shader_obj);
	
	
	if(shader_a.shader_count>=shader_a.array_size)
	{
		shader_ResizeShaderArray(shader_a.array_size*2);
	}
	
	shader_a.shaders[shader_a.shader_count].v_position = -1;
	shader_a.shaders[shader_a.shader_count].v_normal = -1;
	shader_a.shaders[shader_a.shader_count].v_tangent = -1;
	//shader_a.shaders[shader_a.shader_count].v_btangent = -1;
	shader_a.shaders[shader_a.shader_count].v_tcoord = -1;
	
	
	/* NOTE: attribute location binding MUST be done before shader linking. */
	/* brute force attribs to be bound to the correct positions. 
	glGetAttribLocation is (apparently) giving wrong result on AMD's
	drivers, so this is a CPU side shader attribute parsing mechanism. */
	shader_ParseShaderAttributes(v_shader_str, &attribs[0], &t);
	for(i=0; i<t; i++)
	{
		glBindAttribLocation(shader_prog, i, attrib_names[attribs[i]]);
		*(&shader_a.shaders[shader_a.shader_count].v_position+attribs[i])=i;
	}
	
	if(capture_count)
	{
		glTransformFeedbackVaryings(shader_prog, capture_count, (const GLchar **)capture, GL_SEPARATE_ATTRIBS);
	}
	
	glLinkProgram(shader_prog);			
	glGetProgramiv(shader_prog, GL_LINK_STATUS, &b_compiled);
	
	
	if(!b_compiled)
	{
		error_str=(char *)malloc(2048);
		printf("[%s] shader linking failed!\n", name);
		glGetProgramInfoLog(shader_prog, 2048, NULL, error_str);
		printf("shader failed to link. Not my fault!\nError dump:\n%s", error_str);
		free(error_str);
	}
	else console_Print(MESSAGE_NORMAL, "shader [%s] loaded\n", name);
	
	shader_a.shaders[shader_a.shader_count].default_uniforms = (unsigned char *)malloc(UNIFORM_Last + 1);

	
	shader_a.shaders[shader_a.shader_count].flags = flags;
	
	for(i = 0; i < UNIFORM_Last; i++)
	{
		shader_a.shaders[shader_a.shader_count].default_uniforms[UNIFORM_Time + i] = glGetUniformLocation(shader_prog, uniforms[i]);
	}
	
	/*for(i = 0; i < UNIFORM_LIGHT_COUNT; i++)
	{
		shader_a.shaders[shader_a.shader_count].default_uniforms[UNIFORM_LIGHT_LightPosition + i] = glGetUniformLocation(shader_prog, light_uniforms[i]);
	}*/


	shader_a.shaders[shader_a.shader_count].shader_ID=shader_prog;
	shader_a.shaders[shader_a.shader_count++].name=name;
	
	free(v_shader_str); 
	free(f_shader_str);
	//fclose(vertex_shader_file);
	//fclose(fragment_shader_file);

	return shader_a.shader_count-1;
	
}


/*
=============
shader_GetShaderIndex
=============
*/
PEWAPI int shader_GetShaderIndex(char *name)
{
	register int i;
	register int c;
	c=shader_a.shader_count;
	for(i=0; i<c; i++)
	{
		if(!strcmp(shader_a.shaders[i].name, name))
		{
			return i;
		}
	}
	return -1;
}


/*
=============
shader_GetActiveShader
=============
*/
/*PEWAPI shader_t *shader_GetActiveShader()
{
	return &shader_a.shaders[renderer.active_shader_index];
}*/



/*
=============
shader_SetShaderByIndex
=============
*/
PEWAPI void shader_SetShaderByIndex(int shader_index)
{
	camera_t *active_camera;
	if(shader_index>=0)
	{
		active_camera=camera_GetActiveCamera();
		glUseProgram(shader_a.shaders[shader_index].shader_ID);
		//glUniform1f(shader_a.shaders[shader_index].uniforms[UNIFORM_Time], renderer.time);
		//glUniform1f(shader_a.shaders[shader_index].uniforms[UNIFORM_RenderTargetWidth], (float)renderer.width);
	//	glUniform1f(shader_a.shaders[shader_index].uniforms[UNIFORM_RenderTargetHeight], (float)renderer.height);
		//glUniform1f(shader_a.shaders[shader_index].uniforms[UNIFORM_ZNear], active_camera->frustum.znear);
		//glUniform1f(shader_a.shaders[shader_index].uniforms[UNIFORM_ZFar], active_camera->frustum.zfar);
		//glUniformMatrix4fv(shader_a.shaders[shader_index].uniforms[UNIFORM_CameraProjectionMatrix], 1, 0, &active_camera->projection_matrix.floats[0][0]);
		renderer.active_shader_index=shader_index;
	}
	return;
}


/*__attribute((always_inline)) PEWAPI void shader_SetCurrentShaderUniform1i(int uniform, int value)
{
	glUniform1i(shader_a.shaders[renderer.active_shader_index].uniforms[uniform], value);
}

__attribute((always_inline)) PEWAPI void shader_SetCurrentShaderUniform1f(int uniform, float value)
{
	return;
}

__attribute((always_inline)) PEWAPI void shader_SetCurrentShaderUniform4fv(int uniform, float *value)
{
	return; 
}*/



/*
=============
shader_ResizeShaderArray
=============
*/
void shader_ResizeShaderArray(int new_size)
{
	shader_t *temp=(shader_t *)calloc(new_size, sizeof(shader_t));
	if(shader_a.shaders)
	{
		memcpy(temp, shader_a.shaders, sizeof(shader_t)*shader_a.shader_count);
		free(shader_a.shaders);
	}
	shader_a.shaders=temp;
	shader_a.array_size=new_size;
	return;
}


/*
=============
shader_ParseShaderAttributes
=============
*/
/* TODO: add duplicate attribute declaration detection */
void shader_ParseShaderAttributes(char *shader_str, int *attribs_found, int *attrib_count)
{
	int i=0;
	int j=0;
	int current_state=0;
	*attrib_count=0;
	char attrib_name[32];
	while(*(shader_str+i)!='\0')
	{
		if(*(shader_str+i)=='a' && 
		   *(shader_str+i+1)=='t' &&
		   *(shader_str+i+2)=='t' &&
		   *(shader_str+i+3)=='r' &&
		   *(shader_str+i+4)=='i' &&
		   *(shader_str+i+5)=='b' &&
		   *(shader_str+i+6)=='u' &&
		   *(shader_str+i+7)=='t' &&
		   *(shader_str+i+8)=='e')
		   {
		   	   i+=9;
			   current_state=1;
		   }
		
		if(current_state==1)
		{
			while((*(shader_str+i)==' ' || *(shader_str+i)=='\n') && *(shader_str+i)!='\0')i++;
			
			if(*(shader_str+i)=='v' && 
			   *(shader_str+i+1)=='e' &&
			   *(shader_str+i+2)=='c')
			   {
			    	i+=4;
			    	current_state=2;
			   }
		}
		
		if(current_state==2)
		{
			j=0;
			while((*(shader_str+i)==' ' || *(shader_str+i)=='\n') && *(shader_str+i)!='\0')i++;
			
			while(*(shader_str+i+j)!= '\n' && *(shader_str+i+j)!='\0' && *(shader_str+i+j)!=' ' && *(shader_str+i+j)!=';')
			{
				attrib_name[j]=*(shader_str+i+j);
				j++;
			}
			attrib_name[j]='\0';
			i+=j;
			
			if(!strcmp(attrib_name, "vPosition"))
			{
				attribs_found[(*attrib_count)++]=ATTRIBUTE_vPosition;
			}
			else if(!strcmp(attrib_name, "vNormal"))
			{
				attribs_found[(*attrib_count)++]=ATTRIBUTE_vNormal;
			}
			else if(!strcmp(attrib_name, "vTangent"))
			{
				attribs_found[(*attrib_count)++]=ATTRIBUTE_vTangent;
			}
			/*else if(!strcmp(attrib_name, "vBTangent"))
			{
				attribs_found[(*attrib_count)++]=ATTRIBUTE_vBTangent;
			}*/
			else if(!strcmp(attrib_name, "vTexCoord"))
			{
				attribs_found[(*attrib_count)++]=ATTRIBUTE_vTexCoord;
			}
			
			current_state=3;
		}
		
		i++;
	}
}


int shader_Preprocess(char **shader_str, int *flags)
{
	//printf("b0\n");
	int i;
	int j;
	int c = strlen(*shader_str) + 1;
	int k;
	int m;
	char *s = *shader_str;
	int index;
	int line = 0;
	int rtrn = 0;
	//int tflags = 0;
	define_t *r = NULL;
	define_t *b;
	define_t *p;
	
	for(i = 0; i < c; i++)
	{
		//printf("loop\n");
		switch(s[i])
		{
			case '#': 
				m = i;
				i++;
				while(s[i] == ' ') i++;
				
				if(s[i] 	== 'i' &&
				   s[i + 1] == 'n' &&
				   s[i + 2] == 'c' &&
				   s[i + 3] == 'l' &&
				   s[i + 4] == 'u' &&
				   s[i + 5] == 'd' &&
				   s[i + 6] == 'e'
				  )
				{
					//printf("q0\n");
					i += 7;
					if(shader_ExpandInclude(&s, m, i, c, &c, line))
					{
						//shader_ReleaseDefines(r);
						//return 1;
						rtrn = 1;
						goto _gtfo_preprocess;
					}
					i = 0;
					line = 0;
					*shader_str = s;
					//printf("q1\n");
				}
				
				/*else if(s[i] 	 == 'd' &&
				   		s[i + 1] == 'e' &&
				   		s[i + 2] == 'f' &&
				   		s[i + 3] == 'i' &&
				   		s[i + 4] == 'n' &&
				   		s[i + 5] == 'e'
				 	   )
				{
					i += 6;
					//printf("k0\n");
					if(shader_AddDefine(s, &r, &i))
					{
						//shader_ReleaseDefines(r);
						//return 1;
						rtrn = 1;
						goto _gtfo_preprocess;
					}
					//printf("k1\n");
				}*/
				
				else if(s[i] == 'i' && s[i + 1] == 'f')
				{
					i += 2;
					j = shader_FindEndif(s, c, m);
					
					
					if(
					   s[i] 	== 'n' && 
					   s[i + 1] == 'd' && 
					   s[i + 2] == 'e' && 
					   s[i + 3] == 'f'
					  )
					{
						//printf("z0\n");
						if(j < 0)
						{
							printf("error! #ifndef without a #endif! line: %d\n", line);
							//shader_ReleaseDefines(r);
							//return 1;
							rtrn = 1;
							goto _gtfo_preprocess;
						}
						
						i += 4;
						if(shader_CheckDefine(r, s, &i))
						{
							shader_EraseInBetweenDirectives(s, m, j);
						}
						else
						{
							shader_EraseDirectivesOnly(s, m, j);
						}
						
						i = 0;
						line = 0;
						*shader_str = s;
						//printf("z1\n");
						
					}
					
					else if(
							s[i]     == 'd' && 
					   		s[i + 1] == 'e' && 
					   		s[i + 2] == 'f'
					   	   )
					{
						//printf("t0\n");
						if(j < 0)
						{
							printf("error! #ifdef without a #endif! line: %d\n", line);
							//shader_ReleaseDefines(r);
							//return 1;
							rtrn = 1;
							goto _gtfo_preprocess;
						}
						
						i += 3;
						if(!shader_CheckDefine(r, s, &i))
						{
							shader_EraseInBetweenDirectives(s, m, j);
						}
						else
						{
							shader_EraseDirectivesOnly(s, m, j);
						}
						
						i = 0;
						line = 0;
						*shader_str = s;
						//printf("t1\n");
					}
					else
					{
						
					}   	   
				}
				
				if(s[i] 	== 'p' &&
				   s[i + 1] == 'r' &&
				   s[i + 2] == 'a' &&
				   s[i + 3] == 'g' &&
				   s[i + 4] == 'm' &&
				   s[i + 5] == 'a')
				{
					i += 6;
					shader_DoPragma(s, c, m, i, flags);
					*shader_str = s;
				}
				
			break;
			
			case '/':
				if(s[i + 1] == '/')
				{
					i += 2;
					while(s[i] != '\n' && s[i] != '\0') i++;
				}
				else if(s[i + 1] == '*')
				{
					i += 2;
					while(s[i] != '*' && s[i] != '\0')
					{
						i++;
						if(s[i] == '/') break;
						else if(s[i] == '\n') line++;
					}
				}
			break;
			
			case '\n':
				line++;
			break;
				
		}
	}
	
	_gtfo_preprocess:
	shader_ReleaseDefines(r);
	return rtrn;
}

int shader_ExpandInclude(char **shader_str, int start_index, int cur_index, int old_len, int *new_len, int line)
{
	int index = 0;
	int i = cur_index;
	int m = start_index;
	int j;
	int k;
	int c = old_len;
	int file_len;
	char name[512];
	char include_full_path[512];
	char *s = *shader_str;
	char *t;
	char *inc;
	FILE *f;
	while(s[i] == ' ') i++;
	
	if(s[i] != '"')
	{
		printf("error: missing quotation symbol! line: %d\n", line + 1);
		return 1;
	}
	i++;
					
	while(s[i] != ',' && 
		  s[i] != ' ' && 
		  s[i] != '\n' && 
		  s[i] != '\0' &&
		  s[i] != '"')
	{
		name[index++] = s[i++];
	}
	name[index] = '\0';
					
					
	if(s[i] != '"')
	{
		printf("error: missing quotation symbol! wowline: %d\n", line + 1);
		return 1;
	}
	i++;
					
	strcpy(include_full_path, shader_path);
	strcat(include_full_path, name);
					
	if(!(f = fopen(include_full_path, "r")))
	{
		printf("error: %s no such file!\n", name);
		return 1;
	}
					
	file_len = 0;
	while(!feof(f))
	{
		fgetc(f);
		file_len++;
	}
	rewind(f);
	file_len++;
					
	inc = (char *)malloc(file_len + 2);

	index = 0;
	while(!feof(f))
	{
		inc[index] = fgetc(f);
		index++;
	}

	inc[index - 1] = '\n';
	inc[index] = '\0';
					
	fclose(f);
					
	t = (char *)malloc(c + file_len);
					
					
	for(j = 0; j < m; j++)
	{
		t[j] = s[j]; 
	}
	k = j;

	for(j = 0; j < file_len - 1; j++)
	{
		t[j + k] = inc[j];
	}
	k += j;
					
	for(j = 0; j < c - i; j++)
	{
		t[j + k] = s[j + i];
	}
	t[j + k] = '\0';
					
	free(inc);
	free(s);
	s = t;
					
	*shader_str = s;
	*new_len = old_len + file_len;
	
	return 0;

}

int shader_AddDefine(char *shader_str, define_t **root, int *cur_index)
{
	int index = 0;
	int i = *cur_index;
	define_t *b;
	define_t *r = *root;
	define_t *p;
	char *s = shader_str;
	char name[512];
	while(s[i] == ' ') i++;
					
	while(
			s[i] != ',' && 
		  	s[i] != ' ' && 
		  	s[i] != '\n' && 
		  	s[i] != '\0' &&
		  	s[i] != '"'
		 )
						 
	{
		name[index++] = s[i++];
	}
	name[index] = '\0';
				
					
	b = (define_t *)malloc(sizeof(define_t));
	b->str = strdup(name);
	b->next = NULL;
	if(!r)
	{
		r = b;
		*root = b;
	}
	else
	{
		p = r;
		while(p)
		{
			/* if this define is already defined, bail out... */
			if(!strcmp(b->str, p->str))
			{
				printf("error: duplicated define!\n");
				free(b->str);
				free(b);
				p = r;
				while(p)
				{
					b = p->next;
					free(p->str);
					free(p);
					p = b;
				}
				return 1;
			}
			if(!p->next) break;
			p = p->next;
		}
						
		p->next = b;
	}
	*cur_index = i;
	return 0;
}

int shader_CheckDefine(define_t *root, char *shader_str, int *cur_index)
{
	define_t *t = root;
	int index;
	int i = *cur_index;
	char *s = shader_str;
	char name[512];
	
	index = 0;
	while(s[i] == ' ') i++;
	while(
			s[i] != ',' && 
			s[i] != ' ' && 
			s[i] != '\n' && 
			s[i] != '\0' &&
			s[i] != '"'
		)
	{
		name[index++] = s[i++];
	}
	name[index] = '\0';
	
	*cur_index = i;
	
	while(t)
	{
		if(!strcmp(t->str, name))
		{
			return 1;
		}
		t = t->next;
	}
	return 0;
}

int shader_FindEndif(char *shader_str, int str_len, int cur_index)
{
	int i = cur_index;
	int c = str_len;
	char *s = shader_str;
	int level = 0;
	int index = 0;
	
	for(; i < c; i++)
	{
		
		if(s[i] == '#')
		{
			i++;
			if(s[i] == 'i' && s[i + 1] == 'f')
			{
				i += 2;
				if((s[i] 	 == 'n' && 
					s[i + 1] == 'd' && 
					s[i + 2] == 'e' && 
					s[i + 3] == 'f') ||
					   
				   (s[i] 	 == 'd' && 
					s[i + 1] == 'e' && 
					s[i + 2] == 'f')
				  )
				{
					i += 3;
					level++;
				}
			}
			
			else if(s[i] == 'e' && 
					s[i + 1] == 'n' &&
					s[i + 2] == 'd' &&
					s[i + 3] == 'i' &&
					s[i + 4] == 'f'
					)
			{
				//i += 5;
				level--;
				if(!level) return i - 1;
				else i += 5;
			}
		}
	}
	
	if(level) return -1;
	
}

void shader_EraseDirectivesOnly(char *shader_str, int start_pos, int end_pos)
{
	int k;
	char *s = shader_str;
	k = start_pos;
	s[k] = ' ';
	k++;
	while(s[k] == ' ') k++;
	while(s[k] != ' ' && s[k] != '\n' && s[k] != '\0')
	{
		s[k] = ' ';
		k++;	
	}
	while(s[k] == ' ') k++;
	while(s[k] != ' ' && s[k] != '\n' && s[k] != '\0')
	{
		s[k] = ' ';
		k++;	
	}
							
	k = end_pos;
	s[k] = ' ';
	while(s[k] == ' ') k++;
	while(s[k] != ' ' && s[k] != '\n' && s[k] != '\0') s[k++] = ' ';
	
	return;
}

void shader_EraseInBetweenDirectives(char *shader_str, int start_pos, int end_pos)
{
	
	int k;
	char *s = shader_str;
	
	for(k = start_pos; k < end_pos; k++)
	{
		s[k] = ' ';
	}
							
	s[k] = ' ';
	while(s[k] == ' ') k++;
	while(s[k] != ' ' && s[k] != '\n' && s[k] != '\0') s[k++] = ' ';
	
	return;
}


int shader_DoPragma(char *shader_str, int str_len, int start_pos, int cur_pos, int *flags)
{
	int i;
	int c = str_len;
	int index;
	char name[512];
	char *s = shader_str;
	int b;
	int e;
	int tflags = 0;
	//varying_t *v;
	int get_next;	
	
	for(i = cur_pos; i < c; i++)
	{
		while(s[i] == ' ') i++;
		
		/* missing the specific pragma command... */
		if(s[i] == '\n')
		{
			return 1;
		}
		
		index = 0;
		while(s[i] != ' ' && s[i] != '\n' && s[i] != '\0' && s[i] != ',' && s[i] != '(' && s[i] != ')')
		{
			name[index++] = s[i++];
		}
		name[index] = '\0';
		
		//printf("do pragma\n");
		
		if(!strcmp(name, "capture"))
		{
			
			while(s[i] == ' ' || s[i] == '\n') i++;
			
			if(s[i] == '(')
			{
				//_get_next:
				get_next = 0;
				i++;
				while(s[i] == ' ' || s[i] == '\n') i++;
				index = 0;
				while(s[i] != ' ' && s[i] != '\n' && s[i] != ')' && s[i] != ',')
				{
					name[index++] = s[i++];
				}
				/* there was nothing in between the parentesis... */
				/*if(s[i] == ')')
				{
					return 1;
				}*/
				while(s[i] == ' ' || s[i] == '\n') i++;
				/* the closing parentesis is missing... */
				if(s[i] != ')')
				{
					//if(s[i] == ',') get_next = 1;
					return 1;
				}
				i++;
				
				name[index] = '\0';
				
				if(!strcmp(name, "vertex"))
				{
					tflags |= SHADER_CAPTURE_VERTEX;
				}
				else if(!strcmp(name, "normal"))
				{
					tflags |= SHADER_CAPTURE_NORMAL;
				}
				else if(!strcmp(name, "tangent"))
				{
					tflags |= SHADER_CAPTURE_TANGENT;
				}
				
				//if(get_next) goto _get_next;
			}

			e = i;
			for(i = start_pos; i < e; i++)
			{
				s[i] = ' ';
			}
			
			i = start_pos;
			if(tflags & SHADER_CAPTURE_VERTEX)
			{
				for(e = 0; i < str_len && vertex_capture_string[e] != '\0'; i++, e++)
				{
					s[i] = vertex_capture_string[e];
				}
			}
			if(tflags & SHADER_CAPTURE_NORMAL)
			{
				for(e = 0; i < str_len && normal_capture_string[e] != '\0'; i++, e++)
				{
					s[i] = normal_capture_string[e];
				}
			}
			
			if(tflags & SHADER_CAPTURE_TANGENT)
			{
				for(e = 0 ; i < str_len && tangent_capture_string[e] != '\0'; i++, e++)
				{
					s[i] = tangent_capture_string[e];
				}
			}
			
			//printf("%s\n\n\n", s);
			
			//printf("%s\n", s);
			*flags |= tflags;
			return 0;
			
			// capture directive requires braces around
			//the varyings to be captured... 
			/*if(s[i] == '{')
			{
				b = i;
				i++;
				
				while(s[i] != '}')
				{
					while(s[i] == ' ' || s[i] == '\n')i++;
				
					if(s[i] 	== 'v' &&
					   s[i + 1] == 'a' &&
					   s[i + 2] == 'r' &&
					   s[i + 3] == 'y' &&
					   s[i + 4] == 'i' &&
					   s[i + 5] == 'n' &&
					   s[i + 6] == 'g')
					{
						i +=7;	
					}
					else if(s[i] 	 == 'o' &&
							s[i + 1] == 'u' &&
							s[i + 2] == 't')
					{
						i += 3;
					}
					else if(s[i] == '}')
					{
						break;	
					}
					else
					{
						return 1;
					}
				
					index = 0;
					while(s[i] == ' ' || s[i] == '\n')i++;
				
					// the varying name starts with a number
					//(further tests are required...) 
					if(s[i] >= '0' && s[i] <= '9')
					{
						return 1;
					}
					
					if(s[i] 	== 'v' &&
					   s[i + 1] == 'e' &&
					   s[i + 2] == 'c' ||
					   
					   s[i] 	== 'm' &&
					   s[i + 1] == 'a' &&
					   s[i + 2] == 't' ||
					   
					   s[i] 	== 'i' &&
					   s[i + 1] == 'n' &&
					   s[i + 2] == 't')
					{
						i += 4;
					}
					
					else if(s[i] == 'f' &&
							s[i + 1] == 'l' &&
							s[i + 2] == 'o' &&
							s[i + 3] == 'a' &&
							s[i + 4] == 't')
					{
						i += 5;
					} 
					// varying is missing its type... 
					else
					{
						return 1;
					}
					
					while(s[i] == ' ' || s[i] == '\n')i++;
				
					while(s[i] != ' ' && s[i] != '\n' && s[i] != '\0' && s[i] != ';' && s[i] != '}' && s[i] != '{')
					{
						name[index++] = s[i++];
					}
					while(s[i] != '\n' && s[i] != ';')i++;
					// this line is missing its semi-colon... 
					if(s[i] != ';')
					{
						return 1;
					}
					i++;
				
					name[index] = '\0';
					
					v = (varying_t *)malloc(sizeof(varying_t));
					v->name = strdup(name);
					v->type = 0;
					v->next = NULL;
					
					if(!vroot)
					{
						vroot = v;
						vlast = vroot;
					}
					else
					{
						vlast->next = v;
						vlast = v;
					}
				}*/
				/*e = i;
				
				for(i = start_pos; i < e; i++)
				{
					s[i] = ' ';
				}
				s[i] = ' ';*/
			return 0;
		}
		else
		{
			return 1;
		}
	}
	
	return 0;

}
	
	
	
	



void shader_ReleaseDefines(define_t *root)
{
	define_t *p = root;
	define_t *b;
	while(p)
	{
		b = p->next;
		free(p->str);
		free(p);
		p = b;
	}
}


#ifdef __cplusplus
}
#endif





















