#include "material.h"
#include "light.h"




void main()
{
    gl_FragColor = vec4(sysMaterialParams.sysMaterialBaseColor.rgb + sysLightParams[0].sysLightColor.rgb, 1.0);
}
