#include "material.h"



uniform sampler2D sysTextureSampler0;
varying vec3 bi_tangent;
varying vec3 normal;
varying vec2 UV;





void main()
{
    vec4 diffv;
    
    if(sysMaterialParams.sysMaterialFlags & MATERIAL_DiffuseTexture)
	{
		diffv = vec4(texture2D(sysTextureSampler0, UV));
	}
	else
	{
		//diffv = gl_FrontMaterial.diffuse;
		diffv = sysMaterialParams.sysMaterialBaseColor;
	}
    
    gl_FragData[0] = diffv;
    //gl_FragData[0] = vec4(normal.x, normal.y, normal.z, 0.0);
    //gl_FragData[1] = vec4(0);

}
