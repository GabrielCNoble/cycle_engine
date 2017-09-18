#include "light.h"
#include "brdf.h"



varying vec2 UV;
varying vec3 viewRay;


varying mat4 inverse_projection_matrix;

uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler1;
uniform sampler2D sysTextureSampler2;



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

//uniform sampler2D sys2DShadowSampler;
//uniform samplerCube sys3DShadowSampler;


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
uniform mat4 sysCameraToWorldMatrix;
//uniform mat4 sysCameraProjectionMatrix;	
//uniform mat4 worldToLightMatrix;
//uniform mat4 sysCameraToLightProjectionMatrix;
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



/*vec3 project_texture(sampler2D texture_sampler, vec3 frag_pos)
{
	vec4 v = sysCameraToLightProjectionMatrix * vec4(frag_pos, 1.0);
	v /= v.w;
	v.x = -v.x * 0.5 + 0.5;
	v.y = v.y * 0.5 + 0.5;

	return texture2D(texture_sampler, v.xy).rgb;
}*/

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
    int i;
    int c;
    
    
    vec2 uv = vec2(gl_FragCoord.x / sysRenderTargetWidth, gl_FragCoord.y / sysRenderTargetHeight);
    
    //float depth = texture2D(sysDepthSampler, uv).x;
    float depth = texelFetch(sysDepthSampler, ivec2(gl_FragCoord.xy), 0).r;
    vec4 d_texel = texelFetch(sysTextureSampler0, ivec2(gl_FragCoord.xy), 0);
    vec4 n_texel = texelFetch(sysTextureSampler1, ivec2(gl_FragCoord.xy), 0);
  
    vec4 h_pos = vec4(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 p_texel = get_view_pos(h_pos, gl_ProjectionMatrixInverse);

    vec4 f_color=vec4(0.0);
    vec3 light_pos;
    vec3 light_vec;
    //vec3 cam_vec;
    vec3 world_frag_pos;
    vec3 frag_pos;
    vec3 half_vec;
    vec3 view_vec;
    vec4 vcolor=vec4(0.5, 0.5, 0.5, 0.0);
    //vec4 lcolor = gl_LightSource[0].diffuse;
    //vec3 light_color = gl_LightSource[0].diffuse.rgb;
    //vec3 light_color = vec3(1.0);
    //vec3 light_color = sysLightParams[sysLightIndex].sysLightColor.rgb;
    vec3 light_color;
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
	unsigned int kl;
	ivec3 cluster_position;

	//float pixelRayMarchNoise=texture2D(textureSampler0, UV).x;
	
	
	
	//if(sysRenderDrawMode == RENDER_DRAWMODE_WIREFRAME || sysRenderDrawMode == RENDER_DRAWMODE_FLAT)
	//{
		//f_color = texture2D(sysTextureSampler0, uv);
	//	f_color = texelFetch(sysTextureSampler0, ivec2(gl_FragCoord.xy), 0);
	//}
	//else if(sysRenderDrawMode == RENDER_DRAWMODE_LIT)
	//{
	
	
		view_vec = normalize(-p_texel.xyz);
		n_texel.xyz = normalize(n_texel.xyz);
		cluster_position = sysGetCluster(gl_FragCoord.x, gl_FragCoord.y, p_texel.z, 0.1);
		kl = texelFetch(sysClusterTexture, ivec3(cluster_position.xyz), 0).r;
		
		if(length(n_texel.xyz) == 0.0)
		{
			discard;
		}
		
		
		/*switch(cluster_position.z)
		{
			case 15:
				f_color += vec4(0.0625);
			case 14:
				f_color += vec4(0.0625);
			case 13:
				f_color += vec4(0.0625);
			case 12:
				f_color += vec4(0.0625);
			case 11:
				f_color += vec4(0.0625);
			case 10:
				f_color += vec4(0.0625);	
			case 9:
				f_color += vec4(0.0625);
			case 8:
				f_color += vec4(0.0625);
			case 7:
				f_color += vec4(0.0625);		
			case 6:
				f_color += vec4(0.0625);
			case 5:
				f_color += vec4(0.0625);
			case 4:
				f_color += vec4(0.0625);
			case 3:
				f_color += vec4(0.0625);	
			case 2:
				f_color += vec4(0.0625);
			case 1:
				f_color += vec4(0.0625);
			case 0:
				f_color += vec4(0.0625);
			
			
		}
		
		
		if(kl != 0)
		{
			f_color = vec4(0.0, 0.25, 0.0, 0.0);
		}
		/*else
		{
			f_color = vec4(0.25, 0.0, 0.0, 0.0);
		}*/
	
		world_frag_pos = vec3(sysCameraToWorldMatrix * p_texel);
		
		
		for(i = 0; i < MAX_ACTIVE_LIGHTS; i++)
		{
			
			if(kl & 1)
			{
				light_pos = sysLightParams[i].sysLightPosition.xyz;
				light_vec = light_pos - p_texel.xyz;
				light_color = sysLightParams[i].sysLightColor.rgb;
				
				shadow = 1.0;
				
				if(sysLightParams[i].sysLightType == LIGHT_POINT)
				{
					intensity = sysAttenuatePoint(light_vec, sysLightParams[i].sysLightRadius, sysLightParams[i].sysLightLinearFallof, sysLightParams[i].sysLightSquaredFallof);
					shadow = sysSample3DShadowMap(vec4(world_frag_pos, 1.0), p_texel, n_texel.xyz, i);
				}
				else if(sysLightParams[i].sysLightType == LIGHT_SPOT)
				{
					intensity = sysAttenuateSpot(light_vec, sysLightParams[i].sysLightForwardVector, sysLightParams[i].sysLightRadius, sysLightParams[i].sysLightSpotCosCutoff, sysLightParams[i].sysLightSpotBlend, sysLightParams[i].sysLightLinearFallof, sysLightParams[i].sysLightSquaredFallof);	
					shadow = sysSample2DShadowMap(vec4(world_frag_pos, 1.0), i);			
				}
				
				
				
				if(intensity > 0.0)
				{
					f_color += vec4(cook_torrance(normalize(light_vec), view_vec, n_texel.xyz, d_texel.xyz, d_texel.w, n_texel.w) * intensity * light_color * shadow, 1.0);
				}
				
			}
			
			kl >>= 1;
		}
		

		//light_pos = gl_LightSource[0].position.xyz;
		//light_pos = sysLightParams[sysLightIndex].sysLightPosition.xyz;
		//light_vec = light_pos - p_texel.xyz;
		
		//if(sysLightParams[sysLightIndex].sysLightType == LIGHT_POINT)
		//{
		//	intensity = sysAttenuatePoint(light_vec, sysLightParams[sysLightIndex].sysLightRadius, gl_LightSource[0].linearAttenuation, gl_LightSource[0].quadraticAttenuation);
	//	}
		//else if(sysLightParams[sysLightIndex].sysLightType == LIGHT_SPOT)
	//	{
	//		intensity = sysAttenuateSpot(light_vec, sysLightParams[sysLightIndex].sysLightForwardVector /*gl_LightSource[2].spotDirection*/, sysLightParams[sysLightIndex].sysLightRadius, sysLightParams[sysLightIndex].sysLightSpotCosCutoff, sysLightParams[sysLightIndex].sysLightSpotBlend, gl_LightSource[0].linearAttenuation, gl_LightSource[0].quadraticAttenuation);
			/*if(sysProjectTexture == 1)
			{
				light_color *= project_texture(sysTextureSampler2, p_texel.xyz);
			}*/
			
	//	}
		
		
		
			

	//	if(intensity <= 0.0)
	//	{
		//	discard;
	//	}
		
		
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
		//n_texel = texelFetch(sysTextureSampler1, ivec2(gl_FragCoord.xy), 0);
		
	//	if(length(n_texel.xyz) == 0.0)
		//{
		//	discard;
		//}
		
		//n_texel.xyz = normalize(n_texel.xyz);
    	//d_texel = texture2D(sysTextureSampler0, uv);
    	
    	
    	

			
		/*if(gl_LightSource[0].spotCutoff<90.0 && gl_LightSource[0].spotCutoff>0.0)
		{
			cos=max(dot(light_vec, gl_LightSource[0].spotDirection), 0.0);
			intensity*=pow(cos, 1.0);
			intensity*=smoothstep(gl_LightSource[0].spotCosCutoff, gl_LightSource[0].spotCosCutoff + 0.1, cos);
		}*/

		
		//phong(light_vec, cam_vec, n_texel.xyz, d_texel.w, 1.0, diff, spec);
		//cook_torrance(light_vec, cam_vec, n_texel.xyz, 0.1, 0.0, diff, spec);
		
		//shadow = 1.0;
		
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
		//f_color = vec4(cook_torrance(normalize(light_vec), normalize(view_vec), n_texel.xyz, d_texel.xyz, d_texel.w, n_texel.w) * intensity * light_color * shadow, 1.0);
		//f_color = vec4(0.0);
		
		//f_color = vec4(1.0) * max(dot(normalize(light_vec + view_vec), n_texel.xyz), 0.0);

	//}
	
	//vec4 c_color = vec4(0.0);
	
	
	/*switch(cluster_position.z)
	{
		case 0:
			c_color.g = 0.05;
		break;
		
		case 1:
			c_color.g = 0.1;
		break;
		
		case 2:
			c_color.g = 0.15;
		break;
		
		case 3:
			c_color.g = 0.20;
		break;
		
		case 4:
			c_color.g = 0.25;
		break;
		
		case 5:
			c_color.g = 0.30;
		break;
		
		case 6:
			c_color.g = 0.35;
		break;
		
		case 7:
			c_color.g = 0.40;
		break;
		
		case 8:
			c_color.g = 0.45;
		break;
		
		case 9:
			c_color.g = 0.5;
		break;
		
		case 10:
			c_color.g = 0.55;
		break;
		
		case 11:
			c_color.g = 0.6;
		break;
		
		case 12:
			c_color.g = 0.65;
		break;
		
		case 13:
			c_color.g = 0.7;
		break;
		
		case 14:
			c_color.g = 0.75;
		break;
		
		case 15:
			c_color.g = 0.8;
		break;
		
		
		
	}*/
	
	/*if(kl & 1)
	{
		c_color.r = 0.5;
	}
	if(kl & 2)
	{
		c_color.g = 0.5;
	}
	if(kl & 4)
	{
		c_color.b = 0.5;
	}*/
	
	
	/*if(sysGetCluster(gl_FragCoord.x, gl_FragCoord.y, p_texel.z, 0.1).xy == ivec2(41, 0))
	{
		c_color = vec4(0.5);
	}
	else
	{
		c_color = vec4(0);
	}*/
	
	gl_FragColor = f_color;
}



















