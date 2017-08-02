#include "draw.h"
//#include "draw_common.h"

#include "draw_debug.h"
#include "draw_profile.h"

#include "model.h"
#include "shader.h"
#include "material.h"
#include "camera.h"
#include "pew.h"
#include "light.h"
#include "matrix.h"
#include "armature.h"
#include "log.h"
#include "macros.h"
#include "text.h"
#include "texture.h"
#include "gui.h"
#include "gpu.h"

#include <stdarg.h>



//extern unsigned int _512_extra_map;
//extern unsigned int extra_framebuffer;
extern font_array font_a;
//#define PRINT_INFO

extern unsigned int gpu_heap;

extern widget_t *widgets;
extern widget_t *top_widget;
extern int widget_count;

extern unsigned int screen_area_mesh_gpu_buffer;

//extern float screen_quad[3 * 4];

extern int ui_font;

static char formated_str[8192];

int resolutions[7][2]=
{
	1920, 1080,
	1600, 900,
	1366, 768,
	1280, 1024,
	1280, 960, 
	1024, 768,
	800, 600
};


/*static float screen_quad[12]=
{
	-1.0, 1.0, -0.1,
	-1.0,-1.0, -0.1,
	 1.0,-1.0, -0.1,
	 1.0, 1.0, -0.1
};*/

//extern float bone_mesh_octahedron[24*3];

extern float color_conversion_lookup_table[256];

//float circle_v[144];

//float cone_v[54];

//void (*draw_DrawFrameFunc)();

mat4_t widget_projection_matrix;

extern renderer_t renderer;
render_queue render_q;
render_queue t_render_q;				/* for transparent objects */
render_queue e_render_q;				/* emissive objects */
render_queue shadow_q;
//extern shadow_queue shadow_q;
//extern gelem_array gelem_a;
extern entity_array entity_a;
extern material_array material_a;
extern shader_array shader_a;
extern camera_array camera_a;
extern texture_array texture_a;
//extern font_array font_a;
extern console_t console;
extern console_font font;
extern light_array light_a;
extern light_array active_light_a;
extern affecting_lights_list affecting_lights;
extern pew_t pew;
extern armature_list_t armature_list;

int screen_quad_shader_index;
int z_prepass_shader_index;
int deferred_process_shader_index;
int deferred_process_wireframe_shader_index;
int composite_shader_index;
int wireframe_shader_index;
int generate_stencil_shader_index;
int flat_shader_index;
int lit_shader_index;
int draw_translucent_shader_index;
int blend_translucent_shader_index;
int smap_shader_index;
int plvol_shader_index;
int slvol_shader_index;
int bl_shader_index;
int gb_shader_index;
int extract_intensity_shader_index;
int bloom_blur_shader_index;
int intensity0_shader_index;
int intensity1_shader_index;

int stencil_lights_shader;


extern int max_lights_per_pass;


framebuffer_t geometry_buffer;
framebuffer_t transparency_buffer;
static framebuffer_t composite_buffer;
framebuffer_t backbuffer;
//gpu_buffer_t debug_buffer;
//gpu_buffer_t screen_quad_buffer;

static framebuffer_t left_buffer;
static framebuffer_t right_buffer;

static framebuffer_t left_volume_buffer;
static framebuffer_t right_volume_buffer;
static framebuffer_t final_volume_buffer;
static framebuffer_t quarter_volume_buffer;


//static framebuffer_t debug_draw_buffer;
framebuffer_t shadow_buffer;
framebuffer_t picking_buffer;

static framebuffer_t half_buffer;
static framebuffer_t ehalf_buffer;
static framebuffer_t quarter_buffer;
static framebuffer_t equarter_buffer;
static framebuffer_t eighth_buffer;
static framebuffer_t eeighth_buffer;


//static unsigned int h_fb;
//static unsigned int q_fb;
//static unsigned int e_fb;

static unsigned int b_fb_l;
static unsigned int b_fb_r;

static unsigned int h_tex_l;
static unsigned int h_tex_r;
static unsigned int h_tex_depth;

static unsigned int q_tex_l;
static unsigned int q_tex_r;
static unsigned int q_tex_depth;

static unsigned int e_tex_l;
static unsigned int e_tex_r;
static unsigned int e_tex_depth;


static unsigned int itexl;
static unsigned int itexr;
static unsigned int cur_itex;

#define DS_TEX_COUNT 9
static unsigned int ds_tex[DS_TEX_COUNT];

static unsigned int itex1;
static unsigned int itex2;
static unsigned int itex3;
static unsigned int itex4;
static unsigned int itex5;
static unsigned int itex6;
static unsigned int itex7;
static unsigned int itex8;
static unsigned int itex9;

static unsigned int prev_itex;


static unsigned int etex0;
static unsigned int etex1;

//static unsigned int ctex0;
//static unsigned int ctex1;

static unsigned int read_etex;
static unsigned int write_etex;

static unsigned int read_itex;
static unsigned int write_itex;

int debug_level;
int debug_flags = 0;

screen_tile_list screen_tiles;

static unsigned int dither_texture;
static int dither_resolution;

static void (*frame_funcs[20])();
static int frame_func_count;

static float small_bloom_radius;
static float medium_bloom_radius;
static float large_bloom_radius;

static int small_bloom_iterations;
static int medium_bloom_iterations;
static int large_bloom_iterations;

static float bloom_intensity;


int draw_calls = 0;
int texture_binds = 0;
int shader_swaps = 0;



int bm_extensions = 0;

/*int clusters_per_row;
int cluster_rows;
int cluster_z_divs;
cluster_ll_t *clusters_light_lists;
cluster_aabb_t *clusters_aabbs;*/




#ifdef __cplusplus
extern "C"
{
#endif

/*
=============
draw_Init
=============
*/
 void draw_Init(int resolution, int mode)
{
	int i = 0;
	int j;
	int res;
	char msb;
	char lsb;
	int width;
	int height;
	float angle=0.0;
	float step=(2.0*3.14159265)/16.0;
	float *p;
	char *ext_str;
	char *sub_str;
	dither_resolution=4;
	
	int top_res = 1 << DS_TEX_COUNT;
	
	
	char *dither_bytes;
	
	if(SDL_Init(SDL_INIT_EVERYTHING)<0)
	{
		//printf("What a terrible fate. SDL didn't cooperate. Bailing out.\n");
		log_LogMessage("SDL: error initializing SDL!");
		exit(-1);
	}
	
	
	if(mode == INIT_DETECT)
	{
		draw_GetScreenResolution(&width, &height);
		renderer.width=width;
		renderer.height=height;
		renderer.screen_width=width;
		renderer.screen_height=height;
		
		if(renderer.screen_width==1920) resolution=RENDERER_1920x1080;
		else if(renderer.screen_width==1600) resolution=RENDERER_1600x900;
		else if(renderer.screen_width==1366) resolution=RENDERER_1366x768;
		else if(renderer.screen_width==1280)
		{
			if(renderer.screen_height==1024) resolution=RENDERER_1280x1024;
			else resolution=RENDERER_1280x960;
		}
		else if(renderer.screen_width==1024) resolution==RENDERER_1024x768;
		else resolution==RENDERER_800x600;
		
	}
	else
	{
		width=resolutions[resolution][0];
		height=resolutions[resolution][1];
		renderer.width=width;
		renderer.height=height;
		
		
		if(mode == INIT_FORCE_FULLSCREEN)
		{
			draw_GetScreenResolution(&width, &height);
		}
		
		renderer.screen_width=width;
		renderer.screen_height=height;
		
		if(renderer.screen_width==1920) resolution=RENDERER_1920x1080;
		else if(renderer.screen_width==1600)resolution=RENDERER_1600x900;
		else if(renderer.screen_width==1366) resolution=RENDERER_1366x768;
		else if(renderer.screen_width==1280)
		{
			if(renderer.screen_height==1024) resolution=RENDERER_1280x1024;
			else resolution=RENDERER_1280x960;
		}
		else if(renderer.screen_width==1024) resolution==RENDERER_1024x768;
		else resolution==RENDERER_800x600;	

	}
	
	if(mode == INIT_FORCE_FULLSCREEN)
	{
		i = SDL_WINDOW_FULLSCREEN;
	}
	
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	renderer.window=SDL_CreateWindow("game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, renderer.screen_width, renderer.screen_height, SDL_WINDOW_OPENGL | i);
	renderer.context=(SDL_GLContext *)SDL_GL_CreateContext(renderer.window);
	renderer.selected_resolution=resolution;
	renderer.frame_count=0;
	renderer.time=0.0;
	
	//printf("cur context: %lld     %lld\n", wglGetCurrentContext(), SDL_GL_GetCurrentContext());
	
	if(mode == INIT_FORCE_FULLSCREEN)
	{
		SDL_SetWindowFullscreen(renderer.window, SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN);
	}
	
	//SDL_GL_SetAttribute(SDL_GL_STENCILSIZE,8);
	//renderer.renderer_flags=RENDERFLAG_DRAW_VOLUMES|RENDERFLAG_USE_SHADOW_MAPS;
	
	SDL_GL_SetSwapInterval(0);
	
	if(glewInit()!=GLEW_NO_ERROR)
	{
		//printf("What a terrible fate. GLEW didn't cooperate. Bailing out.\n");
		log_LogMessage("GLEW: error initializing GLEW!");
		exit(-2);
	}

	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClearStencil(0x00);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	ext_str = (char *)glGetString(GL_EXTENSIONS);
	
	sub_str = strstr(ext_str, "GL_ARB_seamless_cube_map");
	if(sub_str)
	{
		bm_extensions |= EXT_TEXTURE_CUBE_MAP_SEAMLESS;
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	
	sub_str = strstr(ext_str, "GL_EXT_packed_depth_stencil");
	if(sub_str)
	{
		bm_extensions |= EXT_PACKED_DEPTH_STENCIL;
	}
	
	sub_str = strstr(ext_str, "GL_EXT_transform_feedback");
	if(sub_str)
	{
		bm_extensions |= EXT_TRANSFORM_FEEDBACK;
	}
	
	sub_str = strstr(ext_str, "GL_ARB_uniform_buffer_object");
	if(sub_str)
	{
		bm_extensions |= EXT_UNIFORM_BUFFER_OBJECT;
	}
	
	sub_str = strstr(ext_str, "GL_EXT_multi_draw_arrays");
	if(sub_str)
	{
		bm_extensions |= EXT_MULTI_DRAW_ARRAYS;
	}
	
	sub_str = strstr(ext_str, "GL_ARB_multi_draw_indirect");
	if(sub_str)
	{
		bm_extensions |= EXT_MULTI_DRAW_INDIRECT;
	}
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_CW);
	
	render_q.base=NULL;
	render_q.command_buffers=NULL;
	render_q.count=0;
	draw_ResizeRenderQueue(&render_q, 16);
	
	t_render_q.base = NULL;
	t_render_q.command_buffers = NULL;
	t_render_q.count = 0;
	draw_ResizeRenderQueue(&t_render_q, 16);
	
	e_render_q.base = NULL;
	e_render_q.command_buffers = NULL;
	e_render_q.count = 0;
	draw_ResizeRenderQueue(&e_render_q, 16);
	
	shadow_q.base=NULL;
	shadow_q.command_buffers=NULL;
	shadow_q.count=0;
	draw_ResizeRenderQueue(&shadow_q, 16);
	

	
	//screen_quad_buffer=gpu_CreateGPUBuffer(sizeof(float)*3*4, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
	//gpu_FillGPUBuffer(&screen_quad_buffer, sizeof(float)*3, 4, screen_quad);
	
	//debug_buffer=gpu_CreateGPUBuffer(sizeof(float)*3*49, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
	
	backbuffer.id=0;
	backbuffer.width=renderer.screen_width;
	backbuffer.height=renderer.screen_height;
	backbuffer.color_output_count=1;
	
	
	glGenFramebuffers(1, &shadow_buffer.id);
	
	
	geometry_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_STENCIL, 4, GL_RGBA16F, GL_RGBA16F, GL_R16F, GL_RGB16F);
	transparency_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 3, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F);
	left_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_STENCIL, 1, GL_RGBA16F);
	right_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	left_volume_buffer = framebuffer_CreateFramebuffer(renderer.width / 2, renderer.height / 2, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	right_volume_buffer = framebuffer_CreateFramebuffer(renderer.width / 2, renderer.height / 2, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	final_volume_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	quarter_volume_buffer = framebuffer_CreateFramebuffer(renderer.width / 4, renderer.height / 4, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	composite_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	//debug_draw_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA);
	picking_buffer = framebuffer_CreateFramebuffer(renderer.screen_width, renderer.screen_height, GL_DEPTH_COMPONENT, 1, GL_RGBA32F);
	
	
	glGenFramebuffers(1, &b_fb_l);
	glGenFramebuffers(1, &b_fb_r);
	//glGenFramebuffers(1, &e_fb);
	
	//while(glGetError()!= GL_NO_ERROR);
	
	glGenTextures(1, &itexl);
	glBindTexture(GL_TEXTURE_2D, itexl);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itexr);
	glBindTexture(GL_TEXTURE_2D, itexr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &cur_itex);
	glBindTexture(GL_TEXTURE_2D, itexr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
	
	
	/*for(i = 0; i < DS_TEX_COUNT; i++)
	{
		glGenTextures(1, &ds_tex[i]);
		glBindTexture(GL_TEXTURE_2D, ds_tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, top_res, top_res, 0, GL_RGBA, GL_FLOAT, NULL);
		
		top_res >>= 1;
	}*/
	
	
	glGenTextures(1, &itex1);
	glBindTexture(GL_TEXTURE_2D, itex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 256, 256, 0, GL_RGB, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex2);
	glBindTexture(GL_TEXTURE_2D, itex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex3);
	glBindTexture(GL_TEXTURE_2D, itex3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 64, 64, 0, GL_RGB, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex4);
	glBindTexture(GL_TEXTURE_2D, itex4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex5);
	glBindTexture(GL_TEXTURE_2D, itex5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 16, 16, 0, GL_RGB, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex6);
	glBindTexture(GL_TEXTURE_2D, itex6);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 8, 8, 0, GL_RGB, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex7);
	glBindTexture(GL_TEXTURE_2D, itex7);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex8);
	glBindTexture(GL_TEXTURE_2D, itex8);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 2, 2, 0, GL_RGB, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex9);
	glBindTexture(GL_TEXTURE_2D, itex9);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1, 1, 0, GL_RGB, GL_FLOAT, NULL);
	
	
	
	
	
	glGenTextures(1, &etex0);
	glBindTexture(GL_TEXTURE_2D, etex0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &etex1);
	glBindTexture(GL_TEXTURE_2D, etex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
	
	
	
	glGenTextures(1, &h_tex_l);
	glBindTexture(GL_TEXTURE_2D, h_tex_l);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 2, renderer.height / 2, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &h_tex_r);
	glBindTexture(GL_TEXTURE_2D, h_tex_r);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 2, renderer.height / 2, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &h_tex_depth);
	glBindTexture(GL_TEXTURE_2D, h_tex_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderer.width / 2, renderer.height / 2, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	
	glGenTextures(1, &q_tex_l);
	glBindTexture(GL_TEXTURE_2D, q_tex_l);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 4, renderer.height / 4, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &q_tex_r);
	glBindTexture(GL_TEXTURE_2D, q_tex_r);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 4, renderer.height / 4, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &q_tex_depth);
	glBindTexture(GL_TEXTURE_2D, q_tex_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderer.width / 4, renderer.height / 4, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	
	
	
	
	glGenTextures(1, &e_tex_l);
	glBindTexture(GL_TEXTURE_2D, e_tex_l);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 8, renderer.height / 8, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &e_tex_r);
	glBindTexture(GL_TEXTURE_2D, e_tex_r);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 8, renderer.height / 8, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &e_tex_depth);
	glBindTexture(GL_TEXTURE_2D, e_tex_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderer.width / 8, renderer.height / 8, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	
	draw_SetRenderDrawMode(RENDER_DRAWMODE_LIT);
	draw_SetRenderFlags(RENDERFLAG_USE_SHADOW_MAPS | RENDERFLAG_DRAW_LIGHT_VOLUMES);
	//draw_SetDebugFlag(DEBUG_DRAW_LIGHT_LIMITS);
	draw_SetDebugFlag(DEBUG_DRAW_LIGHT_ORIGINS);
	draw_SetDebugFlag(DEBUG_DRAW_OUTLINES);
	//draw_SetDebugFlag(DEBUG_DRAW_ARMATURES);
	//draw_SetDebugFlag(DEBUG_DRAW_ENTITY_ORIGIN);
	//draw_SetDebugFlag(DEBUG_DRAW_COLLIDERS);
	//draw_SetDebugFlag(DEBUG_DISABLED);
	//draw_SetDebugFlag(DEBUG_DRAW_ENTITY_AABB);
	//draw_SetDebugFlag(DEBUG_DRAW_DBUFFER);
	//draw_SetDebugFlag(DEBUG_DRAW_NBUFFER);
	//draw_SetDebugFlag(DEBUG_DRAW_ZBUFFER);
	
	draw_SetBloomParam(BLOOM_SMALL_RADIUS, DEFAULT_SMALL_BLOOM_RADIUS);
	draw_SetBloomParam(BLOOM_MEDIUM_RADIUS, DEFAULT_MEDIUM_BLOOM_RADIUS);
	draw_SetBloomParam(BLOOM_LARGE_RADIUS, DEFAULT_LARGE_BLOOM_RADIUS);
	
	draw_SetBloomParam(BLOOM_SMALL_ITERATIONS, 2);
	draw_SetBloomParam(BLOOM_MEDIUM_ITERATIONS, 2);
	draw_SetBloomParam(BLOOM_LARGE_ITERATIONS, 2);
	
	draw_SetBloomParam(BLOOM_INTENSITY, DEFAULT_BLOOM_INTENSITY);
	
	while(glGetError()!=GL_NO_ERROR);

	
	dither_bytes=(char *)malloc(dither_resolution*dither_resolution);
	
	glGenTextures(1, &dither_texture);
	glBindTexture(GL_TEXTURE_2D, dither_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	dither_bytes[0]=0;
	dither_bytes[1]=8;
	dither_bytes[2]=2;
	dither_bytes[3]=10;
	
	dither_bytes[4]=12;
	dither_bytes[5]=4;
	dither_bytes[6]=14;
	dither_bytes[7]=6;
	
	dither_bytes[8]=3;
	dither_bytes[9]=11;
	dither_bytes[10]=1;
	dither_bytes[11]=9;
	
	dither_bytes[12]=15;
	dither_bytes[13]=7;
	dither_bytes[14]=13;
	dither_bytes[15]=5;
	
	/*dither_bytes[0]=0;
	dither_bytes[1]=1;
	dither_bytes[2]=1;
	dither_bytes[3]=2;
	
	dither_bytes[4]=2;
	dither_bytes[5]=3;
	dither_bytes[6]=3;
	dither_bytes[7]=4;
	
	dither_bytes[8]=4;
	dither_bytes[9]=5;
	dither_bytes[10]=5;
	dither_bytes[11]=6;
	
	dither_bytes[12]=6;
	dither_bytes[13]=7;
	dither_bytes[14]=7;
	dither_bytes[15]=8;*/
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, dither_resolution, dither_resolution, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, dither_bytes);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	free(dither_bytes);
	
	
	CreateOrthographicMatrix(&widget_projection_matrix, -renderer.width * 0.5, renderer.width * 0.5, renderer.height * 0.5, -renderer.height * 0.5, -1.5, 1.5, NULL);
	
	draw_debug_Init();
	//draw_profile_Init();
	
	/*int b[32] = {123};
	int q = gpu_Alloc(128);
	
	gpu_Write(q, 0, b, 32*sizeof(int));
	gpu_Read(q, 0, b, 32*sizeof(int));
	
	for(int g = 0; g < 32; g++)
	{
		printf("%d\n", b[g]);
	}*/
	
	
	
}

