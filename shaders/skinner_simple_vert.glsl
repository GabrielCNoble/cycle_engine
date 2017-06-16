#version 120 

#include "default_attribs.h"



#pragma capture(vertex)   
#pragma capture(normal)                                                                 



void main()
{
	gl_Position = vPosition;
	_vcap_ = vPosition.xyz;
	_ncap_ = vPosition.xyz;
}
