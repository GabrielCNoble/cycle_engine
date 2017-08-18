#include "draw_debug.h"
//#include "draw_common.h"
#include "draw.h"


#include "camera.h"
#include "gpu.h"
#include "shader.h"
#include "light.h"
#include "armature.h"
#include "camera.h"
#include "framebuffer.h"
#include "entity.h"
#include "physics.h"


/* (~46 MB)  */
#define FLOAT_BUFFER_SIZE 40000
#define DEBUG_NBUFFER_HEIGHT 0.7
#define SCALE 0.25


unsigned int screen_area_mesh_gpu_buffer;
extern framebuffer_t backbuffer;
extern framebuffer_t geometry_buffer;
extern framebuffer_t final_buffer;

int draw_buffer_shader_index;
int draw_z_buffer_shader_index;


float light_mesh_bounds[144 * 3 * 3];
float light_mesh_cone_lod0[16 * 3 * 3 * 3];
float light_mesh_cone_lod1[16 * 3 * 3 * 3];
float light_mesh_sphere_lod0[20 * 3 * 3 * 3 * 3];	/* 80 triangles */
float light_mesh_sphere_lod1[20 * 3 * 3 * 3 * 3];
			

float screen_quad[]=
{
	-1.0, 1.0, -0.1,
	-1.0,-1.0, -0.1,
	 1.0,-1.0, -0.1,
	 1.0, 1.0, -0.1
};

float bone_mesh_octahedron[]=
{
	0.0, 0.0, 0.0, 
	0.5, 0.5,-0.7,
	0.5,-0.5,-0.7,
	
	0.0, 0.0, 0.0,
	0.5,-0.5,-0.7,
   -0.5,-0.5,-0.7,
   
    0.0, 0.0, 0.0,
   -0.5,-0.5,-0.7,
   -0.5, 0.5,-0.7,
   
    0.0, 0.0, 0.0,
   -0.5, 0.5,-0.7,
    0.5, 0.5,-0.7,
    
    0.0, 0.0,-3.5, 
	0.5, 0.5,-0.7,
	0.5,-0.5,-0.7,
	
	0.0, 0.0,-3.5,
	0.5,-0.5,-0.7,
   -0.5,-0.5,-0.7,
   
    0.0, 0.0,-3.5,
   -0.5,-0.5,-0.7,
   -0.5, 0.5,-0.7,
   
    0.0, 0.0,-3.5,
   -0.5, 0.5,-0.7,
    0.5, 0.5,-0.7,
   	
};


extern entity_array entity_a;
extern camera_array camera_a;
extern framebuffer_t backbuffer;
extern renderer_t renderer;
extern shader_array shader_a;
extern light_array active_light_a;
extern armature_list_t armature_list;
extern collider_array collider_a;
extern int debug_flags;

//extern gpu_buffer_t debug_buffer;



static int draw_cmd_count = 0;
static int draw_cmd_list_size = 320;
static debug_draw_t *draw_cmds;
//static debug_draw_t draw_cmds[16];

static float *float_buffer;
static int used_floats = 0;

//float circle_v[];
//float cone_v[];

//static float light_mesh_sphere[144];
//static float light_mesh_cone[54];
//static float bone_mesh_octahedron[24*3];

static float e_origin_color[3] = {0.7,  0.2,  0.2};			/* entity origin color */
static float e_aabb_color[3] = 	 {0.4,  0.4,  1.0};			/* entity aabb color */
static float l_origin_color[3] = {0.65, 0.65, 1.0}; 		/* light origin color */
static float l_limits_point_color[3] = {0.4, 0.4, 1.0};			/* light limits color */
static float l_limits_spot_color[3] = {0.8, 0.5, 0.5};
static float a_origin_color[3] = {0.7,  0.2,  0.2};			/* armature bone origin color */
static float a_color[3] = 		 {1.0,  1.0,  1.0};			/* armature bone color */
static float c_color[3] = 		 {0.6,  0.5,  1.0};			/* constraint color */


extern int wireframe_shader_index;

/* all debug draw commands are drawn on the default framebuffer... */

