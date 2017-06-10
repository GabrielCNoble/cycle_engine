#include "draw.h"
//#include "draw_common.h"

#include "draw_debug.h"

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
#include "gui.h"
#include "gpu.h"

//extern unsigned int _512_extra_map;
//extern unsigned int extra_framebuffer;
extern font_array font_a;
//#define PRINT_INFO

extern unsigned int gpu_heap;

extern wbase_t *widgets;
extern wbase_t *top_widget;
extern int widget_count;

extern unsigned int screen_area_mesh_gpu_buffer;

extern float screen_quad[3 * 4];

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


extern int max_lights_per_pass;


framebuffer_t geometry_buffer;
framebuffer_t transparency_buffer;
static framebuffer_t composite_buffer;
framebuffer_t backbuffer;
gpu_buffer_t debug_buffer;
gpu_buffer_t screen_quad_buffer;

static framebuffer_t left_buffer;
static framebuffer_t right_buffer;

static framebuffer_t left_volume_buffer;
static framebuffer_t right_volume_buffer;
static framebuffer_t final_volume_buffer;
static framebuffer_t quarter_volume_buffer;


static framebuffer_t debug_draw_buffer;
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
	ext_str = (char *)glGetString(GL_EXTENSIONS);
	sub_str = strstr(ext_str, "GL_ARB_seamless_cube_map");
	if(sub_str)
	{
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	
	/*sub_str = strstr(ext_str, "GL_EXT_transform_feedback");
	if(sub_str)
	{
		glEnable(GL_TRANSFORM_FEEDBACK);
	}*/

	
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
	

	
	screen_quad_buffer=gpu_CreateGPUBuffer(sizeof(float)*3*4, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
	gpu_FillGPUBuffer(&screen_quad_buffer, sizeof(float)*3, 4, screen_quad);
	
	//debug_buffer=gpu_CreateGPUBuffer(sizeof(float)*3*49, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
	
	backbuffer.id=0;
	backbuffer.width=renderer.screen_width;
	backbuffer.height=renderer.screen_height;
	backbuffer.color_output_count=1;
	
	
	glGenFramebuffers(1, &shadow_buffer.id);
	
	
	geometry_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 2, GL_RGBA16F, GL_RGBA16F);
	transparency_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 3, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F);
	left_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 3, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F);
	right_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 3, GL_RGBA16F, GL_RGBA16F, GL_RGBA16F);
	left_volume_buffer = framebuffer_CreateFramebuffer(renderer.width / 2, renderer.height / 2, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	right_volume_buffer = framebuffer_CreateFramebuffer(renderer.width / 2, renderer.height / 2, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	final_volume_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	quarter_volume_buffer = framebuffer_CreateFramebuffer(renderer.width / 4, renderer.height / 4, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	composite_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA16F);
	debug_draw_buffer = framebuffer_CreateFramebuffer(renderer.width, renderer.height, GL_DEPTH_COMPONENT, 1, GL_RGBA);
	picking_buffer = framebuffer_CreateFramebuffer(renderer.screen_width, renderer.screen_height, GL_DEPTH_COMPONENT, 1, GL_RGB16F);
	
	
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 256, 256, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex2);
	glBindTexture(GL_TEXTURE_2D, itex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 128, 128, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex3);
	glBindTexture(GL_TEXTURE_2D, itex3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 64, 64, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex4);
	glBindTexture(GL_TEXTURE_2D, itex4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 32, 32, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex5);
	glBindTexture(GL_TEXTURE_2D, itex5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 16, 16, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex6);
	glBindTexture(GL_TEXTURE_2D, itex6);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 8, 8, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex7);
	glBindTexture(GL_TEXTURE_2D, itex7);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex8);
	glBindTexture(GL_TEXTURE_2D, itex8);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 2, 2, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &itex9);
	glBindTexture(GL_TEXTURE_2D, itex9);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
	
	
	
	
	
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
	draw_SetRenderFlags(RENDERFLAG_USE_SHADOW_MAPS);
	//draw_SetDebugFlag(DEBUG_DRAW_LIGHT_LIMITS);
	//draw_SetDebugFlag(DEBUG_DRAW_LIGHT_ORIGINS);
	draw_SetDebugFlag(DEBUG_DRAW_ARMATURES);
	//draw_SetDebugFlag(DEBUG_DRAW_ENTITY_ORIGIN);
	//draw_SetDebugFlag(DEBUG_DRAW_COLLIDERS);
	//draw_SetDebugFlag(DEBUG_DISABLED);
	//draw_SetDebugFlag(DEBUG_DRAW_ENTITY_AABB);
	//draw_SetDebugFlag(DEBUG_DRAW_DBUFFER);
	//draw_SetDebugFlag(DEBUG_DRAW_NBUFFER);
	//draw_SetDebugFlag(DEBUG_DRAW_ZBUFFER);
	
	//draw_SetBloomParam(BLOOM_SMALL_RADIUS, DEFAULT_SMALL_BLOOM_RADIUS);
	draw_SetBloomParam(BLOOM_MEDIUM_RADIUS, DEFAULT_MEDIUM_BLOOM_RADIUS);
	draw_SetBloomParam(BLOOM_LARGE_RADIUS, DEFAULT_LARGE_BLOOM_RADIUS);
	
	//draw_SetBloomParam(BLOOM_SMALL_ITERATIONS, DEFAULT_SMALL_BLOOM_ITERATIONS);
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
	
	
	CreateOrthographicMatrix(&widget_projection_matrix, -renderer.width *0.005, renderer.width *0.005, renderer.height *0.005, -renderer.height *0.005, 0.001, 1.5, NULL);
	
	draw_debug_Init();
	
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
	//glDepthMask(GL_TRUE);
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	//framebuffer_BindFramebuffer(&debug_draw_buffer);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	framebuffer_BindFramebuffer(&right_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glClearColor(0.0, 0.0 ,0.0, 0.0);
	framebuffer_BindFramebuffer(&geometry_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	camera_SetCurrentCameraProjectionMatrix();
	
	return;
}


