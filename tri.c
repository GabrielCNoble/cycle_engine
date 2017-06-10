#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "tri.h"


/*vec3_t cross(vec3_t a, vec3_t b)
{
	vec3_t q;
	
	q.x = a.y * b.z - a.z * b.y;
	q.y = a.x * b.z - a.z * b.x;
	q.z = a.x * b.y - a.y * b.x;
	
	return q;
}

float dot(vec3_t a, vec3_t b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}*/

/*vec3_t lerp(vec3_t a, vec3_t b, float l)
{
	vec3_t q;
	
	q.x = a.x * (1.0 - l) + b.x * l; 
	q.y = a.y * (1.0 - l) + b.y * l;
	q.z = a.z * (1.0 - l) + b.z * l;
	
	return q;
}*/

/*vec3_t normalize(vec3_t v)
{
	vec3_t q;
	float l = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	q.x = v.x / l;
	q.y = v.y / l;
	q.z = v.z / l;
	return q;
}*/




/*plane_t compute_plane(vec3_t a, vec3_t b, vec3_t c)
{
	plane_t p;
	vec3_t s;
	vec3_t t;
	
	s.x = b.x - a.x;
	s.y = b.y - a.y;
	s.z = b.z - a.z;
	s = normalize(s);
	
	t.x = c.x - a.x;
	t.y = c.y - a.y;
	t.z = c.z - a.z;
	t = normalize(t);

	p.n = cross(s, t);
	p.d = dot(a, p.n);
	p.p = a;
	
	return p;
}

vec3_t intersect_plane(vec3_t a, vec3_t b, plane_t p)
{
	vec3_t q;
	vec3_t c;
	
	float t;
	float i;
	
	c.x = p.p.x - a.x;
	c.y = p.p.y - a.y;
	c.z = p.p.z - a.z;
	
	q.x = b.x - a.x;
	q.y = b.y - a.y;
	q.z = b.z - a.z;
	
	t = dot(q, p.n);
	
	i = dot(c, p.n) / t;
	
	q = lerp(a, b, i);
	
	return q;
}*/

void triangulate(vec3_t *in_verts, int in_vert_count, vec3_t **out_verts, int *out_vert_count)
{
	*out_vert_count = 0;
	*out_verts = (vec3_t *)malloc(sizeof(vec3_t) * (in_vert_count - 2) * 3 * 10000);
	trng_rec(in_verts, in_vert_count, *out_verts, out_vert_count);
}


int trng_rec(vec3_t *in_verts, int in_vert_count, vec3_t *out_verts, int *out_vert_count)
{
	
	int i;
	int c = in_vert_count;
	int j;
	int k = c;
	
	int ai = -1;
	int bi = -1;
	
	vec3_t tri[3];
	
	
	vec3_t *vin;
	vec3_t *vout;
	//float *vout1;
	
	vec3_t a;
	vec3_t b;
	vec3_t fn;
	vec3_t h;
	
	vec3_t s;
	vec3_t t;
	
	int p = 0;
	int best = 999999999;
	
	if(in_vert_count < 2)
	{
		return 0;
	}
	else if(in_vert_count == 3)
	{
		out_verts[(*out_vert_count)++] = in_verts[0];
		out_verts[(*out_vert_count)++] = in_verts[1];
		out_verts[(*out_vert_count)++] = in_verts[2];
		return 3;
	}
	
	
	a = in_verts[0];
	b = in_verts[1];
	h = in_verts[2];
	
	s.x = b.x - a.x;
	s.y = b.y - a.y;
	s.z = b.z - a.z;
	
	t.x = h.x - a.x;
	t.y = h.y - a.y;
	t.z = h.z - a.z;
	
	fn = cross(s, t);
	
	for(i = 2; i < c; i++)
	{
		b = in_verts[i];
		
		s.x = b.x - a.x;
		s.y = b.y - a.y;
		s.z = b.z - a.z;
		p = 0;
		for(j = 1; j < c; j++)
		{
			if(j == i) continue;
			h = in_verts[j];
			t.x = h.x - a.x; 
			t.y = h.y - a.y;
			t.z = h.z - a.z;
			
			h = cross(s, t);
			
			if(dot3(h, fn) < 0.0)
			{
				p--;
			}
			else
			{
				p++;
			}
		}
		p = abs(p);
		if(p < best)
		{
			best = p;
			bi = i;
		}
		
	}
	
	k = in_vert_count >> 1;
	vin = (vec3_t *)malloc(sizeof(vec3_t) * in_vert_count);
	for(i = 0; i < bi + 1; i++)
	{
		vin[i] = in_verts[i];
	}
	trng_rec(vin, bi + 1, out_verts, out_vert_count);
	
	//out_verts[(*out_vert_count)++] = tri[0];
	//out_verts[(*out_vert_count)++] = tri[1];
	//out_verts[(*out_vert_count)++] = tri[2];
	
	
	for(i = bi; i < in_vert_count + 1; i++)
	{
		vin[i - bi] = in_verts[i % in_vert_count];
	}
	trng_rec(vin, in_vert_count - bi + 1, out_verts, out_vert_count);
	
	//out_verts[(*out_vert_count)++] = tri[0];
	//out_verts[(*out_vert_count)++] = tri[1];
	//out_verts[(*out_vert_count)++] = tri[2];
	
	free(vin);
	return 0;
	
	
	

}

















