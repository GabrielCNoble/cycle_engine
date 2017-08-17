//varying vec2 UV;
//varying vec3 viewRay;

#include "light.h"


uniform sampler2D sysTextureSampler1;
uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler2;
//uniform sampler2D sysDepthSampler;
//uniform samplerCube sysTextureSamplerCube0;
uniform sampler2D sys2DShadowSampler;
uniform samplerCube sys3DShadowSampler;

//uniform mat4 cameraToWorldMatrix;
//uniform mat4 worldToLightMatrix;
//uniform mat4 lightModelViewProjectionMatrix;
uniform mat4 sysCameraToLightProjectionMatrix;

uniform float sysZNear;
uniform float sysZFar;
uniform float sysLightZNear;
uniform float sysLightZFar;

uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;

//varying mat4 inverse_projection_matrix;
//varying mat4 inverse_modelview_matrix;
varying float z;

float linear_depth(float depth_sample)
{
    float zlin;
    depth_sample = 2.0 * depth_sample - 1.0;
    zlin=2.0*sysZNear*sysZFar/(sysZFar+sysZNear-depth_sample*(sysZFar-sysZNear));
    return zlin;
}

void intersect_cone(vec3 cone_vertice, vec3 cone_direction, float cone_length, float cone_angle, vec3 ray_origin, vec3 ray_direction, out float depth0, out float depth1)
{
	mat3 M;
	float ctetha=cos(cone_angle);
	float sqrdctetha=ctetha*ctetha;
	vec3 rorigin=ray_origin;
	
	M[0][0]=cone_direction.x*cone_direction.x - sqrdctetha;
	M[0][1]=cone_direction.x*cone_direction.y;
	M[0][2]=cone_direction.x*cone_direction.z;
	
	M[1][0]=cone_direction.x*cone_direction.y;
	M[1][1]=cone_direction.y*cone_direction.y - sqrdctetha;
	M[1][2]=cone_direction.y*cone_direction.z;
	
	M[2][0]=cone_direction.x*cone_direction.z;
	M[2][1]=cone_direction.z*cone_direction.y;
	M[2][2]=cone_direction.z*cone_direction.z - sqrdctetha;
	
	
	
	vec3 delta=cone_vertice;
	vec3 cap_normal=cone_direction;
	vec3 cap_point=cone_vertice-cone_direction*cone_length;
	
	vec3 i0;
	vec3 i1;
	
	float t;
	float q=dot(cap_normal, cap_point);
	
	vec3 Mtdelta=M*delta;
	float c0=dot(delta, Mtdelta);
	float c1=dot(ray_direction, Mtdelta);
	float c2=dot(ray_direction, M*ray_direction);
	
	float i=c1*c1 - c0*c2;
	
	if(i<0.0)
	{
		depth0=0.0;
		depth1=0.0;
	}
	
	float d=sqrt(i);
	float t0=(-c1-d)/c2;
	float t1=(-c1+d)/c2;
	
	depth0=-t0;
	depth1=-t1;	
	
	
	i0=rorigin+ray_direction*depth0;
	i1=rorigin+ray_direction*depth1;
	
	vec3 v0=(i0-cone_vertice);
	vec3 v1=(i1-cone_vertice);
	
	float sqrdlen=cone_length*cone_length;
	
	if(dot(-cone_direction, v1)<0.0 || dot(v1, v1)>sqrdlen)
	{
		depth1=0.0;
	}
	if(dot(-cone_direction, v0)<0.0 || dot(v0, v0)>sqrdlen)
	{
		depth0=0.0;
	}
	
	if(depth1<=0.0)
	{
		t=q/dot(cap_normal, ray_direction);
		depth1=t;
	}
}

void intersect_sphere(vec3 center, vec3 view_point, vec3 view_direction, out float depth0, out float depth1, float radius)
{

    vec3 Q = center - view_point;
    float c = length(Q);
    float v = dot(Q,view_direction);
    float d = (radius*radius) - ( c*c - v*v );

    if(d<0.0f)
    {
        depth0 = depth1 = 0.0;
    }
    else
    {
        d = sqrt(d);
        depth0 = v-d;
        depth1 = v+d;
    }
}


