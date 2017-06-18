#include "entity.h"
#include "scenegraph.h"
#include "camera.h"
#include "draw.h"
#include "armature.h"
#include "material.h"
#include "input.h"
#include "scenegraph.h"
//#include "btBulletDynamicsCommon.h"
//#include "BulletCollision\CollisionShapes\btShapeHull.h"

extern input_cache input;

//#include "physics.h"

entity_array entity_a;
entity_def_list entity_defs;
edef_list_t edefs; 

entity_ptr selected_entity;
int v_ent_count = 0;
int v_ent_array_size = 0;
int *v_entities;							/* an array of indexes for visible entities */

static char armature_name[128];


//int entity_c_data_count = 0;
//static entity_c_data *root;
//static entity_c_data *last;

//entity_array v_entity_a;	
extern material_array material_a;
extern armature_list_t armature_list;					
extern camera_array camera_a;
extern renderer_t renderer;
//extern btDiscreteDynamicsWorld *physics_world;	/* this shouldn't be here. Create physics_*** functions to handle this. */
extern collider_array collider_a;

extern int get_entity_under_cursor;
extern int mouse_x;
extern int mouse_y;

extern entity_ptr last_under_cursor;

#ifdef __cplusplus
extern "C"
{
#endif

/*
=============
entity_Init
=============
*/
void entity_Init()
{
	//entity_a.entities=NULL;
	entity_a.position_data=NULL;
	entity_a.draw_data=NULL;
	entity_a.extra_data = NULL;
	entity_a.aabb_data = NULL;
	entity_a.entity_count=0;
	entity_a.stack_top=-1;
	entity_ResizeEntityArray(&entity_a, 2560);
	
	v_ent_array_size = 128;
	
	v_entities = (int *)malloc(sizeof(int) * v_ent_array_size);
	
	entity_defs.count = 0;
	entity_defs.defs = NULL;
	entity_ResizeEntityDefList(128);
	
	edefs.defs = NULL;
	edefs.count = 0;
	entity_ResizeEntityDefList2(16);
	
	//root = (entity_c_data *)malloc(sizeof(entity_c_data));
	//root->next = NULL;
	//last = root;

	
	return;
}

/*
=============
entity_Finish
=============
*/
void entity_Finish()
{
	
	//entity_c_data *temp = root;
	//entity_c_data *temp2;
	
	free(entity_a.position_data);
	free(entity_a.draw_data);
	free(entity_a.extra_data);
	free(entity_a.aabb_data);
	free(v_entities);
	
	free(entity_defs.defs);
	
	/*while(temp)
	{
		temp2 = temp->next;
		free(temp);
		temp = temp2;
	}*/
	
	
	return;
}

/*
=============
entity_ResizeEntityArray
=============
*/
void entity_ResizeEntityArray(entity_array *array, int new_size)
{
	//entity_t *temp=calloc(new_size, sizeof(entity_t));
	entity_position_t *ptemp=(entity_position_t *)malloc(new_size * sizeof(entity_position_t));
	entity_draw_t *gtemp=(entity_draw_t *)malloc(new_size * sizeof(entity_draw_t));
	entity_extra_t *etemp = (entity_extra_t *)malloc(new_size * sizeof(entity_extra_t));
	entity_aabb_t *atemp = (entity_aabb_t *)malloc(new_size * sizeof(entity_aabb_t));
	int *stack_temp=(int *)malloc(new_size * sizeof(int));
	
	if(array->position_data)
	{
		memcpy(ptemp, array->position_data, sizeof(entity_position_t)*array->entity_count);
		memcpy(gtemp, array->draw_data, sizeof(entity_draw_t)*array->entity_count);
		memcpy(etemp, array->extra_data, sizeof(entity_extra_t) * array->entity_count);
		memcpy(atemp, array->aabb_data, sizeof(entity_aabb_t) * array->entity_count);
		free(array->position_data);
		free(array->draw_data);
		free(array->extra_data);
		free(array->aabb_data);
		free(array->free_positions_stack);
	}
	array->position_data=ptemp;
	array->draw_data=gtemp;
	array->extra_data = etemp;
	array->aabb_data = atemp;
	array->free_positions_stack=stack_temp;
	array->array_size=new_size;
	return;
}

void entity_ResizeEntityDefList(int new_size)
{
	entity_def_t *temp=(entity_def_t *)malloc(new_size * sizeof(entity_def_t));
	if(entity_defs.defs)
	{
		memcpy(temp, entity_defs.defs, sizeof(entity_def_t) * entity_defs.size);
		free(entity_defs.defs);
		
	}
	entity_defs.defs = temp;
	entity_defs.size = new_size;
}

void entity_ResizeEntityDefList2(int new_size)
{
	edef_t *temp=(edef_t *)malloc(new_size * sizeof(edef_t));
	int *itemp = (int *)malloc(new_size * sizeof(int));
	if(edefs.defs)
	{
		memcpy(temp, edefs.defs, sizeof(edef_t) * edefs.size);
		free(edefs.defs);
		free(edefs.free_stack);
	}
	else
	{
		edefs.count = 0;
		edefs.stack_top = -1;
	}
	edefs.defs = temp;
	edefs.free_stack = itemp;
	edefs.size = new_size;
}


PEWAPI void entity_SetEntityArmature(int entity_index, int armature_index)
{
	armature_t *a;
	int byte_count = 0;
	mesh_t *mesh;
	if(armature_index >= 0)
	{
		mesh = entity_a.draw_data[entity_index].mesh;
		byte_count = sizeof(float) * 3 * mesh->vert_count;
		
		if(mesh->n_data)
		{
			byte_count += byte_count; 
		}
		if(mesh->t_data)
		{
			byte_count += byte_count;
		}
		if(mesh->t_c_data)
		{
			byte_count += sizeof(float) * 2 * mesh->vert_count;
		}
		
		entity_a.draw_data[entity_index].handle = gpu_Alloc(byte_count);
		entity_a.draw_data[entity_index].start = gpu_GetAllocStart(entity_a.draw_data[entity_index].handle);
		entity_a.position_data[entity_index].bm_flags |= ENTITY_SKINNABLE;
		
		a = &armature_list.armatures[armature_index];
		a->mesh = mesh;
		a->start = entity_a.draw_data[entity_index].start;
		//a->weight_set = weight_set_index;
	}
}

PEWAPI comp_base_t *entity_CreateComponent(char *name, int type, int flags, vec3_t *position, mat3_t *orientation)
{
	comp_base_t *r;
	int size;
	switch(type)
	{
		case ENTITY_COMPONENT_COLLIDER:
			size = sizeof(comp_collider_t);
		break;
		
		case ENTITY_COMPONENT_BASE:
		case ENTITY_COMPONENT_CAMERA:
			size = sizeof(comp_base_t);
		break;
		
		case ENTITY_COMPONENT_GEOMETRY:
			size = sizeof(comp_geometry_t);
		break;
		
		case ENTITY_COMPONENT_ARMATURE:
			size = sizeof(comp_armature_t);
		break;
		
		case ENTITY_COMPONENT_LIGHT:
			size = sizeof(comp_light_t);
		break;
		
		default:
			return NULL;
		break;
	}
	
	r = (comp_base_t *)malloc(size);
	
	r->children = (comp_base_t **) malloc(sizeof(comp_base_t *) * 16);
	r->child_count = 0;
	r->child_size = 16;
	r->name = strdup(name);
	r->comp_type = type;
	r->index = -1;
	memcpy(&r->orientation, orientation, sizeof(mat3_t));
	memcpy(&r->position, position, sizeof(vec3_t));
	r->parent = NULL;
	
	return r;
}

PEWAPI comp_base_t *entity_CreateGeometryComponent(char *name, int flags, mesh_t *mesh, int material_index, int armdef_index, float mass, vec3_t *position, mat3_t *orientation)
{
	comp_geometry_t *g = (comp_geometry_t *) entity_CreateComponent(name, ENTITY_COMPONENT_GEOMETRY, 0, position, orientation);
	float m[3];
	
	
	g->flags = flags;
	g->mesh = mesh;
	g->start = mesh->start;
	g->vert_count = mesh->vert_count;
	g->draw_flags = mesh->draw_mode;
	model_GetMaxMinsFromVertexData(mesh->v_data, g->o_maxmins, mesh->vert_count);
	
	if(mesh->n_data)
	{
		g->draw_flags |= CBATTRIBUTE_NORMAL;
	}
	if(mesh->t_data)
	{
		g->draw_flags |= CBATTRIBUTE_TANGENT;
	}
	if(mesh->t_c_data)
	{
		g->draw_flags |= CBATTRIBUTE_TEX_COORD;
	}
	
}

PEWAPI void entity_AddChildComponent(comp_base_t *parent, comp_base_t *child)
{
	comp_base_t **b;
	comp_base_t *p;
	int i;
	int c;
	if(parent && child)
	{
		if(parent->child_count >= parent->child_size)
		{
			b = (comp_base_t **)malloc(sizeof(comp_base_t *) * (parent->child_size + 16));
			c = parent->child_size;
			for(i = 0; i < c; i++)
			{
				b[i] = parent->children[i];
			}
			free(parent->children);
			parent->children = b;
			parent->child_size += 16;
		}
		
		parent->children[parent->child_count++] = child;
		child->parent = parent;
		
	}
}

PEWAPI comp_base_t *entity_RemoveChildComponent(comp_base_t *parent, char *name)
{
	int i;
	int c;
	comp_base_t *r;
	if(parent)
	{
		c = parent->child_count;
		for(i = 0; i < c; i++)
		{
			if(!strcmp(parent->children[i]->name, name))
			{
				r = parent->children[i];
				if(i < c - 1)
				{
					parent->children[i] = parent->children[c - 1];
				}
				parent->child_count--;
				
				return r;
			}
		}
	}
	return NULL;
}

PEWAPI comp_base_t *entity_RemoveChildComponentByIndex(comp_base_t *parent, int index)
{
	comp_base_t *r;
	if(parent)
	{
		if(index < parent->child_count)
		{
			r = parent->children[index];
			if(parent->child_count > 1)
			{
				parent->children[index] = parent->children[parent->child_count--];
			}
			return r;
		}
		
	}
	return NULL;
}

PEWAPI int entity_CreateEntityDef2(char *name, comp_base_t *root)
{
	edef_t *e;
	comp_base_t *b;
	int index;
	int stack_top = 0;
	int count = 0;
	char children[64];
	if(root)
	{
		if(edefs.stack_top > -1)
		{
			index = edefs.free_stack[edefs.stack_top--];
		}
		else
		{
			index = edefs.count++;
		}
		
		if(index > edefs.size)
		{
			entity_ResizeEntityDefList2(edefs.size + 16);
		}
		
		e = &edefs.defs[index];
		
		e->name = strdup(name);
		e->components = root;
		
		b = root;
		children[stack_top] = 0;
		while(b)
		{
			if(!children[stack_top])
			{
				count++;
			}
			if(children[stack_top] < b->child_count)
			{
				b = b->children[children[stack_top]];
				children[stack_top]++;
				stack_top++;
				children[stack_top] = 0;
			}
			else
			{
				stack_top--;
				b = b->parent;
			}
		}
		
		e->child_count = count;
		
		return index;
	}
	return -1;
}

PEWAPI int entity_CreateEntityDef(char *name, short flags, short material_index, short armdef_index, mesh_t *mesh, float mass, short collision_shape)
{
	int index = entity_defs.count;
	armdef_t *a;
	
	if(likely(index < entity_defs.size))
	{
		_add_entity_def:
			
		if(!mesh)
		{
			console_Print(MESSAGE_ERROR, "null mesh ptr for entity_def_t [%s]\n", name);
			return -1;
		}
		
		index = entity_defs.count;
		entity_defs.defs[index].name = strdup(name);
		entity_defs.defs[index].armdef_index = armdef_index;
		entity_defs.defs[index].material_index = material_index;
		entity_defs.defs[index].flags = flags;
		
		if(mass <= 0.0)
		{
			entity_defs.defs[index].flags |= ENTITY_STATIC_COLLISION;
		}
		
		//entity_defs.defs[index].type = type;
		entity_defs.defs[index].mesh = mesh;
		entity_defs.defs[index].mass = mass;
		entity_defs.defs[index].collision_shape = collision_shape;
		
		entity_defs.defs[index].start = mesh->start;
		entity_defs.defs[index].vert_count = mesh->vert_count;
		entity_defs.defs[index].draw_flags = mesh->draw_mode;
		
		entity_defs.defs[index].armdef_index = armdef_index;
		
		model_GetMaxMinsFromVertexData(mesh->v_data, &entity_defs.defs[index].o_maxmins[0], mesh->vert_count);
			
		if(mesh->n_data)
		{
			entity_defs.defs[index].draw_flags |= CBATTRIBUTE_NORMAL;
		}
		if(mesh->t_data)
		{
			entity_defs.defs[index].draw_flags |= CBATTRIBUTE_TANGENT;
		}
		if(mesh->t_c_data)
		{
			entity_defs.defs[index].draw_flags |= CBATTRIBUTE_TEX_COORD;
		}
		entity_defs.count++;
		
		if(!mesh) 
		{
			printf("warning: null mesh ptr in entity_def_t [%s] creation!\n", name);
		}
		
	}
	else
	{
		entity_ResizeEntityDefList(entity_defs.size + 16);
		goto _add_entity_def;
	}
	return index;
}

PEWAPI int entity_SpawnEntity(char *name, entity_def_t *entity_def, vec3_t position, mat3_t *orientation)
{
	int index;
	int byte_count;
	short vtype;
	short col_shape;
	mesh_t *vmesh;
	float vmass;
	armdef_t *a;
	armature_t *ar;
	char col_name[64];
	//char *armature_name = (char *)(&b_collide + 1);
	if(entity_def)
	{
		if(entity_a.stack_top>=0)
		{
			index = entity_a.free_positions_stack[entity_a.stack_top];
			entity_a.stack_top--;
		}
		else
		{
			if(entity_a.entity_count>=entity_a.array_size)
			{
				entity_ResizeEntityArray(&entity_a, entity_a.array_size<<1);
			}
				
			index = entity_a.entity_count;
			entity_a.entity_count++;	
		}
		
		a = armature_GetArmDefByIndex(entity_def->armdef_index);
		
		entity_a.aabb_data[index].o_maxmins[0] = entity_def->o_maxmins[0];
		entity_a.aabb_data[index].o_maxmins[1] = entity_def->o_maxmins[1];
		entity_a.aabb_data[index].o_maxmins[2] = entity_def->o_maxmins[2];
		
		entity_a.aabb_data[index].c_maxmins[0] = entity_a.aabb_data[index].o_maxmins[0];
		entity_a.aabb_data[index].c_maxmins[1] = entity_a.aabb_data[index].o_maxmins[1];
		entity_a.aabb_data[index].c_maxmins[2] = entity_a.aabb_data[index].o_maxmins[2];
		
		//MatrixCopy3(&entity_a.extra_data[index].local_orientation, orientation);
		memcpy(&entity_a.extra_data[index].local_orientation, orientation, sizeof(mat3_t));
		
		//entity_RotateEntity()
		
		entity_a.extra_data[index].local_position = position;
		entity_a.extra_data[index].name = strdup(name);
		entity_a.position_data[index].bm_flags = entity_def->flags;
		//entity_a.position_data[index].type = entity_def->type;
		
		entity_a.draw_data[index].start = entity_def->start;
		entity_a.draw_data[index].vert_count = entity_def->vert_count;
		entity_a.draw_data[index].draw_flags = entity_def->draw_flags;
		
		//entity_CalculateAABB(&entity_a.aabb_data[index], &entity_a.position_data[index]);
		
		if(!entity_def->mesh)
		{
			printf("warning: entity_def_t has a null pointer for entity [%s]!\n", name);
		}
		
		if(entity_def->type == ENTITY_PLAYER)
		{
			entity_a.draw_data[index].mesh = NULL;
		}
		else
		{
			entity_a.draw_data[index].mesh = entity_def->mesh;
		}
		
		
		if(a && entity_a.draw_data[index].mesh)
		{
			strcpy(armature_name, name);
			strcat(armature_name, "_armature");
			entity_a.draw_data[index].armature_index = armature_CreateArmature(a, armature_name, position, orientation);
			
			vmesh = entity_a.draw_data[index].mesh;
			byte_count = sizeof(float) * 3 * vmesh->vert_count;
			
			if(vmesh->n_data)
			{
				byte_count += sizeof(float) * 3 * vmesh->vert_count; 
			}
			if(vmesh->t_data)
			{
				byte_count += sizeof(float) * 3 * vmesh->vert_count;
			}
			if(vmesh->t_c_data)
			{
				byte_count += sizeof(float) * 2 * vmesh->vert_count;
			}
			
			entity_a.draw_data[index].handle = gpu_Alloc(byte_count);
			entity_a.draw_data[index].start = gpu_GetAllocStart(entity_a.draw_data[index].handle);
			gpu_Write(entity_a.draw_data[index].handle, 0, vmesh->v_data, byte_count, 0);
			entity_a.position_data[index].bm_flags |= ENTITY_SKINNABLE;
			
			ar = &armature_list.armatures[entity_a.draw_data[index].armature_index];
			ar->mesh = vmesh;
			ar->start = entity_a.draw_data[index].start; 
			
			//entity_SetEntityArmature(index, entity_a.draw_data[index].armature_index, a->wset_index);
		}
		else
		{
			entity_a.draw_data[index].armature_index = -1;
		}
		
		//entity_a.draw_data[index].armature_index = entity_def->armature_index;
		entity_a.draw_data[index].material_index = entity_def->material_index;
		
		memcpy(&entity_a.position_data[index].world_orientation, &entity_a.extra_data[index].local_orientation, sizeof(mat3_t));
		//MatrixCopy3(&entity_a.position_data[index].world_orientation, &entity_a.extra_data[index].local_orientation);
		entity_a.position_data[index].world_position = entity_a.extra_data[index].local_position;
		
		entity_a.aabb_data[index].origin.x = entity_a.position_data[index].world_position.x;
		entity_a.aabb_data[index].origin.y = entity_a.position_data[index].world_position.y;
		entity_a.aabb_data[index].origin.z = entity_a.position_data[index].world_position.z;
		
		entity_CalculateAABB(&entity_a.aabb_data[index], &entity_a.position_data[index]);
		
		if(entity_def->flags & ENTITY_COLLIDES)
		{
			switch(entity_def->flags & ENTITY_STATIC_COLLISION)
			{
				
				//case ENTITY_DYNAMIC:
				case 0:
					vtype = COLLIDER_RIGID_BODY;
					vmesh = entity_def->mesh;
					vmass = entity_def->mass;
					col_shape = COLLISION_SHAPE_CONVEX_HULL;
					goto _add;
				break;
				
				case ENTITY_STATIC_COLLISION:
					vtype = COLLIDER_STATIC;
					vmesh = entity_def->mesh;
					col_shape = COLLISION_SHAPE_CONVEX_HULL;
					vmass = 0.0;
					strcpy(col_name, name);
					strcat(col_name, "_collider");
					_add:
					entity_a.position_data[index].collider_index = physics_CreateCollider(col_name, vtype, col_shape, 0, index, 2.0, 1.0, 2.0, 0.8, vmass, 18.0 ,9.6, vmesh, position, orientation, NULL);
				break;
				
				/*case ENTITY_OTHER:
				default:
					entity_a.position_data[index].collider_index = -1;
				break;*/
				
			}
		}
		else
		{
			entity_a.position_data[index].collider_index = -1;
		}
		
		
		if(material_a.materials[entity_def->material_index].diff_color.a < 255)
		{
			entity_a.position_data[index].bm_flags |= ENTITY_TRANSLUCENT;
		}
		
		//entity_a.draw_data[entity_index].material_index = material_index;
		
		
		
		entity_a.position_data[index].entity_index = index;
		entity_a.extra_data[index].assigned_node = scenegraph_AddNode(NODE_ENTITY, index, -1, name);
		return index;
	}
	return -1;
}

PEWAPI void entity_SpawnEntityByIndex(int cdata_index, vec3_t position, mat3_t *orientation, int b_collide)
{
	
}

PEWAPI int entity_GetEntityArraySize()
{
	return entity_a.array_size;
}

PEWAPI entity_ptr entity_GetEntityByIndex(int entity_index)
{
	entity_ptr rtrn;
	if(entity_index>=0)
	{
		if(entity_a.position_data[entity_index].entity_index>=0)
		{
		    rtrn.position_data=&entity_a.position_data[entity_index];
		    rtrn.draw_data=&entity_a.draw_data[entity_index];
		    rtrn.extra_data = &entity_a.extra_data[entity_index];
		    rtrn.aabb_data = &entity_a.aabb_data[entity_index];
		    return rtrn;
		}

	}
	rtrn.position_data = NULL;
	rtrn.draw_data = NULL;
	rtrn.extra_data = NULL;
	rtrn.aabb_data = NULL;
	return rtrn;
	
}


PEWAPI entity_ptr entity_GetEntity(char *name)
{
	register int i;
	register int c;
	entity_ptr rtrn;
	c=entity_a.array_size;
	for(i=0; i<c; i++)
	{
		if(entity_a.position_data[i].entity_index>=0)
		{
			if(!strcmp(name, entity_a.extra_data[i].name))
			{
				rtrn.position_data=&entity_a.position_data[i];
			    rtrn.draw_data=&entity_a.draw_data[i];
			    rtrn.extra_data = &entity_a.extra_data[i];
			    rtrn.aabb_data = &entity_a.aabb_data[i];
			    return rtrn;
			}
		}
	}
	rtrn.position_data = NULL;
	rtrn.draw_data = NULL;
	rtrn.extra_data = NULL;
	rtrn.aabb_data = NULL;
	return rtrn;
}

PEWAPI entity_def_t *entity_GetEntityDef(char *name)
{
	int i;
	int c = entity_defs.count;
	for(i = 0; i < c; i++)
	{
		if(!strcmp(entity_defs.defs[i].name, name))
		{
			return &entity_defs.defs[i];
		}
	}
	return NULL;
}

PEWAPI armature_t *entity_GetEntityArmature(int entity_index)
{
	if(entity_index > -1)
	{
		return &armature_list.armatures[entity_a.draw_data[entity_index].armature_index];
	}
}

PEWAPI void entity_DestroyEntity(entity_ptr entity)
{
	if(entity.draw_data && entity.position_data)
	{
		entity_a.free_positions_stack[++entity_a.stack_top]=entity.position_data->entity_index;
		entity.position_data->entity_index=-1;
		
		if(entity.position_data->bm_flags & ENTITY_SKINNABLE)
		{
			gpu_Free(entity.draw_data->handle);
			armature_list.armatures[entity.draw_data->armature_index].mesh = NULL;
		}

		scenegraph_RemoveNode(entity.extra_data->assigned_node, 0);
	}
}

/*
=============
entity_SetParent
=============
*/
PEWAPI void entity_SetParent(entity_t *entity, entity_t *parent, int b_keep_transform)
{
	/*scenegraph_SetParent(entity->assigned_node, parent->assigned_node, b_keep_transform);
	return;*/
}


/*
=============
entity_RemoveParent
=============
*/
PEWAPI void entity_RemoveParent(entity_t *entity, int b_keep_transform)
{
	/*scenegraph_RemoveParent(entity->assigned_node, b_keep_transform);
	return;*/
}


PEWAPI entity_t *entity_2DRayCast(entity_t *from, entity_t *to, vec2_t *intersection)
{
	/*return physics_2DRayCast(from->world_position, to->world_position, intersection);*/
}

PEWAPI entity_ptr entity_RayCast(vec3_t from, vec3_t to)
{
	entity_ptr p = {NULL, NULL, NULL, NULL};
	general_collider_t *c;
	c = physics_RayCast(from, to);
	if(c)
	{
		p = entity_GetEntityByIndex(c->base.index);
	}
	
	return p;
}

/*
=============
entity_ProcessEntities
=============
*/
PEWAPI void entity_ProcessEntities()
{
	register int i;
	register int c;
	c=entity_a.entity_count;
	mat4_t transform;
	mat4_t model_view_matrix;
	command_buffer_t cb;
	camera_ResetWorldToCameraMatrix();
	
	for(i=0; i<c; i++)
	{
		// do frustum culling stuff... 
		/*mat4_t_compose(&transform, &entity_a.entity_position[i].orientation, entity_a.entity_position[i].local_position);
		mat4_t_mult(&model_view_matrix, &transform, &camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix);
		
		entity_a.entity_position[i].world_position.floats[0]=transform.floats[3][0];
		entity_a.entity_position[i].world_position.floats[1]=transform.floats[3][1];
		entity_a.entity_position[i].world_position.floats[2]=transform.floats[3][2];*/
		
		/*draw_FillCommandBuffer(&cb, &entity_a.entities[i].mesh, entity_a.entities[i].material_index, 0, &model_view_matrix);
		draw_DispatchCommandBuffer(&cb);*/
		
		
		/*mat4_t_compose(&transform, &entity_a.entities[node_index].orientation, entity_a.entities[node_index].local_position);
		
		mat4_t_mult(&c_transform, &transform, parent_transform);
		
		entity_a.entities[node_index].world_position.floats[0]=c_transform.floats[3][0];
		entity_a.entities[node_index].world_position.floats[1]=c_transform.floats[3][1];
		entity_a.entities[node_index].world_position.floats[2]=c_transform.floats[3][2];
		
		
		if(entity_a.entities[node_index].type==ENTITY_NULL) goto NO_SUBMIT;
		
		mat4_t_mult(&model_view_matrix, &c_transform, &camera_a.cameras[renderer.active_camera_index].world_to_camera_matrix);*/
		
	}
}


/*
=============
entity_RotateEntity
=============
*/
PEWAPI void entity_RotateEntity(entity_ptr *entity, vec3_t axis, float angle, int set_rotation)
{
	float temp;
	float v[24];
	//mat3_t_rotate(&entity->local_orientation, axis, angle, set_rotation);
	mat3_t r;
	mat4_t transform;
	r = entity->extra_data->local_orientation;
	mat3_t_rotate(&r, axis, angle, set_rotation);
	btVector3 max;
	btVector3 min;
	btTransform t;
	btDefaultMotionState *ms;
	general_collider_t *collider;
	if(entity->position_data->collider_index >= 0)
	{
		
		if(!(entity->position_data->bm_flags & ENTITY_OVERRIDE_BULLET_ROTATION))
		{
			return;
		}
		collider = &collider_a.colliders[entity->position_data->collider_index];
		btMatrix3x3 m;
		
		
		memcpy(&entity->extra_data->local_orientation, &r, sizeof(mat3_t));
		
		/* bullet seems to use a left hand coordinate system, while
		the Cycle engine is using the right handed convention. That's why
		we're transposing the matrix here. Inverse rotation from one 
		another, and stuff... */
		mat3_t_transpose(&r);
		
		m[0].setValue(r.floats[0][0], r.floats[0][1], r.floats[0][2]);
		m[1].setValue(r.floats[1][0], r.floats[1][1], r.floats[1][2]);
		m[2].setValue(r.floats[2][0], r.floats[2][1], r.floats[2][2]);
		
		
		
		collider->base.rigid_body->activate(true);
		collider->base.rigid_body->getMotionState()->getWorldTransform(t);
		t.setBasis(m);
		collider->base.rigid_body->getMotionState()->setWorldTransform(t);
		collider->base.rigid_body->getAabb(min, max);
		entity->aabb_data->c_maxmins[0] = max[0] - entity->aabb_data->origin.x;
		entity->aabb_data->c_maxmins[1] = max[1] - entity->aabb_data->origin.y;
		entity->aabb_data->c_maxmins[2] = max[2] - entity->aabb_data->origin.z;
	}
	else
	{
		
		//entity->local_orientation = r;
		
		memcpy(&entity->extra_data->local_orientation, &r, sizeof(mat3_t));
		
		v[0]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][0] + 
		 	entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][0] - 
		 	entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][0];
				
		v[1]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][1] + 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][1] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][1];
		
		v[2]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][2] + 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][2] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][2];
				 
			 
		v[3]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][0] - 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][0] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][0];
					
		v[4]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][1] - 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][1] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][1];
			
		v[5]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][2] - 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][2] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][2];
				 
								
		v[6]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][0] - 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][0] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][0];
						
		v[7]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][1] - 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][1] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][1];
			
		v[8]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][2] - 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][2] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][2];	
				 
				 
		v[9]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][0] + 
			 entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][0] - 
			 entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][0];
						
		v[10]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][1] + 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][1] - 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][1];
			
		v[11]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][2] + 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][2] - 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][2];

		v[12]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][0] + 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][0] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][0];
						
		v[13]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][1] + 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][1] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][1];
			
		v[14]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][2] + 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][2] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][2];
				  
						
		v[15]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][0] - 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][0] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][0];
						
		v[16]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][1] - 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][1] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][1];
			
		v[17]=-entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][2] -
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][2] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][2];
				  
						
		v[18]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][0] - 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][0] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][0];
						
		v[19]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][1] - 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][1] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][1];
			
		v[20]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][2] - 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][2] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][2];	
				  
				 
		v[21]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][0] + 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][0] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][0];
						
		v[22]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][1] + 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][1] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][1];
			
		v[23]=entity->aabb_data->o_maxmins[0]*entity->extra_data->local_orientation.floats[0][2] + 
			  entity->aabb_data->o_maxmins[1]*entity->extra_data->local_orientation.floats[1][2] + 
			  entity->aabb_data->o_maxmins[2]*entity->extra_data->local_orientation.floats[2][2];

		model_GetMaxMinsFromVertexData(v, &entity->aabb_data->c_maxmins[0], 8);
	}
			
	entity->position_data->bm_flags |= ENTITY_HAS_MOVED;

	//printf("[%f %f %f]\n[%f %f %f]\n\n", entity->c_maxmins[0], entity->c_maxmins[1], entity->c_maxmins[2], entity->c_maxmins[3],entity->c_maxmins[4],entity->c_maxmins[5]);
	
}


