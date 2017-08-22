#include "brdf.h"
#include "light.h"


varying vec2 UV;
varying vec3 viewRay;


varying mat4 inverse_projection_matrix;

uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler1;
uniform sampler2D sysTextureSampler2;




#define BIAS 0.0000006
#define POINT_LIGHT_BIAS 0.0000006
#define SPOT_LIGHT_BIAS 0.0000009


//#define sysLightType int(gl_LightSource[1].spotCutoff)
//#define sysLightRadius gl_LightSource[0].diffuse.a
#define sysUseShadows gl_LightSource[3].spotExponent
#define sysProjectTexture gl_LightSource[4].spotExponent


#define RENDER_DRAWMODE_LIT 1
#define RENDER_DRAWMODE_FLAT 2
#define RENDER_DRAWMODE_WIREFRAME 3

//#define PI 3.14159265


uniform sampler2D sysDepthSampler;
//uniform sampler2D _2DshadowSampler;
//uniform samplerCube _CubeshadowSampler;

uniform sampler2D sys2DShadowSampler;
uniform samplerCube sys3DShadowSampler;


uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;
uniform float sysShadowMapSize;

uniform float zNear;
uniform float zFar;
uniform float sysLightZNear;
uniform float sysLightZFar;
uniform int sysRenderDrawMode;
//uniform float sysLightType;

//uniform float useShadows;
//uniform int lightCount;

//uniform mat4 cameraToWorldMatrix;
uniform mat4 sysCameraProjectionMatrix;	
//uniform mat4 worldToLightMatrix;
uniform mat4 sysCameraToLightProjectionMatrix;
//uniform mat4 lightProjectionMatrix;

/*vec2 kernel[9] = vec2[]
(
   vec2(0.95581, -0.18159), vec2(0.50147, -0.35807), vec2(0.69607, 0.35559),
   vec2(-0.003682, -0.5915), vec2(0.1593, 0.08975), vec2(-0.6503, 0.05818),
   vec2(0.11915, 0.78449), vec2(-0.34296, 0.51575), vec2(-0.6038, -0.41527)
);*/

float linearDepth(float depthSample)
{
    float zlin;
    depthSample=2.0*depthSample-1.0;	/* map it to -1.0 - 1.0 range */
    
    
    zlin=(2.0*zNear*zFar)/(zFar+zNear-depthSample*(zFar-zNear));
    return zlin;
}

void sample_2D_shadow_map(vec3 frag_pos, vec3 frag_norm, vec3 light_pos, out float factor)
{
	vec4 v = sysCameraToLightProjectionMatrix  *  vec4(frag_pos, 1.0);
	vec3 q = frag_pos-light_pos;
	//vec4 v = lightProjectionMatrix * lightModelViewMatrix * cameraToWorldMatrix * vec4(fragPos, 1.0);
	v/=v.w;
	
	int i;
	
	//sampler2D test = sampler2D(1);
	
	v.x=v.x*0.5 + 0.5;
	v.y=v.y*0.5 + 0.5;
	v.z=v.z*0.5 + 0.5;
	float delta_s = (1.0 / sysShadowMapSize);
	float delta_t = (1.0 / sysShadowMapSize);
	float shadow=1.0;
	int count = int(gl_LightSource[2].spotExponent);
	/* ad-hoc variable bias... */
	float n = ((1.0 - dot(normalize(q), frag_norm)) * SPOT_LIGHT_BIAS) + ((SPOT_LIGHT_BIAS * 75.0 * float(count)) / dot(q, q));
	
	float sub = float(1.0 / (1.0 + float(count) * 4.0));
	factor = 1.0;

	
	if(v.x<=1.0 && v.x>=0.0 && v.y<=1.0 && v.y>=0.0 && v.w >=0.0)
	{
		shadow=texture2D(sys2DShadowSampler, v.xy).x;
		
		/*if(v.z > shadow + n)
		{
			factor -= sub;
		}*/
		factor = float(v.z < shadow + n);
		
	/*	for(i = 1; i <= count; i++)
		{
			shadow=texture2D(sys2DShadowSampler, v.xy + vec2(delta_s, delta_t)).x;
		
			if(v.z > shadow + n)
			{
				factor -= sub;
			}
			
			shadow=texture2D(sys2DShadowSampler, v.xy + vec2(-delta_s, delta_t)).x;
		
			if(v.z > shadow + n)
			{
				factor -= sub;
			}
			
			shadow=texture2D(sys2DShadowSampler, v.xy + vec2(delta_s, -delta_t)).x;
		
			if(v.z > shadow + n)
			{
				factor -= sub;
			}
			
			shadow=texture2D(sys2DShadowSampler, v.xy + vec2(-delta_s, -delta_t)).x;
		
			if(v.z > shadow + n)
			{
				factor -= sub;
			}
		}*/
	}
}

