varying vec2 uv;
varying vec3 normal;

uniform sampler2D sysTextureSampler0;

void main()
{
	vec4 bg = texelFetch(sysTextureSampler0, ivec2(gl_FragCoord.xy), 0);
	vec4 color = gl_FrontMaterial.diffuse;
	
	//gl_FragColor = bg * 0.5;
	gl_FragColor = vec4(bg.rgb * (1.0 - color.a), 1.0) + vec4(color.rgb / clamp(bg.a, 1e-4, 5e4), 1.0) * (1.0 - (1.0 - color.a));
	//gl_FragColor = vec4(1.0);
	
}
