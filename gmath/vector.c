#include "vector.h"


#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI vec2_t vec2(float x, float y)
{
	vec2_t result;
	result.floats[0]=x;
	result.floats[1]=y;
	return result;
}

PEWAPI vec2_t add2(vec2_t vec1, vec2_t vec2)
{
	vec2_t result;
	result.floats[0]=vec1.floats[0]+vec2.floats[0];
	result.floats[1]=vec1.floats[1]+vec2.floats[1];
	return result;
}

PEWAPI vec2_t sub2(vec2_t vec1, vec2_t vec2)
{
	vec2_t result;
	result.floats[0]=vec1.floats[0]-vec2.floats[0];
	result.floats[1]=vec1.floats[1]-vec2.floats[1];
	return result;
}
//For division, just multiply by the reciprocal
PEWAPI vec2_t mul2(vec2_t vec1, float value)
{
	vec2_t result;
	result.floats[0]=vec1.floats[0]*value;
	result.floats[1]=vec1.floats[1]*value;
	return result;
}

PEWAPI vec2_t getmiddle2(vec2_t A, vec2_t B)
{
	vec2_t result;
	result.floats[0]=(B.floats[0]-A.floats[0])/2.0;
	result.floats[1]=(B.floats[1]-A.floats[1])/2.0;
	return result;
}


PEWAPI vec2_t Vec3ToVec2(vec3_t vec_3)
{
	vec2_t result={vec_3.floats[0], vec_3.floats[1]};
	return result;
}

/*PEWAPI vec3_t vec3(float x, float y, float z)
{
	vec3_t result;
	result.floats[0]=x;
	result.floats[1]=y;
	result.floats[2]=z;
	return result;
}*/

/*PEWAPI vec4_t vec4(float x, float y, float z, float w)
{
	vec4_t result;
	result.floats[0]=x;
	result.floats[1]=y;
	result.floats[2]=z;
	result.floats[3]=w;
	return result;
}*/

PEWAPI vec3_t GetVec3To(vec3_t From, vec3_t To)
{
	vec3_t result;
	result.floats[0]=To.floats[0]-From.floats[0];
	result.floats[1]=To.floats[1]-From.floats[1];
	result.floats[2]=To.floats[2]-From.floats[2];
	return result;
}

PEWAPI vec3_t getmiddle3(vec3_t A, vec3_t B)
{
	vec3_t result;
	result=GetVec3To(A, B);
	result.floats[0]/=2.0;
	result.floats[1]/=2.0;
	result.floats[2]/=2.0;
	
	return result;
	
}

PEWAPI vec3_t project3(vec3_t vec1, vec3_t vec2)
{
	float u_dot_v;
	float v_dot_v;
	float k;
	
	u_dot_v=dot3(vec1, vec2);
	v_dot_v=dot3(vec2, vec2);
	k=u_dot_v/v_dot_v;
	
	return mul3(vec2, k);
}

PEWAPI vec3_t project3_NORMALIZED(vec3_t vec1, vec3_t vec2)
{
	float u_dot_v;
	float k;
	u_dot_v=dot3(vec1, vec2);
	return mul3(vec2, u_dot_v);
}

/*PEWAPI vec3_t lerp(vec3_t a, vec3_t b, float t)
{
	vec3_t r;
	r.x = a.x * (1.0 - t) + b.x * t;
	r.y = a.y * (1.0 - t) + b.y * t;
	r.z = a.z * (1.0 - t) + b.z * t;
	return r;
}
*/

PEWAPI vec3_t normalize32(vec3_t vec)
{
	vec3_t r;
	float sqrdlen=vec.floats[0]*vec.floats[0] + vec.floats[1]*vec.floats[1] + vec.floats[2]*vec.floats[2];
	
	r.floats[0]=(vec.floats[0]*vec.floats[0])/sqrdlen;
	r.floats[1]=(vec.floats[1]*vec.floats[1])/sqrdlen;
	r.floats[2]=(vec.floats[2]*vec.floats[2])/sqrdlen;
	
	return r;
	
}

/*PEWAPI vec3_t mul3(vec3_t vec, float value)
{
	vec3_t result;
	result.floats[0]=vec.floats[0]*value;
	result.floats[1]=vec.floats[1]*value;
	result.floats[2]=vec.floats[2]*value;
	return result;
}

PEWAPI vec3_t add3(vec3_t vec1, vec3_t vec2)
{
	vec3_t result;
	result.floats[0]=vec1.floats[0]+vec2.floats[0];
	result.floats[1]=vec1.floats[1]+vec2.floats[1];
	result.floats[2]=vec1.floats[2]+vec2.floats[2];
	return result;
}

PEWAPI vec3_t sub3(vec3_t vec1, vec3_t vec2)
{
	vec3_t result;
	result.floats[0]=vec1.floats[0]-vec2.floats[0];
	result.floats[1]=vec1.floats[1]-vec2.floats[1];
	result.floats[2]=vec1.floats[2]-vec2.floats[2];
	return result;
}*/

/*PEWAPI vec3_t vec4vec3(vec4_t vec)
{
	vec3_t v;
	v.floats[0]=vec.floats[0];
	v.floats[1]=vec.floats[1];
	v.floats[2]=vec.floats[2];
	return v;
}*/