/*
=============
draw_Finish
=============
*/
void draw_Finish()
{
	int i;
	
	draw_debug_Finish();
	//draw_profile_Finish();
	
	SDL_DestroyWindow(renderer.window);
	SDL_GL_DeleteContext(renderer.context);
	
	free(render_q.base);
	free(render_q.entity_indexes);
	
	free(shadow_q.base);
	free(shadow_q.entity_indexes);
	
	free(t_render_q.base);
	free(t_render_q.entity_indexes);
	
	free(e_render_q.base);
	free(e_render_q.entity_indexes);

	return;
}

PEWAPI int draw_GetResolutionMode(int width, int height)
{
	if(width == 1920) return RENDERER_1920x1080;
	else if(width == 1600) return RENDERER_1600x900;
	else if(width == 1366) return RENDERER_1366x768;
	else if(width == 1280)
	{
		if(height == 1024) return RENDERER_1280x1024;
		else return RENDERER_1280x960;
	}
	else if(width == 1024) return RENDERER_1024x768;
	else return RENDERER_800x600;
}

int draw_SetResolution(int width, int height, int mode)
{
	int m = draw_GetResolutionMode(width, height);
	
	switch(m)
	{
		case RENDERER_1920x1080:
		case RENDERER_1600x900:
		case RENDERER_1280x1024:
		case RENDERER_1280x960:
		case RENDERER_1366x768:
		case RENDERER_1024x768:
		case RENDERER_800x600:
		
		break;
		
		default:
			return -1;
	}
	
	renderer.selected_resolution = m;
	
	renderer.screen_width = width;
	renderer.screen_height = height;
	
	SDL_SetWindowSize(renderer.window, renderer.screen_width, renderer.screen_height);
	/*if(renderer.width == renderer.screen_width && renderer.height == renderer.screen_height || mode == INIT_FORCE_FULLSCREEN)
	{
		SDL_SetWindowFullscreen(renderer.window, SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN);
	} */
	
	
	framebuffer_DeleteFramebuffer(&geometry_buffer);
	framebuffer_DeleteFramebuffer(&transparency_buffer);
	framebuffer_DeleteFramebuffer(&composite_buffer);
	framebuffer_DeleteFramebuffer(&left_buffer);
	//framebuffer_DeleteFramebuffer(&right_buffer);
	framebuffer_DeleteFramebuffer(&left_volume_buffer);
	framebuffer_DeleteFramebuffer(&right_volume_buffer);
	framebuffer_DeleteFramebuffer(&picking_buffer);
	//framebuffer_DeleteFramebuffer(&half_buffer);
	//framebuffer_DeleteFramebuffer(&ehalf_buffer);
	//framebuffer_DeleteFramebuffer(&quarter_buffer);
	//framebuffer_DeleteFramebuffer(&equarter_buffer);
	//framebuffer_DeleteFramebuffer(&eighth_buffer);
	//framebuffer_DeleteFramebuffer(&eeighth_buffer);
	
	
	glDeleteTextures(1, &h_tex_l);
	glDeleteTextures(1, &h_tex_r);
	glDeleteTextures(1, &h_tex_depth);
	
	glDeleteTextures(1, &q_tex_l);
	glDeleteTextures(1, &q_tex_r);
	glDeleteTextures(1, &q_tex_depth);
	
	glDeleteTextures(1, &e_tex_l);
	glDeleteTextures(1, &e_tex_r);
	glDeleteTextures(1, &e_tex_depth);
	
	
	
	geometry_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 2, GL_RGBA16F, GL_RGBA16F);
	//transparency_buffer = framebuffer_CreateFramebuffer(re)
	left_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 3, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F);
	left_volume_buffer = framebuffer_CreateFramebuffer(renderer.width / 2, renderer.height / 2, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	right_volume_buffer = framebuffer_CreateFramebuffer(renderer.width / 2, renderer.height / 2, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	composite_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	//debug_draw_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA);
	picking_buffer = framebuffer_CreateFramebuffer(renderer.screen_width, renderer.screen_height, GL_DEPTH_COMPONENT, 1, GL_RGB16F);
	
	glGenTextures(1, &h_tex_l);
	glBindTexture(GL_TEXTURE_2D, h_tex_l);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 2, renderer.height / 2, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &h_tex_r);
	glBindTexture(GL_TEXTURE_2D, h_tex_r);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 2, renderer.height / 2, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &h_tex_depth);
	glBindTexture(GL_TEXTURE_2D, h_tex_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderer.width / 2, renderer.height / 2, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	
	glGenTextures(1, &q_tex_l);
	glBindTexture(GL_TEXTURE_2D, q_tex_l);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 4, renderer.height / 4, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &q_tex_r);
	glBindTexture(GL_TEXTURE_2D, q_tex_r);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 4, renderer.height / 4, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &q_tex_depth);
	glBindTexture(GL_TEXTURE_2D, q_tex_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderer.width / 4, renderer.height / 4, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	
	glGenTextures(1, &e_tex_l);
	glBindTexture(GL_TEXTURE_2D, e_tex_l);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 8, renderer.height / 8, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &e_tex_r);
	glBindTexture(GL_TEXTURE_2D, e_tex_r);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderer.width / 8, renderer.height / 8, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &e_tex_depth);
	glBindTexture(GL_TEXTURE_2D, e_tex_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderer.width / 8, renderer.height / 8, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	
	CreateOrthographicMatrix(&widget_projection_matrix, -renderer.width *0.005, renderer.width *0.005, renderer.height *0.005, -renderer.height *0.005, 0.001, 1.5, NULL);
	
	return 0;
	
}

void draw_Fullscreen(int enable)
{
	int width;
	int height;
	if(enable)
	{
		draw_GetScreenResolution(&width, &height);
		
		renderer.screen_width = width;
		renderer.screen_height = height;
		//draw_SetResolution(width, height);
		
		SDL_SetWindowSize(renderer.window, renderer.screen_width, renderer.screen_height);
		SDL_SetWindowFullscreen(renderer.window, SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN);
		
	}
	else
	{
		renderer.screen_width = renderer.width;
		renderer.screen_height = renderer.height;
		//draw_SetResolution(renderer.width, renderer.height);
		SDL_SetWindowSize(renderer.window, renderer.screen_width, renderer.screen_height);
		
		//SDL_SetWindowFullscreen(renderer.window, SDL_WINDOW_OPENGL);
	}
}

/*
=============
draw_Finish
=============
*/
void draw_OpenFrame()
{
	camera_t *active_camera = camera_GetActiveCamera();
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	float c0[] = {active_camera->frustum.zfar, 0.0, 0.0, 0.0};
	//framebuffer_BindFramebuffer(&debug_draw_buffer);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	framebuffer_BindFramebuffer(&right_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glClearColor(0.0, 0.0 ,0.0, 0.0);
	//glClearDepth(1.0);
	framebuffer_BindFramebuffer(&geometry_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 2, c0);
	
	camera_SetCurrentCameraProjectionMatrix();
	
	draw_calls = 0;
	texture_binds = 0;
	shader_swaps = 0;
	
	return;
}


/*
=============
draw_DrawFrame
=============
*/
void draw_DrawFrame()
{
	int i;
	//while(glGetError() != GL_NO_ERROR);
	for(i=0; i<frame_func_count; i++)
	{
		frame_funcs[i]();
	}
	//printf("%x\n", glGetError());
	draw_DrawWidgets();
	
	if(unlikely(console.bm_status&CONSOLE_VISIBLE)) draw_DrawConsole();
	return;
}

/*
=============
draw_CloseFrame
=============
*/
void draw_CloseFrame()
{
	//pew_GetDeltaTime();
	//draw_DrawBitmap();
	//draw_DrawChar(0, &font_a.fonts[0], 0, 0);
	//vertex_data_t *vdata = model_GetVertexData("wheel.obj");
	//draw_debug_DrawVertexData(vec3(6.0, 0.0 ,0.0), model_GetVertexData("_convex_polygon_"));
	//draw_debug_DrawVertexData(vec3(6.0, 0.0 ,0.0), model_GetVertexData("_cone_"));
	//draw_test_DrawVertexData(vec3(-6.0, 0.0 ,0.0), model_GetVertexData("wow.obj"));
	draw_debug_DrawVertexData(vec3(0.0, 0.0 ,-6.0), model_GetVertexData("stairs.obj"));
	
	/*int i = text_GetFontIndex("consola");
	font_t *f = &font_a.fonts[i];
	draw_DrawString(f, 16, 1, 400, "Testing... %d TESTING!!!", 5);*/
	
	SDL_GL_SwapWindow(renderer.window);
	//printf("%f\n", pew_GetDeltaTime());
	renderer.frame_count++;
	renderer.time+=pew.ti.ms_elapsed;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	/* to guarantee that the polygon mode will be GL_FILL on the next frame's start */
	
	/* don't accept any draw command after the frame
	has been closed */
	//glDepthMask(GL_FALSE);
//	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	
	
	//glFlush();
	return;
}


PEWAPI void draw_GetScreenResolution(int *width, int *height)
{
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	*width=display_mode.w;
	*height=display_mode.h;
	return;
}


/*
=============
draw_ResizeRenderQueue
=============
*/
/*void draw_ResizeRenderQueue(render_queue *r_queue, int new_size)
{
	
	command_buffer_t *base=calloc(new_size+2, sizeof(command_buffer_t));
	command_buffer_t *temp=(((int)base)+63)& ~63;
	
	if(likely(r_queue->base))
	{
		memcpy(temp, r_queue->command_buffers, sizeof(command_buffer_t)*r_queue->count);
		free(r_queue->base);
	}
	r_queue->base=base;
	r_queue->command_buffers=temp;
	r_queue->queue_size=new_size;
	
	return;
}*/




void draw_SortRenderQueue(render_queue *queue, int left, int right)
{
	//register int middle;
	register int i; 
	register int j;
	register int middle;
	command_buffer_t temp;
	camera_t *active_camera = camera_GetActiveCamera();
	vec3_t cam_pos = active_camera->world_position;
	vec3_t obj_pos;
	vec3_t middle_cam_vec;
	vec3_t cam_vec;
	//register int temp;
	
	
	
	middle = (left + right) / 2;
	obj_pos = vec3(queue->command_buffers[middle].model_view_matrix.floats[3][0], queue->command_buffers[middle].model_view_matrix.floats[3][1], queue->command_buffers[middle].model_view_matrix.floats[3][2]);
	middle_cam_vec = sub3(obj_pos, cam_pos);
	
	float middle_len = dot3(middle_cam_vec, middle_cam_vec);
	
	i=left;
	j=right;
	do
	{
		obj_pos = vec3(queue->command_buffers[i].model_view_matrix.floats[3][0], 
					   queue->command_buffers[i].model_view_matrix.floats[3][1], 
					   queue->command_buffers[i].model_view_matrix.floats[3][2]);
		
		cam_vec = sub3(obj_pos, cam_pos);			   			 						 
		for(; i < right && dot3(cam_vec, cam_vec) < middle_len; i++)
		{
			obj_pos = vec3(queue->command_buffers[i].model_view_matrix.floats[3][0], 
						   queue->command_buffers[i].model_view_matrix.floats[3][1], 
						   queue->command_buffers[i].model_view_matrix.floats[3][2]);
			cam_vec = sub3(obj_pos, cam_pos);	
		}
		
		
		obj_pos = vec3(queue->command_buffers[j].model_view_matrix.floats[3][0], 
					   queue->command_buffers[j].model_view_matrix.floats[3][1], 
					   queue->command_buffers[j].model_view_matrix.floats[3][2]);
		cam_vec = sub3(obj_pos, cam_pos);
		for(; j > left && dot3(cam_vec, cam_vec) > middle_len; j--)
		{
			obj_pos = vec3(queue->command_buffers[j].model_view_matrix.floats[3][0], 
						   queue->command_buffers[j].model_view_matrix.floats[3][1], 
						   queue->command_buffers[j].model_view_matrix.floats[3][2]);
			cam_vec = sub3(obj_pos, cam_pos);
		}

		if(i<=j)
		{
			memcpy(&temp, &queue->command_buffers[i], sizeof(command_buffer_t));
			memcpy(&queue->command_buffers[i], &queue->command_buffers[j], sizeof(command_buffer_t));
			memcpy(&queue->command_buffers[j], &temp, sizeof(command_buffer_t));
			i++;
			j--;
		}
		
	}while(i<=j);
	if(j>left) draw_SortRenderQueue(queue, left, j);
	if(i<right) draw_SortRenderQueue(queue, i, right);

	
}


/*
=============
draw_FillCommandBuffer
=============
*/
void draw_FillCommandBuffer(command_buffer_t *cb, mesh_t *mesh, int entity_index, int material_index, int texture_layer, mat4_t *model_view_matrix, unsigned int *light_IDs, unsigned int light_count)
{
	int i;
	int c=light_count;
	mat4_t cb_data;
	
	memcpy(&cb->model_view_matrix, model_view_matrix, sizeof(mat4_t));
	
	//MatrixCopy4(&cb->model_view_matrix, model_view_matrix);
	
	/* To allow multi materials per object, indexed drawing will need to be the norm. The command_buffer_t could
	be modified a bit. The material index could be a short, since 64k materials is just a insane amount. It could
	even be turned into a char, but a short will be enough for now. GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP,
	GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS and GL_QUAD_STRIP all fit within 4 bits, so the field
	containing the drawing mode could be seriously compressed. A command buffer would be 
	emmited for a set of triangles with a specific material. It would contain the material index, 
	and a offset for a gpu buffer with indexes properly laid down. */
	
	//*((unsigned int *)&cb->model_view_matrix.floats[0][3]) = mesh->vertex_buffer;
	//*((unsigned int *)&cb->model_view_matrix.floats[0][3]) = gpu_GetAllocStart(mesh->vcache_slot_id);
	*((unsigned int *)&cb->model_view_matrix.floats[0][3]) = mesh->start;
	*((unsigned int *)&cb->model_view_matrix.floats[1][3]) = mesh->vert_count;
	*((unsigned int *)&cb->model_view_matrix.floats[2][3]) = entity_index;
	*((unsigned int *)&cb->model_view_matrix.floats[3][3]) = (mesh->draw_mode << 16) | ((short)material_index);
	
	if(mesh->n_data)
	{
		*((unsigned int *)&cb->model_view_matrix.floats[3][3]) |= CBATTRIBUTE_NORMAL<<16;
	}
	if(mesh->t_c_data)
	{
		*((unsigned int *)&cb->model_view_matrix.floats[3][3]) |= CBATTRIBUTE_TEX_COORD<<16;
	}
	if(mesh->t_data)
	{
		*((unsigned int *)&cb->model_view_matrix.floats[3][3]) |= CBATTRIBUTE_TANGENT<<16;
	}

	return;
}

void draw_FillCommandBuffer128(command_buffer_t *cb, mesh_t *mesh, int entity_index, int material_index, mat4_t *model_view_matrix, mat4_t *last_model_view_matrix)
{
	int i;
	//int c=light_count;
	mat4_t cb_data;
	
	MatrixCopy4(&cb->model_view_matrix, model_view_matrix);
	
	//*((unsigned int *)&cb->model_view_matrix.floats[0][3])=mesh->vertex_buffer;
	*((unsigned int *)&cb->model_view_matrix.floats[0][3]) = mesh->start;
	*((unsigned int *)&cb->model_view_matrix.floats[1][3])=mesh->vert_count;
	*((unsigned int *)&cb->model_view_matrix.floats[2][3])=entity_index;
	*((unsigned int *)&cb->model_view_matrix.floats[3][3])=(mesh->draw_mode << 16) | ((short)material_index);
	
	if(mesh->n_data)
	{
		*((unsigned int *)&cb->model_view_matrix.floats[3][3])|=CBATTRIBUTE_NORMAL<<16;
	}
	if(mesh->t_c_data)
	{
		*((unsigned int *)&cb->model_view_matrix.floats[3][3])|=CBATTRIBUTE_TEX_COORD<<16;
	}
	if(mesh->t_data)
	{
		*((unsigned int *)&cb->model_view_matrix.floats[3][3])|=CBATTRIBUTE_TANGENT<<16;
	}
	
	//MatrixCopy4(&cb->last_model_view_matrix, last_model_view_matrix);
	//cb->last_model_view_matrix.floats[0][3] = entity_index;
	//*((unsigned int *)&cb->last_model_view_matrix.floats[0][3]) = entity_index;
}


void draw_FillShadowCommandBuffer(command_buffer_t *scb, mat4_t *matrix, int vert_count, short material_index, int start, int draw_mode)
{
	memcpy(&scb->model_view_matrix.floats[0][0], &matrix->floats[0][0], sizeof(mat4_t));
	
	//scb->model_view_matrix = *matrix;
	/* the last byte of scb->model_view_matrix.floats[0][3] CANNOT be used for anything. It MUST remain unchanged. It is used to singal
	to draw_DrawShadowMaps that this command_buffer_t contains geometry data instead of light data. */
	*((short *)&scb->model_view_matrix.floats[0][3]) = material_index;
	*(int *)(&scb->model_view_matrix.floats[1][3]) = start;
	*(int *)(&scb->model_view_matrix.floats[2][3]) = vert_count;
	*(int *)(&scb->model_view_matrix.floats[3][3]) = draw_mode;

	
	return;
}


/*
=============
draw_SubmitCommandBuffer
=============
*/
/*void draw_DispatchCommandBuffer(render_queue *rqueue, command_buffer_t *cb)
{
	
	if(likely(rqueue->count<rqueue->queue_size))
	{
		_add:
		memcpy(&rqueue->command_buffers[rqueue->count++], cb, sizeof(command_buffer_t));
		//rqueue->command_buffers[rqueue->count++]=*cb;
		return;
	}
	else
	{
		draw_ResizeRenderQueue(rqueue, rqueue->queue_size<<1);
		goto _add;
		//rqueue->command_buffers[rqueue.count++]=*cb;
	}

	return;
}*/

