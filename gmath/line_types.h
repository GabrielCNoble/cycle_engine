#ifndef LINE_TYPES_H
#define LINE_TYPES_H

#include "vector.h"


typedef struct
{
	vec2_t a;
	vec2_t b;
	vec2_t v;
}line2_t;

typedef struct
{
	vec3_t a;
	vec3_t b;
	vec3_t v;
}line3_t;



#endif /* LINE_TYPES_H */
