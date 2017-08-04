#ifndef DRAW_PROFILE_H
#define DRAW_PROFILE_H


#define PROFILE_RENDERER

enum STAGE_PROFILING
{
	STAGE_GBUFFER_FILL = 1,
	STAGE_GBUFFER_PROCESS,
	STAGE_EMISSIVE_PROCESS,
	STAGE_TRANSLUCENT_PROCESS,
	STAGE_SHADOW_MAP_GENERATION,
	STAGE_LIGHT_VOLUME_GENERATION,
	STAGE_GUI,
	
};


void draw_profile_Init();

void draw_profile_Finish();

void draw_profile_StartTimer();

float draw_profile_StopTimer();

extern void (*draw_profile_StartCollectionOfStageStatistics)(int stage);

extern void (*draw_profile_StopCollectionOfStageStatistics)();

void draw_profile_StartCollectionOfRendererStatistics();

void draw_profile_StopCollectionOfRendererStatistics();

void draw_profile_OutputRendererPerformanceLog();






#endif 