void draw_DrawFrameDebug()
{
	int i;
	int c;
	int l;
	int a;
	
//	while(glGetError()!= GL_NO_ERROR);
	
	//glEnable(GL_STENCIL_TEST);
	//glClear(GL_STENCIL_BUFFER_BIT);
	//glClearStencil(0x00);
	//glStencilMask(0xff);
	
	//glStencilFunc(GL_ALWAYS, 0xff, 0xff);				/* always pass the stencil test */
	//glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);	/* replace the value by ref on stencil fail, zfail and zpass. */
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glDepthMask(GL_FALSE);
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	
	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glLoadIdentity();
	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glLoadIdentity();
	
	
	//glUseProgram(0);
	//glBegin(GL_QUADS);
	//glVertex3f(-0.5, 0.5, -0.5);
	//glVertex3f(-0.5, -0.5, -0.5);
	//glVertex3f(0.5, -0.5, -0.5);
	//glVertex3f(0.5, 0.5, -0.5);
	//glEnd();
	
	
	//glPopMatrix();
	//glMatrixMode(GL_PROJECTION);
	//glPopMatrix();
	
	
	//glStencilMask(0x1);
	
	//glStencilFunc(GL_EQUAL, 0xff, 0xff);					/* pass only if the value in the stencil buffer is different form ref */
	//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);					/* don't modify the stencil buffer */
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glDepthMask(GL_TRUE);
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_STENCIL_TEST);
	
	//printf("%x\n", glGetError());
	//while(glGetError() != GL_NO_ERROR);
	for(i=0; i<frame_func_count; i++)
	{ 
		frame_funcs[i]();
	}
	
	
	
	//draw_DrawWidgets();

	if(unlikely(console.bm_status&CONSOLE_VISIBLE)) draw_DrawConsole();
	
	

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
	//draw_test_DrawVertexData(vec3(0.0, 0.0 ,-6.0), model_GetVertexData("stairs.obj"));
	
	
	
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



/* TODO: inline those two functions. */
/*
=============
draw_ResetRenderQueue
=============
*/
/*void draw_ResetRenderQueue()
{
	render_q.count=0;
	return;
}


void draw_ResetShadowQueue()
{
	shadow_q.count=0;
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
			frame_funcs[1] = draw_ResolveGBuffer;
			frame_funcs[2] = draw_Dummy;
			frame_funcs[3] = draw_Dummy;
			frame_funcs[4] = draw_DrawLightVolumes;
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
		v_byte_count=vert_count*3*sizeof(float);
		glEnableVertexAttribArray(shader_a.shaders[wireframe_shader_index].v_position);
		glVertexAttribPointer(shader_a.shaders[wireframe_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, (void *)(start));
			
		glDrawArrays(draw_mode, 0, vert_count);
		
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
			
		glDrawArrays(draw_mode, 0, vert_count);
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
	
	__attribute ((aligned(16))) command_buffer_t cb;
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
		
		//shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_LastModelViewMatrix, &model_view_matrix.floats[0][0]);
		//shader_SetCurrentShaderUniform1f(UNIFORM_EntityIndex, entity_index);
		
		//if(material_index != renderer.active_material_index)
		//{
		material_SetMaterialByIndex(material_index);
		//}


		//glBindBuffer(GL_ARRAY_BUFFER, gpu_buffer);
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
		
		//draw_mode &= ~(CBATTRIBUTE_NORMAL|CBATTRIBUTE_TANGENT|CBATTRIBUTE_TEX_COORD);
		glDrawArrays(draw_mode, 0, vert_count);
		
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
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
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
	
	draw_ResolveTranslucent();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	shader_SetShaderByIndex(composite_shader_index);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buffer.buffer_ID);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	/* additive blending */
	
	//while(glGetError()!=GL_NO_ERROR);
	
	/* lit scene */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);
	
	
	/* light volumes  */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, final_volume_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);
	
	/* could add variable exposure code here... */
	
	draw_DrawBloom();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	shader_SetShaderByIndex(composite_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buffer.buffer_ID);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	shader_SetCurrentShaderUniform1f(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, 1.0);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	/* additive blending */
	
	
	glBindTexture(GL_TEXTURE_2D, h_tex_l);
	glDrawArrays(GL_QUADS, 0, 4);
	
	glBindTexture(GL_TEXTURE_2D, q_tex_l);
	glDrawArrays(GL_QUADS, 0, 4);
	
	glBindTexture(GL_TEXTURE_2D, e_tex_l);
	glDrawArrays(GL_QUADS, 0, 4);
	
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
	//camera_t *active_camera = camera_GetActiveCamera();
	//float exposure = active_camera->exposure;
	
	//draw_ResolveTranslucent();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	shader_SetShaderByIndex(composite_shader_index);
	//shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	//shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, composite_buffer.width);
	//shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, composite_buffer.height);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buffer.buffer_ID);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_STENCIL_TEST);
	//glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	/* additive blending */
	
	//while(glGetError()!=GL_NO_ERROR);
	
	/* lit scene */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);
	
	/* light volumes  */
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, final_volume_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);
	
	
	

	
	glFlush();
	
	glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}


