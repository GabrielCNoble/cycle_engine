#ifndef PEW_H
#define PEW_H

#include <time.h>
#include <signal.h>

#ifdef _WIN32
	#ifndef __MSXML_LIBRARY_DEFINED__
		#define __MSXML_LIBRARY_DEFINED__
	#endif
	#include <windows.h>
#endif

#include "conf.h"
#include "includes.h"
#include "draw.h"
#include "input.h"
#include "model.h"
#include "entity.h"
#include "scenegraph.h"
#include "material.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "physics.h"
#include "text.h"
#include "gui.h"
#include "console.h"
#include "sound.h"
#include "particle.h"
#include "particle_behaviour.h"
#include "gpu.h"
#include "framebuffer.h"
#include "light.h"
#include "armature.h"
#include "log.h"
#include "vcache.h"
//#include "b3Clock.h"
//#include "python.h"


enum PEW_STATUS
{
	PEW_EXIT=0,
	PEW_PAUSED=1,
	PEW_PLAYING=2,
	PEW_PAUSED_BY_CONSOLE=4
};

typedef struct
{
	struct timeval start;
	struct timeval end;
	double ms_elapsed; 
}time_info_t;

typedef struct pew_t
{
	void (*gameinit_function)();
	void (*gamemain_function)(float );
	void (*pause_function)();
	void (*resume_function)();
	int esc_state;
	int pew_state;
	int b_console;
	time_info_t ti;
	float time_scale;
}pew_t;



#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void pew_Init(int resolution, int b_fullscreen);

PEWAPI void pew_Finish();

PEWAPI void pew_MainLoop();

PEWAPI void pew_SetInitGameFunction(void (*gameinit_function)());

PEWAPI void pew_SetMainGameFunction(void (*gamemain_function)(float ));

PEWAPI void pew_SetPauseGameFunction(void (*pause_function)());

PEWAPI void pew_SetResumeGameFunction(void (*resume_function)());

PEWAPI void pew_SetPewState(int pew_state);

PEWAPI int pew_GetPewState();

PEWAPI void pew_Pause();

PEWAPI void pew_Resume();

PEWAPI void pew_Exit();

PEWAPI float pew_GetTime();

PEWAPI void pew_SetTimeScale(float scale);

PEWAPI float pew_GetTimeScale();

PEWAPI float pew_UpdateDeltaTime();

PEWAPI float pew_GetDeltaTime();

PEWAPI void pew_SetSigSegHandler(void handler_fn(int));

void pew_HandleSigSeg(int signal);

char *pew_StackBacktrace();

PEWAPI static inline unsigned long long pew_GetCycleCount();



#ifdef __cplusplus
}
#endif


#ifndef PEW_INLINES_INL
#include "pew.inl"
#endif /* ifndef PEW_INLINES_INL */


#endif /* PEW_H */























