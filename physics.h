#ifndef PHYSICS_H
#define PHYSICS_H

#include "conf.h"
//#include "includes.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "scenegraph.h"
#include "physics.h"
#include "model.h"
#include "gmath/vector.h"
#include "gmath/matrix.h"
#include "gmath/plane.h"
//#include "entity.h"

//struct entity_t;

enum COLLIDER_FLAGS
{
	COLLIDER_FLYING = 1,
	COLLIDER_JUST_JUMPED = 1<<1,					/* active for a single frame. Useful whenever an action has to be taken in the same frame the collider jumped */
	COLLIDER_JUMPED = 1<<2,							/* this will tell the way the collider got in the flying state. If this is set, then the collider jumped. Otherwise,
												   the collider fell from a ledge or something... */
	COLLIDER_JUST_LANDED = 1<<3,											   
	COLLIDER_ASCENDING = 1<<4,
	COLLIDER_CROUCHED = 1<<5,
	COLLIDER_MOVING = 1<<6,
												   
	COLLIDER_LOCK_X_ROT = 1<<7,					/* those are used mostly for the dynamic collider... */
	COLLIDER_LOCK_Y_ROT = 1<<8,
	COLLIDER_LOCK_Z_ROT = 1<<9,
	COLLIDER_LOCK_X_LOC = 1<<10,
	COLLIDER_LOCK_Y_LOC = 1<<11,
	COLLIDER_LOCK_Z_LOC = 1<<12,
	COLLIDER_IMMUTABLE_INDEX = 1<<13,					/* rigid bodies that don't have this flag might get their index changed in an array defrag operation. 
													   This flag ensures its index won't change */	
													   
	COLLIDER_CREATE_SCENEGRAPH_NODE = 1<<14												   											   
	
};

enum COLLISION_SHAPE
{
	COLLISION_SHAPE_NONE = 0,
	COLLISION_SHAPE_BOX,
	COLLISION_SHAPE_SPHERE,
	COLLISION_SHAPE_CAPSULE,
	COLLISION_SHAPE_CYLINDER,
	COLLISION_SHAPE_CONE,
	COLLISION_SHAPE_CONVEX_HULL,
	COLLISION_SHAPE_TRIANGLE_MESH
};

enum COLLIDER_TYPE
{
	COLLIDER_CHARACTER_CONTROLLER = 1,
	COLLIDER_STATIC,
	COLLIDER_RIGID_BODY
};

enum CONSTRAINT_TYPE
{
	CONSTRAINT_HINGE,
	CONSTRAINT_SLIDER, 
	CONSTRAINT_POINT_TO_POINT
};

enum HINGE_CONSTRAINT_FLAGS
{
	HINGE_LOCKED = 1,
	HINGE_JUST_HIT_MIN = 2,
	HINGE_JUST_HIT_MAX = 4,
};

/* NOTE: to do ray-casting and getting the closest collision, it is
necessary to declare a btCollisionWorld :: ClosestRayResultCallback, 
pass the from and to params, and then call physics_world->rayTest
passing the 'from', 'to' and the callback as params. Then check if there's
a collision using .hasHit() member function of the callback. To
find the point where the ray hit, just get the from point, 
and use its .lerp() member function, passing 'to' and
getting the .m_hitFraction member from the callback. Funny, instead 
of returning the point, bullet just returns the baricentric coord
from the line from-to...*/

/*
	NOTE: a better implementation to a character controller would be
	to use a capsule shape with a ghost object. The ghost object
	would be used to determine if the capsule is touching something
	on the ground. This will give a more consistent test, as no cracks
	will give incorrect results. To the controller step up a step
	smoothly, there's two ways this can be solved. First: one could
	detect the step up 'event', and interpolate between the current 
	height and the step height. Second: stairs and such could
	use a simplified collision mesh, with an invisible ramp
	over its steps. The second one would be better in both
	implementation and performance, for the bullet engine wouldn't
	need to test collision with hundreds of triangles. This one 
	won't solve the problem of stepping up objects loose in the ground.
	The controller will just snap to the object's height, unless
	that object was completely intended for the player to get on top of.
	This would add unecessary pressure on the art pipeline, forcing map
	creators to model EVERYTHING.  
*/

/*typedef struct
{
	btRigidBody *rigid_body;
	void (*collision_callback)(int entity_a_index , int entity_b_index);
	struct constraint_t *constraint;
	float step_height;
	float ground_height;
	float ground_crouch_height;
	float height;
	float crouch_height;
	float width;
	float jump_force;
	int entity_index;
	int collider_index;
	short bm_flags;
	short type;
}collider_t;	*/								/* colliders used in constraints will be forced immutable... */


typedef struct
{
	//mat3_t orientation;
	btRigidBody *rigid_body;
	char *name;																				/**/
	void (*collision_callback)(int entity_a_index , int entity_b_index);	
	node_t *assigned_node;
	float mass;																								/**/
	int collider_index;																						/**/
	int index;
	short collision_shape;																					/**/
	short type;																								/**/
	short bm_flags;	
}collider_base_t;



