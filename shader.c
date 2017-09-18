#include "shader.h"
#include "draw.h"
#include "console.h"
#include "camera.h"
#include "file.h"
#include "macros.h"
#include "light.h"

//#define MAX_SHADER_COMPILE_TRIES 10

#define PARANOID


//#define PRINT_INFO

static int shader_path_len;
static char shader_path[256];

static char attrib_names[][32]={"vPosition", 
								"vNormal", 
								"vTangent",
							   "vTexCoord",
							   "vColor"};
							   	
static char capture_varyings[3][10] = {"_vcap_", 
									   "_ncap_", 
									   "_tcap_"};
									   
static char *material_params_uniform_block = {"sysMaterialParamsUniformBlock"};

static char *material_params_uniform_fields[MATERIAL_PARAMS_MAX_NAME_LEN] = { "sysMaterialParams.sysMaterialBaseColor",
																			  "sysMaterialParams.sysMaterialGlossiness",
																			  "sysMaterialParams.sysMaterialMetallic",
																			  "sysMaterialParams.sysMaterialEmissive",
																			  "sysMaterialParams.sysMaterialFlags"};
static int material_params_uniform_offsets[MATERIAL_PARAMS_FIELDS];	
static int material_params_uniform_types[MATERIAL_PARAMS_FIELDS];
int material_params_uniform_buffer_size;



static char *offset_uniform_block = "sysOffsetQueryUniformBlock";

static char *offset_uniform_fields[OFFSET_TYPE_COUNT + 1] = 
{
	"sysOffsets.sysMat4Field",
	"sysOffsets.sysMat3Field",
	"sysOffsets.sysMat2Field",
	"sysOffsets.sysVec4Field",
	"sysOffsets.sysVec3Field",
	"sysOffsets.sysVec2Field",
	"sysOffsets.sysFloatField",
	"sysOffsets.sysIntField",
	"sysOffsets.sysShortField",
	"sysOffsets.sysLastField",
};

int type_offsets[OFFSET_TYPE_COUNT + 1];



int uniform_buffer_alignment;																		  
																			  																		  
													
													

static char *light_params_uniform_block = {"sysLightParamsUniformBlock"};
static char *cluster_uniform_block = {"sysClusterUniformBlock"};

static char *light_params_uniform_name = {"sysLightParams"};

static char *light_params_uniform_fields[MATERIAL_PARAMS_MAX_NAME_LEN] = {
																		  "sysLightParams[0].sysLightProjectionMatrix",
																		  "sysLightParams[0].sysLightOrientation",
																		  "sysLightParams[0].sysLightPosition",
																		  "sysLightParams[0].sysLightRadius",
																		  "sysLightParams[0].sysLightColor",
																		  "sysLightParams[0].sysLightLinearAttenuation",
																		  "sysLightParams[0].sysLightShadowMapOrigin",															
																		  "sysLightParams[0].sysLightQuadraticAttenuation",
																		  "sysLightParams[0].sysLightShadowMapSize",
																		  "sysLightParams[0].sysZNear",
																		  "sysLightParams[0].sysZFar",
																		  "sysLightParams[0].sysLightType",
																		  "sysLightParams[0].sysShadowMapSamples",
																		  };
																		  
																		  
static int light_params_uniform_offsets[LIGHT_PARAMS_FIELDS];	
static int light_params_uniform_types[LIGHT_PARAMS_FIELDS];
int light_params_uniform_buffer_size;	

																	  									   

static char vertex_capture_string[] = "varying vec3 _vcap_;";
static char normal_capture_string[] = "varying vec3 _ncap_;";
static char tangent_capture_string[] = "varying vec3 _tcap_;";


