#include "main.h"
#include "input.h"
#include "light.h"
#include "script.h"
#include "armature.h"
#include "physics.h"
#include "log.h"
#include "gui.h"
#include "file.h"
#include "draw_debug.h"
#include "framebuffer.h"

extern entity_array entity_a;
extern input_cache input;
extern light_array light_a;

int l_count;


enum HANDLE_3D_FLAGS
{
	HANDLE_3D_GRABBED_X_AXIS = 1,
	HANDLE_3D_GRABBED_Y_AXIS = 1 << 1,
	HANDLE_3D_GRABBED_Z_AXIS = 1 << 2,
};


int handle_3d_bm;
vec3_t handle_3d_pos;

extern framebuffer_t picking_buffer;
extern framebuffer_t *cur_fb;

entity_ptr selected = {NULL, NULL, NULL, NULL};
entity_ptr detected = {NULL, NULL, NULL, NULL};
void gmain(float delta_time)
{
	
	//selected.extra_data = NULL;
	int s = pew_GetPewState();
	static float f = 0.0;
	vec3_t v;
	mat3_t orientation = mat3_t_id();
	mat4_t model_view_projection_matrix;
	light_ptr l;
	vec4_t p;
	camera_t *active_camera = camera_GetActiveCamera();
	float screen_x;
	float screen_y;
	float mouse_x;
	float mouse_y;
	float delta_x;
	float delta_y;
	if(s != PEW_PLAYING)
	{
		if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_JUST_CLICKED)
		{
			if(!(input.bm_mouse & MOUSE_OVER_WIDGET))
			{
				
				check_3d_handle();
				
				
				
				if(!handle_3d_bm)
				{
					entity_QueryEntityUnderCursor();
					selected = entity_GetEntityUnderCursor();
					
					if(selected.extra_data)
					{
						handle_3d_pos = selected.position_data->world_position;
					}
					
				}

			}
		}
		else if(input.bm_mouse & MOUSE_RIGHT_BUTTON_CLICKED)
		{
			ginput(delta_time);
		}
	}
	else
	{
		ginput(delta_time);
		
		/*l = light_GetLight("swing_light");
		
		l.position_data->local_position.x = sin(f) * 5.0;
		f += 0.001 * delta_time;*/
		
	}
	
	
	if(selected.extra_data)
	{	
	
		if(input.bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
		{
			
			
			p.x = selected.position_data->world_position.x;
			p.y = selected.position_data->world_position.y;
			p.z = selected.position_data->world_position.z;
			p.w = 1.0;
			
			mat4_t_mult(&model_view_projection_matrix, &active_camera->world_to_camera_matrix, &active_camera->projection_matrix);
			
			p = MultiplyVector4(&model_view_projection_matrix, p);
			
			p.x /= p.w;
			p.y /= p.w;
			p.z /= p.w;
			
			screen_x = p.x * 0.5 + 0.5;
			screen_y = p.y * 0.5 + 0.5;
			
			mouse_x = input.normalized_mouse_x * 0.5 + 0.5;
			mouse_y = input.normalized_mouse_y * 0.5 + 0.5;
			
			delta_x = mouse_x - screen_x;
			delta_y = mouse_y - screen_y;
			
			switch(handle_3d_bm)
			{
				case HANDLE_3D_GRABBED_X_AXIS:
					p = vec4(1.0, 0.0, 0.0, 0.0);
					p = MultiplyVector4(&active_camera->world_to_camera_matrix, p);
					delta_x = delta_x * p.x;
					entity_TranslateEntity(&selected, vec3(delta_x, 0.0, 0.0), 1.0, 0);
					//selected.extra_data->local_position.x += delta_x;
				break;
				
				case HANDLE_3D_GRABBED_Y_AXIS:
					p = vec4(0.0, 1.0, 0.0, 0.0);
					p = MultiplyVector4(&active_camera->world_to_camera_matrix, p);
					delta_x = delta_y * p.y;
					entity_TranslateEntity(&selected, vec3(0.0, delta_y, 0.0), 1.0, 0);
					//selected.extra_data->local_position.y += delta_y;
				break;
				
				case HANDLE_3D_GRABBED_Z_AXIS:
					p = vec4(0.0, 0.0, 1.0, 0.0);
					p = MultiplyVector4(&active_camera->world_to_camera_matrix, p);
					delta_x = delta_x * p.z;
					entity_TranslateEntity(&selected, vec3(0.0, 0.0, delta_x), 1.0, 0);
					//selected.extra_data->local_position.z += delta_x;
				break;
			}
		}
	
		draw_debug_DrawOutline(selected.position_data->world_position, &selected.position_data->world_orientation, selected.draw_data->mesh, vec3(1.0, 0.3, 1.0), 2.0, 0);
		handle_3d_pos = selected.position_data->world_position;
		draw_3d_handle();
	}

}