void draw_DispatchCommandBuffer(render_queue *rqueue, int entity_index, int material_index, int affecting_lights_index, int draw_flags, int start, int vert_count, mat4_t *model_view_matrix)
{
	command_buffer_t *scb;
	
	
	if(likely(rqueue->count<rqueue->queue_size))
	{
		_add:
		rqueue->entity_indexes[rqueue->count] = entity_index;
		scb = &rqueue->command_buffers[rqueue->count++];
		
		//MatrixCopy4(&scb->model_view_matrix, model_view_matrix);	
		memcpy(&scb->model_view_matrix.floats[0][0], &model_view_matrix->floats[0][0], sizeof(mat4_t));
		
		scb->start = start;
		scb->vert_count = vert_count;
		scb->lights_index = affecting_lights_index;
		scb->entity_index = entity_index;
		scb->draw_flags = draw_flags;
		scb->material_index = material_index; 
			
		//*((unsigned int *)&scb->model_view_matrix.floats[0][3]) = start;
		//*((unsigned int *)&scb->model_view_matrix.floats[1][3]) = vert_count;
		//*((unsigned int *)&scb->model_view_matrix.floats[2][2]) = affecting_lights_index;
		//*((unsigned int *)&scb->model_view_matrix.floats[2][3]) = entity_index;
		//*((unsigned int *)&scb->model_view_matrix.floats[3][3]) = (draw_flags << 16) | ((short)material_index);
	
		/*if(mesh->n_data)
		{
			*((unsigned int *)&scb->model_view_matrix.floats[3][3]) |= CBATTRIBUTE_NORMAL<<16;
		}
		if(mesh->t_c_data)
		{
			*((unsigned int *)&scb->model_view_matrix.floats[3][3]) |= CBATTRIBUTE_TEX_COORD<<16;
		}
		if(mesh->t_data)
		{
			*((unsigned int *)&scb->model_view_matrix.floats[3][3]) |= CBATTRIBUTE_TANGENT<<16;
		}*/
		
		//rqueue->count++;
		return;
	}
	else
	{
		draw_ResizeRenderQueue(rqueue, rqueue->queue_size<<1);
		goto _add;
	}

}

int draw_DispatchShadowCommandBuffer(command_buffer_t *cb)
{
	command_buffer_t *scb;
	if(likely(shadow_q.count<shadow_q.queue_size))
	{
		_add_scb:
		//printf("submit cb\n");
		scb = &shadow_q.command_buffers[shadow_q.count];
		//MatrixCopy4(&scb->model_view_matrix, &cb->model_view_matrix);	
		memcpy(&scb->model_view_matrix.floats[0][0], &cb->model_view_matrix.floats[0][0], sizeof(mat4_t));
		shadow_q.count++;
	}
	else
	{
		draw_ResizeRenderQueue(&shadow_q, shadow_q.queue_size<<1);
		goto _add_scb;
	}

	return 0;
}



int draw_FillAndDispatchShadowCommandBuffer(int vert_count, short material_index, int start, int draw_mode, mat4_t *matrix)
{
	//int scb_index=0;
	command_buffer_t *scb;
	if(likely(shadow_q.count<shadow_q.queue_size))
	{
		_add_scb:
		
		scb = &shadow_q.command_buffers[shadow_q.count];
		//MatrixCopy4(&scb->model_view_matrix, matrix);	
		memcpy(&scb->model_view_matrix.floats[0][0], &matrix->floats[0][0], sizeof(mat4_t));

		/* the last byte of scb->model_view_matrix.floats[0][3] CANNOT be used for anything. It MUST remain unchanged. It is used to singal
		to draw_DrawShadowMaps that this command_buffer_t contains geometry data instead of light data. */
		*(short *)(&scb->model_view_matrix.floats[0][3]) = material_index;
		*(unsigned int *)(&scb->model_view_matrix.floats[1][3])=start;
		*(unsigned int *)(&scb->model_view_matrix.floats[2][3])=vert_count;
		*(unsigned int *)(&scb->model_view_matrix.floats[3][3])=draw_mode;
		
		shadow_q.count++;

	
	return 0;
		//scb_index=shadow_q.count;
		//shadow_q.command_buffers[shadow_q.count++]=*scb;
		//return scb_index;
	}
	else
	{
		//scb_index=shadow_q.count;
		draw_ResizeRenderQueue(&shadow_q, shadow_q.queue_size<<1);
		goto _add_scb;
		//shadow_q.command_buffers[shadow_q.count++]=*scb;
	}

	return 0;
	//return shadow_q.count-1;
}


PEWAPI void draw_SetRenderDrawMode(int draw_mode)
{
	switch(draw_mode)
	{
		case RENDER_DRAWMODE_WIREFRAME:
			renderer.render_mode=draw_mode;
			frame_funcs[0]=draw_DrawWireframe;
			//frame_funcs[1] = draw_Dummy;
			glDisable(GL_CULL_FACE);
		break;
		
		case RENDER_DRAWMODE_FLAT:
			renderer.render_mode=draw_mode;
			frame_funcs[0]=draw_DrawFlat;
			//frame_funcs[1] = draw_Dummy;
			glEnable(GL_CULL_FACE);
		break;
		
		//case RENDERMODE_UNLIT:
		case RENDER_DRAWMODE_LIT:
			renderer.render_mode=draw_mode;
			frame_funcs[0] = draw_DrawLit;
			glEnable(GL_CULL_FACE);
		break;
		
		default:
			
		break;	
	}
}


PEWAPI void draw_SetRenderFlags(int flags)
{
	switch(flags)
	{
		case 0:
			renderer.renderer_flags=flags;
			frame_funcs[1] = draw_ResolveGBuffer;
			frame_funcs[2] = draw_Dummy;
			frame_funcs[3] = draw_Dummy;
			frame_funcs[4] = draw_ComposeNoVolNoBloom;
			frame_funcs[5] = draw_BlitToScreen;
			frame_func_count = 6;
		break;
		
		case RENDERFLAG_DRAW_LIGHT_VOLUMES:
			renderer.renderer_flags=flags;
			frame_funcs[1] = draw_DrawLightVolumes;
			frame_funcs[2] = draw_ResolveGBuffer;
			frame_funcs[3] = draw_Dummy;
			frame_funcs[4] = draw_Dummy;
			frame_funcs[5] = draw_ComposeVolNoBloom;
			frame_funcs[6] = draw_BlitToScreen;
			frame_func_count = 7;
		break;
		
		case RENDERFLAG_USE_SHADOW_MAPS:
			renderer.renderer_flags=flags;
			frame_funcs[1] = draw_DrawShadowMaps;
			frame_funcs[2] = draw_ResolveGBuffer;
			frame_funcs[3] = draw_Dummy;
			frame_funcs[4] = draw_Dummy;
			frame_funcs[5] = draw_ComposeNoVolNoBloom;
			frame_funcs[6] = draw_BlitToScreen;
			frame_func_count = 7;
		break;
		
		case RENDERFLAG_USE_SHADOW_MAPS | RENDERFLAG_DRAW_LIGHT_VOLUMES:
			renderer.renderer_flags=flags;
			frame_funcs[1] = draw_DrawShadowMaps;
			frame_funcs[2] = draw_DrawLightVolumes;
			frame_funcs[3] = draw_ResolveGBuffer;
			frame_funcs[4] = draw_Dummy;
			frame_funcs[5] = draw_Dummy;
			frame_funcs[6] = draw_ComposeVolNoBloom;
			frame_funcs[7] = draw_BlitToScreen;
			frame_func_count = 8;
		break;
		
		case RENDERFLAG_USE_BLOOM:
			renderer.renderer_flags = flags;
			frame_funcs[1] = draw_ResolveGBuffer;
			frame_funcs[2] = draw_Dummy;
			frame_funcs[3] = draw_ComposeNoVolBloom;
			frame_funcs[4] = draw_BlitToScreen;
			frame_func_count = 5;
		break;
		
		case RENDERFLAG_USE_BLOOM | RENDERFLAG_DRAW_LIGHT_VOLUMES:
			 
		break;
		
		case RENDERFLAG_USE_BLOOM | RENDERFLAG_USE_SHADOW_MAPS:
			renderer.renderer_flags = flags;
			frame_funcs[1] = draw_DrawShadowMaps;
			frame_funcs[2] = draw_ResolveGBuffer;
			frame_funcs[3] = draw_Dummy;
			frame_funcs[4] = draw_Dummy;
			frame_funcs[5] = draw_ComposeNoVolBloom;
			frame_funcs[6] = draw_BlitToScreen;
			frame_func_count = 7;
		break;
		
		case RENDERFLAG_USE_SHADOW_MAPS | RENDERFLAG_USE_BLOOM | RENDERFLAG_DRAW_LIGHT_VOLUMES:
			renderer.renderer_flags = flags;
			frame_funcs[1] = draw_DrawShadowMaps;
			frame_funcs[2] = draw_DrawLightVolumes;
			frame_funcs[3] = draw_ResolveGBuffer;
			frame_funcs[4] = draw_Dummy;
			frame_funcs[5] = draw_Dummy;
			frame_funcs[6] = draw_Compose;
			frame_funcs[7] = draw_BlitToScreen;
			frame_func_count = 8;
		break;
	}
	
	if(debug_flags)
	{
		frame_funcs[frame_func_count++] = draw_debug_Draw;
	}
	
	/*switch(debug_flags)
	{
		case DEBUG_DISABLED:
		
		break;
		
		case DEBUG_DRAW_AABB:
			
		break;
		
		case DEBUG_DRAW_LIGHT_ORIGINS:
			frame_funcs[frame_func_count++] = draw_debug_DrawLights;
		break;
	}*/
}

PEWAPI void draw_SetDebugLevel(int level)
{
	
	
	
	/*switch(level)
	{
		case DEBUG_DISABLED:
			debug_level=level;
			draw_DrawFrameFunc=draw_DrawFrameNormal;
		break;
		
		case DEBUG_L1:
		case DEBUG_L2:
		case DEBUG_L3:
			debug_level=level;
			draw_DrawFrameFunc=draw_DrawFrameDebug;
		break;
	}*/
	
	return;
}

PEWAPI void draw_SetDebugFlag(int flag)
{
	switch(flag)
	{
		case DEBUG_DISABLED:
			
			if(debug_flags)
			{
				frame_func_count--;
			}
			
			debug_flags = flag;
			return;
		break;
		
		case DEBUG_DRAW_LIGHT_ORIGINS:
		case DEBUG_DRAW_LIGHT_LIMITS:
		case DEBUG_DRAW_ENTITY_ORIGIN:
		case DEBUG_DRAW_ENTITY_AABB:
		case DEBUG_DRAW_COLLIDERS:
		case DEBUG_DRAW_NBUFFER:
		case DEBUG_DRAW_ZBUFFER:
		case DEBUG_DRAW_DBUFFER:
		case DEBUG_DRAW_ARMATURES:
		case DEBUG_DRAW_OUTLINES:
			debug_flags |= flag;
		break;
		
	}
	
	switch(renderer.renderer_flags)
	{
		
		case 0:
			frame_func_count = 6;
		break;
		
		case RENDERFLAG_DRAW_LIGHT_VOLUMES:
			frame_func_count = 7;
		break;
		
		case RENDERFLAG_USE_SHADOW_MAPS:
			frame_func_count = 7;
		break;
		
		case RENDERFLAG_USE_SHADOW_MAPS | RENDERFLAG_DRAW_LIGHT_VOLUMES:
			frame_func_count = 8;
		break;
		
		case RENDERFLAG_USE_BLOOM:
			frame_func_count = 6;
		break;
		
		case RENDERFLAG_USE_BLOOM | RENDERFLAG_DRAW_LIGHT_VOLUMES:
			 
		break;
		
		case RENDERFLAG_USE_BLOOM | RENDERFLAG_USE_SHADOW_MAPS:
			frame_func_count = 7;
		break;
		
		case RENDERFLAG_USE_SHADOW_MAPS | RENDERFLAG_USE_BLOOM | RENDERFLAG_DRAW_LIGHT_VOLUMES:
			frame_func_count = 8;
		break;
		
	}
	
	
	frame_funcs[frame_func_count++] = draw_debug_Draw;
	
}

PEWAPI void draw_ResetDebugFlag(int flag)
{
	int b_decrement = 0;
	
	if(debug_flags)
	{
		b_decrement = 1;
	}
	debug_flags &= ~flag;
	
	if(debug_flags == DEBUG_DISABLED)
	{
		if(b_decrement)
		{
			frame_func_count--;
		}
	}
}

void draw_DrawWireframe()
{
	
	
	int v_byte_count=0;
	int n_byte_count=0;
	int t_byte_count=0;
	int t_c_byte_count=0;
	int offset=0;
	
	unsigned int draw_mode;
	unsigned int vert_count;
	unsigned int material_index;
	unsigned int gpu_buffer;
	unsigned int start;
	unsigned int attrib_flags;
	
	float wcolor[3]={0.0, 0.6, 0.0};
	int i;
	int c;
	//float *gpu_buffer;
	mat4_t model_view_matrix;
	//if(likely(cb))
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wcolor);
	shader_SetShaderByIndex(wireframe_shader_index);
	
	glEnableVertexAttribArray(shader_a.shaders[wireframe_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[wireframe_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, (void *)(0));	

	glMatrixMode(GL_MODELVIEW);
	c=render_q.count;
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	for(i=0; i<c; i++)
	{

		model_view_matrix=render_q.command_buffers[i].model_view_matrix;
		//gpu_buffer=*(unsigned int *)&model_view_matrix.floats[0][3];
		start = *(unsigned int *)&model_view_matrix.floats[0][3];
		//start *= sizeof(vec3_t);
		vert_count=*(unsigned int *)&model_view_matrix.floats[1][3];
		//vert_count *= 3;
		material_index=*(unsigned int *)&model_view_matrix.floats[2][3];
		draw_mode=*(unsigned int *)&model_view_matrix.floats[3][3];
		
		model_view_matrix.floats[2][2] = model_view_matrix.floats[0][0] * model_view_matrix.floats[1][1] - 
										 model_view_matrix.floats[0][1] * model_view_matrix.floats[1][0];
		
		material_index = draw_mode & 0x0000ffff;
		draw_mode = (draw_mode >> 16);
		attrib_flags = draw_mode & 0x00000f00;
		draw_mode &= 0x0000000f;
		
		//draw_mode &= ~(CBATTRIBUTE_NORMAL|CBATTRIBUTE_TANGENT|CBATTRIBUTE_TEX_COORD));
		
		model_view_matrix.floats[0][3]=0.0;
		model_view_matrix.floats[1][3]=0.0;
		model_view_matrix.floats[2][3]=0.0;
		model_view_matrix.floats[3][3]=1.0;
		
		glLoadMatrixf(&model_view_matrix.floats[0][0]);
		
		//glBindBuffer(GL_ARRAY_BUFFER, gpu_buffer);
		//v_byte_count=vert_count*3*sizeof(float);
		
		start /= sizeof(float) * 3;
		
		//glEnableVertexAttribArray(shader_a.shaders[wireframe_shader_index].v_position);
		//glVertexAttribPointer(shader_a.shaders[wireframe_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *)(0));
		draw_DrawArrays(draw_mode, start, vert_count);	
		//glDrawArrays(draw_mode, 0, vert_count);
		
	}
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	draw_ResetRenderQueue();
	
	return;
}

void draw_DrawFlat()
{
	int v_byte_count=0;
	int n_byte_count=0;
	int t_byte_count=0;
	int b_byte_count = 0;
	int t_c_byte_count=0;
	int offset=0;
	
	unsigned int draw_mode;
	unsigned int vert_count;
	unsigned int material_index;
	unsigned int gpu_buffer;
	unsigned int start;
	unsigned int attrib_flags;
	
	float wcolor[3]={0.0, 0.6, 0.0};
	int i;
	int c;
	//float *gpu_buffer;
	mat4_t model_view_matrix;
	//if(likely(cb))
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wcolor);
	shader_SetShaderByIndex(flat_shader_index);	
	//shader_SetCurrentShaderUniform1i(UNIFORM_RenderMode, RENDER_DRAWMODE_FLAT);
	
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	c=render_q.count;

	for(i=0; i<c; i++)
	{
			
		model_view_matrix=render_q.command_buffers[i].model_view_matrix;
		
		//gpu_buffer=*(unsigned int *)&model_view_matrix.floats[0][3];
		start = *(unsigned int *)&model_view_matrix.floats[0][3];
		//start *= sizeof(vec3_t);
		model_view_matrix.floats[0][3]=0.0;
		
		vert_count=*(unsigned int *)&model_view_matrix.floats[1][3];
		//vert_count *= 3;
		model_view_matrix.floats[1][3]=0.0;
		
		//material_index=*(unsigned int *)&model_view_matrix.floats[2][3];
		model_view_matrix.floats[2][3]=0.0;
		
		draw_mode=*(unsigned int *)&model_view_matrix.floats[3][3];
		model_view_matrix.floats[3][3]=1.0;
		
		model_view_matrix.floats[2][2] = model_view_matrix.floats[0][0] * model_view_matrix.floats[1][1] - 
										 model_view_matrix.floats[0][1] * model_view_matrix.floats[1][0];
		
		material_index = draw_mode & 0x0000ffff;
		draw_mode = (draw_mode >> 16);
		attrib_flags = draw_mode & 0x00000f00;
		draw_mode &= 0x0000000f;
		
		
		
		material_SetMaterialByIndex(material_index);
		
		
		
		
		glLoadMatrixf(&model_view_matrix.floats[0][0]);
		
		//glBindBuffer(GL_ARRAY_BUFFER, gpu_buffer);
		v_byte_count=vert_count*3*sizeof(float);
		glEnableVertexAttribArray(shader_a.shaders[flat_shader_index].v_position);
		glVertexAttribPointer(shader_a.shaders[flat_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, (void *)(start));
		
		if(attrib_flags&CBATTRIBUTE_NORMAL)
		{
			n_byte_count=v_byte_count;
			glEnableVertexAttribArray(shader_a.shaders[flat_shader_index].v_normal);
			glVertexAttribPointer(shader_a.shaders[flat_shader_index].v_normal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(v_byte_count + start));
		}
		if(attrib_flags&CBATTRIBUTE_TANGENT)
		{
			t_byte_count=v_byte_count;
			glEnableVertexAttribArray(shader_a.shaders[flat_shader_index].v_tangent);
			glVertexAttribPointer(shader_a.shaders[flat_shader_index].v_tangent, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(v_byte_count+n_byte_count + start));
		}
		if(attrib_flags&CBATTRIBUTE_TEX_COORD)
		{
			t_c_byte_count=vert_count*2*sizeof(float);
			glEnableVertexAttribArray(shader_a.shaders[flat_shader_index].v_tcoord);
			glVertexAttribPointer(shader_a.shaders[flat_shader_index].v_tcoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(v_byte_count + n_byte_count + t_byte_count + start));
		}
		draw_DrawArrays(draw_mode, 0, vert_count);	
		//glDrawArrays(draw_mode, 0, vert_count);
	}
	
	
	draw_ResetRenderQueue();
	
	return;
}


void draw_DrawLit()
{
	int v_byte_count=0;
	short v_position;
	short v_normal;
	short v_tangent;
	short v_tex_coord;
	//int n_byte_count=0;
	//int t_byte_count=0;
	//int b_byte_count = 0;
	//int t_c_byte_count=0;
	//int offset=0;
	camera_t *active_camera = camera_GetActiveCamera();
	unsigned int draw_mode;
	unsigned int vert_count;
	unsigned int material_index;
	//unsigned int gpu_buffer;
	unsigned int start;
	unsigned int q;
	unsigned int attrib_flags;
	int shader_index;
	int entity_index;
	
	int i;
	int c;
	
/*	unsigned long long s;
	unsigned long long e;
	static unsigned long long q = 0;
	static int qc = 0;*/
	
	//framebuffer_BindFramebuffer(&right_buffer);
	
	command_buffer_t cb;
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	
	c=render_q.count;	
	
	#define SAMPLES 60
	
	//draw_SortRenderQueue(&render_q, 0, c-1);	
	for(i=0; i<c; i++)
	{
		//s = _rdtsc();
		
		/* could get rid of this copy... */
		memcpy(&cb.model_view_matrix.floats[0][0], &render_q.command_buffers[i].model_view_matrix, sizeof(mat4_t));
		

		start = cb.start;
		cb.model_view_matrix.floats[0][3] = 0.0;
		
		vert_count = cb.vert_count;
		cb.model_view_matrix.floats[1][3] = 0.0;
		
		entity_index = cb.entity_index;
		cb.model_view_matrix.floats[2][3] = 0.0;
		
		draw_mode = cb.draw_flags;
		material_index = cb.material_index;
		cb.model_view_matrix.floats[3][3] = 1.0;

		attrib_flags = draw_mode & 0x00000f00;
		draw_mode &= 0x0000000f;
		
		/*model_view_matrix.floats[2][0] = model_view_matrix.floats[0][1] * model_view_matrix.floats[1][2] - 
										 model_view_matrix.floats[0][2] * model_view_matrix.floats[1][1];
										 
		model_view_matrix.floats[2][1] = model_view_matrix.floats[0][2] * model_view_matrix.floats[1][0] - 
										 model_view_matrix.floats[0][0] * model_view_matrix.floats[1][2];*/
										 								 
		cb.model_view_matrix.floats[2][2] = cb.model_view_matrix.floats[0][0] * cb.model_view_matrix.floats[1][1] - 
										    cb.model_view_matrix.floats[0][1] * cb.model_view_matrix.floats[1][0];
		
		glLoadMatrixf(&cb.model_view_matrix.floats[0][0]);	
		
		/* this could be cached inside the command buffer... */
		shader_index = material_a.materials[material_index].shader_index;
		
		if(shader_index != renderer.active_shader_index) 
		{
			shader_SetShaderByIndex(shader_index);	
			shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraProjectionMatrix, &active_camera->projection_matrix.floats[0][0]);
			shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, geometry_buffer.width);
			shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, geometry_buffer.height);
			v_position = shader_a.shaders[shader_index].v_position;
			v_normal = shader_a.shaders[shader_index].v_normal;
			v_tangent = shader_a.shaders[shader_index].v_tangent;
			v_tex_coord = shader_a.shaders[shader_index].v_tcoord;
			
			
			
		}
		
		
		v_byte_count = vert_count*3*sizeof(float);
			
		q = 0;
		glEnableVertexAttribArray(v_position);
		glVertexAttribPointer(v_position, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		q += v_byte_count;
		if(attrib_flags&CBATTRIBUTE_NORMAL)
		{	
			glEnableVertexAttribArray(v_normal);
			glVertexAttribPointer(v_normal, 3, GL_FLOAT, GL_FALSE, 0, (void *)(q));
			q += v_byte_count;
		}
		if(attrib_flags&CBATTRIBUTE_TEX_COORD)
		{
			if(attrib_flags&CBATTRIBUTE_TANGENT)
			{				
				glEnableVertexAttribArray(v_tangent);
				glVertexAttribPointer(v_tangent, 3, GL_FLOAT, GL_FALSE, 0, (void *)(q));
				q += v_byte_count;
			}
			glEnableVertexAttribArray(v_tex_coord);
			glVertexAttribPointer(v_tex_coord, 2, GL_FLOAT, GL_FALSE, 0, (void *)(q));
		}
	
		material_SetMaterialByIndex(material_index);

		start /= sizeof(float) * 3;
		
		draw_DrawArrays(draw_mode, start, vert_count);

		
		/*e = _rdtsc();
		q += e - s;
		qc++;
		
		if(qc == SAMPLES)
		{
			q /= SAMPLES;
			qc = 0;
			
			q = 0;
		}*/
		
	}
	
	
	//printf("frame %d with %d cbs\n", renderer.frame_count, c);
	
	//draw_ResetRenderQueue();
	//render_q.count = 0;
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	return;
}

