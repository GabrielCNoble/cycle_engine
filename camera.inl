#ifndef CAMERA_INL
#define CAMERA_INL

#include "camera_types.h"
#include "draw_types.h"

extern camera_array camera_a;
extern renderer_t renderer;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI camera_t *camera_GetActiveCamera()
{
	return &camera_a.cameras[renderer.active_camera_index];
}

#ifdef __cplusplus
}
#endif



#endif /* CAMERA_INL */
