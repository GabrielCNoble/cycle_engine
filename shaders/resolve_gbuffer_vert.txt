
#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define LIGHT_DIRECTIONAL 4


#include "default_attribs.h"


#define sysLightType int(gl_LightSource[1].spotCutoff)
#define sysLightRadius gl_LightSource[0].diffuse.a

#define DegToRad(x) ((3.14159265*x)/180.0)


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
	
	l_rot[0] = gl_LightSource[2].spotDirection;
	l_rot[1] = gl_LightSource[1].spotDirection;
	l_rot[2] = gl_LightSource[0].spotDirection;
	
	l_rot = transpose(l_rot);
	
	p = vPosition;
	
	if(int(gl_LightSource[1].spotExponent) == LIGHT_POINT)
	{
		p = vec4((p.xyz * gl_LightSource[0].diffuse.a * 1.08) + gl_LightSource[0].position.xyz, 1.0);
		p = gl_ProjectionMatrix * p;
	}
	else if(int(gl_LightSource[1].spotExponent) == LIGHT_SPOT)
	{
		p.xy *= tan(((3.14159265*gl_LightSource[0].spotCutoff)/180.0)) * gl_LightSource[0].diffuse.a * 1.05;
		p.z *= gl_LightSource[0].diffuse.a;
		p = vec4(p.xyz * l_rot + gl_LightSource[0].position.xyz, 1.0);
		p = gl_ProjectionMatrix * p;
	}

	//inverse_projection_matrix = inverse(sysCameraProjectionMatrix);
	inverse_projection_matrix = gl_ProjectionMatrixInverse;
	
    gl_Position = p;

}
