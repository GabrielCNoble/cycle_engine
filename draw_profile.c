#include "draw_profile.h"
#include "pew.h"
#include "includes.h"

static unsigned int timer;


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
	float ms;
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(timer, GL_QUERY_RESULT, &elapsed);
	
	ms = (float)elapsed / 1000000.0;
	
	return ms;
	
}








