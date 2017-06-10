#include "line.h"


PEWAPI line2_t line2(vec2_t a, vec2_t b)
{
	line2_t l;
	l.a=a;
	l.b=b;
	l.v=normalize2(sub2(b, a));
	return l;
}

PEWAPI line3_t line3(vec3_t a, vec3_t b)
{
	line3_t l;
	l.a=a;
	l.b=b;
	l.v=normalize3(sub3(b, a));
	return l;
}

PEWAPI int line_PointInLine2(line2_t line, vec3_t p)
{
	float a=line.v.floats[1]*(p.floats[0]-line.a.floats[0]);
	float b=line.v.floats[2]*(p.floats[1]-line.a.floats[1]);

	
	if(a==b) return 0;
	if(a<b) return -1;
	if(a>b) return 1;
}