#define UNIFORM_COUNT 38
static char *uniforms[64] = {"sysTime",
							 "sysRenderTargetWidth",
							 "sysRenderTargetHeight",
							 //"sysShadowMapSize",
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
							 //"sys3DShadowSampler",
							 "sysTextureLayer0",
							 "sysZNear",
							 "sysZFar",
							 //"sysLightZNear",
							 //"sysLightZFar",
							 //"sysLightType",
							 "sysLightCount",
							 //"sysMaterialFlags",
							 //"sysFlagShadeless",
							 //"sysFlagDiffuseTexture",
							 //"sysFlagNormalTexture",
							 //"sysFlagGlossTexture",
							 //"sysFlagMetallicTexture",
							 //"sysFlagHeightTexture",
							 //"sysFlagFrontAndBack",
						 	 "sysBloomRadius",
							 "sysBloomIntensity",
							 "sysExposure",
							 "sysRenderDrawMode",
							 "sysCameraToWorldMatrix",
							 //"sysWorldToLightMatrix",
							 //"sysCameraToLightProjectionMatrix",
							 //"sysLightProjectionMatrix",
							 //"sysLightModelViewMatrix",
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
extern int shade_deferred_clustered_shader;
extern int shade_deferred_classic_shader;
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
extern int stencil_lights_shader;

static int capture_count;
static varying_t *vroot = NULL;
static varying_t *vlast;

int light_pick_shader;
int brush_pick_shader;
extern int bm_extensions;

static varying_t *froot = NULL;
static varying_t *flast;

define_t *global_defines = NULL;
define_t *last_added = NULL;
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
	shader_t *init_shader;
	int i;
	int s;
	int init_shader_index;
	unsigned int indexes[MATERIAL_PARAMS_FIELDS + LIGHT_PARAMS_FIELDS];
	char *ext_str;
	
	strcpy(shader_path, path);
	shader_path_len = strlen(shader_path);
	
	
	if(bm_extensions & EXT_UNIFORM_BUFFER_OBJECT)
	{
		shader_AddGlobalDefine("_GL3A_");
	}
	else
	{
		printf("ERROR! Uniform buffer objects not supported by current driver!\n");
		exit(1);
		//shader_AddGlobalDefine("_GL2B_");
	}
	
	
	init_shader_index = shader_LoadShader("init_vert.glsl", "init_frag.glsl", "init");
	if(!(init_shader = shader_GetShaderByIndex(init_shader_index)))
	{
		printf("error loading init shader! aborting...\n");
		exit(-4);
	}

	if(bm_extensions & EXT_UNIFORM_BUFFER_OBJECT)
	{
		
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform_buffer_alignment);
		
		i = glGetUniformBlockIndex(init_shader->shader_ID, material_params_uniform_block);	
		if(i != GL_INVALID_INDEX)
		{			
			glGetActiveUniformBlockiv(init_shader->shader_ID, i, GL_UNIFORM_BLOCK_DATA_SIZE, &material_params_uniform_buffer_size);
		}
		else
		{
			printf("init shader appears to have problems! aborting...\n");
			exit(-5);
		}
		
		
		//glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &i);
		
		/*i = glGetUniformBlockIndex(init_shader->shader_ID, light_params_uniform_block);
		if(i != GL_INVALID_INDEX)
		{
			glGetActiveUniformBlockiv(init_shader->shader_ID, i, GL_UNIFORM_BLOCK_DATA_SIZE, &light_params_uniform_buffer_size);
		}
		else
		{
			printf("init shader appears to have problems! aborting...\n");
			exit(-5);
		}*/
		
		material_params_uniform_buffer_size = (material_params_uniform_buffer_size + uniform_buffer_alignment - 1) & (~(uniform_buffer_alignment - 1));
	//	light_params_uniform_buffer_size = (light_params_uniform_buffer_size / 16 + uniform_buffer_alignment - 1) & (~(uniform_buffer_alignment - 1));
		
		
		//light_params_uniform_buffer_size /= MAX_ACTIVE_LIGHTS;
		
		//printf("%d %d %d\n", material_params_uniform_buffer_size, sizeof(gpu_lamp_t) * MAX_ACTIVE_LIGHTS, light_params_uniform_buffer_size);
		
		//printf("%d %d\n", i, sizeof(gpu_lamp_t) * MAX_ACTIVE_LIGHTS);
		
		
	}
	
	
	shader_DeleteShaderByIndex(init_shader_index);
	
	
	screen_quad_shader_index=shader_LoadShader("screen_quad_vert.glsl", "screen_quad_frag.glsl", "screen_quad");
	composite_shader_index=shader_LoadShader("composite_vert.glsl", "composite_frag.glsl", "composite");
	
	
	lit_shader_index = shader_LoadShader("lit_vert.glsl", "lit_frag.glsl", "lit");
	draw_translucent_shader_index = shader_LoadShader("draw_translucent_vert.glsl", "draw_translucent_frag.glsl", "draw_translucent");
	blend_translucent_shader_index = shader_LoadShader("resolve_translucent_vert.glsl", "resolve_translucent_frag.glsl", "resolve_translucent");
	
	
	shade_deferred_clustered_shader = shader_LoadShader("shade_deferred_clustered_vert.glsl", "shade_deferred_clustered_frag.glsl", "shade_deferred_clustered");
	//shade_deferred_classic_shader = shader_LoadShader("shade_deferred_classic_vert.glsl", "shade_deferred_classic_frag.glsl", "shade_deferred_classic");
	
	wireframe_shader_index=shader_LoadShader("wireframe_vert.glsl", "wireframe_frag.glsl", "wireframe");
	flat_shader_index=shader_LoadShader("flat_vert.glsl", "flat_frag.glsl", "flat");
	smap_shader_index=shader_LoadShader("smap_vert.glsl", "smap_frag.glsl", "smap");
	//plvol_shader_index=shader_LoadShader("volumetric_light_vert.glsl", "volumetric_light_frag.glsl", "volumetric_light");
	//slvol_shader_index=shader_LoadShader("volumetric_spot_light_vert.txt", "volumetric_spot_light_frag.txt", "volumetric_spot_light");
	bl_shader_index=shader_LoadShader("bilateral_blur_vert.glsl", "bilateral_blur_frag.glsl", "bilateral_blur");
	gb_shader_index = shader_LoadShader("gaussian_blur_vert.glsl", "gaussian_blur_frag.glsl", "gaussian_blur");
	extract_intensity_shader_index = shader_LoadShader("extract_intensity_vert.glsl", "extract_intensity_frag.glsl", "extract_intensity");
	bloom_blur_shader_index = shader_LoadShader("bloom_blur_vert.glsl", "bloom_blur_frag.glsl", "bloom_blur");
	draw_buffer_shader_index = shader_LoadShader("debug_draw_buffer_vert.glsl", "debug_draw_buffer_frag.glsl", "draw_buffer");
	draw_z_buffer_shader_index = shader_LoadShader("debug_draw_z_buffer_vert.glsl", "debug_draw_z_buffer_frag.glsl", "draw_z_buffer");
	intensity0_shader_index = shader_LoadShader("intensity_vert.glsl", "intensity0_frag.glsl", "intensity0");
	intensity1_shader_index = shader_LoadShader("intensity_vert.glsl", "intensity1_frag.glsl", "intensity1");
	
	light_pick_shader = shader_LoadShader("light_pick_vert.glsl", "light_pick_frag.glsl", "light_pick");
	brush_pick_shader = shader_LoadShader("brush_pick_vert.glsl", "brush_pick_frag.glsl", "brush_pick");
	stencil_lights_shader = shader_LoadShader("stencil_lights_vert.glsl", "stencil_lights_frag.glsl", "stencil_lights");
	
	//printf("here\n");
	
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
	free(shader_a.shaders);
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
	shader_t *shader;
	int shader_index;
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
	
	/* I guess the path was getting bigger than 256 chars... good to know... */
	char vfull_path[1024];
	char ffull_path[1024];
	
	
	strcpy(vfull_path, shader_path);
	strcpy(ffull_path, shader_path);
	strcat(vfull_path, vertex_shader_name);
	strcat(ffull_path, fragment_shader_name);
	int capture_count = 0;
	char *capture[3];
	
	
	printf("shader %s\n", name);
	
	//unsigned int indexes[MATERIAL_PARAMS_FIELDS];	
	//unsigned int offsets[MATERIAL_PARAMS_FIELDS];


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

	
	//printf("%s\n", v_shader_str);
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

	
	//printf("%s\n", f_shader_str);
	//printf("a-2");
	
	v_shader_obj=glCreateShader(GL_VERTEX_SHADER);
	b_compiled=0;
	
	//printf("a-1");

	glShaderSource(v_shader_obj, 1, (const GLchar **)&v_shader_str, NULL);
	glCompileShader(v_shader_obj);
	glGetShaderiv(v_shader_obj, GL_COMPILE_STATUS, &b_compiled);

	if(!b_compiled)
	{
		error_str=(char *)calloc(8192, 1);
		console_Print(MESSAGE_ERROR, "[%s] vertex shader compilation failed!\nerror dump in external console\n", name);
		glGetShaderInfoLog(v_shader_obj, 8192, NULL, error_str);
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
		error_str=(char *)malloc(8192);
		glGetShaderInfoLog(f_shader_obj, 8192, NULL, error_str);
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
	
	if(shader_a.stack_top > -1)
	{
		shader_index = shader_a.free_stack[shader_a.stack_top--];
	}
	else
	{
		shader_index = shader_a.shader_count++;
		if(shader_index >= shader_a.array_size)
		{
			shader_ResizeShaderArray(shader_a.array_size << 1);
		}
	}
	
	shader = &shader_a.shaders[shader_index];
	
	
	
	shader->v_position = -1;
	shader->v_normal = -1;
	shader->v_tangent = -1;
	//shader_a.shaders[shader_a.shader_count].v_btangent = -1;
	shader->v_tcoord = -1;
	shader->v_color = -1;
	
	
	/* NOTE: attribute location binding MUST be done before shader linking. */
	/* brute force attribs to be bound to the correct positions. 
	glGetAttribLocation is (apparently) giving wrong result on AMD's
	drivers, so this is a CPU side shader attribute parsing mechanism. */
	shader_ParseShaderAttributes(v_shader_str, &attribs[0], &t);
	for(i=0; i<t; i++)
	{
		glBindAttribLocation(shader_prog, i, attrib_names[attribs[i]]);
		*(&shader->v_position+attribs[i])=i;
	}
	
	//printf("%d %d %d %d %d\n", shader->v_position, shader->v_normal, shader->v_tangent, shader->v_tcoord, shader->v_color);
	/*if(capture_count)
	{
		glTransformFeedbackVaryings(shader_prog, capture_count, (const GLchar **)capture, GL_SEPARATE_ATTRIBS);
	}*/
	
	glLinkProgram(shader_prog);			
	glGetProgramiv(shader_prog, GL_LINK_STATUS, &b_compiled);
	
	
	if(!b_compiled)
	{
		error_str=(char *)malloc(8192);
		printf("[%s] shader linking failed!\n", name);
		glGetProgramInfoLog(shader_prog, 8192, NULL, error_str);
		printf("shader failed to link. Not my fault!\nError dump:\n%s", error_str);
		free(error_str);
	}
	else console_Print(MESSAGE_NORMAL, "shader [%s] loaded\n", name);
	
	shader->default_uniforms = (unsigned short *)malloc(sizeof(short ) * (UNIFORM_Last + 1));
	shader->flags = flags;
	
	if(bm_extensions & EXT_UNIFORM_BUFFER_OBJECT)
	{
		if((i = glGetUniformBlockIndex(shader_prog, material_params_uniform_block)) != GL_INVALID_INDEX)
		{
			glUniformBlockBinding(shader_prog, i, MATERIAL_PARAMS_BINDING);
			printf("shader %s has material uniform block %d\n", name, i);
		}
		
		if((i = glGetUniformBlockIndex(shader_prog, light_params_uniform_block)) != GL_INVALID_INDEX)
		{
			glUniformBlockBinding(shader_prog, i, LIGHT_PARAMS_BINDING);
			printf("shader %s has light uniform block %d\n", name, i);
		}
	}
	
	
	/* Flag the shaders for deletion,
	as soon as the shader program gets deleted.*/
	glDeleteShader(v_shader_obj);
	glDeleteShader(f_shader_obj);

	
	for(i = 0; i < UNIFORM_Last; i++)
	{
		shader->default_uniforms[UNIFORM_Time + i] = glGetUniformLocation(shader_prog, uniforms[i]);
	
	}
	
	shader->sysLightCount = glGetUniformLocation(shader_prog, "sysLightCount");
	shader->sysLightIndex = glGetUniformLocation(shader_prog, "sysLightIndex");
	shader->sysClusterTexture = glGetUniformLocation(shader_prog, "sysClusterTexture");

	//printf("%d %d\n", shader->sysLightCount, shader->sysLightIndexes);
	
	//printf("blufs\n");
	//glUniformBlockBinding(shader_prog, 1, LIGHT_PARAMS_BINDING);


	shader->shader_ID=shader_prog;
	shader->name = strdup(name);
	
	free(v_shader_str);
	//printf("nufs\n"); 
	free(f_shader_str);
	

	
	//printf("clufs\n");
	//fclose(vertex_shader_file);
	//fclose(fragment_shader_file);

	return shader_index;
	
}

