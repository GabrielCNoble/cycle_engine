#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "conf.h"
#include <math.h>
#include "frustum_types.h"
#include "vector_types.h"

PEWAPI int PointInsideFrustum(frustum_t *frustum, vec3_t point);

PEWAPI int SphereInsideFrustum(frustum_t *frustum, vec3_t center, float radius);


#endif /* FRUSTUM_H */
