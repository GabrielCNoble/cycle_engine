uniform sampler2D sysTextureSampler0;
uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;
varying vec2 UV;


float gweights[] = {0.146768, 0.092651, 0.023226, 0.002291};

void main()
{
    float dx;
    float dy;
    float factor = 2.8;
    
    vec4 color;

    color = texture2D(sysTextureSampler0, UV) * gweights[0]*factor;
    
    if(sysRenderTargetWidth != 0)
	{
		dx = 1.0 / sysRenderTargetWidth;
		color += texture2D(sysTextureSampler0, UV + vec2(dx * 0.5, 0.0)) * gweights[1]*factor;
    	color += texture2D(sysTextureSampler0, UV - vec2(dx * 0.5, 0.0)) * gweights[1]*factor;
    	color += texture2D(sysTextureSampler0, UV + vec2(dx * 1.5, 0.0)) * gweights[2]*factor;
    	color += texture2D(sysTextureSampler0, UV - vec2(dx * 1.5, 0.0)) * gweights[2]*factor;
    	color += texture2D(sysTextureSampler0, UV + vec2(dx * 2.5, 0.0)) * gweights[3]*factor;
    	color += texture2D(sysTextureSampler0, UV - vec2(dx * 2.5, 0.0)) * gweights[3]*factor;
    	//color += texture2D(textureSampler0, UV + vec2(dx * 2.5, 0.0));
    	//color += texture2D(textureSampler0, UV - vec2(dx * 2.5, 0.0));
    	//color += texture2D(textureSampler0, UV + vec2(dx * 3.5, 0.0));
    	//color += texture2D(textureSampler0, UV - vec2(dx * 3.5, 0.0));
	}
	else
	{
		dy = 1.0 / sysRenderTargetHeight;
		color += texture2D(sysTextureSampler0, UV + vec2(0.0, dy * 0.5)) * gweights[1]*factor;
    	color += texture2D(sysTextureSampler0, UV - vec2(0.0, dy * 0.5)) * gweights[1]*factor;
    	color += texture2D(sysTextureSampler0, UV + vec2(0.0, dy * 1.5)) * gweights[2]*factor;
    	color += texture2D(sysTextureSampler0, UV - vec2(0.0, dy * 1.5)) * gweights[2]*factor;
    	color += texture2D(sysTextureSampler0, UV + vec2(0.0, dy * 2.5)) * gweights[3]*factor;
    	color += texture2D(sysTextureSampler0, UV - vec2(0.0, dy * 2.5)) * gweights[3]*factor;
    	//color += texture2D(textureSampler0, UV + vec2(0.0, dy * 2.5));
    	//color += texture2D(textureSampler0, UV - vec2(0.0, dy * 2.5));
    	//color += texture2D(textureSampler0, UV + vec2(0.0, dy * 3.5));
    	//color += texture2D(textureSampler0, UV - vec2(0.0, dy * 3.5));
	}
    
    
    

    //color /= 5;
    gl_FragColor = color;

}
