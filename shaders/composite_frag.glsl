varying vec2 UV;

uniform sampler2D sysTextureSampler0;
//uniform sampler2D sysTextureSampler1;
//uniform sampler2D sysTextureSampler2;
uniform float sysExposure;

//uniform float sysRenderTargetWidth;
//uniform float sysRenderTargetHeight;


void main()
{
	//float e = texture2D(sysTextureSampler1, UV).r;
	
	gl_FragColor = texture2D(sysTextureSampler0, UV) * sysExposure;	
	//gl_FragColor = color * sysExposure;
}
