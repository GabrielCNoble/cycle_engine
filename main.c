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


extern entity_array entity_a;
extern input_cache input;
extern light_array light_a;

int l_count;

entity_ptr selected = {NULL, NULL, NULL, NULL};
void gmain(float delta_time)
{
	
	//selected.extra_data = NULL;
	int s = pew_GetPewState();
	vec3_t v;
	mat3_t orientation = mat3_t_id();
	
	if(s != PEW_PLAYING)
	{
		if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_JUST_CLICKED)
		{
			if(!(input.bm_mouse & MOUSE_OVER_WIDGET))
			{
				entity_QueryEntityUnderCursor();
				selected = entity_GetEntityUnderCursor();
				//if(selected.extra_data) printf("entity selected: %s\n", selected.extra_data->name);
			}
			
		}
	}
	else
	{
		ginput(delta_time);
	}
	
	if(selected.extra_data)
	{	
		draw_debug_DrawOutline(selected.position_data->world_position, &selected.position_data->world_orientation, selected.draw_data->mesh, vec3(1.0, 0.3, 1.0), 2.0, 0);
	}
	
	//draw_debug_DrawPoint(vec3(0.0, 10.0, 0.0), vec3(0.0, 1.0, 0.0), 8.0);

	
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
		dir.floats[2] = -1.0 * delta_time * 0.1;
	}
		
	else if(input_GetKeyPressed(SDL_SCANCODE_S))
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
	}

		
	pitch+=input.mouse_dy;
		
	if(pitch<-0.5)pitch=-0.5;
	else if(pitch > 0.5)pitch=0.5;
		
	yaw+=input.mouse_dx;
	
	//printf("%f\n", yaw);
	physics_Yaw(controller, yaw);
	//entity_RotateEntity(&eptr, vec3(0.0, 1.0, 0.0), -input.mouse_dx, 0);
	camera_RotateCamera(active_camera, vec3(1.0, 0.0, 0.0), -pitch, 1);
	r = controller->base.rigid_body->getCenterOfMassTransform().getBasis();
	p.x = r[0][0];
	p.y = r[0][1];
	p.z = r[0][2];
	
	wdir.x = p.x * dir.x + p.y * dir.y + p.z * dir.z;
	wdir.y = 0.0;
	
	p.x = r[2][0];
	p.y = r[2][1];
	p.z = r[2][2];
	
	wdir.z = p.x * dir.x + p.y * dir.y + p.z * dir.z;
		
	//dir = MultiplyVector3(&eptr.extra_data->local_orientation, dir);
		
	physics_Move(controller, wdir);
	
	p = add3(active_camera->world_position, mul3(active_camera->world_orientation.f_axis, -3.5));
	
	if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
		hit = entity_RayCast(active_camera->world_position, p);
	}
	
	if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_CLICKED)
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
	}
	
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
	
	tex_info_t tif;
	
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
	
	//armature_LoadBVH("tri.bvh", NULL, NULL);
	//armature_index = armature_GetArmatureIndex("Bone");
	

	/*parent = armature_CreateBone("root", vec3(0.0, 0.0, 0.0), &id);
	parent->tip = vec3(0.0, 0.0, -3.5);
	child = armature_CreateBone("child0", vec3(0.0, 0.0, -3.5), &id);
	child->tip = vec3(0.0, 0.0, -3.5);
	armature_AddBoneChild(&parent, child);
	child_child = armature_CreateBone("child1", vec3(0.0, 0.0, -3.5), &id);
	child_child->tip = vec3(0.0, 0.0, -3.5);
	armature_AddBoneChild(&child, child_child);
	child_child_child = armature_CreateBone("child2", vec3(0.0, 0.0, -3.5), &id);
	child_child_child->tip = vec3(0.0, 0.0, -3.5);
	armature_AddBoneChild(&child_child, child_child_child);
	child_child_child_child = armature_CreateBone("child3", vec3(0.0, 0.0, -3.5), &id);
	child_child_child_child->tip = vec3(0.0, 0.0, -3.5);
	armature_AddBoneChild(&child_child_child, child_child_child_child);

	armature_StoreBoneChain(parent);*/

	
	//armature = armature_GetArmature("Bone");
	//animation = armature_test_GenerateAnimation(armature->bone_count, 60.0, 240);
	//armature_PlayAnimation(armature, animation);

	/*int w = gpu_Alloc(15);
	gpu_Free(w);
	gpu_Defrag();*/

	/*int q[500];
	int w;
	
	for(w = 0; w < 500; w++)
	{
		
		q[w] = gpu_Alloc(512);
		
	}
	
	for(w = 0; w < 500; w++)
	{
		gpu_Free(q[w]);
	}
	gpu_Defrag();*/
	
	/*int in_v[32];
	int out_v[64];
	int q = gpu_Alloc(128);
	
	for(int g = 0; g < 32; g++)
	{
		in_v[g] = g;
	}

	gpu_Write(q, 0, in_v, 32*sizeof(int));
	
	for(int g = 0; g < 32; g++)
	{
		in_v[g] = g + 32;
	}
	gpu_Write(q, 32 * sizeof(int), in_v, 32*sizeof(int));
	
	
	gpu_Read(q, 0, out_v, 64*sizeof(int));
	
	for(int g = 0; g < 64; g++)
	{
		printf("%d\n", out_v[g]);
	}
	
	gpu_Free(q);*/

	
	
	/*parent = armature_CreateBone("test", vec3(0.0, 0.0, 0.0), &id);
	
	
	mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), -0.1, 1);
	child = armature_CreateBone("child", vec3(0.0, 0.0, 0.0), &id);
	armature_AddBoneChild(&parent, child);
	
	mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), 0.5, 1);
	child = armature_CreateBone("child2", vec3(0.0, 0.0, 0.0), &id);
	armature_AddBoneChild(&parent, child);
	
	child_child = armature_CreateBone("child_child", vec3(0.0, 0.0, 0.0), &id);
	armature_AddBoneChild(&child, child_child);
	id = mat3_t_id();
	child_child_child = armature_CreateBone("child_child_child", vec3(0.0, 0.0, 0.0), &id);
	armature_AddBoneChild(&child_child, child_child_child);
	
	armature_StoreBoneChain(parent);*/
	
	//mat3_t_rotate(&id, vec3(0.0, 1.0, 0.0), -0.3, 1);
	/*child = armature_CreateBone("child_child_child", vec3(0.0, 0.0, 0.0), &id);
	armature_AddBoneChild(&parent, child);*/
	
	
	id = mat3_t_id();

	
	
	
	
	
	//console_PassParam("clear");
	//console_PassParam("help enable");
	

	//shader_LoadShader("flat_vert.txt", "flat_frag.txt", "flat");
	//model_LoadModel("models\\teapot.obj", "teapot");
	//model_LoadModel("monkey.dae", "monkey");
	//model_LoadModel("monkey2.obj", "monkey2");
	//model_LoadModel("triangle.obj", "triangle");
	//model_LoadModel("Cube.obj", "cube");
	model_LoadModel("CubeUV2.obj", "cubeUV");
	model_LoadModel("ico_uv.obj", "ico");
	//model_LoadModel("single.obj", "single");
	
	//model_LoadModel("piramid.dae", "piramid");
	//model_LoadModel("rig9.dae", "full");
	//model_LoadModel("spiral.dae", "spiral");
	//armature_LoadBVH("dual.bvh", NULL, NULL);
	//model_LoadModel("dual.dae", "dual");
	//model_LoadModel("full_rigged2.dae", "rig");
	//model_LoadModel("mul.dae", "rig2");
	//model_LoadModel("cam_anim_idle.dae", "cam_anim");
	//model_LoadModel("new.dae", "new");
	
	//ad = armature_GetArmDefByIndex(0);
	
	//mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), 0.2, 1);
	/*armature_CreateArmature(ad, "test", vec3(0.0, 0.0, 0.0), &id);
	armature = armature_GetArmature("test");
	animation = armature_GetAnimationIndex("unnamed_animation.0000");
	armature_PlayAnimation(armature, animation);*/
	
	//model_LoadModel("rig6.dae", "rig");
	//model_LoadModel("uv_ico.obj", "uv_ico");
	model_LoadModel("pew_plane.obj", "pew_plane");
	
	//model_GenereateIcoSphere(2.0, 1);
	//model_GenerateIcoSphereMesh(2.0, 1);
	
	//model_GenerateConeMesh(1.0, 45.0, 8);
	//model_GenerateConvexPoly(1.0, 5);
	
	//model_LoadModel("models\\monkey2.obj", "monkey");
	//model_LoadModel("models\\IcoSmooth.obj", "sphere");
	model_LoadModel("BigPlane.obj", "plane");
	//model_LoadModel("models\\stencil.obj", "stencil");
	//model_LoadModel("models\\stencil2.obj", "stencil2");
	//model_LoadModel("wheel.obj", "wheel");
	//model_LoadModel("sky_dome.obj", "sky_dome");
	model_LoadModel("stairs.obj", "stairs");
	//model_LoadModel("platform.obj", "platform");
	//model_LoadModel("wow.obj", "wow");
	model_LoadModel("plat2.obj", "plat2");
	//model_LoadModel("Barel_blue.obj", "barrel");
	//texture_LoadTexture("brick.png", "brick");
	//texture_LoadTexture("cdiff.png", "cdiff", 0);
	//texture_LoadTexture("cnorm.png", "cnorm", 0);
	//texture_LoadTexture("skydome4.png", "skydome", 0);
	
	texture_LoadTexture("Barel_blue_D.tga", "barrel_blue_d", 0);
	texture_LoadTexture("Barel_blue_N.tga", "barrel_blue_n", 0);
	texture_LoadTexture("Barel_blue_S.tga", "barrel_blue_s", 0);
	//texture_LoadTexture("soil_t.png", "brick");
	texture_LoadTexture("ROCK09_N.png", "brick_n", 0);
	//texture_LoadTexture("brick_07_h.png", "brick_h", 0);
	
	//texture_LoadTexture("ROCK09.PNG", "rock_d", 0);
	//texture_LoadTexture("ROCK09_N.PNG", "rock_n", 0);
	//texture_LoadTexture("ROCK09_H.PNG", "rock_h", 0);
	
	//texture_LoadTexture("ROCK10.PNG", "rock3_d", 0);
	//texture_LoadTexture("ROCK10_N.PNG", "rock3_n", 0);
	//texture_LoadTexture("ROCK10_H.PNG", "rock3_h", 0);
	
	texture_LoadTexture("ROCK11.PNG", "rock4_d", 0);
	texture_LoadTexture("ROCK11_N.PNG", "rock4_n", 0);
	texture_LoadTexture("ROCK11_H.PNG", "rock4_h", 0);
	
	texture_LoadTexture("sand_diffuse.tga", "sand_diffuse", 0);
	texture_LoadTexture("sand_normal.tga", "sand_normal", 0);
	texture_LoadTexture("sand_specular.tga", "sand_specular", 0);
	//texture_LoadTexture("ROCK00.PNG", "rock2_d", 0);
	//texture_LoadTexture("ROCK00_N.PNG", "rock2_n", 0);
	//texture_LoadTexture("ROCK00_H.PNG", "rock2_h", 0);
	
	//texture_LoadTexture("metal_d.png", "metal_d", 0);
	//texture_LoadTexture("metal_n.png", "metal_n", 0);
	
	//texture_LoadTexture("tile_d.png", "tile_d", 0);
	//texture_LoadTexture("tile_n.png", "tile_n", 0);
	texture_LoadTexture("pew_logo.png", "tile_h", 0);
	
	texture_LoadTexture("greasy-pan-2-albedo.png", "greasy_diffuse", 0);
	texture_LoadTexture("greasy-pan-2-normal.png", "greasy_normal", 0);
	texture_LoadTexture("greasy-pan-2-roughness.png", "greasy_gloss", 0);
	texture_LoadTexture("greasy-pan-2-metal.png", "greasy_mettalic", 0);
	
	texture_LoadTexture("iron-rusted4-basecolor.png", "iron_rusted_diffuse", 0);
	texture_LoadTexture("iron-rusted4-normal.png", "iron_rusted_normal", 0);
	texture_LoadTexture("iron-rusted4-roughness.png", "iron_rusted_gloss", 0);
	texture_LoadTexture("iron-rusted4-metalness.png", "iron_rusted_mettalic", 0);
	
	texture_LoadTexture("oakfloor_basecolor.png", "dungeon_diffuse", 0);
	texture_LoadTexture("oakfloor_normal.png", "dungeon_normal", 0);
	texture_LoadTexture("oakfloor_roughness.png", "dungeon_gloss", 0);
	texture_LoadTexture("dungeon-stone1-metalness.png", "dungeon_mettalic", 0);
	texture_LoadTexture("dungeon-stone1-height.png", "dungeon_height", 0);
	
	texture_LoadTexture("scifi tile 1_COLOR.png", "scifi_d", 0);
	texture_LoadTexture("scifi tile 1_NRM.png", "scifi_n", 0);
	texture_LoadTexture("scifi tile 1_SPEC.png", "scifi_s", 0);
	
	//texture_LoadTexture("Pilip.tga", "pilip", 0);
	
	texture_LoadTexture("pew_logo.png", "pew", 0);
	
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
	cubeptr = model_GetMeshPtr("cube");
	stencilptr = model_GetMeshPtr("stencil");
	stencil2ptr = model_GetMeshPtr("stencil2");
	holesptr = model_GetMeshPtr("holes");
	
	
	

	
	//wbase_t *test = gui_CreateWidget("test0", WIDGET_HEADER|WIDGET_GRABBABLE|WIDGET_MOVABLE|WIDGET_TRANSLUCENT, -4.5, -2.0, 4.0, 4.0, 0.3, 0.3, 0.3, 0.5, texture_GetTextureID("pilip"), 0);
	wbase_t *serious_test = gui_CreateWidget("test1", WIDGET_HEADER|WIDGET_GRABBABLE|WIDGET_MOVABLE|WIDGET_TRANSLUCENT, 5.0, -3.0, 2.5, 1.2, 0.5, 0.3, 0.2, 0.2, WIDGET_NO_TEXTURE, 0);
	//gui_AddSubWidget(test, 0, WIDGET_BUTTON, "test_button", -0.8, 0.0, 0.5, 0.3, 0.0, 0.0, 0.5, 0.0, 0.0, 0.9, texture_GetTextureID("pew"), test, (void *)gui_test_CloseWidget);
	//gui_AddSubWidget(test, 0, WIDGET_BUTTON, "console_button", 0.8, 0.0, 0.5, 0.3, 0.0, 0.0, 0.0, 0.5, 0.0, 1.0, WIDGET_NO_TEXTURE, test, (void *)gui_test_CloseConsole);
	//gui_AddSubWidget(test, 0, WIDGET_VERTICAL_SCROLLER, "test_vertical_scroller", 1.0, 0.0, 0.0, 1.0, 0.5, -0.5, 0.2, 0.2, 0.2, 1.0, WIDGET_NO_TEXTURE, test, NULL);
	
	
	gui_AddSubWidget(serious_test, 0, WIDGET_BUTTON, "toggle volumetric lights", -0.5, -0.2, 0.3, 0.3, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, WIDGET_NO_TEXTURE, serious_test, (void *)gui_test_ToggleVolumetricLights);
	gui_AddSubWidget(serious_test, 0, WIDGET_BUTTON, "toggle shadows", 0.0, -0.2, 0.3, 0.3, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, WIDGET_NO_TEXTURE, serious_test, (void *)gui_test_ToggleShadows);
	gui_AddSubWidget(serious_test, 0, WIDGET_BUTTON, "toggle bloom", 0.5, -0.2, 0.3, 0.3, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, WIDGET_NO_TEXTURE, serious_test, (void *)gui_test_ToggleBloom);
	
	
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
	
	material_CreateMaterial("red", 2048, 1.0, 1.0 ,1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0, NULL);
	material_CreateMaterial("green", 128, 0.9, 0.9, 0.9, 1.0, 0.1, 0.1 ,0.1, 1.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_HeightTexture, &tif);
	
	
	//tif.diff_tex = (short)texture_GetTextureIndex("scifi_d");
	//tif.norm_tex = (short)texture_GetTextureIndex("scifi_n");
	//tif.spec_tex = (short)texture_GetTextureIndex("scifi_s");
	
	//material_CreateMaterial("test", 32, 0.9, 0.9, 0.9, 1.0, 0.1, 0.1 ,0.1, 0.5, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_SpecularTexture, &tif);

	tif.diff_tex = (short)texture_GetTextureIndex("greasy_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("greasy_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("greasy_gloss");
	tif.met_tex = (short)texture_GetTextureIndex("greasy_mettalic");
	material_CreateMaterial("greasy", 128, 0.9, 0.9, 0.9, 1.0, 1.0, 1.0 ,1.0, 1.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture|MATERIAL_MetallicTexture, &tif);
	
	
	tif.diff_tex = (short)texture_GetTextureIndex("iron_rusted_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("iron_rusted_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("iron_rusted_gloss");
	tif.met_tex = (short)texture_GetTextureIndex("iron_rusted_mettalic");
	material_CreateMaterial("iron_rusted", 128, 0.9, 0.9, 0.9, 1.0, 1.0, 1.0 ,1.0, 1.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture|MATERIAL_MetallicTexture, &tif);
	
	
	tif.diff_tex = (short)texture_GetTextureIndex("dungeon_diffuse");
	tif.norm_tex = (short)texture_GetTextureIndex("dungeon_normal");
	tif.gloss_tex = (short)texture_GetTextureIndex("dungeon_gloss");
	tif.met_tex = (short)texture_GetTextureIndex("dungeon_mettalic");
	tif.heig_tex = (short)texture_GetTextureIndex("dungeon_height");
	material_CreateMaterial("dungeon", 128, 0.9, 0.9, 0.9, 1.0, 1.0, 1.0 ,1.0, 1.0, MATERIAL_DiffuseTexture|MATERIAL_NormalTexture|MATERIAL_GlossTexture, &tif);
	
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
	
	camera_SetCameraByIndex(camera_CreateCamera("camera0", vec3(0.0, 0.5, 0.0), &id, 0.68, (float) renderer.width, (float) renderer.height, 0.1, 1000.0));
	
	//mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), -0.5, 1);
	//entity_CreateEntity("piramid", ENTITY_DYNAMIC, 0, vec3(0.0, 1.0, 3.0), &id, 2.0 ,model_GetMeshPtr("piramid"), material_GetMaterialIndex("red"), 1);
	
	id = mat3_t_id();
	
	entity_CreateEntityDef("plane", ENTITY_COLLIDES|ENTITY_STATIC_COLLISION, material_GetMaterialIndex("dungeon"), -1, planeptr, 0.0, COLLISION_SHAPE_CONVEX_HULL);
	
	entity_CreateEntityDef("pew_plane", ENTITY_COLLIDES|ENTITY_STATIC_COLLISION, material_GetMaterialIndex("translucent1"), -1, model_GetMeshPtr("pew_plane"), 0.0, COLLISION_SHAPE_CONVEX_HULL);
	//entity_CreateEntityDef("piramid", ENTITY_DYNAMIC, ENTITY_COLLIDES, material_GetMaterialIndex("red"), -1, model_GetMeshPtr("piramid"), 2.0, COLLISION_SHAPE_CONVEX_HULL);
	entity_CreateEntityDef("stairs", ENTITY_COLLIDES|ENTITY_STATIC_COLLISION, material_GetMaterialIndex("red"), -1, model_GetMeshPtr("stairs"), 1.0, COLLISION_SHAPE_CONVEX_HULL);
	entity_CreateEntityDef("ico_greasy", ENTITY_COLLIDES, material_GetMaterialIndex("greasy"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_iron_rusted", ENTITY_COLLIDES, material_GetMaterialIndex("iron_rusted"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("cube_greasy", ENTITY_COLLIDES, material_GetMaterialIndex("greasy"), -1, model_GetMeshPtr("cubeUV"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("cube_iron_rusted", ENTITY_COLLIDES, material_GetMaterialIndex("iron_rusted"), -1, model_GetMeshPtr("cubeUV"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_red", ENTITY_COLLIDES, material_GetMaterialIndex("translucent1"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_green", ENTITY_COLLIDES, material_GetMaterialIndex("translucent2"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	entity_CreateEntityDef("ico_blue", ENTITY_COLLIDES, material_GetMaterialIndex("translucent3"), -1, model_GetMeshPtr("ico"), 2.0, COLLISION_SHAPE_SPHERE);
	//entity_CreateEntityDef("spiral", ENTITY_STATIC, ENTITY_COLLIDES, material_GetMaterialIndex("red"), -1, model_GetMeshPtr("spiral"), 0.0, COLLISION_SHAPE_CONVEX_HULL);
	//entity_CreateEntityDef("rigged", 0, material_GetMaterialIndex("red"), 0, model_GetMeshPtr("rig"), 0.0, COLLISION_SHAPE_CONVEX_HULL);
	
	entity_def_t *def;
	//entity_def_t *def = entity_GetEntityDef("piramid");
	//entity_SpawnEntity("piramid", def, vec3(0.0, 0.0, 0.0), &id);
	def = entity_GetEntityDef("plane");
	entity_SpawnEntity("plane", def, vec3(0.0, -6.0, 0.0), &id);
	//entity_SpawnEntity("plane2", def, vec3(0.0, -6.0, 100.0), &id);
	def = entity_GetEntityDef("stairs");

	
	id = mat3_t_id();
	def = entity_GetEntityDef("ico_greasy");
	entity_SpawnEntity("ico_greasy", def, vec3(3.0, 0.0, 0.0), &id);
	
	def = entity_GetEntityDef("ico_iron_rusted");
	entity_SpawnEntity("ico_iron_rusted", def, vec3(-3.0, 0.0, 0.0), &id);
	
	
	def = entity_GetEntityDef("cube_greasy");
	entity_SpawnEntity("cube_greasy", def, vec3(0.0, 0.0, 3.0), &id);
	
	def = entity_GetEntityDef("cube_iron_rusted");
	entity_SpawnEntity("cube_iron_rusted", def, vec3(0.0, 0.0, -3.0), &id);
	
	
	/*def = entity_GetEntityDef("ico_red");
	entity_SpawnEntity("ico_red", def, vec3(0.0,  0.0, -5.0), &id);
	def = entity_GetEntityDef("ico_green");
	entity_SpawnEntity("ico_green", def, vec3(0.0, 0.0, 0.0), &id);
	def = entity_GetEntityDef("ico_blue");
	entity_SpawnEntity("ico_blue", def, vec3(0.0, 0.0, 5.0), &id);
	
	def = entity_GetEntityDef("ico_red");
	entity_SpawnEntity("ico_red", def, vec3(0.0,  0.0, -10.0), &id);
	def = entity_GetEntityDef("ico_green");
	entity_SpawnEntity("ico_green", def, vec3(5.0, 0.0, 0.0), &id);
	def = entity_GetEntityDef("ico_blue");
	entity_SpawnEntity("ico_blue", def, vec3(0.0, 0.0, 10.0), &id);
	
	def = entity_GetEntityDef("ico_red");
	entity_SpawnEntity("ico_red", def, vec3(0.0,  0.0, -10.0), &id);
	def = entity_GetEntityDef("ico_green");
	entity_SpawnEntity("ico_green", def, vec3(-5.0, 0.0, 0.0), &id);
	def = entity_GetEntityDef("ico_blue");
	entity_SpawnEntity("ico_blue", def, vec3(0.0, 0.0, 10.0), &id);*/
	//node_t *n = scenegraph_AddNode(NODE_BONE, 0, 1, "bone_node");
	//eptr = entity_GetEntity("ico");
	//scenegraph_SetParent(eptr.extra_data->assigned_node, n, 0);
	/*def = entity_GetEntityDef("ico");
	for(k = 0; k < 1; k++)
	{
		for(i=0; i < 10; i++)
		{
			for(j = 0; j < 10; j++)
			{
				entity_SpawnEntity("ico", def, vec3(-20.0 + i * 4, 9.0 + 4.0 * k, -20.0 + j * 4), &id);
				//entity_CreateEntity("cube0", ENTITY_DYNAMIC, 0, vec3(-20.0 + i * 4, 9.0 + 4.0 * k, -20.0 + j * 4), &id, 1.0, model_GetMeshPtr("ico"), material_GetMaterialIndex("blue"), 0);
			}
			
		}
	}*/
	
	
	id = mat3_t_id();
	
	for(i=0; i<1; i++)
	{	
		light_CreateLight("lightwow6", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(0.0, -2.0, 0.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 10.0, 10.0, 40, 0.5, 0.01, 0.001, 20.5, 64, 4, 256, 256, 2, -1);
		//light_CreateLight("lightwow6", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(0.0, -3.5, 0.0, 1.0), &id, vec3(1.0, 0.0, 0.0), 10.0, 10.0, 40, 0.5, 0.02, 0.0, 0.03, 32, 32, 2048, 256, 2);
		
		
		light_CreateLight("lightwow8", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(-10.0, -1.0, 0.0, 1.0), &id, vec3(0.5, 1.0, 0.5), 20.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 512, 512, 1, -1);
		mat3_t_rotate(&id, vec3(1.0, 0.0, 0.0), -0.7, 1);
		light_CreateLight("lightwow7", LIGHT_SPOT|LIGHT_GENERATE_SHADOWS|LIGHT_DRAW_VOLUME, vec4(-3.0, -1.0, -15.0, 1.0), &id, vec3(1.0, 0.6, 0.6), 20.0, 10.0, 10, 0.05, 0.02, 0.0, 0.03, 32, 32, 512, 512, 2, -1);
		
		light_CreateLight("lightwow8", LIGHT_SPOT|LIGHT_GENERATE_SHADOWS|LIGHT_DRAW_VOLUME, vec4(3.0, -1.0, -15.0, 1.0), &id, vec3(0.6, 0.6, 1.0), 20.0, 10.0, 50, 0.05, 0.02, 0.0, 0.03, 32, 32, 512, 512, 2, -1);
		
		/*light_CreateLight("lightwow9", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(10.0, -1.0, 0.0, 1.0), &id, vec3(0.5, 0.5, 1.0), 20.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 512, 512, 1, -1);
		light_CreateLight("lightwow10", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(20.0, 2.0, -10.0, 1.0), &id, vec3(1.0, 0.6, 1.0), 10.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 512, 512, 1, -1);
		light_CreateLight("lightwow11", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(20.0, 2.0, 8.0, 1.0), &id, vec3(0.0, 0.6, 0.8), 10.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 512, 512, 1, -1);
		light_CreateLight("lightwow12", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(-6.0, 2.0, 25.0, 1.0), &id, vec3(0.2, 0.7, 0.6), 10.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 512, 512, 1, -1);
		light_CreateLight("lightwow13", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(-23.0, 2.0, -10.0, 1.0), &id, vec3(1.0, 1.0, 1.0), 10.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 512, 512, 1, -1);
		
		light_CreateLight("lightwow14", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(0.0, -3.0, -25.0, 1.0), &id, vec3(0.2, 0.2, 1.0), 8.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 128, 128, 1, -1);
		light_CreateLight("lightwow15", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(8.0, -3.0, -25.0, 1.0), &id, vec3(0.2, 0.2, 1.0), 8.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 128, 128, 1, -1);
		light_CreateLight("lightwow16", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(-8.0, -3.0, -25.0, 1.0), &id, vec3(0.2, 0.2, 1.0), 8.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 128, 128, 1, -1);
		light_CreateLight("lightwow17", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(16.0, -3.0, -25.0, 1.0), &id, vec3(0.2, 0.2, 1.0), 8.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 128, 128, 1, -1);
		light_CreateLight("lightwow18", LIGHT_POINT|LIGHT_GENERATE_SHADOWS, vec4(-16.0, -3.0, -25.0, 1.0), &id, vec3(0.2, 0.2, 1.0), 8.0, 10.0, 40, 0.0, 0.02, 0.0, 0.03, 32, 32, 128, 128, 1, -1);*/
	}
	
	
	id = mat3_t_id();
	cptr=camera_GetActiveCamera();
	camera_TranslateCamera(cptr, vec3(0.0, 1.0 ,0.0), 1.7, 1);
	int col_index = physics_CreateCollider("_player_", COLLIDER_CHARACTER_CONTROLLER, COLLISION_SHAPE_CAPSULE, COLLIDER_CREATE_SCENEGRAPH_NODE, -1, 0.0, 0.0, 2.0, 0.5, 5.0, 14.7, 50.0, NULL, vec3(0.0, 0.0, 0.0), &id, NULL);
	general_collider_t *c = physics_GetColliderByIndex(col_index);
	scenegraph_SetParent(cptr->assigned_node, c->base.assigned_node, 0);
	

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















