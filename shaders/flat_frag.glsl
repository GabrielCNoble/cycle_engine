uniform sampler2D sysTextureSampler0;
varying vec3 bi_tangent;
varying vec3 normal;
varying vec2 UV;

uniform int mFlagShadeless;
uniform int mFlagDiffuseTexture;
uniform int mFlagNormalTexture;
uniform int mFlagHeightTexture;
uniform int mFlagSpecularTexture;

#define MATERIAL_Shadeless 1
#define MATERIAL_DiffuseTexture 4
#define MATERIAL_NormalTexture 8
#define MATERIAL_HeightTexture 16
#define MATERIAL_SpecularTexture 32



void main()
{
    vec4 diffv;
    
    if(mFlagDiffuseTexture == MATERIAL_DiffuseTexture)
	{
		diffv = vec4(texture2D(sysTextureSampler0, UV));
	}
	else
	{
		diffv = gl_FrontMaterial.diffuse;
	}
    
    gl_FragData[0] = diffv;
    //gl_FragData[0] = vec4(normal.x, normal.y, normal.z, 0.0);
    //gl_FragData[1] = vec4(0);

}
