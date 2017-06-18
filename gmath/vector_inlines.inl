#ifndef VECTOR_INLINES_INL
#define VECTOR_INLINES_INL

#include "vector_types.h"

PEWAPI float dot3(vec3_t vec1, vec3_t vec2)
{
	return (vec1.floats[0]*vec2.floats[0] + vec1.floats[1]*vec2.floats[1] + vec1.floats[2]*vec2.floats[2]);
}

PEWAPI float length3(vec3_t vec)
{
	return sqrt((vec.floats[0]*vec.floats[0]) + (vec.floats[1]*vec.floats[1]) + (vec.floats[2]*vec.floats[2]));
}

PEWAPI vec3_t vec3(float x, float y, float z)
{
	vec3_t result;
	result.floats[0]=x;
	result.floats[1]=y;
	result.floats[2]=z;
	return result;
}

PEWAPI vec4_t vec4(float x, float y, float z, float w)
{
	vec4_t result;
	result.floats[0]=x;
	result.floats[1]=y;
	result.floats[2]=z;
	result.floats[3]=w;
	return result;
}

PEWAPI vec3_t mul3(vec3_t vec, float value)
{
	vec3_t result;
	result.floats[0]=vec.floats[0]*value;
	result.floats[1]=vec.floats[1]*value;
	result.floats[2]=vec.floats[2]*value;
	return result;
}

PEWAPI vec3_t add3(vec3_t vec1, vec3_t vec2)
{
	vec3_t result;
	result.floats[0]=vec1.floats[0]+vec2.floats[0];
	result.floats[1]=vec1.floats[1]+vec2.floats[1];
	result.floats[2]=vec1.floats[2]+vec2.floats[2];
	return result;
}

PEWAPI vec3_t sub3(vec3_t vec1, vec3_t vec2)
{
	vec3_t result;
	result.floats[0]=vec1.floats[0]-vec2.floats[0];
	result.floats[1]=vec1.floats[1]-vec2.floats[1];
	result.floats[2]=vec1.floats[2]-vec2.floats[2];
	return result;
}

PEWAPI vec3_t vec4vec3(vec4_t vec)
{
	vec3_t v;
	v.floats[0]=vec.floats[0];
	v.floats[1]=vec.floats[1];
	v.floats[2]=vec.floats[2];
	return v;
}

PEWAPI vec4_t vec3vec4(vec3_t vec)
{
	vec4_t v;
	v.floats[0]=vec.floats[0];
	v.floats[1]=vec.floats[1];
	v.floats[2]=vec.floats[2];
	v.floats[3]=1.0;
	return v;
}

PEWAPI vec3_t normalize3(vec3_t vec)
{
	vec3_t temp;
	float length=sqrt((vec.floats[0]*vec.floats[0]) + (vec.floats[1]*vec.floats[1]) + (vec.floats[2]*vec.floats[2]));
	temp.floats[0]=vec.floats[0]/length;
	temp.floats[1]=vec.floats[1]/length;
	temp.floats[2]=vec.floats[2]/length;
	return temp;
}

PEWAPI vec3_t cross(vec3_t vec1, vec3_t vec2)
{
	vec3_t result;
	result.floats[0]=vec1.floats[1]*vec2.floats[2] - vec1.floats[2]*vec2.floats[1];
	result.floats[1]=vec1.floats[2]*vec2.floats[0] - vec1.floats[0]*vec2.floats[2];
	result.floats[2]=vec1.floats[0]*vec2.floats[1] - vec1.floats[1]*vec2.floats[0];
	return result;
}

PEWAPI vec3_t gs_orthg(vec3_t ref, vec3_t src)
{
	vec3_t r;
	float d1 = dot3(ref, src);
	float d2 = dot3(ref, ref);
	float d3 = d1 / d2;
	
	r.floats[0] = src.floats[0] - ref.floats[0] * d3;
	r.floats[1] = src.floats[1] - ref.floats[1] * d3;
	r.floats[2] = src.floats[2] - ref.floats[2] * d3;
	
	r = normalize3(r);
	return r;
}

PEWAPI vec3_t lerp3(vec3_t a, vec3_t b, float t)
{
	vec3_t r;
	float q = 1.0 - t;
	r.x = a.x * q + b.x * t;
	r.y = a.y * q + b.y * t;
	r.z = a.z * q + b.z * t;
	return r;
}

PEWAPI quaternion_t lerp4(quaternion_t *a, quaternion_t *b, float t)
{
	quaternion_t r;
	float l;
	float q = 1.0 - t;
	
	r.x = a->x * q + b->x * t;
	r.y = a->y * q + b->y * t;
	r.z = a->z * q + b->z * t;
	r.w = a->w * q + b->w * t;
	l = sqrt(r.x * r.x + r.y * r.y + r.z * r.z + r.w * r.w);	
	r.x /= l;
	r.y /= l;
	r.z /= l;
	r.w /= l;
	
	return r;
}