PEWAPI void entity_TranslateEntity(entity_ptr *entity, vec3_t direction, float amount, int b_set)
{
	mat4_t transform;
	general_collider_t *collider;
	if(!b_set)
	{
		entity->extra_data->local_position.floats[0]+=direction.floats[0]*amount;
		entity->extra_data->local_position.floats[1]+=direction.floats[1]*amount;
		entity->extra_data->local_position.floats[2]+=direction.floats[2]*amount;
	}
	else
	{
		entity->extra_data->local_position.floats[0]=direction.floats[0]*amount;
		entity->extra_data->local_position.floats[1]=direction.floats[1]*amount;
		entity->extra_data->local_position.floats[2]=direction.floats[2]*amount;
	}
	
	if(entity->position_data->collider_index >= 0)
	{
		collider = &collider_a.colliders[entity->position_data->collider_index];
		//memcpy(&transform, &entity->position_data->world_transform, sizeof(mat4_t));
		//transform.floats[3][0] = entity->extra_data->local_position.floats[0];
		//transform.floats[3][1] = entity->extra_data->local_position.floats[1];
		//transform.floats[3][2] = entity->extra_data->local_position.floats[2];
		//collider->rigid_body->getWorldTransform().setFromOpenGLMatrix((btScalar *)&transform.floats[0][0]);
		collider->base.rigid_body->getWorldTransform().setOrigin(btVector3(entity_a.extra_data->local_position.x, entity_a.extra_data->local_position.y, entity_a.extra_data->local_position.z));
		collider->base.rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
		collider->base.rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
	}
	
	entity->position_data->bm_flags |= ENTITY_HAS_MOVED;
	
	
}

