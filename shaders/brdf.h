float ndf_ggx(vec3 normal, vec3 half_vec, float roughness)
{
	//vec3 h = normalize(light_vec + view_vec);
	float q = max(dot(normal, half_vec), 0.0);
	float a = roughness * roughness;
	a *= a;
	float b = q * q * (a - 1.0) + 1.0;
	b = b * b * 3.14159265;
	return a / b;
}

float g_schlick_ggx(vec3 normal, vec3 direction, float roughness)
{
	float k = (roughness + 1.0);
	k = (k * k) / 8.0;
	float d = max(dot(normal, direction), 0.0);
	return d / (d * (1.0 - k) + k);
}

float g_smith(vec3 normal, vec3 view_vec, vec3 light_vec, float roughness)
{
	return max(g_schlick_ggx(normal, view_vec, roughness) * g_schlick_ggx(normal, light_vec, roughness), 0.0);
}

vec3 f_schlick(vec3 normal, vec3 direction, vec3 base, float metalness)
{
	float q = 1.0 - max(dot(normal, direction), 0.0);
	//q = pow(q, 5.0);
	
	vec3 f = mix(vec3(0.04), base, metalness);
	//q = q * q * q * q * q;
	//return (f + (vec3(1.0) - f) * q);
	return f + (1.0 - f) * pow(q, 5.0);
}



vec3 cook_torrance(vec3 light_vec, vec3 view_vec, vec3 normal, vec3 base, float roughness, float metalness)
{
	vec3 half_vec = normalize(light_vec + view_vec);
	//float t = max(dot(normal, half_vec), 0.0);
	
	vec3 f = f_schlick(normal, view_vec, base, metalness);
	vec3 a = ndf_ggx(normal, half_vec, roughness) *  
			 g_smith(normal, view_vec, light_vec, roughness) * f;
			 
	float q = max(dot(normal, light_vec), 0.0);		  
	float b = 4.0 * max(dot(normal, view_vec), 0.0) * q + 0.001;		  	
			  
	vec3 s = a / b;
	vec3 d = (vec3(1.0) - f) * (1.0 - metalness);
	return vec3(d * base / 3.14159265 + s) * q; 
}