void draw_DrawEmissive()
{
	int v_byte_count=0;
	int n_byte_count=0;
	int t_byte_count=0;
	int b_byte_count = 0;
	int t_c_byte_count=0;
	int offset=0;
	camera_t *active_camera = camera_GetActiveCamera();
	unsigned int draw_mode;
	unsigned int vert_count;
	unsigned int material_index;
	unsigned int gpu_buffer;
	unsigned int start;
	unsigned int attrib_flags;
	int shader_index;
	int entity_index;
	
	int i;
	int c;
	
	//framebuffer_BindFramebuffer(&right_buffer);
	
	mat4_t model_view_matrix;
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	
	c=e_render_q.count;	
	
	framebuffer_BindFramebuffer(&left_buffer);	

	for(i=0; i<c; i++)
	{	
		
		memcpy(&model_view_matrix, &e_render_q.command_buffers[i].model_view_matrix, sizeof(mat4_t));
		start=*(unsigned int *)&model_view_matrix.floats[0][3];
		//start *= sizeof(vec3_t);
		model_view_matrix.floats[0][3]=0.0;
		
		vert_count=*(unsigned int *)&model_view_matrix.floats[1][3];
		//vert_count *= 3;
		model_view_matrix.floats[1][3]=0.0;
		
		entity_index = *(unsigned int *)&model_view_matrix.floats[2][3];
		model_view_matrix.floats[2][3]=0.0;
		
		draw_mode=*(unsigned int *)&model_view_matrix.floats[3][3];
		model_view_matrix.floats[3][3]=1.0;
		
		material_index = draw_mode & 0x0000ffff;
		draw_mode = (draw_mode >> 16);
		attrib_flags = draw_mode & 0x00000f00;
		draw_mode &= 0x0000000f;
		
		model_view_matrix.floats[2][2] = model_view_matrix.floats[0][0] * model_view_matrix.floats[1][1] - 
										 model_view_matrix.floats[0][1] * model_view_matrix.floats[1][0];
		
		glLoadMatrixf(&model_view_matrix.floats[0][0]);
		
		shader_index = material_a.materials[material_index].shader_index;
		
		if(shader_index != renderer.active_shader_index) 
		{
			shader_SetShaderByIndex(shader_index);	
			//shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraProjectionMatrix, &active_camera->projection_matrix.floats[0][0]);
			//shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, geometry_buffer.width);
			//shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, geometry_buffer.height);
			
		}
		
		material_SetMaterialByIndex(material_index);
		v_byte_count=vert_count*3*sizeof(float);
		
		
		
		glEnableVertexAttribArray(shader_a.shaders[shader_index].v_position);
		glVertexAttribPointer(shader_a.shaders[shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, (void *)start);
		
		if(attrib_flags&CBATTRIBUTE_NORMAL)
		{
			n_byte_count=v_byte_count;
			glEnableVertexAttribArray(shader_a.shaders[shader_index].v_normal);
			glVertexAttribPointer(shader_a.shaders[shader_index].v_normal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(v_byte_count + start));
		}
		if(attrib_flags&CBATTRIBUTE_TANGENT)
		{
			t_byte_count=v_byte_count;
			glEnableVertexAttribArray(shader_a.shaders[shader_index].v_tangent);
			glVertexAttribPointer(shader_a.shaders[shader_index].v_tangent, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(v_byte_count+n_byte_count + start));
		}
		if(attrib_flags&CBATTRIBUTE_TEX_COORD)
		{
			t_c_byte_count=vert_count*2*sizeof(float);
			glEnableVertexAttribArray(shader_a.shaders[shader_index].v_tcoord);
			glVertexAttribPointer(shader_a.shaders[shader_index].v_tcoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(v_byte_count + n_byte_count + t_byte_count + start));
		}
		
		glDrawArrays(draw_mode, 0, vert_count);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	return;
}


void draw_DrawTransparencyFlat()
{
	
}


/* a z pre-pass could solve the problem of
nearer objects being refracted by translucent 
surfaces... Could also solve the problem of
unlit surfaces showing up over well lit ones...*/
void draw_DrawTranslucent()
{

	
	int i;
	int c = t_render_q.count;
	
	int j;
	int k;
	
	int m;
	int n;
	register int p;
	
	int max_lights = max_lights_per_pass;
	camera_t *active_camera = camera_GetActiveCamera();
   	int v_byte_count=0;
   	int v_position;
   	int v_normal;
   	int v_tangent;
   	int v_tex_coord;
	//int n_byte_count=0;
	//int t_byte_count=0;
	//int b_byte_count = 0;
	//int t_c_byte_count=0;
	affecting_lights_t *al;
	int offset=0;
	int color_attachments[]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	
	unsigned int draw_mode;
	unsigned int vert_count;
	unsigned int material_index;
	unsigned int start;
	unsigned int entity_index;
	unsigned int attrib_flags;
	unsigned int al_index;
	int shader_index;
	float c0[4] = {0.0, 0.0, 0.0, 0.0};
	float c1[4] = {1.0, 1.0, 1.0, 1.0}; 
	float v[4];
	//GLenum q[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	
	//mat4_t model_view_matrix;
	
	if(!c) return;
	
	command_buffer_t cb;
	glMatrixMode(GL_MODELVIEW);
	//glEnable(GL_BLEND);
	//glDisable(GL_BLEND);
	//glDepthMask(GL_FALSE);
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	//glBlendFunc(GL_ONE, GL_ONE);
	//while(glGetError() != GL_NO_ERROR);
	glBlendFunci(0, GL_ONE, GL_ONE);
	glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunci(2, GL_ONE, GL_ONE);

	framebuffer_BindFramebuffer(&transparency_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//glClearBufferfv(GL_COLOR, 0, c0);
	//glClearBufferfv(GL_COLOR, 2, c0);
	glClearBufferfv(GL_COLOR, 1, c1);
	//glDrawBuffer(GL_NONE);
	
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, geometry_buffer.z_buffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	
	
	/*shader_SetShaderByIndex(z_prepass_shader_index);
	v_position = shader_a.shaders[z_prepass_shader_index].v_position;
	
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	for(i = 0; i < c; i++)
	{
		memcpy(&cb.model_view_matrix.floats[0][0], &t_render_q.command_buffers[i].model_view_matrix, sizeof(mat4_t));

		start = cb.start;
		cb.model_view_matrix.floats[0][3] = 0.0;
		
		vert_count = cb.vert_count;
		cb.model_view_matrix.floats[1][3] = 0.0;
		
		al_index = cb.affecting_lights_index;
		
		entity_index = cb.entity_index;
		cb.model_view_matrix.floats[2][3] = 0.0;
		
		draw_mode = cb.draw_flags;
		material_index = cb.material_index;
		cb.model_view_matrix.floats[3][3] = 1.0;

		attrib_flags = draw_mode & 0x00000f00;
		draw_mode &= 0x0000000f;
										 								 
		cb.model_view_matrix.floats[2][2] = cb.model_view_matrix.floats[0][0] * cb.model_view_matrix.floats[1][1] - 
										    cb.model_view_matrix.floats[0][1] * cb.model_view_matrix.floats[1][0];
		
		glLoadMatrixf(&cb.model_view_matrix.floats[0][0]);	
		
		glEnableVertexAttribArray(v_position);
		glVertexAttribPointer(v_position, 3, GL_FLOAT, GL_FALSE, 0, (void *)start);
		
	}*/
	
	
	
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_CULL_FACE);
	//glDrawBuffers(3, color_attachments);
	for(i=0; i<c; i++)
	{
		memcpy(&cb.model_view_matrix.floats[0][0], &t_render_q.command_buffers[i].model_view_matrix, sizeof(mat4_t));

		start = cb.start;
		cb.model_view_matrix.floats[0][3] = 0.0;
		
		vert_count = cb.vert_count;
		cb.model_view_matrix.floats[1][3] = 0.0;
		
		al_index = cb.lights_index;
		
		entity_index = cb.entity_index;
		cb.model_view_matrix.floats[2][3] = 0.0;
		
		draw_mode = cb.draw_flags;
		material_index = cb.material_index;
		cb.model_view_matrix.floats[3][3] = 1.0;

		attrib_flags = draw_mode & 0x00000f00;
		draw_mode &= 0x0000000f;
		
		/*model_view_matrix.floats[2][0] = model_view_matrix.floats[0][1] * model_view_matrix.floats[1][2] - 
										 model_view_matrix.floats[0][2] * model_view_matrix.floats[1][1];
										 
		model_view_matrix.floats[2][1] = model_view_matrix.floats[0][2] * model_view_matrix.floats[1][0] - 
										 model_view_matrix.floats[0][0] * model_view_matrix.floats[1][2];*/
										 								 
		cb.model_view_matrix.floats[2][2] = cb.model_view_matrix.floats[0][0] * cb.model_view_matrix.floats[1][1] - 
										    cb.model_view_matrix.floats[0][1] * cb.model_view_matrix.floats[1][0];
		
		glLoadMatrixf(&cb.model_view_matrix.floats[0][0]);	
		//glPushMatrix();
		
		/* this could be cached inside the command buffer... */
		shader_index = material_a.materials[material_index].shader_index;
		
		if(shader_index != renderer.active_shader_index) 
		{
			shader_SetShaderByIndex(shader_index);	
			shader_SetCurrentShaderUniform1i(UNIFORM_DepthSampler, 4);
			//shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 1);
			
			v_position = shader_a.shaders[shader_index].v_position;
			v_normal = shader_a.shaders[shader_index].v_normal;
			v_tangent = shader_a.shaders[shader_index].v_tangent;
			v_tex_coord = shader_a.shaders[shader_index].v_tcoord;
		}
		
		//shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_LastModelViewMatrix, &model_view_matrix.floats[0][0]);
		//shader_SetCurrentShaderUniform1f(UNIFORM_EntityIndex, entity_index);
		
		//if(material_index != renderer.active_material_index)
		//{
		material_SetMaterialByIndex(material_index);
		//}

		v_byte_count=vert_count*3*sizeof(float);
		
		
		
		glEnableVertexAttribArray(v_position);
		glVertexAttribPointer(v_position, 3, GL_FLOAT, GL_FALSE, 0, (void *)start);
		start += v_byte_count;
		if(attrib_flags&CBATTRIBUTE_NORMAL)
		{	
			glEnableVertexAttribArray(v_normal);
			glVertexAttribPointer(v_normal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(start));
			start += v_byte_count;
		}
		if(attrib_flags&CBATTRIBUTE_TEX_COORD)
		{
			if(attrib_flags&CBATTRIBUTE_TANGENT)
			{				
				glEnableVertexAttribArray(v_tangent);
				glVertexAttribPointer(v_tangent, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(start));
				start += v_byte_count;
			}
			glEnableVertexAttribArray(v_tex_coord);
			glVertexAttribPointer(v_tex_coord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(start));
		}
		
		al = &affecting_lights.lights[al_index];
		k = al->count;
		//for(j = 0; j < k; j++)
		j = 0;
		do
		{
			glPushMatrix();
			glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]); 
			for(m = 0; m < max_lights && j < k; m++, j++)
			{	
				p = al->light_IDs[j];
				glLightfv(GL_LIGHT0 + m, GL_POSITION, &active_light_a.position_data[p].world_position.floats[0]);
		 		
		 		v[0] = active_light_a.position_data[p].world_orientation.floats[2][0];
				v[1] = active_light_a.position_data[p].world_orientation.floats[2][1];
				v[2] = active_light_a.position_data[p].world_orientation.floats[2][2];
				glLightfv(GL_LIGHT0 + m, GL_SPOT_DIRECTION, v);
		 		
		 		v[0] = (float)active_light_a.params[p].r / 255.0;
		 		v[1] = (float)active_light_a.params[p].g / 255.0;
		 		v[2] = (float)active_light_a.params[p].b / 255.0;
		 		v[3] = active_light_a.position_data[p].radius;
				glLightfv(GL_LIGHT0 + m, GL_DIFFUSE, v);
				
				/* uniform buffers... ? */
				glLightf(GL_LIGHT0 + m, GL_SPOT_CUTOFF, (float)active_light_a.position_data[p].spot_co);
				glLighti(GL_LIGHT0 + m, GL_SPOT_EXPONENT, active_light_a.params[p].spot_e);
				glLightf(GL_LIGHT0 + m, GL_LINEAR_ATTENUATION, (float)active_light_a.params[p].lin_fallof/255.0);
				glLightf(GL_LIGHT0 + m, GL_QUADRATIC_ATTENUATION, (float)active_light_a.params[p].sqr_fallof/255.0);	
			}
			glPopMatrix();
				
			shader_SetCurrentShaderUniform1i(UNIFORM_LightCount, m);
			glDrawArrays(draw_mode, 0, vert_count);
		}while(j < k);
		
		
	}
	
	
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	
}

void draw_Dummy()
{
	
}

void draw_Compose()
{	
	camera_t *active_camera = camera_GetActiveCamera();
	float exposure = active_camera->exposure;
	
	//draw_ResolveTranslucent();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	shader_SetShaderByIndex(composite_shader_index);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	/* additive blending */
	
	//while(glGetError()!=GL_NO_ERROR);
	
	/* lit scene */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	
	/* light volumes  */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, final_volume_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);
	
	/* could add variable exposure code here... */
	
	draw_DrawBloom();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	shader_SetShaderByIndex(composite_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	shader_SetCurrentShaderUniform1f(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, 1.0);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	
	
	
	glBindTexture(GL_TEXTURE_2D, h_tex_l);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	glBindTexture(GL_TEXTURE_2D, q_tex_l);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	glBindTexture(GL_TEXTURE_2D, e_tex_l);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	//printf("%x\n", glGetError());
	
	//glBindTexture(GL_TEXTURE_2D, debug_draw_buffer.color_out1);
	//glDrawArrays(GL_QUADS, 0, 4);
	/*draw_DrawBloom();
	
	glBindTexture(GL_TEXTURE_2D, quarter_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);*/
	
	/* debug lines */
	//glBindTexture(GL_TEXTURE_2D, debug_draw_buffer.color_out1);
	//glDrawArrays(GL_QUADS, 0, 4);
	
	glFlush();
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

}

void draw_ComposeVolNoBloom()
{
	camera_t *active_camera = camera_GetActiveCamera();
	float exposure = active_camera->exposure;
	
	framebuffer_BindFramebuffer(&composite_buffer);
	glClear(GL_COLOR_BUFFER_BIT);
	
	shader_SetShaderByIndex(composite_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glActiveTexture(GL_TEXTURE0);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	//glBlendEquation(GL_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
	
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT); 
	glBindTexture(GL_TEXTURE_2D, final_volume_buffer.color_out1);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);   
	                                    
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glFlush();

}