void entity_CalculateAABB(entity_aabb_t *aabb, entity_position_t *position)
{
	float v[24];
	int i;
	float x = -9999999999999.0;
	float y = -9999999999999.0;
	float z = -9999999999999.0;
	
	v[0]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][0] + 
	 	aabb->o_maxmins[1]*position->world_orientation.floats[1][0] - 
	 	aabb->o_maxmins[2]*position->world_orientation.floats[2][0];
				
	v[1]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][1] + 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][1] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][1];
		
	v[2]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][2] + 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][2] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][2];
				 
			 
	v[3]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][0] - 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][0] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][0];
					
	v[4]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][1] - 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][1] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][1];
			
	v[5]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][2] - 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][2] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][2];
				 
								
	v[6]=aabb->o_maxmins[0]*position->world_orientation.floats[0][0] - 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][0] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][0];
						
	v[7]=aabb->o_maxmins[0]*position->world_orientation.floats[0][1] - 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][1] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][1];
			
	v[8]=aabb->o_maxmins[0]*position->world_orientation.floats[0][2] - 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][2] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][2];	
				 
				 
	v[9]=aabb->o_maxmins[0]*position->world_orientation.floats[0][0] + 
		 aabb->o_maxmins[1]*position->world_orientation.floats[1][0] - 
		 aabb->o_maxmins[2]*position->world_orientation.floats[2][0];
						
	v[10]=aabb->o_maxmins[0]*position->world_orientation.floats[0][1] + 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][1] - 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][1];
			
	v[11]=aabb->o_maxmins[0]*position->world_orientation.floats[0][2] + 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][2] - 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][2];

	v[12]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][0] + 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][0] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][0];
						
	v[13]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][1] + 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][1] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][1];
			
	v[14]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][2] + 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][2] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][2];
				  
						
	v[15]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][0] - 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][0] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][0];
						
	v[16]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][1] - 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][1] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][1];
			
	v[17]=-aabb->o_maxmins[0]*position->world_orientation.floats[0][2] -
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][2] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][2];
				  
						
	v[18]=aabb->o_maxmins[0]*position->world_orientation.floats[0][0] - 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][0] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][0];
						
	v[19]=aabb->o_maxmins[0]*position->world_orientation.floats[0][1] - 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][1] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][1];
			
	v[20]=aabb->o_maxmins[0]*position->world_orientation.floats[0][2] - 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][2] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][2];	
				  
				 
	v[21]=aabb->o_maxmins[0]*position->world_orientation.floats[0][0] + 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][0] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][0];
						
	v[22]=aabb->o_maxmins[0]*position->world_orientation.floats[0][1] + 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][1] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][1];
			
	v[23]=aabb->o_maxmins[0]*position->world_orientation.floats[0][2] + 
		  aabb->o_maxmins[1]*position->world_orientation.floats[1][2] + 
		  aabb->o_maxmins[2]*position->world_orientation.floats[2][2];
		 
	for(i = 0; i < 8; i++)
	{
		if(v[i * 3] > x) x = v[i * 3];
		if(v[i * 3 + 1] > y) y = v[i * 3 + 1];
		if(v[i * 3 + 2] > z) z = v[i * 3 + 2];
	}
	aabb->c_maxmins[0] = x;
	aabb->c_maxmins[1] = y;
	aabb->c_maxmins[2] = z;	  
	//model_GetMaxMinsFromVertexData(v, &aabb->c_maxmins[0], 8);	  
	
	aabb->origin = position->world_position;
}

