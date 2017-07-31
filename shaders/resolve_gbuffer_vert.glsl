attribute vec4 vPosition;
attribute vec4 vNormal;
attribute vec4 vTangent;
attribute vec2 vTexCoord;



#include "light.h"


uniform mat4 sysCameraProjectionMatrix;




varying vec2 UV;
varying vec3 viewRay;
//varying vec3 vt;
varying float v;

mat3 l_rot;

varying mat4 inverse_projection_matrix;

vec4 p;
void main()
{
	
	l_rot[0] = gl_LightSource[0].spotDirection;
	l_rot[1] = gl_LightSource[1].spotDirection;
	l_rot[2] = gl_LightSource[2].spotDirection;
	
	//l_rot = transpose(l_rot);
	
	p = vPosition;
	
	if(int(gl_LightSource[1].spotExponent) == LIGHT_POINT)
	{
		p = vec4(gl_NormalMatrix * (p.xyz * sysLightParams[0].sysLightRadius * 1.08) + gl_LightSource[0].position.xyz, 1.0);
		p = gl_ProjectionMatrix * p;
	}
	else if(int(gl_LightSource[1].spotExponent) == LIGHT_SPOT)
	{
		p.xy *= tan(((3.14159265*gl_LightSource[0].spotCutoff)/180.0)) * sysLightParams[0].sysLightRadius * 1.05;
		p.z *= sysLightParams[0].sysLightRadius;
		p = vec4(l_rot * p.xyz  + gl_LightSource[0].position.xyz, 1.0);
		p = gl_ProjectionMatrix * p;
	}

	//inverse_projection_matrix = inverse(sysCameraProjectionMatrix);
	inverse_projection_matrix = gl_ProjectionMatrixInverse;
	
    gl_Position = p;

}