void draw_ComposeNoVolNoBloom()
{
	camera_t *active_camera = camera_GetActiveCamera();
	float exposure = active_camera->exposure;
	
	//draw_AutoAdjust();
	
	//draw_ResolveTranslucent();
	//glClearDepth(1.0);
	framebuffer_BindFramebuffer(&composite_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	shader_SetShaderByIndex(composite_shader_index);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	//shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, composite_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, composite_buffer.height);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFunci(0, GL_ONE, GL_ONE);
	//glBlendFunc(GL_ONE, GL_ONE);
	/* lit scene */
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, right_buffer.color_out1);
	//glDrawArrays(GL_QUADS, 0, 4);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_DEPTH_TEST);
	
	glFlush();
}


void draw_ComposeNoVolBloom()
{
	
	camera_t *active_camera = camera_GetActiveCamera();
	float exposure = active_camera->exposure;
	
	//void draw_AutoAdjust();
	
	//draw_ResolveTranslucent();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	shader_SetShaderByIndex(composite_shader_index);
	shader_SetCurrentShaderUniform1f(UNIFORM_TextureSampler0, 0);
	//shader_SetCurrentShaderUniform1f(UNIFORM_TextureSampler1, 1);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	/* lit scene */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, write_etex);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	draw_DrawBloom();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	shader_SetShaderByIndex(composite_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, 1.0);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	/* additive blending */
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, h_tex_l);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	glBindTexture(GL_TEXTURE_2D, q_tex_l);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	glBindTexture(GL_TEXTURE_2D, e_tex_l);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}


void draw_FillStencilBuffer()
{
	framebuffer_BindFramebuffer(&geometry_buffer);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	shader_SetShaderByIndex(generate_stencil_shader_index);
	
	
	
	
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
}





void draw_ResolveGBuffer()
{
	
	int i;
	int j;
	int c;
	
	int q;
	int k;
	
	int x;
	int y;
	int pass_count=0;
	mat4_t cam_transform;
	float fx;
	float fy;
	
	light_data0 *position;
	light_data1 *params;
	light_data2 *shadow;
	light_data3 *extra;
	
	mat3_t orientation;
	mat4_t camera_to_world_matrix;
	mat4_t camera_to_light_matrix;
	mat4_t camera_to_light_projection_matrix;
	mat4_t light_transform;
	mat4_t model_view_matrix;
	
	int imin_x;
	int imax_x;
	int imin_y;
	int imax_y;
	
	//float use_shadows;
	int light_type = 0;
	int area_type = 0;
	int use_shadows = 0;
	unsigned int target;
	unsigned int uniform;
	
	float v[4];
	int draw_begin;
	int draw_count;
	int draw_mode;
	
    //affecting_lights_list light_list;
    camera_t *active_camera=camera_GetActiveCamera();
	
	framebuffer_CopyFramebuffer(&left_buffer, &geometry_buffer, COLOR_COMPONENT_DEPTH);
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, left_buffer.id);
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, geometry_buffer.z_buffer, 0);
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, geometry_buffer.z_buffer, 0);
	

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearStencil(0);
	framebuffer_BindFramebuffer(&left_buffer);	
	
	//framebuffer_ResetFramebufferComponents(&left_buffer, COLOR_COMPONENT_DEPTH);
	//framebuffer_ResetFramebufferComponents(&geometry_buffer, COLOR_COMPONENT_DEPTH);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	
	
	
	
	
	
//#define TEST_STENCIL_MF_BUFFER	
	
#ifdef TEST_STENCIL_MF_BUFFER

	glEnable(GL_STENCIL_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glUseProgram(0);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);	
	
	glBegin(GL_QUADS);
	glVertex3f(-0.1, 0.1, -0.5);
	glVertex3f(-0.1, -0.6, -0.5);
	glVertex3f(0.1, -0.1, -0.5);
	glVertex3f(0.1, 0.6, -0.5);
	glEnd();	
	
	glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	
#endif
	
	//glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);	
	
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	
//	#define DEBUG_DRAW
	//if(renderer.render_mode==RENDERMODE_LIT)
	switch(renderer.render_mode)
	{

		case RENDER_DRAWMODE_LIT:
		
		//	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, geometry_buffer.z_buffer, 0);
		//	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, geometry_buffer.z_buffer, 0);
			
			c = active_light_a.light_count;
			
			shader_SetShaderByIndex(stencil_lights_shader);
			glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
			glEnableVertexAttribArray(shader_a.shaders[stencil_lights_shader].v_position);
			glVertexAttribPointer(shader_a.shaders[stencil_lights_shader].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			
			
			
			
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			
			
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);	
			
			glStencilFunc(GL_ALWAYS, 0, 0xff);
			//glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR, GL_KEEP);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR, GL_KEEP);
			
			//glBindBufferRange(GL_UNIFORM_BUFFER, LIGHT_PARAMS_BINDING, light_cache);
			//light_BindLightCache();
			
			light_BindLightCache();
			
			for(i = 0; i < c; i++)
			{
				position = &active_light_a.position_data[i];				
				glLightfv(GL_LIGHT0, GL_POSITION, &position->world_position.floats[0]);
				
				glUniform1iv(shader_a.shaders[stencil_lights_shader].sysLightIndexes, 1, &active_light_a.position_data[i].light_index);
				
				//v[0] = 0;
		 		//v[1] = 0;
		 		//v[2] = 0;
		 		//v[3] = position->radius;
				//glLightfv(GL_LIGHT0, GL_DIFFUSE, v);
				
				glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, (float)position->spot_co);
				
		 		v[0] = position->world_orientation.floats[0][0];
				v[1] = position->world_orientation.floats[0][1];
				v[2] = position->world_orientation.floats[0][2];
				glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, v);
				
				v[0] = position->world_orientation.floats[1][0];
				v[1] = position->world_orientation.floats[1][1];
				v[2] = position->world_orientation.floats[1][2];
				glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, v); 
				
				v[0] = position->world_orientation.floats[2][0];
				v[1] = position->world_orientation.floats[2][1];
				v[2] = position->world_orientation.floats[2][2];
				glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, v);
				
				
				if(position->bm_flags&LIGHT_SPOT)
				{
					draw_begin = DRAW_CONE_LOD0_BEGIN;
					draw_count = DRAW_CONE_LOD0_COUNT;
					area_type = LIGHT_SPOT;
					//glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
					//glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
				}
				else if(position->bm_flags&LIGHT_POINT)
				{
					draw_begin = DRAW_SPHERE_LOD0_BEGIN;
					draw_count = DRAW_SPHERE_LOD0_COUNT;
					area_type = LIGHT_POINT;	
					//glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR, GL_KEEP);
					//glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR, GL_KEEP);
				}
				
				light_SetAreaType(area_type);
				
				glCullFace(GL_BACK);
				//glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
				draw_DrawArrays(GL_TRIANGLES, draw_begin, draw_count);
				
				glCullFace(GL_FRONT);
				//glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
				draw_DrawArrays(GL_TRIANGLES, draw_begin, draw_count);
			}
			
			
			glStencilFunc(GL_NOTEQUAL, 0x0, 0xff);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			
			//framebuffer_ResetFramebufferComponents(&left_buffer, COLOR_COMPONENT_DEPTH);
			//framebuffer_ResetFramebufferComponents(&geometry_buffer, COLOR_COMPONENT_DEPTH);
			
			
			shader_SetShaderByIndex(deferred_process_shader_index);
			glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
			glEnableVertexAttribArray(shader_a.shaders[deferred_process_shader_index].v_position);
			glVertexAttribPointer(shader_a.shaders[deferred_process_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, geometry_buffer.color_out1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, geometry_buffer.color_out2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, geometry_buffer.z_buffer);
				
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		
			
			shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
			shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
			shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler2, 5);
			shader_SetCurrentShaderUniform1i(UNIFORM_DepthSampler, 2);
			
			shader_SetCurrentShaderUniform1i(UNIFORM_2DShadowSampler, 3);
			shader_SetCurrentShaderUniform1i(UNIFORM_3DShadowSampler, 4);
			//shader_SetCurrentShaderUniform1i(UNIFORM_RenderDrawMode, RENDER_DRAWMODE_LIT);
			
			shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_camera->frustum.znear);
			shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_camera->frustum.zfar);
			shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, left_buffer.width);
			shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, left_buffer.height);
			
			mat4_t_compose(&camera_to_world_matrix, &active_camera->world_orientation, active_camera->world_position);
			
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			//glDepthMask(GL_FALSE);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			
			//light_BindLightCache();
			
			for(i = 0; i < c; i++)
			{
				
				position = &active_light_a.position_data[i];
				params = &active_light_a.params[i];
				shadow = &active_light_a.shadow_data[i];
				extra = &active_light_a.extra_data[i];
		 		//light_SetLight(i);
		 		
		 		//mat4_t_compose(&light_transform, &active_light_a.position_data[i].world_orientation, active_light_a.position_data[i].world_position.vec3);
		 		//mat4_t_mult(&model_view_matrix, &light_transform, &active_camera->world_to_camera_matrix);
		 		//glLoadMatrixf(&model_view_matrix.floats[0][0]);
		 		
		 		glUniform1iv(shader_a.shaders[deferred_process_shader_index].sysLightIndexes, 1, &active_light_a.position_data[i].light_index);
		 		
		 		glLightfv(GL_LIGHT0, GL_POSITION, &position->world_position.floats[0]);
		 		
		 		/* right vector */
		 		v[0] = position->world_orientation.floats[0][0];
				v[1] = position->world_orientation.floats[0][1];
				v[2] = position->world_orientation.floats[0][2];
				glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, v);
				
				/* up vector */
				v[0] = position->world_orientation.floats[1][0];
				v[1] = position->world_orientation.floats[1][1];
				v[2] = position->world_orientation.floats[1][2];
				glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, v); 
				
				/* forward vector */
				v[0] = position->world_orientation.floats[2][0];
				v[1] = position->world_orientation.floats[2][1];
				v[2] = position->world_orientation.floats[2][2];
				glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, v);
		 		
		 		
		 		//v[0] = (float)params->r / 255.0;
		 		//v[1] = (float)params->g / 255.0;
		 		//v[2] = (float)params->b / 255.0;
		 		//v[3] = position->radius;
				//glLightfv(GL_LIGHT0, GL_DIFFUSE, v);
				
				
				glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, (float)position->spot_co);
				glLighti(GL_LIGHT0, GL_SPOT_EXPONENT, params->spot_e);
				glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, (float)params->lin_fallof/0xffff);
				glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, (float)params->sqr_fallof/0xffff);
				
				
				/*imin_x=active_light_a.position_data[i].smin_x;
				imax_x=active_light_a.position_data[i].smax_x;
				imin_y=active_light_a.position_data[i].smin_y;
				imax_y=active_light_a.position_data[i].smax_y;*/
				
				light_type = 0;
				
				if(position->bm_flags&LIGHT_SPOT)
				{
					draw_begin = DRAW_CONE_LOD0_BEGIN;
					draw_count = DRAW_CONE_LOD0_COUNT;
					draw_mode = GL_TRIANGLES;
					light_type = LIGHT_SPOT;
					area_type = LIGHT_SPOT;
					
					/*if(active_light_a.position_data[i].tex_index > -1)
					{
						glActiveTexture(GL_TEXTURE5);
						glBindTexture(GL_TEXTURE_2D, texture_a.textures[active_light_a.position_data[i].tex_index].tex_ID);
						glLighti(GL_LIGHT4, GL_SPOT_EXPONENT, 1);
					}
					else
					{
						glLighti(GL_LIGHT4, GL_SPOT_EXPONENT, 0);
					}*/
				}
				else if(position->bm_flags&LIGHT_POINT)
				{
					draw_begin = DRAW_SPHERE_LOD0_BEGIN;
					draw_count = DRAW_SPHERE_LOD0_COUNT;
					
					draw_mode = GL_TRIANGLES;
					area_type = LIGHT_POINT;
					
					light_type = LIGHT_POINT;
				}
				
				/* spot exponent goes to the vertex shader... */
				//glLighti(GL_LIGHT1, GL_SPOT_EXPONENT, area_type);
				light_SetAreaType(area_type);
				
				if(position->bm_flags&LIGHT_GENERATE_SHADOWS && renderer.renderer_flags&RENDERFLAG_USE_SHADOW_MAPS)
				{
					switch(light_type)
					{
						case LIGHT_SPOT:
							glActiveTexture(GL_TEXTURE3);
							glBindTexture(GL_TEXTURE_2D, shadow->shadow_map.shadow_map);
						break;
						
						case LIGHT_POINT:
							glActiveTexture(GL_TEXTURE4);
							glBindTexture(GL_TEXTURE_CUBE_MAP, shadow->shadow_map.shadow_map);
						break;
					}		

					mat4_t_mult(&camera_to_light_matrix, &camera_to_world_matrix, &extra->world_to_light_matrix);
					mat4_t_mult(&camera_to_light_projection_matrix, &camera_to_light_matrix, &extra->light_projection_matrix);

					shader_SetCurrentShaderUniform1f(UNIFORM_LightZNear, shadow->znear);
					shader_SetCurrentShaderUniform1f(UNIFORM_LightZFar, shadow->zfar);
					shader_SetCurrentShaderUniform1f(UNIFORM_ShadowMapSize, params->shadow_map_res * MIN_SHADOW_MAP_RES);
					shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraToLightProjectionMatrix, &camera_to_light_projection_matrix.floats[0][0]);
					use_shadows = 1;
					
				}
				else
				{
					use_shadows = 0;
					//light_type = 0;
				}
				
				/* spot cutoff goes to the fragment shader... */
				//glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, light_type);
				light_SetLightType(light_type);
				glLighti(GL_LIGHT2, GL_SPOT_EXPONENT, params->max_shadow_aa_samples);
				glLighti(GL_LIGHT3, GL_SPOT_EXPONENT, use_shadows);
				draw_DrawArrays(draw_mode, draw_begin, draw_count);
				//glDrawArrays(draw_mode, draw_begin, draw_count);
			}
			
			light_UnbindLightCache();
		break;
		
		case RENDER_DRAWMODE_WIREFRAME:
		case RENDER_DRAWMODE_FLAT:			
			glBindFramebuffer(GL_READ_FRAMEBUFFER, geometry_buffer.id);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glBlitFramebuffer(0, 0, geometry_buffer.width, geometry_buffer.height, 0, 0, left_buffer.width, left_buffer.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		break;
		
		/*case RENDER_DRAWMODE_FLAT:
			shader_SetCurrentShaderUniform1i(UNIFORM_RenderDrawMode, RENDER_DRAWMODE_FLAT);
			glLighti(GL_LIGHT1, GL_SPOT_EXPONENT, 0);
			draw_DrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		break;*/
	}
	
	//glDisable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	//glLighti(GL_LIGHT1, GL_SPOT_CUTOFF, 0);
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, 0);	
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	//glMatrixMode(GL_PROJECTION);
	//glPopMatrix();
	//glMatrixMode(GL_MODELVIEW);
	//glPopMatrix();
	
	//glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void draw_ResolveTranslucent()
{
	//printf("1\n");
	shader_SetShaderByIndex(blend_translucent_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[blend_translucent_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[blend_translucent_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler2, 2);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler3, 3);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, transparency_buffer.color_out1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, transparency_buffer.color_out2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, transparency_buffer.color_out3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunci(0, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	glBlendFunci(1, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	//glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	
	framebuffer_BindFramebuffer(&right_buffer);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	//glBlendFunc(GL_ONE, GL_ONE);
	//framebuffer_BindFramebuffer(&left_buffer);
	//gl
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}


void draw_ProcessTransparencyDeferredBuffer()
{
	
}


/* TODO: frustum culling for point lights... */
void draw_DrawShadowMaps()
{
	//int i;
	//int c;
	//int j;
	//int k;
	//int o;
	//int p;

	
	unsigned int m;
	unsigned int n;
	
	int vert_count;
	int draw_mode;
	//int gpu_buffer;
	int start;
	int shadow_map_id;
	//int alpha_map_id;
	int shadow_map_res;
	int material_index;
	//int cb_count;
	int shadow_map_face;
	mat4_t *model_view_matrix;
	//command_buffer_t *cb asm ("%%esi\n") = shadow_q.command_buffers;
	
	int v_byte_count;
	unsigned int attrib;
	unsigned long long so = 0;
	unsigned long long eo = 0;
	unsigned long long si = 0;
	unsigned long long ei = 0; 
	
	
	//int texture;
	//float c_color[4];
	
	
	
	//c=active_light_a.light_count;
	n=shadow_q.count;
	//k=entity_a.entity_count;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	
	//mat4_t transform;
	//mat4_t light_model_view_projection_matrix;


	//glUseProgram(shader_a.shaders[smap_shader_index].shader_ID);
	//renderer.active_shader_index=smap_shader_index;
	shader_SetShaderByIndex(smap_shader_index);
	//glUniform1f(shader_a.shaders[smap_shader_index].default_uniforms[UNIFORM_Time], renderer.time);
	//shader_SetCurrentShaderUniform1f()
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, (float)shadow_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, (float)shadow_buffer.height);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	
	attrib = shader_a.shaders[smap_shader_index].v_position;
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	//shader_SetCurrentShaderUniform1i(UNIFORM_DepthSampler, 1);
	//shader_SetCurrentShaderUniform1f(UNIFORM_UseShadows, 2.0);
	
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.18, 1.0);
	//glPolygonOffset(0.08, 1.0);
	//glPolygonOffset(-2.0, 1.0);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	//glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE);
	
	//_mm_prefetch(&shadow_q.command_buffers[1], _MM_HINT_T0);
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_buffer.id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	//glClearDepth(1.0);
	
	
	/* this can cause a very sneaky bug... */
	asm volatile
	(
		"movl %0, %%esi\n" : : "m" (shadow_q.command_buffers) : "esi"
	);
	
	for(m=0; m<n; m++)
	{
		
		/* If this command_buffer_t's .floats[0][3] last byte is a non-zero value, it denotes the start of a render queue, and 
		it contains the light's model_view_projection matrix, which is uploaded into the GL_PROJECTION matrix stack,
		shadow map resolution, shadow map id, and which face must be rendered. */
		//printf("%d\n", *(((unsigned char *)&shadow_q.command_buffers[m].model_view_matrix.floats[0][3])+3));
		
		//so = _rdtsc();
		
		//if(*(((unsigned char *)&shadow_q.command_buffers[m].model_view_matrix.floats[0][3])+3))
		//if(*(((unsigned char *)&shadow_q.command_buffers[0].model_view_matrix.floats[0][3])+m+3))
		if((*(((unsigned int *)&shadow_q.command_buffers[m].model_view_matrix.floats[0][3])) & 0xff000000))
		{
			//while(glGetError()!=GL_NO_ERROR);
			/* the texture handles can be converted to unsigned shorts. 65536 is an INSANE
			amount of textures. The shadow map face can be converted
			to a short. shadow_map_id and alpha_map_id can be packed in the same 
			matrix position. shadow_map_res and shadow_map_face can be packed in the same
			matrix position. */
			
			
			model_view_matrix = &shadow_q.command_buffers[m].model_view_matrix;
			
			model_view_matrix->floats[0][3] = model_view_matrix->floats[0][2];
			
			shadow_map_id = *(int *)&model_view_matrix->floats[1][3];
			model_view_matrix->floats[1][3] = model_view_matrix->floats[1][2];
			//alpha_map_id = shadow_map_id & 0x0000ffff;
			shadow_map_id = (shadow_map_id >> 16) & 0x0000ffff;
			
			shadow_map_face = *(int *)&model_view_matrix->floats[2][3];
			model_view_matrix->floats[2][3] = model_view_matrix->floats[2][2];
			shadow_map_res = shadow_map_face & 0x0000ffff;
			shadow_map_face = (shadow_map_face >> 16) & 0x0000ffff;
			
			//printf("%d\n", shadow_map_res * MIN_SHADOW_MAP_RES);
			
			glViewport(0, 0, shadow_map_res * MIN_SHADOW_MAP_RES, shadow_map_res * MIN_SHADOW_MAP_RES);
			
			
			//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, extra_framebuffer);
			//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _512_extra_map, 0);
			//glDrawBuffer(GL_NONE);
			//glClear(GL_DEPTH_BUFFER_BIT);
			//glBindFramebuffer(GL_READ_FRAMEBUFFER, extra_framebuffer);
			
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_map_face, shadow_map_id, 0);
			//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadow_map_face, alpha_map_id, 0);
			glClear(GL_DEPTH_BUFFER_BIT);
			
			//break;
			
			
			

			
			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, _512_extra_map);	/* this is not right, but oh well ... */
			//shader_SetCurrentShaderUniform1i(UNIFORM_DepthSampler, 1);
		
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(&model_view_matrix->floats[0][0]);
			glMatrixMode(GL_MODELVIEW);
		
			//while(!(*(((unsigned char *)&shadow_q.command_buffers[++m].model_view_matrix.floats[0][3])+3)))
			while(!(*(((unsigned int *)&shadow_q.command_buffers[++m].model_view_matrix.floats[0][3])) & 0xff000000))
			{
			//	unsigned long long s = _rdtsc();
			
				//si = _rdtsc();
				
				/* 0x3f800000 -> 1.0f */
				//model_view_matrix = &shadow_q.command_buffers[m].model_view_matrix;

				asm volatile
				(
					"movl %[rq], %%esi\n"
					"movl %[mc], %%ebx\n"
					"shl $6, %%ebx\n"
					"lea (%%ebx, %%esi), %%eax\n"
					
					"movl %%eax, %[mvm]\n"
					"add $0x0c, %%eax\n"
	
					"xor %%ecx, %%ecx\n"		
					"movl (%%eax), %%ebx\n"				
					"movl %%ebx, %[mi]\n"								
					"movl %%ecx, (%%eax)\n"				
		
					"addl $0x10, %%eax\n"				
					"movl (%%eax), %%ebx \n"			
					"movl %%ebx, %[strt]\n"				
					"movl %%ecx, (%%eax)\n"									
					
					"addl $0x10, %%eax\n"				
					"movl (%%eax), %%ebx \n"			
					"movl %%ebx, %[vc]\n"			
					"movl %%ecx, (%%eax)\n"							
					
					"addl $0x10, %%eax\n"				
					"movl (%%eax), %%ebx \n"			
					"movl %%ebx, %[dm]\n"
					"movl $0x3f800000, %%ebx\n"
					"movl %%ebx, (%%eax)\n"		
				
					: 
					[mi] "=m" (material_index), 
					[strt] "=m" (start), 
					[vc] "=m" (vert_count), 
					[dm] "=m" (draw_mode),
					[mvm] "+m" (model_view_matrix) 
					:
					[rq] "m" (shadow_q.command_buffers),
					[mc] "m" (m)
					: 
					"eax", "ebx", "ecx"
				);
				
				/*material_index = *(short *)&model_view_matrix->floats[0][3];
				model_view_matrix->floats[0][3] = 0.0;
				
				start = *(int *)&model_view_matrix->floats[1][3];
				model_view_matrix->floats[1][3] = 0.0;
				
				vert_count=*(int *)&model_view_matrix->floats[2][3];
				model_view_matrix->floats[2][3] = 0.0;
				
				draw_mode=*(int *)&model_view_matrix->floats[3][3];
				model_view_matrix->floats[3][3] = 1.0;*/
				
				glLoadMatrixf(&model_view_matrix->floats[0][0]);

				v_byte_count=vert_count*sizeof(float) * 3;
				
				//glEnableVertexAttribArray(shader_a.shaders[smap_shader_index].v_position);
				//glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, (void *)start);
					
				if(material_a.materials[material_index].bm_flags & MATERIAL_FrontAndBack)
				{
					glDisable(GL_CULL_FACE);
				}
				else
				{
					glEnable(GL_CULL_FACE);
				}
				start /= sizeof(float) * 3;
				draw_DrawArrays(draw_mode, start, vert_count);
				//glDrawArrays(draw_mode, 0, vert_count);
				
				//ei = _rdtsc();
				
			}
			m--;

		}
		
		//e = _rdtsc();
	}
	
	glCullFace(GL_BACK);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_BLEND);
	//glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
	
	//glFlush();
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	//draw_ResetShadowQueue();

}



