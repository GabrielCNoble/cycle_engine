#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define LIGHT_DIRECTIONAL 4

#define MAX_ACTIVE_LIGHTS 32
#define CLUSTER_SIZE 32
#define CLUSTER_Z_DIVS 16

#define TAN_FOVY 0.80866137514256524544022586959226


#define POINT_LIGHT_BIAS 0.0000006
#define SPOT_LIGHT_BIAS 0.0000025

#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_arrays_of_arrays : enable
#extension GL_ARB_uniform_buffer_object : enable

uniform int sysLightCount;
uniform int sysLightIndex;
uniform sampler2D sys2DShadowSampler;
uniform usampler3D sysClusterTexture;



/* rotation component */
vec4 sysIndirectionLut0[6][2] = vec4[6][2](vec4[2](vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 0.0)),		/* +X -> -Z, +X -> +Y */

					   				     vec4[2](vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0,-1.0, 0.0)),		/* -X -> +Z, -X -> +Y */
									     
										 vec4[2](vec4(0.0, 1.0, 0.0, 0.0), vec4(-1.0,0.0, 0.0, 0.0)),		/* +Y -> +X, +Y -> -Z */
										 
										 vec4[2](vec4(0.0,-1.0, 0.0, 0.0), vec4(1.0, 0.0, 0.0, 0.0)),		/* -Y -> +X, -Y -> +Z */
										 
										 vec4[2](vec4(0.0, 0.0, 0.0, 1.0), vec4(1.0, 0.0, 0.0, 0.0)),		/* +Z -> +X, +Z -> +Y */
										 
										 vec4[2](vec4(0.0, 0.0, 0.0, 1.0), vec4(-1.0,0.0, 0.0, 0.0)));		/* -Z -> -X, -Z -> +Y */
										 
										 


/* translation component */
vec2 sysIndirectionLut1[6][2] = vec2[6][2](vec2[2](vec2(0.0, 0.0), vec2(1.0, 0.0)),							/* +X -> -Z, +X -> +Y */

									     vec2[2](vec2(0.0, 0.0), vec2(0.0, 1.0)),							/* -X -> +Z, -X -> +Y */
									     
										 vec2[2](vec2(0.0, 1.0/* - 1.0 / 128.0*/), vec2(1.0, 1.0)),							/* +Y -> +X, +Y -> -Z */
										 
										 vec2[2](vec2(1.0, 0.0), vec2(0.0, 0.0)),							/* -Y -> +X, -Y -> +Z */
										 
										 vec2[2](vec2(0.0, 0.0), vec2(0.0, 0.0)),							/* +Z -> +X, +Z -> +Y */
										 
										 vec2[2](vec2(0.0, 0.0), vec2(1.0, 1.0)));							/* -Z -> -X, -Z -> +Y */
										 
										 
										 

/* face origin */
vec2 sysIndirectionLut2[6][2] = vec2[6][2](vec2[2](vec2(2.0, 1.0), vec2(1.0, 0.0)),							/* +X -> -Z, +X -> +Y */

									     vec2[2](vec2(2.0, 0.0), vec2(1.0, 0.0)),							/* -X -> +Z, -X -> +Y */
									     
										 vec2[2](vec2(0.0, 0.0), vec2(2.0, 1.0)),							/* +Y -> +X, +Y -> -Z */
										 
										 vec2[2](vec2(0.0, 0.0), vec2(2.0, 0.0)),							/* -Y -> +X, -Y -> +Z */
										 
										 vec2[2](vec2(0.0, 0.0), vec2(1.0, 0.0)),							/* +Z -> +X, +Z -> +Y */
										 
										 vec2[2](vec2(0.0, 1.0), vec2(1.0, 0.0)));							/* -Z -> -X, -Z -> +Y */



struct sysLightParamsFields
{
	//mat4 sysWorldToLightProjectionMatrix;
	mat4 sysLightModelViewProjectionMatrix;
	
	vec4 sysLightRightVector;
	vec4 sysLightUpVector;
	vec4 sysLightForwardVector;
	vec4 sysLightPosition;
	vec4 sysLightColor;
	
	float sysLightRadius;
	float sysLightSpotCutoff;
	float sysLightSpotCosCutoff;
	float sysLightSpotBlend;
	
	float sysLightLinearFallof;
	float sysLightSquaredFallof;
	
	float sysLightShadowX;
	float sysLightShadowY;
	float sysLightShadowSize;
	int sysLightType;
	
	float sysLightZNear;
	float sysLightZFar;	
};

struct sysClusterFields
{
	int time_stamp;							/* if this value is different from the current frame count, this cluster is invalid if accessed... */
	int bm0;
	int bm1;
	int bm2;
};

layout (std140) uniform sysLightParamsUniformBlock
{
	sysLightParamsFields sysLightParams[MAX_ACTIVE_LIGHTS];	
};

/*layout (std140) uniform sysClusterUniformBlock
{
	sysClusterFields sysClusters[MAX_ACTIVE_LIGHTS];
};*/



