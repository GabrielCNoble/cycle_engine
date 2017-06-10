#ifndef LINE_H
#define LINE_H

#include "conf.h"
#include "line_types.h"


PEWAPI line2_t line2(vec2_t a, vec2_t b);

PEWAPI line3_t line3(vec3_t a, vec3_t b);

PEWAPI int line_PointInLine2(line2_t line);




#endif /* LINE_H */