void draw_DrawLightVolumes()
{
	int i;
	int c;
	int imin_x;
	int imax_x;
	int imin_y;
	int imax_y;
	int light_type;
	int area_type;
	int draw_count;
	int draw_start;
	int project_texture = 0;
	mat3_t orientation;
	mat4_t cam_transform;
	float v[4];
	camera_t *active_camera=camera_GetActiveCamera();
	light_data0 *position;
	light_data1 *params;
	light_data2 *shadow;
	light_data3 *extra;
	mat4_t camera_to_world_matrix;
	mat4_t camera_to_light_matrix;
	mat4_t camera_to_light_projection_matrix;
	
	
	//framebuffer_CopyFramebuffer(&right_volume_buffer, &geometry_buffer, COPY_DEPTH);
	framebuffer_BindFramebuffer(&right_volume_buffer);

	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	


	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dither_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, geometry_buffer.color_out3);
	//glActiveTexture(GL_TEXTURE2);
	
	mat4_t_compose(&camera_to_world_matrix, &active_camera->world_orientation, active_camera->world_position);

	c=active_light_a.light_count;
	
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_ONE, GL_ONE);
	//glEnable(GL_SCISSOR_TEST);
	
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	
	shader_SetShaderByIndex(plvol_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[plvol_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[plvol_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 5);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler2, 1);
	//shader_SetCurrentShaderUniform1i(UNIFORM_DepthSampler, 1);
	shader_SetCurrentShaderUniform1i(UNIFORM_3DShadowSampler, 2);
	shader_SetCurrentShaderUniform1i(UNIFORM_2DShadowSampler, 3);
	
	shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraProjectionMatrix, &active_camera->projection_matrix.floats[0][0]);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_camera->frustum.znear);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_camera->frustum.zfar);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, right_volume_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, right_volume_buffer.height);
		
	
	for(i=0; i<c; i++)
	{
		
		position = &active_light_a.position_data[i];
		params = &active_light_a.params[i];
		shadow = &active_light_a.shadow_data[i];
		extra = &active_light_a.extra_data[i];
		
		if(!(position->bm_flags&LIGHT_DRAW_VOLUME))
		{
			continue;
		}
		if(position->bm_flags&LIGHT_SPOT)
		{
			//shader_to_use=slvol_shader_index;	
			area_type = LIGHT_SPOT;
			light_type = LIGHT_SPOT;
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, shadow->shadow_map.shadow_map);
			draw_count = DRAW_CONE_LOD0_COUNT;
			draw_start = DRAW_CONE_LOD0_BEGIN;
			//glCullFace(GL_BACK);
			
			/*if(position->tex_index)
			{
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, texture_a.textures[position->tex_index].tex_ID);
				glLighti(GL_LIGHT4, GL_SPOT_EXPONENT, 1);
			}
			else
			{
				glLighti(GL_LIGHT4, GL_SPOT_EXPONENT, 0);
			}*/
			//continue;
			
			//glLighti(GL_LIGHT4, GL_SPOT_EXPONENT, 0);
		}
		else if(position->bm_flags&LIGHT_POINT)
		{
			area_type = LIGHT_POINT;
			light_type = LIGHT_POINT;
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, shadow->shadow_map.shadow_map);
			draw_count = DRAW_SPHERE_LOD0_COUNT;
			draw_start = DRAW_SPHERE_LOD0_BEGIN;
		}
		
		v[0] = position->world_position.x;
		v[1] = position->world_position.y;
		v[2] = position->world_position.z;
		v[3] = 1.0;
		glLightfv(GL_LIGHT0, GL_POSITION, v);
		 		
		v[0] = position->world_orientation.floats[2][0];
		v[1] = position->world_orientation.floats[2][1];
		v[2] = position->world_orientation.floats[2][2];
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, v);
				
		/* up vector */
		v[0] = position->world_orientation.floats[1][0];
		v[1] = position->world_orientation.floats[1][1];
		v[2] = position->world_orientation.floats[1][2];
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, v); 
				
		/* right vector */
		v[0] = position->world_orientation.floats[0][0];
		v[1] = position->world_orientation.floats[0][1];
		v[2] = position->world_orientation.floats[0][2];
		glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, v);
		 		
		 		
		v[0] = (float)params->r / 255.0;
		v[1] = (float)params->g / 255.0;
		v[2] = (float)params->b / 255.0;
		v[3] = position->radius;
		glLightfv(GL_LIGHT0, GL_DIFFUSE, v);
				
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, (float)position->spot_co);
		glLighti(GL_LIGHT0, GL_SPOT_EXPONENT, (int)params->spot_e);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, (float)params->lin_fallof/0xffff);
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, (float)params->sqr_fallof/0xffff);
		
		glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, ((float)params->scattering / (float)0xffff) * MAX_LIGHT_VOLUME_SCATTERING);
		glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, ((float)params->energy / (float)0xffff) * MAX_LIGHT_ENERGY);
		glLighti(GL_LIGHT2, GL_SPOT_CUTOFF, params->volume_samples);
		
		if(position->bm_flags & LIGHT_GENERATE_SHADOWS)
		{
			mat4_t_mult(&camera_to_light_matrix, &camera_to_world_matrix, &extra->world_to_light_matrix);
			mat4_t_mult(&camera_to_light_projection_matrix, &camera_to_light_matrix, &extra->light_projection_matrix);
			shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraToLightProjectionMatrix, &camera_to_light_projection_matrix.floats[0][0]);
		}
		
		
		shader_SetCurrentShaderUniform1f(UNIFORM_LightZNear, shadow->znear);
		shader_SetCurrentShaderUniform1f(UNIFORM_LightZFar, shadow->zfar);
		light_SetAreaType(area_type);
		light_SetLightType(light_type);
		
		draw_DrawArrays(GL_TRIANGLES, draw_start, draw_count);
	//	glDrawArrays(GL_TRIANGLES, draw_start, draw_count);

	}

	glFlush();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glDepthMask(GL_TRUE);

	framebuffer_BindFramebuffer(&final_volume_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	shader_SetShaderByIndex(bl_shader_index);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, right_volume_buffer.color_out1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, geometry_buffer.z_buffer);
	
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_DepthSampler, 1);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_camera->frustum.znear);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_camera->frustum.zfar);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, final_volume_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, final_volume_buffer.height);
	
	glDrawArrays(GL_QUADS, 0, 4);
	
	/*framebuffer_BindFramebuffer(&right_volume_buffer);
	shader_SetShaderByIndex(bloom_blur_shader_index);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_volume_buffer.color_out1);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_camera->frustum.znear);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_camera->frustum.zfar);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, 0);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, left_volume_buffer.height);
	shader_SetCurrentShaderUniform1f(UNIFORM_BloomRadius, 8.0);
	shader_SetCurrentShaderUniform1f(UNIFORM_BloomIntensity, 1.0);
	glDrawArrays(GL_QUADS, 0, 4);
	
	
	framebuffer_BindFramebuffer(&left_volume_buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, right_volume_buffer.color_out1);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, right_volume_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, right_volume_buffer.height);
	glDrawArrays(GL_QUADS, 0, 4);*/
	
	
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, quarter_volume_buffer.id);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, left_volume_buffer.id);
	//glBlitFramebuffer(0, 0, left_volume_buffer.width, left_volume_buffer.height, 0, 0, quarter_volume_buffer.width, quarter_volume_buffer.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	/*glBindFramebuffer(GL_DRAW_FRAMEBUFFER, left_volume_buffer.id);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, quarter_volume_buffer.id);
	glBlitFramebuffer(0, 0, quarter_volume_buffer.width, quarter_volume_buffer.height, 0, 0, left_volume_buffer.width, left_volume_buffer.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);*/
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, final_volume_buffer.id);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, left_volume_buffer.id);
	//while(glGetError() != GL_NO_ERROR);
	//glBlitFramebuffer(0, 0, left_volume_buffer.width, left_volume_buffer.height, 0, 0, final_volume_buffer.width, final_volume_buffer.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//printf("%x\n", glGetError());
	/*for(i = 0; i < 4; i++)
	{
		framebuffer_BindFramebuffer(&volume_buffer_dithered);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, volume_buffer_final.color_out1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, volume_buffer_final.z_buffer);
		glDrawArrays(GL_QUADS, 0, 4);
		
		framebuffer_BindFramebuffer(&volume_buffer_final);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, volume_buffer_dithered.color_out1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, volume_buffer_dithered.z_buffer);
		glDrawArrays(GL_QUADS, 0, 4);
	}*/
	
	glFlush();

}

