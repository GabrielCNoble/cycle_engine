#ifndef CAMERA_H
#define CAMERA_H
#include "camera_types.h"
#include "conf.h"
#include "includes.h"
#include "matrix.h"
#include "vector.h"
#include "frustum.h"
#include "scenegraph.h"



#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void camera_Init();

PEWAPI void camera_Finish();

PEWAPI void camera_ResizeCameraArray(int new_size);

PEWAPI int camera_CreateCamera(char *name, vec3_t position, mat3_t *orientation, float fovy, float width, float height, float znear, float zfar);

PEWAPI int camera_CreateCameraFromData(camera_t *camera);

PEWAPI void camera_SetCameraByIndex(int camera_index);

PEWAPI void camera_ComputeWorldToCameraMatrix(camera_t *camera);

PEWAPI void camera_ResetWorldToCameraMatrix();

PEWAPI void camera_SetCurrentCameraProjectionMatrix();

PEWAPI void camera_SetCameraProjectionMatrix(camera_t *camera);

PEWAPI void camera_TranslateCamera(camera_t *camera, vec3_t direction, float amount, int b_set);

PEWAPI void camera_TranslateActiveCamera(vec3_t direction, float amount, int b_set);

PEWAPI void camera_RotateCamera(camera_t *camera, vec3_t axis, float angle, int b_set);

PEWAPI void camera_ApplyPitchYawToCamera(camera_t *camera, float yaw, float pitch, vec3_t yaw_axis, vec3_t pitch_axis);

PEWAPI inline camera_t *camera_GetActiveCamera();

PEWAPI frustum_t camera_GetActiveCameraFrustum();

PEWAPI vec3_t camera_GetCameraLocalRightVector(camera_t *camera);

PEWAPI vec3_t camera_GetCameraLocalUpVector(camera_t *camera);

PEWAPI vec3_t camera_GetCameraLocalForwardVector(camera_t *camera);

PEWAPI vec3_t camera_GetCameraWorldRightVector(camera_t *camera);

PEWAPI vec3_t camera_GetCameraWorldUpVector(camera_t *camera);

PEWAPI vec3_t camera_GetCameraWorldForwardVector(camera_t *camera);

#include "camera.inl"

#ifdef __cplusplus
}
#endif




#endif /* CAMERA_H */