PEWAPI quaternion_t slerp(quaternion_t *a, quaternion_t *b, float t)
{
	quaternion_t q;
	float c;
	float s;
	float i;
	float o;
	
	if(a->x == b->x &&
	   a->y == b->y &&
	   a->z == b->z &&
	   a->w == b->w)
	{
		q.x = a->x;
		q.y = a->y;
		q.z = a->z;
		q.w = a->w;
	}
	else
	{
		c = acos(dot4((vec4_t *)a, (vec4_t *)b));
		s = sin(c);
		i = sin((1.0 - t) * c);
		o = sin(t * c);
		
		q.x = (a->x * i + b->x * o) / s;
		q.y = (a->y * i + b->y * o) / s;
		q.z = (a->z * i + b->z * o) / s;
		q.w = (a->w * i + b->w * o) / s;
	}
	return q;
}


/*PEWAPI quaternion_t squad(quaternion_t *a, quaternion_t *b, quaternion_t *c, float t)
{
	quaternion_t s0;
	quaternion_t s1;
	quaternion_t l0;
	quaternion_t l1;
	quaternion_t q;
	q.x = -b->x;
	q.y = -b->y;
	q.z = -b->z;
	
	l0 = qlog(quat_mult(q, c));
	l1 = qlog(quat_mult(q, a));
	
	l0.x = (l0.x + l1.x) / -4.0;
	l0.y = (l0.y + l1.y) / -4.0;
	l0.z = (l0.z + l1.z) / -4.0;
	l0.w = (l0.w + l1.w) / -4.0;
	
	s0 = quat_mult(b, l0);
	
}*/

PEWAPI quaternion_t qlog(quaternion_t *q)
{
	quaternion_t r;
	
	float c = acos(q->w);
	float s = sin(c);
	if(s == 0.0) s = 1.0;
	r.x = (q->x / s) * c;
	r.y = (q->y / s) * c;
	r.z = (q->z / s) * c;
	r.w = 0;
	return r;
}

PEWAPI quaternion_t qexp(quaternion_t *q)
{
	quaternion_t r;
	float a = sqrt(q->x * q->x + q->y * q->y + q->z * q->z) - 1.0;
	float s = sin(a);
	float c = cos(a);
	if(a == 0.0) a = 1.0;
	r.x = (q->x / a) * s;
	r.y = (q->y / a) * s;
	r.z = (q->z / a) * s;
	r.w = c;
	
	return r;
}

PEWAPI float dot4(vec4_t *a, vec4_t *b)
{
	return (a->floats[0] * b->floats[0] +
			a->floats[1] * b->floats[1] +
			a->floats[2] * b->floats[2] +
			a->floats[3] * b->floats[3]);
}

PEWAPI quaternion_t qinverse(quaternion_t *q)
{
	quaternion_t r;
	
	float l = q->floats[0] * q->floats[0] +
			  q->floats[1] * q->floats[1] +
			  q->floats[2] * q->floats[2] +
			  q->floats[3] * q->floats[3];
			  
	/*  q* / (||q||^2)  */		  
	r.floats[0] = -q->floats[0] / l;	
	r.floats[1] = -q->floats[1] / l;
	r.floats[2] = -q->floats[2] / l;
	r.floats[3] =  q->floats[3] / l;
	
	return r;	  
}

PEWAPI quaternion_t qmult(quaternion_t *a, quaternion_t *b)
{
	quaternion_t q;
	
	/*vxv' + sv' + s'v*/
	
	q.floats[0] = a->floats[1] * b->floats[2] - a->floats[2] * b->floats[1] + 
	  			  a->floats[3] * b->floats[0] + b->floats[3] * a->floats[0];
	  			  
	q.floats[1] = a->floats[0] * b->floats[2] - a->floats[2] * b->floats[0] + 
	  			  a->floats[3] * b->floats[1] + b->floats[3] * a->floats[1];  
	
	q.floats[2] = a->floats[0] * b->floats[1] - a->floats[1] * b->floats[0] + 
	  			  a->floats[3] * b->floats[2] + b->floats[3] * a->floats[2];							  
	
	/* ss' - v . v' */
	q.floats[3] = a->floats[3] * b->floats[3] - 
				  a->floats[0] * b->floats[0] + 
				  a->floats[1] * b->floats[1] +
				  a->floats[2] * b->floats[2];
}
#endif /* VECTOR_INLINES_INL */













