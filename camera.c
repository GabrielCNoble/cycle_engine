#include "camera.h"
#include "draw.h"

extern renderer_t renderer;
extern camera_array camera_a;

#ifdef __cplusplus
extern "C"
{
#endif

/*
=============
camera_Init
=============
*/
PEWAPI void camera_Init()
{
	camera_t default_camera;
	
	camera_a.cameras=NULL;
	camera_a.camera_count=0;
	camera_ResizeCameraArray(8);
	
	mat3_t orientation = mat3_t_id();
	
	/*CreateOrthographicMatrix(&default_camera.projection_matrix, -renderer.width/40, renderer.width/40, renderer.height/40, -renderer.height/40, 0.1, 100.0);*/
	//CreatePerspectiveMatrix(&default_camera.projection_matrix, 0.68, (float)renderer.width/(float)renderer.height, 0.1, 1000.0, &default_camera.frustum);

	
	//default_camera.local_position=vec3(0.0, 1.5 , 0.0);
	//default_camera.local_orientation=mat3_t_id();
	//default_camera.name=(char *)"default_camera";
	//camera_RotateCamera(&default_camera, vec3(0.0, 1.0, 0.0), 0.5, 1);
	//camera_RotateCamera(&default_camera, vec3(1.0, 0.0, 0.0), -0.02, 0);
	
	//camera_CreateCamera(&default_camera);
	camera_CreateCamera("default_camera", vec3(0.0, 1.5, 0.0), &orientation, 0.68 , (float) renderer.width, (float) renderer.height, 0.1, 1000.0);
	camera_SetCameraByIndex(0);	
	
	return;
}


/*
=============
camera_Finish
=============
*/
PEWAPI void camera_Finish()
{
	free(camera_a.cameras);
	return;
}


/*
=============
camera_ResizeCameraArray
=============
*/
PEWAPI void camera_ResizeCameraArray(int new_size)
{
	camera_t *temp=(camera_t *)calloc(new_size, sizeof(camera_t));
	if(camera_a.cameras)
	{
		memcpy(temp, camera_a.cameras, sizeof(camera_t)*camera_a.camera_count);
		free(camera_a.cameras);
	}
	camera_a.cameras=temp;
	camera_a.array_size=new_size;
	return;
}



PEWAPI int camera_CreateCamera(char *name, vec3_t position, mat3_t *orientation, float fovy, float width, float height, float znear, float zfar)
{
	int camera_index = camera_a.camera_count;
	if(camera_index >= camera_a.array_size)
	{
		camera_ResizeCameraArray(camera_a.camera_count + 2);
	}
	camera_a.cameras[camera_index].name = strdup(name);
	camera_a.cameras[camera_index].local_position = position;
	memcpy(&camera_a.cameras[camera_index].local_orientation, orientation, sizeof(mat3_t));
	CreatePerspectiveMatrix(&camera_a.cameras[camera_index].projection_matrix, fovy, width/height, znear, zfar, &camera_a.cameras[camera_index].frustum);
	camera_a.cameras[camera_index].width = width;
	camera_a.cameras[camera_index].height = height;
	camera_a.cameras[camera_index].zoom = 1.0;
	camera_a.cameras[camera_index].exposure = 1.0;
	camera_a.cameras[camera_index].camera_index = camera_index;
	camera_a.cameras[camera_index].assigned_node=scenegraph_AddNode(NODE_CAMERA, camera_index, -1, camera_a.cameras[camera_index].name);
	camera_ComputeWorldToCameraMatrix(&camera_a.cameras[camera_index]);
	
	camera_a.camera_count++;
	
	return camera_index;
}

/*
=============
camera_CreateCamera
=============
*/
PEWAPI int camera_CreateCameraFromData(camera_t *camera)
{
	if(camera_a.camera_count>=camera_a.array_size)
	{
		camera_ResizeCameraArray(camera_a.array_size);
	}
	
	camera_ComputeWorldToCameraMatrix(camera);
	
	camera_a.cameras[camera_a.camera_count]=*camera;
	camera_a.cameras[camera_a.camera_count].camera_index=camera_a.camera_count;
	camera_a.cameras[camera_a.camera_count].assigned_node=scenegraph_AddNode(NODE_CAMERA, camera_a.camera_count, -1, camera->name);
	camera_a.camera_count++;
	return camera_a.camera_count-1;
}


/*
=============
camera_SetCameraByIndex
=============
*/
PEWAPI void camera_SetCameraByIndex(int camera_index)
{
	if(camera_index>-1)
	{
		renderer.active_camera_index=camera_index;
	}
}


/*
=============
camera_ComputeWorldToCameraMatrix
=============
*/
PEWAPI void camera_ComputeWorldToCameraMatrix(camera_t *camera)
{
	mat4_t world_to_camera_matrix;
	mat3_t t_rot;
	MatrixCopy3(&t_rot, &camera->world_orientation);
	
	mat3_t_transpose(&t_rot);
	
	mat4_t_compose(&world_to_camera_matrix, &t_rot, vec3(0.0, 0.0 ,0.0));
	
	world_to_camera_matrix.floats[3][0]=(-camera->world_position.floats[0])*world_to_camera_matrix.floats[0][0]	+	
										(-camera->world_position.floats[1])*world_to_camera_matrix.floats[1][0]	+
										(-camera->world_position.floats[2])*world_to_camera_matrix.floats[2][0];
										
	world_to_camera_matrix.floats[3][1]=(-camera->world_position.floats[0])*world_to_camera_matrix.floats[0][1]	+	
										(-camera->world_position.floats[1])*world_to_camera_matrix.floats[1][1]	+
										(-camera->world_position.floats[2])*world_to_camera_matrix.floats[2][1];
										
	world_to_camera_matrix.floats[3][2]=(-camera->world_position.floats[0])*world_to_camera_matrix.floats[0][2]	+	
										(-camera->world_position.floats[1])*world_to_camera_matrix.floats[1][2]	+
										(-camera->world_position.floats[2])*world_to_camera_matrix.floats[2][2];
										
	MatrixCopy4(&camera->world_to_camera_matrix, &world_to_camera_matrix);
	//camera->world_to_camera_matrix=world_to_camera_matrix;
}


/*
=============
camera_ResetWorldToCameraMatrix
=============
*/
PEWAPI void camera_ResetWorldToCameraMatrix()
{
	mat4_t world_to_camera_matrix;
	
	mat3_t t_rot = camera_a.cameras[renderer.active_camera_index].world_orientation;
	mat3_t_transpose(&t_rot);
	
	mat4_t_compose(&world_to_camera_matrix, &t_rot, vec3(0.0, 0.0, 0.0));
	
	world_to_camera_matrix.floats[3][0]=(-camera_a.cameras[renderer.active_camera_index].world_position.floats[0])*world_to_camera_matrix.floats[0][0]	+	
										(-camera_a.cameras[renderer.active_camera_index].world_position.floats[1])*world_to_camera_matrix.floats[1][0]	+
										(-camera_a.cameras[renderer.active_camera_index].world_position.floats[2])*world_to_camera_matrix.floats[2][0];
										
	world_to_camera_matrix.floats[3][1]=(-camera_a.cameras[renderer.active_camera_index].world_position.floats[0])*world_to_camera_matrix.floats[0][1]	+	
										(-camera_a.cameras[renderer.active_camera_index].world_position.floats[1])*world_to_camera_matrix.floats[1][1]	+
										(-camera_a.cameras[renderer.active_camera_index].world_position.floats[2])*world_to_camera_matrix.floats[2][1];
										
	world_to_camera_matrix.floats[3][2]=(-camera_a.cameras[renderer.active_camera_index].world_position.floats[0])*world_to_camera_matrix.floats[0][2]	+	
										(-camera_a.cameras[renderer.active_camera_index].world_position.floats[1])*world_to_camera_matrix.floats[1][2]	+
										(-camera_a.cameras[renderer.active_camera_index].world_position.floats[2])*world_to_camera_matrix.floats[2][2];
	
	camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix=world_to_camera_matrix;
}


/*
=============
camera_SetCurrentCameraProjectionMatrix
=============
*/
PEWAPI void camera_SetCurrentCameraProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&camera_a.cameras[renderer.active_camera_index].projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
}


