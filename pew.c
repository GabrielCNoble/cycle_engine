#include "pew.h"
#include "SDL2/SDL_timer.h"
#include <intrin.h>

/* global declarations */
renderer_t renderer;
//sound_system sound;
//text_renderer_t text_renderer;
camera_array camera_a;

//render_queue render_q;
//render_queue t_render_q;
//render_queue shadow_q;
//shadow_queue shadow_q;

input_cache input;

//mesh_array mesh_a;
//material_array material_a;
//shader_array shader_a;
//texture_array texture_a;
//particle_system_array particle_system_a;
//active_particle_system_array active_particle_system_a;
//entity_array entity_a;
//scenegraph_t scenegraph;
//collider_array collider_a;
console_font font;
//font_array font_a;
//gelem_array gelem_a;
console_t console;
//source_array source_a;
//abuffer_array abuffer_a;
//pcm_array pcm_a;
pew_t pew;
//light_array light_a;
//light_array active_light_a;
 
//static TIME_INFO ti;
//static float last_delta;

char backtrace[2048];



#ifdef __cplusplus
extern "C"
{
#endif


/*
=============
pew_Init
=============
*/
PEWAPI void pew_Init(int resolution, int mode)
{
	
	pew.esc_state=ESC_RELEASED;
	
	draw_Init(resolution, mode);
	gpu_Init();
	console_Init();
	physics_Init();
	vcache_Init();
	sound_Init();
	text_Init("fonts\\");
	input_Init();
	model_Init("models\\");
	entity_Init();
	light_Init();
	shader_Init("shaders\\");
	texture_Init("textures\\");
	material_Init("materials\\");
	//particle_Init();
	scenegraph_Init();
	camera_Init();
	armature_Init();
	//physics_Init();
	gui_Init();
	
	//last_delta = 0.0;
	pew.ti.ms_elapsed = 0.0;
	pew.ti.end.tv_usec = 0;
	pew.ti.end.tv_sec = 0;
	pew.ti.start.tv_sec = 0;
	pew.ti.start.tv_usec = 0;
	pew.time_scale = 1.0;
	//pew_SetSigSegHandler(pew_HandleSigSeg);
	
	//timeBeginPeriod(1);
	
	return;
}


/*
=============
pew_Finish
=============
*/
PEWAPI void pew_Finish()
{
	gui_Finish();
	printf("1\n");
	//physics_Finish();
	armature_Finish();
	printf("2\n");
	camera_Finish();
	printf("3\n");
	scenegraph_Finish();
	printf("4\n");
	material_Finish();
	printf("5\n");
	vcache_Finish();
	printf("6\n");
	//particle_Finish();
	texture_Finish();
	printf("7\n");
	shader_Finish();
	printf("8\n");
	light_Finish();
	printf("9\n");
	entity_Finish();
	printf("10\n");
	model_Finish();
	printf("11\n");
	input_Finish();
	printf("12\n");
	text_Finish();
	printf("13\n");
	sound_Finish();
	printf("14\n");
	gpu_Finish();
	printf("15\n");
	draw_Finish();
	printf("16\n");
	physics_Finish();
	printf("17\n");
	console_Finish();
	printf("18\n");
	return;
}


/*
=============
pew_MainLoop
=============
*/
PEWAPI void pew_MainLoop()
{
	int a;
	mat4_t m=mat4_t_id();
	vec4_t q;
	vec4_t v;
	

	pew.gameinit_function();
	float f;
	float lastf = 0.0;
	float fps_sum;
	float fps_disp;
	static float max = -1.0;
	static float min = 100000000.0;
	long long unsigned int c0;
	long long unsigned int c1;
	pew_Resume();
	pew_UpdateDeltaTime();
	while(pew.pew_state)
	{
		
		//printf("avarage fps (10 frames): %.02f	 \r", fps_disp);
		
		pew_UpdateDeltaTime();
		f = pew_GetDeltaTime();
		draw_OpenFrame();
		//sound_ProcessSound();
		input_GetInput();
		gui_ProcessWidgets();
		console_ProcessConsole();
		if(!(pew.pew_state&PEW_PAUSED) && !(pew.b_console))
		{
			armature_ProcessArmatures(pew.ti.ms_elapsed * pew.time_scale);
			physics_ProcessCollisions(pew.ti.ms_elapsed * pew.time_scale); 
		}
		
		scenegraph_ProcessScenegraph();	
		pew.gamemain_function(pew.ti.ms_elapsed * pew.time_scale);
		
		//f = pew_GetDeltaTime();
		lastf = pew_GetDeltaTime();
		//printf("processing: %.02f ", lastf - f);
		
		
		draw_DrawFrame();
		draw_CloseFrame();
		//lastf = pew_GetDeltaTime();
		
		//printf("rendition: %.02f ", pew_GetDeltaTime() - lastf);
		
		fps_sum = 1000.0 / pew_GetDeltaTime();
		//if(fps_sum > max) max = fps_sum;
		//if(fps_sum < min) min = fps_sum;
		//printf("fps: %0.2f max: %0.2f min: %0.2f	\r",  fps_sum, max, min);
		printf("fps: %0.2f	\r",  fps_sum);
		
	}

}


/*
=============
pew_SetInitGameFunction
=============
*/
PEWAPI void pew_SetInitGameFunction(void (*gameinit_function)())
{
	pew.gameinit_function=gameinit_function;
	return;
}


/*
=============
pew_SetMainGameFunction
=============
*/
PEWAPI void pew_SetMainGameFunction(void (*gamemain_function)(float ))
{
	pew.gamemain_function=gamemain_function;
}



PEWAPI void pew_SetPauseGameFunction(void (*pause_function)())
{
	pew.pause_function=pause_function;
}




PEWAPI void pew_SetResumeGameFunction(void (*resume_function)())
{
	pew.resume_function=resume_function;
}




PEWAPI void pew_SetPewState(int pew_state)
{
	pew.pew_state=pew_state;
}

PEWAPI int pew_GetPewState()
{
	return pew.pew_state;
}



PEWAPI void pew_Pause()
{
	if(pew.pew_state!=PEW_PAUSED)
	{
		pew.pew_state=PEW_PAUSED;
		pew.pause_function();
		//sound_PauseAllSources();
	}
	
}



PEWAPI void pew_Resume()
{
	if(pew.pew_state!=PEW_PLAYING)
	{
		pew.pew_state=PEW_PLAYING;
		pew.resume_function();
		//sound_ResumeAllSources();
	}
	
}

PEWAPI void pew_Exit()
{
	pew.pew_state = PEW_EXIT;
}

PEWAPI float pew_GetTime()
{
	return renderer.time;
}

PEWAPI void pew_SetTimeScale(float scale)
{
	if(scale > 0.0 && scale < 5.0)
		pew.time_scale = scale;
}

PEWAPI float pew_GetTimeScale()
{
	return pew.time_scale;
}

PEWAPI float pew_UpdateDeltaTime()
{
	struct timezone t;
	mingw_gettimeofday(&pew.ti.end, &t);
	pew.ti.ms_elapsed=((double)(pew.ti.end.tv_usec-pew.ti.start.tv_usec)/1000)+((double)(pew.ti.end.tv_sec-pew.ti.start.tv_sec))*1000;
	pew.ti.start = pew.ti.end;
	return pew.ti.ms_elapsed;
}

PEWAPI float pew_GetDeltaTime()
{
	struct timezone t;
	struct timeval tv;
	mingw_gettimeofday(&tv, &t);
	return (((double)(tv.tv_usec-pew.ti.start.tv_usec)/1000)+((double)(tv.tv_sec-pew.ti.start.tv_sec))*1000);
}

PEWAPI void pew_SetSigSegHandler(void handler_fn(int))
{
	signal(SIGSEGV, (__p_sig_fn_t)handler_fn);
}

void pew_HandleSigSeg(int signal)
{
	printf("\noh shit...\n");
	//int i;
	//log_LogMessage("SIGSEGV\nbacktrace:\n");
	
	
	
	
	/*int *p = (int *)malloc(sizeof(int) * 1000);
	
	for(i = 0; i < 20; i++)
	{
		printf("%d\n", p[i]);
	}
	puts("\n\n");
	
	RtlCaptureStackBackTrace(0, 1, (void **)&p, 0);
	
	for(i = 0; i < 20; i++)
	{
		printf("%x\n", p[i]);
	}*/
	
	//pew_Finish();
	exit(-1);
}



#ifdef __cplusplus
}
#endif






