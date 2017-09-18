#include "scenegraph.h"
#include "draw.h"
#include "draw_debug.h"
#include "entity.h"
#include "camera.h"
#include "particle.h"
#include "shader.h"
#include "light.h"
#include "macros.h"
#include "plane.h"
#include "physics.h"
#include "material.h"
#include "physics.h"
#include "vcache.h"
#include "gpu.h"
#include "armature.h"
#include "input.h"
#include "brush.h"
#include <intrin.h>


extern int debug_level;
scenegraph_t scenegraph;
extern material_array material_a;
extern renderer_t renderer;
extern render_queue shadow_q;
extern render_queue render_q;
extern render_queue t_render_q;
extern render_queue e_render_q;
extern entity_array entity_a;
extern entity_array static_entity_a;
extern brush_list_t brush_list;
extern brush_render_queue_t brush_render_queue;
extern camera_array camera_a;
extern collider_array collider_a;
//extern active_particle_system_array active_particle_system_a;
extern light_array light_a;
extern light_array active_light_a;
extern int *active_light_indexes;
extern mat4_t *active_light_transforms;
extern affecting_lights_list affecting_lights;
extern screen_tile_list screen_tiles;
extern armature_list_t armature_list;
extern input_cache input;
extern framebuffer_t picking_buffer;
extern framebuffer_t *cur_fb;

extern unsigned  int gpu_heap;

extern int engine_state;

extern int wireframe_shader_index;
extern int light_pick_shader;
extern int brush_pick_shader;

extern mat4_t cube_shadow_mats[6];

int get_entity_under_cursor = 0;
//int mouse_x;
//int mouse_y;

entity_ptr last_under_cursor;

/* near, left, right, up and down planes are the same for point lights, no need to regenerate them for every light. */
//static plane_t cached_planes[5];



static int sizes[6] = {0, 0, 0, 0, 0, 0};
static int *(rqs[6]);
static int used[6] = {0, 0, 0, 0, 0, 0};
static int check_values[6] = {0x00000065, 0x0000009a, 0x00000606, 0x00000909, 0x00000aa0, 0x00000550};

static vec3_t point_frustum_normals[6];
static plane_t point_frustum_planes[6];

unsigned int light_picking_vbo;

#define FRUSTUM_ANGLE 1.570796325


/* TODO: add a field to each node, called hint, which is
a hint to the scenegraph about the usage of that node.
Nodes that will not be parented or have anything parented
to them at runtime can be serialized in memory, to speed up 
processing. */

enum CORNER_EVALUATION
{
	EVALUATE_POSITIVE_X_POSITIVE_Y=0,
	EVALUATE_POSITIVE_X_NEGATIVE_Y,
	EVALUATE_NEGATIVE_X_POSITIVE_Y,
	EVALUATE_NEGATIVE_X_NEGATIVE_Y
};

static int corner[4][2]=
{
	2, 3,
	2, 1,
	0, 3,
	0, 1
};


typedef struct
{
	int list_size;
	int cursor;
	int *indexes;
}dispatch_indexes_list;		/* for separating scenegraph processing from geometry submission */

int b_static_visible;
int static_start;
dispatch_indexes_list dispatch_list;










