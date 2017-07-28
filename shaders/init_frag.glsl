#include "material.h"
#include "light.h"


void main()
{
	vec3 light_color = sysLightParams[0].sysLightColor;
    gl_FragColor = vec4(light_color, 1.0);
}