void ginput(float delta_time)
{
	int i;
	static float pitch=0.0;
	static float yaw=0.0;
	static float f=0.0;
	static int t=0;
	int s;
	camera_t *active_camera=camera_GetActiveCamera();
	//entity_ptr eptr=entity_GetEntity("body");
	entity_ptr selected;
	general_collider_t *col;
	static entity_ptr hit;
	//general_collider_t *collider = physics_GetColliderByIndex(eptr.position_data->collider_index);
	character_controller_t *controller = (character_controller_t *)physics_GetCollider("_player_");
	mat4_t transform;
	entity_ptr e;
	//light_ptr l=light_GetLight("lightwow4");
	btTransform trm;
	btMatrix3x3 r;
	vec3_t dir = vec3(0.0, 0.0, 0.0);
	vec3_t wdir;
	vec3_t p;
	vec3_t spd;
	vec3_t fdir;
	vec3_t sdir;
	float len;
	static int intensity = draw_GetBloomParam(BLOOM_INTENSITY);
	static int small_bloom_radius = draw_GetBloomParam(BLOOM_SMALL_RADIUS);
	static int medium_bloom_radius = draw_GetBloomParam(BLOOM_MEDIUM_RADIUS);
	static int large_bloom_radius = draw_GetBloomParam(BLOOM_LARGE_RADIUS);
	static int small_bloom_iterations = draw_GetBloomParam(BLOOM_SMALL_ITERATIONS);
	static int medium_bloom_iterations = draw_GetBloomParam(BLOOM_MEDIUM_ITERATIONS);
	static int large_bloom_iterations = draw_GetBloomParam(BLOOM_LARGE_ITERATIONS);
	
	s = pew_GetPewState();
		
	if(input_GetKeyPressed(SDL_SCANCODE_UP) && !t)
	{
		intensity += 1;
		draw_SetBloomParam(BLOOM_INTENSITY, intensity);
			
		//if(intensity < 0) intensity = 0;
		t = 1;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_DOWN) && !t)
	{
		intensity -= 1;
		if(intensity < 0) intensity = 0;
		draw_SetBloomParam(BLOOM_INTENSITY, intensity);
		t = 1;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_7) && !t)
	{
		small_bloom_radius += 1;
		draw_SetBloomParam(BLOOM_SMALL_RADIUS, small_bloom_radius);
		t = 1;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_4) && !t)
	{
		small_bloom_radius -= 1;
		if(small_bloom_radius < 0) small_bloom_radius = 0;
		draw_SetBloomParam(BLOOM_SMALL_RADIUS, small_bloom_radius);
		t = 1;
	}
		
		
	if(input_GetKeyPressed(SDL_SCANCODE_8) && !t)
	{
		medium_bloom_radius += 1;
		draw_SetBloomParam(BLOOM_MEDIUM_RADIUS, medium_bloom_radius);
		t = 1;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_5) && !t)
	{
		medium_bloom_radius -= 1;
		if(medium_bloom_radius < 0) medium_bloom_radius = 0;
		draw_SetBloomParam(BLOOM_MEDIUM_RADIUS, medium_bloom_radius);
		t = 1;
	}
		
		
	if(input_GetKeyPressed(SDL_SCANCODE_9) && !t)
	{
		large_bloom_radius += 1;
		draw_SetBloomParam(BLOOM_LARGE_RADIUS, large_bloom_radius);
		t = 1;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_6) && !t)
	{
		large_bloom_radius -= 1;
		if(large_bloom_radius < 0) large_bloom_radius = 0;
		draw_SetBloomParam(BLOOM_LARGE_RADIUS, large_bloom_radius);
		t = 1;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_LEFT) && !t)
	{
		if(active_camera->exposure > 0.2)
			active_camera->exposure -= 0.1;
		//t = 1;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_RIGHT) && !t)
	{
		active_camera->exposure += 0.1;
		//t = 1;
	}
		
		
	
		
	if(!input_GetKeyPressed(SDL_SCANCODE_UP) && 
	   !input_GetKeyPressed(SDL_SCANCODE_DOWN) &&
	   !input_GetKeyPressed(SDL_SCANCODE_7) &&
	   !input_GetKeyPressed(SDL_SCANCODE_4) &&
	   !input_GetKeyPressed(SDL_SCANCODE_8) &&
	   !input_GetKeyPressed(SDL_SCANCODE_5) &&
	   !input_GetKeyPressed(SDL_SCANCODE_9) &&
	   !input_GetKeyPressed(SDL_SCANCODE_6) &&
	   !input_GetKeyPressed(SDL_SCANCODE_LEFT) &&
	   !input_GetKeyPressed(SDL_SCANCODE_RIGHT))
	{
		t=0;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_SPACE))
	{
		physics_Jump(controller);
	}
	
	/*if(input_GetKeyPressed(SDL_SCANCODE_K))
	{
		draw_Fullscreen(1);
	}
	else if(input_GetKeyPressed(SDL_SCANCODE_L))
	{
		draw_Fullscreen(0);
	}*/
		
		
	if(input_GetKeyPressed(SDL_SCANCODE_W))
	{
		//dir.floats[2] = * delta_time * 0.1;
		fdir.x = -active_camera->world_orientation.f_axis.x;
		fdir.y = -active_camera->world_orientation.f_axis.y;
		fdir.z = -active_camera->world_orientation.f_axis.z;
	}
	else if(input_GetKeyPressed(SDL_SCANCODE_S))
	{
		fdir.x = active_camera->world_orientation.f_axis.x;
		fdir.y = active_camera->world_orientation.f_axis.y;
		fdir.z = active_camera->world_orientation.f_axis.z;
	}
	else
	{
		fdir.x = 0.0;
		fdir.y = 0.0;
		fdir.z = 0.0;
	}
	
	if(input_GetKeyPressed(SDL_SCANCODE_A))
	{
		//dir.floats[2] = * delta_time * 0.1;
		sdir.x = -active_camera->world_orientation.r_axis.x;
		sdir.y = -active_camera->world_orientation.r_axis.y;
		sdir.z = -active_camera->world_orientation.r_axis.z;
	}
	else if(input_GetKeyPressed(SDL_SCANCODE_D))
	{
		sdir.x = active_camera->world_orientation.r_axis.x;
		sdir.y = active_camera->world_orientation.r_axis.y;
		sdir.z = active_camera->world_orientation.r_axis.z;
	}
	else
	{
		sdir.x = 0.0;
		sdir.y = 0.0;
		sdir.z = 0.0;
	}
	
	fdir.x += sdir.x;
	fdir.y += sdir.y;
	fdir.z += sdir.z;	
	/*if(input_GetKeyPressed(SDL_SCANCODE_S))
	{
		dir.floats[2] = 1.0 * delta_time * 0.1;
	}
	else
	{
		dir.floats[2] = 0.0;
	}
		
	if(input_GetKeyPressed(SDL_SCANCODE_A))
	{
		dir.floats[0] = -1.0 * delta_time * 0.1;
	}
	else if(input_GetKeyPressed(SDL_SCANCODE_D))
	{
		dir.floats[0] = 1.0 * delta_time * 0.1;
	}
	else 
	{
		dir.floats[0] = 0.0;
	}*/

		
	pitch+=input.mouse_dy;
		
	if(pitch<-0.5)pitch=-0.5;
	else if(pitch > 0.5)pitch=0.5;
		
	yaw+=input.mouse_dx;
	
	
	//dir = MultiplyVector3(&active_camera->world_orientation, dir);
	
	camera_TranslateCamera(active_camera, fdir, delta_time * 0.01, 0.0);
	camera_ApplyPitchYawToCamera(active_camera, -yaw, pitch, vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0));
	
	//printf("%f\n", yaw);
	//physics_Yaw(controller, yaw);
	//entity_RotateEntity(&eptr, vec3(0.0, 1.0, 0.0), -input.mouse_dx, 0);
	//camera_RotateCamera(active_camera, vec3(1.0, 0.0, 0.0), -pitch, 1);