/*
=============
camera_SetCameraProjectionMatrix
=============
*/
PEWAPI void camera_SetCameraProjectionMatrix(camera_t *camera)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&camera->projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
}


/*
=============
camera_TranslateCamera
=============
*/
PEWAPI void camera_TranslateCamera(camera_t *camera, vec3_t direction, float amount, int b_set)
{
	camera->local_position.floats[0]=direction.floats[0]*amount;
	camera->local_position.floats[1]=direction.floats[1]*amount;
	camera->local_position.floats[2]=direction.floats[2]*amount;
	camera_ComputeWorldToCameraMatrix(camera);
	return;
}

PEWAPI void camera_TranslateActiveCamera(vec3_t direction, float amount, int b_set)
{
	camera_a.cameras[renderer.active_camera_index].local_position.floats[0]-=direction.floats[0]*amount;
	camera_a.cameras[renderer.active_camera_index].local_position.floats[1]-=direction.floats[1]*amount;
	camera_a.cameras[renderer.active_camera_index].local_position.floats[2]-=direction.floats[2]*amount;
	camera_ComputeWorldToCameraMatrix(&camera_a.cameras[renderer.active_camera_index]);
	return;
}


/*
=============
camera_RotateCamera
=============
*/
PEWAPI void camera_RotateCamera(camera_t *camera, vec3_t axis, float angle, int b_set)
{
	mat3_t_rotate(&camera->local_orientation, axis, -angle, b_set);
	//camera_ComputeWorldToCameraMatrix(camera);
	return;
}


