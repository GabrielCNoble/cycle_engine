#ifndef ENTITY_H
#define ENTITY_H

#include "conf.h"
#include "includes.h"
#include "model.h"
#include "scenegraph.h"
#include "physics.h"
#include "armature.h"



enum ENTITY_FLAGS
{
	ENTITY_HAS_MOVED = 1,
	ENTITY_TRANSLUCENT = 1<<1,
	ENTITY_STATIC = 1 << 2,		
	ENTITY_INVISIBLE = 1<<3,
	ENTITY_SKINNABLE = 1<<4,
	ENTITY_COLLIDES = 1<<5,
	//ENTITY_STATIC_COLLISION = 1<<6
};

enum COMPONENT_FLAGS
{
	COMPONENT_GEOMETRY_COLLIDE = 1,
	COMPONENT_GEOMETRY_ARMATURE = 1 << 1,
};



enum ENTITY_TYPE
{
	ENTITY_PLAYER=1,		
	ENTITY_DYNAMIC,
	ENTITY_OTHER,			
	ENTITY_KINEMATIC,
	ENTITY_SENSOR
};


enum ENTITY_COLLISION_SHAPE
{
	ENTITY_COLLISION_SHAPE_BOX,
	ENTITY_COLLISION_SHAPE_SPHERE,
	ENTITY_COLLISION_SHAPE_CONE,
	ENTITY_COLLISION_SHAPE_CYLINDER,
	ENTITY_COLLISION_SHAPE_CONVEX_HULL
};

enum ENTITY_COMPONENT
{
	ENTITY_COMPONENT_BASE = 0,
	ENTITY_COMPONENT_GEOMETRY,						/* anything drawable */
	
	ENTITY_COMPONENT_ARMATURE,						/* a component can have just one armature as child component, but an entity can  
													   have a limitiless count of armatures */
													   
	ENTITY_COMPONENT_COLLIDER,						/* a component can have just one collider as child component, and the entity should
													   have a single collider */
													   
	ENTITY_COMPONENT_LIGHT,							/* a component may have as many lights as child component */
	
	ENTITY_COMPONENT_CAMERA,

};

enum HOOK_TYPE
{
	HOOK_INDEX,
	HOOK_ADDRESS
};

typedef union hook_val
{
	int hook_index;				/* if this hook is an index */
	void *hook_ptr;
}hook_val;

typedef struct hook_t
{
	hook_val value;
	int type;
}hook_t;

/*typedef struct entity_t
{
	mesh_t mesh;
	mat3_t orientation;
	vec3_t local_position;
	vec3_t world_position;
	float o_maxmins[6];
	float c_maxmins[6];
	collider_t *collider;		
	int collider_index;			
	int material_index;
	int texture_layer;
	hook_t hook;			
	int entity_index;
	node_t *assigned_node; 
	int type;
	int bm_state;
	char *name;
}entity_t;*/


typedef struct
{
	mat3_t world_orientation;
	vec3_t world_position;
	//short type;											
	short bm_flags;
	short collider_index;
	short entity_index;
	short affecting_lights_index;
	//char align[8];
}entity_position_t;

typedef struct
{
	//mat4_t last_model_view_matrix;
	mat3_t local_orientation;
	vec3_t local_position;
	//mat3_t last_world_orientation;
	//vec3_t last_world_position;
	node_t *assigned_node;
	char *name;
}entity_extra_t;

typedef struct
{					
	short material_index;				
	short draw_flags;
	int vert_count;
	int start;
	int handle;	
	mesh_t *mesh;							/* is this really necessary here? it could go as extra data... */		
	short armature_index;					/* index into the armature array... could be a short... */			
}entity_draw_t;

typedef struct
{
	float o_maxmins[3];
	float c_maxmins[3];
	vec3_t origin;
}entity_aabb_t;



typedef struct comp_base_t				/* component base */
{
	mat3_t orientation;					/* orientation relative to its parent */
	vec3_t position;					/* position relative to its parent */
	char *name;
	short comp_type;
	short index;						/* if this index is different from -1, it means that
										   whatever this component may be, it is to be parented
										   to a bone instead of an armature. */
	short align;
	short child_count;
	short child_size;
	struct comp_base_t *parent;
	struct comp_base_t **children;
}comp_base_t;

typedef struct
{
	comp_base_t base;
	float o_maxmins[3];
	mesh_t *mesh;
	int vert_count;
	int start;
	short flags;
	short type;
	short draw_flags;
	short material_index;
	short armdef_index;
}comp_geometry_t;

typedef struct
{
	comp_base_t base;
	short armdef_index;
}comp_armature_t;

typedef struct
{
	comp_base_t base;
	short flags;
	short collision_shape;
}comp_collider_t;

typedef struct
{
	comp_base_t base;
	vec3_t color;
	float radius;
	short flags;
	short max_shadow_map_res;
	short max_volume_samples;
}comp_light_t;

typedef struct
{
	char *name;
	int child_count;
	comp_base_t *components;
}edef_t;

typedef struct
{
	int size;
	int count;
	int stack_top;
	int *free_stack;
	edef_t *defs; 
}edef_list_t;

