#ifndef _MATRIX_TYPES_H_
#define _MATRIX_TYPES_H_

#pragma once

//#include "includes.h"
#include "GL/glew.h"
#include "vector_types.h"




typedef struct mat2_t
{
	GLfloat floats[2][2];
}mat2_t;

typedef union mat3_t
{
	struct
	{
		GLfloat floats[3][3];
	};
	
	struct
	{
		GLfloat lfloats[9];
	};
	
	struct
	{
		GLfloat a00;
		GLfloat a01;
		GLfloat a02;
		
		GLfloat a10;
		GLfloat a11;
		GLfloat a12;
		
		GLfloat a20;
		GLfloat a21;
		GLfloat a22;
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
		GLfloat floats[4][4];
	};
	
	struct
	{
		GLfloat lfloats[16];
	};
	
	struct
	{
		GLfloat a00;
		GLfloat a01;
		GLfloat a02;
		GLfloat a03;
		
		GLfloat a10;
		GLfloat a11;
		GLfloat a12;
		GLfloat a13;
		
		GLfloat a20;
		GLfloat a21;
		GLfloat a22;
		GLfloat a23;
		
		GLfloat a30;
		GLfloat a31;
		GLfloat a32;
		GLfloat a33;
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
#endif // _MATRIX_TYPES_H_