/*	r = controller->base.rigid_body->getCenterOfMassTransform().getBasis();
	p.x = r[0][0];
	p.y = r[0][1];
	p.z = r[0][2];
	
	wdir.x = p.x * dir.x + p.y * dir.y + p.z * dir.z;
	wdir.y = 0.0;
	
	p.x = r[2][0];
	p.y = r[2][1];
	p.z = r[2][2];
	
	wdir.z = p.x * dir.x + p.y * dir.y + p.z * dir.z;*/
		
	//dir = MultiplyVector3(&eptr.extra_data->local_orientation, dir);
		
/*	physics_Move(controller, wdir);
	
	p = add3(active_camera->world_position, mul3(active_camera->world_orientation.f_axis, -3.5));
	
	if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
		hit = entity_RayCast(active_camera->world_position, p);
	}*/
	
	/*if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_CLICKED)
	{
		if(hit.position_data)
		{
			
			
			if(input_GetMouseButton(SDL_BUTTON_RIGHT) & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
			{
				entity_ApplyForce(&hit, mul3(active_camera->world_orientation.f_axis, -800.0), vec3(0.0, 0.0, 0.0));
				hit.position_data = NULL;
			}
			else
			{
				col = physics_GetColliderByIndex(hit.position_data->collider_index);
				btVector3 tspd = col->base.rigid_body->getLinearVelocity(); 
				spd = vec3(tspd[0], tspd[1], tspd[2]);
				dir = sub3(p, hit.position_data->world_position);
				len = length3(dir);
				dir = sub3(mul3(dir, (250.0 + col->base.mass * 0.5)), mul3(spd, 18.0 + col->base.mass));
				entity_ApplyForce(&hit, dir, vec3(0.0, 0.0, 0.0));
			}

		}
	}
	else
	{
		hit.position_data = NULL;
	}*/
	
}


void draw_3d_handle()
{
	if(selected.extra_data)
	{
		draw_debug_DrawLine(handle_3d_pos, add3(handle_3d_pos, vec3(1.0, 0.0, 0.0)), vec3(1.0, 0.0, 0.0), 4.0, 0, 0);
		draw_debug_DrawLine(handle_3d_pos, add3(handle_3d_pos, vec3(0.0, 1.0, 0.0)), vec3(0.0, 1.0, 0.0), 4.0, 0, 0);
		draw_debug_DrawLine(handle_3d_pos, add3(handle_3d_pos, vec3(0.0, 0.0, 1.0)), vec3(0.0, 0.0, 1.0), 4.0, 0, 0);
	}
}

void check_3d_handle()
{
	
	handle_3d_bm = 0;
	
	camera_t *active_camera = camera_GetActiveCamera();
	framebuffer_t *f = cur_fb;
	framebuffer_BindFramebuffer(&picking_buffer);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	
	float pixel[4];


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);

	glLineWidth(16.0);
	
	glUseProgram(0);
	
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(handle_3d_pos.x, handle_3d_pos.y, handle_3d_pos.z);
	glVertex3f(handle_3d_pos.x + 1.0, handle_3d_pos.y, handle_3d_pos.z);
	
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(handle_3d_pos.x, handle_3d_pos.y, handle_3d_pos.z);
	glVertex3f(handle_3d_pos.x, handle_3d_pos.y + 1.0, handle_3d_pos.z);
	
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(handle_3d_pos.x, handle_3d_pos.y, handle_3d_pos.z);
	glVertex3f(handle_3d_pos.x, handle_3d_pos.y, handle_3d_pos.z + 1.0);
	glEnd();
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, picking_buffer.id);
	
	glReadPixels(input.mouse_x, input.mouse_y, 1, 1, GL_RGB, GL_FLOAT, pixel);
	if(pixel[0] == 1.0)
	{
		handle_3d_bm |= HANDLE_3D_GRABBED_X_AXIS;
		printf("x axis\n");
	}
	else if(pixel[1] == 1.0)
	{
		handle_3d_bm |= HANDLE_3D_GRABBED_Y_AXIS;
		printf("y axis\n");
	}
	else if(pixel[2] == 1.0)
	{
		handle_3d_bm |= HANDLE_3D_GRABBED_Z_AXIS;
		printf("z axis\n");
	}

	glLineWidth(1.0);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	framebuffer_BindFramebuffer(f);

}

void widget_cb(swidget_t *sub_widget, void *data)
{
	
	//printf("clicked on subwidget %s!\n", sub_widget->name);
}

void enable_shadow_mapping(swidget_t *sub_widget, void *data)
{
	wbutton_t *button = (wbutton_t *)sub_widget;
	if(button->button_flags & BUTTON_CHECK_BOX_CHECKED)
	{
		draw_SetRenderFlags(renderer.renderer_flags | RENDERFLAG_USE_SHADOW_MAPS);
	}
	else
	{
		draw_SetRenderFlags(renderer.renderer_flags & (~RENDERFLAG_USE_SHADOW_MAPS));
	}
}

void enable_volumetric_lights(swidget_t *sub_widget, void *data)
{
	wbutton_t *button = (wbutton_t *)sub_widget;
	if(button->button_flags & BUTTON_CHECK_BOX_CHECKED)
	{
		draw_SetRenderFlags(renderer.renderer_flags | RENDERFLAG_DRAW_LIGHT_VOLUMES);
	}
	else
	{
		draw_SetRenderFlags(renderer.renderer_flags & (~RENDERFLAG_DRAW_LIGHT_VOLUMES));
	}
}

void exit_engine(swidget_t *sub_widget, void *data)
{
	pew_Exit();
}

