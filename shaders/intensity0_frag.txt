uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler1;

varying vec2 uv;

void main()
{
	vec4 c; 
	float i; 
	
	c = texture2D(sysTextureSampler0, uv);
	i = max(c.r, max(c.g, c.b));
	
	gl_FragColor = vec4(i);
}