PEWAPI void entity_ApplyForce(entity_ptr *entity, vec3_t direction, vec3_t relative_position)
{
	general_collider_t *c;
	if(entity)
	{
		if(entity->position_data->collider_index >= 0)
		{
			c = physics_GetColliderByIndex(entity->position_data->collider_index);
			c->base.rigid_body->activate(true);
			c->base.rigid_body->applyForce(btVector3(direction.x, direction.y, direction.z), btVector3(relative_position.x, relative_position.y, relative_position.z));
		}
	}
}


PEWAPI void entity_ApplyImpulse(entity_ptr *entity, vec3_t impulse, vec3_t relative_position)
{
	if(entity)
	{
		/*if(entity->position_data->collider_index >= 0)
		{
			c = physics_GetColliderByIndex(entity->position_data->collider_index);
			c->rigid_body->activate(true);
			c->rigid_body->applyImpulse(btVector3(direction.x, direction.y, direction.z), btVector3(relative_position.x, relative_position.y, relative_position.z));
		}*/
	}
}


PEWAPI void entity_ApplyTorque(entity_ptr *entity, vec3_t torque)
{
	if(entity)
	{
		/*if(entity->rigid_body)
		{
			entity->rigid_body->applyTorque((btVector3 &) torque);
		}*/
	}
}

