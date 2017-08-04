#include "draw_profile.h"
#include "pew.h"
#include "includes.h"
#include <stdio.h>
#include "entity.h"
#include "light.h"

#include "draw_types.h"

extern renderer_t renderer;

float fill_gbuffer_time;
float process_gbuffer_time;
float generate_shadow_map_time;
float gui_time;

float total_fill_gbuffer_time;
float total_process_gbuffer_time;
float total_generate_shadow_map_time;
float total_gui_time;

static unsigned int timer;
static unsigned int collection_start_frame_count;
static unsigned int collection_end_frame_count;
static int current_profiling_stage;


void (*draw_profile_StartCollectionOfStageStatistics)(int stage);

void (*draw_profile_StopCollectionOfStageStatistics)();


void draw_profile_Init()
{
	glGenQueries(1, &timer);
}


void draw_profile_Finish()
{
	glDeleteQueries(1, &timer);
}

void draw_profile_StartTimer()
{	
	glBeginQuery(GL_TIME_ELAPSED, timer);
}

float draw_profile_StopTimer()
{
	unsigned int elapsed;
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(timer, GL_QUERY_RESULT, &elapsed);	
	return (float)elapsed / 1000000.0;
}

void draw_profile_StartCollectionOfStageStatisticsNop(int stage)
{
	
}

void draw_profile_StartCollectionOfStageStatisticsReal(int stage)
{
	switch(stage)
	{
		case STAGE_GBUFFER_FILL:
		case STAGE_GBUFFER_PROCESS:
		case STAGE_SHADOW_MAP_GENERATION:
		case STAGE_GUI:
			current_profiling_stage = stage;
			draw_profile_StartTimer();
		break;
	}
}

void draw_profile_StopCollectionOfStageStatisticsNop()
{
	
}

void draw_profile_StopCollectionOfStageStatisticsReal()
{
	switch(current_profiling_stage)
	{
		case STAGE_GBUFFER_FILL:
			fill_gbuffer_time = draw_profile_StopTimer();	
			total_fill_gbuffer_time += fill_gbuffer_time;
			current_profiling_stage = 0;
		break;
		
		case STAGE_GBUFFER_PROCESS:
			process_gbuffer_time = draw_profile_StopTimer();
			total_process_gbuffer_time += process_gbuffer_time;
			current_profiling_stage = 0;
		break;
		
		case STAGE_SHADOW_MAP_GENERATION:
			generate_shadow_map_time = draw_profile_StopTimer();
			total_generate_shadow_map_time += generate_shadow_map_time;
			current_profiling_stage = 0;
		break;
		
		case STAGE_GUI:
			gui_time = draw_profile_StopTimer();
			total_gui_time += gui_time;
			current_profiling_stage = 0;
		break;
	}
}

void draw_profile_StartCollectionOfRendererStatistics()
{
	draw_profile_StartCollectionOfStageStatistics = draw_profile_StartCollectionOfStageStatisticsReal;
	draw_profile_StopCollectionOfStageStatistics = draw_profile_StopCollectionOfStageStatisticsReal;
	
	total_fill_gbuffer_time = 0.0;
	total_process_gbuffer_time = 0.0;
	total_generate_shadow_map_time = 0.0;
	total_gui_time = 0.0;
	
}

void draw_profile_StopCollectionOfRendererStatistics()
{
	draw_profile_StartCollectionOfStageStatistics = draw_profile_StartCollectionOfStageStatisticsNop;
	draw_profile_StopCollectionOfStageStatistics = draw_profile_StopCollectionOfStageStatisticsNop;
	
	collection_end_frame_count = renderer.frame_count;
	
	int delta_frame = collection_end_frame_count - collection_start_frame_count;
	
	total_fill_gbuffer_time /= (float)delta_frame;
	total_process_gbuffer_time /= (float)delta_frame;
	total_generate_shadow_map_time /= (float)delta_frame;
	total_gui_time /= (float)delta_frame;
	
	draw_profile_OutputRendererPerformanceLog();
	
	
}

void draw_profile_OutputRendererPerformanceLog()
{
	FILE *log;
	log = fopen("renderer.plog", "a");
	int light_count = light_GetLightCount();
	int i;
	
	fprintf(log, "##########################\n\n");
	fprintf(log, "frames: %d\n\n", collection_end_frame_count - collection_start_frame_count);
	fprintf(log, "avarage fill gbuffer time: %f\n"
				 "avarage process gbuffer time: %f\n"
				 "avarage generate shadow map time: %f\n"
				 "avarage gui time: %f\n", total_fill_gbuffer_time, 
				 						   total_process_gbuffer_time,
										   total_generate_shadow_map_time,
										   total_gui_time);
	fprintf(log, "##########################\n\n");									   

	
	
	
}








