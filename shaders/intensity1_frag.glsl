uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler1;
uniform float sysExposure;

varying vec2 uv;

#define ADJUSTMENT 0.01

void main()
{
	//float target = 0.5 / 0.2;
	float target = 2.0;
	
	/* avarage intensity from last frame... */
	float c = texture2D(sysTextureSampler0, uv).r;
	/* exposure from last frame... */
	float l = texture2D(sysTextureSampler1, uv).r;
	
	float q = l + (target - (c * l)) * ADJUSTMENT; 
	
	gl_FragColor = vec4(q);
}
