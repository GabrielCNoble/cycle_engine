#ifndef _VECTOR_TYPES_H_
#define _VECTOR_TYPES_H_

//#include "GL/glew.h"


typedef struct
{
	struct
	{
		float x;
		float y;
	};
	
	struct
	{
		float floats[2];	
	};
	
	struct
	{
		float a0;
		float a1;
	};
}vec2_t;

typedef union 
{
	struct
	{
		float x;
		float y;
		float z;
	};
	
	struct
	{
		float floats[3];	
	};
	
	struct
	{
		float r;
		float g;
		float b;
	};
	
	struct
	{
		float a0;
		float a1;
		float a2;
	};
	
}vec3_t;

typedef union 
{
	struct 
	{
		float x;
		float y;
		float z;
		float w;
	};
	
	struct
	{
		float floats[4];	
	};
	
	struct 
	{
		float r;
		float g;
		float b;
		float a;
	};
	
	struct
	{
		float a0;
		float a1;
		float a2;
		float a3;
	};
	struct
	{
		vec3_t vec3;
		float pad;
	};
	
}vec4_t;


typedef union quaternion_t 
{
	struct
	{
		float x, y, z, w;
	};
	struct
	{
		vec3_t v;
		float s;
	};
	struct 
	{
		float floats[4];
	};
}quaternion_t;

//typedef vec4_t quaternion_t;




#endif //_VECTOR_TYPES_H_
