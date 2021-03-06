#include "material.h"

#extension GL_EXT_gpu_shader4 : enable

varying vec3 normal;
varying vec4 position;
varying vec4 last_position;
varying vec3 tangent;
varying vec2 UV;
varying float w;
varying vec2 delta;
varying float entity_id;


uniform sampler2D sysTextureSampler0;
uniform sampler2D sysTextureSampler1;
uniform sampler2D sysTextureSampler2;
uniform sampler2D sysTextureSampler3;
uniform sampler2D sysTextureSampler4;


uniform float sysTime;
uniform float sysEntityIndex;



vec4 get_view_pos(vec4 homogeneous_position, mat4 inverse_projection_matrix)
{
	vec4 temp = inverse_projection_matrix * homogeneous_position;
	temp /= temp.w;
	return temp;
}


void main()
{
    //vec4 pixel=vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
    int i;
    int q=0;
    int sample_count = 64;
    float f;
    
    float h0;
    float h1;
    float eye_h;
    float eye_h_step;
    
    float x;
    float y;
    float hx;
    float hy;
    
    float step;
    float horizon;
    float p_len;
    float perspective_bias = 0.005;
    float parallax_bias;
    float parallax_len;
    
    mat3 tbn;
    vec3 btangent;
    vec4 diffv;
    vec4 normv;
    vec3 parallax_bn;
    vec2 nUV;
    vec2 temp_dir;
    vec3 tangent_eye;
    vec2 parallax_dir;
    vec2 parallax_disp;
    float parallaxScale = 0.05;
    float parallax_scale =1.0;
    float n_dir = 1.0;
    btangent = cross(tangent, normal);
	tbn[0] = tangent.xyz;
    tbn[1] = btangent.xyz;
    tbn[2] = normal.xyz;
    
    /* parallax occlusion mapping */
    //if(sysFlagHeightTexture == MATERIAL_HeightTexture)
    if(sysMaterialParams.sysMaterialFlags & MATERIAL_HeightTexture)
	{
		tangent_eye = transpose(tbn) * normalize(-position.xyz);
		
		const float minLayers = 8;
	    const float maxLayers = 16;
	    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), tangent_eye)));
	
	   	// height of each layer
	   	float layerHeight = 1.0 / numLayers;
	   	// current depth of the layer
	   	float curLayerHeight = 0;
	   	// shift of texture coordinates for each layer
	   	vec2 dtex = parallaxScale * (tangent_eye.xy / (tangent_eye.z * numLayers));
	   	//vec2 dtex = parallaxScale * tangent_eye.xy / tangent_eye.z / numLayers;
	
	   	// current texture coordinates
	   	vec2 currentTextureCoords = vec2(UV.x, -UV.y);
	
	   	// depth from heightmap
	   	float heightFromTexture = abs(texture2D(sysTextureSampler2, currentTextureCoords).r - 1.0);
	
	   	// while point is above the surface
	   	i = 0;
	   	while(heightFromTexture > curLayerHeight && i < sample_count) 
	   	{
	   	   // to the next layer
	   	   curLayerHeight += layerHeight; 
	   	   // shift of texture coordinates
	   	   currentTextureCoords -= dtex;
	   	   i++;
	   	   // new depth from heightmap
	   	   heightFromTexture = abs(texture2D(sysTextureSampler2, currentTextureCoords).r - 1.0);
	  	}
	
	 	  ///////////////////////////////////////////////////////////
	
	 	  // previous texture coordinates
    	vec2 prevTCoords = currentTextureCoords + dtex;
	
	   	// heights for linear interpolation
	   	float nextH	= heightFromTexture - curLayerHeight;
	   	float prevH	= abs(texture2D(sysTextureSampler2, prevTCoords).r - 1.0) - curLayerHeight + layerHeight;
	
	   	// proportions for linear interpolation
	   	float weight = nextH / (nextH - prevH);
	
	   	// interpolation of texture coordinates
	   	vec2 finalTexCoords = prevTCoords * weight + currentTextureCoords * (1.0-weight);
	
	   	// interpolation of depth values
	  	// parallaxHeight = curLayerHeight + prevH * weight + nextH * (1.0 - weight);
	   	nUV = vec2(finalTexCoords.x, finalTexCoords.y);
	   	n_dir = -1.0;
	    /*nUV = UV;
	    UV.y = -UV.y;
		
		parallax_dir = normalize(tangent_eye.xy);
		parallax_len = sqrt(1.0 - tangent_eye.z * tangent_eye.z) / tangent_eye.z;
		parallax_disp = parallax_dir * parallax_len * parallax_scale;
		
		eye_h_step = 1.0 / float(sample_count);
		eye_h = 0.0;
		
		step = parallax_len / float(sample_count);
		
		
		temp_dir = UV + parallax_disp;
		h0 = abs(texture2D(textureSampler2, temp_dir).r - 1.0) ;
		
		
		for(i = 0; i < sample_count; i++)
		{
			temp_dir -= parallax_dir * step;
			h1 = abs(texture2D(textureSampler2, temp_dir).r - 1.0);
			if(eye_h > h1)
			{
					
				x = step * (sample_count - i );
				y = step * (sample_count - i + 1);
				hx = h1;
				hy = h0;
				
				float parallax_effect = (x * (y - hy) - y * (x - hx)) / ((y - hy) - (x - hx));
				nUV = UV + parallax_disp * parallax_effect * 0.2;
				
			}
			h0 = h1;
			eye_h += eye_h_step;
		}	*/
		
	}
	else
	{
		nUV = UV;
	}
    
    //if(sysFlagDiffuseTexture == MATERIAL_DiffuseTexture)
    if(sysMaterialParams.sysMaterialFlags & MATERIAL_DiffuseTexture)
	{
		diffv = vec4(texture2D(sysTextureSampler0, nUV));
	}
	else
	{
		//diffv = gl_FrontMaterial.diffuse;
		diffv = sysMaterialParams.sysMaterialBaseColor;
	}
	
	//diffv.w = float(gl_FrontMaterial.shininess);
	
	/* normal mapping */
	//if(sysFlagNormalTexture == MATERIAL_NormalTexture)
	if(sysMaterialParams.sysMaterialFlags & MATERIAL_NormalTexture)
	{
		vec3 n = (texture2D(sysTextureSampler1, nUV).xyz * 2.0 - 1.0);
		//n.y *= n_dir;
		//n.y = -n.y;
		normv = vec4(tbn * n, 1.0);
		
		
	}
	else
	{
		normv = vec4(normal.xyz, 1.0);
		
	}
	//normv.a = 1.0;
	//normv.a = gl_FrontMaterial.specular.a;
	
	//if(sysFlagGlossTexture == MATERIAL_GlossTexture)
	if(sysMaterialParams.sysMaterialFlags & MATERIAL_GlossTexture)
	{
		diffv.a = texture2D(sysTextureSampler3, nUV).r;
	}
	else
	{
		//diffv.a = gl_FrontMaterial.shininess;
		diffv.a = sysMaterialParams.sysMaterialGlossiness;
	}
	
	//if(sysFlagMetallicTexture == MATERIAL_MetallicTexture)
	if(sysMaterialParams.sysMaterialFlags & MATERIAL_MetallicTexture)
	{
		normv.a = texture2D(sysTextureSampler4, nUV).r;
	}
	else
	{
		//normv.a = 0.0;
		normv.a = sysMaterialParams.sysMaterialMetallic;
	}
    
    gl_FragData[0] = diffv;
    gl_FragData[1] = normv;
    gl_FragData[2] = vec4(-position.z);
    
} 
