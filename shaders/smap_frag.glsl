
varying vec2 uv;

uniform float renderTargetWidth;
uniform float renderTargetHeight;

uniform sampler2D textureSampler0;
uniform sampler2D depthSampler;
uniform int mFlagDiffuseTexture;

#define MATERIAL_DiffuseTexture 4
#define MATERIAL_NormalTexture 8
#define MATERIAL_SpecularTexture 16
#define MATERIAL_HeightTexture 32

varying vec4 position;
varying vec3 normal;
#define BIAS 0.00001
void main()
{
	//float depth = texture2D(depthSampler, uv).r;
	float alpha;
	
	/*if(mFlagDiffuseTexture == MATERIAL_DiffuseTexture)
	{
		alpha = texture2D(textureSampler0, uv).a;
		if(alpha < 0.5)
		{
			discard;
		}
		
		gl_FragDepth = gl_FragCoord.z + BIAS;
	}
	else
	{*/
		//gl_FragDepth = gl_FragCoord.z + BIAS;
	//}
	
	
	
}