float shade_point_point(vec3 light_pos, float light_radius, vec3 position)
{
	vec3 lvec=light_pos-position;
	float lvec_len=dot(lvec, lvec);
	//return clamp(((light_radius*light_radius)-lvec_len)/lvec_len, 0.0f, 1.0f);
	return (((light_radius*light_radius)-lvec_len)/(lvec_len*lvec_len)) * 0.1;
}

float shade_point_spot(vec3 light_pos, vec3 light_direction, float light_radius, vec3 position)
{
	vec3 lvec=light_pos-position;
	/*float lvec_len=dot(lvec, lvec);*/
	float fallof=length(lvec);
	vec3 nlvec=lvec/fallof;
	float q=(light_radius-fallof)/light_radius;
	//return 0.01;
	//return light_radius*dot(nlvec, light_direction)/(lvec_len*0.005);
	return (dot(nlvec, light_direction)*q)/(fallof*fallof*0.5);
}

vec4 get_view_pos(vec4 h_pos, mat4 inverse_projection_matrix)
{
	vec4 temp = inverse_projection_matrix * h_pos;
	temp /= temp.w;
	return temp;
}

float sample_2d_shadow_map(vec3 frag_pos)
{
	vec4 v = sysCameraToLightProjectionMatrix  *  vec4(frag_pos, 1.0);
	v/=v.w;
	
	v.x=v.x*0.5 + 0.5;
	v.y=v.y*0.5 + 0.5;
	v.z=v.z*0.5 + 0.5;

	if(v.x<=1.0 && v.x>=0.0 && v.y<=1.0 && v.y>=0.0 && v.w >=0.0)
	{
		float shadow = texture2D(sys2DShadowSampler, v.xy).x;
		
		return float((v.z < shadow));
	}
}

float sample_3d_shadow_map(vec3 frag_pos)
{
	vec4 v = sysCameraToLightProjectionMatrix * vec4(frag_pos, 1.0);	
	float shadow;
	
	vec3 d=normalize(v.xyz);
	
	vec3 av = abs(v.xyz);
	float fz = max(av.x, max(av.y, av.z));
	fz = (sysLightZFar + sysLightZNear) / (sysLightZFar - sysLightZNear)  - (2.0 * sysLightZFar * sysLightZNear)  / ((sysLightZFar - sysLightZNear)*fz);
	fz = fz * 0.5 + 0.5;	
	
	d.y = -d.y;
	shadow = textureCube(sys3DShadowSampler, d).r;
	
	return float((fz < shadow));
	/*if(fz > shadow)
	{
		return 0.0;
	}
	return 1.0;*/
}

vec3 project_texture(sampler2D texture_sampler, vec3 frag_pos)
{
	vec4 v = sysCameraToLightProjectionMatrix * vec4(frag_pos, 1.0);
	v /= v.w;
	v.x = -v.x * 0.5 + 0.5;
	v.y = v.y * 0.5 + 0.5;

	return texture2D(texture_sampler, v.xy).rgb;
}