void sample_3D_shadow_map(vec3 frag_pos, vec3 frag_norm, vec3 light_pos, out float factor)
{
	vec4 v = sysCameraToLightProjectionMatrix * vec4(frag_pos, 1.0);
	vec3 q = frag_pos-light_pos;
	vec3 d;
	vec3 tex_delta;
	vec3 tex_delta_arr[8];
	float delta_s = (1.0 / sysShadowMapSize);
	float delta_t = (1.0 / sysShadowMapSize);
	float shadow=1.0;
	float alpha;
	float fz;
	float sz;
	float s;
	float t;
	float m;
	int count = int(gl_LightSource[2].spotExponent);
	//int count = 0;
	
	/* ad-hoc variable bias... */
	float n = ((1.0 - dot(normalize(q), frag_norm)) * POINT_LIGHT_BIAS) + ((POINT_LIGHT_BIAS * 100.0 * (float(count + 1))) / dot(q, q));
	
	int i;
	//factor = 1.0;
	//float sub = float(1.0 / (1.0 + float(count) * 4.0));
	
	d=normalize(v.xyz);
	
	vec3 av = abs(v.xyz);
	fz = max(av.x, max(av.y, av.z));
	m = fz;
	fz = (sysLightZFar + sysLightZNear) / (sysLightZFar - sysLightZNear)  - (2.0 * sysLightZFar * sysLightZNear)  / ((sysLightZFar - sysLightZNear)*fz);
	fz = fz * 0.5 + 0.5;
	
	
	/*if(m == av.x)
	{
		tex_delta_arr[0].x = 0.0;
		tex_delta_arr[0].y = delta_s;
		tex_delta_arr[0].z = delta_t;
		
		tex_delta_arr[1].x = 0.0;
		tex_delta_arr[1].y = -delta_s;
		tex_delta_arr[1].z = delta_t;
		
		tex_delta_arr[2].x = 0.0;
		tex_delta_arr[2].y = -delta_s;
		tex_delta_arr[2].z = -delta_t;
		
		tex_delta_arr[3].x = 0.0;
		tex_delta_arr[3].y = delta_s;
		tex_delta_arr[3].z = -delta_t;
		
	}
	else if(m == av.y)
	{
		tex_delta_arr[0].x = delta_s;
		tex_delta_arr[0].y = 0.0;
		tex_delta_arr[0].z = delta_t;
		
		tex_delta_arr[1].x = -delta_s;
		tex_delta_arr[1].y = 0.0;
		tex_delta_arr[1].z = delta_t;
		
		tex_delta_arr[2].x = -delta_s;
		tex_delta_arr[2].y = 0.0;
		tex_delta_arr[2].z = -delta_t;
		
		tex_delta_arr[3].x = delta_s;
		tex_delta_arr[3].y = 0.0;
		tex_delta_arr[3].z = -delta_t;
	}
	else
	{
		tex_delta_arr[0].x = delta_s;
		tex_delta_arr[0].y = delta_t;
		tex_delta_arr[0].z = 0.0;
		
		tex_delta_arr[1].x = -delta_s;
		tex_delta_arr[1].y = delta_t;
		tex_delta_arr[1].z = 0.0;
		
		tex_delta_arr[2].x = -delta_s;
		tex_delta_arr[2].y = -delta_t;
		tex_delta_arr[2].z = 0.0;
		
		tex_delta_arr[3].x = delta_s;
		tex_delta_arr[3].y = -delta_t;
		tex_delta_arr[3].z = 0.0;
	}*/
	
	
	
	
	d.y = -d.y;
	shadow = textureCube(sys3DShadowSampler, d).r;
	
	/*if(fz > shadow + n)
	{
		factor -= sub;
	}*/
	
	//factor = float(fz < shadow + n);
	if(fz > shadow + n)
	{
		factor = 0.0;
	}
	else
	{
		factor = 1.0;
	}

	/*for(i = 1; i <= count; i++)
	{
		shadow = textureCube(sys3DShadowSampler, d + tex_delta_arr[0] * float(i)).r;
		if(fz > shadow + n)
		{
			factor -= sub;
		}
		
		shadow = textureCube(sys3DShadowSampler, d + tex_delta_arr[1] * float(i)).r;
		if(fz > shadow + n)
		{
			factor -= sub;
		}
		
		shadow = textureCube(sys3DShadowSampler, d + tex_delta_arr[2] * float(i)).r;
		if(fz > shadow + n)
		{
			factor -= sub;
		}
		
		shadow = textureCube(sys3DShadowSampler, d + tex_delta_arr[3] * float(i)).r;
		if(fz > shadow + n)
		{
			factor -= sub;
		}

	}*/

}

