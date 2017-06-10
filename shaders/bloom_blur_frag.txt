uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler1;
varying vec2 uv;

uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;
uniform float sysBloomRadius;

void main()
{
	vec4 color = texture2D(sysTextureSampler0, uv);
	int i;
	int c = int(sysBloomRadius);
	float multiplier = 1.0;
	if(sysRenderTargetWidth == 0)	/* vertical blur */
	{
		float dy = 1.0 / sysRenderTargetHeight;
		for(i = 0; i < c; i++)
		{
			color += texture2D(sysTextureSampler0, uv + vec2(0.0, dy * i));
			color += texture2D(sysTextureSampler0, uv + vec2(0.0, -dy * i));
		}
	}
	else						/* horizontal blur */
	{
		float dx = 1.0 / sysRenderTargetWidth;
		for(i = 0; i < c; i++)
		{
			color += texture2D(sysTextureSampler0, uv + vec2(-dx * i, 0.0));
			color += texture2D(sysTextureSampler0, uv + vec2(dx * i, 0.0));
		}
	}
	

	
	
	
	
	gl_FragData[0] = (color / (c * 2 + 1)) * multiplier;
}