void draw_ComposeNoVolNoBloom()
{
	
	camera_t *active_camera = camera_GetActiveCamera();
	float exposure = active_camera->exposure;
	
	//draw_AutoAdjust();
	
	//draw_ResolveTranslucent();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	shader_SetShaderByIndex(composite_shader_index);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, composite_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, composite_buffer.height);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buffer.buffer_ID);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE);
	glBlendFunci(0, GL_ONE, GL_ONE);
	//glBlendFunc(GL_ONE, GL_ONE);
	/* lit scene */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, right_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);
	
	//glEnable(GL_DEPTH_TEST);
	
	glFlush();
}


void draw_ComposeNoVolBloom()
{
	
	camera_t *active_camera = camera_GetActiveCamera();
	float exposure = active_camera->exposure;
	
	//void draw_AutoAdjust();
	
	draw_ResolveTranslucent();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	shader_SetShaderByIndex(composite_shader_index);
	shader_SetCurrentShaderUniform1f(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1f(UNIFORM_TextureSampler1, 1);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buffer.buffer_ID);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	/* lit scene */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, left_buffer.color_out1);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, write_etex);
	glDrawArrays(GL_QUADS, 0, 4);
	
	draw_DrawBloom();
	
	framebuffer_BindFramebuffer(&composite_buffer);
	shader_SetShaderByIndex(composite_shader_index);
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buffer.buffer_ID);
	glEnableVertexAttribArray(shader_a.shaders[composite_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[composite_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
	shader_SetCurrentShaderUniform1f(UNIFORM_Exposure, exposure);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	/* additive blending */
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, h_tex_l);
	glDrawArrays(GL_QUADS, 0, 4);
	
	glBindTexture(GL_TEXTURE_2D, q_tex_l);
	glDrawArrays(GL_QUADS, 0, 4);
	
	glBindTexture(GL_TEXTURE_2D, e_tex_l);
	glDrawArrays(GL_QUADS, 0, 4);
	
	
	
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
	
	float use_shadows;
	int light_type = 0;
	int area_type = 0;
	unsigned int target;
	unsigned int uniform;
	
	float v[4];
	int draw_begin;
	int draw_count;
	int draw_mode;
	
    //affecting_lights_list light_list;
    camera_t *active_camera=camera_GetActiveCamera();
	
	//framebuffer_CopyFramebuffer(&left_buffer, &geometry_buffer, COPY_DEPTH);
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	framebuffer_BindFramebuffer(&left_buffer);
	glClear(GL_COLOR_BUFFER_BIT);
	
	
	
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
	//glActiveTexture(GL_TEXTURE5);
	//glBindTexture(GL_TEXTURE_2D, geometry_buffer.color_out3);
	
	
	/* 
	   To bear in mind:
	   Don't use the same texture unit for
	   different types of texture. It works,
	   but just kinda. 
	*/
	
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


	//orientation=active_camera->world_orientation;
	
	//mat4_t_compose(&cam_transform, &orientation, mul3(active_camera->world_position, 1.0));
	
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler1, 1);
	//hader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler2, 5);
	shader_SetCurrentShaderUniform1i(UNIFORM_DepthSampler, 2);
	
	shader_SetCurrentShaderUniform1i(UNIFORM_2DShadowSampler, 3);
	shader_SetCurrentShaderUniform1i(UNIFORM_3DShadowSampler, 4);
	
	shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraToWorldMatrix, &cam_transform.floats[0][0]);
	shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraProjectionMatrix, &active_camera->projection_matrix.floats[0][0]);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_camera->frustum.znear);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_camera->frustum.zfar);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, left_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, left_buffer.height);
	//glUniform1f(shader_a.shaders[shader_index].uniforms[UNIFORM_ZNear], active_camera->frustum.znear);
	//glUniform1f(shader_a.shaders[shader_index].uniforms[UNIFORM_ZFar], active_camera->frustum.zfar);
	
	mat4_t_compose(&camera_to_world_matrix, &active_camera->world_orientation, active_camera->world_position);
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	//glDisable(GL_CULL_FACE);
	glCullFace(GL_CCW);
	glEnable(GL_BLEND);
	
	//glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	
	glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
