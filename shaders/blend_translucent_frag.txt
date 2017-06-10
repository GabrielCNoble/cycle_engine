uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler1;
uniform sampler2D sysTextureSampler2;
uniform sampler2D sysTextureSampler3;

varying vec2 uv;



void main()
{
	vec3 c = vec3(0);
	vec3 normal = texelFetch(sysTextureSampler2, ivec2(gl_FragCoord.xy), 0).xyz;
	vec4 accum = texelFetch(sysTextureSampler0, ivec2(gl_FragCoord.xy), 0);
	
	float r = texelFetch(sysTextureSampler1, ivec2(gl_FragCoord.xy), 0).r;
	//if(length(normal) > 0.0)
	//{
	c = texelFetch(sysTextureSampler3, ivec2(gl_FragCoord.xy + normal.xy * 20.0), 0).xyz;
		//gl_FragData[0] = vec4(c / clamp(accum.a, 1e-4, 5e4), r);
		//gl_FragData[0] = vec4(c, 1.0);
	//}
	//else
	//{
	gl_FragData[0] = vec4((accum.rgb + c) / clamp(accum.a, 1e-4, 5e4), r);
	//}
	
	
	
}