PEWAPI vec3_t entity_GetEntityLocalForwardVector(entity_ptr *entity)
{
	vec3_t v;
	v.floats[0]=entity->extra_data->local_orientation.floats[2][0];
	v.floats[1]=entity->extra_data->local_orientation.floats[2][1];
	v.floats[2]=entity->extra_data->local_orientation.floats[2][2];
	return v;
}

PEWAPI vec3_t entity_GetEntityLocalRightVector(entity_ptr *entity)
{
	vec3_t v;
	v.floats[0]=entity->extra_data->local_orientation.floats[0][0];
	v.floats[1]=entity->extra_data->local_orientation.floats[0][1];
	v.floats[2]=entity->extra_data->local_orientation.floats[0][2];
	return v;
}

PEWAPI vec3_t entity_GetEntityLocalUpVector(entity_ptr *entity)
{
	vec3_t v;
	v.floats[0]=entity->extra_data->local_orientation.floats[1][0];
	v.floats[1]=entity->extra_data->local_orientation.floats[1][1];
	v.floats[2]=entity->extra_data->local_orientation.floats[1][2];
	return v;
}


PEWAPI vec3_t entity_GetEntityWorldForwardVector(entity_ptr *entity)
{
	vec3_t v;
	v.floats[0]=entity->position_data->world_orientation.floats[2][0];
	v.floats[1]=entity->position_data->world_orientation.floats[2][1];
	v.floats[2]=entity->position_data->world_orientation.floats[2][2];
	return v;
}

