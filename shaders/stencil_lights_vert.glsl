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
	
	//l_rot[0] = gl_LightSource[0].spotDirection;
	//l_rot[1] = gl_LightSource[1].spotDirection;
	//l_rot[2] = gl_LightSource[2].spotDirection;
	
	l_rot[0] = sysLightParams[sysLightIndex].sysLightRightVector.xyz;
	l_rot[1] = sysLightParams[sysLightIndex].sysLightUpVector.xyz;
	l_rot[2] = sysLightParams[sysLightIndex].sysLightForwardVector.xyz;
	
	//l_rot = transpose(l_rot);
	
	p = vPosition;
	
	//if(int(gl_LightSource[1].spotExponent) == LIGHT_POINT)
	if(sysLightParams[sysLightIndex].sysLightType == LIGHT_POINT)
	{
		p = vec4(gl_NormalMatrix * (p.xyz * sysLightParams[sysLightIndex].sysLightRadius * 1.08) + sysLightParams[sysLightIndex].sysLightPosition.xyz /*gl_LightSource[0].position.xyz*/, 1.0);
		p = gl_ProjectionMatrix * p;
	}
	//else if(int(gl_LightSource[1].spotExponent) == LIGHT_SPOT)
	else if(sysLightParams[sysLightIndex].sysLightType == LIGHT_SPOT)
	{
		p.xy *= tan(((3.14159265 * sysLightParams[sysLightIndex].sysLightSpotCutoff)/180.0)) * sysLightParams[sysLightIndex].sysLightRadius * 1.15;
		p.z *= sysLightParams[sysLightIndex].sysLightRadius;
		p = vec4(l_rot * p.xyz  + sysLightParams[sysLightIndex].sysLightPosition.xyz/*gl_LightSource[0].position.xyz*/, 1.0);
		p = gl_ProjectionMatrix * p;
	}

	//inverse_projection_matrix = inverse(sysCameraProjectionMatrix);
	inverse_projection_matrix = gl_ProjectionMatrixInverse;
	
    gl_Position = p;

}
