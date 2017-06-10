#ifndef _PLANE_H_
#define _PLANE_H_

#pragma once

#include <math.h>

#include "conf.h"
#include "plane_types.h"
#include "vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI plane_t ComputePlane(vec3_t A, vec3_t B, vec3_t C);

PEWAPI float GetDistancePlanePoint(plane_t plane, vec3_t point);

PEWAPI float GetDistancePlanePoint_NORMALIZED(plane_t plane, vec3_t point);

PEWAPI int GetIntersectionLinePlane(plane_t plane, vec3_t a, vec3_t b, vec3_t *intersection);

#ifdef __cplusplus
}
#endif




#endif // _PLANE_H_
