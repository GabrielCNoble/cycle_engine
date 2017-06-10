#include "plane.h"

#ifdef __cplusplus
extern "C"
{
#endif 

PEWAPI plane_t ComputePlane(vec3_t A, vec3_t B, vec3_t C)
{
	plane_t p;
	p.normal=normalize3(cross(sub3(B, A), sub3(C, A)));
	p.d=dot3(p.normal, A);
	return p;
}

PEWAPI float GetDistancePlanePoint(plane_t plane, vec3_t point)
{
	float t=(dot3(plane.normal, point) - plane.d)/dot3(plane.normal, plane.normal);
	return length3(mul3(plane.normal, t));		
}

PEWAPI float GetDistancePlanePoint_NORMALIZED(plane_t plane, vec3_t point)
{
	return (dot3(plane.normal, point) - plane.d);
}


PEWAPI int GetIntersectionLinePlane(plane_t plane, vec3_t a, vec3_t b, vec3_t *intersection)
{
	/*float t;
	vec3_t ab=sub3(b, a);
	if(t>=0.0 && t<=1.0)
	{
		t=(plane.d-dot3(plane.normal, a))/(dot3(plane.normal, ab));
		*intersection=add3(a, mul3(ab, t));
		return 1;
	}
	return 0;*/
	
}

#ifdef __cplusplus
}
#endif 