//	#define DEBUG_DRAW
	//if(renderer.render_mode==RENDERMODE_LIT)
	switch(renderer.render_mode)
	{

		case RENDER_DRAWMODE_LIT:
			shader_SetCurrentShaderUniform1i(UNIFORM_RenderDrawMode, RENDER_DRAWMODE_LIT);
			glBlendFunc(GL_ONE, GL_ONE);			/* additive blending... */
			//glEnable(GL_SCISSOR_TEST);
			
			c=active_light_a.light_count;
			for(i=0; i<active_light_a.light_count; i++)
			{
		 		//light_SetLight(i);
		 		
		 		//mat4_t_compose(&light_transform, &active_light_a.position_data[i].world_orientation, active_light_a.position_data[i].world_position.vec3);
		 		//mat4_t_mult(&model_view_matrix, &light_transform, &active_camera->world_to_camera_matrix);
		 		//glLoadMatrixf(&model_view_matrix.floats[0][0]);
		 		
		 		glLightfv(GL_LIGHT0, GL_POSITION, &active_light_a.position_data[i].world_position.floats[0]);
		 		
		 		/* forward vector */
		 		v[0] = active_light_a.position_data[i].world_orientation.floats[2][0];
				v[1] = active_light_a.position_data[i].world_orientation.floats[2][1];
				v[2] = active_light_a.position_data[i].world_orientation.floats[2][2];
				glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, v);
				
				/* up vector */
				v[0] = active_light_a.position_data[i].world_orientation.floats[1][0];
				v[1] = active_light_a.position_data[i].world_orientation.floats[1][1];
				v[2] = active_light_a.position_data[i].world_orientation.floats[1][2];
				glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, v); 
				
				/* right vector */
				v[0] = active_light_a.position_data[i].world_orientation.floats[0][0];
				v[1] = active_light_a.position_data[i].world_orientation.floats[0][1];
				v[2] = active_light_a.position_data[i].world_orientation.floats[0][2];
				glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, v);
		 		
		 		
		 		v[0] = (float)active_light_a.params[i].r / 255.0;
		 		v[1] = (float)active_light_a.params[i].g / 255.0;
		 		v[2] = (float)active_light_a.params[i].b / 255.0;
		 		v[3] = active_light_a.position_data[i].radius;
				glLightfv(GL_LIGHT0, GL_DIFFUSE, v);
				
				
				glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, (float)active_light_a.position_data[i].spot_co);
				glLighti(GL_LIGHT0, GL_SPOT_EXPONENT, active_light_a.params[i].spot_e);
				glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, (float)active_light_a.params[i].lin_fallof/0xffff);
				glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, (float)active_light_a.params[i].sqr_fallof/0xffff);
				
				
				/*imin_x=active_light_a.position_data[i].smin_x;
				imax_x=active_light_a.position_data[i].smax_x;
				imin_y=active_light_a.position_data[i].smin_y;
				imax_y=active_light_a.position_data[i].smax_y;*/
				
				light_type = 0;
				
				if(active_light_a.position_data[i].bm_flags&LIGHT_SPOT)
				{
					draw_begin = DRAW_CONE_LOD0_BEGIN;
					draw_count = DRAW_CONE_LOD0_COUNT;
					draw_mode = GL_TRIANGLES;
					light_type = LIGHT_SPOT;
					area_type = LIGHT_SPOT;
				}
				else if(active_light_a.position_data[i].bm_flags&LIGHT_POINT)
				{
					
					if(active_light_a.position_data[i].bm_flags & LIGHT_VIEWPOINT_INSIDE_VOLUME)
					{
						draw_begin = DRAW_SCREEN_QUAD_BEGIN;
						draw_count = DRAW_SCREEN_QUAD_COUNT;
						draw_mode = GL_QUADS;
						area_type = 0;
					}
					else
					{
						if(active_light_a.position_data[i].screen_value > 0.35)
						{
							draw_begin = DRAW_SPHERE_LOD0_BEGIN;
							draw_count = DRAW_SPHERE_LOD0_COUNT;
						}
						else
						{
							draw_begin = DRAW_SPHERE_LOD1_BEGIN;
							draw_count = DRAW_SPHERE_LOD1_COUNT;
						}
						
						draw_mode = GL_TRIANGLES;
						area_type = LIGHT_POINT;
					}
					light_type = LIGHT_POINT;
				
					
				}
				
				/* spot exponent goes to the vertex shader... */
				//glLighti(GL_LIGHT1, GL_SPOT_EXPONENT, area_type);
				light_SetAreaType(area_type);
				
				if(active_light_a.position_data[i].bm_flags&LIGHT_GENERATE_SHADOWS && renderer.renderer_flags&RENDERFLAG_USE_SHADOW_MAPS)
				{
					switch(light_type)
					{
						case LIGHT_SPOT:
							glActiveTexture(GL_TEXTURE3);
							glBindTexture(GL_TEXTURE_2D, active_light_a.shadow_data[i].shadow_map.shadow_map);
						break;
						
						case LIGHT_POINT:
							glActiveTexture(GL_TEXTURE4);
							glBindTexture(GL_TEXTURE_CUBE_MAP, active_light_a.shadow_data[i].shadow_map.shadow_map);
						break;
					}		

					mat4_t_mult(&camera_to_light_matrix, &camera_to_world_matrix, &active_light_a.extra_data[i].world_to_light_matrix);
					mat4_t_mult(&camera_to_light_projection_matrix, &camera_to_light_matrix, &active_light_a.extra_data[i].light_projection_matrix);

					shader_SetCurrentShaderUniform1f(UNIFORM_LightZNear, active_light_a.shadow_data[i].znear);
					shader_SetCurrentShaderUniform1f(UNIFORM_LightZFar, active_light_a.shadow_data[i].zfar);
					shader_SetCurrentShaderUniform1f(UNIFORM_ShadowMapSize, active_light_a.params[i].max_shadow_map_res * MIN_SHADOW_MAP_RES);
					shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraToLightProjectionMatrix, &camera_to_light_projection_matrix.floats[0][0]);
				}
				else
				{
					light_type = 0;
				}
				
				/* spot cutoff goes to the fragment shader... */
				//glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, light_type);
				light_SetLightType(light_type);
				glLighti(GL_LIGHT2, GL_SPOT_EXPONENT, active_light_a.params[i].max_shadow_aa_samples);
				glDrawArrays(draw_mode, draw_begin, draw_count);
			}
		break;
		
		case RENDER_DRAWMODE_WIREFRAME:
			shader_SetCurrentShaderUniform1i(UNIFORM_RenderDrawMode, RENDER_DRAWMODE_WIREFRAME);
			glLighti(GL_LIGHT1, GL_SPOT_EXPONENT, 0);
			glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		break;
		
		case RENDER_DRAWMODE_FLAT:
			shader_SetCurrentShaderUniform1i(UNIFORM_RenderDrawMode, RENDER_DRAWMODE_FLAT);
			glLighti(GL_LIGHT1, GL_SPOT_EXPONENT, 0);
			glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		break;
	}
	
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_CW);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	//glLighti(GL_LIGHT1, GL_SPOT_CUTOFF, 0);
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, 0);	
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
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
				glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, (void *)start);
					
				if(material_a.materials[material_index].bm_flags & MATERIAL_FrontAndBack)
				{
					glDisable(GL_CULL_FACE);
				}
				else
				{
					glEnable(GL_CULL_FACE);
				}
				glDrawArrays(draw_mode, 0, vert_count);
				
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
	int shader_to_use;
	mat3_t orientation;
	mat4_t cam_transform;
	float v[4];
	camera_t *active_camera=camera_GetActiveCamera();
	
	mat4_t camera_to_world_matrix;
	mat4_t camera_to_light_matrix;
	mat4_t camera_to_light_projection_matrix;
	
	
	//framebuffer_CopyFramebuffer(&right_volume_buffer, &geometry_buffer, COPY_DEPTH);
	framebuffer_BindFramebuffer(&right_volume_buffer);

	glDepthMask(GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	


	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dither_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, geometry_buffer.z_buffer);
	glActiveTexture(GL_TEXTURE2);
	
	mat4_t_compose(&camera_to_world_matrix, &active_camera->world_orientation, active_camera->world_position);

	c=active_light_a.light_count;
	
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
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
	shader_SetCurrentShaderUniform1i(UNIFORM_DepthSampler, 1);
	shader_SetCurrentShaderUniform1i(UNIFORM_3DShadowSampler, 2);
	
	shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraProjectionMatrix, &active_camera->projection_matrix.floats[0][0]);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_camera->frustum.znear);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_camera->frustum.zfar);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, right_volume_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, right_volume_buffer.height);
		
	
	for(i=0; i<c; i++)
	{
		
		
		
		//if(!(active_light_a.position_data[i].bm_flags&LIGHT_DRAW_VOLUME))
		//{
		//	continue;
		//}
		if(active_light_a.position_data[i].bm_flags&LIGHT_SPOT)
		{
			//shader_to_use=slvol_shader_index;	
			//glBindTexture(GL_TEXTURE_2D, active_light_a.shadow_data[i].shadow_map.shadow_map);
			continue;
		}
		else if(active_light_a.position_data[i].bm_flags&LIGHT_POINT)
		{
			shader_to_use=plvol_shader_index;
			glBindTexture(GL_TEXTURE_CUBE_MAP, active_light_a.shadow_data[i].shadow_map.shadow_map);
		}
		
		v[0] = active_light_a.position_data[i].world_position.x;
		v[1] = active_light_a.position_data[i].world_position.y;
		v[2] = active_light_a.position_data[i].world_position.z;
		v[3] = 1.0;
		glLightfv(GL_LIGHT0, GL_POSITION, v);
		 		
		v[0] = -active_light_a.position_data[i].world_orientation.floats[2][0];
		v[1] = -active_light_a.position_data[i].world_orientation.floats[2][1];
		v[2] = -active_light_a.position_data[i].world_orientation.floats[2][2];
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, v);
		 		
		 		
		v[0] = (float)active_light_a.params[i].r / 255.0;
		v[1] = (float)active_light_a.params[i].g / 255.0;
		v[2] = (float)active_light_a.params[i].b / 255.0;
		v[3] = active_light_a.position_data[i].radius;
		glLightfv(GL_LIGHT0, GL_DIFFUSE, v);
				
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, (float)active_light_a.position_data[i].spot_co);
		glLighti(GL_LIGHT0, GL_SPOT_EXPONENT, (int)active_light_a.params[i].spot_e);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, (float)active_light_a.params[i].lin_fallof/255.0);
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, (float)active_light_a.params[i].sqr_fallof/255.0);
		
		glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, ((float)active_light_a.params[i].scattering / (float)0xffff) * MAX_LIGHT_VOLUME_SCATTERING);
		glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, ((float)active_light_a.params[i].energy / (float)0xffff) * MAX_LIGHT_ENERGY);
		glLighti(GL_LIGHT2, GL_SPOT_CUTOFF, active_light_a.params[i].max_samples);
		
		mat4_t_mult(&camera_to_light_matrix, &camera_to_world_matrix, &active_light_a.extra_data[i].world_to_light_matrix);
		mat4_t_mult(&camera_to_light_projection_matrix, &camera_to_light_matrix, &active_light_a.extra_data[i].light_projection_matrix);

		shader_SetCurrentShaderUniformMatrix4fv(UNIFORM_CameraToLightProjectionMatrix, &camera_to_light_projection_matrix.floats[0][0]);
		
		shader_SetCurrentShaderUniform1f(UNIFORM_LightZNear, active_light_a.shadow_data[i].znear);
		shader_SetCurrentShaderUniform1f(UNIFORM_LightZFar, active_light_a.shadow_data[i].zfar);
		light_SetAreaType(LIGHT_POINT);
		glDrawArrays(GL_TRIANGLES, DRAW_SPHERE_LOD0_BEGIN, DRAW_SPHERE_LOD0_COUNT);

	}

	glFlush();
	//glDisable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glDepthMask(GL_TRUE);

	framebuffer_BindFramebuffer(&left_volume_buffer);
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
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, left_volume_buffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, left_volume_buffer.height);
	
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
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, final_volume_buffer.id);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, left_volume_buffer.id);
	glBlitFramebuffer(0, 0, left_volume_buffer.width, left_volume_buffer.height, 0, 0, final_volume_buffer.width, final_volume_buffer.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
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
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderVertexAttribArray(ATTRIBUTE_vPosition);
	shader_SetCurrentShaderUniform1f(UNIFORM_BloomIntensity, bloom_intensity);
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
	glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
	
	glEnable(GL_BLEND);
	glBlendFunci(0, GL_ONE, GL_ONE);
	
	
	shader_SetShaderByIndex(bloom_blur_shader_index);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	shader_SetCurrentShaderVertexAttribArray(ATTRIBUTE_vPosition);
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
		glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, h_tex_l, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, 0);
		glBindTexture(GL_TEXTURE_2D, h_tex_r);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
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
		glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, q_tex_l, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, 0);
		glBindTexture(GL_TEXTURE_2D, q_tex_r);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
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
		glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, e_tex_l, 0);
		shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, 0);
		glBindTexture(GL_TEXTURE_2D, e_tex_r);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_QUADS, DRAW_SCREEN_QUAD_BEGIN, DRAW_SCREEN_QUAD_COUNT);
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

	int i = GL_COLOR_ATTACHMENT0;
	
	
	framebuffer_BindFramebuffer(&backbuffer);
	glDisable(GL_STENCIL_TEST);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glDrawBuffer(GL_NONE);
	
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, geometry_buffer.id);
	//glReadBuffer(GL_NONE);
	
	glViewport(0, 0, backbuffer.width, backbuffer.height);
	glBlitFramebuffer(0, 0, geometry_buffer.width, geometry_buffer.height, 0, 0, backbuffer.width, backbuffer.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
	//glDrawBuffer(GL_LEFT);
	//glDrawBuffers(1, (GLenum *)&i);
	

	
	shader_SetShaderByIndex(screen_quad_shader_index);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_buffer.buffer_ID);
	glEnableVertexAttribArray(shader_a.shaders[screen_quad_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[screen_quad_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glActiveTexture(GL_TEXTURE0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	//shader_SetCurrentShaderUniform1i(UNIFORM_TextureSamplerCube0, 0);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);	/* additive blending */
	
	/* composite scene */
	glBindTexture(GL_TEXTURE_2D, composite_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);
	
	/*glBindTexture(GL_TEXTURE_2D, final_volume_buffer.color_out1);
	glDrawArrays(GL_QUADS, 0, 4);*/
	
	//glBindTexture(GL_TEXTURE_2D, right_buffer.color_out1);
	//glDrawArrays(GL_QUADS, 0, 4);
	
	//glBindTexture(GL_TEXTURE_2D, picking_buffer.color_out1);
	//glDrawArrays(GL_QUADS, 0, 4);
	
	//framebuffer_CopyFramebuffer(&backbuffer, &geometry_buffer, COPY_DEPTH);
	
	
                                               
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	
	//draw_DrawString(NULL, &font_a.fonts[0], 0, 0);
	
	
	
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE);	/* additive blending */
	

	
	
	
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
			return (int)(bloom_intensity*100.0);
		break;
	}
	return 0;
}

PEWAPI void draw_DrawString(char *str, font_t *font, int x, int y)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	//glDisable(GL_LIGHTING);
	
	//while(glGetError()!= GL_NO_ERROR);
	glRasterPos3f(-0.9, 0.0, -0.2);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glUseProgram(0);
	glEnable(GL_BLEND);
	glPixelZoom(1.0, 1.0);
	glColor3f(1.0, 1.0, 0.0);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	//printf("%c\n", font->chars[0].char_code);
	glDrawPixels(font->max_width * font->char_count, font->max_height, GL_RGBA, GL_UNSIGNED_BYTE, font->buffer + font->max_width * font->max_height * 0);
	
	//printf("%x\n", glGetError());
	
	
	/*glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glBindTexture(GL_TEXTURE_2D, font->tex);
	
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-0.9, 0.5, 0.0);
	
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.9, -0.5, 0.0);
	
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.9, -0.5, 0.0);
	
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.9, 0.5, 0.0);
	
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);*/
	
	glDisable(GL_BLEND);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


PEWAPI void draw_DrawWidgets()
{
	wbase_t *cwidget = widgets->next;
	wbase_t *cswidget;
	
	float x_scale = 2.0/(renderer.width * 0.01);
	float y_scale = 2.0/(renderer.height * 0.01);
	
	float scissor_x;
	float scissor_y;
	float scissor_w;
	float scissor_h;
	
	float header_x;
	float header_y;
	float header_w;
	float header_h;
	
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
	//glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);
	//glEnable(GL_BLEND);
	
	
	
	while(cwidget)
	{
		
		 
		if(!(cwidget->bm_flags & WIDGET_VISIBLE))
		{
			goto _skip_widget0;
		}
		
		if(cwidget->a < 1.0)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		
		/*scissor_x = cwidget->x * x_scale * 0.5 + 0.5;
		scissor_y = cwidget->y * y_scale * 0.5 + 0.5;
		scissor_w = cwidget->w * x_scale * 0.25;
		scissor_h = cwidget->h * y_scale * 0.25;*/
		
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0xff, 0xff);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		
		if(cwidget->tex_handle > 0)
		{
			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, cwidget->tex_handle);
			glBegin(GL_QUADS);
			glColor4f(cwidget->r, cwidget->g, cwidget->b, cwidget->a);
			glVertex3f(cwidget->x - cwidget->w / 2.0, cwidget->y + cwidget->h / 2.0, -0.5);
			glTexCoord2f(0.0, 1.0);
			glVertex3f(cwidget->x - cwidget->w / 2.0, cwidget->y - cwidget->h / 2.0, -0.5);
			glTexCoord2f(1.0, 1.0);
			glVertex3f(cwidget->x + cwidget->w / 2.0, cwidget->y - cwidget->h / 2.0, -0.5);
			glTexCoord2f(1.0, 0.0);
			glVertex3f(cwidget->x + cwidget->w / 2.0, cwidget->y + cwidget->h / 2.0, -0.5);
			glTexCoord2f(0.0, 0.0);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_QUADS);
			glColor4f(cwidget->r, cwidget->g, cwidget->b, cwidget->a);
			glVertex3f(cwidget->x - cwidget->w / 2.0, cwidget->y + cwidget->h / 2.0, -0.5);
			glVertex3f(cwidget->x - cwidget->w / 2.0, cwidget->y - cwidget->h / 2.0, -0.5);
			glVertex3f(cwidget->x + cwidget->w / 2.0, cwidget->y - cwidget->h / 2.0, -0.5);
			glVertex3f(cwidget->x + cwidget->w / 2.0, cwidget->y + cwidget->h / 2.0, -0.5);
			glEnd();
		}
		
		if(cwidget->bm_flags & WIDGET_HEADER)
		{
			glStencilFunc(GL_ALWAYS, 0x00, 0xff);
			glBegin(GL_QUADS);
			glColor4f(cwidget->r * 0.5, cwidget->g * 0.5, cwidget->b * 0.5, 1.0);
			glVertex3f(cwidget->x - cwidget->w / 2.0, cwidget->y + cwidget->h / 2.0, -0.5);
			glVertex3f(cwidget->x - cwidget->w / 2.0, cwidget->y + cwidget->h / 2.0 - 0.2, -0.5);
			glVertex3f(cwidget->x + cwidget->w / 2.0, cwidget->y + cwidget->h / 2.0 - 0.2, -0.5);
			glVertex3f(cwidget->x + cwidget->w / 2.0, cwidget->y + cwidget->h / 2.0, -0.5);
			glEnd();
			//header_h = 0.2 * y_scale * 0.25;
		}
		
		
		
		//glScissor((scissor_x - scissor_w)*renderer.width, (scissor_y - scissor_h)*renderer.height, scissor_w*renderer.width * 2.0, (scissor_h - header_h)*renderer.height * 2.0);
		
		
		
		glStencilFunc(GL_EQUAL, 0xff, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		
		cswidget = cwidget->w_widgets;
		while(cswidget)
		{
			swidget_x = (cswidget->relative_x + cwidget->x);
			swidget_y = (cswidget->relative_y + cwidget->y);
			
			if(cswidget->a < 1.0)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				glDisable(GL_BLEND);
			}
			
			switch(cswidget->type)
			{
				case WIDGET_BUTTON:
					
					if(cswidget->tex_handle > 0)
					{
						glEnable(GL_TEXTURE_2D);
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, cswidget->tex_handle);
						glBegin(GL_QUADS);
						glColor4f(cswidget->r, cswidget->g, cswidget->b, cswidget->a);
						glVertex3f(swidget_x - cswidget->relative_w / 2.0, swidget_y + cswidget->relative_h / 2.0, -0.5);
						glTexCoord2f(0.0, 1.0);
						glVertex3f(swidget_x - cswidget->relative_w / 2.0, swidget_y - cswidget->relative_h / 2.0, -0.5);
						glTexCoord2f(1.0, 1.0);
						glVertex3f(swidget_x + cswidget->relative_w / 2.0, swidget_y - cswidget->relative_h / 2.0, -0.5);
						glTexCoord2f(1.0, 0.0);
						glVertex3f(swidget_x + cswidget->relative_w / 2.0, swidget_y + cswidget->relative_h / 2.0, -0.5);
						glTexCoord2f(0.0, 0.0);
						glEnd();
						glBindTexture(GL_TEXTURE_2D, 0);
					}
					else
					{
						glBegin(GL_QUADS);
						glColor4f(cswidget->r, cswidget->g, cswidget->b, cswidget->a);
						glVertex3f(swidget_x - cswidget->relative_w / 2.0, swidget_y + cswidget->relative_h / 2.0, -0.5);
						glVertex3f(swidget_x - cswidget->relative_w / 2.0, swidget_y - cswidget->relative_h / 2.0, -0.5);
						glVertex3f(swidget_x + cswidget->relative_w / 2.0, swidget_y - cswidget->relative_h / 2.0, -0.5);
						glVertex3f(swidget_x + cswidget->relative_w / 2.0, swidget_y + cswidget->relative_h / 2.0, -0.5);
						glEnd();
					}
					
					
				break;
				
				case WIDGET_VERTICAL_SCROLLER:
					glBegin(GL_QUADS);
					glColor4f(cswidget->r * 0.5, cswidget->g * 0.5, cswidget->b * 0.5, cswidget->a);
					glVertex3f(swidget_x - cswidget->relative_w / 2.0, swidget_y + (cswidget->relative_h / 2.0), -0.5);
					glVertex3f(swidget_x - cswidget->relative_w / 2.0, swidget_y - (cswidget->relative_h / 2.0), -0.5);
					glVertex3f(swidget_x + cswidget->relative_w / 2.0, swidget_y - (cswidget->relative_h / 2.0), -0.5);
					glVertex3f(swidget_x + cswidget->relative_w / 2.0, swidget_y + (cswidget->relative_h / 2.0), -0.5);
					
					glColor4f(cswidget->r, cswidget->g , cswidget->b, cswidget->a);
					glVertex3f(swidget_x - (cswidget->relative_w / 2.0) * 0.85, swidget_y + (cswidget->relative_h / 2.0) - 0.05, -0.5);
					glVertex3f(swidget_x - (cswidget->relative_w / 2.0) * 0.85, swidget_y - (cswidget->relative_h / 2.0) - 0.05, -0.5);
					glVertex3f(swidget_x + (cswidget->relative_w / 2.0) * 0.85, swidget_y - (cswidget->relative_h / 2.0) - 0.05, -0.5);
					glVertex3f(swidget_x + (cswidget->relative_w / 2.0) * 0.85, swidget_y + (cswidget->relative_h / 2.0) - 0.05, -0.5);
					
					glEnd();
				break;
			}
			//glDisable(GL_BLEND);
			
			cswidget = cswidget->next;
		}
		
		_skip_widget0:
		
		cwidget = cwidget->next;
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