float sysAttenuatePoint(vec3 light_vec, float light_radius, float linear_fallof, float quadratic_fallof)
{
	float l = length(light_vec);
	float a = 1.0 / (l * linear_fallof + l * l * quadratic_fallof);
	return a * clamp((light_radius - l) / light_radius, 0.0, 1.0);
}

float sysAttenuateSpot(vec3 light_vec, vec3 spot_direction, float light_distance, float spot_cos_cutoff, float spot_blend, float linear_fallof, float quadratic_fallof)
{
	float l = length(light_vec);
	float a = 1.0 / (l * linear_fallof + l * l * quadratic_fallof);
	float cos = max(dot(light_vec / l, spot_direction), 0.0);
	
	a *= cos * smoothstep(spot_cos_cutoff, spot_cos_cutoff + spot_blend, cos) * clamp((light_distance - l) / light_distance, 0.0, 1.0);
	return a;
}

float sysSample2DShadowMap(vec4 frag_pos, int light_index)
{
	vec4 v = sysLightParams[light_index].sysLightModelViewProjectionMatrix * frag_pos;
	v /= v.w;
	
	v.x=v.x*0.5 + 0.5;
	v.y=v.y*0.5 + 0.5;
	v.z=v.z*0.5 + 0.5;
	
	
	if(v.x<=1.0 && v.x>=0.0 && v.y<=1.0 && v.y>=0.0 && v.w >=0.0)
	{
		/* v.x and v.y are in range [0.0 ... 1.0], and represent the coords
		within the light's virtual shadow map. Here they have to be mapped
		to physical shadow map coords. x and y are the the bottom left
		corner of the shadow map and w and h are the width and height
		of the virtual shadow map. They all are represented in physical
		shadow map coords. The mapping happens by multiplying the w
		and h by the virtual coords, and then adding the x and y 
		origins. The products w * v.x and h * v.y essentially allows
		to 'slide' within the virtual shadow map in physical
		coords. */
		
		float x = sysLightParams[light_index].sysLightShadowX;
		float y = sysLightParams[light_index].sysLightShadowY;
		float s = sysLightParams[light_index].sysLightShadowSize;

		
		v.x = x + s * v.x;
		v.y = y + s * v.y;
		//v.y = tex_coord_mapping / v.y;
		//shadow = texture2D(sys2DShadowSampler, v.xy).x;
		
		//t = 1.0 - dot(normalize(q), frag_norm);
		if(v.z > texture2D(sys2DShadowSampler, v.xy).x + 0.0000005)
		//if(v.z > shadow)
		{
			return 0.0;
		}
	}
	
	return 1.0;
}