void main()
{

    vec3 view_vec;
    vec3 eye_view_vec;
    vec3 camera_pos;
    vec3 light_pos;
    
    
    //vec3 vcolor = gl_LightSource[0].diffuse.rgb;
    vec3 vcolor = sysLightParams[sysLightIndexes[0]].sysLightColor.rgb;
	vec2 uv = vec2(gl_FragCoord.x / sysRenderTargetWidth, gl_FragCoord.y / sysRenderTargetHeight);
	vec3 view_ray;
	vec3 p;
	vec4 t;
	vec3 cur_pos;
	vec3 eye_cur_pos;
	float depth0;
	float depth1;
	float start_depth;
	float end_depth;
	float step_len_world;
	//float scattering = gl_LightSource[2].linearAttenuation;
	float scattering = 0.15;
	float accum=0.0;
	float shadow;
	float dFactor;
	float depth = texture2D(sysTextureSampler2, uv).x;
	//float depth=linear_depth(texture2D(sysDepthSampler, uv).x);	/* 0...1 to 0...far */
	//float depth = texture2D(sysDepthSampler, uv).x;
	//float depth;
	float noise=texture2D(sysTextureSampler0, vec2(uv.x, uv.y)*vec2(192.0, 108.0)).x * 10.0;

	
	float fz;
	int l;
	//int step_count = int(gl_LightSource[2].spotCutoff);
	int step_count = 8;
	
	/* this could go to the vertex shader... */
	vec4 j = gl_ProjectionMatrixInverse * vec4(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0, -gl_FragCoord.z * 2.0 - 1.0, 1.0);
	j.xyz / j.w;
	view_vec = normalize(j.xyz);
	
	//depth = -j.z;

	
	if(sysLightParams[sysLightIndexes[0]].sysLightType == LIGHT_POINT)
	{
		intersect_sphere(gl_LightSource[0].position.xyz, vec3(0.0, 0.0, 0.0), view_vec, depth0, depth1, sysLightParams[sysLightIndexes[0]].sysLightRadius);
	
	    start_depth = max(0.0, depth0);
	  	start_depth=min(depth / (-view_vec.z), start_depth);
	  	//end_depth = j.z;
	    end_depth = max(0.0, depth1);
		end_depth=min(depth / (-view_vec.z), end_depth);

		cur_pos = view_vec * start_depth;
		step_len_world = (end_depth - start_depth) / float(step_count);
			
		cur_pos += view_vec*step_len_world*(2.0*noise-1.0);
		shadow=1.0;
		
		for(l=0; l<step_count; l++)
		{
			cur_pos += step_len_world * view_vec;
			shadow = sample_3d_shadow_map(cur_pos);
			accum += shade_point_point(gl_LightSource[0].position.xyz, sysLightParams[sysLightIndexes[0]].sysLightRadius, cur_pos) * scattering * step_len_world * shadow;
		}
	}
	else if(sysLightType == LIGHT_SPOT)
	{
		intersect_cone(gl_LightSource[0].position.xyz, gl_LightSource[0].spotDirection, sysLightParams[sysLightIndexes[0]].sysLightRadius, ((3.14159265*sysLightParams[sysLightIndexes[0]].sysLightSpotCutoff)/180.0), vec3(0.0, 0.0, 0.0), view_vec, depth0, depth1);
	    start_depth = max(0.0, depth0);
	  	start_depth=min(depth / (-view_vec.z), start_depth);
	    end_depth = max(0.0, depth1);
		end_depth=min(depth / (-view_vec.z), end_depth);
		
		//start_depth = j.z;
	
		cur_pos = view_vec * start_depth;
		step_len_world = (end_depth - start_depth) / float(step_count);
			
		cur_pos += view_vec*step_len_world*(2.0*noise-1.0);
		shadow=1.0;
		p = 1.0;
		
		/*if(sysProjectTexture == 1)
		{
			for(l=0; l<step_count; l++)
			{
				cur_pos += step_len_world * view_vec;
				shadow = sample_2d_shadow_map(cur_pos);
				p = project_texture(sysTextureSampler1, cur_pos);
				accum += shade_point_spot(gl_LightSource[0].position.xyz, gl_LightSource[0].spotDirection, gl_LightSource[0].diffuse.a, cur_pos) * scattering * step_len_world * shadow * p;
			}
		}
		else
		{*/
			for(l=0; l<step_count; l++)
			{
				cur_pos += step_len_world * view_vec;
				shadow = sample_2d_shadow_map(cur_pos);
				accum += shade_point_spot(gl_LightSource[0].position.xyz, gl_LightSource[0].spotDirection, sysLightParams[sysLightIndexes[0]].sysLightRadius, cur_pos) * scattering * step_len_world * shadow;
			}
		//}
		
		
	}
	
	
	
	
	gl_FragColor = vec4(vcolor*accum, 1.0);
}




