#ifndef TRI_H
#define TRI_H

#include "vector.h"

/*typedef struct
{
	float x, y, z;
}vec3_t;

typedef struct
{
	vec3_t n;
	vec3_t p;
	float d;
}plane_t;*/


//vec3_t cross(vec3_t a, vec3_t b);

//float dot(vec3_t a, vec3_t b);

vec3_t lerp(vec3_t a, vec3_t b, float l);

//vec3_t normalize(vec3_t v);

//plane_t compute_plane(vec3_t a, vec3_t b, vec3_t c);

//vec3_t intersect_plane(vec3_t a, vec3_t b, plane_t p);

void triangulate(vec3_t *in_verts, int in_vert_count, vec3_t **out_verts, int *out_vert_count);

int trng_rec(vec3_t *in_verts, int in_vert_count, vec3_t *out_verts, int *out_vert_count);







#endif