/**************************************************************************************************************/
typedef struct																								/**/	
{																											/**/
	collider_base_t base;																						/**/
	/***********************/																				/**/
	btPairCachingGhostObject *ghost_object;																	/**/
	float step_height;																						/**/
	float height;																							/**/
	float width;																							/**/
	float jump_force;																						/**/
}character_controller_t;																					/**/
																											/**/
typedef struct																								/**/
{																											/**/
	collider_base_t base;																						/**/
	/***********************/																				/**/
	struct constraint_t *constraint;																		/**/
	int material_index;																						/**/
	int align0;																								/**/
	int align1;																								/**/
	int align2;																								/**/
}general_collider_t;																						/**/
/**************************************************************************************************************/


typedef struct
{
	btConvexHullShape *convex_hull;
	btCapsuleShape *capsule;
	btBoxShape *box;
	btConeShape *cone;
	btCylinderShape *cylinder;
	btBvhTriangleMeshShape *triangle_mesh;
	short type;
	short align;
}coldata_t;

typedef struct
{
	int size;
	int count;
	coldata_t *coldata;
}coldata_list_t;


typedef struct
{
	int size;
	int count;
	int stack_size;
	int stack_top;
	int *free_positions_stack;
	//collider_t *base;
	//collider_t *colliders;
	general_collider_t *base;
	general_collider_t *colliders;
}collider_array;

typedef struct constraint_t 
{
	constraint_t *next;
	btTypedConstraint *constraint;
	int collider_a;
	int collider_b;
	float lower_limit;
	float upper_limit;
	short type;
	short flags;
	
}constraint_t;	/* we won't be iterating this list frequently, so a sllist will do little damage... */



#ifdef __cplusplus
extern "C"
{
#endif
void physics_Init();

void physics_Finish();

void physics_ResizeColliderArray(int new_size);

void physics_DefragColliderArray();

PEWAPI int physics_CreateCollider(char *name, short type, short collision_shape, short bm_flags, int entity_index, float ground_height, float step_height, float height, float width, float mass, float gravity, float jump_force, mesh_t *collision_shape_mesh, vec3_t position, mat3_t *orientation, void (*collision_callback)(int entity_a_index , int entity_b_index));

PEWAPI int physics_CreateCharacterController(short bm_flags, int entity_index, float height, float width, float step_height, float gravity, float jump_force, vec3_t position, void (*collision_callback)(int entity_a_index , int entity_b_index));

PEWAPI int physics_CreateGeneralCollider(short bm_flags, int entity_index, float mass, mesh_t *collision_shape_mesh, vec3_t position, mat3_t *orientation, void (*collision_callback)(int entity_a_index , int entity_b_index));

PEWAPI void physics_DestroyCollider(general_collider_t *collider);

PEWAPI void physics_DestroyColliderByIndex(int collider_index);

PEWAPI void physics_CreateConstraint(int a_collider, int b_collider, short type, vec3_t a_pivot, vec3_t b_pivot, vec3_t a_axis, vec3_t b_axis, float upper_limit, float lower_limit);

PEWAPI void physics_DeleteConstraint(constraint_t *constraint);

PEWAPI void physics_LockHinge(constraint_t *constraint);

PEWAPI void physics_UnlockHinge(constraint_t *constraint);

PEWAPI void physics_SetHingeLimits(constraint_t *constraint, float lower_limit, float upper_limit);

PEWAPI float physics_GetHingeAngle(constraint_t *constraint);

PEWAPI general_collider_t *physics_RayCast(vec3_t from, vec3_t to);

void physics_ProcessCollisions(float delta_time);

void physics_ProcessColliderMovement(character_controller_t *controller);

PEWAPI void physics_Jump(character_controller_t *controller);

PEWAPI void physics_Move(character_controller_t *controller, vec3_t direction);

PEWAPI void physics_Crouch(character_controller_t *controller);

PEWAPI void physics_Yaw(character_controller_t *controller, float yaw);



//PEWAPI void physics_SetColliderPosition(collider_t *collider, vec2_t position, int b_set);

//PEWAPI void physics_SetColliderVelocity(collider_t *collider, vec2_t velocity, int b_set);

//PEWAPI void physics_SetColliderPositionByIndex(int collider_index, vec2_t position, int b_set);

//PEWAPI void physics_SetColliderVelocityByIndex(int collider_index, vec2_t position, int b_set);

PEWAPI general_collider_t *physics_GetCollider(char *name);

PEWAPI general_collider_t *physics_GetColliderByIndex(int collider_index);


//PEWAPI struct entity_t *physics_2DRayCast(vec2_t from, vec2_t to, vec2_t *intersection);

//PEWAPI int physics_GetIntersectionPlaneLine(vec3_t a, vec3_t b, plane_t *plane, vec3_t *out);

//PEWAPI float physics_Signed2DTriangleArea(vec2_t a, vec2_t b, vec2_t c);

//PEWAPI int physics_Test2DSegmentSegment(vec2_t a, vec2_t b, vec2_t c, vec2_t d, vec2_t *intersection);

//PEWAPI int physics_PointInside2DHalfSpaces(vec2_t point, vec2_t a, vec2_t b, vec2_t c, vec2_t d, vec2_t e, vec2_t f, vec2_t g, vec2_t h);

#ifdef __cplusplus
}
#endif

#include "physics.inl"



#endif /* PHYSICS_H */