vec3 project_texture(sampler2D texture_sampler, vec3 frag_pos)
{
	vec4 v = sysCameraToLightProjectionMatrix * vec4(frag_pos, 1.0);
	v /= v.w;
	v.x = -v.x * 0.5 + 0.5;
	v.y = v.y * 0.5 + 0.5;

	return texture2D(texture_sampler, v.xy).rgb;
}

vec4 get_view_pos(vec4 h_pos, mat4 inverse_projection_matrix)
{
	vec4 temp = inverse_projection_matrix * h_pos;
	temp /= temp.w;
	return temp;
}

/*float attenuate_point(vec3 light_vec, float light_radius, float linear_fallof, float quadratic_fallof)
{
	//vec3 light_vec = frag_pos - light_pos;
	float l = length(light_vec);
	//light_vec /= l;
	//float a = 1.0 / l * l;
	float a = 1.0 / (l * linear_fallof + l * l * quadratic_fallof);
	//return a;
	return a * clamp((light_radius - l) / light_radius, 0.0, 1.0);
	
}

float attenuate_spot(vec3 light_vec, vec3 spot_direction, float light_distance, float spot_cos_cutoff, float spot_exponent, float linear_fallof, float quadratic_fallof)
{
	float l = length(light_vec);
	float a = 1.0 / (l * linear_fallof + l * l * quadratic_fallof);
	float cos = max(dot(light_vec / l, spot_direction), 0.0);
	
	a *= cos * smoothstep(spot_cos_cutoff, spot_cos_cutoff + float(spot_exponent) / 255.0, cos) * clamp((light_distance - l) / light_distance, 0.0, 1.0);
	return a;
}*/


