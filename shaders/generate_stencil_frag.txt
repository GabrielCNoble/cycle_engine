varying vec2 UV;
uniform sampler2D textureSampler0;


uniform float l_radius[8];

void main()
{
	/* TODO: try to use the fragment's normal to determine if the fragment is facing the light. If not, discard that fragment as well... */
	vec4 d_texel=texture2D(textureSampler0, UV);
	vec3 lightvec=gl_LightSource[0].position.xyz-d_texel.xyz;
	
	
	if(dot(lightvec, lightvec)>l_radius[0]*l_radius[0]) discard;
	gl_FragColor=vec4(1.0);
		
}