#ifdef _cplusplus
extern "C"
{
#endif 

void draw_debug_Init()
{
	
	float angle=0.0;
	float step=(2.0*3.14159265)/16.0;
	int i;
	int j;
	int k;
	
	float *verts;
	int s0_face_count;
	int s1_face_count;
	int c0_face_count;	
	int c1_face_count;
	
	for(i=0; i<16; i++)
	{
		light_mesh_bounds[i*3]=cos(angle);
		light_mesh_bounds[i*3+1]=0.0;
		light_mesh_bounds[i*3+2]=sin(angle);
		angle+=step;
	}
	for(i=0; i<16; i++)
	{
		light_mesh_bounds[48+i*3]=0.0;
		light_mesh_bounds[48+i*3+1]=light_mesh_bounds[i*3];
		light_mesh_bounds[48+i*3+2]=light_mesh_bounds[i*3+2];
	}
	
	for(i=0; i<16; i++)
	{
		light_mesh_bounds[96+i*3]=light_mesh_bounds[i*3];
		light_mesh_bounds[96+i*3+1]=light_mesh_bounds[i*3+2];
		light_mesh_bounds[96+i*3+2]=0.0;
	}
	
	model_GenerateIcoSphere(1.0, 1, &verts, &s0_face_count);
	
	for(i = 0; i < s0_face_count * 3; i++)
	{
		light_mesh_sphere_lod0[i * 3] = verts[i * 3];
		light_mesh_sphere_lod0[i * 3 + 1] = verts[i * 3 + 1];
		light_mesh_sphere_lod0[i * 3 + 2] = verts[i * 3 + 2];
	}
	free(verts);
	
	model_GenerateIcoSphere(1.0, 0, &verts, &s1_face_count);
	
	for(i = 0; i < s1_face_count * 3; i++)
	{
		light_mesh_sphere_lod1[i * 3] = verts[i * 3];
		light_mesh_sphere_lod1[i * 3 + 1] = verts[i * 3 + 1];
		light_mesh_sphere_lod1[i * 3 + 2] = verts[i * 3 + 2];
	}
	free(verts);
	
	model_GenerateCone(1.0, 45.0, 8, &verts, &c0_face_count);
	
	for(i = 0; i < c0_face_count * 9; i++)
	{
		light_mesh_cone_lod0[i] = verts[i];
	}
	free(verts);
	
	model_GenerateCone(1.0, 45.0, 5, &verts, &c1_face_count);
	
	for(i = 0; i < c1_face_count * 9; i++)
	{
		light_mesh_cone_lod1[i] = verts[i];
	}
	free(verts);
	
	glGenBuffers(1, &screen_area_mesh_gpu_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 3 * (s0_face_count + s1_face_count + c0_face_count + c1_face_count + 4), NULL, GL_STATIC_DRAW);
	verts = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	
	for(i = 0; i < 12; i++)
	{
		verts[i] = screen_quad[i];
	}
	j = i;
	
	for(i = 0; i < s0_face_count * 9; i++)
	{
		verts[j + i] = light_mesh_sphere_lod0[i];
	}
	j += i;
	
	for(i = 0; i < s1_face_count * 9; i++)
	{
		verts[j + i] = light_mesh_sphere_lod1[i];
	}
	j += i;
	
	for(i = 0; i < c0_face_count * 9; i++)
	{
		verts[j + i] = light_mesh_cone_lod0[i];
	}
	
	j += i;
	
	for(i = 0; i < c1_face_count * 9; i++)
	{
		verts[j + i] = light_mesh_cone_lod1[i];
	}
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	
	float_buffer = (float *)malloc(sizeof(float) * FLOAT_BUFFER_SIZE);
	draw_cmds = (debug_draw_t *)malloc(sizeof(debug_draw_t) * draw_cmd_list_size);
	
	draw_cmd_count = 0;
	used_floats = 0;
	
}

void draw_debug_Finish()
{
	free(float_buffer);
}

void draw_debug_Draw()
{
	camera_t *active_camera = camera_GetActiveCamera();
	int i;
	int c = draw_cmd_count;
	int j;
	int l;
	
	float nzfar;
	/* negated znear */
	float nznear;
	
	float ftop;
	float ntop;
	float fright;
	float nright;
	float radius;
	float distance;
	float len;
	
	float *verts;
	mesh_t *m;
	mat3_t *o;
	vec3_t *w;
	mat3_t r;
	mat4_t p;
	mat4_t q;
	vec3_t d;
	float ns;
	
	vec3_t fvec;
	vec3_t uvec;
	vec3_t rvec;
	
	vec3_t l_origin;
	vec3_t e_origin;
	vec3_t le_vec;
	
	frustum_t *frustum;
	
	vec3_t ftl;
	vec3_t fbl;
	vec3_t ftr;
	vec3_t fbr;
	vec3_t ntl;
	vec3_t nbl;
	vec3_t ntr;
	vec3_t nbr;
	vec3_t diag;
	vec3_t fc;
	vec3_t nc;
	
	framebuffer_t *framebuffer;
	
	//framebuffer_BindFramebuffer(&backbuffer);
	framebuffer_BindFramebuffer(&final_buffer);
	
	glUseProgram(0);
	
	glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	//glViewport(0, 0, active_camera->width, active_camera->height);
	
	glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	vec3_t v;
	float t;
	
	mat3_t entity_orientation;
	
	for(i = 0; i < c; i++)
	{
		switch(draw_cmds[i].type)
		{
			
			case DRAW_TRIANGLE:
				
				glBegin(GL_TRIANGLES);
				glColor3f(draw_cmds[i].data[0], draw_cmds[i].data[1], draw_cmds[i].data[2]);
				glVertex3f(draw_cmds[i].data[3], draw_cmds[i].data[4], draw_cmds[i].data[5]);
				glColor3f(draw_cmds[i].data[6], draw_cmds[i].data[7], draw_cmds[i].data[8]);
				glVertex3f(draw_cmds[i].data[9], draw_cmds[i].data[10], draw_cmds[i].data[11]);
				glColor3f(draw_cmds[i].data[12], draw_cmds[i].data[13], draw_cmds[i].data[14]);
				glVertex3f(draw_cmds[i].data[15], draw_cmds[i].data[16], draw_cmds[i].data[17]);
				glEnd();
				
			break;
			
			case DRAW_LINE:
				glLineWidth(draw_cmds[i].data[9]);
				/*glEnable(GL_STENCIL_TEST);
				
				glStencilFunc(GL_EQUAL, 0, 0xff);
				glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
				
				if(draw_cmds[i].data[11])
				{
					glClearStencil(0);
					glClear(GL_STENCIL_BUFFER_BIT);
				}*/
				
				if(draw_cmds[i].data[10])
				{
					glDisable(GL_DEPTH_TEST);
				}

				glBegin(GL_LINES);
				glColor3f(draw_cmds[i].data[0], draw_cmds[i].data[1], draw_cmds[i].data[2]);
				glVertex3f(draw_cmds[i].data[3], draw_cmds[i].data[4], draw_cmds[i].data[5]);
				glVertex3f(draw_cmds[i].data[6], draw_cmds[i].data[7], draw_cmds[i].data[8]);
				glEnd();
				glLineWidth(1.0);
				glEnable(GL_DEPTH_TEST);
			break;
			
			case DRAW_LINE_LOOP:
				glLineWidth(draw_cmds[i].data[8]);
				glEnable(GL_LINE_SMOOTH);
				if(draw_cmds[i].data[9])
				{
					glDisable(GL_DEPTH_TEST);
				}
				
				verts = (float *)*(int *)&draw_cmds[i].data[6];
				l = *(int *)&draw_cmds[i].data[7];
				
				glBegin(GL_LINE_LOOP);
				glColor3f(draw_cmds[i].data[0], draw_cmds[i].data[1], draw_cmds[i].data[2]);
				
				v.x = draw_cmds[i].data[3];
				v.y = draw_cmds[i].data[4];
				v.z = draw_cmds[i].data[5];
				
				for(j = 0; j < l; j++)
				{
					glVertex3f(verts[j * 3] + v.x, verts[j * 3 + 1] + v.y, verts[j * 3 + 2] + v.z);
				}
				glEnd();
				glLineWidth(1.0);
				glDisable(GL_LINE_SMOOTH);
				glEnable(GL_DEPTH_TEST);
			break;
			
			case DRAW_LINE_HOMOGENEOUS:
				
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
				
				glLineWidth(draw_cmds[i].data[9]);
				if(draw_cmds[i].data[10])
				{
					glDisable(GL_DEPTH_TEST);
				}

				glBegin(GL_LINES);
				glColor3f(draw_cmds[i].data[0], draw_cmds[i].data[1], draw_cmds[i].data[2]);
				glVertex3f(draw_cmds[i].data[3], draw_cmds[i].data[4], draw_cmds[i].data[5]);
				glVertex3f(draw_cmds[i].data[6], draw_cmds[i].data[7], draw_cmds[i].data[8]);
				glEnd();
				glLineWidth(1.0);
				glEnable(GL_DEPTH_TEST);
				
				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
				
			break;
			
			case DRAW_POINT:

				glPointSize(draw_cmds[i].data[6]);
				glEnable(GL_POINT_SMOOTH);
				
				glBegin(GL_POINTS);
				glColor3f(draw_cmds[i].data[0], draw_cmds[i].data[1], draw_cmds[i].data[2]);
				glVertex3f(draw_cmds[i].data[3], draw_cmds[i].data[4], draw_cmds[i].data[5]);
				glEnd();
				
				glPointSize(1.0);
				glDisable(GL_POINT_SMOOTH);
				
			break;
			
			case DRAW_POINT_HOMOGENEOUS:
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
				
				glPointSize(draw_cmds[i].data[6]);
				glEnable(GL_POINT_SMOOTH);
				
				glBegin(GL_POINTS);
				glColor3f(draw_cmds[i].data[0], draw_cmds[i].data[1], draw_cmds[i].data[2]);
				glVertex3f(draw_cmds[i].data[3], draw_cmds[i].data[4], draw_cmds[i].data[5]);
				glEnd();
				
				glPointSize(1.0);
				glDisable(GL_POINT_SMOOTH);
				
				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
			
			break;
			
			case DRAW_FRUSTUM:
				
				w = (vec3_t *)&draw_cmds[i].data[0];
				o = (mat3_t *)&draw_cmds[i].data[3];
				frustum = (frustum_t *)&draw_cmds[i].data[12];
				
				
				rvec.floats[0] = o->floats[0][0];
				rvec.floats[1] = o->floats[0][1];
				rvec.floats[2] = o->floats[0][2];
				
				uvec.floats[0] = o->floats[1][0];
				uvec.floats[1] = o->floats[1][1];
				uvec.floats[2] = o->floats[1][2];
				
				fvec.floats[0] = o->floats[2][0];
				fvec.floats[1] = o->floats[2][1];
				fvec.floats[2] = o->floats[2][2];
				
				
				
				nright = frustum->right;
				ntop = frustum->top;
				
				nznear = -frustum->znear;
				nzfar = -frustum->zfar;
					
				ftop = (nzfar*ntop)/nznear;
				fright = (nzfar*nright)/nznear;
						
				nc.floats[0] = w->floats[0] + fvec.floats[0]*nznear;
				nc.floats[1] = w->floats[1] + fvec.floats[1]*nznear;
				nc.floats[2] = w->floats[2] + fvec.floats[2]*nznear;
						
				fc.floats[0] = w->floats[0] + fvec.floats[0]*nzfar;
				fc.floats[1] = w->floats[1] + fvec.floats[1]*nzfar;
				fc.floats[2] = w->floats[2] + fvec.floats[2]*nzfar;
						
				ftl.floats[0] = fc.floats[0] - rvec.floats[0]*fright + uvec.floats[0]*ftop;
				ftl.floats[1] = fc.floats[1] - rvec.floats[1]*fright + uvec.floats[1]*ftop;
				ftl.floats[2] = fc.floats[2] - rvec.floats[2]*fright + uvec.floats[2]*ftop;
					
				ftr.floats[0] = fc.floats[0] + rvec.floats[0]*fright + uvec.floats[0]*ftop;
				ftr.floats[1] = fc.floats[1] + rvec.floats[1]*fright + uvec.floats[1]*ftop;
				ftr.floats[2] = fc.floats[2] + rvec.floats[2]*fright + uvec.floats[2]*ftop;
					
				fbl.floats[0] = fc.floats[0] - rvec.floats[0]*fright - uvec.floats[0]*ftop;
				fbl.floats[1] = fc.floats[1] - rvec.floats[1]*fright - uvec.floats[1]*ftop;
				fbl.floats[2] = fc.floats[2] - rvec.floats[2]*fright - uvec.floats[2]*ftop;
					
				fbr.floats[0] = fc.floats[0] + rvec.floats[0]*fright - uvec.floats[0]*ftop;
				fbr.floats[1] = fc.floats[1] + rvec.floats[1]*fright - uvec.floats[1]*ftop;
				fbr.floats[2] = fc.floats[2] + rvec.floats[2]*fright - uvec.floats[2]*ftop;
					
					
				ntl.floats[0] = nc.floats[0] - rvec.floats[0]*nright + uvec.floats[0]*ntop;
				ntl.floats[1] = nc.floats[1] - rvec.floats[1]*nright + uvec.floats[1]*ntop;
				ntl.floats[2] = nc.floats[2] - rvec.floats[2]*nright + uvec.floats[2]*ntop;
					
				nbl.floats[0] = nc.floats[0] - rvec.floats[0]*nright - uvec.floats[0]*ntop;
				nbl.floats[1] = nc.floats[1] - rvec.floats[1]*nright - uvec.floats[1]*ntop;
				nbl.floats[2] = nc.floats[2] - rvec.floats[2]*nright - uvec.floats[2]*ntop;
					
				ntr.floats[0] = nc.floats[0] + rvec.floats[0]*nright + uvec.floats[0]*ntop;
				ntr.floats[1] = nc.floats[1] + rvec.floats[1]*nright + uvec.floats[1]*ntop;
				ntr.floats[2] = nc.floats[2] + rvec.floats[2]*nright + uvec.floats[2]*ntop;
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				
				glLineWidth(6.0);
				glBegin(GL_TRIANGLES);
				glColor3f(0.7, 0.7, 0.9);
				
				glVertex3f(w->x, w->y, w->z);
				glVertex3f(ftl.x, ftl.y, ftl.z);
				glVertex3f(ftr.x, ftr.y, ftr.z);
				
				glVertex3f(w->x, w->y, w->z);
				glVertex3f(ftl.x, ftl.y, ftl.z);
				glVertex3f(fbl.x, fbl.y, fbl.z);
				
				glVertex3f(w->x, w->y, w->z);
				glVertex3f(ftr.x, ftr.y, ftr.z);
				glVertex3f(fbr.x, fbr.y, fbr.z);
				
				glVertex3f(w->x, w->y, w->z);
				glVertex3f(fbl.x, fbl.y, fbl.z);
				glVertex3f(fbr.x, fbr.y, fbr.z);
				glEnd();
				glLineWidth(1.0);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				
				glBegin(GL_TRIANGLES);
				glColor4f(0.7, 0.7, 0.9, 0.2);
				
				glVertex3f(w->x, w->y, w->z);
				glVertex3f(ftl.x, ftl.y, ftl.z);
				glVertex3f(ftr.x, ftr.y, ftr.z);
				
				glVertex3f(w->x, w->y, w->z);
				glVertex3f(ftl.x, ftl.y, ftl.z);
				glVertex3f(fbl.x, fbl.y, fbl.z);
				
				glVertex3f(w->x, w->y, w->z);
				glVertex3f(ftr.x, ftr.y, ftr.z);
				glVertex3f(fbr.x, fbr.y, fbr.z);
				
				glVertex3f(w->x, w->y, w->z);
				glVertex3f(fbl.x, fbl.y, fbl.z);
				glVertex3f(fbr.x, fbr.y, fbr.z);
				glEnd();
				
				glDisable(GL_BLEND);
				
				
			break;
			
			case DRAW_OUTLINE:
				glEnable(GL_STENCIL_TEST);
				//glDisable(GL_STENCIL_TEST);
				glClearStencil(0);
				glClear(GL_STENCIL_BUFFER_BIT);
				
				
				glStencilFunc(GL_ALWAYS, 0x1, 0xff);
				glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
				
				for(j = 0; j < 9; j++)
				{
					r.lfloats[j] = draw_cmds[i].data[j + 3];
				}
				glEnable(GL_DEPTH_TEST);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				mat4_t_compose(&p, &r, vec3(draw_cmds[i].data[0], draw_cmds[i].data[1], draw_cmds[i].data[2]));
				mat4_t_mult(&q, &p, &active_camera->world_to_camera_matrix);
				glLoadMatrixf(&q.floats[0][0]);				
				
				
				
				m = *(mesh_t **)&draw_cmds[i].data[12];
				l = m->vert_count;
				glLineWidth(draw_cmds[i].data[16]);
				
				glCullFace(GL_BACK);
				glStencilFunc(GL_ALWAYS, 0x1, 0xff);
				glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				//glEnable(GL_POLYGON_OFFSET_FILL);
				//glPolygonOffset(1.0, -2.0);
				
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				glDepthMask(GL_FALSE);
				
				glBegin(GL_TRIANGLES);
				glColor3f(draw_cmds[i].data[13], draw_cmds[i].data[14], draw_cmds[i].data[15]);
				for(j = 0; j < l;)
				{
					glVertex3f(m->v_data[j * 3], m->v_data[j * 3 + 1], m->v_data[j * 3 + 2]);
					j++;
					glVertex3f(m->v_data[j * 3], m->v_data[j * 3 + 1], m->v_data[j * 3 + 2]);
					j++;
					glVertex3f(m->v_data[j * 3], m->v_data[j * 3 + 1], m->v_data[j * 3 + 2]);
					j++;
				}
				glEnd();
				
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				
				glStencilFunc(GL_NOTEQUAL, 0x1, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				
				glBegin(GL_TRIANGLES);
				glColor3f(draw_cmds[i].data[13], draw_cmds[i].data[14], draw_cmds[i].data[15]);
				for(j = 0; j < l;)
				{
					glVertex3f(m->v_data[j * 3], m->v_data[j * 3 + 1], m->v_data[j * 3 + 2]);
					j++;
					glVertex3f(m->v_data[j * 3], m->v_data[j * 3 + 1], m->v_data[j * 3 + 2]);
					j++;
					glVertex3f(m->v_data[j * 3], m->v_data[j * 3 + 1], m->v_data[j * 3 + 2]);
					j++;
				}
				glEnd();
				
				
				glDisable(GL_POLYGON_OFFSET_LINE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glDisable(GL_STENCIL_TEST);
				glEnable(GL_CULL_FACE);
				glLineWidth(1.0);
				glPopMatrix();
			break;
			
			case DRAW_BRUSH_OUTLINE:
				
				verts = *(float **)&draw_cmds[i].data[0];
				l = *(int *)&draw_cmds[i].data[1];
				
				glEnable(GL_STENCIL_TEST);
				glClearStencil(0);
				glClear(GL_STENCIL_BUFFER_BIT);
				
				glStencilFunc(GL_ALWAYS, 0x1, 0xff);
				glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
				
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				glDepthMask(GL_FALSE);
				glBegin(GL_TRIANGLES);
				for(j = 0; j < l;)
				{
					glVertex3f(verts[j * 6], verts[j * 6 + 1], verts[j * 6 + 2]);
					j++;
					glVertex3f(verts[j * 6], verts[j * 6 + 1], verts[j * 6 + 2]);
					j++;
					glVertex3f(verts[j * 6], verts[j * 6 + 1], verts[j * 6 + 2]);
					j++;
				}
				glEnd();
				
				
				glStencilFunc(GL_NOTEQUAL, 0x1, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glLineWidth(6.0);
				glBegin(GL_TRIANGLES);
				glColor3f(draw_cmds[i].data[2], draw_cmds[i].data[3], draw_cmds[i].data[4]);
				for(j = 0; j < l;)
				{
					glVertex3f(verts[j * 6], verts[j * 6 + 1], verts[j * 6 + 2]);
					j++;
					glVertex3f(verts[j * 6], verts[j * 6 + 1], verts[j * 6 + 2]);
					j++;
					glVertex3f(verts[j * 6], verts[j * 6 + 1], verts[j * 6 + 2]);
					j++;
				}
				glEnd();
				glLineWidth(1.0);
				glDisable(GL_STENCIL_TEST);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
			break;
			
			case DRAW_BLIT_FRAMEBUFFER:
				framebuffer = *(framebuffer_t **)&draw_cmds[i].data[0];
				i = *(int *)&draw_cmds[i].data[1];
				j = *(int *)&draw_cmds[i].data[2];
				glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer->id);
				glBlitFramebuffer(0, 0, framebuffer->width, framebuffer->height, i, j, framebuffer->width, framebuffer->height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			break;
		}
	}
	
	c = active_light_a.light_count;
	
	
	if(debug_flags & DEBUG_DRAW_LIGHT_ORIGINS)
	{
		//printf("draw light origins\n");
		glEnable(GL_POINT_SMOOTH);
		glPointSize(12.0);
		for(i = 0; i < c; i++)
		{	
			glBegin(GL_POINTS);
			glColor3f(l_origin_color[0], l_origin_color[1], l_origin_color[2]);
			glVertex3f(active_light_a.position_data[i].world_position.x,
					   active_light_a.position_data[i].world_position.y,
					   active_light_a.position_data[i].world_position.z);
			glEnd();
			
			if(active_light_a.position_data[i].bm_flags & LIGHT_SPOT)
			{
				glLineWidth(4.0);
				glBegin(GL_LINES);
				glColor3f(1.0, 0.0, 0.0);
				glVertex3f(active_light_a.position_data[i].world_position.x, 
					   	   active_light_a.position_data[i].world_position.y, 
					       active_light_a.position_data[i].world_position.z);
						   
				glVertex3f(-active_light_a.position_data[i].world_orientation.floats[2][0] * active_light_a.position_data[i].radius + active_light_a.position_data[i].world_position.x, 
					       -active_light_a.position_data[i].world_orientation.floats[2][1] * active_light_a.position_data[i].radius + active_light_a.position_data[i].world_position.y, 
					       -active_light_a.position_data[i].world_orientation.floats[2][2] * active_light_a.position_data[i].radius + active_light_a.position_data[i].world_position.z);
				glEnd();
				glLineWidth(1.0);
			}
		
		}
		glDisable(GL_POINT_SMOOTH);
		glPointSize(1.0);
	}
	
	
	if(debug_flags & DEBUG_DRAW_LIGHT_LIMITS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glCullFace(GL_FRONT);
		//glDisable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
		for(i = 0; i < c; i++)
		{
			if(active_light_a.position_data[i].bm_flags & LIGHT_SPOT)
			{
				
				t = tan(DegToRad((float)active_light_a.position_data[i].spot_co)) * active_light_a.position_data[i].radius;
				
				
				//glCullFace(GL_BACK);
				//glEnable(GL_CULL_FACE);
				//glEnable(GL_DEPTH_TEST);
				//glCullFace(GL_FRONT);
				glBegin(GL_TRIANGLES);
				glColor4f(l_limits_spot_color[0], l_limits_spot_color[1], l_limits_spot_color[2], 0.2);
				for(j = 0; j < 48; j++)
				{
					
					v.x = light_mesh_cone_lod0[j * 3] * t;
					v.y = light_mesh_cone_lod0[j * 3 + 1] * t;
					v.z = light_mesh_cone_lod0[j * 3 + 2] * active_light_a.position_data[i].radius;
					
					v = MultiplyVector3(&active_light_a.position_data[i].world_orientation, v);
					
					glVertex3f(v.x + active_light_a.position_data[i].world_position.x, 
							   v.y + active_light_a.position_data[i].world_position.y, 
							   v.z + active_light_a.position_data[i].world_position.z);
							   
							   
				}
				glEnd();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				
				
				
				
				
			}
			else if(active_light_a.position_data[i].bm_flags & LIGHT_POINT)
			{
				//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				//glEnable(GL_BLEND);
				//glEnable(GL_CULL_FACE);
				//glDisable(GL_DEPTH_TEST);
				//glCullFace(GL_BACK);
				
				//glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				glBegin(GL_TRIANGLES);
				glColor4f(l_limits_point_color[0], l_limits_point_color[1], l_limits_point_color[2], 0.2);
				for(j = 0; j < 240; j++)
				{
					glVertex3f(light_mesh_sphere_lod0[j * 3] * active_light_a.position_data[i].radius + active_light_a.position_data[i].world_position.x, 
							   light_mesh_sphere_lod0[j * 3 + 1] * active_light_a.position_data[i].radius + active_light_a.position_data[i].world_position.y,
							   light_mesh_sphere_lod0[j * 3 + 2] * active_light_a.position_data[i].radius + active_light_a.position_data[i].world_position.z);
				}
				glEnd();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	
	c = entity_a.entity_count;
	if(debug_flags & DEBUG_DRAW_ENTITY_ORIGIN)
	{
		//for(i = 0; i < c; i++)
		//{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(12.0);
		
		for(i = 0; i < c; i++)
		{	
			
			
			
			glBegin(GL_POINTS);
			glColor3f(e_origin_color[0], e_origin_color[1], e_origin_color[2]);
			glVertex3f(entity_a.position_data[i].world_position.x,
					   entity_a.position_data[i].world_position.y,
					   entity_a.position_data[i].world_position.z);
			glEnd();
			
			glBegin(GL_LINES);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(entity_a.position_data[i].world_position.x,
					   entity_a.position_data[i].world_position.y,
					   entity_a.position_data[i].world_position.z);
			
			glVertex3f(entity_a.position_data[i].world_position.x + entity_a.position_data[i].world_orientation.floats[0][0],
					   entity_a.position_data[i].world_position.y + entity_a.position_data[i].world_orientation.floats[0][1],
					   entity_a.position_data[i].world_position.z + entity_a.position_data[i].world_orientation.floats[0][2]);
					   
			
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(entity_a.position_data[i].world_position.x,
					   entity_a.position_data[i].world_position.y,
					   entity_a.position_data[i].world_position.z);
			
			glVertex3f(entity_a.position_data[i].world_position.x + entity_a.position_data[i].world_orientation.floats[1][0],
					   entity_a.position_data[i].world_position.y + entity_a.position_data[i].world_orientation.floats[1][1],
					   entity_a.position_data[i].world_position.z + entity_a.position_data[i].world_orientation.floats[1][2]);
			
			
			glColor3f(0.0, 0.0, 1.0);
			glVertex3f(entity_a.position_data[i].world_position.x,
					   entity_a.position_data[i].world_position.y,
					   entity_a.position_data[i].world_position.z);
			
			glVertex3f(entity_a.position_data[i].world_position.x + entity_a.position_data[i].world_orientation.floats[2][0],
					   entity_a.position_data[i].world_position.y + entity_a.position_data[i].world_orientation.floats[2][1],
					   entity_a.position_data[i].world_position.z + entity_a.position_data[i].world_orientation.floats[2][2]);
			
			glEnd();		   		   
			
					   		   	
		}
		glDisable(GL_POINT_SMOOTH);
		glPointSize(1.0);
		glEnable(GL_DEPTH_TEST);
	}
	
	if(debug_flags & DEBUG_DRAW_ENTITY_AABB)
	{
		//for(i = 0; i < c; i++)
		//{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(12.0);
		
		for(i = 0; i < c; i++)
		{	
			
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
			glBegin(GL_QUADS);
			glColor3f(e_aabb_color[0], e_aabb_color[1], e_aabb_color[2]);
			
			glVertex3f(entity_a.aabb_data[i].origin.x - entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y + entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z + entity_a.aabb_data[i].c_maxmins[2]);
					   
			glVertex3f(entity_a.aabb_data[i].origin.x - entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y - entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z + entity_a.aabb_data[i].c_maxmins[2]);
			
			glVertex3f(entity_a.aabb_data[i].origin.x + entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y - entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z + entity_a.aabb_data[i].c_maxmins[2]);
					   
			glVertex3f(entity_a.aabb_data[i].origin.x + entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y + entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z + entity_a.aabb_data[i].c_maxmins[2]);
		
			
			
			glVertex3f(entity_a.aabb_data[i].origin.x - entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y + entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z - entity_a.aabb_data[i].c_maxmins[2]);
					   
			glVertex3f(entity_a.aabb_data[i].origin.x - entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y - entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z - entity_a.aabb_data[i].c_maxmins[2]);
			
			glVertex3f(entity_a.aabb_data[i].origin.x + entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y - entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z - entity_a.aabb_data[i].c_maxmins[2]);
					   
			glVertex3f(entity_a.aabb_data[i].origin.x + entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y + entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z - entity_a.aabb_data[i].c_maxmins[2]);
					   
			
			
			
			glVertex3f(entity_a.aabb_data[i].origin.x + entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y + entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z - entity_a.aabb_data[i].c_maxmins[2]);
					   
			glVertex3f(entity_a.aabb_data[i].origin.x + entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y - entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z - entity_a.aabb_data[i].c_maxmins[2]);
			
			glVertex3f(entity_a.aabb_data[i].origin.x + entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y - entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z + entity_a.aabb_data[i].c_maxmins[2]);
					   
			glVertex3f(entity_a.aabb_data[i].origin.x + entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y + entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z + entity_a.aabb_data[i].c_maxmins[2]);
					   
					   
					   
			
			glVertex3f(entity_a.aabb_data[i].origin.x - entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y + entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z - entity_a.aabb_data[i].c_maxmins[2]);
					   
			glVertex3f(entity_a.aabb_data[i].origin.x - entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y - entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z - entity_a.aabb_data[i].c_maxmins[2]);
			
			glVertex3f(entity_a.aabb_data[i].origin.x - entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y - entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z + entity_a.aabb_data[i].c_maxmins[2]);
					   
			glVertex3f(entity_a.aabb_data[i].origin.x - entity_a.aabb_data[i].c_maxmins[0], 
					   entity_a.aabb_data[i].origin.y + entity_a.aabb_data[i].c_maxmins[1],
					   entity_a.aabb_data[i].origin.z + entity_a.aabb_data[i].c_maxmins[2]);	   		   
					   
 
			glEnd();		   		  		   		   
			   		   		   		   	
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
		glDisable(GL_POINT_SMOOTH);
		glPointSize(1.0);
		glEnable(GL_DEPTH_TEST);
	}
	
	c = collider_a.count;
	if(debug_flags & DEBUG_DRAW_COLLIDERS)
	{
		glPointSize(4.0);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		
		for(i = 0; i < c; i++)
		{
			if(collider_a.colliders[i].base.type != COLLIDER_CHARACTER_CONTROLLER)
			{
				btConvexHullShape *collision_shape = (btConvexHullShape*) collider_a.colliders[i].base.rigid_body->getCollisionShape();
				btVector3 *verts = collision_shape->getUnscaledPoints();
				l = collision_shape->getNumPoints();
				
				btVector3 p = collider_a.colliders[i].base.rigid_body->getCenterOfMassPosition();
				btMatrix3x3 rot = collider_a.colliders[i].base.rigid_body->getCenterOfMassTransform().getBasis(); 
				vec3_t tp;
				
				glBegin(GL_POINTS);
				if(!collider_a.colliders[i].base.rigid_body->isActive())
				{
					glColor3f(0.2, 0.5, 0.3);
				}
				else
				{
					glColor3f(0.2, 1.0, 0.3);
				}
				
				for(j = 0; j < l;)
				{
					tp.x = verts[j][0] * rot[0][0] + verts[j][1] * rot[0][1] + verts[j][2] * rot[0][2];
					tp.y = verts[j][0] * rot[1][0] + verts[j][1] * rot[1][1] + verts[j][2] * rot[1][2];
					tp.z = verts[j][0] * rot[2][0] + verts[j][1] * rot[2][1] + verts[j][2] * rot[2][2];
					glVertex3f(tp.x + p[0], tp.y + p[1], tp.z + p[2]);	
					
					j++;
					
					/*tp.x = verts[j][0] * rot[0][0] + verts[j][1] * rot[0][1] + verts[j][2] * rot[0][2];
					tp.y = verts[j][0] * rot[1][0] + verts[j][1] * rot[1][1] + verts[j][2] * rot[1][2];
					tp.z = verts[j][0] * rot[2][0] + verts[j][1] * rot[2][1] + verts[j][2] * rot[2][2];
					glVertex3f(tp.x + p[0], tp.y + p[1], tp.z + p[2]);	*/
					
					//glVertex3f(0.0, 0.0, 0.0);
				}
				glEnd();
			}
		
		}
		glPointSize(1.0);
		glDisable(GL_POINT_SMOOTH);
		glEnable(GL_DEPTH_TEST);
	}
	
	if(debug_flags & DEBUG_DRAW_ARMATURES)
	{
		draw_debug_DrawArmatures();
	}
	
	if(debug_flags & DEBUG_DRAW_NBUFFER)
	{
		draw_debug_DrawNBuffer();
	}
	
	if(debug_flags & DEBUG_DRAW_ZBUFFER)
	{
		draw_debug_DrawZBuffer();
	}
	
	if(debug_flags & DEBUG_DRAW_DBUFFER)
	{
		draw_debug_DrawDBuffer();
	}
	
	
	
	used_floats = 0;
	draw_cmd_count = 0;
	
	glMatrixMode(GL_PROJECTION);
	//glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	//glPopMatrix();
	
}

PEWAPI void draw_debug_DrawTriangle(vec3_t a, vec3_t b, vec3_t c, vec3_t a_color, vec3_t b_color, vec3_t c_color)
{
	debug_draw_t d;
	
	if(debug_flags && used_floats + 11 < FLOAT_BUFFER_SIZE)
	{
		d.data = &float_buffer[used_floats];
		d.count = 1;
		d.type = DRAW_TRIANGLE;
		
		float_buffer[used_floats++] = a_color.r;
		float_buffer[used_floats++] = a_color.g;
		float_buffer[used_floats++] = a_color.b;
		
		float_buffer[used_floats++] = a.x;
		float_buffer[used_floats++] = a.y;
		float_buffer[used_floats++] = a.z;
		
		float_buffer[used_floats++] = b_color.r;
		float_buffer[used_floats++] = b_color.g;
		float_buffer[used_floats++] = b_color.b;
		
		float_buffer[used_floats++] = b.x;
		float_buffer[used_floats++] = b.y;
		float_buffer[used_floats++] = b.z;
		
		float_buffer[used_floats++] = c_color.r;
		float_buffer[used_floats++] = c_color.g;
		float_buffer[used_floats++] = c_color.b;
		
		float_buffer[used_floats++] = c.x;
		float_buffer[used_floats++] = c.y;
		float_buffer[used_floats++] = c.z;
		
		
		draw_cmds[draw_cmd_count++] = d;
	}
}

PEWAPI void draw_debug_DrawLine(vec3_t from, vec3_t to, vec3_t color, float line_thickness, int b_xray, int b_homogeneous, int b_clear_stencil)
{
	debug_draw_t d;
	
	if(debug_flags && used_floats + 12 < FLOAT_BUFFER_SIZE)
	{
		d.data = &float_buffer[used_floats];
		d.count = 1;
		if(!b_homogeneous)
			d.type = DRAW_LINE;
		else
			d.type = DRAW_LINE_HOMOGENEOUS;
		
		float_buffer[used_floats++] = color.r;
		float_buffer[used_floats++] = color.g;
		float_buffer[used_floats++] = color.b;
		
		float_buffer[used_floats++] = from.x;
		float_buffer[used_floats++] = from.y;
		float_buffer[used_floats++] = from.z;
		
		float_buffer[used_floats++] = to.x;
		float_buffer[used_floats++] = to.y;
		float_buffer[used_floats++] = to.z;
		
		float_buffer[used_floats++] = line_thickness;
		float_buffer[used_floats++] = b_xray;
		float_buffer[used_floats++] = b_clear_stencil;
		
		draw_cmds[draw_cmd_count++] = d;
	}
}


PEWAPI void draw_debug_DrawLineLoop(vec3_t origin, float *verts, int count, vec3_t color, float line_thickness, int b_xray, int homogeneous)
{
	debug_draw_t d;
	
	if(debug_flags && used_floats + 11 < FLOAT_BUFFER_SIZE)
	{
		d.data = &float_buffer[used_floats];
		d.count = 1;
		if(!homogeneous)
			d.type = DRAW_LINE_LOOP;
		else
			d.type = DRAW_LINE_LOOP_HOMOGENEOUS;
		
		float_buffer[used_floats++] = color.r;
		float_buffer[used_floats++] = color.g;
		float_buffer[used_floats++] = color.b;
		
		float_buffer[used_floats++] = origin.x;
		float_buffer[used_floats++] = origin.y;
		float_buffer[used_floats++] = origin.z;
		
		*(int *)&float_buffer[used_floats++] = (int)verts;
		*(int *)&float_buffer[used_floats++] = count;
		
		float_buffer[used_floats++] = line_thickness;
		float_buffer[used_floats++] = b_xray;
		
		draw_cmds[draw_cmd_count++] = d;
	}
}

PEWAPI void draw_debug_Draw2DLine(vec2_t from, vec2_t to, vec3_t color)
{
	/*glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(&camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix.floats[0][0]);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&camera_a.cameras[renderer.active_camera_index].projection_matrix.floats[0][0]);
	glUseProgram(0);
	glBegin(GL_LINES);
	glColor3f(color.floats[0], color.floats[1] ,color.floats[2]);
	glVertex3f(from.floats[0], from.floats[1],-1.0);
	glVertex3f(to.floats[0], to.floats[1],-1.0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glUseProgram(shader_a.shaders[renderer.active_shader_index].shader_ID);*/
}

PEWAPI void draw_debug_DrawPoint(vec3_t position, vec3_t color, float point_size, int homogeneous)
{
	debug_draw_t d;
	
	if(debug_flags && used_floats + 7 < FLOAT_BUFFER_SIZE)
	{
		d.data = &float_buffer[used_floats];
		d.count = 1;
		if(!homogeneous)
			d.type = DRAW_POINT;
		else
			d.type = DRAW_POINT_HOMOGENEOUS;
			
			
		float_buffer[used_floats++] = color.r;
		float_buffer[used_floats++] = color.g;
		float_buffer[used_floats++] = color.b;
		
		float_buffer[used_floats++] = position.x;
		float_buffer[used_floats++] = position.y;
		float_buffer[used_floats++] = position.z;
		float_buffer[used_floats++] = point_size;
		
		draw_cmds[draw_cmd_count++] = d;
	}

}

PEWAPI void draw_debug_DrawPointHomogeneous(vec3_t position, vec3_t color, float point_size)
{
	
}

PEWAPI void draw_debug_DrawFrustum(vec3_t origin, mat3_t *orientation, frustum_t *frustum)
{
	debug_draw_t d;
	
	if(debug_flags && used_floats + 7 < FLOAT_BUFFER_SIZE)
	{
		d.data = &float_buffer[used_floats];
		d.count = 1;
		d.type = DRAW_FRUSTUM;

		
		float_buffer[used_floats++] = origin.x;
		float_buffer[used_floats++] = origin.y;
		float_buffer[used_floats++] = origin.z;
		
		memcpy(float_buffer + used_floats, orientation, sizeof(mat3_t));
		used_floats += 9;
		
		memcpy(float_buffer + used_floats, frustum, sizeof(frustum_t));
		used_floats += 6;
		
		
		
		
		
		draw_cmds[draw_cmd_count++] = d;
	}
}


PEWAPI void draw_debug_DrawOutline(vec3_t position, mat3_t *orientation, mesh_t *mesh, vec3_t color, float line_thickness, int b_xray)
{
	debug_draw_t d;
	int i;
	if(debug_flags)
	{
		d.data = &float_buffer[used_floats];
		d.count = 1;
		d.type = DRAW_OUTLINE;
		
		float_buffer[used_floats++] = position.x;
		float_buffer[used_floats++] = position.y;
		float_buffer[used_floats++] = position.z;
		for(i = 0; i < 9; i++)
		{
			float_buffer[used_floats++] = orientation->lfloats[i];
			//float_buffer[used_floats++] = orientation->floats[0][0];
			//float_buffer[used_floats++] = orientation->floats[0][0];
		}
		
		/* this might break in 64 bit... */
		*(mesh_t **)&float_buffer[used_floats++] = mesh;
		
		
		float_buffer[used_floats++] = color.r;
		float_buffer[used_floats++] = color.g;
		float_buffer[used_floats++] = color.b;
		
		float_buffer[used_floats++] = line_thickness;
		float_buffer[used_floats++] = b_xray;
		
		
		draw_cmds[draw_cmd_count++] = d;
		
		//float_buffer[used_floats++] = orientation->floats[0][0];
		//float_buffer[used_floats++] = orientation->floats[0][0];
		//float_buffer[used_floats++] = position.z;
		
		
	}
}

PEWAPI void draw_debug_DrawBrushOutline(bmodel_ptr brush, vec3_t color)
{
	debug_draw_t d;
	int i;
	if(debug_flags)
	{
		d.data = &float_buffer[used_floats];
		d.count = 1;
		d.type = DRAW_BRUSH_OUTLINE;
	
		*(float **)&float_buffer[used_floats++] = brush.draw_data->verts;
		*(int *)&float_buffer[used_floats++] = brush.draw_data->vert_count;
		float_buffer[used_floats++] = color.r;
		float_buffer[used_floats++] = color.g;
		float_buffer[used_floats++] = color.b;
		
		draw_cmds[draw_cmd_count++] = d;
	}
}

PEWAPI void draw_debug_BlitFramebuffer(framebuffer_t *framebuffer, int offset_x, int offset_y)
{
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer->id);
	//glBlitFramebuffer(0, 0, framebuffer->width, framebuffer->height, offset_x, offset_y, framebuffer->width, framebuffer->height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	debug_draw_t d;
	int i;
	if(debug_flags)
	{
		d.data = &float_buffer[used_floats];
		d.count = 1;
		d.type = DRAW_BLIT_FRAMEBUFFER;
	
		*(framebuffer_t **)&float_buffer[used_floats++] = framebuffer;
		*(int *)&float_buffer[used_floats++] = offset_x;
		*(int *)&float_buffer[used_floats++] = offset_y;
		draw_cmds[draw_cmd_count++] = d;
	}
	
}

PEWAPI void draw_debug_Draw3DHandle(int mode, vec3_t position, mat3_t *orientation)
{
	
}

PEWAPI void draw_debug_Draw2DPoint(vec2_t position, vec3_t color, float point_size)
{
	/*glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(&camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix.floats[0][0]);
	glUseProgram(0);
	glPointSize(point_size);
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	glColor3f(color.floats[0], color.floats[1] ,color.floats[2]);
	glVertex3f(position.floats[0], position.floats[1],-0.11);
	glEnd();
	glPointSize(1.0);
	glPopMatrix();
	glDisable(GL_POINT_SMOOTH);
	glUseProgram(shader_a.shaders[renderer.active_shader_index].shader_ID);*/
}


/*PEWAPI void draw_debug_DrawPointHomogeneous(vec3_t position, vec3_t color, float point_size)
{
	//camera_t *active_camera = camera_GetActiveCamera();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_BLEND);

	framebuffer_BindFramebuffer(&backbuffer);
	glUseProgram(0);
	glPointSize(point_size);
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	glColor3f(color.floats[0], color.floats[1], color.floats[2]);
	glVertex3f(position.floats[0],position.floats[1], position.floats[2]);
	glEnd();
	glPointSize(1.0);
	glDisable(GL_POINT_SMOOTH);
	//framebuffer_BindFramebuffer(&geometry_buffer);
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glUseProgram(shader_a.shaders[renderer.active_shader_index].shader_ID);
}*/

/*PEWAPI void draw_debug_DrawLineHomogeneous(vec3_t a, vec3_t b, vec3_t color)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_BLEND);

	framebuffer_BindFramebuffer(&backbuffer);
	glUseProgram(0);
	//glPointSize(point_size);
	//glEnable(GL_POINT_SMOOTH);
	glBegin(GL_LINES);
	glColor3f(color.floats[0], color.floats[1], color.floats[2]);
	glVertex3f(a.floats[0], a.floats[1], a.floats[2]);
	glVertex3f(b.floats[0], b.floats[1], b.floats[2]);
	glEnd();
	
	//glPointSize(1.0);
	//glDisable(GL_POINT_SMOOTH);
	framebuffer_BindFramebuffer(&geometry_buffer);
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glUseProgram(shader_a.shaders[renderer.active_shader_index].shader_ID);
}*/



void draw_debug_DrawEntitiesAABBs()
{
	
}






void draw_debug_DrawLights(int b_draw_limits)
{
	/*mat4_t light_transform;
	mat4_t view_matrix;
	mat4_t model_view_matrix;
	float *gpu_buffer;
	float radius;
	float f;
	int i;
	int j;
	int k = active_light_a.light_count;
	vec3_t lpos;
	vec3_t v;
	mat3_t id = mat3_t_id();
	float c;
	glDisable(GL_BLEND);
	//vec3_t v2;
	//vec3_t v3;
	mat3_t light_world_matrix;
	//mat3_t v_space;
	//mat3_t v2_space;
	//mat3_t v3_space;
	//mat3_t v4_space;
	//mat3_t v5_space;
	//mat3_t v6_space;
	//mat4_t_compose(&light_transform, &position_data->world_orientation, lpos);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&camera_a.cameras[renderer.active_camera_index].projection_matrix.floats[0][0]);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//glLoadMatrixf(&camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix.floats[0][0]);
	
	view_matrix = camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix;
	

	framebuffer_BindFramebuffer(&backbuffer);
	glUseProgram(shader_a.shaders[wireframe_shader_index].shader_ID);
	glBindBuffer(GL_ARRAY_BUFFER, debug_buffer.buffer_ID);
	glEnableVertexAttribArray(shader_a.shaders[wireframe_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[wireframe_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glDisable(GL_CULL_FACE);
	
	v=vec3(0.65, 0.65, 1.0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &v.floats[0]);
	
	for(j = 0; j < k; j++)
	{
		
		//gpu_buffer=gpu_MapGPUBuffer(&debug_buffer, GL_READ_WRITE);
		
		
		
		gpu_buffer = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		
		lpos.floats[0] = active_light_a.position_data[j].world_position.floats[0];
		lpos.floats[1] = active_light_a.position_data[j].world_position.floats[1];
		lpos.floats[2] = active_light_a.position_data[j].world_position.floats[2];
		radius = active_light_a.position_data[j].radius;
		
		gpu_buffer[0]=0.0;
		gpu_buffer[1]=0.0;
		gpu_buffer[2]=0.0;
		
		glUnmapBuffer(GL_ARRAY_BUFFER);

		
		if(b_draw_limits)
		{
			gpu_buffer = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			
			if(active_light_a.position_data[j].bm_flags & LIGHT_POINT)
			{
				mat4_t_compose(&light_transform, &id, lpos);
				mat4_t_mult(&model_view_matrix, &light_transform, &view_matrix);
				glLoadMatrixf(&model_view_matrix.floats[0][0]);
				
				for(i=0; i<48; i++)
				{
					gpu_buffer[3+i*3]=(circle_v[i*3]*radius);
					gpu_buffer[3+i*3+1]=(circle_v[i*3+1]*radius);
					gpu_buffer[3+i*3+2]=(circle_v[i*3+2]*radius);
				}
				glUnmapBuffer(GL_ARRAY_BUFFER);
				
				glEnable(GL_POINT_SMOOTH);
				glPointSize(16.0);
				glDrawArrays(GL_POINTS, 0, 1);
				glPointSize(1.0);
				glDisable(GL_POINT_SMOOTH);
				
				glDrawArrays(GL_LINE_LOOP, 1, 16);
				glDrawArrays(GL_LINE_LOOP, 17, 16);
				glDrawArrays(GL_LINE_LOOP, 33, 16);
			}
			else if(active_light_a.position_data[j].bm_flags & LIGHT_SPOT)
			{
				
				mat4_t_compose(&light_transform, &active_light_a.position_data[j].world_orientation, lpos);
				mat4_t_mult(&model_view_matrix, &light_transform, &view_matrix);
				glLoadMatrixf(&model_view_matrix.floats[0][0]);
				
				gpu_buffer[3] = 0.0;
				gpu_buffer[4] = 0.0;
				gpu_buffer[5] = 0.0;
				
				c = cos(DegToRad((float)active_light_a.position_data[j].spot_co));
				c *= radius;
				for(i = 1; i<17; i++)
				{
					gpu_buffer[3+3*i] = (cone_v[i*3]*c);
					gpu_buffer[3+3*i+1] = (cone_v[i*3+1]*c);
					gpu_buffer[3+3*i+2] =  -radius;
				}
				gpu_buffer[3+3*i] = (cone_v[3]*c);
				gpu_buffer[3+3*i+1] = (cone_v[4]*c);
				gpu_buffer[3+3*i+2] = -radius;
				
				glUnmapBuffer(GL_ARRAY_BUFFER);
				
				glEnable(GL_POINT_SMOOTH);
				glPointSize(16.0);
				glDrawArrays(GL_POINTS, 0, 1);
				glPointSize(1.0);
				glDisable(GL_POINT_SMOOTH);
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawArrays(GL_TRIANGLE_FAN, 1, 18);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
		else
		{
			mat4_t_compose(&light_transform, &id, lpos);
			mat4_t_mult(&model_view_matrix, &light_transform, &view_matrix);
			glLoadMatrixf(&model_view_matrix.floats[0][0]);
			
			glEnable(GL_POINT_SMOOTH);
			glPointSize(16.0);
			glDrawArrays(GL_POINTS, 0, 1);
			glPointSize(1.0);
			glDisable(GL_POINT_SMOOTH);
		}
				
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glPointSize(1.0);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_CULL_FACE);
	glUseProgram(shader_a.shaders[renderer.active_shader_index].shader_ID);
	//framebuffer_BindFramebuffer(NULL);*/
}


void draw_debug_DrawArmatures()
{
	int i;
	int c;
	int j;
	int k;
	float *gpu_buffer;
	c = armature_list.count;
	bone_t *root;
	armature_t *a;
	camera_t *active_camera = camera_GetActiveCamera();
	mat4_t view_matrix = active_camera->world_to_camera_matrix;
	mat4_t bone_transform;
	//mat4_t temp;
	//mat4_t temp2;
	//mat4_t parent_bone_transform;
	mat4_t local_bone_transform;
	mat4_t model_view_matrix;
	mat4_t armature_transform;
	int q = 0;
	mat4_t transform_stack_base[65];
	mat4_t *transform_stack = &transform_stack_base[1];
	int stack_top = -1;
	bone_t *bone_stack[64];
	int processed_child_stack[64];
	vec3_t p;
	vec3_t t;
	
	float color[3] = {0.8, 0.8, 0.8};
	
	framebuffer_BindFramebuffer(&backbuffer);
	glUseProgram(0);
	//glUseProgram(shader_a.shaders[wireframe_shader_index].shader_ID);
	//glBindBuffer(GL_ARRAY_BUFFER, debug_buffer.buffer_ID);
	//glEnableVertexAttribArray(shader_a.shaders[wireframe_shader_index].v_position);
	//glVertexAttribPointer(shader_a.shaders[wireframe_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	//gpu_buffer = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	
	/*glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	
	for(j = 0; j < 72; j++)
	{
		gpu_buffer[j] = bone_mesh_octahedron[j];
	}
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);*/
	glPointSize(10.0);   
	glLineWidth(8.0);
	glEnable(GL_POINT_SMOOTH);

	for(i = 0; i < c; i++)
	{
			
		a = &armature_list.armatures[i];
		root = a->bones;
		processed_child_stack[0] = 0;
		transform_stack[-1] = mat4_t_id();
		//transform_stack[0] = local_bone_transform;
		stack_top = 0;
		
		mat4_t_compose(&armature_transform, &a->world_orientation, a->world_position);
		
		while(1)
		{
			
			
			mat4_t_compose(&local_bone_transform, &root->orientation, root->position);
			mat4_t_mult(&bone_transform, &local_bone_transform, &transform_stack[stack_top - 1]);
			//MatrixCopy4(&transform_stack[stack_top], &bone_transform);
			memcpy(&transform_stack[stack_top].floats[0][0], &bone_transform.floats[0][0], sizeof(mat4_t));
			
			_lb:
				
			processed_child_stack[stack_top + 1] = 0;
			
			if(processed_child_stack[stack_top] < root->child_count)
			{
				root = root->children[processed_child_stack[stack_top]];
				processed_child_stack[stack_top]++;
				stack_top++;
				continue;
			}
			
			//MatrixCopy4(&local_bone_transform, &transform_stack[stack_top]);
			memcpy(&local_bone_transform.floats[0][0], &transform_stack[stack_top].floats[0][0], sizeof(mat4_t));
			mat4_t_mult(&bone_transform, &local_bone_transform, &armature_transform);
			
			p.x = bone_transform.floats[3][0];
			p.y = bone_transform.floats[3][1];
			p.z = bone_transform.floats[3][2];			
				
			t.x = root->tip.x * bone_transform.floats[0][0] + 
				  root->tip.y * bone_transform.floats[1][0] +
				  root->tip.z * bone_transform.floats[2][0] + bone_transform.floats[3][0];
				
			t.y = root->tip.x * bone_transform.floats[0][1] + 
				  root->tip.y * bone_transform.floats[1][1] +
				  root->tip.z * bone_transform.floats[2][1] + bone_transform.floats[3][1];      
				
			t.z = root->tip.x * bone_transform.floats[0][2] + 
				  root->tip.y * bone_transform.floats[1][2] +
				  root->tip.z * bone_transform.floats[2][2] + bone_transform.floats[3][2];
			
			glBegin(GL_LINES);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(p.x, p.y, p.z);
			glColor3f(0.2, 0.2, 0.2);
			glVertex3f(t.x, t.y, t.z);
			glEnd();
			
			glBegin(GL_POINTS);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(p.x, p.y, p.z);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(t.x, t.y, t.z);
			glEnd();
			
			root = root->parent;
			stack_top--;
			if(!root) break;
			goto _lb;
			
				//printf("\n\n\ncurrent root: %s\n", root->name);
			/*if(root->child_count)
			{	
				
				if(processed_child_stack[stack_top + 1] >= root->child_count)
				{
					// All children of this bone have been drawn. Draw it then. 
					goto _draw_parent;
				}
				stack_top++;

																 
				mat4_t_compose(&local_bone_transform, &root->orientation, root->position);
				mat4_t_mult(&bone_transform, &local_bone_transform, &transform_stack[stack_top - 1]);
				transform_stack[stack_top] = bone_transform;
				root = root->children[processed_child_stack[stack_top]]; 
				processed_child_stack[stack_top]++;
					
				// clear the next stack slot 
				processed_child_stack[stack_top + 1] = 0;
					
			}
			else
			{
				_draw_parent:

				//mat4_t_compose(&local_bone_transform, &root->orientation, root->position);
				//mat4_t_mult(&bone_transform, &local_bone_transform, &transform_stack[stack_top]);
				bone_transform = transform_stack[stack_top];
				
				p.x = root->position.x * bone_transform.floats[0][0] + 
				      root->position.y * bone_transform.floats[0][1] +
				      root->position.z * bone_transform.floats[0][2] + bone_transform.floats[3][0];
				
				p.y = root->position.x * bone_transform.floats[1][0] + 
				      root->position.y * bone_transform.floats[1][1] +
				      root->position.z * bone_transform.floats[1][2] + bone_transform.floats[3][1];      
				
				p.z = root->position.x * bone_transform.floats[2][0] + 
				      root->position.y * bone_transform.floats[2][1] +
				      root->position.z * bone_transform.floats[2][2] + bone_transform.floats[3][2];
				      
				
				t.x = (root->tip.x + root->position.x) * bone_transform.floats[0][0] + 
				      (root->tip.y + root->position.y) * bone_transform.floats[0][1] +
				      (root->tip.z + root->position.z) * bone_transform.floats[0][2] + bone_transform.floats[3][0];
				
				t.y = (root->tip.x + root->position.x) * bone_transform.floats[1][0] + 
				      (root->tip.y + root->position.y) * bone_transform.floats[1][1] +
				      (root->tip.z + root->position.z) * bone_transform.floats[1][2] + bone_transform.floats[3][1];      
				
				t.z = (root->tip.x + root->position.x) * bone_transform.floats[2][0] + 
				      (root->tip.y + root->position.y) * bone_transform.floats[2][1] +
				      (root->tip.z + root->position.z) * bone_transform.floats[2][2] + bone_transform.floats[3][2];
    

				glBegin(GL_LINES);
				glColor3f(1.0, 1.0, 1.0);
				glVertex3f(p.x, p.y, p.z);
				glColor3f(0.2, 0.2, 0.2);
				glVertex3f(t.x, t.y, t.z);
				glEnd();
				
				glBegin(GL_POINTS);
				glColor3f(0.6, 0.6, 1.0);
				glVertex3f(p.x, p.y, p.z);
				glVertex3f(t.x, t.y, t.z);
				glEnd();

				
				
				root = root->parent;
				stack_top --;
				if(!root) break;
			}*/

		}
			
			
	}
	
	glPointSize(1.0);
	glLineWidth(1.0);
	glDisable(GL_POINT_SMOOTH);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glFlush();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	return;
}


void draw_debug_Constraints()
{
	
}


void draw_debug_DrawZBuffer()
{
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	float q;
	camera_t *active_camera = camera_GetActiveCamera();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	framebuffer_BindFramebuffer(&backbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, screen_area_mesh_gpu_buffer);
	
	shader_SetShaderByIndex(draw_z_buffer_shader_index);
	glEnableVertexAttribArray(shader_a.shaders[draw_z_buffer_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[draw_z_buffer_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetWidth, backbuffer.width);
	shader_SetCurrentShaderUniform1f(UNIFORM_RenderTargetHeight, backbuffer.height);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZNear, active_camera->frustum.znear);
	shader_SetCurrentShaderUniform1f(UNIFORM_ZFar, active_camera->frustum.zfar);
	shader_SetCurrentShaderUniform1i(UNIFORM_TextureSampler0, 0);
	
	//#define DEBUG_NBUFFER_HEIGHT 0.15
	
	//q = (float)backbuffer.width / (float)backbuffer.height;
	
	//printf("%f\n", q);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, geometry_buffer.z_buffer);
	/*glEnable(GL_TEXTURE_2D);
	
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0, -1.0 + DEBUG_NBUFFER_HEIGHT + DEBUG_NBUFFER_HEIGHT, 0.0);
	
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0 + DEBUG_NBUFFER_HEIGHT, 0.0);
	
	glTexCoord2f(1.0, 0.0);
	glVertex3f(q * 0.5 * DEBUG_NBUFFER_HEIGHT - 1.0, -1.0 + DEBUG_NBUFFER_HEIGHT, 0.0);
	
	glTexCoord2f(1.0, 1.0);
	glVertex3f(q * 0.5 * DEBUG_NBUFFER_HEIGHT - 1.0, -1.0 + DEBUG_NBUFFER_HEIGHT + DEBUG_NBUFFER_HEIGHT, 0.0);

	glEnd();*/
	
	glDrawArrays(GL_QUADS, 0, 4);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void draw_debug_DrawNBuffer()
{
	//glUseProgram(0);
	
	float q = backbuffer.width / backbuffer.height;	
	float h = 2.0 * SCALE;
	float w = h * q * SCALE;
	int iw = backbuffer.width * w;
	framebuffer_BindFramebuffer(&backbuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, geometry_buffer.id);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(0, 0, geometry_buffer.width, geometry_buffer.height, 0, backbuffer.height * 0.25, backbuffer.width * w * 2.0, backbuffer.height * 0.25 * 2.0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void draw_debug_DrawDBuffer()
{
	float q = backbuffer.width / backbuffer.height;	
	float h = 2.0 * SCALE;
	float w = h * q * SCALE;
	int iw = backbuffer.width * w;
	framebuffer_BindFramebuffer(&backbuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, geometry_buffer.id);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, geometry_buffer.width, geometry_buffer.height, 0, backbuffer.height * 0.25 * 2.0, backbuffer.width * w * 2.0, backbuffer.height * 0.25 * 3.0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}


void draw_debug_DrawVertexData(vec3_t position, vertex_data_t *data)
{
	mat4_t transform;
	mat4_t model_view_matrix;
	camera_t *active_camera;
	int i;
	if(data)
	{
		active_camera = camera_GetActiveCamera();
		transform = mat4_t_id();
		transform.floats[3][0] = position.x;
		transform.floats[3][1] = position.y;
		transform.floats[3][2] = position.z;
		
		mat4_t_mult(&model_view_matrix, &transform, &active_camera->world_to_camera_matrix);
		
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&model_view_matrix.floats[0][0]);
		glUseProgram(0);
		glPointSize(8.0);
		glEnable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
		
		glColor3f(0.2, 1.0, 0.2);
		for(i = 0; i < data->vertex_count; i++)
		{
			glVertex3fv(&data->position[i * 3]);
		}
		glEnd();
		
		glDisable(GL_CULL_FACE);
		glBegin(GL_TRIANGLES);
		//glColor3f(0.2, 0.2, 1.0);
		
		for(i = 0; i < data->vertex_count;)
		{
			//glColor3f(data->position[i * 3], data->position[i * 3 + 1], data->position[i * 3 + 2]);
			glColor3f((float)(rand()%0xffff)/(float)0xffff, (float)(rand()%0xffff)/(float)0xffff, (float)(rand()%0xffff)/(float)0xffff);
			glVertex3fv(&data->position[i * 3]);
			i++;
			glVertex3fv(&data->position[i * 3]);
			i++;
			glVertex3fv(&data->position[i * 3]);
			i++;
		}
		glEnd();
		/*glColor3f(0.2, 0.2, 1.0);
		glBegin(GL_LINES);
		for(i = 0; i < data->edge_count; i++)
		{
			glVertex3fv(&data->position[data->edges[i].vert0 * 3]);
			glVertex3fv(&data->position[data->edges[i].vert1 * 3]);
		}
		glEnd();*/
		glPointSize(1.0);
		glEnable(GL_CULL_FACE);
		glDisable(GL_POINT_SMOOTH);
		
		
		
		
	}
}


#ifdef _cplusplus
}
#endif