#ifdef __cplusplus
extern "C"
{
#endif

static void scenegraph_PurgeNode(node_t *node);

static void scenegraph_ProcessNode(node_t *node, mat4_t *model_view_matrix);

static void scenegraph_ProcessNodes();

static void scenegraph_GetAffectedScreenTiles();

static void scenegraph_CullLights();

static void scenegraph_KnapSack();

static void scenegraph_CullGeometry();

static void scenegraph_CullBrushes();

static void scenegraph_CullStaticGeometry();

static void scenegraph_FillShadowQueue();

static void scenegraph_DispatchGeometry();

static void scenegraph_GetAffectingLights();

static void scenegraph_UpdateVertexCache();

static void scenegraph_UpdateColliders();

static void scenegraph_GroupPerHint();

/* extend the list inside a node. */
static void scenegraph_ExtendNodeChildrenList(node_t **node);

/* recompact the list of children of a node. Necessary after 
deleting one of the node's children. */
static void scenegraph_RecompactNodeChildrenList(node_t *node);





/*
=============
scenegraph_Init
=============
*/
void scenegraph_Init()
{
	scenegraph.graph_size=0;
	scenegraph.node_count=1;
	float c;
	float s;
	
	/* a full node + space for 16 child nodes. */
	scenegraph.root=(node_t *)malloc(sizeof(node_t)+15*sizeof(void *));
	scenegraph.root->children_count=0;
	scenegraph.root->max_children=16;
	scenegraph.root->name="root";
	scenegraph.root->index=-1;
	scenegraph.root->parent=NULL;
	scenegraph.root->node_index=-1;
	scenegraph.root->type=NODE_ROOT;
	scenegraph.root->parent_type=NODE_ROOT;
	scenegraph.root->children=NULL;
	
	dispatch_list.cursor=0;
	dispatch_list.indexes=(int *)calloc(128, sizeof(int));
	dispatch_list.list_size=128;
	
	
	rqs[0] = (int *)malloc(sizeof(int) * 64);
	sizes[0] = 64; 
	
	rqs[1] = (int *)malloc(sizeof(int) * 64);
	sizes[1] = 64;
	
	rqs[2] = (int *)malloc(sizeof(int) * 64);
	sizes[2] = 64;
	
	rqs[3] = (int *)malloc(sizeof(int) * 64);
	sizes[3] = 64;
	
	rqs[4] = (int *)malloc(sizeof(int) * 64);
	sizes[4] = 64;
	
	rqs[5] = (int *)malloc(sizeof(int) * 64);
	sizes[5] = 64;
	
	c = cos(3.14159265 * 0.25);
	s = sin(3.14159265 * 0.25);
	
	point_frustum_planes[0].normal = vec3(0.0, s, c);		/* p0 */	
	point_frustum_planes[1].normal = vec3(0.0, -s, c);		/* p1 */	
	point_frustum_planes[2].normal = vec3(c, 0.0, s);		/* p2 */
	point_frustum_planes[3].normal = vec3(-c, 0.0, s);		/* p3 */
	point_frustum_planes[4].normal = vec3(c, s, 0.0);		/* p4 */
	point_frustum_planes[5].normal = vec3(c, -s, 0.0);		/* p5 */
	
	glGenBuffers(1, &light_picking_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, light_picking_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 128, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}


/*
=============
scenegraph_Finish
=============
*/
void scenegraph_Finish()
{
	scenegraph_FreeGraph(scenegraph.root);
	free(rqs[0]);
	free(rqs[1]);
	free(rqs[2]);
	free(rqs[3]);
	free(rqs[4]);
	free(rqs[5]);
}


/*
=============
scenegraph_Finish
=============
*/
void scenegraph_FreeGraph(node_t *node)
{
	int i;
	int c;
	node_t *temp;
	
	if(node)
	{
		
		if(node->children_count)
		{
			c=node->children_count;
			
			for(i=0; i<c; i++)
			{
				temp=(*((node_t **)&node->children+i));
				scenegraph_FreeGraph(temp);
			}
		}
		free(node);
	}
}


PEWAPI void scenegraph_PurgeAllNodes()
{
	scenegraph_PurgeNode(scenegraph.root);
	//dispatch_list.cursor=0;
}


static void scenegraph_PurgeNode(node_t *node)
{
	int i;
	int c;
	node_t *temp;
	if(node)
	{
		
		if(node->children_count)
		{
			c=node->children_count;
			
			for(i=0; i<c; i++)
			{
				temp=(*((node_t **)&node->children+i));
				scenegraph_PurgeNode(temp);
			}
		}
		
		if(node!=scenegraph.root)
		{
			scenegraph_RemoveNode(node, 0);	
		}

	}
}

/*
=============
scenegraph_AddNode
=============
*/
node_t *scenegraph_AddNode(int type, int index, short sub_index, char *name)
{
	node_t *temp;
	if(scenegraph.root->children_count>=scenegraph.root->max_children)
	{
		scenegraph_ExtendNodeChildrenList(&scenegraph.root);
	}
	temp=(node_t *)malloc(sizeof(node_t)+7*sizeof(void *));
	temp->children_count=0;
	temp->max_children=8;
	temp->parent=scenegraph.root;
	temp->node_index=scenegraph.root->children_count;
	temp->type=type;
	temp->parent_type=NODE_ROOT;
	temp->index=index;
	temp->sub_index = sub_index;
	temp->name=name;
	*((node_t **)&scenegraph.root->children+scenegraph.root->children_count)=temp;
	scenegraph.node_count++;
	scenegraph.root->children_count++;
		
	return *((node_t **)&scenegraph.root->children+scenegraph.root->children_count-1);
}


/*
=============
scenegraph_RemoveNode
=============
*/
void scenegraph_RemoveNode(node_t *node, int b_recursive)
{
	int i;
	int c;
	int f;
	if(node)
	{
		*((node_t **)&node->parent->children+node->node_index)=NULL;
		scenegraph_RecompactNodeChildrenList(scenegraph.root);
		if(node->children_count)
		{
			c=node->children_count;
			for(i=0; i<c; i++)
			{
				scenegraph_SetParent(*((node_t **)&node->children+i), scenegraph.root, 0);
			}
		}
		f=node->node_index;
		free(node);
		//printf("node %d freed\n\n", f);
	}
}


/*
=============
scenegraph_ExtendNodeChildrenList
=============
*/
static void scenegraph_ExtendNodeChildrenList(node_t **node)
{
	int i;
	int c;
	node_t *temp;
	if(node)
	{
		if(*node)
		{
			/* linear increment instead of exponential... */
			(*node)->max_children+=16;
			temp=(node_t *)malloc(sizeof(node_t)+sizeof(void *)*((*node)->max_children));
			
			/* copy relevant parent-children info to the new node. Don't need to recursively do it, 
			because the children of its children won't ever notice the change */
			c=(*node)->children_count;
			for(i=0; i<c; i++)
			{
			
				(*((node_t **)&temp->children+i))=(*((node_t **)&(*node)->children+i));
				(*((node_t **)&temp->children+i))->parent=temp;
			
			}
			temp->parent=(*node)->parent;
			if((*node)->parent)
			{
				(*(((node_t **)&(*node)->parent->children)+(*node)->node_index))=temp;
			}
			temp->children_count=(*node)->children_count;
			temp->max_children=(*node)->max_children;
			temp->name=(*node)->name;
			temp->node_index=(*node)->node_index;
			temp->index=(*node)->index;
			temp->type=(*node)->type;
			temp->parent_type=(*node)->parent_type;
			free(*node);
			*node=temp;
		}
	}
}


/*
=============
scenegraph_RecompactNodeChildrenList
=============
*/
static void scenegraph_RecompactNodeChildrenList(node_t *node)
{
	int i;
	int t;
	int k;
	int c;
	int null_nodes=0;
	int null_start=0;
	int null_end=0;
	int offset=0;
	c=node->children_count;
	for(i=0; i<c; i++)
	{
		if(!(*((node_t **)&node->children+i)))
		{
			null_start=i;

			for(t=null_start+1; t<c; t++)
			{
				if((*((node_t **)&node->children+t)))
				{
					null_end=t;
					break;
				}
			}
			if(null_start>=null_end)		/* node is last in the list */
			{
				node->children_count--;
				return;
			}
			offset=null_end-null_start;	/* gap size */

			for(k=c-1; k>=c-offset; k--)
			{
				/* fill the gap with the end of the list */
				if((*((node_t **)&node->children+k)))
				{
					(*((node_t **)&node->children+null_start))=(*((node_t **)&node->children+k));
					(*((node_t **)&node->children+k))=NULL;
					(*((node_t **)&node->children+null_start))->node_index=null_start;
					
					switch((*((node_t **)&node->children+null_start))->type)
					{
						case NODE_ENTITY:
							entity_a.extra_data[(*((node_t **)&node->children+null_start))->index].assigned_node=(*((node_t **)&node->children+null_start));
						break;
						
						/*case NODE_PARTICLE:
							active_particle_system_a.systems[(*((node_t **)&node->children+null_start))->index].assigned_node=(*((node_t **)&node->children+null_start));
						break;*/
					}

					null_start++;
				}
				else offset++;
				
				node->children_count--;
				
			}
			c=node->children_count;	
		}
	}
}


/*
=============
scenegraph_SetParent
=============
*/
PEWAPI void scenegraph_SetParent(node_t *node, node_t *parent, int b_keep_transform)
{
	int i;
	int c;
	mat4_t transform;
	mat4_t parent_transform;
	mat4_t c_transform;
	//mat3_t rotation;
	//mat3_t parent_rotation;
	
	//mat4_t parent_orientation;
	//mat4_t parent_position;
	mat4_t world_to_parent_matrix;
	mat4_t new_position;
	//vec3_t v;
	mat3_t parent_orientation;
	vec3_t parent_position;
	mat3_t child_orientation;
	vec3_t child_position;
	
	
	
	/* if this node is already parented to this parent, do nothing */
	if(node->parent == parent)
	{
		return;
	}
	/* this node already has a parent, that differs from the parent we want to set. So, remove the old parent and 
	recompact its children list */
	else if(node->parent)
	{
		(*((node_t **)&node->parent->children+node->node_index))=NULL;
		scenegraph_RecompactNodeChildrenList(node->parent);
	}
	

	
		
	if(parent->children_count>=parent->max_children)
	{
		scenegraph_ExtendNodeChildrenList(&parent);
	}
	//i = node->node_index;
	node->node_index=parent->children_count;
	(*((node_t **)&parent->children+parent->children_count))=node;
	switch(parent->type)
	{
		case NODE_ROOT:
			node->parent_type = NODE_ROOT;
		break;
		
		case NODE_ENTITY:
			node->parent_type = NODE_ENTITY;
		break;
		
		case NODE_LIGHT:
			node->parent_type = NODE_LIGHT;
		break;
		
		case NODE_COLLIDER:
			node->parent_type = NODE_COLLIDER;
		break;
		
		case NODE_ARMATURE:
			node->parent_type = NODE_ARMATURE;
		break;
		
		case NODE_BONE:
			node->parent_type = NODE_BONE;
		break;
		
		case NODE_CAMERA:
			node->parent_type = NODE_CAMERA;
		break;

	}	
	node->parent=parent;
	
	if(b_keep_transform)
	{	
		if(node->type!=NODE_ROOT)
		{
			
			switch(node->parent_type)
			{
				case NODE_ENTITY:	
					//parent_orientation = entity_a.entity_position[parent->index].world_orientation;
					
					for(i=0; i<4; i++)
					{
						parent_orientation.floats[i][0] = entity_a.position_data[parent->index].world_orientation.floats[i][0];
						parent_orientation.floats[i][1] = entity_a.position_data[parent->index].world_orientation.floats[i][1];
						parent_orientation.floats[i][2] = entity_a.position_data[parent->index].world_orientation.floats[i][2];
						parent_orientation.floats[i][3] = 0.0;
					}
					parent_orientation.floats[3][3] = 1.0;
					
					
					
					parent_position.floats[0] = entity_a.position_data[parent->index].world_position.x;
					parent_position.floats[1] = entity_a.position_data[parent->index].world_position.y;
					parent_position.floats[2] = entity_a.position_data[parent->index].world_position.z;
				break;
				
				case NODE_LIGHT:
					parent_orientation = light_a.position_data[parent->index].world_orientation;
					parent_position.floats[0] = light_a.position_data[parent->index].world_position.floats[0];
					parent_position.floats[1] = light_a.position_data[parent->index].world_position.floats[1];
					parent_position.floats[2] = light_a.position_data[parent->index].world_position.floats[2];
				break;
				
				case NODE_CAMERA:
					parent_orientation = camera_a.cameras[parent->index].world_orientation;
					parent_position.floats[0] = camera_a.cameras[parent->index].world_position.floats[0];
					parent_position.floats[1] = camera_a.cameras[parent->index].world_position.floats[1];
					parent_position.floats[2] = camera_a.cameras[parent->index].world_position.floats[2];
				break;
				
				case NODE_COLLIDER:
					
				break;
			}
			
			
			/* create the inverse of the parent node's transform. So, transpose the rotation,
			and invert the translation. Easy. */
			mat3_t_transpose(&parent_orientation);
			
			world_to_parent_matrix.floats[0][0] = parent_orientation.floats[0][0];	
			world_to_parent_matrix.floats[0][1] = parent_orientation.floats[0][1];
			world_to_parent_matrix.floats[0][2] = parent_orientation.floats[0][2];
			world_to_parent_matrix.floats[0][3] = 0.0;
					
			world_to_parent_matrix.floats[1][0] = parent_orientation.floats[1][0];	
			world_to_parent_matrix.floats[1][1] = parent_orientation.floats[1][1];
			world_to_parent_matrix.floats[1][2] = parent_orientation.floats[1][2];
			world_to_parent_matrix.floats[1][3] = 0.0;	
					
			world_to_parent_matrix.floats[2][0] = parent_orientation.floats[2][0];	
			world_to_parent_matrix.floats[2][1] = parent_orientation.floats[2][1];
			world_to_parent_matrix.floats[2][2] = parent_orientation.floats[2][2];
			world_to_parent_matrix.floats[2][3] = 0.0;	
					
			world_to_parent_matrix.floats[3][0] = -parent_position.floats[0];	
			world_to_parent_matrix.floats[3][1] = -parent_position.floats[1];
			world_to_parent_matrix.floats[3][2] = -parent_position.floats[2];
			world_to_parent_matrix.floats[3][3] = 1.0;
		
			
			
			switch(node->type)
			{
				case NODE_ENTITY:
					mat4_t_compose(&transform, &entity_a.position_data[node->index].world_orientation, entity_a.position_data[node->index].world_position);
					mat4_t_mult(&new_position, &transform, &world_to_parent_matrix);
						
					entity_a.extra_data[node->index].local_position.floats[0] = new_position.floats[3][0];
					entity_a.extra_data[node->index].local_position.floats[1] = new_position.floats[3][1];
					entity_a.extra_data[node->index].local_position.floats[2] = new_position.floats[3][2];
					
					entity_a.extra_data[node->index].local_orientation.floats[0][0] = new_position.floats[0][0];
					entity_a.extra_data[node->index].local_orientation.floats[0][1] = new_position.floats[0][1];
					entity_a.extra_data[node->index].local_orientation.floats[0][2] = new_position.floats[0][2];
						
					entity_a.extra_data[node->index].local_orientation.floats[1][0] = new_position.floats[1][0];
					entity_a.extra_data[node->index].local_orientation.floats[1][1] = new_position.floats[1][1];
					entity_a.extra_data[node->index].local_orientation.floats[1][2] = new_position.floats[1][2];
						
					entity_a.extra_data[node->index].local_orientation.floats[2][0] = new_position.floats[2][0];
					entity_a.extra_data[node->index].local_orientation.floats[2][1] = new_position.floats[2][1];
					entity_a.extra_data[node->index].local_orientation.floats[2][2] = new_position.floats[2][2];	
				break;
					
				
				case NODE_LIGHT:
					mat4_t_compose(&transform, &light_a.position_data[node->index].world_orientation, vec4vec3(light_a.position_data[node->index].world_position));
					mat4_t_mult(&new_position, &transform, &world_to_parent_matrix);
						
					light_a.position_data[node->index].local_position.floats[0] = new_position.floats[3][0];
					light_a.position_data[node->index].local_position.floats[1] = new_position.floats[3][1];
					light_a.position_data[node->index].local_position.floats[2] = new_position.floats[3][2];
					
					light_a.position_data[node->index].local_orientation.floats[0][0] = new_position.floats[0][0];
					light_a.position_data[node->index].local_orientation.floats[0][1] = new_position.floats[0][1];
					light_a.position_data[node->index].local_orientation.floats[0][2] = new_position.floats[0][2];
						
					light_a.position_data[node->index].local_orientation.floats[1][0] = new_position.floats[1][0];
					light_a.position_data[node->index].local_orientation.floats[1][1] = new_position.floats[1][1];
					light_a.position_data[node->index].local_orientation.floats[1][2] = new_position.floats[1][2];
						
					light_a.position_data[node->index].local_orientation.floats[2][0] = new_position.floats[2][0];
					light_a.position_data[node->index].local_orientation.floats[2][1] = new_position.floats[2][1];
					light_a.position_data[node->index].local_orientation.floats[2][2] = new_position.floats[2][2];	
				break;
				
				/* well, looks like we can parent a camera to a camera... not sure why one would do that, but it is possible. */
				case NODE_CAMERA:
					mat4_t_compose(&transform, &camera_a.cameras[node->index].world_orientation, camera_a.cameras[node->index].world_position);
					mat4_t_mult(&new_position, &transform, &world_to_parent_matrix);
						
					camera_a.cameras[node->index].local_position.floats[0] = new_position.floats[3][0];
					camera_a.cameras[node->index].local_position.floats[1] = new_position.floats[3][1];
					camera_a.cameras[node->index].local_position.floats[2] = new_position.floats[3][2];
					
					camera_a.cameras[node->index].local_orientation.floats[0][0] = new_position.floats[0][0];
					camera_a.cameras[node->index].local_orientation.floats[0][1] = new_position.floats[0][1];
					camera_a.cameras[node->index].local_orientation.floats[0][2] = new_position.floats[0][2];
						
					camera_a.cameras[node->index].local_orientation.floats[1][0] = new_position.floats[1][0];
					camera_a.cameras[node->index].local_orientation.floats[1][1] = new_position.floats[1][1];
					camera_a.cameras[node->index].local_orientation.floats[1][2] = new_position.floats[1][2];
						
					camera_a.cameras[node->index].local_orientation.floats[2][0] = new_position.floats[2][0];
					camera_a.cameras[node->index].local_orientation.floats[2][1] = new_position.floats[2][1];
					camera_a.cameras[node->index].local_orientation.floats[2][2] = new_position.floats[2][2];
				break;
			}
			
		}
	}
	
	parent->children_count++;
}


/*
=============
scenegraph_RemoveParent
=============
*/
PEWAPI void scenegraph_RemoveParent(node_t *node, int b_keep_transform)
{
	int b;
	mat4_t transform;
	mat4_t parent_transform;
	mat4_t c_transform;
	mat3_t rotation;
	mat2_t m;
	mat3_t parent_rotation;
	vec3_t v;
	if(node)
	{
		if(node->parent!=scenegraph.root)
		{
			(*((node_t **)&node->parent->children+node->node_index))=NULL;
			scenegraph_RecompactNodeChildrenList(node->parent);
			/*if(b_keep_transform)
			{	
				entity_a.position_data[node->index].local_position.floats[0]+=entity_a.position_data[node->parent->index].local_position.floats[0];
				entity_a.position_data[node->index].local_position.floats[1]+=entity_a.position_data[node->parent->index].local_position.floats[1];
			}*/
			scenegraph_SetParent(node, scenegraph.root, 1);				
			node->parent_type=NODE_ROOT;
		}
		
	}
}


/*
=============
scenegraph_SetChild
=============
*/
PEWAPI void scenegraph_SetChild(node_t *node, node_t *child)
{
	/* stuff... */
	return;
}


/*
=============
scenegraph_ProcessScenegraph
=============
*/
void scenegraph_ProcessScenegraph()
{ 
	mat4_t root_transform=mat4_t_id();
	mat4_t model_view_projection_matrix;
	
	mat4_t_mult_fast(&model_view_projection_matrix, &camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix, &camera_a.cameras[renderer.active_camera_index].projection_matrix);
	
	if(engine_state == 2)
	{
		scenegraph_UpdateColliders();
	}
	
	scenegraph_ProcessNode(scenegraph.root, &root_transform);
	//camera_ResetWorldToCameraMatrix();
	scenegraph_CullBrushes();
	scenegraph_CullGeometry();
	scenegraph_CullLights();
	scenegraph_DispatchGeometry();
	//scenegraph_GetAffectingLights();
	if(renderer.renderer_flags&RENDERFLAG_USE_SHADOW_MAPS)
	{
		scenegraph_KnapSack();
		scenegraph_FillShadowQueue();
	}
	
	light_UploadLightTransforms();
	light_AssignLightsToClusters();
	
	/*if(get_entity_under_cursor)
	{
		scenegraph_GetEntityUnderMouse();
	}*/
	
	
	
	
 
}


/*
=============
scenegraph_ProcessNode
=============
*/
static void scenegraph_ProcessNode(node_t *node, mat4_t *parent_transform)
{
	mat4_t transform;
	mat4_t c_transform=*parent_transform;
	mat4_t model_view_matrix;
	mat4_t translation;
	mat4_t orientation;
	mat3_t rot;
	mat3_t rotation=mat3_t_id();
	//affecting_lights_list light_list;
	//command_buffer_t cb;

	particle_system *p;
	node_t *c_node;
	vec4_t v;
	vec3_t pos;
	btVector3 bpos;
	//vec4_t box_corners[4];
	int j;
	int k;
	int i;
	int c;
	int node_index = node->index;
	int sub_index = node->sub_index;
	int child_count = node->children_count;
	//int corner_index=0;
	//int b_onscreen;
	int *temp;
	//float c_y;
	//float c_x;
	//float z;
	//float rb_margin;
	btTransform tr;
	btVector3 max;
	btVector3 min;
	entity_position_t *e;
	entity_extra_t *ex;
	entity_aabb_t *a;
	general_collider_t *collider;
	btRigidBody *rigid_body;
	switch(node->type)
	{
		case NODE_PARTICLE:
		break;
		
		case NODE_ENTITY:
			e = &entity_a.position_data[node_index];
			ex = &entity_a.extra_data[node_index];
			a = &entity_a.aabb_data[node_index];
			
			mat4_t_compose(&transform, &entity_a.extra_data[node_index].local_orientation, entity_a.extra_data[node_index].local_position);
			mat4_t_mult_fast(&c_transform, &transform, parent_transform);
			
			if(entity_a.position_data[node_index].bm_flags & ENTITY_HAS_MOVED)
			{
					//mat4_t_compose(&transform, &entity_a.extra_data[node_index].local_orientation, entity_a.extra_data[node_index].local_position);
					//mat4_t_mult(&c_transform, &transform, parent_transform);
					
				e->world_orientation.floats[0][0] = c_transform.floats[0][0];
				e->world_orientation.floats[0][1] = c_transform.floats[0][1];
				e->world_orientation.floats[0][2] = c_transform.floats[0][2];
						
				e->world_orientation.floats[1][0] = c_transform.floats[1][0];
				e->world_orientation.floats[1][1] = c_transform.floats[1][1];
				e->world_orientation.floats[1][2] = c_transform.floats[1][2];
						
				e->world_orientation.floats[2][0] = c_transform.floats[2][0];
				e->world_orientation.floats[2][1] = c_transform.floats[2][1];
				e->world_orientation.floats[2][2] = c_transform.floats[2][2];
					
				e->world_position.x = c_transform.floats[3][0];
				e->world_position.y = c_transform.floats[3][1];
				e->world_position.z = c_transform.floats[3][2];
					
				entity_CalculateAABB(a, e);
				entity_a.position_data[node_index].bm_flags &= ~ENTITY_HAS_MOVED;
			}

			//}
			
		break;
		
		case NODE_CAMERA:
			mat4_t_compose(&transform, &camera_a.cameras[node_index].local_orientation, camera_a.cameras[node_index].local_position);
			mat4_t_mult_fast(&c_transform, &transform, parent_transform);
			
			camera_a.cameras[node_index].world_position.floats[0]=c_transform.floats[3][0];
			camera_a.cameras[node_index].world_position.floats[1]=c_transform.floats[3][1];
			camera_a.cameras[node_index].world_position.floats[2]=c_transform.floats[3][2];

			mat4_t_mat3_t(&camera_a.cameras[node_index].world_orientation, &c_transform);
			
			camera_ComputeWorldToCameraMatrix(&camera_a.cameras[node_index]);												   					   
		break;
		
		case NODE_LIGHT:
			
			mat4_t_compose(&transform, &light_a.position_data[node_index].local_orientation, light_a.position_data[node_index].local_position.vec3);
			mat4_t_mult_fast(&c_transform, &transform, parent_transform);
			
			light_a.position_data[node_index].world_position.floats[0] = c_transform.floats[3][0];
			light_a.position_data[node_index].world_position.floats[1] = c_transform.floats[3][1];
			light_a.position_data[node_index].world_position.floats[2] = c_transform.floats[3][2];
			
			mat4_t_mat3_t(&rot, &c_transform);
			light_a.position_data[node_index].world_orientation = rot;
			
			if(light_a.position_data[node_index].bm_flags&LIGHT_POINT)
			{
				rot=mat3_t_id();
			}
			else
			{
				mat3_t_transpose(&rot);
			}
			
			mat4_t_compose(&orientation, &rot, vec3(0.0, 0.0, 0.0));
			
			light_a.extra_data[node_index].world_to_light_matrix=orientation;
			
			
			light_a.extra_data[node_index].world_to_light_matrix.floats[3][0]=(-light_a.position_data[node_index].world_position.floats[0])*light_a.extra_data[node_index].world_to_light_matrix.floats[0][0]	+	
										(-light_a.position_data[node_index].world_position.floats[1])*light_a.extra_data[node_index].world_to_light_matrix.floats[1][0]	+
										(-light_a.position_data[node_index].world_position.floats[2])*light_a.extra_data[node_index].world_to_light_matrix.floats[2][0];
										
			light_a.extra_data[node_index].world_to_light_matrix.floats[3][1]=(-light_a.position_data[node_index].world_position.floats[0])*light_a.extra_data[node_index].world_to_light_matrix.floats[0][1]	+	
										(-light_a.position_data[node_index].world_position.floats[1])*light_a.extra_data[node_index].world_to_light_matrix.floats[1][1]	+
										(-light_a.position_data[node_index].world_position.floats[2])*light_a.extra_data[node_index].world_to_light_matrix.floats[2][1];
										
			light_a.extra_data[node_index].world_to_light_matrix.floats[3][2]=(-light_a.position_data[node_index].world_position.floats[0])*light_a.extra_data[node_index].world_to_light_matrix.floats[0][2]	+	
										(-light_a.position_data[node_index].world_position.floats[1])*light_a.extra_data[node_index].world_to_light_matrix.floats[1][2]	+
										(-light_a.position_data[node_index].world_position.floats[2])*light_a.extra_data[node_index].world_to_light_matrix.floats[2][2];
										
			mat4_t_mult_fast(&light_a.shadow_data[node_index].model_view_projection_matrix, &light_a.extra_data[node_index].world_to_light_matrix, &light_a.extra_data[node_index].light_projection_matrix);
		break;
		
		case NODE_ARMATURE:
			mat4_t_compose(&transform, &armature_list.armatures[node_index].local_orientation, armature_list.armatures[node_index].local_position);
			mat4_t_mult_fast(&c_transform, &transform, parent_transform);
			
			/*for(i = 0; i < 3; i++)
			{
				armature_list.armatures[node_index].orientation.floats[i][0] = c_transform.floats[i][0];
				armature_list.armatures[node_index].orientation.floats[i][1] = c_transform.floats[i][1];
				armature_list.armatures[node_index].orientation.floats[i][2] = c_transform.floats[i][2];
			}*/
			
			
			//printf("[%f %f %f]\n", armature_list.armatures[node_index].position.floats[0], armature_list.armatures[node_index].position.floats[1], armature_list.armatures[node_index].position.floats[2]);
			
			mat4_t_mat3_t(&armature_list.armatures[node_index].world_orientation, &c_transform);
			
			armature_list.armatures[node_index].world_position.x = c_transform.floats[3][0];
			armature_list.armatures[node_index].world_position.y = c_transform.floats[3][1];
			armature_list.armatures[node_index].world_position.z = c_transform.floats[3][2];
			
			//printf("[%f %f %f]\n", c_transform.floats[3][0], c_transform.floats[3][1], c_transform.floats[3][2]);
		break;
		
		case NODE_BONE:
			mat4_t_compose(&transform, &armature_list.armatures[node_index].world_orientation, armature_list.armatures[node_index].world_position);
			mat4_t_mult_fast(&c_transform, &armature_list.armatures[node_index].global_transform[sub_index], &transform);
		break;	
		
		case NODE_COLLIDER:
			
			if(child_count)
			{
				collider = &collider_a.colliders[node_index];
				rigid_body = collider->base.rigid_body;
				tr = rigid_body->getCenterOfMassTransform();
				
				/*switch(collider->base.type)
				{
					case COLLIDER_CHARACTER_CONTROLLER:
						bpos = tr.getOrigin();
						pos.x = bpos[0];
						pos.y = bpos[1];
						pos.z = bpos[2];
						mat4_t_compose(&transform, NULL, pos);
						//printf("controller!\n");
					break;
					
					case COLLIDER_RIGID_BODY:*/
				tr.getOpenGLMatrix(&transform.floats[0][0]);
				/*	break;
				}*/
				mat4_t_mult_fast(&c_transform, &transform, parent_transform);
			}
		break;
	}
	
	c=node->children_count;
	for(i=0; i<c; i++)
	{
		c_node=(*((node_t **)&node->children+i));
		scenegraph_ProcessNode(c_node, &c_transform);
	}
	return;
}


static void scenegraph_ProcessNodes()
{
	mat4_t transform;
	//mat4_t c_transform=*parent_transform;
	mat4_t c_transform;
	mat4_t model_view_matrix;
	mat4_t translation;
	mat4_t orientation;
	mat3_t rot;
	mat3_t rotation=mat3_t_id();
	//affecting_lights_list light_list;
	command_buffer_t cb;
	particle_system *p;
	node_t *c_node=scenegraph.root;
	vec4_t v;
	vec4_t box_corners[4];
	int j;
	int k;
	int i;
	int c;
	int node_index;
	int corner_index=0;
	int b_onscreen;
	int *temp;
	float c_y;
	float c_x;
	float z;
	
	while(c_node)
	{
		
	}
	
	
}

static void scenegraph_GetAffectedScreenTiles()
{
	/*int i;
	int c;
	int j;
	int k;
	camera_t *camera=camera_GetActiveCamera();
	c=active_light_a.light_count;
	vec4_t w_position;
	mat4_t model_view_projection_matrix;
	
	float x_max;
	float y_max;
	float x_min;
	float y_min;
	
	mat4_t_mult(&model_view_projection_matrix, &camera->world_to_camera_matrix, &camera->projection_matrix);
	draw_ClearScreenTiles();
	for(i=0; i<c; i++)
	{
		w_position=active_light_a.lights[i].world_position;
		w_position=MultiplyVector4(&model_view_projection_matrix, w_position);
		
		w_position.floats[0]/=w_position.floats[3];	
		w_position.floats[1]/=w_position.floats[3];
		w_position.floats[2]/=w_position.floats[3];
		
		w_position.floats[0]=(w_position.floats[0]/2.0)+0.5;
		w_position.floats[1]=(w_position.floats[1]/2.0)+0.5;
		w_position.floats[2]=(w_position.floats[2]/2.0)+0.5;
		
		j=(int)(renderer.screen_width*w_position.floats[0])/screen_tiles.tile_width;
		k=(int)(renderer.screen_height*w_position.floats[1])/screen_tiles.tile_height;
		
		if(j<0)j=0;
		else if(j>=screen_tiles.tiles_per_row)j=screen_tiles.tiles_per_row-1;
		if(k<0)k=0;
		else if(k>=screen_tiles.tiles_per_coloumn)k=screen_tiles.tiles_per_coloumn-1;

		
		if(w_position.floats[0]>screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].x_min && 
		   w_position.floats[0]<screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].x_max)
		{
			if(w_position.floats[1]>screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].y_min && 
		   	   w_position.floats[1]<screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].y_max)
			{
				screen_tiles.tiles[j+k*screen_tiles.tiles_per_row].light_count++;
			}
		
		}
		
	}*/
}

static void scenegraph_CullLights()
{
	int i;
	int c;
	int s;
	int k;
	int l;
	int m;
	int samples;
	vec3_t l_org3;
	vec4_t l_org4;
	//plane_t p;
	plane_t top;
	plane_t bottom;
	plane_t right;
	plane_t left;
	plane_t near;
	plane_t far;
	
	//vec4_t p_origin;
	float distance;
	camera_t *active_camera=camera_GetActiveCamera();
	
	mat4_t view_matrix;
	mat4_t projection_matrix;
	mat4_t view_projection_matrix;
	
	vec4_t corners[8];
	vec3_t vecs[4];
	vec3_t le_vec;
	vec3_t *row;
	
	vec4_t light_origin;
	float l_rad;
	
	float x_min;
	float x_max;
	float y_min;
	float y_max;
	float area;
	float factor;
	float tangent;
	
	vec3_t fvec=camera_GetCameraWorldForwardVector(active_camera);
	vec3_t uvec=camera_GetCameraWorldUpVector(active_camera);
	vec3_t rvec=camera_GetCameraWorldRightVector(active_camera);
	
	vec3_t cpos=active_camera->world_position;
	
	vec3_t ftl;
	vec3_t fbl;
	vec3_t ftr;
	vec3_t fbr;
	
	vec3_t ntl;
	vec3_t nbl;
	vec3_t ntr;
	vec3_t nbr;
	
	vec3_t fc;
	vec3_t nc;
	
	vec3_t light_vec;
	
	memcpy(&view_matrix, &active_camera->world_to_camera_matrix, sizeof(mat4_t));
	memcpy(&projection_matrix, &active_camera->projection_matrix, sizeof(mat4_t));
	
	mat4_t_mult_fast(&view_projection_matrix, &active_camera->world_to_camera_matrix, &active_camera->projection_matrix);
	
	//unsigned long long start = _rdtsc();
	
	float nzfar=-active_camera->frustum.zfar;
	float nznear=-active_camera->frustum.znear;
	
	float ftop=(nzfar*active_camera->frustum.top)/nznear;
	float ntop=active_camera->frustum.top;
	float fright=(nzfar*active_camera->frustum.right)/nznear;
	float nright=active_camera->frustum.right;
	
	
	nc=add3(cpos, mul3(fvec, nznear));
	fc=add3(cpos, mul3(fvec, nzfar));

	ftl.floats[0]=fc.floats[0] - rvec.floats[0]*fright + uvec.floats[0]*ftop;
	ftl.floats[1]=fc.floats[1] - rvec.floats[1]*fright + uvec.floats[1]*ftop;
	ftl.floats[2]=fc.floats[2] - rvec.floats[2]*fright + uvec.floats[2]*ftop;
	
	ftr.floats[0]=fc.floats[0] + rvec.floats[0]*fright + uvec.floats[0]*ftop;
	ftr.floats[1]=fc.floats[1] + rvec.floats[1]*fright + uvec.floats[1]*ftop;
	ftr.floats[2]=fc.floats[2] + rvec.floats[2]*fright + uvec.floats[2]*ftop;
	
	fbl.floats[0]=fc.floats[0] - rvec.floats[0]*fright - uvec.floats[0]*ftop;
	fbl.floats[1]=fc.floats[1] - rvec.floats[1]*fright - uvec.floats[1]*ftop;
	fbl.floats[2]=fc.floats[2] - rvec.floats[2]*fright - uvec.floats[2]*ftop;
	
	fbr.floats[0]=fc.floats[0] + rvec.floats[0]*fright - uvec.floats[0]*ftop;
	fbr.floats[1]=fc.floats[1] + rvec.floats[1]*fright - uvec.floats[1]*ftop;
	fbr.floats[2]=fc.floats[2] + rvec.floats[2]*fright - uvec.floats[2]*ftop;
	
	
	ntl.floats[0]=nc.floats[0] - rvec.floats[0]*nright + uvec.floats[0]*ntop;
	ntl.floats[1]=nc.floats[1] - rvec.floats[1]*nright + uvec.floats[1]*ntop;
	ntl.floats[2]=nc.floats[2] - rvec.floats[2]*nright + uvec.floats[2]*ntop;
	
	nbl.floats[0]=nc.floats[0] - rvec.floats[0]*nright - uvec.floats[0]*ntop;
	nbl.floats[1]=nc.floats[1] - rvec.floats[1]*nright - uvec.floats[1]*ntop;
	nbl.floats[2]=nc.floats[2] - rvec.floats[2]*nright - uvec.floats[2]*ntop;
	
	ntr.floats[0]=nc.floats[0] + rvec.floats[0]*nright + uvec.floats[0]*ntop;
	ntr.floats[1]=nc.floats[1] + rvec.floats[1]*nright + uvec.floats[1]*ntop;
	ntr.floats[2]=nc.floats[2] + rvec.floats[2]*nright + uvec.floats[2]*ntop;


	c=light_a.light_count;
	/* wouldn't be faster to just rotate 
	the planes original normals by the camera
	global orientation instead of recomputing
	the whole frustum every time? */
	top=ComputePlane(cpos, ftl, ftr);
	bottom=ComputePlane(cpos, fbr, fbl);
	left=ComputePlane(cpos, fbl, ftl);
	right=ComputePlane(cpos, ftr, fbr);
	near=ComputePlane(ntl, ntr, nbl);
	far=ComputePlane(ftl, fbr, ftr);
	
	active_light_a.light_count = 0;
	
	//unsigned long long end = _rdtsc();
	
	//printf("calculation took %llu cycles\n", end - start);
	
	for(i=0; i<c; i++)
	{
		//light_a.lights[i].bm_status|= LIGHT_FRUSTUM_CULLED;
		
		if(light_a.position_data[i].light_index < 0) continue;
		
		
		l_org4 = light_a.position_data[i].world_position;
		l_rad = light_a.position_data[i].radius;
		

		/* top plane */
		distance=dot3(l_org4.vec3, top.normal) - top.d;
		if(distance<-l_rad) continue;

		/* bottom plane */
		distance=dot3(l_org4.vec3, bottom.normal) - bottom.d;
		if(distance<-l_rad) continue;

		/* left plane */
		distance=dot3(l_org4.vec3, left.normal) - left.d;
		if(distance<-l_rad) continue;

		/* right plane */
		distance=dot3(l_org4.vec3, right.normal) - right.d;
		if(distance<-l_rad) continue;

		/* near plane */
		distance=dot3(l_org4.vec3, near.normal) - near.d;
		if(distance<-l_rad) continue;
		
		/* far plane */
		distance=dot3(l_org4.vec3, far.normal) - far.d;
		if(distance<-l_rad) continue;
		//printf("light inside frustum\n");
		if(likely(active_light_a.light_count<active_light_a.array_size))
		{
			_add:
			
			m = active_light_a.light_count;
				
			memcpy(&active_light_a.position_data[m], &light_a.position_data[i], sizeof(light_data0));
			memcpy(&active_light_a.shadow_data[m], &light_a.shadow_data[i], sizeof(light_data2));
			memcpy(&active_light_a.extra_data[m], &light_a.extra_data[i], sizeof(light_data3));
			memcpy(&active_light_a.params[m], &light_a.params[i], sizeof(light_data1));
			
			
			//printf("%d %d %d\n", active_light_a.shadow_data[m].x, active_light_a.shadow_data[m].y, active_light_a.shadow_data[m].w);
			
			
			/* this could be made faster with sse... */
			for(l = 0; l < 3; l++)
			{
				row = (vec3_t *)&active_light_a.position_data[m].world_orientation.floats[l][0];
			
				l_org3.x = row->x * active_camera->world_to_camera_matrix.floats[0][0] + 
						   row->y * active_camera->world_to_camera_matrix.floats[1][0] + 
						   row->z * active_camera->world_to_camera_matrix.floats[2][0];
				
				l_org3.y = row->x * active_camera->world_to_camera_matrix.floats[0][1] + 
						   row->y * active_camera->world_to_camera_matrix.floats[1][1] + 
						   row->z * active_camera->world_to_camera_matrix.floats[2][1];
						   
				l_org3.z = row->x * active_camera->world_to_camera_matrix.floats[0][2] + 
						   row->y * active_camera->world_to_camera_matrix.floats[1][2] + 
						   row->z * active_camera->world_to_camera_matrix.floats[2][2];
						   
				active_light_transforms[m].floats[l][0] = l_org3.x;
				active_light_transforms[m].floats[l][1] = l_org3.y;
				active_light_transforms[m].floats[l][2] = l_org3.z;
				active_light_transforms[m].floats[l][3] = 0.0;
				
				
			}
			

			l_org3.x = l_org4.x * active_camera->world_to_camera_matrix.floats[0][0] + 
					   l_org4.y * active_camera->world_to_camera_matrix.floats[1][0] + 
					   l_org4.z * active_camera->world_to_camera_matrix.floats[2][0] + 
					   active_camera->world_to_camera_matrix.floats[3][0]; 
			
			l_org3.y = l_org4.x * active_camera->world_to_camera_matrix.floats[0][1] + 
					   l_org4.y * active_camera->world_to_camera_matrix.floats[1][1] + 
					   l_org4.z * active_camera->world_to_camera_matrix.floats[2][1] + 
					   active_camera->world_to_camera_matrix.floats[3][1];
					   
			l_org3.z = l_org4.x * active_camera->world_to_camera_matrix.floats[0][2] + 
					   l_org4.y * active_camera->world_to_camera_matrix.floats[1][2] + 
					   l_org4.z * active_camera->world_to_camera_matrix.floats[2][2] + 
					   active_camera->world_to_camera_matrix.floats[3][2];	
					   	   
					   
			active_light_transforms[m].floats[3][0] = l_org3.x;
			active_light_transforms[m].floats[3][1] = l_org3.y;
			active_light_transforms[m].floats[3][2] = l_org3.z;
			active_light_transforms[m].floats[3][3] = 1.0;	
			
			active_light_a.light_count++;
		}
		else
		{
			/* max active lights already cached, gtfo... */
			break;
			//light_ResizeLightArray(&active_light_a, active_light_a.array_size<<1);
			//goto _add;
		}
			
	}
	
	//light_UploadLightTransforms();
	//light_AssignLightsToClusters();
	//printf("\n");
	//printf("%d\n", active_light_a.light_count);
}

static void scenegraph_KnapSack()
{
	light_FitLights();
}


static void scenegraph_CullGeometry()
{
	int i;
	int c;
	int k;
	int *temp;
	int b_onscreen;
	
	entity_array *array;
	entity_position_t *position;
	entity_aabb_t *aabb;
	//plane_t top;
	//plane_t bottom;
	//plane_t right;
	//plane_t left;
	//plane_t near;
	//plane_t far;
	plane_t frustum_planes[6];
	//mat4_t transform;
	//mat4_t model_view_matrix;
	command_buffer_t cb;
	//camera_t *active_camera=camera_GetActiveCamera();
	//affecting_lights_list light_list;
	
	camera_t *active_camera=camera_GetActiveCamera();
	vec3_t fvec=camera_GetCameraWorldForwardVector(active_camera);
	vec3_t uvec=camera_GetCameraWorldUpVector(active_camera);
	vec3_t rvec=camera_GetCameraWorldRightVector(active_camera);
	vec3_t cpos=active_camera->world_position;
	
	vec3_t l_origin;
	
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
	float nzfar=-active_camera->frustum.zfar;
	float nznear=-active_camera->frustum.znear;
	
	float ftop=(nzfar*active_camera->frustum.top)/nznear;
	float ntop=active_camera->frustum.top;
	float fright=(nzfar*active_camera->frustum.right)/nznear;
	float nright=active_camera->frustum.right;
	float radius;
	float distance;
	
	nc=add3(cpos, mul3(fvec, nznear));
	fc=add3(cpos, mul3(fvec, nzfar));

	ftl.floats[0]=fc.floats[0] - rvec.floats[0]*fright + uvec.floats[0]*ftop;
	ftl.floats[1]=fc.floats[1] - rvec.floats[1]*fright + uvec.floats[1]*ftop;
	ftl.floats[2]=fc.floats[2] - rvec.floats[2]*fright + uvec.floats[2]*ftop;
	
	ftr.floats[0]=fc.floats[0] + rvec.floats[0]*fright + uvec.floats[0]*ftop;
	ftr.floats[1]=fc.floats[1] + rvec.floats[1]*fright + uvec.floats[1]*ftop;
	ftr.floats[2]=fc.floats[2] + rvec.floats[2]*fright + uvec.floats[2]*ftop;
	
	fbl.floats[0]=fc.floats[0] - rvec.floats[0]*fright - uvec.floats[0]*ftop;
	fbl.floats[1]=fc.floats[1] - rvec.floats[1]*fright - uvec.floats[1]*ftop;
	fbl.floats[2]=fc.floats[2] - rvec.floats[2]*fright - uvec.floats[2]*ftop;
	
	fbr.floats[0]=fc.floats[0] + rvec.floats[0]*fright - uvec.floats[0]*ftop;
	fbr.floats[1]=fc.floats[1] + rvec.floats[1]*fright - uvec.floats[1]*ftop;
	fbr.floats[2]=fc.floats[2] + rvec.floats[2]*fright - uvec.floats[2]*ftop;
	
	
	ntl.floats[0]=nc.floats[0] - rvec.floats[0]*nright + uvec.floats[0]*ntop;
	ntl.floats[1]=nc.floats[1] - rvec.floats[1]*nright + uvec.floats[1]*ntop;
	ntl.floats[2]=nc.floats[2] - rvec.floats[2]*nright + uvec.floats[2]*ntop;
	
	nbl.floats[0]=nc.floats[0] - rvec.floats[0]*nright - uvec.floats[0]*ntop;
	nbl.floats[1]=nc.floats[1] - rvec.floats[1]*nright - uvec.floats[1]*ntop;
	nbl.floats[2]=nc.floats[2] - rvec.floats[2]*nright - uvec.floats[2]*ntop;
	
	ntr.floats[0]=nc.floats[0] + rvec.floats[0]*nright + uvec.floats[0]*ntop;
	ntr.floats[1]=nc.floats[1] + rvec.floats[1]*nright + uvec.floats[1]*ntop;
	ntr.floats[2]=nc.floats[2] + rvec.floats[2]*nright + uvec.floats[2]*ntop;
	
	
	
	/*top.normal=cross(sub3(ftl, cpos), sub3(ftr, cpos));
	top.d=dot3(cpos, top.normal);*/
	
	
	/*top=ComputePlane(cpos, ftl, ftr);
	bottom=ComputePlane(cpos, fbr, fbl);
	left=ComputePlane(cpos, fbl, ftl);
	right=ComputePlane(cpos, ftr, fbr);
	near=ComputePlane(ntl, ntr, nbl);
	far=ComputePlane(ftl, fbr, ftr);*/
	
	frustum_planes[0] = ComputePlane(cpos, ftl, ftr);
	frustum_planes[1] = ComputePlane(cpos, fbr, fbl);
	frustum_planes[2] = ComputePlane(cpos, fbl, ftl);
	frustum_planes[3] = ComputePlane(cpos, ftr, fbr);
	frustum_planes[4] = ComputePlane(ntl, ntr, nbl);
	frustum_planes[5] = ComputePlane(ftl, fbr, ftr);
	
	
	array = &entity_a;
	//dispatch_list.cursor = 0;
	//dispatch_list.cursor=0;
	
	//static_start = -1;
	
	_do_static_entities:	
	
	c = array->entity_count;		
	
	for(i=0; i<c; i++)
	{
		
		position = &array->position_data[i];
		aabb = &array->aabb_data[i];
		/* TODO: find a better way to skip deleted entities (a separate array for active entities maybe?) */
		if(position->entity_index < 0 || 
		   //entity_a.position_data[i].type == ENTITY_PLAYER || 
		   (position->bm_flags & ENTITY_INVISIBLE)) continue;
		
		
		/*l_origin.floats[0]=entity_a.position_data[i].world_transform.floats[3][0];
		l_origin.floats[1]=entity_a.position_data[i].world_transform.floats[3][1];
		l_origin.floats[2]=entity_a.position_data[i].world_transform.floats[3][2];
		
		printf("[%f %f %f]\n", l_origin.x, l_origin.y, l_origin.z);*/
		
		l_origin.x = aabb->origin.x;
		l_origin.y = aabb->origin.y;
		l_origin.z = aabb->origin.z;
		
		
		for(k = 0; k < 6; k++)
		{
			radius = (aabb->c_maxmins[0])*fabs(frustum_planes[k].normal.floats[0]) + 
			     	 (aabb->c_maxmins[1])*fabs(frustum_planes[k].normal.floats[1]) +
			     	 (aabb->c_maxmins[2])*fabs(frustum_planes[k].normal.floats[2]); 
			
			distance = dot3(l_origin, frustum_planes[k].normal) - frustum_planes[k].d;
			
			//if(distance <= -radius) goto _fail;	 
			if(distance <= -radius) break;   	 
		}
		/*goto _pass;
		
		_fail:
			continue;
		_pass:*/
		
		if(k < 6) continue;
		
		if(likely(dispatch_list.cursor < dispatch_list.list_size))
		{
			_add_index:
			dispatch_list.indexes[dispatch_list.cursor++] = i;	
		}
		else
		{
			temp = (int *)malloc(sizeof(int) * (dispatch_list.list_size + 128));
			memcpy(temp, dispatch_list.indexes, sizeof(int) * dispatch_list.cursor);
			free(dispatch_list.indexes);
			dispatch_list.indexes = temp;
			dispatch_list.list_size += 128;
			goto _add_index;
		}	
		
	}
	
	/*if(static_start < 0)
	{
		array = &static_entity_a;
		static_start = dispatch_list.cursor;
		goto _do_static_entities;
	}*/
	
}

static void scenegraph_CullBrushes()
{
	int i;
	int c = brush_list.count;
	
	//brush_render_queue.command_buffer_count = 0;
	draw_ResetBrushRenderQueue();
	
	for(i = 0; i < c; i++)
	{
		draw_DispatchBrushCommandBuffer(brush_list.draw_data[i].start / sizeof(vertex_t), brush_list.draw_data[i].vert_count, brush_list.draw_data[i].material_index);
	}
}

static void scenegraph_CullStaticGeometry()
{
	
}

static void scenegraph_FillShadowQueue()
{
	int i;
	int c;
	int j;
	int k;
	int m;
	int n;
	int *temp;
	int p_index;
	
	//int *temp;
	//int b_onscreen;
	int scb_index;
	int entities_dispatched;
	entity_array *array;
	
	plane_t frustum_planes[6];
	vec3_t vecs[8];

	
	
	mat4_t transform;
	mat4_t translation;
	mat4_t orientation;
	mat4_t light_model_view_projection_matrix;
	mat4_t projection_matrix;
	command_buffer_t scb;
	
	/* forward vector */
	vec3_t fvec;
	/* up vector */
	vec3_t uvec;
	/* right vector */
	vec3_t rvec;
	/* light origin */
	//vec3_t lpos;
	
	vec3_t l_origin;
	vec3_t e_origin;
	vec3_t le_vec;
	
	
	/* far top left */
	vec3_t ftl;
	/* far bottom left */
	vec3_t fbl;
	/* far top right */
	vec3_t ftr;
	/* far bottom right */
	vec3_t fbr;
	/* near top left */
	vec3_t ntl;
	/* near bottom left */
	vec3_t nbl;
	/* near top right */
	vec3_t ntr;
	/* near bottom right */
	vec3_t nbr;
	/* self explanatory */
	vec3_t diag;
	/* far center */
	vec3_t fc;
	/* near center */
	vec3_t nc;
	
	
	/* negated zfar */
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
	
	int bm_sides = 0;
	int bm_inserted;
	
	int x;
	int y;
	int w;
	int h;
	
	k=active_light_a.light_count;
	c=entity_a.entity_count;
	
	shadow_q.count = 0;
	
	for(j=0; j<k; j++)
	{
		
		
		if(!(active_light_a.position_data[j].bm_flags&LIGHT_GENERATE_SHADOWS))
		{
			continue;
		}
		
		l_origin.floats[0]=active_light_a.position_data[j].world_position.floats[0];
		l_origin.floats[1]=active_light_a.position_data[j].world_position.floats[1];
		l_origin.floats[2]=active_light_a.position_data[j].world_position.floats[2];	
		
		if(active_light_a.position_data[j].bm_flags&LIGHT_SPOT)
		{
			/* this should be cached, and recalculated just when a light has moved/rotated */
			rvec.floats[0] = active_light_a.position_data[j].world_orientation.floats[0][0];
			rvec.floats[1] = active_light_a.position_data[j].world_orientation.floats[0][1];
			rvec.floats[2] = active_light_a.position_data[j].world_orientation.floats[0][2];
			
			uvec.floats[0] = active_light_a.position_data[j].world_orientation.floats[1][0];
			uvec.floats[1] = active_light_a.position_data[j].world_orientation.floats[1][1];
			uvec.floats[2] = active_light_a.position_data[j].world_orientation.floats[1][2];
			
			fvec.floats[0] = active_light_a.position_data[j].world_orientation.floats[2][0];
			fvec.floats[1] = active_light_a.position_data[j].world_orientation.floats[2][1];
			fvec.floats[2] = active_light_a.position_data[j].world_orientation.floats[2][2];
			
			//draw_debug_DrawFrustum(l_origin, &active_light_a.position_data[j].world_orientation, &active_light_a.extra_data[j].generated_frustum);
			
			nright = active_light_a.extra_data[j].generated_frustum.right;
			ntop = active_light_a.extra_data[j].generated_frustum.top;
			
			nznear = -active_light_a.extra_data[j].generated_frustum.znear;
			nzfar = -active_light_a.extra_data[j].generated_frustum.zfar;
				
			ftop = (nzfar*ntop)/nznear;
			fright = (nzfar*nright)/nznear;
					
			nc.floats[0] = l_origin.floats[0] + fvec.floats[0]*nznear;
			nc.floats[1] = l_origin.floats[1] + fvec.floats[1]*nznear;
			nc.floats[2] = l_origin.floats[2] + fvec.floats[2]*nznear;
					
			fc.floats[0] = l_origin.floats[0] + fvec.floats[0]*nzfar;
			fc.floats[1] = l_origin.floats[1] + fvec.floats[1]*nzfar;
			fc.floats[2] = l_origin.floats[2] + fvec.floats[2]*nzfar;
					
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
			
			frustum_planes[0] = ComputePlane(l_origin, ftl, ftr);	/* top */
			frustum_planes[1] = ComputePlane(l_origin, fbr, fbl);	/* bottom */
			frustum_planes[2] = ComputePlane(l_origin, fbl, ftl);	/* left */
			frustum_planes[3] = ComputePlane(l_origin, ftr, fbr);	/* right */
			frustum_planes[4] = ComputePlane(ntl, ntr, nbl);		/* near */
			frustum_planes[5] = ComputePlane(ftl, fbr, ftr);		/* far */
			
			projection_matrix=active_light_a.extra_data[j].light_projection_matrix;
			mat4_t_mult_fast(&scb.model_view_matrix, &active_light_a.extra_data[j].world_to_light_matrix, &active_light_a.extra_data[j].light_projection_matrix);
				
				
			*(int *)&scb.model_view_matrix.floats[0][3] = 0xff000000;				/* this tells draw_DrawShadowMaps that this command_buffer_t starts a light's render queue */
			*(int *)&scb.model_view_matrix.floats[1][3] = (active_light_a.shadow_data[j].x << 16) | active_light_a.shadow_data[j].y;						
			*(int *)&scb.model_view_matrix.floats[2][3] = (active_light_a.shadow_data[j].w << 16) | active_light_a.shadow_data[j].h;
			//*(int *)&scb.model_view_matrix.floats[1][3] = (active_light_a.shadow_data[j].shadow_map.shadow_map << 16);
			//*(int *)&scb.model_view_matrix.floats[2][3] = (GL_TEXTURE_2D << 16) | (active_light_a.params[j].shadow_map_res) /*(active_light_a.shadow_data[j].shadow_map.resolution)*/;
			draw_DispatchShadowCommandBuffer(&scb);
			
			p_index = 0;	
				
			for(i=0; i<c; i++)
			{
				
				if(entity_a.position_data[i].entity_index < 0 || entity_a.position_data[i].bm_flags & ENTITY_INVISIBLE) continue;
				   
				/* TODO: check if the light or the object have moved. If they have, then re-run the 
				frustum culling test for the object. */	
				//l_origin=entity_a.entity_position[i].world_position;
				
				//e_origin.floats[0]=entity_a.position_data[i].world_position.x;
				//e_origin.floats[1]=entity_a.position_data[i].world_position.y;
				//e_origin.floats[2]=entity_a.position_data[i].world_position.z;
				
				e_origin.floats[0] = entity_a.aabb_data[i].origin.x;
				e_origin.floats[1] = entity_a.aabb_data[i].origin.y;
				e_origin.floats[2] = entity_a.aabb_data[i].origin.z;
				
				for(p_index = 0; p_index <6; p_index++)
				{
					
						
					radius = entity_a.aabb_data[i].c_maxmins[0] * fabs(frustum_planes[p_index].normal.floats[0]) + 
							 entity_a.aabb_data[i].c_maxmins[1] * fabs(frustum_planes[p_index].normal.floats[1]) + 
							 entity_a.aabb_data[i].c_maxmins[2] * fabs(frustum_planes[p_index].normal.floats[2]);		 
					distance = (dot3(e_origin, frustum_planes[p_index].normal) - frustum_planes[p_index].d);
					/*printf("%d %s %f %f   [%f %f %f]\n", p_index, entity_a.extra_data[i].name, distance, radius, frustum_planes[p_index].normal.floats[0],
																												 frustum_planes[p_index].normal.floats[1],
																												 frustum_planes[p_index].normal.floats[2]);*/
					if(fabs(distance) > radius)
					{
						if(distance < 0.0)
						{
							break;
						}
					}
					/*if(distance < 0.0)
					{
						if(fabs(distance) > radius)
						{
							break;
						}
					}*/
				}
				
				p_index++;
				
				if(p_index <= 6)
				{
					continue;	
				}
				//printf("entity [%s] inside frustum\n", entity_a.extra_data[i].name);
				
				mat4_t_compose(&transform, &entity_a.position_data[i].world_orientation, entity_a.position_data[i].world_position);
				draw_FillShadowCommandBuffer(&scb, &transform, entity_a.draw_data[i].vert_count, (short)entity_a.draw_data[i].material_index, entity_a.draw_data[i].start, entity_a.draw_data[i].mesh->draw_mode);
				draw_DispatchShadowCommandBuffer(&scb);
				
			}
			
			//printf("\n\n");
			
			/* if this light didn't "see" anything, remove the beginning of its render queue */
			if(!p_index)
			{
				shadow_q.count--;
			}
		}
			
		else if(active_light_a.position_data[j].bm_flags&LIGHT_POINT)
		{
			
			
			for(n = 0; n < 6; n++)
			{
				//frustum_planes[n].normal = point_frustum_normals[n];
				point_frustum_planes[n].d = dot3(point_frustum_planes[n].normal, l_origin);
			}
			
			memcpy(&projection_matrix.floats[0][0], &active_light_a.extra_data[j].light_projection_matrix.floats[0][0], sizeof(mat4_t));
			used[0] = 0;
			used[1] = 0;
			used[2] = 0;
			used[3] = 0;
			used[4] = 0;
			used[5] = 0;
				
			for(i=0; i<c; i++)
			{
					
				if(entity_a.position_data[i].entity_index < 0 /*|| entity_a.position_data[i].type == ENTITY_PLAYER*/) continue;
				
				//printf("[%s]\n", entity_a.entity_geometry[i].name);
							
				e_origin.x = entity_a.aabb_data[i].origin.x;
				e_origin.y = entity_a.aabb_data[i].origin.y;
				e_origin.z = entity_a.aabb_data[i].origin.z;
					
				le_vec.x = e_origin.x - l_origin.x;
				le_vec.y = e_origin.y - l_origin.y;
				le_vec.z = e_origin.z - l_origin.z;
				len = length3(le_vec);
				le_vec.x /= len;
				le_vec.y /= len;
				le_vec.z /= len;
				
					
				/*radius = entity_a.position_data[i].c_maxmins[0] * fabs(le_vec.x) + 
						 entity_a.position_data[i].c_maxmins[1] * fabs(le_vec.y) +
						 entity_a.position_data[i].c_maxmins[2] * fabs(le_vec.z);*/
						 
				radius = entity_a.aabb_data[i].c_maxmins[0] * fabs(le_vec.x) + 
						 entity_a.aabb_data[i].c_maxmins[1] * fabs(le_vec.y) +
						 entity_a.aabb_data[i].c_maxmins[2] * fabs(le_vec.z);
						 
							 
					//distance = vecs[0].x * vecs[0].x + vecs[0].y * vecs[0].y + vecs[0].z * vecs[0].z - radius * radius;
				distance = len - radius;
				if(distance > active_light_a.position_data[j].radius) continue;
					
				bm_inserted = 0;
				bm_sides = 0;
				/* NOTE: here would be a good place to select which faces
				to be skipped in rendering */
				for(n = 0; n < 6; n++)
				{
					/*radius = entity_a.position_data[i].c_maxmins[0] * fabs(frustum_planes[n].normal.x) + 
						 	 entity_a.position_data[i].c_maxmins[1] * fabs(frustum_planes[n].normal.y) +
						 	 entity_a.position_data[i].c_maxmins[2] * fabs(frustum_planes[n].normal.z);*/
					
					radius = entity_a.aabb_data[i].c_maxmins[0] * fabs(point_frustum_planes[n].normal.x) + 
						 	 entity_a.aabb_data[i].c_maxmins[1] * fabs(point_frustum_planes[n].normal.y) +
						 	 entity_a.aabb_data[i].c_maxmins[2] * fabs(point_frustum_planes[n].normal.z);	 	 
						 	 
					distance = dot3(e_origin, point_frustum_planes[n].normal) - point_frustum_planes[n].d;
					
					bm_sides <<= 2;
					
					/* TODO: implement some sort of near plane checking. Not
					having this is making the engine render big objects
					onto the six faces of the shadow map... */
					
					if(fabs(distance) < radius)
					{
						bm_sides |= 3;  	/* object intersected plane */
					}
					else
					{
						if(distance > 0.0)
						{
							bm_sides |= 1;	/* object in positive half-space */
						}
						else
						{
							bm_sides |= 2;	/* object in negative half-space */
						}
					}	
						 	  	  
				}
				
				for(n = 0; n < 6; n++)
				{
					if((bm_sides & check_values[n]) == check_values[n])
					{
						if(used[n] >= sizes[n])
						{
							temp = (int *)malloc(sizeof(int) * (sizes[n] << 1));
							memcpy(temp, rqs[n], sizeof(int) * sizes[n]);
							free(rqs[n]);
							rqs[n] = temp;
							sizes[n] <<= 1;
						}
						
						rqs[n][used[n]] = i;
						used[n]++;
					}
					
				}	
			}
			
			translation=mat4_t_id();
			translation.floats[3][0] = -l_origin.x;
			translation.floats[3][1] = -l_origin.y;
			translation.floats[3][2] = -l_origin.z;
				
			for(m = 0; m < 6; m++)
			{
				//orientation = cube_shadow_mats[m];
				
				if(used[m] == 0)
				{
					continue;
				}
				
				//memcpy(&orientation, &cube_shadow_mats[m], sizeof(mat4_t));
				//mat4_t_transpose(&orientation);
				//transform.floats[3][0] = - l_origin.x;
				//transform.floats[3][1] = - l_origin.y;
				//transform.floats[3][2] = - l_origin.z;
				mat4_t_mult_fast(&transform, &translation, &cube_shadow_mats[m]);	
				mat4_t_mult_fast(&scb.model_view_matrix, &transform, &projection_matrix);
				
				
				w = active_light_a.shadow_data[j].w / 3;
				h = active_light_a.shadow_data[j].h >> 1;
				
				switch(m)
				{
					/* +X */
					case 0:
						x = active_light_a.shadow_data[j].x;
						y = active_light_a.shadow_data[j].y;
					break;
					
					/* -X */
					case 1:	
						x = active_light_a.shadow_data[j].x;
						y = active_light_a.shadow_data[j].y + h;
					break;
					
					/* +Y */
					case 2:
						x = active_light_a.shadow_data[j].x + w;
						y = active_light_a.shadow_data[j].y;
					break;
					
					/* -Y */
					case 3:
						x = active_light_a.shadow_data[j].x + w;
						y = active_light_a.shadow_data[j].y + h;
					break;
					
					/* +Z */
					case 4:
						x = active_light_a.shadow_data[j].x + 2 * w;
						y = active_light_a.shadow_data[j].y;
					break;
					
					/* -Z */
					case 5:
						x = active_light_a.shadow_data[j].x + 2 * w;
						y = active_light_a.shadow_data[j].y + h;
					break;
				}
				
				
				
				/* ignore frustums that have no objects in it. */
					
				*(int *)&scb.model_view_matrix.floats[0][3] = 0xff000000; 				/* this tells draw_DrawShadowMaps that this command_buffer_t starts a light's render queue */
				*(int *)&scb.model_view_matrix.floats[1][3] = (x << 16) | y;						
				*(int *)&scb.model_view_matrix.floats[2][3] = (w << 16) | h;
			//	*(int *)&scb.model_view_matrix.floats[1][3] = (active_light_a.shadow_data[j].shadow_map.shadow_map << 16);
			//	*(int *)&scb.model_view_matrix.floats[2][3] = ((GL_TEXTURE_CUBE_MAP_POSITIVE_X+m) << 16) | (active_light_a.params[j].shadow_map_res) /*(active_light_a.shadow_data[j].shadow_map.resolution)*/;		
				draw_DispatchShadowCommandBuffer(&scb);
				
				//printf("m: %d used: %d\n", m, used[m]);
					
				for(n = 0; n < used[m]; n++)
				{
					mat4_t_compose(&transform, &entity_a.position_data[rqs[m][n]].world_orientation, entity_a.position_data[rqs[m][n]].world_position);
					draw_FillShadowCommandBuffer(&scb, &transform, entity_a.draw_data[rqs[m][n]].vert_count, entity_a.draw_data[rqs[m][n]].material_index, entity_a.draw_data[rqs[m][n]].start, entity_a.draw_data[rqs[m][n]].mesh->draw_mode);
					draw_DispatchShadowCommandBuffer(&scb);
				}
					
			}
					
		}
		
	}
	/* sentinel */
	//printf("end frame\n");
	//printf("%d\n", bm_p_intersected);
	shadow_q.command_buffers[shadow_q.count].model_view_matrix.floats[0][3]=0xff000000;
		
}


static void scenegraph_DispatchGeometry()
{
	int i;
	int c;
	register int q;
	command_buffer_t cb;
	camera_t *active_camera=camera_GetActiveCamera();
	//affecting_lights_list light_list;
	mat4_t transform;
	mat4_t model_view_matrix;
	render_queue *r;
	entity_array *array;
	material_t *m;
	
	render_q.count = 0;
	t_render_q.count = 0;
	e_render_q.count = 0;
	
	c = dispatch_list.cursor;
	array = &entity_a;
	i = 0;
	
	_submit_static_entities:
	
	for(; i<c; i++)
	{
		q = dispatch_list.indexes[i];
		mat4_t_compose(&transform, &array->position_data[q].world_orientation, array->position_data[q].world_position);
		
		/* hmm... this doesn't make sense... */
		
		mat4_t_mult_fast(&model_view_matrix, &transform, &active_camera->world_to_camera_matrix);
		//mat4_t_mult(&model_view_matrix, &transform, &active_camera->world_to_camera_matrix);
		
		m = &material_a.materials[array->draw_data[q].material_index];
		if(m->bm_flags & MATERIAL_Translucent)
		{
			r = &t_render_q;
		}
		else if(m->bm_flags & MATERIAL_Emissive)
		{
			r = &e_render_q;
		}
		else
		{
			r = &render_q;
		}
		
		
		draw_DispatchCommandBuffer(r, q, array->draw_data[q].material_index,
										 -1, 
										 array->draw_data[q].draw_flags, 
										 array->draw_data[q].start, 
										 array->draw_data[q].vert_count, 
									     &model_view_matrix);
	}
	
	/*if(c != dispatch_list.cursor)
	{
		i = static_start;
		c = dispatch_list.cursor;
		array = &static_entity_a;
		goto _submit_static_entities;
	}*/
		
	dispatch_list.cursor=0;
}

static void scenegraph_GetAffectingLights()
{
	int i;
	int c;
	int j;
	int k;
	int m;
	int index;
	int al_count;
	float r;
	float l;
	c = t_render_q.count;
	k = active_light_a.light_count;
	entity_aabb_t *aabb;
	light_data0 *ldata;
	vec3_t d;
	//vec4_t d = {0.0, 0.0, 0.0, 0.0};
	affecting_lights.count = 0;
	affecting_lights_t *al;
	command_buffer_t *cb;
	
	
	if(c >= affecting_lights.size)
	{
		light_ResizeAffectingLightList(affecting_lights.size + 16);
	}
	
	if(!k) return;
	
	for(i = 0; i < c; i++)
	{
		index = t_render_q.entity_indexes[i];
		aabb = &entity_a.aabb_data[index];
		al_count = 0;
		m = affecting_lights.count;
		al = &affecting_lights.lights[m];
		al->count = 0;
		for(j = 0; j < k; j++)
		{
			ldata = &active_light_a.position_data[j];
			
			d.x = ldata->world_position.x - aabb->origin.x;
			d.y = ldata->world_position.y - aabb->origin.y;
			d.z = ldata->world_position.z - aabb->origin.z;
			
			/* separating axis test might be faster for this, but 
			less precise for AABBs... */
			l = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
			
			d.x /= l;
			d.y /= l;
			d.z /= l;
			
			r = aabb->c_maxmins[0] * d.x + 
				aabb->c_maxmins[1] * d.y +
				aabb->c_maxmins[2] * d.z;
				
			if(l - r > ldata->radius) continue;	
			al->light_IDs[al->count] = j;
			al->count++;
			
			if(al->count >= MAX_LIGHTS_PER_ENTITY)
			{
				break;
			}
			
		}
		
		if(al->count)
		{
			affecting_lights.count++;
			t_render_q.command_buffers[i].lights_index = m;
		}
	}
}


static void scenegraph_UpdateVertexCache()
{
	int i;
	int c = render_q.count;
	
	for(i = 0; i < c; i++)
	{
		
	}
}


static void scenegraph_UpdateColliders()
{
	int i;
	int j;
	int c = collider_a.count;
	collider_base_t *collider;
	entity_extra_t *entity;
	entity_position_t *position;
	mat4_t transform;
	btTransform tr;
	for(i = 0; i < c; i++)
	{
		collider = &collider_a.colliders[i].base;
		
		/* colliders added to the scenegraph will
		be automatically handled by it... */
		if(collider->assigned_node) continue;
		
		/* if this collider isn't active, no need to
		update anything... */
		else if(!collider->rigid_body->isActive()) continue;
		
		/* only entities can have colliders
		directly attached to them... */
		entity = &entity_a.extra_data[collider->index];
		position = &entity_a.position_data[collider->index];
		
		position->bm_flags |= ENTITY_HAS_MOVED;
		
		//collider->rigid_body->getMotionState()->getWorldTransform(tr);
		collider->rigid_body->getWorldTransform().getOpenGLMatrix(&transform.floats[0][0]);
		//tr.getOpenGLMatrix(&transform.floats[0][0]);

		for(j = 0; j < 3; j++)
		{
			entity->local_orientation.floats[j][0] = transform.floats[j][0];
			entity->local_orientation.floats[j][1] = transform.floats[j][1];
			entity->local_orientation.floats[j][2] = transform.floats[j][2];
		}
					
		entity->local_position.x = transform.floats[3][0];
		entity->local_position.y = transform.floats[3][1];
		entity->local_position.z = transform.floats[3][2];

	}
}

static void scenegraph_GroupPerHint()
{
	
}


/*
=============
scenegraph_PrintChildIDs
=============
*/
PEWAPI void scenegraph_PrintChildsNames(node_t *node)
{
	int i;
	int c;
	c=node->children_count;
	printf("node name is: [%s]\n", node->name);
	for(i=0; i<c; i++)
	{
		printf("node [%s] has child node called: [%s]\n", node->name, (*((node_t **)&node->children+i))->name);
	}
}


int scenegraph_GetEntityUnderMouse()
{
	/*int i;
	int c = render_q.count;
	
	mat4_t model_view_matrix;
	
	camera_t *active_camera = camera_GetActiveCamera();
	framebuffer_t *f = cur_fb;
	framebuffer_BindFramebuffer(&picking_buffer);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	
	//int offset=0;
	
	unsigned int draw_mode;
	unsigned int vert_count;
	unsigned int material_index;
	unsigned int gpu_buffer;
	unsigned int start;
	unsigned int attrib_flags;
	unsigned int entity_index;

	float wcolor[4];
	float pixel[4];
	
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wcolor);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	shader_SetShaderByIndex(wireframe_shader_index);	

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	c = render_q.count;
	
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	
	if(renderer.render_mode == RENDER_DRAWMODE_WIREFRAME)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(8.0);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	//printf("%d\n", c);
	for(i=0; i < c; i++)
	{

		//MatrixCopy4(&model_view_matrix, &render_q.command_buffers[i].model_view_matrix);
		memcpy(&model_view_matrix.floats[0][0], &render_q.command_buffers[i].model_view_matrix.floats[0][0], sizeof(mat4_t));
		start = *(unsigned int *)&model_view_matrix.floats[0][3];
		model_view_matrix.floats[0][3]=0.0;
		
		vert_count = *(unsigned int *)&model_view_matrix.floats[1][3];
		model_view_matrix.floats[1][3]=0.0;
		
		
		entity_index = *(unsigned int *)&model_view_matrix.floats[2][3];
		model_view_matrix.floats[2][3]=0.0;
		
		
		draw_mode = *(unsigned int *)&model_view_matrix.floats[3][3];
		model_view_matrix.floats[3][3]=1.0;
		
		model_view_matrix.floats[2][2] = model_view_matrix.floats[0][0] * model_view_matrix.floats[1][1] - 
										 model_view_matrix.floats[0][1] * model_view_matrix.floats[1][0];
		
		
		material_index = draw_mode & 0x0000ffff;
		draw_mode = (draw_mode >> 16);
		attrib_flags = draw_mode & 0x00000f00;
		draw_mode &= 0x0000000f;
		
		wcolor[0] = (entity_index + 1);
		wcolor[1] = 1.0;
		wcolor[2] = 0.0;
		wcolor[3] = 0.0;
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wcolor);
		glLoadMatrixf(&model_view_matrix.floats[0][0]);
		//glBindBuffer(GL_ARRAY_BUFFER, gpu_buffer);
		glEnableVertexAttribArray(shader_a.shaders[wireframe_shader_index].v_position);
		glVertexAttribPointer(shader_a.shaders[wireframe_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, 0, (void *)(start));
		glDrawArrays(draw_mode, 0, vert_count);
	
	}
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, picking_buffer.id);
	
	//printf("%d %d\n", mouse_x, mouse_y);
	
	glReadPixels(mouse_x, mouse_y, 1, 1, GL_RGB, GL_FLOAT, pixel);
	entity_index = pixel[0];
	entity_index--;
	
	//printf("%d\n", entity_index);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1.0);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	framebuffer_BindFramebuffer(f);
	
	return entity_index;
	
	//get_entity_under_cursor = 0;*/
}


pick_record_t scenegraph_Pick(float mouse_x, float mouse_y)
{
	int i;
	int c = render_q.count;
	
	//float mouse_x = input.mouse_x;
	//float mouse_y = input.mouse_y;
	mat4_t model_view_matrix;
	
	camera_t *active_camera = camera_GetActiveCamera();
	framebuffer_t *f = cur_fb;
	framebuffer_BindFramebuffer(&picking_buffer);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	
	glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	
	//int offset=0;
	
	unsigned int draw_mode;
	unsigned int vert_count;
	unsigned int material_index;
	unsigned int gpu_buffer;
	unsigned int start;
	unsigned int attrib_flags;
	unsigned int entity_index;

	float wcolor[4];
	float pixel[4];
	float *d;
	pick_record_t r;
	
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wcolor);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	shader_SetShaderByIndex(wireframe_shader_index);
	glEnableVertexAttribArray(shader_a.shaders[wireframe_shader_index].v_position);
	glVertexAttribPointer(shader_a.shaders[wireframe_shader_index].v_position, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t ), (void *)(0));	

	glMatrixMode(GL_MODELVIEW);
	c = render_q.count;
	
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	
	if(renderer.render_mode == RENDER_DRAWMODE_WIREFRAME)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(8.0);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	*(int *)&wcolor[1] = PICK_ENTITY;
	for(i=0; i < c; i++)
	{
		memcpy(&model_view_matrix.floats[0][0], &render_q.command_buffers[i].model_view_matrix.floats[0][0], sizeof(mat4_t));
		start = *(unsigned int *)&model_view_matrix.floats[0][3];
		model_view_matrix.floats[0][3]=0.0;
		
		vert_count = *(unsigned int *)&model_view_matrix.floats[1][3];
		model_view_matrix.floats[1][3]=0.0;
		
		
		entity_index = *(unsigned int *)&model_view_matrix.floats[2][3];
		model_view_matrix.floats[2][3]=0.0;
		
		
		draw_mode = *(unsigned int *)&model_view_matrix.floats[3][3];
		model_view_matrix.floats[3][3]=1.0;
		
		model_view_matrix.floats[2][2] = model_view_matrix.floats[0][0] * model_view_matrix.floats[1][1] - 
										 model_view_matrix.floats[0][1] * model_view_matrix.floats[1][0];
		
		
		material_index = draw_mode & 0x0000ffff;
		draw_mode = (draw_mode >> 16);
		attrib_flags = draw_mode & 0x00000f00;
		draw_mode &= 0x0000000f;
		
		*(int *)&wcolor[0] = (entity_index + 1);
		wcolor[2] = 0.0;
		wcolor[3] = 0.0;
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wcolor);
		glLoadMatrixf(&model_view_matrix.floats[0][0]);
		start /= sizeof(vertex_t);
		glDrawArrays(draw_mode, start, vert_count);
	}
	
	shader_SetShaderByIndex(brush_pick_shader);
	glEnableVertexAttribArray(shader_a.shaders[brush_pick_shader].v_position);
	glVertexAttribPointer(shader_a.shaders[brush_pick_shader].v_position, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(0));
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	*(int *)&wcolor[1] = PICK_BMODEL;
	c = brush_list.count;
	//c = brush_render_queue.command_buffer_count;
	for(i = 0; i < c; i++)
	{
		start = brush_list.draw_data[i].start / sizeof(vertex_t);
		
		*(int *)&wcolor[0] = (i + 1);
		wcolor[2] = start / 3 + brush_list.draw_data[i].vert_count / 3;
		wcolor[3] = 0.0;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wcolor);
		glDrawArrays(GL_TRIANGLES, start, brush_list.draw_data[i].vert_count);
		
		
		
	}
	
	
	
	
	c = light_a.light_count;
	
	
	//glUseProgram(0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glBindBuffer(GL_ARRAY_BUFFER, light_picking_vbo);
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	glPointSize(16.0);
	glEnable(GL_POINT_SMOOTH);
	
	shader_SetShaderByIndex(light_pick_shader);
	
	glEnableVertexAttribArray(shader_a.shaders[light_pick_shader].v_position);
	glVertexAttribPointer(shader_a.shaders[light_pick_shader].v_position, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	d = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	*(int *)&wcolor[1] = PICK_LIGHT;
	for(i = 0; i < c; i++)
	{
		d[i * 3] = light_a.position_data[i].world_position.x;
		d[i * 3 + 1] = light_a.position_data[i].world_position.y;
		d[i * 3 + 2] = light_a.position_data[i].world_position.z;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	for(i = 0; i < c; i++)
	{
		*(int *)&wcolor[0] = (i + 1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wcolor);
		draw_DrawArrays(GL_POINTS, i, 1);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glPointSize(1.0);
	glDisable(GL_POINT_SMOOTH);
	
	
	
	
	
	
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, picking_buffer.id);
	
	//printf("%d %d\n", mouse_x, mouse_y);
	
	glReadPixels(mouse_x, mouse_y, 1, 1, GL_RGB, GL_FLOAT, pixel);
	
	//printf("%d %d %f\n", *(int *)&pixel[0], *(int *)&pixel[1], pixel[2]);
	r.index = *(int *)&pixel[0] - 1;
	r.type = *(int *)&pixel[1];
	r.face = 0;
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1.0);
	
	/*glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();*/
	
	framebuffer_BindFramebuffer(f);
	
	return r;
}

#ifdef __cplusplus
}
#endif