PEWAPI vec3_t entity_GetEntityWorldRightVector(entity_ptr *entity)
{
	vec3_t v;
	v.floats[0]=entity->position_data->world_orientation.floats[0][0];
	v.floats[1]=entity->position_data->world_orientation.floats[0][1];
	v.floats[2]=entity->position_data->world_orientation.floats[0][2];
	return v;
}

PEWAPI vec3_t entity_GetEntityWorldUpVector(entity_ptr *entity)
{
	vec3_t v;
	v.floats[0]=entity->position_data->world_orientation.floats[1][0];
	v.floats[1]=entity->position_data->world_orientation.floats[1][1];
	v.floats[2]=entity->position_data->world_orientation.floats[1][2];
	return v;
}


/* this function is subject to 'drifting'. It is not recomended
to use it while the camera is moving... */
PEWAPI void entity_QueryEntityUnderCursor()
{
	/* just a single query allowed */
	int i;
	get_entity_under_cursor = 1;
	mouse_x = input.mouse_x;
	mouse_y = input.mouse_y;
	
	i = scenegraph_GetEntityUnderMouse();
	
	if(i >= 0)
	{
		selected_entity.position_data = &entity_a.position_data[i];
		selected_entity.draw_data = &entity_a.draw_data[i];
		selected_entity.aabb_data = &entity_a.aabb_data[i];
		selected_entity.extra_data = &entity_a.extra_data[i];
	}
	
}

PEWAPI entity_ptr entity_GetEntityUnderCursor()
{
	return selected_entity;
}




#ifdef __cplusplus
}
#endif