float sysSample3DShadowMap(vec4 world_space_frag_pos, vec4 camera_space_frag_pos, vec3 frag_normal, int light_index)
{
	vec4 v = sysLightParams[light_index].sysLightModelViewProjectionMatrix * world_space_frag_pos;
	//v0 /= v0.w;
	vec3 q = camera_space_frag_pos.xyz - sysLightParams[light_index].sysLightPosition.xyz;
	int face;
	int c = 0;	
	//int d;			
						
	float shadow0;
	float shadow1;
	//float alpha;
	float fz;
	//float sz;
	//float sample_count = 10.0;
	//float samples = 0.0;
	//float step = 1.0 / sample_count;
	float u0;
	float v0;
	float u1;
	float v1;
	//factor = 1.0;
	float ox0;
	float oy0;


	float x = sysLightParams[light_index].sysLightShadowX;
	float y = sysLightParams[light_index].sysLightShadowY;
	float s = sysLightParams[light_index].sysLightShadowSize / 3.0;
	
	float znear = sysLightParams[light_index].sysLightZNear;
	float zfar = sysLightParams[light_index].sysLightZFar;

	//float x = gl_LightSource[1].diffuse.x;
	//float y = gl_LightSource[1].diffuse.y;
	//float w = gl_LightSource[1].diffuse.z / 3.0;
	//float h = gl_LightSource[1].diffuse.w / 2.0;
	float delta_u;
	float delta_v;
	float x0;
	float x1;

	
	float dist;
	vec2 pixel_pos;
	vec2 texel_pos;
	
	vec2 t0pos;
	vec2 t1pos;
	vec2 t2pos;
	vec2 t3pos;
	
	vec4 i0;
	vec2 i1;
	vec2 i2;
	//vec4 r1;
	
	float m;
	
	
	
	float n = ((1.0 - dot(normalize(q), frag_normal)) * POINT_LIGHT_BIAS) + ((POINT_LIGHT_BIAS * 100.0 * (10.0)) / dot(q, q));	
	
	//vec2 uv;
	
	//v.y = -v.y;
	
	vec3 av = abs(v.xyz);
	//vec3 av = v.xyz * sign(v.xyz);
	fz = max(av.x, max(av.y, av.z));
	m = fz;
	fz = (zfar + znear) / (zfar - znear) - (2.0 * zfar * znear) / ((zfar - znear) * fz);
	fz = fz * 0.5 + 0.5;
	
	vec2 sms_size = textureSize(sys2DShadowSampler, 0);
	
	
	// u and v are swapped... but it's just a naming convention... 
	delta_u = 1.0 / (sms_size.y * s);
	delta_v = 1.0 / (sms_size.x * s);
	
	//float size = sms.x * s;
	
	//delta = 1.0 / size;
	
	if(m == av.x)
	{	
		u0 = v.y;
		if(v.x < 0.0)
		{
			/* -X */
			v0 = v.z;
			oy0 = s;
			face = 1;
		}
		else
		{
			/* +X */
			v0 = -v.z;	
			oy0 = 0.0;
			face = 0;
		}
		ox0 = 0.0;
	}
	else if(m == av.y)
	{	
		v0 = v.x;
		if(v.y < 0.0)
		{
			/* -Y */
			u0 = v.z;
			oy0 = s;
			face = 3;
		}
		else
		{
			/* +Y */
			u0 = -v.z;	
			oy0 = 0.0;
			face = 2;
		}
		ox0 = s;	
	}
	else
	{	
		u0 = v.y;
		if(v.z < 0.0)
		{
			/* -Z */
			v0 = -v.x;
			oy0 = s;
			face = 5;
			
		}
		else
		{
			/* +Z */
			v0 = v.x;	
			oy0 = 0.0;
			face = 4;
		}
		ox0 = 2.0 * s;
	}
	
	u0 = 0.5 * (u0 / m) + 0.5;
	v0 = 0.5 * (v0 / m) + 0.5;
	
	u1 = u0;
	v1 = v0;
	
	c = int(v0 >= 1.0 - delta_v) | (int(u0 >= 1.0 - delta_u) << 1);
	
	v0 = x + ox0 + s * v0;
	u0 = y + oy0 + s * u0;
	
	
	pixel_pos = vec2(sms_size.x * v0, sms_size.y * u0);
	t0pos = floor(pixel_pos);
	
	/*if(face == 0)
	{
		factor = 0.25;
		return;
	}*/
	
	
	if(c > 0)
	{
		switch(c)
		{
			/* vertical edge... */
			case 1:
				
				i0 = sysIndirectionLut0[face][0];
				i1 = sysIndirectionLut1[face][0];
				i2 = sysIndirectionLut2[face][0];
				
				v0 = x + i2.x * s + s * (i1.x + (v1 * i0.x + u1 * i0.y));
				u0 = y + i2.y * s + s * (i1.y + (v1 * i0.z + u1 * i0.w));
				
				t1pos = t0pos + vec2(0, 1);
				t2pos = floor(vec2(sms_size.x * v0, sms_size.y * u0));
				t3pos = t2pos + vec2(0, 1);
				
			break;	
			
			/* horizontal edge... */
			case 2:
				
				
				i0 = sysIndirectionLut0[face][1];
				i1 = sysIndirectionLut1[face][1];
				i2 = sysIndirectionLut2[face][1];
				
				v0 = x + i2.x * s + s * (i1.x + (v1 * i0.x + u1 * i0.y));
				u0 = y + i2.y * s + s * (i1.y + (v1 * i0.z + u1 * i0.w));
				
				t1pos = floor(vec2(sms_size.x * v0, sms_size.y * u0));
				t2pos = t0pos + vec2(1, 0);
				t3pos = t1pos + vec2(1, 0);
				
			break;
			
			/* corner... */
			case 3:
				t1pos = t0pos;
				t2pos = t0pos;
				t3pos = t0pos;
			break;
		}

	}
	else
	{
		t1pos = t0pos + vec2(0, 1);
		t2pos = t0pos + vec2(1, 0);
		t3pos = t0pos + vec2(1, 1);
	}
	
	/*t1pos = t0pos + vec2(0, 1);
	t2pos = t0pos + vec2(1, 0);
	t3pos = t0pos + vec2(1, 1);*/
	
	dist = pixel_pos.x - t0pos.x;
		
	x0 = (1.0 - dist) * texelFetch(sys2DShadowSampler, ivec2(t0pos), 0).r + 
				dist  * texelFetch(sys2DShadowSampler, ivec2(t2pos), 0).r;
					
	x1 = (1.0 - dist) * texelFetch(sys2DShadowSampler, ivec2(t1pos), 0).r + 
				dist  * texelFetch(sys2DShadowSampler, ivec2(t3pos), 0).r;			
		
	dist = pixel_pos.y - t0pos.y;
		
	shadow0 = (1.0 - dist) * x0 + dist * x1;
	
	if(fz > shadow0 + n)
	{
		return 0.0;
	}
	return 1.0;
}

ivec3 sysGetCluster(float x_coord, float y_coord, float view_z, float z_near)
{
	ivec3 pos;
	pos.x = int(floor(x_coord / CLUSTER_SIZE));
	pos.y = int(floor(y_coord / CLUSTER_SIZE));
	pos.z = int(log(-view_z / z_near) / log(1.0 + (2.0 * tan(0.68)) / CLUSTER_SIZE)) / CLUSTER_Z_DIVS;
		
	if(pos.z > CLUSTER_Z_DIVS) pos.z = CLUSTER_Z_DIVS;
	else if(pos.z < 0) pos.z = 0;
	
	return pos;
}







