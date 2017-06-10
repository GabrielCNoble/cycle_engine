//uniform sampler2D sys2DShadowSampler;
//uniform samplerCube sysCubeShadowSampler;
uniform sampler2D sysDepthSampler;
uniform sampler2D sysShadowSampler;

uniform mat4 sysLightModelViewProjectionMatrix;
uniform mat4 sysCameraProjectionMatrix;
uniform mat4 sysCameraToLightProjectionMatrix;
uniform mat4 sysCameraToWorldMatrix;
uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;
uniform float sysLightZNear;
uniform float sysLightZFar;

/* the tex coord mapping value is being sent through gl_LightSource[0].spotExponent */
/* sysLightZNear and sysLightZFar will be sent through gl_LightSource[0].xy */

varying vec2 uv;


void sample_2D_shadow(sampler2D shadow_map_sampler, vec3 frag_pos, out float dFactor, out float sFactor)
{
	vec4 v = sysCameraToLightProjectionMatrix *  vec4(frag_pos, 1.0);
	v/=v.w;
	
	int i;
	
	v.x=v.x*0.5 + 0.5;
	v.y=v.y*0.5 + 0.5;
	v.z=v.z*0.5 + 0.5;
	
	float shadow=1.0;
	float alpha;
	dFactor=1.0;
	sFactor=1.0;
	float t;
	
	if(v.x<=1.0 && v.x>=0.0 && v.y<=1.0 && v.y>=0.0 && v.w >=0.0)
	{
		v.x = gl_LightSource[0].diffuse.x + gl_LightSource[0].diffuse.z * v.x;
		v.y = gl_LightSource[0].diffuse.y + gl_LightSource[0].diffuse.w * v.y;
		//v.y = tex_coord_mapping / v.y;
		shadow=texture2D(shadow_map_sampler, v.xy).x;
		
		if(v.z>shadow)
		{
			dFactor=0.0;
			sFactor=0.0;
		}
	}
}

void sample_cube_shadow(sampler2D cube_shadow_sampler, vec3 frag_pos, vec3 frag_norm, vec3 light_nos, vec3 light_FVector, out float dFactor, out float sFactor, float tex_coord_mapping)
{
	vec4 v = sysCameraToLightProjectionMatrix * vec4(frag_pos, 1.0);
	vec3 q = frag_pos-light_nos;
	vec3 d;
	float shadow=1.0;
	float alpha;
	float fz;
	float sz;
	float sample_count = 10.0;
	float samples = 0.0;
	dFactor=1.0;
	sFactor=1.0;
	float step = 1.0 / sample_count;
	float s;
	float t;
	float lznear = gl_LightSource[0].diffuse.x;
	float lzfar = gl_LightSource[0].diffuse.y;
	
	d=normalize(v.xyz);
	
	vec3 av = abs(v.xyz);
	fz = max(av.x, max(av.y, av.z));
	fz = (lzfar + lznear) / (lzfar - lznear)  - (2.0 * lzfar * lznear)  / ((lzfar - lznear) * fz);
	fz = fz * 0.5 + 0.5;

	
	/* +x or -x */
	if(abs(dot(d, vec3(1.0, 0.0, 0.0))) > 0.707)
	{
		s = d.y * 0.5 + 0.5;
		t = d.z * 0.5 + 0.5;
	}
	/* +y or -y */
	else if(abs(dot(d, vec3(0.0, 1.0, 0.0))) > 0.707)
	{
		s = d.x * 0.5 + 0.5;
		t = d.z * 0.5 + 0.5;
	}
	/* +z or -z */
	else
	{
		s = d.x * 0.5 + 0.5;
		t = d.y * 0.5 + 0.5;
	}
	
	#define BIAS 0.0000005
	
	s = tex_coord_mapping / s;		/* map from 0..1 to 0..sysCoordMapping */
	t = tex_coord_mapping / t;

	
	shadow = texture2D(cube_shadow_sampler, vec2(s, t)).r;

	if(fz > shadow + (1.0 - dot(normalize(q), frag_norm)) * BIAS)
	{
		dFactor = 0.0;
		sFactor = 0.0;
	}
}


vec4 get_view_pos(vec4 h_pos, mat4 inverse_projection_matrix)
{
	vec4 temp = inverse_projection_matrix * h_pos;
	temp /= temp.w;
	return temp;
}






void main()
{
	float depth = texture2D(sysDepthSampler, uv);
	vec4 h_pos = vec4((gl_FragCoord.x/sysRenderTargetWidth) * 2.0 - 1.0, (gl_FragCoord.y/sysRenderTargetHeight) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 p_texel = get_view_pos(h_pos, inverse(sysCameraProjectionMatrix));
	
	float d_factor = 1.0;
	float s_factor = 1.0;
	
	sample_2D_shadow(sysShadowSampler, p_texel.xyz, d_factor, s_factor);
	
	//gl_FragColor = vec4(d_factor);
	gl_FragColor = vec4(1.0) * d_factor;
	
	/*if(p_texel.z < -20.0)
	{
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
	}*/
	
	
	
	
	
}