PEWAPI void camera_ApplyPitchYawToCamera(camera_t *camera, float yaw, float pitch, vec3_t yaw_axis, vec3_t pitch_axis)
{
	
	mat3_t p=mat3_t_id();
	mat3_t y=mat3_t_id();
	mat3_t temp;
	
	
	mat3_t_rotate(&y, yaw_axis, yaw, 1);
	
	//pitch_axis = MultiplyVector3(&y, pitch_axis);
	
	mat3_t_rotate(&p, pitch_axis, pitch, 1);
	
	/* First apply pitch, THEN yaw. This is necesary
	because the camera is using the world vector
	(1, 0, 0) as its right vector, which is not 
	modified when the camera turns. So, if applying 
	the yaw first, the camera's right vector might
	become its forward vector at times, so it will
	roll instead of pitch. To apply the yaw before
	the pitch, the world right vector must be rotated
	by the yaw matrix, so it will always be pointing 
	to the right relative to the camera.*/
	mat3_t_mult(&camera->local_orientation, &p, &y);
				
	//mat3_t_mult(&camera->local_orientation, &y, &p);
	
}


/*
=============
camera_GetActiveCamera
=============
*/
/*PEWAPI camera_t *camera_GetActiveCamera()
{
	return &camera_a.cameras[renderer.active_camera_index];
}*/

PEWAPI frustum_t camera_GetActiveCameraFrustum()
{
	return camera_a.cameras[renderer.active_camera_index].frustum;
}

/*
=============
camera_GetCameraRightVector
=============
*/
PEWAPI vec3_t camera_GetCameraLocalRightVector(camera_t *camera)
{
	vec3_t r;
	r.floats[0]=-camera->local_orientation.floats[0][0];
	r.floats[1]=-camera->local_orientation.floats[0][1];
	r.floats[2]=-camera->local_orientation.floats[0][2];
	return r;
}


/*
=============
camera_GetCameraUpVector
=============
*/
PEWAPI vec3_t camera_GetCameraLocalUpVector(camera_t *camera)
{
	vec3_t r;
	r.floats[0]=-camera->local_orientation.floats[1][0];
	r.floats[1]=-camera->local_orientation.floats[1][1];
	r.floats[2]=-camera->local_orientation.floats[1][2];
	return r;
}


/*
=============
camera_GetCameraForwardVector
=============
*/
PEWAPI vec3_t camera_GetCameraLocalForwardVector(camera_t *camera)
{
	vec3_t r;
	r.floats[0]=-camera->local_orientation.floats[2][0];
	r.floats[1]=-camera->local_orientation.floats[2][1];
	r.floats[2]=-camera->local_orientation.floats[2][2];
	return r;
}


PEWAPI vec3_t camera_GetCameraWorldRightVector(camera_t *camera)
{
	vec3_t r;
	/*r.floats[0]=camera->world_orientation.floats[0][0];
	r.floats[1]=camera->world_orientation.floats[1][0];
	r.floats[2]=camera->world_orientation.floats[2][0];*/
	r.floats[0]=camera->world_orientation.floats[0][0];
	r.floats[1]=camera->world_orientation.floats[0][1];
	r.floats[2]=camera->world_orientation.floats[0][2];
	return r;
}

PEWAPI vec3_t camera_GetCameraWorldUpVector(camera_t *camera)
{
	vec3_t r;
	/*r.floats[0]=camera->world_orientation.floats[0][1];
	r.floats[1]=camera->world_orientation.floats[1][1];
	r.floats[2]=camera->world_orientation.floats[2][1];*/
	r.floats[0]=camera->world_orientation.floats[1][0];
	r.floats[1]=camera->world_orientation.floats[1][1];
	r.floats[2]=camera->world_orientation.floats[1][2];
	return r;
}

PEWAPI vec3_t camera_GetCameraWorldForwardVector(camera_t *camera)
{
	vec3_t r;
	/*r.floats[0]=camera->world_orientation.floats[0][2];
	r.floats[1]=camera->world_orientation.floats[1][2];
	r.floats[2]=camera->world_orientation.floats[2][2];*/
	
	r.floats[0]=camera->world_orientation.floats[2][0];
	r.floats[1]=camera->world_orientation.floats[2][1];
	r.floats[2]=camera->world_orientation.floats[2][2];
	return r;
}


#ifdef __cplusplus
}
#endif