PEWAPI void shader_DeleteShaderByIndex(int shader_index)
{
	shader_t *shader;
	if(shader_index >= 0 && shader_index < shader_a.shader_count)
	{
		shader = &shader_a.shaders[shader_index];
		
		/*#ifdef PARANOID
			printf("shader %d:[%s] got deleted. ", shader_index, shader->name);
		#endif	*/
			
		free(shader->name);
		free(shader->default_uniforms);
		
		glDeleteProgram(shader->shader_ID);
		shader->shader_ID = 0;
		if(shader_index == shader_a.shader_count - 1)
		{
			shader_a.shader_count--;
			
			/*#ifdef PARANOID
				printf("%d shaders remain\n", shader_a.shader_count);
			#endif */
		}
		else
		{
			shader_a.free_stack[++shader_a.stack_top] = shader_index;
			/*#ifdef PARANOID
				printf("index added to free positions stack\n");
			#endif	*/
		}
		
		
		
		
	}
	
	return;
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

PEWAPI shader_t *shader_GetShaderByIndex(int shader_index)
{
	if(shader_index >= 0 && shader_index < shader_a.shader_count)
	{
		return &shader_a.shaders[shader_index];
	}
	return NULL;
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
	//camera_t *active_camera;
	if(shader_index>=0)
	{
		//active_camera=camera_GetActiveCamera();
		//glUseProgram(shader_a.shaders[shader_index].shader_ID);
		draw_UseProgram(shader_a.shaders[shader_index].shader_ID);
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

/*PEWAPI void shader_UploadMaterialParams(material_t *material)
{
	void *b;
	void *p;
	if(likely(material))
	{
		glBindBuffer(GL_UNIFORM_BUFFER, material->uniform_buffer);
		b = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		
		p = b;
		*((float *)p) = (float)material->diff_color.r / 0xff; 
		p = ((char *)p) + sizeof(float);
		*((float *)p) = (float)material->diff_color.g / 0xff; 
		p = ((char *)p) + sizeof(float);
		*((float *)p) = (float)material->diff_color.b / 0xff; 
		p = ((char *)p) + sizeof(float);
		*((float *)p) = (float)material->diff_color.a / 0xff; 
		p = ((char *)p) + sizeof(float);
		
		p = ((char *)p) + (material_params_uniform_offsets[1] - (((char *)p) - ((char *)b)));
		
		*((float *)p) = (float)material->glossiness / 0xffff;
		
		p = ((char *)p) + (material_params_uniform_offsets[2] - (((char *)p) - ((char *)b)));
		
		*((float *)p) = (float)material->metallic / 0xffff;
		
		p = ((char *)p) + (material_params_uniform_offsets[3] - (((char *)p) - ((char *)b)));
		
		*((float *)p) = ((float)material->emissive / 0xffff) * MAX_MATERIAL_EMISSIVE;
		
		p = ((char *)p) + (material_params_uniform_offsets[4] - (((char *)p) - ((char *)b)));
		
		*((int *)p) = (int)material->bm_flags;
		
		
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		
	}
}*/


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
	shader_t *temp=(shader_t *)malloc(new_size * sizeof(shader_t));
	int *i = (int *)malloc(sizeof(int) * new_size);
	if(shader_a.shaders)
	{
		memcpy(temp, shader_a.shaders, sizeof(shader_t)*shader_a.shader_count);
		free(shader_a.shaders);
		free(shader_a.free_stack);
	}
	else
	{
		shader_a.shader_count = 0;
		shader_a.stack_top = -1;
	}
	shader_a.shaders=temp;
	shader_a.free_stack = i;
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
			
			else if(!strcmp(attrib_name, "vColor"))
			{
				attribs_found[(*attrib_count)++]=ATTRIBUTE_vColor;
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
	cond_t *h;
	
	//printf("preprocess\n");
	
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
					h = shader_CreateCondTree(s, c, m);
					shader_SolveCondTree(s, c, m, h, r);
						
					i = 0;
					line = 0;
					*shader_str = s;
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
				j = i;
				if(s[i + 1] == '/')
				{
					i += 2;
					while(s[i] != '\n' && s[i] != '\0') i++;
				}
				else if(s[i + 1] == '*')
				{
					i += 2;
					
					while(s[i] != '\0')
					{
						if(s[i] == '*' && s[i + 1] == '/')
						{
							i += 2;
							break;
						}
						else if(s[i] == '\n') line++;
						i++;
					}
				}
				else
				{
					break;
				}
				
				shader_RemoveCommented(s, j, i);
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
	long cur;
	long file_len;
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
		name[index] = s[i];
		index++;
		i++;
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
					
//	file_len = 0;
//	while(!feof(f))
//	{
	//	fgetc(f);
	//	file_len++;
	//}
	//rewind(f);
	//file_len = (file_len + 3)
	//file_len++;

	//long size;
	
	//cur = ftell(f);
	fseek(f, 0, SEEK_END);
	file_len = ftell(f);
	rewind(f);
	
	//j = file_len;
	
	//file_len = (file_len + 3) & (~3);
	//fseek(f, cur, SEEK_SET);
	
	/* this will come back to bite me in the ass... */				
	//inc = (char *)calloc(10000, 1);
	inc = (char *)calloc(file_len, 1);
	fread(inc, 1, file_len, f);
	inc[file_len - 1] = '\0';
	//printf("%s\n", inc);
	/*index = 0;
	while(!feof(f))
	{
		inc[index] = fgetc(f);
		index++;
	}*/

	//inc[index - 1] = '\n';
	//inc[index] = '\0';
					
	fclose(f);
					
	t = (char *)malloc(c + file_len);
					
					
	for(j = 0; j < m; j++)
	{
		t[j] = s[j]; 
	}
	k = j;

	for(j = 0; j < file_len && inc[j] != '\0'; j++)
	{
		t[j + k] = inc[j];
	}
	k += j;
					
	for(j = 0; j < c - i; j++)
	{
		t[j + k] = s[j + i];
	}
	t[j + k] = '\0';
	
	//printf("%s\n", t);
					
	free(inc);
	free(s);
	s = t;
					
	*shader_str = s;
	*new_len = old_len + file_len;
	
	return 0;

}

int shader_AddGlobalDefine(char *define_str)
{
	define_t *d = (define_t *)malloc(sizeof(define_t));
	d->str = strdup(define_str);
	d->next = NULL;
	
	if(!global_defines)
	{
		global_defines = d;
	}
	else
	{
		last_added->next = d;
	}
	last_added = d;
}

int shader_RemoveGlobalDefine(char *define_str)
{
	define_t *d = global_defines;
	define_t *p = NULL;
	while(d)
	{
		
		if(!strcmp(d->str, define_str))
		{
			if(!p)
			{
				global_defines = d->next;
			}
			else
			{
				p->next = d->next;
			}
			free(d->str);
			free(d);
			return 1;
		}
		p = d;
		d = d->next;
	}
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
	
	t = global_defines;
	
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

int shader_TestDefine(define_t *defines, char *define)
{
	define_t *t = defines;
	
	while(t)
	{
		if(!strcmp(t->str, define))
		{
			return 1;
		}
		
		t = t->next;
		
	}
	
	t = global_defines;
	
	while(t)
	{
		if(!strcmp(t->str, define))
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


cond_t *shader_CreateCondTree(char *shader_str, unsigned int str_len, unsigned int cur_index)
{
	cond_t *root = NULL;
	cond_t *q;
	cond_t *cur = (cond_t *)0xffffffff;
	cond_t **t;
	cond_t *p;
	unsigned int i = cur_index;
	unsigned int j;
	unsigned int pos;
	int c = str_len;
	char *s = shader_str;
	int level = 0;
	int index = 0;
	short type;
	int exp_index;
	char exp[512];
	
	for(; i < c && cur; i++)
	{
		
		if(s[i] == '#')
		{
			pos = i;
			i++;
			if(s[i] == 'i' && s[i + 1] == 'f')
			{
				i += 2;
				level++;
				
				q = (cond_t *)malloc(sizeof(cond_t));
				q->max_nested = 0;
				q->nested = NULL;
				q->last_nested = NULL;
				q->nested_count = 0;
				q->next_cond = NULL;
				q->last = q;
				q->pos = pos;
				
				if((s[i] 	 == 'n' && 
					s[i + 1] == 'd' && 
					s[i + 2] == 'e' && 
					s[i + 3] == 'f' &&
					s[i + 4] == ' ')
				  )
				{
					q->type = COND_IFNDEF;
					i += 4;
				}
									   
				else if(s[i] 	 == 'd' && 
						s[i + 1] == 'e' && 
						s[i + 2] == 'f' &&
						s[i + 3] == ' ')
				{
					q->type = COND_IFDEF;
					i += 3;
				}
				else
				{
					q->type = COND_IF;
				}
				
				if(!root)
				{
					root = q;
					root->parent_cond = NULL;
					cur = root;
				}
				else
				{
					if(!cur->nested)
					{
						cur->nested = q;
					}
					else
					{
						cur->last_nested->next_cond = q;
					}
					
					cur->last_nested = q;
					
					q->parent_cond = cur;
					cur = q;
				}
				
				while(s[i] == ' ') i++;
				
				if(s[i] == '\n' || s[i] == '\0')
				{
					/* error: no condition... */
				}
				
				exp_index = 0;
				
				while(s[i] != ' ' && s[i] != '\n')
				{
					exp[exp_index++] = s[i++];
				}
				exp[exp_index] = '\0';
				
				q->exp = strdup(exp);
			}
			
			else
			{
				if(s[i] 	== 'e' &&
				   s[i + 1] == 'l' &&
				   s[i + 2] == 'i' &&
				   s[i + 3] == 'f' &&
				   s[i + 4] == ' ')
				{
					i += 4;
					
					while(s[i] == ' ') i++;
					type = 0;
					if(s[i] == '!')
					{
						while(s[i] == ' ') i++;
						q->type = COND_ELIF_NDEF;
					}
					
					if(s[i] 	== 'd' &&
					   s[i + 1] == 'e' &&
					   s[i + 2] == 'f' &&
					   s[i + 3] == 'i' &&
					   s[i + 4] == 'n' &&
					   s[i + 5] == 'e' &&
					   s[i + 6] == 'd' &&
					   s[i + 7] == ' ')
					{
						i += 7;
						if(!type)
						{
							q->type = COND_ELIF_DEF;
						}

					}
					else
					{
						q->type = COND_ELIF;	
					}
					
					p = cur;
				}
				
				
				else if(s[i] 	 == 'e' &&
						s[i + 1] == 'l' &&
						s[i + 2] == 's' &&
						s[i + 3] == 'e' &&
					   (s[i + 4] == ' ' || 
					    s[i + 4] == '\n'))
				{
					i += 4;
					type = COND_ELSE;
					p = cur;		
				}
				
				else if(s[i] 	 == 'e' && 
						s[i + 1] == 'n' &&
						s[i + 2] == 'd' &&
						s[i + 3] == 'i' &&
						s[i + 4] == 'f' &&
					   (s[i + 5] == ' ' || 
					    s[i + 5] == '\n'|| 
						s[i + 5] == '\0'))
				{
					i += 5;
					level--;
					type = COND_ENDIF;
					p = cur->parent_cond;
				}
				
				else
				{
					continue;	
				}
				
				q = (cond_t *)malloc(sizeof(cond_t));
				q->type = type;
				q->nested = NULL;
				q->last_nested = NULL;
				q->nested_count = 0;
				q->last = NULL;
				q->parent_cond = cur;
				q->next_cond = NULL;
				q->pos = pos;
				q->exp = NULL;
					
				cur->last->next_cond = q;
				cur->last = q;
				
				if(cur->parent_cond)
				{
					cur->parent_cond->last_nested = q;
				}
				
				cur = p;
				
				if(type != COND_ENDIF && type != COND_ELSE)
				{
					while(s[i] == ' ') i++;
				
					if(s[i] == '\n' || s[i] == '\0')
					{
						/* error: no condition... */
					}
					
					exp_index = 0;
					
					while(s[i] != ' ' && s[i] != '\n')
					{
						exp[exp_index++] = s[i++];
					}
					exp[exp_index] = '\0';
					
					q->exp = strdup(exp);
						
				}
					
			}
			
		}
		else if(s[i] == '/')
		{
			j = i;
			if(s[i + 1] == '/')
			{
				i += 2;
				while(s[i] != '\n' && s[i] != '\0') i++;
				
				shader_RemoveCommented(s, j, i);
				
			}
			else if(s[i + 1] == '*')
			{
				i += 2;
							
				while(s[i] != '\0')
				{
					if(s[i] == '*' && s[i + 1] == '/')
					{
						i += 2;
						break;
					}
					i++;
				}
				
				shader_RemoveCommented(s, j, i);
			}
			
		}
						
	}
	
	return root;
}

void shader_SolveCondTree(char *shader_str, unsigned int str_len, unsigned int cur_index, cond_t *root, define_t *defines)
{
	cond_t *cur = root;
	cond_t *p;
	cond_t *t;
	cond_t *r;
	
	cond_t *passed;
	int stack_top = -1;
	cond_t *stack[64];
	
	while(cur)
	{
		p = cur;
		passed = NULL;
		while(p && !passed)
		{

			cur = NULL;			
			
			switch(p->type)
			{
				case COND_IFDEF:
				case COND_ELIF_DEF:
					if(shader_TestDefine(defines, p->exp) && !passed)
					{
						passed = p;	
						goto _passed;
					}
					
				goto _delete;
					
				case COND_IFNDEF:
				case COND_ELIF_NDEF:
					if(!shader_TestDefine(defines, p->exp) && !passed)
					{
						passed = p;	
						goto _passed;
					}
				goto _delete;
				
				case COND_IF:
				case COND_ELIF:
					/* TODO: this... */
				break;
				
				case COND_ELSE:
					passed = p;
					goto _passed;
				break;
				
				case 0xffffffff:
					_passed:
					shader_EraseDirectivesOnly(shader_str, passed->pos, passed->next_cond->pos);
					
					
					//printf("%s\n", shader_str);
					
					if(p->nested)
					{
						cur = p->nested;
					}
					
					t = p->next_cond;
					free(p->exp);
					free(p);
					p = t;

					if(p->type == COND_ENDIF)
					{
						t = p->next_cond;
						free(p);
						p = t;
						passed = NULL;
						continue;
					}
				
				case 0xfffffffe:
					_delete:
					t = p;
					stack_top = 0;
					
					shader_EraseInBetweenDirectives(shader_str, p->pos, p->next_cond->pos);
					
					//printf("%s\n", shader_str);
					
					p = p->next_cond;
					do
					{
						if(t->nested)
						{
							stack[stack_top] = t;
							stack_top++;
							t = t->nested;
							continue;
						}
						else
						{
							r = t->next_cond;
							if(t->exp)
							{
								free(t->exp);
							}
							free(t);
							t = r;
							
						}
						
						if(stack_top > 0)
						{
							t = stack[stack_top];
							stack_top--;
						}
					}while(t != p);
					
					if(p)
					{
						if(p->type == COND_ENDIF)
						{
							t = p->next_cond;
							free(p);
							p = t;
							continue;
						}
						else if(passed)
						{
							goto _delete;
						}
					}

				continue;
					
			}	
			p = p->next_cond;
		}	
	}
	
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

void shader_RemoveCommented(char *shader_str, int start_pos, int end_pos)
{
	int i = start_pos;
	int c = end_pos;
	
	for(; i < c; i++)
	{
		shader_str[i] = ' ';
	}
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






