void draw_DrawBloom()
{
	
	int i;
	
	//glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buffer.buffer_ID);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	shader_SetShaderByIndex(extract_intensity_shader_index);
	
	glEnableVertexAttribArray(shader_a.shaders[extract_intensity_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[extract_intensity_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderVertexAttribArray(ATTRIBUTE_vPosition);
	shader_SetCurrentShaderUniform1f(UNIFORM_BloomIntensity, 1.0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, composite_buffer.color_out1);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b_fb_l);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, h_tex_l, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, h_tex_depth, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, renderer.width / 2, renderer.height / 2);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_DrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	//glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	glEnable(GL_BLEND);
	glBlendFunci(0, GL_ONE, GL_ONE);
	
	
	
	shader_SetShaderByIndex(bloom_blur_shader_index);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	//shader_SetCurrentShaderVertexAttribArray(ATTRIBUTE_vPosition);
	glEnableVertexAttribArray(shader_a.shaders[bloom_blur_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[bloom_blur_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, renderer.height / 2);
	shader_SetCurrentShaderUniform1f(UNIFORM_BloomRadius, small_bloom_radius);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, b_fb_l);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b_fb_r);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, q_tex_l, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, q_tex_depth, 0);
	glBlitFramebuffer(0, 0, renderer.width >> 1, renderer.height >> 1, 0, 0, renderer.width >> 2, renderer.height >> 2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b_fb_l);
	for(i = 0; i < small_bloom_iterations; i++)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, h_tex_r, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, renderer.width / 2);
		glBindTexture(GL_TEXTURE_2D, h_tex_l);
		glClear(GL_COLOR_BUFFER_BIT);
		draw_DrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		//glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, h_tex_l, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, 0);
		glBindTexture(GL_TEXTURE_2D, h_tex_r);
		glClear(GL_COLOR_BUFFER_BIT);
		draw_DrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		//glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	}
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, b_fb_r);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b_fb_l);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e_tex_l, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, e_tex_depth, 0);
	glBlitFramebuffer(0, 0, renderer.width >> 2, renderer.height >> 2, 0, 0, renderer.width >> 3, renderer.height >> 3, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glViewport(0, 0, renderer.width / 4, renderer.height / 4);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b_fb_r);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, renderer.height / 4);
	shader_SetCurrentShaderUniform1f(UNIFORM_BloomRadius, medium_bloom_radius);
	for(i = 0; i < medium_bloom_iterations; i++)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, q_tex_r, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, renderer.width / 4);
		glBindTexture(GL_TEXTURE_2D, q_tex_l);
		glClear(GL_COLOR_BUFFER_BIT);
		draw_DrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		//glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, q_tex_l, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, 0);
		glBindTexture(GL_TEXTURE_2D, q_tex_r);
		glClear(GL_COLOR_BUFFER_BIT);
		draw_DrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		//glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b_fb_l);
	glViewport(0, 0, renderer.width / 8, renderer.height / 8);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, renderer.height / 8);
	shader_SetCurrentShaderUniform1f(UNIFORM_BloomRadius, large_bloom_radius);
	for(i = 0; i < large_bloom_iterations; i++)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e_tex_r, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, renderer.width / 8);
		glBindTexture(GL_TEXTURE_2D, e_tex_l);
		glClear(GL_COLOR_BUFFER_BIT);
		draw_DrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		//glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e_tex_l, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, 0);
		glBindTexture(GL_TEXTURE_2D, e_tex_r);
		glClear(GL_COLOR_BUFFER_BIT);
		draw_DrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		//glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	}
	
	
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void draw_AutoAdjust()
{
	int i;
	camera_t *active_camera = camera_GetActiveCamera();
	float exposure = active_camera->exposure;
	int res[10][2] = {512, 512,
					 256, 256,
					 128, 128,
					 64, 64,
					 32, 32,
					 16, 16,
					 8,8,
					 4,4,
					 2,2,
					 1,1};
	
	shader_SetShaderByIndex(intensity0_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[intensity0_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[intensity0_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
	
	if(!(renderer.frame_count%2))
	{
		read_itex = itexl;
		write_itex = itexr;
		
		read_etex = etex0;
		write_etex = etex1;
	}
	else
	{
		read_itex = itexr;
		write_itex = itexl;
		
		read_etex = etex1;
		write_etex = etex0;
	}

	
	
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b_fb_l);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, write_itex, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	
	glViewport(0, 0, res[0][0], res[0][1]);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, b_fb_r);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, b_fb_l);
	
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ds_tex[0], 0);
	//glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, write_itex, 0);
	//glBlitFramebuffer(0, 0, res[0][0], res[0][1], 0, 0, res[1][0], res[1][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	//for(i = 1; i < DS_TEX_COUNT - 1; i++)
	{
		
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ds_tex[i], 0);
		//glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ds_tex[i - 1], 0);
		//glBlitFramebuffer(0, 0, res[i][0], res[i][1], 0, 0, res[i + 1][0], res[i + 1][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex1, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, write_itex, 0);
		glBlitFramebuffer(0, 0, res[0][0], res[0][1], 0, 0, res[1][0], res[1][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex2, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex1, 0);
		glBlitFramebuffer(0, 0, res[1][0], res[1][1], 0, 0, res[2][0], res[2][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);	
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex3, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex2, 0);
		glBlitFramebuffer(0, 0, res[2][0], res[2][1], 0, 0, res[3][0], res[3][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex4, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex3, 0);
		glBlitFramebuffer(0, 0, res[3][0], res[3][1], 0, 0, res[4][0], res[4][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex5, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex4, 0);
		glBlitFramebuffer(0, 0, res[4][0], res[4][1], 0, 0, res[5][0], res[5][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex6, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex5, 0);
		glBlitFramebuffer(0, 0, res[5][0], res[5][1], 0, 0, res[6][0], res[6][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex7, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex6, 0);
		glBlitFramebuffer(0, 0, res[6][0], res[6][1], 0, 0, res[7][0], res[7][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex8, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex7, 0);
		glBlitFramebuffer(0, 0, res[7][0], res[7][1], 0, 0, res[8][0], res[8][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex9, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex8, 0);
		glBlitFramebuffer(0, 0, res[8][0], res[8][1], 0, 0, res[9][0], res[9][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, write_itex, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex9, 0);
		glBlitFramebuffer(0, 0, res[9][0], res[9][1], 0, 0, res[0][0], res[0][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
	
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, write_itex, 0);
	//glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ds_tex[i], 0);
	//glBlitFramebuffer(0, 0, res[i][0], res[i][1], 0, 0, res[0][0], res[0][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	shader_SetShaderByIndex(intensity1_shader_index);
	glEnableVertexAttribArray(shader_a.shaders[intensity1_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[intensity1_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, read_itex);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, read_etex);
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, write_etex, 0);
	glViewport(0, 0, res[0][0], res[0][1]);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	
	/*for(i = 0; i < 8; i++)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex1, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, etex, 0);
		glBlitFramebuffer(0, 0, res[0][0], res[0][1], 0, 0, res[1][0], res[1][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex2, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex1, 0);
		glBlitFramebuffer(0, 0, res[1][0], res[1][1], 0, 0, res[2][0], res[2][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);	
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex3, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex2, 0);
		glBlitFramebuffer(0, 0, res[2][0], res[2][1], 0, 0, res[3][0], res[3][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex4, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex3, 0);
		glBlitFramebuffer(0, 0, res[3][0], res[3][1], 0, 0, res[4][0], res[4][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, etex, 0);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, itex4, 0);
		glBlitFramebuffer(0, 0, res[4][0], res[4][1], 0, 0, res[0][0], res[0][1], GL_COLOR_BUFFER_BIT, GL_LINEAR);	
	}*/
	
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}



/*
=============
draw_DrawScreenQuad
=============
*/
void draw_BlitToScreen()
{

	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, backbuffer.id);
	//framebuffer_BindFramebuffer(&backbuffer);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, composite_buffer.id);
	//glBlitFramebuffer(0, 0, geometry_buffer.width, geometry_buffer.height, 0, 0, backbuffer.width, backbuffer.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
	camera_t *active_camera = camera_GetActiveCamera();
	//int i = GL_COLOR_ATTACHMENT0;
	
	//framebuffer_BindFramebuffer(&backbuffer);
	//glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glDrawBuffer(GL_NONE);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, geometry_buffer.id);
	glReadBuffer(GL_NONE);
	
	//glViewport(0, 0, backbuffer.width, backbuffer.height);
	glBlitFramebuffer(0, 0, geometry_buffer.width, geometry_buffer.height, 0, 0, backbuffer.width, backbuffer.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
	//glDrawBuffer(GL_LEFT);
	//glDrawBuffers(1, (GLenum *)&i);
	
	/* tone mapping is done here... */
	framebuffer_BindFramebuffer(&backbuffer);
	//glDrawBuffer(GL_LEFT);
	glClear(GL_COLOR_BUFFER_BIT);
	
	
	
	//framebuffer_BindFramebuffer(&composite_buffer);
	
	shader_SetShaderByIndex(screen_quad_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glEnableVertexAttribArray(shader_a.shaders[screen_quad_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[screen_quad_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_3DShadowSampler, 1);
	//shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_light_a.shadow_data[0].znear);
	//shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_light_a.shadow_data[0].zfar);
	
	shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_camera->frustum.znear);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_camera->frustum.zfar);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, composite_buffer.color_out1);
	//glBindTexture(GL_TEXTURE_2D, left_buffer.z_buffer);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, active_light_a.shadow_data[0].shadow_map.shadow_map);
	//glBindTexture(GL_TEXTURE_2D, geometry_buffer.color_out3);
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT); 
	
	                                    
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glFlush();

}

PEWAPI void draw_SetBloomParam(int param, int value)
{ 
	switch(param)
	{
		case BLOOM_SMALL_RADIUS:
			small_bloom_radius = (float)value;
		break;
		
		case BLOOM_MEDIUM_RADIUS:
			medium_bloom_radius = (float)value;
		break;
		
		case BLOOM_LARGE_RADIUS:
			large_bloom_radius = (float)value;
		break;
		
		case BLOOM_SMALL_ITERATIONS:
			small_bloom_iterations = value;
		break;
		
		case BLOOM_MEDIUM_ITERATIONS:
			medium_bloom_iterations = value;
		break;
		
		case BLOOM_LARGE_ITERATIONS:
			large_bloom_iterations = value;
		break;
		
		case BLOOM_INTENSITY:
			bloom_intensity = ((float)value) / 100.0;
		break;
	}
	return ;
}

PEWAPI int draw_GetBloomParam(int param)
{
	switch(param)
	{
		case BLOOM_SMALL_RADIUS:
			return (int)small_bloom_radius;
		break;
		
		case BLOOM_MEDIUM_RADIUS:
			return (int)medium_bloom_radius;
		break;
		
		case BLOOM_LARGE_RADIUS:
			return (int)large_bloom_radius;
		break;
		
		case BLOOM_SMALL_ITERATIONS:
			return small_bloom_iterations;
		break;
		
		case BLOOM_MEDIUM_ITERATIONS:
			return medium_bloom_iterations;
		break;
		
		case BLOOM_LARGE_ITERATIONS:
			return large_bloom_iterations;
		break;
		
		case BLOOM_INTENSITY:
			return bloom_intensity * 100.0;
		break;
	}
	return 0;
}

PEWAPI unsigned int draw_GetCompositeBufferTexture()
{
	return composite_buffer.color_out1;
}

PEWAPI void draw_EnableOutputToBackbuffer(int enable)
{
	if(enable)
	{
		
	}
	else
	{
		
	}
}

PEWAPI void draw_DrawString(int font_index, int size, int x, int y, int line_length, vec3_t color, char *str, ...)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	char cparm[64];
	
	font_t *font = &font_a.fonts[font_index];
	
	
	int iparm;
	float fparm;
	char *strparm;
	
	va_list args;
	va_start(args, str);
	
	int i;
	int decimal;
	int desired_decimal;
	int sign;
	//void *parms = ((char *)&str) + sizeof(char *);
	char *p = str;
	char *o = formated_str;
	char *q = cparm;
	char *t;
	
	//if(size > MAX_FONT_PSIZE) size = MAX_FONT_PSIZE;
	//else if(size < MIN_FONT_PSIZE) size = MIN_FONT_PSIZE;
	
	//float zoom = size * FONT_ZOOM_STEP;
	
	while(*p)
	{
		if(*p == '%')
		{
			p++;
			q = cparm;
			desired_decimal = 999;
			switch(*p)
			{
				case 'd':
					iparm = va_arg(args, int);
					itoa(iparm, q, 10);
					while(*q)
					{
						*o++ = *q++;
					}
					p++;
				break;
				
				case 'f':
					_do_float:
						
					fparm = va_arg(args, double);
					
					i = 0;
					q = ecvt(fparm, 12, &decimal, &sign);
					if(sign)
					{
						*o++ = '-';
					}
					if(decimal <= 0)
					{
						*o++ = '0';
						*o++ = '.';
						while(*q && desired_decimal > 0)
						{
							*o++ = *q++;
							desired_decimal--;
						}
					}
					else
					{
						while(*q && i < decimal)
						{
							*o++ = *q++;
							i++;
						}
						*o++ = '.';
						while(*q && desired_decimal > 0)
						{
							*o++ = *q++;
							i++;
							desired_decimal--;
						}
					}
					p++;
				break;
				
				case 's':
					strparm = va_arg(args, char *);
					while(*strparm)
					{
						*o++ = *strparm++;
					}
					p++;
				break;
				
				case '.':
					i = 0;
					p++;
					while(*p >= '0' && *p <= '9')
					{
						cparm[i++] = *p++;
					}
					cparm[i] = '\0';
					desired_decimal = atoi(cparm);
					goto _do_float;
				break;
			}
		}
		else
		{
			if(*p == '\t')
			{
				*o++ = ' ';
				*o++ = ' ';
				*o++ = ' ';
				*o++ = ' ';
				p++;
			}
			*o++ = *p++;
		}
	}
	
	*o = '\0';
	
	if(color.r > 1.0) color.r = 1.0;
	else if(color.r < 0.0) color.r = 0.0;
	
	if(color.g > 1.0) color.g = 1.0;
	else if(color.g < 0.0) color.g = 0.0;
	
	if(color.b > 1.0) color.b = 1.0;
	else if(color.b < 0.0) color.b = 0.0;
	
	
	SDL_Color f = {255 * color.r, 255 * color.g, 255 * color.b, 255};
	SDL_Color b = {0, 0, 0, 0};
	//SDL_Surface *s = TTF_RenderUTF8_Blended(font->font, formated_str, f);
	SDL_Surface *s = TTF_RenderUTF8_Blended_Wrapped(font->font, formated_str, f, line_length);
	

	glRasterPos2f(((float)x / (float)renderer.screen_width) * 2.0 - 1.0, ((float)(y) / (float)renderer.screen_height) * 2.0 - 1.0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glUseProgram(0);
	glEnable(GL_BLEND);
	glPixelZoom(1.0, -1.0);
	glColor3f(1.0, 0.0, 0.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawPixels(s->w, s->h, GL_BGRA, GL_UNSIGNED_BYTE, s->pixels);

	SDL_FreeSurface(s);
	glPixelZoom(1.0, 1.0);
	glDisable(GL_BLEND);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


PEWAPI void draw_DrawWidgets()
{
	widget_t *cwidget;
	swidget_t *cswidget;
	wbutton_t *button;
	wtabbar_t *tabbar;
	wdropdown_t *dropdown;
	wtab_t *tab;
	wvar_t *var;
	wslider_t *slider;
	wslidergroup_t *slider_group;
	
	int i;
	
	
	int *i32var;
	short *i16var;
	vec3_t *v3tvar;
	mat3_t *m3tvar;
	char *strvar;
	vec3_t v;
	
	int stack_top = -1;
	swidget_t *swidget_stack[64];
	vec2_t pos_stack[64];
	int stencil_stack[64];
	
	int base_stencil = 1;
	float x_scale = 2.0 / renderer.screen_width;
	float y_scale = 2.0 / renderer.screen_height;
	
	float r;
	float g;
	float b;
	float a;
	float tab_label_width;
	float option_height;
	
	//float scissor_x;
	//float scissor_y;
	//float scissor_w;
	//float scissor_h;
	
	//float header_x;
	//float header_y;
	//float header_w;
	//float header_h;
	
	float x0;
	float y0;
	float x1;
	float y1;
	
	float x;
	float y;
	
	float cw;
	float ch;
	
	float hw;
	float hh;
	
	float sh0;
	float sh1;
	//float sw0;
	//float sw1;
	
	framebuffer_BindFramebuffer(&backbuffer);
	
	glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	glPushMatrix();
	glLoadMatrixf(&widget_projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	float swidget_x;
	float swidget_y;
	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_SCISSOR_TEST);
	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);
	//glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);
	SDL_Color c = {255, 255, 255, 255};
	SDL_Surface *s;
	//glEnable(GL_BLEND);
	
	if(widget_count > 1)
	{
		cwidget = widgets->next->next;
	}
	else
	{
		cwidget = widgets->next;
	}
	
	while(cwidget)
	{
		
		 _draw_top_widget:

		
		if(cwidget->a < 1.0)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glDisable(GL_BLEND);
		}

		
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0x1, 0xff);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		
		hw = cwidget->w / 2.0;
		hh = cwidget->h / 2.0;
		
		x = cwidget->x;
		y = cwidget->y;
		
		cw = (cwidget->w / cwidget->cw);
		ch = (cwidget->h / cwidget->ch);
		
		r = cwidget->r;
		g = cwidget->g;
		b = cwidget->b;
		a = cwidget->a;
		
		if(!(cwidget->bm_flags & WIDGET_MOUSE_OVER))
		{
			r *= 0.8;	
			g *= 0.8;
			b *= 0.8;
		}
		
		if(cwidget->tex_handle > -1)
		{

			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, cwidget->tex_handle);
			glBegin(GL_QUADS);
			glColor4f(r, g, b, a);
			glTexCoord2f(0.0, 1.0);
			glVertex3f(x - hw, y + hh, -0.5);
			glTexCoord2f(0.0, 0.0);
			glVertex3f(x - hw, y - hh, -0.5);
			glTexCoord2f(1.0, 0.0);
			glVertex3f(x + hw, y - hh, -0.5);
			glTexCoord2f(1.0, 1.0);
			glVertex3f(x + hw, y + hh, -0.5);
			
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			glColor4f(r, g, b, a);
			glRectf(x - hw, y - hh, x + hw, y + hh);
		
		}
		
		if(cwidget->bm_flags & WIDGET_HIGHTLIGHT_BORDERS)
		{
			sh0 = y + hh;
			sh1 = y - hh;
			
			
			if(cwidget->bm_flags & WIDGET_HEADER)
			{
				sh0 -= WIDGET_HEADER_PIXEL_HEIGHT;
				if(cwidget->bm_flags & WIDGET_MOUSE_OVER_HEADER)
				{
					r *= 0.8;	
					g *= 0.8;
					b *= 0.8;
				}
				else
				{
					r *= 0.5;	
					g *= 0.5;
					b *= 0.5;
				}
				
				glStencilFunc(GL_ALWAYS, 0x00, 0xff);
				glColor4f(r, g, b, 1.0);
				glRectf(x - hw, y + hh - WIDGET_HEADER_PIXEL_HEIGHT, x + hw, y + hh);
			}
			else if(cwidget->bm_flags & WIDGET_MOUSE_OVER_TOP_BORDER)
			{
				sh0 -= WIDGET_BORDER_PIXEL_WIDTH;
				glColor4f(cwidget->r * 0.9, cwidget->g * 0.9, cwidget->b * 0.9, cwidget->a * 1.001);
				glRectf(x - hw, y + hh - WIDGET_BORDER_PIXEL_WIDTH, x + hw, y + hh);
			}
			glColor4f(cwidget->r * 0.9, cwidget->g * 0.9, cwidget->b * 0.9, cwidget->a * 1.001);
			
			if(cwidget->bm_flags & WIDGET_MOUSE_OVER_BOTTOM_BORDER)
			{
				sh1 += WIDGET_BORDER_PIXEL_WIDTH;
				glRectf(x - hw, y - hh, x + hw, y - hh + WIDGET_BORDER_PIXEL_WIDTH);
			}
			
			if(cwidget->bm_flags & WIDGET_MOUSE_OVER_LEFT_BORDER)
			{
				glRectf(x - hw, sh1, x - hw + WIDGET_BORDER_PIXEL_WIDTH, sh0);
			}
			else if(cwidget->bm_flags & WIDGET_MOUSE_OVER_RIGHT_BORDER)
			{
				glRectf(x + hw - WIDGET_BORDER_PIXEL_WIDTH, sh1, x + hw, sh0);
			}
		}
		
		//draw_DrawString(ui_font, 16, 0, 0, 500, vec3(1.0, 1.0, 1.0), "test");
		
		//glDisable(GL_BLEND);
		cswidget = cwidget->sub_widgets;
		//stencil_val = 1;
		while(cswidget)
		{
			//glDisable(GL_STENCIL_TEST);
			//glStencilFunc(GL_GEQUAL, 1, 0xff);
			//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			//if(stencil_val == 255) stencil_val--;
			//draw_DrawString(ui_font, 16, 0, 0, 500, vec3(1.0, 1.0, 1.0), "test");
			
			_draw_nested_swidgets:
			
			switch(cswidget->type)
			{
				
				
				
				case WIDGET_BUTTON:
					
					glStencilFunc(GL_EQUAL, base_stencil, 0xff);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					
					button = (wbutton_t *)cswidget;
					hw = button->swidget.w / 2.0;
					hh = button->swidget.h / 2.0;
					if(button->button_flags & BUTTON_CHECK_BOX)
					{
						glEnable(GL_LINE_SMOOTH);
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						if(button->swidget.bm_flags & WIDGET_MOUSE_OVER)
						{
							g = 1.0;
						}
						else
						{
							g = 0.5;
						}
					
						glColor3f(g, g, g);
						glRectf(button->swidget.x + x - hw, button->swidget.y + y - hh, button->swidget.x + x + hw, button->swidget.y + y + hh);
						
						if(button->swidget.w < button->swidget.h)
						{
							cw = button->swidget.w;
						}
						else
						{
							cw = button->swidget.h;
						}
						
						
						if(!(button->button_flags & BUTTON_CHECK_BOX_CHECKED))
						{
							glRectf(button->swidget.x + x + hw - cw, button->swidget.y + y - hh, button->swidget.x + x + hw, button->swidget.y + y + hh);
						}
						//glRectf(button->swidget.x + x + hw - cw, button->swidget.y + y - hh, button->swidget.x + x + hw, button->swidget.y + y + hh);
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						glDisable(GL_LINE_SMOOTH);
						
						if(button->button_flags & BUTTON_CHECK_BOX_CHECKED)
						{
							glRectf(button->swidget.x + x + hw - cw, button->swidget.y + y - hh, button->swidget.x + x + hw, button->swidget.y + y + hh);
						}
						
						glStencilFunc(GL_EQUAL, base_stencil, 0xff);
						glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
						
						glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
						
						glRectf(button->swidget.x + x - hw, button->swidget.y + y - hh, button->swidget.x + x + hw, button->swidget.y + y + hh);
						
						
						glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
						glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
						glRectf(button->swidget.x + x + hw - cw, button->swidget.y + y - hh, button->swidget.x + x + hw, button->swidget.y + y + hh);
						
						glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
						
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
						
						draw_DrawString(ui_font, 16, (button->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (button->swidget.y + y + hh) + renderer.screen_height * 0.5 + 1, 500, vec3(1.0, 1.0, 1.0), button->swidget.name);
					}
					else
					{
						
						if(button->swidget.bm_flags & WIDGET_MOUSE_OVER)
						{
							r = 0.6;
							g = 0.6;
							b = 0.6;
						}
						else
						{
							r = 0.4;
							g = 0.4;
							b = 0.4;
						}
						
						if(button->button_flags & BUTTON_TOGGLED)
						{
							r -= 0.2;
							g -= 0.2;
							b -= 0.2;
						}
						
						glColor3f(r, g, b);
						glStencilFunc(GL_EQUAL, base_stencil, 0xff);
						glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
						glRectf(button->swidget.x + x - hw, button->swidget.y + y - hh, button->swidget.x + x + hw, button->swidget.y + y + hh);
						
						glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
						
						//hw = (button->swidget.x + x - hw)
						
						draw_DrawString(ui_font, 16, (button->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (button->swidget.y + y + hh) + renderer.screen_height * 0.5 + 1, 500, vec3(1.0, 1.0, 1.0), button->swidget.name);
						
					}
				break;
				
				
				case WIDGET_VAR:
					
					//glStencilFunc(GL_EQUAL, 1, 0xff);
					//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					
					var = (wvar_t *)cswidget;
					hw = var->swidget.w / 2.0;
					hh = var->swidget.h / 2.0;
					
					switch(var->var_type)
					{
						case VAR_INT_32:
							glColor3f(0.4, 0.4, 0.4);
							glStencilFunc(GL_EQUAL, base_stencil, 0xff);
							glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
							glRectf(var->swidget.x + x - hw, var->swidget.y + y - hh, var->swidget.x + x + hw, var->swidget.y + y + hh);
							/*glBegin(GL_QUADS);
							glVertex3f(var->swidget.x + x - hw, var->swidget.y + y + hh, -0.5);
							glVertex3f(var->swidget.x + x - hw, var->swidget.y + y - hh, -0.5);
							glVertex3f(var->swidget.x + x + hw, var->swidget.y + y - hh, -0.5);
							glVertex3f(var->swidget.x + x + hw, var->swidget.y + y + hh, -0.5);
							glEnd();*/
							
							
							i32var = (int *)var->var;
							
							if(var->var_flags & VAR_ADDR)
							{
								i32var = (int *)*i32var;
							}
							
							glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
							glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
							
							draw_DrawString(ui_font, 16, (var->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (var->swidget.y + y + hh) + renderer.screen_height * 0.5 + 1, 500, vec3(1.0, 1.0, 1.0), "%s %d", var->swidget.name, *i32var);	
						break;
						
						case VAR_VEC3T:
							glColor3f(0.4, 0.4, 0.4);
							glStencilFunc(GL_EQUAL, base_stencil, 0xff);
							glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
							glRectf(var->swidget.x + x - hw, var->swidget.y + y - hh, var->swidget.x + x + hw, var->swidget.y + y + hh);
							/*glBegin(GL_QUADS);
							glVertex3f(var->swidget.x + x - hw, var->swidget.y + y + hh, -0.5);
							glVertex3f(var->swidget.x + x - hw, var->swidget.y + y - hh, -0.5);
							glVertex3f(var->swidget.x + x + hw, var->swidget.y + y - hh, -0.5);
							glVertex3f(var->swidget.x + x + hw, var->swidget.y + y + hh, -0.5);
							glEnd();*/
							
							v3tvar = (vec3_t *)var->var;
							
							if(v3tvar)
							{
								if(var->var_flags & VAR_ADDR)
								{
									v3tvar = (vec3_t *)(&v3tvar->x);
								}
								
								v.x = v3tvar->x;
								v.y = v3tvar->y;
								v.z = v3tvar->z;
							}
							else
							{
								v.x = 0.0;
								v.y = 0.0;
								v.z = 0.0;
							}
							
							glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
							glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
							
							draw_DrawString(ui_font, 16, (var->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (var->swidget.y + y + hh) + renderer.screen_height * 0.5 + 1, 500, vec3(1.0, 1.0, 1.0), "%s:\t[%.2f %.2f %.2f]", var->swidget.name, v.x, v.y, v.z);
						break;
						
						case VAR_MAT3T:
							glColor3f(0.4, 0.4, 0.4);
							glStencilFunc(GL_EQUAL, base_stencil, 0xff);
							glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
							glRectf(var->swidget.x + x - hw, var->swidget.y + y - hh, var->swidget.x + x + hw, var->swidget.y + y + hh);
							
							m3tvar = (mat3_t *)var->var;
							glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
							glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
							draw_DrawString(ui_font, 16, (var->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (var->swidget.y + y + hh) + renderer.screen_height * 0.5, 500, vec3(1.0, 1.0, 1.0), "%s:", var->swidget.name);
							draw_DrawString(ui_font, 16, (var->swidget.x + x - hw + 150) + renderer.screen_width * 0.5 + 1,  (var->swidget.y + y + hh) + renderer.screen_height * 0.5, 500, vec3(1.0, 1.0, 1.0), "[%.2f %.2f %.2f]\n[%.2f %.2f %.2f]\n[%.2f %.2f %.2f]", m3tvar->floats[0][0],
																																																																		m3tvar->floats[0][1],
																																																																		m3tvar->floats[0][2],
																																																																								 
																																																																		m3tvar->floats[1][0],
																																																																		m3tvar->floats[1][1],
																																																																		m3tvar->floats[1][2],
																																																																								 
																																																																		m3tvar->floats[2][0],
																																																																		m3tvar->floats[2][1],
																																																																		m3tvar->floats[2][2]);
																																																																								 
																																																																								 
							/*glBegin(GL_QUADS);
							glVertex3f(var->swidget.x + x - hw, var->swidget.y + y + hh, -0.5);
							glVertex3f(var->swidget.x + x - hw, var->swidget.y + y - hh, -0.5);
							glVertex3f(var->swidget.x + x + hw, var->swidget.y + y - hh, -0.5);
							glVertex3f(var->swidget.x + x + hw, var->swidget.y + y + hh, -0.5);
							glEnd();*/
							
							/*v3tvar = (vec3_t *)var->var;
							
							if(v3tvar)
							{
								if(var->var_flags & VAR_ADDR)
								{
									v3tvar = (vec3_t *)(&v3tvar->x);
								}
								
								v.x = v3tvar->x;
								v.y = v3tvar->y;
								v.z = v3tvar->z;
							}
							else
							{
								v.x = 0.0;
								v.y = 0.0;
								v.z = 0.0;
							}
							
							draw_DrawString(ui_font, 16, (var->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (var->swidget.y + y - hh) + renderer.screen_height * 0.5 - 1, 500, vec3(1.0, 1.0, 1.0), "%s:    [%.2f %.2f %.2f]", var->swidget.name, v.x, v.y, v.z);*/
						break;
						
						case VAR_STR:
							glColor3f(0.4, 0.4, 0.4);
							glStencilFunc(GL_EQUAL, base_stencil, 0xff);
							glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
							glRectf(var->swidget.x + x - hw, var->swidget.y + y - hh, var->swidget.x + x + hw, var->swidget.y + y + hh);
							/*glBegin(GL_QUADS);
							glVertex3f(var->swidget.x + x - hw, var->swidget.y + y + hh, -0.5);
							glVertex3f(var->swidget.x + x - hw, var->swidget.y + y - hh, -0.5);
							glVertex3f(var->swidget.x + x + hw, var->swidget.y + y - hh, -0.5);
							glVertex3f(var->swidget.x + x + hw, var->swidget.y + y + hh, -0.5);
							glEnd();*/
							
							strvar = *((char **)var->var);
							if(!strvar)
							{
								strvar = "<null>";
							}
							glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
							glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
							draw_DrawString(ui_font, 16, (var->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (var->swidget.y + y + hh) + renderer.screen_height * 0.5 + 1, 500, vec3(1.0, 1.0, 1.0), "%s:    %s", var->swidget.name, strvar);
						break;
					}
					
				break;
				
				case WIDGET_TAB_BAR:
					tabbar = (wtabbar_t *)cswidget;
					
					hw = tabbar->swidget.w / 2.0;
					hh = tabbar->swidget.h / 2.0;
					
					glColor3f(0.4, 0.4, 0.4);
					glStencilFunc(GL_EQUAL, base_stencil, 0xff);
					glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
					glRectf(tabbar->swidget.x + x - hw, tabbar->swidget.y + y - hh, tabbar->swidget.x + x + hw, tabbar->swidget.y + y + hh);
					
					tab_label_width = tabbar->swidget.w / (float)tabbar->tab_count;
					
					
					
					x0 = tabbar->swidget.x + x;
					y0 = tabbar->swidget.y + y - hh;
					y1 = tabbar->swidget.y + y + hh;
					
					hw = tab_label_width / 2.0;
					
					x1 = -hw * (tabbar->tab_count - 1);
					
					for(i = 0; i < tabbar->tab_count; i++)
					{
						
						glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
						glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
						
						if(tabbar->tabs[i].bm_flags & TAB_SELECTED)
						{
							glColor3f(0.3, 0.3, 0.3);
						}
						else
						{
							glColor3f(0.5, 0.5, 0.5);
						}
						
						glRectf(x0 + x1 - hw + 2 , y0 + 2, x0 + x1 + hw - 2, y1 - 2);
						
						glStencilFunc(GL_EQUAL, base_stencil + 2, 0xff);
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
						
						draw_DrawString(ui_font, 16, (x0 + x1 - hw + 2) + renderer.screen_width * 0.5 + 1,  (y1) + renderer.screen_height * 0.5 - 2, 500, vec3(1.0, 1.0, 1.0), "%s", tabbar->tabs[i].name);
						
						x1 += tab_label_width;
					}
					
					
					
					hw = tabbar->swidget.w / 2.0;
					
					glStencilFunc(GL_EQUAL, base_stencil, 0xff);
					glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
					
					if(!(tabbar->active_tab->bm_flags & TAB_NO_SUB_WIDGETS))
					{
						glColor3f(0.4, 0.4, 0.4);
						glRectf(x0 - hw , y0 - 160, x0 + hw, y0 - 2);
						
						if(tabbar->active_tab->swidget_count)
						{
							stack_top++;
							swidget_stack[stack_top] = cswidget;
							pos_stack[stack_top].x = x;
							pos_stack[stack_top].y = y;
							stencil_stack[stack_top] = base_stencil;
							base_stencil += 1;
										
							x += tabbar->swidget.x;
							y += tabbar->swidget.y;
							cswidget = tabbar->active_tab->swidgets;
							goto _draw_nested_swidgets;
						}
						
					}
					
					
				break;
				
				case WIDGET_DROP_DOWN:
					
					dropdown = (wdropdown_t *)cswidget;
					
					hw = dropdown->swidget.w / 2.0;
					hh = OPTION_HEIGHT / 2.0;
					
					
					//glStencilFunc(GL_EQUAL, base_stencil, 0xff);
					//glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
					
					//option_height = dropdown->swidget.h / (float)dropdown->option_count;
					
					x0 = dropdown->swidget.x + x - hw;
					x1 = dropdown->swidget.x + x + hw;
					
					
					if(!(dropdown->bm_flags & DROP_DOWN_NO_HEADER))
					{
						glColor3f(0.3, 0.3, 0.3);
						
						y0 = dropdown->swidget.y + y - hh;
						y1 = dropdown->swidget.y + y + hh;
					
						glStencilFunc(GL_EQUAL, base_stencil, 0xff);
						glStencilOp(GL_KEEP, GL_INCR, GL_INCR);			
						glRectf(x0, y0, x1, y1);
						
						glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
						
						
						if(dropdown->active_option)
						{
							draw_DrawString(ui_font, 16, x0 + renderer.screen_width * 0.5 + 1,  (y1) + renderer.screen_height * 0.5 - 2, 500, vec3(1.0, 1.0, 1.0), "%s", dropdown->active_option->name);
						}
						
						
					}
					else
					{
						y0 = dropdown->swidget.y + y + hh; 
					}
					
					y1 = 0;
					
					
					
					if(dropdown->bm_flags & DROP_DOWN_DROPPED)
					{
						if(dropdown->option_count)
						{
							for(i = 0; i < dropdown->option_count; i++)
							{
								if(dropdown->options[i].bm_flags & OPTION_MOUSE_OVER)
								{
									r = 0.5;
									g = 0.5;
									b = 0.5;
								}
								else
								{
									r = 0.4;
									g = 0.4;
									b = 0.4;
								}
								glColor3f(r, g, b);
								
								glStencilFunc(GL_EQUAL, base_stencil, 0xff);
								glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
								glRectf(x0, y0 - y1 - OPTION_HEIGHT, x1, y0 - y1);
								
								
								
								if(dropdown->options[i].nested)
								{
									glDisable(GL_STENCIL_TEST);
									
									if(dropdown->options[i].bm_flags & OPTION_MOUSE_OVER)
									{
										glColor3f(1.0, 1.0, 1.0);
									}
									else
									{
										glColor3f(r - 0.08, g - 0.08, b - 0.08);
									}
									
									
									//glColor3f(0.1, 0.1, 0.1);
									
									glBegin(GL_TRIANGLES);
									glVertex3f(x1 - 10, y0 - y1 - 5, 0.0);
									glVertex3f(x1 - 10, y0 - y1 - OPTION_HEIGHT + 5, 0.0);
									glVertex3f(x1, y0 - y1 - OPTION_HEIGHT / 2.0, 0.0);
									glEnd();
									glColor3f(r, g, b);
									glEnable(GL_STENCIL_TEST);
								}
								
								
								glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
								glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
								
								draw_DrawString(ui_font, 16, x0 + renderer.screen_width * 0.5 + 1,  (y0 - y1) + renderer.screen_height * 0.5 - 2, 500, vec3(1.0, 1.0, 1.0), "%s", dropdown->options[i].name);
								y1 += OPTION_HEIGHT;
								
							}
						}
						glColor3f(0.35, 0.35, 0.35);
					}
					else
					{
						glColor3f(0.4, 0.4, 0.4);
					}
					
					/*y1 = dropdown->swidget.y + y + hh;
					
					glStencilFunc(GL_EQUAL, base_stencil, 0xff);
					glStencilOp(GL_KEEP, GL_INCR, GL_INCR);			
					glRectf(x0, y0, x1, y1);
					
					glStencilFunc(GL_EQUAL, base_stencil + 1, 0xff);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					
					
					if(dropdown->active_option)
					{
						draw_DrawString(ui_font, 16, x0 + renderer.screen_width * 0.5 + 1,  (y1) + renderer.screen_height * 0.5 - 2, 500, vec3(1.0, 1.0, 1.0), "%s", dropdown->active_option->name);
					}*/
					
					if(dropdown->options[dropdown->cur_option].nested && dropdown->bm_flags & DROP_DOWN_DROPPED)
					{
						stack_top++;
						swidget_stack[stack_top] = cswidget;
						stencil_stack[stack_top] = base_stencil;
						cswidget = dropdown->options[dropdown->cur_option].nested;
						goto _draw_nested_swidgets;
					}
					
					//glEnable(GL_STENCIL_TEST);
					
					
					
				break;
				
				case WIDGET_SLIDER:
					slider = (wslider_t *)cswidget;
					hw = slider->swidget.w / 2.0;
					
					x0 = slider->swidget.x - hw;
					x1 = slider->swidget.x + hw;
					
					hh = SLIDER_INNER_HEIGHT / 2.0;
					
					y0 = slider->swidget.y - hh;
					y1 = slider->swidget.y + hh;
					
					
					glColor3f(0.4, 0.4, 0.4);
					glRectf(x0, y0, x1, y1);
					
					hh = SLIDER_OUTER_HEIGHT / 2.0;
					
					x0 = slider->swidget.x - hw + hw * 2.0 * slider->pos;
					
					y0 = slider->swidget.y - hh;
					y1 = slider->swidget.y + hh;
					
					glEnable(GL_POINT_SMOOTH);
					glPointSize(14.0);
					
					glBegin(GL_POINTS);
					glColor3f(0.7, 0.7, 0.7);
					glVertex3f(x0, slider->swidget.y, 0.0);
					glEnd();
					glDisable(GL_POINT_SMOOTH);
					glPointSize(1.0);
					//glRectf(x0 - hh, y0, x0 + hh, y1);
					
				break;
				
				case WIDGET_SLIDER_GROUP:
					slider_group = (wslidergroup_t *) cswidget;
					stack_top++;
					swidget_stack[stack_top] = cswidget;
					pos_stack[stack_top].x = x;
					pos_stack[stack_top].y = y;
					stencil_stack[stack_top] = base_stencil;
						//base_stencil += 1;
										
					x += cswidget->x;
					y += cswidget->y;
					cswidget = (swidget_t *)slider_group->sliders;
					goto _draw_nested_swidgets;
				
				break;
			}
			
			cswidget = cswidget->next;
			
			if(!cswidget)
			{
				if(stack_top != -1)
				{
					cswidget = swidget_stack[stack_top];
					x = pos_stack[stack_top].x;
					y = pos_stack[stack_top].y;
					base_stencil = stencil_stack[stack_top];
					stack_top--;
					cswidget = cswidget->next;
				}
			}
			
		}
		
		
		
		if(cwidget == top_widget) break;
		
		cwidget = cwidget->next;
	}
	
	if(widget_count > 1 && cwidget != top_widget)
	{
		cwidget = top_widget;
		goto _draw_top_widget;
	}
	
	
	
	
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	//glEnable(GL_TEXTURE_2D);
	
	/* NOTE: disabling GL_TEXTURE_2D DOES NOT MATTER for
	non-imediate functionality. And keeping it enabled
	fucks up anything that does use imediate mode
	if this thing doesn't use any textures (console, debug lines, etc...) */
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

/*void draw_BuildClusters()
{
	clusters_per_row = renderer.width / CLUSTER_WIDTH;
	clusters_rows = renderer.height / CLUSTER_WIDTH;
	
	
}*/

 void draw_DrawConsole()
{
	float half_width;
	float half_height;
	
	framebuffer_BindFramebuffer(&backbuffer);
	half_width=console.width/2.0;
	half_height=console.height/2.0;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(console.x, console.y, 0.0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_LIGHTING);
	//glDisable(GL_STENCIL_TEST);
	glBegin(GL_QUADS);

	glColor3f(0.1, 0.1, 0.1);
	glVertex3f(-half_width, half_height, -0.5);
	glVertex3f(-half_width, -half_height+(renderer.height*0.00002), -0.5);
	glVertex3f(half_width, -half_height+(renderer.height*0.00002),-0.5);
	glVertex3f(half_width, half_height, -0.5);
	
	glColor3f(0.0, 0.0, 0.0);
	glVertex3f(-half_width+0.02, half_height, -0.5);
	glVertex3f(-half_width+0.02, -half_height+0.12, -0.5);
	glVertex3f(half_width-0.02, -half_height+0.12, -0.5);
	glVertex3f(half_width-0.02, half_height, -0.5);
	
	glColor3f(0.0, 0.0, 0.0);
	glVertex3f(-half_width+0.02, half_height-0.9, -0.5);
	glVertex3f(-half_width+0.02, -half_height+0.02+(renderer.height*0.00002), -0.5);
	glVertex3f(half_width-0.02, -half_height+0.02+(renderer.height*0.00002), -0.5);
	glVertex3f(half_width-0.02, half_height-0.9, -0.5);
	glEnd();
	
	//glColor3f(1.0, 0.0, 1.0);
	console_BlitTextBuffer();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	renderer.active_shader_index=-1;
	//glUseProgram(shader_a.shaders[renderer.active_shader_index].shader_ID);
	
	return;
}


PEWAPI void draw_Enable(int param)
{
	glEnable(param);
	return;
}

PEWAPI void draw_Disable(int param)
{
	glDisable(param);
	return;
}


PEWAPI void draw_SetBlendMode(int blend_mode)
{
	switch(blend_mode)
	{
		case BLEND_ADDITIVE:
			glBlendFunc(GL_ONE, GL_ONE);
		break;
	}
}




#ifdef __cplusplus
}
#endif