void ginit()
{
	material_t ma;
	int b;
	//collider_t *collider;
	/*mesh_t teapot;
	mesh_t monkey;
	mesh_t sphere;
	mesh_t plane;
	mesh_t cube;
	mesh_t stencil;
	mesh_t stencil2;
	mesh_t holes;*/
	static int p;
	tex_info_t tif;
	
//	TwInit(TW_OPENGL, NULL);
	//TwWindowSize(renderer.screen_width, renderer.screen_height);
	//TwBar *bar = TwNewBar("test");
	
	
	//typedef enum {BAZ, BAR, FOO} test_enum;
	//test_enum tt = BAZ;
	
	//TwEnumVal tests[] = {{BAZ, "BAZ"}, {BAR, "BAR"}, {FOO, "FOO"}};
	
	//TwType test_type = TwDefineEnum("test_enum", tests, 3);
	
	//TwAddVarRW(bar, "test_enum", test_type, &tt, NULL);
	
	//TwAddButton(bar, "button", cb, NULL, "key=c");
	//TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with SDL and OpenGL.\nPress [Space] to toggle fullscreen.' ");
	
	//TwAddVarRO(bar, "frame", TW_TYPE_INT32, &renderer.frame_count, NULL);
               
    //TwAddVarRW(bar, "count", TW_TYPE_INT32, &p, " min=1 max=100 keyIncr=c keyDecr=f ");           
	
	mesh_t *sphereptr;
	mesh_t *planeptr;
	mesh_t *cubeptr;
	mesh_t *stencilptr;
	mesh_t *stencil2ptr;
	mesh_t *holesptr;
	mesh_t *mesh;
	
	entity_t e;
	entity_ptr eptr;
	entity_ptr eptr2;
	hook_t hook;
	camera_t *cptr;
	vec3_t in;
	vec3_t out;
	mat3_t id = mat3_t_id();
	
	
	//light_position_data pdata;
	//light_params lparams;
	light_ptr lptr;
	bone_t *parent;
	bone_t *child;
	bone_t *child_child;
	bone_t *child_child_child;
	bone_t *child_child_child_child;
	armature_t *armature;
	armdef_t *ad;
	int armature_index;
	int entity_index;
	int animation;
	int set_index;
	
	text_LoadFont("consola.ttf", "consola_16", 16);
	text_LoadFont("consola.ttf", "consola_12", 12);

	id = mat3_t_id();

	
	model_LoadModel("CubeUV2.obj", "cubeUV");
	model_LoadModel("ico_uv.obj", "ico");
	model_LoadModel("pole.obj", "pole");
	model_LoadModel("bus_stop.obj", "bus_stop");
//	model_LoadModel("wheel.obj", "wheel");
//	model_LoadModel("pew_plane.obj", "pew_plane");
	model_LoadModel("BigPlane.obj", "plane");
	//model_LoadModel("stairs.obj", "stairs");
	//model_LoadModel("plat2.obj", "plat2");

	
	texture_LoadTexture("tile_d.png", "tile_d", 0);
	texture_LoadTexture("tile_n.png", "tile_n", 0);
	
	texture_LoadTexture("dungeon-stone1-albedo2.png", "dungeon_diffuse", 0);
	texture_LoadTexture("dungeon-stone1-normal.png", "dungeon_normal", 0);
	texture_LoadTexture("dungeon-stone1-roughness.png", "dungeon_gloss", 0);
	texture_LoadTexture("dungeon-stone1-metalness.png", "dungeon_metallic", 0);
	texture_LoadTexture("dungeon-stone1-height.png", "dungeon_height", 0);
	
	texture_LoadTexture("greasy-pan-2-albedo.png", "greasy_diffuse", 0);
	texture_LoadTexture("greasy-pan-2-normal.png", "greasy_normal", 0);
	texture_LoadTexture("greasy-pan-2-roughness.png", "greasy_gloss", 0);
	texture_LoadTexture("greasy-pan-2-metal.png", "greasy_metallic", 0);
	
	texture_LoadTexture("iron-rusted4-basecolor.png", "iron_rusted_diffuse", 0);
	texture_LoadTexture("iron-rusted4-normal.png", "iron_rusted_normal", 0);
	texture_LoadTexture("iron-rusted4-roughness.png", "iron_rusted_gloss", 0);
	texture_LoadTexture("iron-rusted4-metalness.png", "iron_rusted_metallic", 0);
	
	texture_LoadTexture("TexturesCom_BrushedStainless_1024_albedo.png", "brushed_metal_diffuse", 0);
	texture_LoadTexture("TexturesCom_BrushedStainless_1024_normal.png", "brushed_metal_normal", 0);
	texture_LoadTexture("TexturesCom_BrushedStainless_1024_roughness.png", "brushed_metal_gloss", 0);
	texture_LoadTexture("TexturesCom_BrushedStainless_1024_metallic.png", "brushed_metal_metallic", 0);
	
	texture_LoadTexture("TexturesCom_PaintedMetal_1024_albedo.png", "painted_metal_diffuse", 0);
	texture_LoadTexture("TexturesCom_PaintedMetal_1024_normal.png", "painted_metal_normal", 0);
	texture_LoadTexture("TexturesCom_PaintedMetal_1024_roughness.png", "painted_metal_gloss", 0);
	texture_LoadTexture("TexturesCom_PaintedMetal_1024_metallic.png", "painted_metal_metallic", 0);
	
	texture_LoadTexture("TexturesCom_TuftedLeather_1024_albedo.png", "tufted_leather_diffuse", 0);
	texture_LoadTexture("TexturesCom_TuftedLeather_1024_normal.png", "tufted_leather_normal", 0);
	texture_LoadTexture("TexturesCom_TuftedLeather_1024_roughness.png", "tufted_leather_gloss", 0);
	texture_LoadTexture("TexturesCom_TuftedLeather_1024_height.png", "tufted_leather_height", 0);
	
	texture_LoadTexture("oakfloor_basecolor.png", "oakfloor_diffuse", 0);
	texture_LoadTexture("oakfloor_normal.png", "oakfloor_normal", 0);
	texture_LoadTexture("oakfloor_roughness.png", "oakfloor_gloss", 0);
	//texture_LoadTexture("harsh_metalness.png", "oakfloor_mettalic", 0);
	//texture_LoadTexture("oakfloor_Height.png", "oakfloor_height", 0);
	
	/*texture_LoadTexture("scifi tile 1_COLOR.png", "scifi_d", 0);
	texture_LoadTexture("scifi tile 1_NRM.png", "scifi_n", 0);
	texture_LoadTexture("scifi tile 1_SPEC.png", "scifi_s", 0);*/
	
	//texture_LoadTexture("Pilip.tga", "pilip", 0);
	
	//texture_LoadTexture("pew_logo.png", "pew", 0);
	
	//text_LoadFont("fixedsys.ttf", "fixedsys", FONT_SIZE_8);
	
	
	//armature = armature_GetArmature("Bone");
	//animation = armature_GetAnimationIndex("unnamed_animation.0000");
	
	//if(armature && animation > -1)
	//{
	//	armature_PlayAnimation(armature, animation);
	//}
	
	/*int *indexes=malloc(sizeof(int)*3*6);*/
	int i;
	int j;
	int k;
	//teapot=model_GetMesh("teapot", 0);
	//monkey=model_GetMesh("monkey", 0);
	/*sphere=model_GetMesh("sphere", 0);
	plane=model_GetMesh("plane", 0);
	cube=model_GetMesh("cube", 0);
	stencil=model_GetMesh("stencil", 0);
	stencil2=model_GetMesh("stencil2", 0);
	holes=model_GetMesh("holes", 0);*/
	
	
	sphereptr = model_GetMeshPtr("sphere");
	planeptr = model_GetMeshPtr("plane");
//	cubeptr = model_GetMeshPtr("cube");
//	stencilptr = model_GetMeshPtr("stencil");
//	stencil2ptr = model_GetMeshPtr("stencil2");
//	holesptr = model_GetMeshPtr("holes");
	
	
	

	
	widget_t *w = gui_CreateWidget("test0", WIDGET_HEADER|WIDGET_GRABBABLE|WIDGET_MOVABLE|WIDGET_TRANSLUCENT|WIDGET_HIGHTLIGHT_BORDERS, 400.0, -200.0, 320, 240, 0.3, 0.3, 0.3, 0.7, WIDGET_NO_TEXTURE, 0);
	gui_AddButton(w, "Enable shadow mapping", WIDGET_LOCK_Y_SCALE | WIDGET_KEEP_RELATIVE_X_POSITION, BUTTON_CHECK_BOX, 0, 0, 290.0, 15.0, 1.0, 1.0, 1.0, 1.0, enable_shadow_mapping);
	gui_AddButton(w, "Enable volumetric lights", WIDGET_LOCK_Y_SCALE | WIDGET_KEEP_RELATIVE_X_POSITION, BUTTON_CHECK_BOX, 0, -20, 290.0, 15.0, 1.0, 1.0, 1.0, 1.0, enable_volumetric_lights);
	gui_AddButton(w, "Exit", WIDGET_LOCK_Y_SCALE | WIDGET_KEEP_RELATIVE_X_POSITION, 0, 0, 20, 290.0, 15.0, 1.0, 1.0, 1.0, 1.0, exit_engine);
	
	
	//w = gui_CreateWidget("test1", WIDGET_HEADER|WIDGET_GRABBABLE|WIDGET_MOVABLE|WIDGET_TRANSLUCENT|WIDGET_HIGHTLIGHT_BORDERS, 50.0, -2.0, 320, 240, 0.8, 0.3, 0.3, 0.7, WIDGET_NO_TEXTURE, 0);
	//gui_AddButton(w, "button0", WIDGET_LOCK_Y_SCALE | WIDGET_KEEP_RELATIVE_X_POSITION, BUTTON_CHECK_BOX, 0, 0, 290.0, 15.0, 1.0, 1.0, 1.0, 1.0);
	//gui_AddButton(w, "button1", WIDGET_LOCK_Y_SCALE | WIDGET_KEEP_RELATIVE_X_POSITION, BUTTON_CHECK_BOX, 0, -20, 290.0, 15.0, 1.0, 1.0, 1.0, 1.0);
	//gui_AddButton(w, "button2", WIDGET_LOCK_Y_SCALE | WIDGET_KEEP_RELATIVE_X_POSITION, BUTTON_CHECK_BOX, 0, 20, 290.0, 15.0, 1.0, 1.0, 1.0, 1.0);
	//gui_CreateWidget("test1", WIDGET_HEADER|WIDGET_GRABBABLE|WIDGET_MOVABLE|WIDGET_TRANSLUCENT, 0.0, 0.0, 320, 240, 0.5, 0.3, 0.2, 0.7, WIDGET_NO_TEXTURE, 0);
	
	//gui_CreateWidget("test2", WIDGET_HEADER|WIDGET_GRABBABLE|WIDGET_MOVABLE|WIDGET_TRANSLUCENT|WIDGET_HIGHTLIGHT_BORDERS, -10.0, 0.0, 640.0, 240.0, 0.5, 0.5, 1.0, 1.0, WIDGET_NO_TEXTURE, 0);
	//gui_AddSubWidget(test, 0, WIDGET_BUTTON, "test_button", -0.8, 0.0, 0.5, 0.3, 0.0, 0.0, 0.5, 0.0, 0.0, 0.9, texture_GetTextureID("pew"), test, (void *)gui_test_CloseWidget);
	//gui_AddSubWidget(test, 0, WIDGET_BUTTON, "console_button", 0.8, 0.0, 0.5, 0.3, 0.0, 0.0, 0.0, 0.5, 0.0, 1.0, WIDGET_NO_TEXTURE, test, (void *)gui_test_CloseConsole);
	//gui_AddSubWidget(test, 0, WIDGET_VERTICAL_SCROLLER, "test_vertical_scroller", 1.0, 0.0, 0.0, 1.0, 0.5, -0.5, 0.2, 0.2, 0.2, 1.0, WIDGET_NO_TEXTURE, test, NULL);
	
	
	//gui_AddSubWidget(serious_test, 0, WIDGET_BUTTON, "toggle volumetric lights", -0.5, -0.2, 0.3, 0.3, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, WIDGET_NO_TEXTURE, serious_test, (void *)gui_test_ToggleVolumetricLights);
	//gui_AddSubWidget(serious_test, 0, WIDGET_BUTTON, "toggle shadows", 0.0, -0.2, 0.3, 0.3, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, WIDGET_NO_TEXTURE, serious_test, (void *)gui_test_ToggleShadows);
	//gui_AddSubWidget(serious_test, 0, WIDGET_BUTTON, "toggle bloom", 0.5, -0.2, 0.3, 0.3, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, WIDGET_NO_TEXTURE, serious_test, (void *)gui_test_ToggleBloom);
	
	
/*	material_SetMaterialDiffuseColor(&ma, 1.0, 1.0, 1.0 , 1.0);
	
	ma.diff_mult.r=1;
	ma.diff_mult.g=1;
	ma.diff_mult.b=1;
	ma.diff_mult.a=1;*/
	
	
	
	//ma.texture=-1;
	//ma.name="red";
	//ma.shader_index=shader_GetShaderIndex("lit");
	//ma.bm_flags=0;
	
	//printf("%d\n", ma.shader_index);
	//material_CreateMaterialFromData(&ma);
	//tif.diff_tex_count = 1;
	tif.diff_tex = (short)texture_GetTextureIndex("rock4_d");
	tif.norm_tex = (short)texture_GetTextureIndex("rock4_n");
	tif.gloss_tex = -1;
	tif.met_tex = -1;
	//tif.spec_tex = (short)texture_GetTextureIndex("rock4_d");
	tif.heig_tex = (short)texture_GetTextureIndex("rock4_h");
	
	material_CreateMaterial("red", 0.5, 0.0, vec4(1.0, 0.0, 0.0, 1.0), 0.0, 0, NULL);
	material_CreateMaterial("green", 0.5, 0.0, vec4(0.0, 1.0, 0.0, 1.0), 0.0, 0, NULL);
	material_CreateMaterial("blue", 0.5, 0.0, vec4(0.0, 0.0, 1.0, 1.0), 0.0, 0, NULL);
	material_CreateMaterial("white", 0.5, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 0.0, 0, NULL);
	
	//material_CreateMaterial("green", 0.5, 0.0, vec4(0.0, 1.0, 0.0, 1.0), 0.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_HeightTexture, &tif);
	
	
	//tif.diff_tex = (short)texture_GetTextureIndex("scifi_d");
	//tif.norm_tex = (short)texture_GetTextureIndex("scifi_n");
	//tif.spec_tex = (short)texture_GetTextureIndex("scifi_s");
	
	//material_CreateMaterial("test", 32, 0.9, 0.9, 0.9, 1.0, 0.1, 0.1 ,0.1, 0.5, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_SpecularTexture, &tif);

	tif.diff_tex = (short)texture_GetTextureIndex("greasy_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("greasy_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("greasy_gloss");
	tif.met_tex = (short)texture_GetTextureIndex("greasy_metallic");
	material_CreateMaterial("greasy", 0.5, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 0.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture|MATERIAL_MetallicTexture, &tif);
	
	
	tif.diff_tex = (short)texture_GetTextureIndex("iron_rusted_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("iron_rusted_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("iron_rusted_gloss");
	tif.met_tex = (short)texture_GetTextureIndex("iron_rusted_metallic");
	material_CreateMaterial("iron_rusted", 0.5, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 0.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture|MATERIAL_MetallicTexture, &tif);
	
	
	tif.diff_tex = (short)texture_GetTextureIndex("painted_metal_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("painted_metal_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("painted_metal_gloss");
	tif.met_tex = (short)texture_GetTextureIndex("painted_metal_metallic");
	material_CreateMaterial("painted_metal", 0.5, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 0.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture|MATERIAL_MetallicTexture, &tif);
	
	
	tif.diff_tex = (short)texture_GetTextureIndex("brushed_metal_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("brushed_metal_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("brushed_metal_gloss");
	tif.met_tex = (short)texture_GetTextureIndex("brushed_metal_metallic");
	material_CreateMaterial("brushed_metal", 0.5, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 0.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture|MATERIAL_MetallicTexture, &tif);
	
	tif.diff_tex = (short)texture_GetTextureIndex("tufted_leather_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("tufted_leather_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("tufted_leather_gloss");
	tif.heig_tex = (short)texture_GetTextureIndex("tufted_leather_height");
	material_CreateMaterial("tufted_leather", 0.5, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 0.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture|MATERIAL_HeightTexture, &tif);
	
	
	tif.diff_tex = (short)texture_GetTextureIndex("dungeon_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("dungeon_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("dungeon_gloss");
	tif.heig_tex = (short)texture_GetTextureIndex("dungeon_height");
	tif.heig_tex = (short)texture_GetTextureIndex("dungeon_metallic");
	material_CreateMaterial("dungeon", 0.5, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 0.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture|MATERIAL_HeightTexture|MATERIAL_MetallicTexture, &tif);
	
	
	tif.diff_tex = (short)texture_GetTextureIndex("oakfloor_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("oakfloor_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("oakfloor_gloss");
	material_CreateMaterial("oakfloor", 0.5, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 1.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture, &tif);
	
	tif.diff_tex = (short)texture_GetTextureIndex("tile_d");
	tif.norm_tex = (short)texture_GetTextureIndex("tile_n");
	material_CreateMaterial("tile", 0.7, 0.0, vec4(1.0, 1.0, 1.0, 1.0), 1.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture, &tif);
	
	/*tif.diff_tex = (short)texture_GetTextureIndex("brick_d");
	tif.norm_tex = (short)texture_GetTextureIndex("brick_n");
	
	material_CreateMaterial("translucent1", 512, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, MATERIAL_Translucent, &tif);
	material_CreateMaterial("translucent2", 512, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, MATERIAL_Translucent, NULL);
	material_CreateMaterial("translucent3", 512, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, MATERIAL_Translucent, NULL);*/
	
	//tif.diff_tex = (short) texture_GetTextureIndex("skydome");
	//material_CreateMaterial("skydome", 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ,0.0, 0.0, MATERIAL_DiffuseTexture|MATERIAL_Shadeless, &tif);
	
	//tif.diff_tex = (short) texture_GetTextureIndex("pew");
	//material_CreateMaterial("pew", 512, 1.0, 1.0, 1.0, 0.9, 1.0, 1.0, 1.0, 0.0, MATERIAL_FrontAndBack|MATERIAL_Translucent, &tif);

	
	//camera_CreateCamera("camera0", vec3(0.0, 1.5, 0.0), &id, 0.68, (float) renderer.width, (float) renderer.height, 0.1, 1000.0);
	
	//mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), 1.0, 1);
	
	//camera_SetCameraByIndex(camera_CreateCamera("camera0", vec3(0.0, 0.5, 0.0), &id, 0.68, (float) renderer.width, (float) renderer.height, 0.1, 1000.0));
	
	//mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), -0.5, 1);
	//entity_CreateEntity("piramid", ENTITY_DYNAMIC, 0, vec3(0.0, 1.0, 3.0), &id, 2.0 ,model_GetMeshPtr("piramid"), material_GetMaterialIndex("red"), 1);
	
	id = mat3_t_id();
	
	entity_CreateEntityDef("plane", ENTITY_COLLIDES|ENTITY_STATIC_COLLISION, material_GetMaterialIndex("oakfloor"), -1, planeptr, 0.0, COLLISION_SHAPE_CONVEX_HULL);
	entity_CreateEntityDef("wall", ENTITY_COLLIDES|ENTITY_STATIC_COLLISION, material_GetMaterialIndex("tile"), -1, planeptr, 0.0, COLLISION_SHAPE_CONVEX_HULL);
	entity_CreateEntityDef("cieling", ENTITY_COLLIDES|ENTITY_STATIC_COLLISION, material_GetMaterialIndex("dungeon"), -1, planeptr, 0.0, COLLISION_SHAPE_CONVEX_HULL);
	
	
	//entity_CreateEntityDef("wheel", 0, material_GetMaterialIndex("red"), -1, model_GetMeshPtr("wheel"), 0.0, 0);
	//entity_CreateEntityDef("pew_plane", ENTITY_COLLIDES|ENTITY_STATIC_COLLISION, material_GetMaterialIndex("translucent1"), -1, model_GetMeshPtr("pew_plane"), 0.0, COLLISION_SHAPE_CONVEX_HULL);
	//entity_CreateEntityDef("piramid", ENTITY_DYNAMIC, ENTITY_COLLIDES, material_GetMaterialIndex("red"), -1, model_GetMeshPtr("piramid"), 2.0, COLLISION_SHAPE_CONVEX_HULL);
	//entity_CreateEntityDef("stairs", ENTITY_COLLIDES|ENTITY_STATIC_COLLISION, material_GetMaterialIndex("red"), -1, model_GetMeshPtr("stairs"), 1.0, COLLISION_SHAPE_CONVEX_HULL);
	entity_CreateEntityDef("ico_brushed_metal", ENTITY_COLLIDES, material_GetMaterialIndex("brushed_metal"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_painted_metal", ENTITY_COLLIDES, material_GetMaterialIndex("painted_metal"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_tufted_leather", ENTITY_COLLIDES, material_GetMaterialIndex("tufted_leather"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	
	entity_CreateEntityDef("pole", 0, material_GetMaterialIndex("white"), -1, model_GetMeshPtr("pole"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("bus_stop", 0, material_GetMaterialIndex("white"), -1, model_GetMeshPtr("bus_stop"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("cube", ENTITY_COLLIDES, material_GetMaterialIndex("white"), -1, model_GetMeshPtr("cubeUV"), 2.0, COLLISION_SHAPE_SPHERE);
	//entity_CreateEntityDef("cube_iron_rusted", ENTITY_COLLIDES, material_GetMaterialIndex("iron_rusted"), -1, model_GetMeshPtr("cubeUV"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_red", ENTITY_COLLIDES, material_GetMaterialIndex("translucent1"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_green", ENTITY_COLLIDES, material_GetMaterialIndex("translucent2"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_blue", ENTITY_COLLIDES, material_GetMaterialIndex("translucent3"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	//entity_CreateEntityDef("spiral", ENTITY_STATIC, ENTITY_COLLIDES, material_GetMaterialIndex("red"), -1, model_GetMeshPtr("spiral"), 0.0, COLLISION_SHAPE_CONVEX_HULL);
	//entity_CreateEntityDef("rigged", 0, material_GetMaterialIndex("red"), 0, model_GetMeshPtr("rig"), 0.0, COLLISION_SHAPE_CONVEX_HULL);
	
	entity_def_t *def0;
	entity_def_t *def1;
	entity_def_t *def2;
	//entity_def_t *def = entity_GetEntityDef("piramid");
	//entity_SpawnEntity("piramid", def, vec3(0.0, 0.0, 0.0), &id);
	def0 = entity_GetEntityDef("plane");
	entity_SpawnEntity("plane", def0, vec3(0.0, -6.0, 0.0), &id);
	
	def0 = entity_GetEntityDef("cube");
	id = mat3_t_id();
	//mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), -0.5, 1);
	entity_SpawnEntity("cube", def0, vec3(0.0, -5.0, 0.0), &id);
	/*mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), 0.5, 1);
	entity_SpawnEntity("wall1", def0, vec3(0.0, 0.0, -15.0), &id);
	mat3_t_rotate(&id, vec3(0.0, 0.0, 1.0), 0.5, 1);
	entity_SpawnEntity("wall2", def0, vec3(15.0, 0.0, 0.0), &id);
	mat3_t_rotate(&id, vec3(0.0, 0.0, 1.0), -0.5, 1);
	entity_SpawnEntity("wall3", def0, vec3(-15.0, 0.0, 0.0), &id);
	
	
	def0 = entity_GetEntityDef("cieling");
	mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), 1.0, 1);
	entity_SpawnEntity("cieling", def0, vec3(0.0, 6.0, 0.0), &id);*/
	
	
	//entity_SpawnEntity("plane2", def, vec3(0.0, -6.0, 100.0), &id);
	//def = entity_GetEntityDef("stairs");
	
	/*id = mat3_t_id();
	def0 = entity_GetEntityDef("ico_brushed_metal");
	entity_SpawnEntity("ico_brushed_metal", def0, vec3(3.0, 0.0, 0.0), &id);
	
	def1 = entity_GetEntityDef("ico_painted_metal");
	entity_SpawnEntity("ico_painted_metal", def1, vec3(-3.0, 0.0, 0.0), &id);
	
	def2 = entity_GetEntityDef("ico_tufted_leather");
	entity_SpawnEntity("ico_tufted_leather", def2, vec3(-5.0, 0.0, 0.0), &id);
	
	
	for(i = 0; i < 10; i++)
	{
		entity_SpawnEntity("ico_brushed_metal", def0, vec3(4.0, 0.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_painted_metal", def1, vec3(0.0, 0.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_tufted_leather", def2, vec3(-4.0, 0.0, -5.0 + i), &id);
	}
	
	for(i = 0; i < 10; i++)
	{
		entity_SpawnEntity("ico_brushed_metal", def0, vec3(4.0, 2.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_painted_metal", def1, vec3(0.0, 2.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_tufted_leather", def2, vec3(-4.0, 2.0, -5.0 + i), &id);
	}
	
	for(i = 0; i < 10; i++)
	{
		entity_SpawnEntity("ico_brushed_metal", def0, vec3(4.0, 4.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_painted_metal", def1, vec3(0.0, 4.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_tufted_leather", def2, vec3(-4.0, 4.0, -5.0 + i), &id);
	}
	
	for(i = 0; i < 10; i++)
	{
		entity_SpawnEntity("ico_brushed_metal", def0, vec3(4.0, 6.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_painted_metal", def1, vec3(0.0, 6.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_tufted_leather", def2, vec3(-4.0, 6.0, -5.0 + i), &id);
	}
	
	for(i = 0; i < 10; i++)
	{
		entity_SpawnEntity("ico_brushed_metal", def0, vec3(4.0, 8.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_painted_metal", def1, vec3(0.0, 8.0, -5.0 + i), &id);
		entity_SpawnEntity("ico_tufted_leather", def2, vec3(-4.0, 8.0, -5.0 + i), &id);
	}


	mat3_t_rotate(&id, vec3(0.0, 1.0, 0.0), 0.5, 1);
	def2 = entity_GetEntityDef("pole");
	entity_SpawnEntity("pole", def2, vec3(8.0, -8.0, 6.0), &id);
	
	mat3_t_rotate(&id, vec3(0.0, 1.0, 0.0), -0.5, 1);
	def2 = entity_GetEntityDef("bus_stop");
	entity_SpawnEntity("bus_stop", def2, vec3(8.0, -3.2, 0.0), &id);*/
	
	
	id = mat3_t_id();
	
	for(i=0; i<1; i++)
	{	
		mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), -0.5, 1);
		//light_CreateSpotLight("spot", LIGHT_GENERATE_SHADOWS|LIGHT_DRAW_VOLUME, vec4(4.5, 4.8, 6.0, 1.0), &id, vec3(0.8, 0.6, 0.2), 35.0, 10.0, 45.0, 0.5, 0.01, 0.005, 0.01, 4, 1024, -1);
		light_CreatePointLight("lightwow0", LIGHT_GENERATE_SHADOWS, vec4(0.0, 0.0, 10.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 35.0, 10.0, 0.02, 0.01, 0.01, 4, 256);
		light_CreatePointLight("lightwow1", LIGHT_GENERATE_SHADOWS, vec4(0.0, 0.0, -10.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 35.0, 10.0, 0.02, 0.01, 0.01, 4, 256);
		light_CreatePointLight("lightwow2", LIGHT_GENERATE_SHADOWS, vec4(10.0, 0.0, 0.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 35.0, 10.0, 0.02, 0.01, 0.01, 4, 256);
		light_CreatePointLight("lightwow3", LIGHT_GENERATE_SHADOWS, vec4(-10.0, 0.0, 0.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 35.0, 10.0, 0.02, 0.01, 0.01, 4, 256);
		//light_CreatePointLight("lightwow1", LIGHT_GENERATE_SHADOWS, vec4(-10.0, -2.0, 0.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 15.0, 10.0, 0.02, 0.01, 0.01, 4, 256);
		//light_CreatePointLight("lightwow2", LIGHT_GENERATE_SHADOWS, vec4(0.0, -2.0, 10.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 15.0, 10.0, 0.02, 0.01, 0.01, 4, 256);
		//light_CreatePointLight("lightwow3", LIGHT_GENERATE_SHADOWS, vec4(0.0, -2.0, -10.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 15.0, 10.0, 0.02, 0.01, 0.01, 4, 256);
		//light_CreatePointLight("lightwow4", LIGHT_GENERATE_SHADOWS, vec4(0.0, -2.0, 0.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 15.0, 10.0, 0.02, 0.01, 0.01, 4, 256);
	}
	
	
	//id = mat3_t_id();
	//cptr=camera_GetActiveCamera();
	//camera_TranslateCamera(cptr, vec3(0.0, 1.0 ,0.0), 1.7, 1);
	//int col_index = physics_CreateCollider("_player_", COLLIDER_CHARACTER_CONTROLLER, COLLISION_SHAPE_CAPSULE, COLLIDER_CREATE_SCENEGRAPH_NODE, -1, 0.0, 0.0, 2.0, 0.5, 5.0, 14.7, 50.0, NULL, vec3(0.0, 0.0, 0.0), &id, NULL);
	//general_collider_t *c = physics_GetColliderByIndex(col_index);
	//scenegraph_SetParent(cptr->assigned_node, c->base.assigned_node, 0);
	

	pew_SetTimeScale(1.0);

}

void gpause()
{
	pew_SetPewState(PEW_PAUSED);
}

void gresume()
{
	pew_SetPewState(PEW_PLAYING);
}

void cache_test()
{
	mat3_t m;
	mat3_t n;
	int i;
	for(i=0; i<10000000; i++)
	{
		m.floats[0][0]=m.floats[0][1];
		m.floats[0][1]=m.floats[0][2];
		m.floats[0][2]=m.floats[1][0];
		
		m.floats[1][0]=m.floats[1][1];
		m.floats[1][1]=m.floats[1][2];
		m.floats[1][2]=m.floats[2][0];
		
		m.floats[2][0]=m.floats[2][1];
		m.floats[2][1]=m.floats[2][2];
		m.floats[2][2]=1.0;
		
		
		n.floats[0][0]=m.floats[0][1];
		n.floats[0][1]=m.floats[0][2];
		n.floats[0][2]=m.floats[1][0];
		
		n.floats[1][0]=m.floats[1][1];
		n.floats[1][1]=m.floats[1][2];
		n.floats[1][2]=m.floats[2][0];
		
		n.floats[2][0]=m.floats[2][1];
		n.floats[2][1]=m.floats[2][2];
		n.floats[2][2]=1.0;
	}
}


int main(int argc, char *argv[])
{
	//getchar();
	
	
	int res = RENDERER_1366x768;
	int mode = INIT_WINDOWED;
	int i = 1;
	
	if(argc > 1)
	{
		while(i < argc)
		{
			if(!strcmp(argv[i], "-r"))
			{
				i++;
				if(!strcmp(argv[i], "800x600"))
				{
					res = RENDERER_800x600;
				}
				
				else if(!strcmp(argv[i], "1366x768"))
				{
					res = RENDERER_1366x768;
				}
				
				else if(!strcmp(argv[i], "1600x900"))
				{
					res = RENDERER_1600x900;
				}
				
				else if(!strcmp(argv[i], "1920x1080"))
				{
					res = RENDERER_1920x1080;
				}
			}
			else if(!strcmp(argv[i], "-m"))
			{
				i++;
				if(!strcmp(argv[i], "fullscreen"))
				{
					mode = INIT_FORCE_FULLSCREEN;
				}
				else if(!strcmp(argv[i], "detect"))
				{
					mode = INIT_DETECT;
				}
			}
			else
			{
				i++;
			}
		}
	}
	
	
	pew_Init(res, mode);
	pew_SetInitGameFunction(ginit);
	pew_SetMainGameFunction(gmain);
	pew_SetPauseGameFunction(gpause);
	pew_SetResumeGameFunction(gresume);
	pew_MainLoop();
	pew_Finish();
}















