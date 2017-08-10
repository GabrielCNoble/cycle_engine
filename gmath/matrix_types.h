#ifndef _MATRIX_TYPES_H_
#define _MATRIX_TYPES_H_


//#include "includes.h"
#include "vector_types.h"




typedef struct mat2_t
{
	float floats[2][2];
}mat2_t;

typedef union mat3_t
{
	struct
	{
		float floats[3][3];
	};
	
	struct
	{
		float lfloats[9];
	};
	
	struct
	{
		float a00;
		float a01;
		float a02;
		
		float a10;
		float a11;
		float a12;
		
		float a20;
		float a21;
		float a22;
	};
	
	struct
	{
		vec3_t r0;
		vec3_t r1;
		vec3_t r2;
	};
	
	struct
	{
		vec3_t r_axis;
		vec3_t u_axis;
		vec3_t f_axis;
	};
	
}mat3_t;

typedef union mat4_t
{
	struct
	{
		float floats[4][4];
	};
	
	struct
	{
		float lfloats[16];
	};
	
	struct
	{
		float a00;
		float a01;
		float a02;
		float a03;
		
		float a10;
		float a11;
		float a12;
		float a13;
		
		float a20;
		float a21;
		float a22;
		float a23;
		
		float a30;
		float a31;
		float a32;
		float a33;
	};
	
	struct
	{
		vec4_t r0;
		vec4_t r1;
		vec4_t r2;
		vec4_t r3;
	};
	
	struct
	{
		vec3_t r_axis;
		float e0;
		vec3_t u_axis;
		float e1;
		vec3_t f_axis;
		float e2;
		vec3_t pos;
		float e3;
	};
	
}mat4_t;


typedef __attribute((aligned(16)))mat4_t amat4_t; 


#endif // _MATRIX_TYPES_H_