/*PEWAPI vec3_t normalize3(vec3_t vec)
{
	vec3_t temp;
	float length=sqrt((vec.floats[0]*vec.floats[0]) + (vec.floats[1]*vec.floats[1]) + (vec.floats[2]*vec.floats[2]));
	temp.floats[0]=vec.floats[0]/length;
	temp.floats[1]=vec.floats[1]/length;
	temp.floats[2]=vec.floats[2]/length;
	return temp;
}*/

PEWAPI vec2_t normalize2(vec2_t vec)
{
	vec2_t temp;
	float length=sqrt((vec.floats[0]*vec.floats[0])+(vec.floats[1]*vec.floats[1]));
	temp.floats[0]=vec.floats[0]/length;
	temp.floats[1]=vec.floats[1]/length;
	return temp;
}

/*vec3_t invert3(vec3_t vec)
{
	vec3_t result;
	result.floats[0]=-vec.floats[0];
	result.floats[1]=-vec.floats[1];
	result.floats[2]=-vec.floats[2];
	return result;
}*/

/*vec4_t invert4(vec4_t vec)
{
	vec4_t result;
	result.floats[0]=-vec.floats[0];
	result.floats[1]=-vec.floats[1];
	result.floats[2]=-vec.floats[2];
	result.floats[3]=-vec.floats[3];
	return result;
}*/

/*vec4_t mul4mat(mat4_t *mat, vec4_t vec)
{
	vec4_t result;
	register float f1=vec.floats[0];
	register float f2=vec.floats[1];
	register float f3=vec.floats[2];
	register float f4=vec.floats[3];
				   
	result.floats[0]=f1*mat->floats[0][0] + 
	   			     f2*mat->floats[1][0] +
	   			     f3*mat->floats[2][0] +
	   			     f4*mat->floats[3][0]  ;
	   			   
	result.floats[1]=f1*mat->floats[0][1] + 
				     f2*mat->floats[1][1] +
				     f3*mat->floats[2][1] +
				     f4*mat->floats[3][1]  ;
	
	result.floats[2]=f1*mat->floats[0][2] + 
				     f2*mat->floats[1][2] +
				     f3*mat->floats[2][2] +
				     f4*mat->floats[3][2]  ;
	
	result.floats[3]=f1*mat->floats[0][3] + 
				     f2*mat->floats[1][3] +
				     f3*mat->floats[2][3] +
				     f4*mat->floats[3][3]  ;
	     

	return result;			   		   			   			   
				   
				   
}*/

/*PEWAPI vec3_t cross(vec3_t vec1, vec3_t vec2)
{
	vec3_t result;
	result.floats[0]=vec1.floats[1]*vec2.floats[2] - vec1.floats[2]*vec2.floats[1];
	result.floats[1]=vec1.floats[2]*vec2.floats[0] - vec1.floats[0]*vec2.floats[2];
	result.floats[2]=vec1.floats[0]*vec2.floats[1] - vec1.floats[1]*vec2.floats[0];
	return result;
}*/

/*PEWAPI float dot3(vec3_t vec1, vec3_t vec2)
{
	return (vec1.floats[0]*vec2.floats[0] + vec1.floats[1]*vec2.floats[1] + vec1.floats[2]*vec2.floats[2]);
}*/

PEWAPI float angle3(vec3_t vec1, vec3_t vec2)
{
	return acos(dot3(vec1, vec2)/(length3(vec1)*length3(vec2)));
}

PEWAPI float angle3_NORMALIZED(vec3_t vec1, vec3_t vec2)
{
	return acos(dot3(vec1, vec2));
}

/*PEWAPI float length3(vec3_t vec)
{
	return sqrt((vec.floats[0]*vec.floats[0]) + (vec.floats[1]*vec.floats[1]) + (vec.floats[2]*vec.floats[2]));
}*/

PEWAPI float length2(vec2_t vec)
{
	return sqrt((vec.floats[0]*vec.floats[0]) + (vec.floats[1]*vec.floats[1]));
}

PEWAPI float dot2(vec2_t vec1, vec2_t vec2)
{
	return vec1.floats[0]*vec2.floats[0] + vec1.floats[1]*vec2.floats[1];
}


/* gran-schimt orthogonalization */
/*PEWAPI vec3_t gs_orthg(vec3_t ref, vec3_t src)
{
	vec3_t r;
	float d1 = dot3(ref, src);
	float d2 = dot3(ref, ref);
	float d3 = d1 / d2;
	
	r.floats[0] = src.floats[0] - ref.floats[0] * d3;
	r.floats[1] = src.floats[1] - ref.floats[1] * d3;
	r.floats[2] = src.floats[2] - ref.floats[2] * d3;
	
	r = normalize3(r);
	return r;
}*/



/*void memcpyvec3_t(void *dst, void *src, int count)
{
	register float *i_src=(float *)src;
	register float *i_dst=(float *)dst;
	register int i;
	register int c=count;
	for(i=0; i<c;)
	{
		*(i_dst+i)=*(i_src+i);
		i++;
		*(i_dst+i)=*(i_src+i);
		i++;
		*(i_dst+i)=*(i_src+i);
		i++;
	}
	return;
}*/


#ifdef __cplusplus
}
#endif