typedef struct entity_def_t 
{
	mesh_t *mesh;
	float o_maxmins[3];
	float mass;
	char *name;	
	int vert_count;
	int start;
	short flags;									
	short type;
	short draw_flags;					
	short material_index;
	short armdef_index;
	short collision_shape;
}entity_def_t;						


typedef struct entity_t
{
	entity_position_t position_data;
	entity_draw_t draw_data;
	entity_extra_t extra_data;
}entity_t;

typedef struct
{
	entity_position_t *position_data;
	entity_draw_t *draw_data;
	entity_extra_t *extra_data;
	entity_aabb_t *aabb_data;
}entity_ptr;

typedef struct
{
	int array_size;
	int stack_top;
	int *free_positions_stack;
	int entity_count;
	//entity_t *entities;
	entity_position_t *position_data;	/*  */
	entity_draw_t *draw_data;			/*  */
	entity_extra_t *extra_data;			/*  */
	entity_aabb_t *aabb_data;			/*  */
}entity_array;

typedef struct
{
	int size;
	int count;
	entity_def_t *defs;
}entity_def_list;

#ifdef __cplusplus
extern "C"
{
#endif

void entity_Init();

void entity_Finish();

void entity_ResizeEntityArray(entity_array *array, int new_size);

void entity_ResizeEntityDefList(int new_size);

void entity_ResizeEntityDefList2(int new_size);

//PEWAPI int entity_CreateEntityFromData(entity_t *entity, int b_collide, int b_static, void (*collision_callback)(void *, void *, vec2_t *), hook_t hook);

//PEWAPI int entity_CreateEntity(char *name, int type, int flags, vec3_t position, mat3_t *orientation, float mass, mesh_t *mesh, int material_index, int b_collide);

PEWAPI void entity_SetEntityArmature(int entity_index, int armature_index);

PEWAPI comp_base_t *entity_CreateComponent(char *name, int type, int flags, vec3_t *position, mat3_t *orientation);

PEWAPI comp_base_t *entity_CreateGeometryComponent(char *name, int flags, mesh_t *mesh, int material_index, int armdef_index, float mass, vec3_t *position, mat3_t *orientation);

PEWAPI void entity_AddChildComponent(comp_base_t *parent, comp_base_t *child);

PEWAPI comp_base_t *entity_RemoveChildComponent(comp_base_t *parent, char *name);

PEWAPI comp_base_t *entity_RemoveChildComponentByIndex(comp_base_t *parent, int index);

PEWAPI int entity_CreateEntityDef2(char *name, comp_base_t *root);

PEWAPI int entity_CreateEntityDef(char *name, short flags, short material_index, short armdef_index, mesh_t *mesh, float mass, short collision_shape);

PEWAPI int entity_SpawnEntity(char *name, entity_def_t *entity_def, vec3_t position, mat3_t *orientation);

PEWAPI void entity_SpawnEntityByIndex(int cdata_index, vec3_t position, mat3_t *orientation, int b_collide);

PEWAPI int entity_GetEntityArraySize();

PEWAPI entity_ptr entity_GetEntityByIndex(int entity_index);

PEWAPI entity_ptr entity_GetEntity(char *name);

PEWAPI entity_def_t *entity_GetEntityDef(char *name);

PEWAPI armature_t *entity_GetEntityArmature(int entity_index);

PEWAPI void entity_DestroyEntity(entity_ptr entity);

PEWAPI void entity_SetParent(entity_t *entity, entity_t *parent, int b_keep_transform);

PEWAPI void entity_RemoveParent(entity_t *entity, int b_keep_transform);

PEWAPI entity_t *entity_2DRayCast(entity_t *from, entity_t *to, vec2_t *intersection);

PEWAPI entity_ptr entity_RayCast(vec3_t from, vec3_t to);

PEWAPI void entity_ProcessEntities();

PEWAPI void entity_RotateEntity(entity_ptr *entity, vec3_t axis, float angle, int set_rotation);

PEWAPI void entity_TranslateEntity(entity_ptr *entity, vec3_t direction, float amount, int b_set);

void entity_CalculateAABB(entity_aabb_t *aabb, entity_position_t *position);

PEWAPI void entity_ApplyForce(entity_ptr *entity, vec3_t direction, vec3_t relative_position);

PEWAPI void entity_ApplyImpulse(entity_ptr *entity, vec3_t impulse, vec3_t relative_position);

PEWAPI void entity_ApplyTorque(entity_ptr *entity, vec3_t torque);

PEWAPI vec3_t entity_GetEntityLocalForwardVector(entity_ptr *entity);

PEWAPI vec3_t entity_GetEntityLocalRightVector(entity_ptr *entity);

PEWAPI vec3_t entity_GetEntityLocalUpVector(entity_ptr *entity);

PEWAPI vec3_t entity_GetEntityWorldForwardVector(entity_ptr *entity);

PEWAPI vec3_t entity_GetEntityWorldRightVector(entity_ptr *entity);

PEWAPI vec3_t entity_GetEntityWorldUpVector(entity_ptr *entity);

PEWAPI void entity_QueryEntityUnderCursor();

PEWAPI entity_ptr entity_GetEntityUnderCursor();




#ifdef __cplusplus
}
#endif





#endif /* ENTITY_H */