void main()
{    
    
    vec2 uv = vec2(gl_FragCoord.x / sysRenderTargetWidth, gl_FragCoord.y / sysRenderTargetHeight);
    
    //float depth = texture2D(sysDepthSampler, uv).x;
    float depth = texelFetch(sysDepthSampler, ivec2(gl_FragCoord.xy), 0).r;
    vec4 d_texel;
    vec4 n_texel;
  
    vec4 h_pos = vec4(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 p_texel = get_view_pos(h_pos, gl_ProjectionMatrixInverse);

    vec4 f_color=vec4(0.0);
    vec3 light_pos;
    vec3 light_vec;
    //vec3 cam_vec;
    vec3 frag_pos;
    vec3 half_vec;
    vec3 view_vec;
    vec4 vcolor=vec4(0.5, 0.5, 0.5, 0.0);
    //vec4 lcolor = gl_LightSource[0].diffuse;
    //vec3 light_color = gl_LightSource[0].diffuse.rgb;
    //vec3 light_color = vec3(1.0);
    vec3 light_color = sysLightParams[sysLightIndex].sysLightColor.rgb;
    float diff;
    float intensity=1.0;
    float spec;
    float fallof;
    float cos;
	float f_discard=0.0; 
	float light_vec_len;
	//float dFactor;
	//float sFactor;
	float shadow;

	//float pixelRayMarchNoise=texture2D(textureSampler0, UV).x;
	
	
	
	//if(sysRenderDrawMode == RENDER_DRAWMODE_WIREFRAME || sysRenderDrawMode == RENDER_DRAWMODE_FLAT)
	//{
		//f_color = texture2D(sysTextureSampler0, uv);
	//	f_color = texelFetch(sysTextureSampler0, ivec2(gl_FragCoord.xy), 0);
	//}
	//else if(sysRenderDrawMode == RENDER_DRAWMODE_LIT)
	//{
		view_vec = -p_texel.xyz;

		//light_pos = gl_LightSource[0].position.xyz;
		light_pos = sysLightParams[sysLightIndex].sysLightPosition.xyz;
		light_vec = light_pos - p_texel.xyz;
		
		if(sysLightParams[sysLightIndex].sysLightType == LIGHT_POINT)
		{
			intensity = sysAttenuatePoint(light_vec, sysLightParams[sysLightIndex].sysLightRadius, gl_LightSource[0].linearAttenuation, gl_LightSource[0].quadraticAttenuation);
		}
		else if(sysLightParams[sysLightIndex].sysLightType == LIGHT_SPOT)
		{
			intensity = sysAttenuateSpot(light_vec, sysLightParams[sysLightIndex].sysLightForwardVector /*gl_LightSource[2].spotDirection*/, sysLightParams[sysLightIndex].sysLightRadius, sysLightParams[sysLightIndex].sysLightSpotCosCutoff, sysLightParams[sysLightIndex].sysLightSpotBlend, gl_LightSource[0].linearAttenuation, gl_LightSource[0].quadraticAttenuation);
			/*if(sysProjectTexture == 1)
			{
				light_color *= project_texture(sysTextureSampler2, p_texel.xyz);
			}*/
			
		}
		
		
		
			

		if(intensity <= 0.0)
		{
			discard;
		}
		
		
		/*else
		{
			if(sysLightType == LIGHT_SPOT)
			{
				if(dot(normalize(light_vec), gl_LightSource[0].spotDirection.xyz) < gl_LightSource[0].spotCosCutoff)
				{
					discard;
				}
			}
		}*/
		
		//n_texel = texture2D(sysTextureSampler1, uv);
		n_texel = texelFetch(sysTextureSampler1, ivec2(gl_FragCoord.xy), 0);
		
		if(length(n_texel.xyz) == 0.0)
		{
			discard;
		}
		
		n_texel.xyz = normalize(n_texel.xyz);
    	//d_texel = texture2D(sysTextureSampler0, uv);
    	d_texel = texelFetch(sysTextureSampler0, ivec2(gl_FragCoord.xy), 0);
    	
    	

			
		/*if(gl_LightSource[0].spotCutoff<90.0 && gl_LightSource[0].spotCutoff>0.0)
		{
			cos=max(dot(light_vec, gl_LightSource[0].spotDirection), 0.0);
			intensity*=pow(cos, 1.0);
			intensity*=smoothstep(gl_LightSource[0].spotCosCutoff, gl_LightSource[0].spotCosCutoff + 0.1, cos);
		}*/

		
		//phong(light_vec, cam_vec, n_texel.xyz, d_texel.w, 1.0, diff, spec);
		//cook_torrance(light_vec, cam_vec, n_texel.xyz, 0.1, 0.0, diff, spec);
		
		shadow = 1.0;
		
		/*if(sysUseShadows)
		{
			if(sysLightType == LIGHT_SPOT)
			{
				sample_2D_shadow_map(p_texel.xyz, n_texel.xyz, gl_LightSource[0].position.xyz, shadow);
			}
			else if(sysLightType == LIGHT_POINT)
			{
				sample_3D_shadow_map(p_texel.xyz, n_texel.xyz, gl_LightSource[0].position.xyz, shadow);
			}
		}*/
		
		
		//f_color = vec4(1.0) * ndf_ggx(n_texel.xyz, normalize(view_vec + light_vec), 0.5);
		//f_color = vec4(1.0) * g_smith(n_texel.xyz, normalize(view_vec), normalize(light_vec), 0.5);
		//f_color = vec4(f_schlick(n_texel.xyz, normalize(view_vec), vec3(0.0, 0.0 ,0.0), 0.0), 1.0);
		//f_color=((d_texel / PI) * diff + spec) * lcolor * intensity * shadow; 
		f_color = vec4(cook_torrance(normalize(light_vec), normalize(view_vec), n_texel.xyz, d_texel.xyz, d_texel.w, n_texel.w) * intensity * light_color * shadow, 1.0);
		//f_color = vec4(0.0);
		
		//f_color = vec4(1.0) * max(dot(normalize(light_vec + view_vec), n_texel.xyz), 0.0);

	//}
	
	vec4 c_color = vec4(0.0);
	unsigned int kl;
	
	
	ivec3 cluster_position = sysGetCluster(gl_FragCoord.x, gl_FragCoord.y, p_texel.z, 0.1);
	
	kl = texelFetch(sysClusterTexture, ivec3(cluster_position.xy, 0), 0).r;
	
	if(kl != 0)
	{
		c_color = vec4(0.05);
	}
	
	
	/*if(sysGetCluster(gl_FragCoord.x, gl_FragCoord.y, p_texel.z, 0.1).xy == ivec2(41, 0))
	{
		c_color = vec4(0.5);
	}
	else
	{
		c_color = vec4(0);
	}*/
	
	gl_FragColor = f_color + c_color;
}



















