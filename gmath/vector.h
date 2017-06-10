#ifndef _VECTOR_H_
#define _VECTOR_H_

#pragma once

#include <math.h>

#include "conf.h"
#include "GL/glew.h"

#include "matrix_types.h"
#include "vector_types.h"





#ifdef __cplusplus
extern "C"
{
#endif


PEWAPI vec2_t vec2(float x, float y);

PEWAPI vec2_t add2(vec2_t vec1, vec2_t vec2);

PEWAPI vec2_t sub2(vec2_t vec1, vec2_t vec2);

PEWAPI vec2_t mul2(vec2_t vec1, float value);

PEWAPI vec2_t getmiddle2(vec2_t A, vec2_t B);

PEWAPI vec2_t Vec3ToVec2(vec3_t vec_3);

PEWAPI static inline vec3_t vec3(float x, float y, float z);

PEWAPI static inline vec4_t vec4(float x, float y, float z, float w);

PEWAPI vec3_t GetVec3To(vec3_t From, vec3_t To);

PEWAPI vec3_t GetMiddlePoint(vec3_t A, vec3_t B);

PEWAPI vec3_t project3(vec3_t vec1, vec3_t vec2);

PEWAPI vec3_t project3_NORMALIZED(vec3_t vec1, vec3_t vec2);

PEWAPI static inline vec3_t mul3(vec3_t vec, float value);

PEWAPI static inline vec3_t add3(vec3_t vec1, vec3_t vec2);

PEWAPI static inline vec3_t sub3(vec3_t vec1, vec3_t vec2);

PEWAPI static inline vec3_t vec4vec3(vec4_t vec);

PEWAPI static inline vec4_t vec3vec4(vec3_t vec);

PEWAPI static inline vec3_t normalize3(vec3_t vec);

PEWAPI static inline vec3_t lerp3(vec3_t a, vec3_t b, float t);

PEWAPI vec3_t normalize32(vec3_t vec);

PEWAPI vec2_t normalize2(vec2_t vec);

PEWAPI static inline vec3_t cross(vec3_t vec1, vec3_t vec2);

PEWAPI vec3_t invert3(vec3_t vec);

PEWAPI vec4_t invert4(vec4_t vec);

PEWAPI vec4_t mul4mat(mat4_t *mat, vec4_t vec);

PEWAPI static inline quaternion_t lerp4(quaternion_t *a, quaternion_t *b, float t);

PEWAPI static inline quaternion_t slerp(quaternion_t *a, quaternion_t *b, float t);

//PEWAPI static inline quaternion_t squad(quaternion_t *a, quaternion_t *b, quaternion_t *c, float t);

PEWAPI static inline quaternion_t qlog(quaternion_t *q);

PEWAPI static inline quaternion_t qexp(quaternion_t *q);

PEWAPI static inline float dot4(vec4_t *a, vec4_t *b);

PEWAPI static inline quaternion_t qinverse(quaternion_t *q);

PEWAPI static inline quaternion_t qmult(quaternion_t *a, quaternion_t *b);

PEWAPI static inline float dot3(vec3_t vec1, vec3_t vec2);

PEWAPI float angle3(vec3_t vec1, vec3_t vec2);

PEWAPI float angle3_NORMALIZED(vec3_t vec1, vec3_t vec2);

PEWAPI static inline float length3(vec3_t vec);

PEWAPI float length2(vec2_t vec);

PEWAPI float dot2(vec2_t vec1, vec2_t vec2);

PEWAPI void memcpy_vec2_t(void *dst, void *src, int count);

PEWAPI static inline  vec3_t gs_orthg(vec3_t ref, vec3_t src);



#ifdef __cplusplus
}
#endif

#ifndef VECTOR_INLINES_INL
#include "vector_inlines.inl"
#endif 


#endif // _VECTOR_H_


