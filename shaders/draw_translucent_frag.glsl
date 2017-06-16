varying vec2 uv;
varying vec3 normal;
varying vec3 tangent;
varying vec3 position;

uniform sampler2D sysDepthSampler;
uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler1;
uniform int sysLightCount;

uniform int sysFlagNormalTexture;
uniform int sysFlagDiffuseTexture;


float w(float z, float a)
{
	return pow(a, 0.1) * clamp(0.3 / (1e-5 + pow(z / 1000, 4.0)), 1e-2, 3e3);
}

void phong(vec3 light_vec, vec3 view_vec, vec3 normal, float spec_power, float spec_mask, out float d_factor, out float s_factor)
{
	d_factor = max(0.0, dot(light_vec, normal));
	s_factor = max(0.0, dot(normalize(view_vec + light_vec), normal));	
	s_factor = pow(s_factor,  spec_power) * spec_mask;
}

void main()
{
	
	float depth = texelFetch(sysDepthSampler, ivec2(gl_FragCoord.xy), 0).r;
	float d;
	float s;
	float l;
	float fallof;
	float f_discard;
	float att;
	vec3 n;
	vec3 rn;
	vec3 tn;
	vec3 btangent;
	vec3 view_vec;
	vec3 light_vec;
	vec3 r;
	int i;
	vec4 diffuse;
	vec4 color = vec4(0);
	
	mat3 tbn;

	
	
	
	
	
	if(gl_FragCoord.z < depth)
	{
		//if(sysLightCount > 0)
		//{
		
		if(sysFlagDiffuseTexture > 0)
		{
			diffuse = texture2D(sysTextureSampler0, uv);
		}
		else
		{
			diffuse = gl_FrontMaterial.diffuse;
		}
		
		if(sysFlagNormalTexture > 0)
		{
			
			btangent = cross(tangent, normal);
			
			tbn[0] = tangent.xyz;
    		tbn[1] = btangent.xyz;
    		tbn[2] = normal.xyz;
			
			tn =  normalize(texture2D(sysTextureSampler1, uv).xyz * 2.0 - 1.0);
			n = tbn * tn;
		}
		else
		{
			n = normalize(normal);
		}
		
		r = refract(normalize(view_vec), normalize(n), 0.0);
		att = 2.0 - length(r.xy);
		
		for(i = 0; i < sysLightCount; i++)
		{
			light_vec = gl_LightSource[i].position.xyz - position;
			l = max(length(light_vec), 1e-8);
			light_vec /= l;
			view_vec = -position;
			att = 1.0;
			
			phong(light_vec, view_vec, n, float(gl_FrontMaterial.shininess), 1.0, d, s);
				
			fallof = 1.0 / (gl_LightSource[i].linearAttenuation * l + gl_LightSource[i].quadraticAttenuation * l * l);
			f_discard = 1.0 - clamp((gl_LightSource[i].diffuse.a - fallof) / gl_LightSource[i].diffuse.a, 0.0, 1.0);
				
			color += (diffuse.rgba * d * gl_LightSource[i].diffuse.rgba + gl_LightSource[i].diffuse.rgba * s) * fallof * f_discard;
		}
		
		
		
		
		gl_FragData[0] = vec4(color.rgb * diffuse.a, diffuse.a);
		gl_FragData[1] = vec4(diffuse.a);
		gl_FragData[2] = vec4(r, 1.0);
		//}
		/*else
		{
			gl_FragData[0] = vec4(0.0);
			gl_FragData[1] = vec4(color);
		}*/
		
	}
	else
	{
		discard;
		//gl_FragData[0] = vec4(0.0);
		//gl_FragData[1] = vec4(0.0);
	}
	
	
	
	
	//vec4 bg = texelFetch(sysTextureSampler0, ivec2(gl_FragCoord.xy), 0);
	//vec4 color = gl_FrontMaterial.diffuse;
	
	//gl_FragColor = vec4(color.rgb - color.a * bg.rgb, 1.0);
	//gl_FragColor = vec4(bg.rgb * (1.0 - color.a), 1.0) + vec4(color.rgb / clamp(bg.a, 1e-4, 5e4), 1.0) * (1.0 - (1.0 - color.a));
	//gl_FragColor = vec4(1.0);
	
}
