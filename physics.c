#include "physics.h"
#include "entity.h"
#include "camera.h"
#include "gmath/vector.h"
#include "pew.h"
#include "brush.h"

#include "BulletCollision\NarrowPhaseCollision\btPersistentManifold.h"
#include "BulletCollision\NarrowPhaseCollision\btRaycastCallback.h"
#include "BulletCollision\CollisionDispatch\btCollisionObject.h"
#include "BulletCollision\CollisionShapes\btTriangleMesh.h"
#include "BulletCollision\CollisionShapes\btBvhTriangleMeshShape.h"


#define GRAVITY -0.042
#define MAX_FALL_SPEED 2.0
#define JUMP_ACCELERATION 0.9
#define WALK_ACCELERATION 0.09
#define MAX_HORIZONTAL_SPEED 12.0
#define WALK_FRICTION_GROUND 0.87
#define Y_RETRACTION 0.00001

#define DEFAULT_MASS 1.0

#define COLLIDER_SIZE (sizeof(general_collider_t) > sizeof(character_controller_t) ? sizeof(general_collider_t) : sizeof(character_controller_t))


collider_array collider_a;
static int constraint_count = 0;
static constraint_t *root;
static constraint_t *last;
static int deletions = 0;
extern entity_array entity_a;
extern brush_list_t brush_list;

btDefaultCollisionConfiguration *collision_configuration;
btCollisionDispatcher *narrow_phase;
btBroadphaseInterface *broad_phase;
btSequentialImpulseConstraintSolver *constraint_solver;
btDiscreteDynamicsWorld *physics_world;

btCollisionObject *static_world = NULL;


#ifdef __cplusplus
extern "C"
{
#endif

void near_call_back(btBroadphasePair& collision_pair, btCollisionDispatcher& narrow_phase, btDispatcherInfo &dispatchInfo)
{
 	btRigidBody *rb1 = (btRigidBody *)collision_pair.m_pProxy0->m_clientObject;
 	btRigidBody *rb2 = (btRigidBody *)collision_pair.m_pProxy1->m_clientObject;
 	
 	if(rb1->getLinearVelocity().length() > 0.1)
	{
		
	}
	if(rb2->getLinearVelocity().length() > 0.1)
	{
		
	}
 	
	narrow_phase.defaultNearCallback(collision_pair, narrow_phase, dispatchInfo);
}

/*
=============
physics_Init
=============
*/
PEWAPI void physics_Init()
{
	collider_a.base = NULL;
	collider_a.colliders = NULL;
	collider_a.size = 0;
	collider_a.count = 0;
	physics_ResizeColliderArray(16);
	
	root = (constraint_t *) malloc(sizeof(constraint_t));
	root->next = NULL;
	last = root;
	
	collision_configuration = new btDefaultCollisionConfiguration();
	narrow_phase = new btCollisionDispatcher(collision_configuration);
	//narrow_phase->setNearCallback((btNearCallback)near_call_back);
	broad_phase = new btDbvtBroadphase();
	broad_phase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	constraint_solver = new btSequentialImpulseConstraintSolver();
	physics_world = new btDiscreteDynamicsWorld(narrow_phase, broad_phase, constraint_solver, collision_configuration);
	physics_world->setGravity(btVector3(0.0, -10.0, 0.0));
	
	return;
}


/*
=============
physics_Finish
=============
*/
PEWAPI void physics_Finish()
{
	free(collider_a.base);
	//delete physics_world;
	//free(collider_a.colliders);
}


/*
=============
physics_ResizeColliderArray
=============
*/
PEWAPI void physics_ResizeColliderArray(int new_size)
{
	general_collider_t *temp = (general_collider_t *)malloc(new_size * COLLIDER_SIZE + COLLIDER_SIZE);
	int *stack_temp = (int *)malloc(new_size * sizeof(int));
	if(collider_a.base)
	{
		memcpy(temp, collider_a.base, COLLIDER_SIZE * collider_a.size);
		free(collider_a.base);
		free(collider_a.free_positions_stack);
	}
	collider_a.base = temp;
	//collider_a.colliders = (collider_t *)(((int)collider_a.base + (sizeof(collider_t ) - 1)) & (~(sizeof(collider_t )-1)));
	collider_a.colliders = collider_a.base;
	collider_a.free_positions_stack = stack_temp;
	collider_a.stack_size = new_size;
	collider_a.stack_top = -1;
	collider_a.size = new_size;
	
	return;
}

void physics_DefragColliderArray()
{
	int i = collider_a.count - 1;
	int index;
	int last = -1;
	for(; i >= 0; i--)
	{
		if(!(collider_a.colliders[i].base.bm_flags & COLLIDER_IMMUTABLE_INDEX))
		{
			if(collider_a.stack_top < 0)
			{
				break;
			}
			index = collider_a.free_positions_stack[collider_a.stack_top--];
			memcpy(&collider_a.colliders[index], &collider_a.colliders[i], COLLIDER_SIZE);
		}
		else
		{
			if(last == -1)
			{
				last = i;	/* keep the position of the last immutable collider, so collider_a.count is properly updated... */
			}
		}
	}
	if(last != -1)
	{
		collider_a.count = last;
	}
}


void physics_UpdateStaticWorld()
{
	
	int i;
	int c = brush_list.count;
	int j;
	int k;
	
	btTriangleMesh *triangle_mesh;
	btBvhTriangleMeshShape *shape;
	if(static_world)
	{
		physics_world->removeCollisionObject(static_world);
		delete static_world;
	}
	
	triangle_mesh = new btTriangleMesh;
	
	
	
	for(i = 0; i < c; i++)
	{
		k = brush_list.draw_data[i].vert_count;
		
		for(j = 0; j < k;)
		{
			/*triangle_mesh->addTriangle(btVector3(brush_list.draw_data[i].verts[j * 6], 	     brush_list.draw_data[i].verts[j * 6 + 1], 		 brush_list.draw_data[i].verts[j * 6 + 2]),
			      					   btVector3(brush_list.draw_data[i].verts[(j + 1) * 6], brush_list.draw_data[i].verts[(j + 1) * 6 + 1], brush_list.draw_data[i].verts[(j + 1) * 6 + 2]),
									   btVector3(brush_list.draw_data[i].verts[(j + 2) * 6], brush_list.draw_data[i].verts[(j + 2) * 6 + 1], brush_list.draw_data[i].verts[(j + 2) * 6 + 2]));*/
			
			triangle_mesh->addTriangle(btVector3(brush_list.draw_data[i].vertices[j].position.x, brush_list.draw_data[i].vertices[j].position.y, brush_list.draw_data[i].vertices[j].position.z),
			      					   btVector3(brush_list.draw_data[i].vertices[j + 1].position.x, brush_list.draw_data[i].vertices[j + 1].position.y, brush_list.draw_data[i].vertices[j + 1].position.z),
									   btVector3(brush_list.draw_data[i].vertices[j + 2].position.x, brush_list.draw_data[i].vertices[j + 2].position.y, brush_list.draw_data[i].vertices[j + 2].position.z));						   
									   
			j += 3;						  
		}
		
	}
	
	shape = new btBvhTriangleMeshShape(triangle_mesh, true, true);
	
	
	static_world = new btCollisionObject();
	static_world->setCollisionShape(shape);
	
	physics_world->addCollisionObject(static_world);
	
	
	
}

/*
=============
physics_CreateCollider
=============
*/
PEWAPI int physics_CreateCollider(char *name, short type, short collision_shape, short bm_flags, int index, float ground_height, float step_height, float height, float width, float mass, float gravity, float jump_force, mesh_t *collision_shape_mesh, vec3_t position, mat3_t *orientation, void (*collision_callback)(int entity_a_index , int entity_b_index))
{
	int collider_index = collider_a.count;
	btConvexShape *col_shape;
	btConvexHullShape *convex_hull;
	btScalar vmass = mass;
	btVector3 inertia;
	btVector3 linear_factor;
	btVector3 angular_factor;
	btDefaultMotionState *motion_state;
	btTransform btransform;
	mat4_t transform;
	btCollisionObject *collision_object;
	btRigidBody *rigid_body;
	
	if(collider_index >= collider_a.size)
	{
		physics_ResizeColliderArray(collider_a.size << 1);
	}
	/* make sure we don't get a dynamic 'static' rigid body. */
	if(mass <= 0.0)
	{
		type = COLLIDER_STATIC;
	}
	
	collider_a.colliders[collider_index].base.type = type;
	collider_a.colliders[collider_index].base.bm_flags = bm_flags;
	collider_a.colliders[collider_index].base.collision_callback = collision_callback;
	collider_a.colliders[collider_index].base.index = index;
	collider_a.colliders[collider_index].base.collider_index = collider_index;
	collider_a.colliders[collider_index].base.mass = mass;
	collider_a.colliders[collider_index].base.name = strdup(name);
	
	mat4_t_compose(&transform, orientation, position);
	btransform.setFromOpenGLMatrix(&transform.floats[0][0]);
	motion_state = new btDefaultMotionState(btransform);

	
	if(type == COLLIDER_CHARACTER_CONTROLLER)
	{
		col_shape = new btCapsuleShape(width, height);
		col_shape->calculateLocalInertia(vmass, inertia);
		linear_factor = btVector3(1, 1, 1);
		angular_factor = btVector3(0, 0, 0);	
		collider_a.colliders[collider_index].base.rigid_body = new btRigidBody(vmass, motion_state, col_shape, inertia);
		collider_a.colliders[collider_index].base.bm_flags |= COLLIDER_IMMUTABLE_INDEX;	/* character controllers have to be immutable... */
		((character_controller_t *)&collider_a.colliders[collider_index])->step_height = step_height; 
		((character_controller_t *)&collider_a.colliders[collider_index])->height = height;
		((character_controller_t *)&collider_a.colliders[collider_index])->width = width;
		((character_controller_t *)&collider_a.colliders[collider_index])->jump_force = jump_force;
		//((character_controller_t *)&collider_a.colliders[collider_index])->ghost_object = new btPairCachingGhostObject();
		//((character_controller_t *)&collider_a.colliders[collider_index])->ghost_object->getWorldTransform().getBasis().setIdentity();
		//((character_controller_t *)&collider_a.colliders[collider_index])->ghost_object->getWorldTransform().setOrigin(btVector3(position.x, position.y, position.z));
	}
	else
	{
		convex_hull = new btConvexHullShape();
		*convex_hull = *(collision_shape_mesh->collision_shape);
		//linear_factor = btVector3(1, 1, 1);
		//angular_factor = btVector3(1, 1, 1);
		if(bm_flags & COLLIDER_LOCK_X_ROT)
		{
			angular_factor[0] = 0;
		}
		else
		{
			angular_factor[0] = 1;
		}
		if(bm_flags & COLLIDER_LOCK_Y_ROT)
		{
			angular_factor[1] = 0;
		}
		else
		{
			angular_factor[1] = 1;
		}
		if(bm_flags & COLLIDER_LOCK_Z_ROT)
		{
			angular_factor[2] = 0;
		}
		else
		{
			angular_factor[2] = 1;
		}
		
		if(bm_flags & COLLIDER_LOCK_X_LOC)
		{
			linear_factor[0] = 0;
		}
		else
		{
			linear_factor[0] = 1;
		}
		if(bm_flags & COLLIDER_LOCK_Y_LOC)
		{
			linear_factor[1] = 0;
		}
		else
		{
			linear_factor[1] = 1;
		}
		if(bm_flags & COLLIDER_LOCK_Z_LOC)
		{
			linear_factor[2] = 0;
		}
		else
		{
			linear_factor[2] = 1;
		}
		
		convex_hull->calculateLocalInertia(vmass, inertia);
		collider_a.colliders[collider_index].base.rigid_body = new btRigidBody(vmass, motion_state, convex_hull, inertia);
	}
	
	if(bm_flags & COLLIDER_CREATE_SCENEGRAPH_NODE)
	{
		collider_a.colliders[collider_index].base.assigned_node = scenegraph_AddNode(NODE_COLLIDER, collider_index, -1, "_collider_");
	}
	else
	{
		collider_a.colliders[collider_index].base.assigned_node = NULL;
	}
	
	
	//collision_shape->calculateLocalInertia(vmass, inertia);
	//collider_a.colliders[collider_index].rigid_body = new btRigidBody(vmass, motion_state, collision_shape, inertia);
	rigid_body = collider_a.colliders[collider_index].base.rigid_body;
	physics_world->addRigidBody(rigid_body);
	rigid_body->setGravity(btVector3(0.0, -gravity, 0.0));
	rigid_body->setLinearFactor(linear_factor);
	rigid_body->setAngularFactor(angular_factor);
	rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
	rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
	rigid_body->setUserIndex(index);
	rigid_body->getCollisionShape()->setUserIndex(collider_index);
	collider_a.count++;
	return collider_index;
}

PEWAPI int physics_CreateCharacterController(short bm_flags, int index, float height, float width, float step_height, float gravity, float jump_force, vec3_t position, void (*collision_callback)(int entity_a_index , int entity_b_index))
{

	int collider_index = collider_a.count;
	btVector3 inertia;
	character_controller_t *controller = ((character_controller_t *)&collider_a.colliders[collider_index]);
	btCapsuleShape *collision_shape = new btCapsuleShape(width, height);
	collision_shape->calculateLocalInertia(1.0, inertia);
	btVector3 linear_factor = btVector3(1, 1, 1);
	btVector3 angular_factor = btVector3(0, 0, 0);	
	
	mat4_t transform;
	mat3_t orientation = mat3_t_id();
	btTransform btransform;
	
	mat4_t_compose(&transform, &orientation, position);
	btransform.setFromOpenGLMatrix(&transform.floats[0][0]);
	btDefaultMotionState *motion_state = new btDefaultMotionState(btransform);
	
	controller->base.rigid_body = new btRigidBody(1.0, motion_state, collision_shape, inertia);
	controller->step_height = step_height; 
	controller->height = height;
	controller->width = width;
	controller->jump_force = jump_force;
	
	controller->base.type = COLLIDER_CHARACTER_CONTROLLER;
	controller->base.bm_flags = bm_flags | COLLIDER_IMMUTABLE_INDEX;
	controller->base.collision_callback = collision_callback;

	controller->base.index = index;
	controller->base.collider_index = collider_index;
	
	physics_world->addRigidBody(controller->base.rigid_body);
	controller->base.rigid_body->setGravity(btVector3(0.0, -gravity, 0.0));
	controller->base.rigid_body->setLinearFactor(linear_factor);
	controller->base.rigid_body->setAngularFactor(angular_factor);
	controller->base.rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
	controller->base.rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
	controller->base.assigned_node = scenegraph_AddNode(NODE_COLLIDER, collider_index, -1, "_collider_");
	
	collider_a.count++;
	return collider_index;
}


PEWAPI void physics_DestroyCollider(general_collider_t *collider)
{
	if(collider)
	{
		physics_world->removeRigidBody(collider->base.rigid_body);
		delete(collider->base.rigid_body);
		if(collider->base.type == COLLIDER_CHARACTER_CONTROLLER)
		{
			
		}
		collider_a.free_positions_stack[++collider_a.stack_top] = collider->base.collider_index;
		collider->base.collider_index = -1;
	}
}

PEWAPI void physics_DestroyColliderByIndex(int collider_index)
{
	general_collider_t *collider;
	if(collider_index >= 0 && collider_index < collider_a.count)
	{
		collider = &collider_a.colliders[collider_index];
		
		physics_world->removeRigidBody(collider->base.rigid_body);
		delete(collider->base.rigid_body);
		
		if(collider->base.type == COLLIDER_CHARACTER_CONTROLLER)
		{
			
		}
		collider_a.free_positions_stack[++collider_a.stack_top] = collider_index;
		collider->base.collider_index = -1;
	}
}

PEWAPI void physics_CreateConstraint(int a_collider, int b_collider, short type, vec3_t a_pivot, vec3_t b_pivot, vec3_t a_axis, vec3_t b_axis, float upper_limit, float lower_limit)
{
	constraint_t *temp;

	btRigidBody *a = collider_a.colliders[a_collider].base.rigid_body;
	btRigidBody *b = collider_a.colliders[b_collider].base.rigid_body;

	if(a && b)
	{
		temp = (constraint_t *) malloc(sizeof(constraint_t));
		temp->collider_a = a_collider;
		temp->collider_b = b_collider;
		temp->flags = 0;
		
		collider_a.colliders[a_collider].constraint = temp;
		collider_a.colliders[b_collider].constraint = temp;
		a->setDamping(a->getLinearDamping(), 0.9);
		b->setDamping(b->getLinearDamping(), 0.9);
		a->getCollisionShape()->setMargin(0.005);
		b->getCollisionShape()->setMargin(0.005);
		switch(type)
		{
			case CONSTRAINT_HINGE:
				temp->type = CONSTRAINT_HINGE;
				temp->constraint = new btHingeConstraint(*a, *b, btVector3(a_pivot.floats[0], a_pivot.floats[1], a_pivot.floats[2]), 
															 	 btVector3(b_pivot.floats[0], b_pivot.floats[1], b_pivot.floats[2]),
															 	 btVector3(a_axis.floats[0], a_axis.floats[1], a_axis.floats[2]),
															 	 btVector3(b_axis.floats[0], b_axis.floats[1], b_axis.floats[2]), false);
															 
				((btHingeConstraint *)temp->constraint)->setLimit(lower_limit * 3.14159265, upper_limit * 3.14159265, 0.0, 0.5, 2.0);		
				temp->lower_limit = lower_limit;
				temp->upper_limit = upper_limit;									 									 
			break;
			
			/* TODO: the rest of them... :) */
			
			default:
				free(temp);
			break;
		}
		
		physics_world->addConstraint(temp->constraint);
		temp->next = NULL;
		last->next = temp;
		last = temp;
	}
	
	
}


PEWAPI void physics_DeleteConstraint(constraint_t *constraint)
{
	
}

PEWAPI void physics_LockHinge(constraint_t *constraint)
{
	float angle;
	if(constraint)
	{
		if(constraint->type == CONSTRAINT_HINGE)
		{
			constraint->flags |= HINGE_LOCKED;
			angle = ((btHingeConstraint *)constraint->constraint)->getHingeAngle();
			((btHingeConstraint *)constraint->constraint)->setLimit((angle - 0.01) * 3.14159265, (angle + 0.01) * 3.14159265, 0.0, 0.5, 2.0);
		}
	}
}

PEWAPI void physics_UnlockHinge(constraint_t *constraint)
{
	if(constraint)
	{
		if(constraint->type == CONSTRAINT_HINGE)
		{
			constraint->flags &= ~HINGE_LOCKED;
			((btHingeConstraint *)constraint->constraint)->setLimit(constraint->lower_limit * 3.14159265, constraint->upper_limit * 3.14159265, 0.0, 0.5, 2.0);
		}
	}
}

PEWAPI void physics_SetHingeLimits(constraint_t *constraint, float lower_limit, float upper_limit)
{
	if(constraint)
	{
		if(constraint->type == CONSTRAINT_HINGE)
		{
			((btHingeConstraint *)constraint->constraint)->setLimit(lower_limit * 3.14159265, upper_limit * 3.14159265, 0.0, 0.5, 2.0);
			constraint->upper_limit = upper_limit;
			constraint->lower_limit = lower_limit;
		}
	}
}


PEWAPI float physics_GetHingeAngle(constraint_t *constraint)
{
	if(constraint)
	{
		if(constraint->type == CONSTRAINT_HINGE)
		{
			return ((btHingeConstraint *)constraint->constraint)->getHingeAngle();
		}
	}
}

PEWAPI general_collider_t *physics_RayCast(vec3_t from, vec3_t to)
{
	btVector3 s = btVector3(from.x, from.y, from.z);
	btVector3 e = btVector3(to.x, to.y, to.z);
	btCollisionWorld :: ClosestRayResultCallback hit(s, e);
	hit.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
	physics_world->rayTest(s, e, hit);
	general_collider_t *c;
	
	if(hit.hasHit())
	{
		c = &collider_a.colliders[hit.m_collisionObject->getCollisionShape()->getUserIndex()];
		return c;
	}
	return NULL;
}


/*
=============
physics_ProcessCollisions
=============
*/

PEWAPI void physics_ProcessCollisions(float delta_time)
{
	int i;
	int c;
	int j;

	int contact_count;
	btVector3 cp;
	
	btCollisionObject *o1;
	btCollisionObject *o2;
	btPersistentManifold *contact_manifold;
	btManifoldPoint p;
	general_collider_t *collider;
	float frequency = 1000.0/delta_time;
	//physics_world->stepSimulation(1.0/frequency, 5, 1.0 / 60.0);

	c = collider_a.count;
	
	for(i = 0; i < c; i++)
	{
		if(collider_a.colliders[i].base.type == COLLIDER_CHARACTER_CONTROLLER)
		{
			physics_ProcessColliderMovement((character_controller_t *)&collider_a.colliders[i]);
		}
	}
	
	c = narrow_phase->getNumManifolds();
	
	for(i = 0; i < c; i++)
	{
		contact_manifold = narrow_phase->getManifoldByIndexInternal(i);
		contact_count = contact_manifold->getNumContacts();
		o1 = (btCollisionObject *)contact_manifold->getBody0();
		o2 = (btCollisionObject *)contact_manifold->getBody1();
		
		for(j = 0; j < contact_count; j++)
		{
			p = contact_manifold->getContactPoint(j);
			
			if(p.getLifeTime() == 1)
			{
				
			}
		}
	}
	
	if(deletions > 10)
	{
		physics_DefragColliderArray();
		deletions = 0;
	}
	
	physics_world->stepSimulation(1.0/frequency, 5);
	
}


void physics_ProcessColliderMovement(character_controller_t *collider)
{
	btVector3 collider_center;
	btVector3 ray_origin;
	btVector3 ray_end;
	btVector3 hit;
	btVector3 movement_velocity;
	btVector3 movement_direction;
	btVector3 angular_velocity;
	btCapsuleShape *capsule = (btCapsuleShape *)collider->base.rigid_body->getCollisionShape();
	float k = 125.0;
	float damping = 18.0;
	float force;
	const btCollisionShape *collisioned_with;
	//float ray_length = 1.5;
	float deacceleration = 1.0;
	btVector3 ray_direction;// = btVector3(0.0, -collider->ground_height, 0.0);

	ray_direction = btVector3(0.0, -1.0, 0.0);
	collider_center = collider->base.rigid_body->getCenterOfMassPosition();
	ray_origin = collider_center - btVector3(0.0, capsule->getHalfHeight() - capsule->getMargin(), 0.0);
	ray_end = ray_origin + ray_direction;
	
	btCollisionWorld :: ClosestConvexResultCallback closest_convex_hit(ray_origin, ray_end);
	
	btTransform from = collider->base.rigid_body->getWorldTransform();
	btTransform to = from;
	to.setOrigin(to.getOrigin() + btVector3(0.0, -capsule->getHalfHeight(), 0.0));
	physics_world->convexSweepTest(capsule, from, to, closest_convex_hit, 0.0);
	movement_velocity = collider->base.rigid_body->getLinearVelocity();
	//angular_velocity = collider->base.rigid_body->getAngularVelocity();
	//angular_velocity[1] = 0.0;
	//collider->base.rigid_body->setAngularVelocity(angular_velocity);
	
	if(collider->base.bm_flags & COLLIDER_JUST_LANDED)
	{
		collider->base.bm_flags &= ~COLLIDER_JUST_LANDED;
	}
		
	if(closest_convex_hit.hasHit())
	{
		if(!(collider->base.bm_flags & COLLIDER_ASCENDING))
		{
			
			btVector3 contact = ray_origin.lerp(ray_end, closest_convex_hit.m_closestHitFraction);
			float f = closest_convex_hit.m_hitNormalWorld.dot(btVector3(0.0, 1.0, 0.0));
			
			//printf("%f  %f  %f\n", fabs(contact[1] - collider->rigid_body->getCenterOfMassPosition()[1]), fabs(closest_convex_hit.m_hitPointWorld[1] - collider->rigid_body->getCenterOfMassPosition()[1]) - capsule->getHalfHeight(), f);
			
			/*if(fabs(closest_convex_hit.m_hitPointWorld[1] - collider->rigid_body->getCenterOfMassPosition()[1]) - capsule->getHalfHeight() < 0.1)
			{
				printf("hit step!\n");
			}
			else printf("nope\n");*/
			
			//printf("%f\n", f);
			
			if(fabs(contact[1] - collider->base.rigid_body->getCenterOfMassPosition()[1]) <= capsule->getHalfHeight() && f > 0.80)
			{
				if(collider->base.bm_flags & COLLIDER_FLYING)
				{
					collider->base.bm_flags |= COLLIDER_JUST_LANDED;
				}
				collider->base.bm_flags &= ~(COLLIDER_FLYING | COLLIDER_JUMPED);
			}
			else
			{
				collider->base.bm_flags |= COLLIDER_FLYING;
			}
	
		}
		
	}
	else
	{
		collider->base.bm_flags |= COLLIDER_FLYING;
	}
	
	if(movement_velocity[1] <= 0.0) 
	{
		collider->base.bm_flags &= ~COLLIDER_ASCENDING;
	}

	
	if(collider->base.bm_flags & COLLIDER_JUST_JUMPED)
	{
		collider->base.bm_flags &= ~COLLIDER_JUST_JUMPED;
	}

	
	movement_velocity[1] = 0.0;
	if(!(collider->base.bm_flags & COLLIDER_FLYING))
	{
		
		
		if(movement_velocity.dot(movement_velocity) < 0.1)
		{
			movement_velocity[0] = 0.0;
			movement_velocity[2] = 0.0;

			collider->base.rigid_body->setLinearVelocity(movement_velocity);
			collider->base.rigid_body->applyCentralForce(-collider->base.rigid_body->getGravity());
		}
		else
		{	
			collider->base.rigid_body->applyCentralForce(-movement_velocity * 10.0 * collider->base.mass);
		}
		
	}
	else
	{	
		collider->base.rigid_body->applyCentralForce(-movement_velocity);
	}
	

	collider->base.bm_flags &= ~(COLLIDER_MOVING | COLLIDER_CROUCHED);
}

PEWAPI void physics_Jump(character_controller_t *collider)
{
	btVector3 velocity;
	if(collider)
	{
		if(!(collider->base.bm_flags & COLLIDER_FLYING))
		{	
			collider->base.rigid_body->applyCentralImpulse(btVector3(0.0, collider->jump_force, 0.0));
			collider->base.bm_flags |= COLLIDER_JUMPED | COLLIDER_JUST_JUMPED | COLLIDER_FLYING | COLLIDER_ASCENDING;
		}
	}
}


PEWAPI void physics_Move(character_controller_t *collider, vec3_t direction)
{
	float velocity;
	float intensity = 1.0;
	if(collider)
	{
		if(dot3(direction, direction) != 0.0)
		{
			//if(collider->bm_flags & COLLIDER_FLYING) intensity = 0.08;
			collider->base.rigid_body->activate(true);
			//velocity = collider->rigid_body->getLinearVelocity();
			velocity = collider->base.rigid_body->m_linearVelocity.dot(collider->base.rigid_body->m_linearVelocity);
			
			if(collider->base.bm_flags & COLLIDER_FLYING)
			{
				direction.x *= 0.2;
				direction.z *= 0.2;
				if(velocity > MAX_HORIZONTAL_SPEED * MAX_HORIZONTAL_SPEED)
				{
					direction.x = -collider->base.rigid_body->m_linearVelocity[0] * 0.1;
					direction.z = -collider->base.rigid_body->m_linearVelocity[2] * 0.1;
				}

			}

			
			collider->base.rigid_body->applyCentralImpulse(btVector3(direction.floats[0] * intensity * collider->base.mass, 0.0, direction.floats[2]  * intensity * collider->base.mass));
			collider->base.bm_flags |= COLLIDER_MOVING;
		}
		
	}
}


PEWAPI void physics_Crouch(character_controller_t *collider)
{
	btCapsuleShape *capsule;
	if(collider)
	{
		//capsule = (btCapsuleShape *)collider->rigid_body->getCollisionShape();
		collider->base.bm_flags |= COLLIDER_CROUCHED;
		//capsule->setLocalScaling(btVector3(1, 0.1, 1));
	}
}

PEWAPI void physics_Yaw(character_controller_t *controller, float yaw)
{
	btTransform btransform;
	btMatrix3x3 brotation;
	mat3_t p;
	mat3_t y;
	mat3_t rotation;
	mat4_t transform;
	if(controller)
	{
		
		//mat3_t_rotate(&y, vec3(0.0, 1.0, 0.0), yaw, 1);
		//mat3_t_rotate(&p, vec3(1.0, 0.0, 0.0), pitch, 1);
		//mat3_t_mult(&rotation, &p, &y);
		mat3_t_rotate(&rotation, vec3(0.0, 1.0, 0.0), yaw, 1);
		btransform = controller->base.rigid_body->getCenterOfMassTransform();
		//brotation = btransform.getBasis();
		brotation[0][0] = rotation.floats[0][0]; 
		brotation[0][1] = rotation.floats[0][1];
		brotation[0][2] = rotation.floats[0][2];
		
		brotation[1][0] = rotation.floats[1][0];
		brotation[1][1] = rotation.floats[1][1];
		brotation[1][2] = rotation.floats[1][2];
		
		brotation[2][0] = rotation.floats[2][0];
		brotation[2][1] = rotation.floats[2][1];
		brotation[2][2] = rotation.floats[2][2];
		
		btransform.setBasis(brotation);
		//btransform.setFromOpenGLSubMatrix(&rotation.floats[0][0]);
		controller->base.rigid_body->setCenterOfMassTransform(btransform);
		
		controller->base.rigid_body->activate(true);
	}
}

/*
=============
physics_SetColliderPosition
=============
*/
PEWAPI void physics_SetColliderPosition(general_collider_t *collider, vec2_t position, int b_set)
{
	/*if(collider)
	{
		if(!b_set)
		{
			collider->position.floats[0]+=position.floats[0];
			collider->position.floats[1]+=position.floats[1];
		}
		else
		{
			collider->position.floats[0]=position.floats[0];
			collider->position.floats[1]=position.floats[1];
		}
	}
	
	return;*/
}


/*
=============
physics_SetColliderVelocity
=============
*/
PEWAPI void physics_SetColliderVelocity(general_collider_t *collider, vec2_t velocity, int b_set)
{
	/*if(collider)
	{
		if(!b_set)
		{
			collider->velocity.floats[0]+=velocity.floats[0];
			collider->velocity.floats[1]+=velocity.floats[1];
		}
		else
		{
			collider->velocity.floats[0]=velocity.floats[0];
			collider->velocity.floats[1]=velocity.floats[1];
		}
	}
	
	return;*/
}


/*
=============
physics_SetColliderPositionByIndex
=============
*/
PEWAPI void physics_SetColliderPositionByIndex(int collider_index, vec2_t position, int b_set)
{
	/*if(collider_index>=0)
	{
		if(!b_set)
		{
			collider_a.colliders[collider_index].position.floats[0]+=position.floats[0];
			collider_a.colliders[collider_index].position.floats[1]+=position.floats[1];
		}
		else
		{
			collider_a.colliders[collider_index].position.floats[0]=position.floats[0];
			collider_a.colliders[collider_index].position.floats[1]=position.floats[1];
		}
	}
	
	return;*/
}


/*
=============
physics_SetColliderVelocityByIndex
=============
*/
PEWAPI void physics_SetColliderVelocityByIndex(int collider_index, vec2_t velocity, int b_set)
{
	/*if(collider_index>=0)
	{
		if(!b_set)
		{
			collider_a.colliders[collider_index].velocity.floats[0]+=velocity.floats[0];
			collider_a.colliders[collider_index].velocity.floats[1]+=velocity.floats[1];
		}
		else
		{
			collider_a.colliders[collider_index].velocity.floats[0]=velocity.floats[0];
			collider_a.colliders[collider_index].velocity.floats[1]=velocity.floats[1];
		}
	}
	
	return;*/
}

PEWAPI general_collider_t *physics_GetCollider(char *name)
{
	int i;
	int c = collider_a.count;
	
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, collider_a.colliders[i].base.name))
		{
			return &collider_a.colliders[i];
		}
	}
	return NULL;
}


PEWAPI general_collider_t *physics_GetColliderByIndex(int collider_index)
{
	if(collider_index >= 0 && collider_index < collider_a.count)
	{
		return &collider_a.colliders[collider_index];
	}
	return NULL;
}


/*
=============
physics_2DRayCast
=============
*/
PEWAPI struct entity_t *physics_2DRayCast(vec2_t from, vec2_t to, vec2_t *intersection)
{
	/*register int i;
	register int c;
	register int index=-1;
	c=collider_a.collider_count;
	vec2_t lines[4][2];
	vec2_t t_center;
	float t_max_x;
	float t_min_x;
	float t_max_y;
	float t_min_y;
	float t_dx;
	float t_dy;
	vec2_t v;
	vec2_t intersec;
	float c_sqrd_len=99999999999999.0;
	float sqrd_len=99999999999999.0;
	vec2_t p;
	
	for(i=0; i<c; i++)
	{
		if(!(collider_a.colliders[i].bm_flags&COLLIDER_PROJECTILE) && collider_a.colliders[i].collider_index>=0)
		{
			t_center=collider_a.colliders[i].position;
			t_max_x=entity_a.entities[collider_a.colliders[i].assigned_entity_index].c_maxmins[0];
			t_min_x=entity_a.entities[collider_a.colliders[i].assigned_entity_index].c_maxmins[2];
			t_max_y=entity_a.entities[collider_a.colliders[i].assigned_entity_index].c_maxmins[1];
			t_min_y=entity_a.entities[collider_a.colliders[i].assigned_entity_index].c_maxmins[3];
		
			lines[0][0].floats[0]=t_center.floats[0]+t_max_x;
			lines[0][0].floats[1]=t_center.floats[1]+t_max_y-Y_RETRACTION;
			
			lines[0][1].floats[0]=t_center.floats[0]+t_max_x;
			lines[0][1].floats[1]=t_center.floats[1]+t_min_y+Y_RETRACTION;
		
			lines[1][0].floats[0]=t_center.floats[0]+t_max_x;
			lines[1][0].floats[1]=t_center.floats[1]+t_min_y+Y_RETRACTION;
		
			lines[1][1].floats[0]=t_center.floats[0]+t_min_x;
			lines[1][1].floats[1]=t_center.floats[1]+t_min_y+Y_RETRACTION;
		
			lines[2][0].floats[0]=t_center.floats[0]+t_min_x;
			lines[2][0].floats[1]=t_center.floats[1]+t_min_y+Y_RETRACTION;
		
			lines[2][1].floats[0]=t_center.floats[0]+t_min_x;
			lines[2][1].floats[1]=t_center.floats[1]+t_max_y-Y_RETRACTION;
		
			lines[3][0].floats[0]=t_center.floats[0]+t_min_x;
			lines[3][0].floats[1]=t_center.floats[1]+t_max_y-Y_RETRACTION;
		
			lines[3][1].floats[0]=t_center.floats[0]+t_max_x;
			lines[3][1].floats[1]=t_center.floats[1]+t_max_y-Y_RETRACTION;
		
			p=from;
		
		
			if(from.floats[0]<lines[0][0].floats[0] && from.floats[0]>lines[1][1].floats[0])
			{
				if(from.floats[1]<t_center.floats[1]+t_max_y && from.floats[1]>t_center.floats[1]+t_min_y)
				{

					p=to;
					if(p.floats[0]>t_center.floats[0]+t_max_x) p.floats[0]=t_center.floats[0]+t_max_x;
					if(p.floats[0]<t_center.floats[0]+t_min_x) p.floats[0]=t_center.floats[0]+t_min_x;
					
					if(p.floats[1]>t_center.floats[1]+t_max_y) p.floats[1]=t_center.floats[1]+t_max_y;
					if(p.floats[1]<t_center.floats[1]+t_min_y) p.floats[1]=t_center.floats[1]+t_min_y;
				}
			}
		
			if(physics_Test2DSegmentSegment(p, to, lines[0][0], lines[0][1], &intersec))
			{
				v=vec2(intersec.floats[0]-p.floats[0], intersec.floats[1]-p.floats[1]);
				c_sqrd_len=dot2(v, v);
				if(c_sqrd_len<sqrd_len)
				{
					sqrd_len=c_sqrd_len;
					index=i;
					*intersection=intersec;
				}
			}
			if(physics_Test2DSegmentSegment(p, to, lines[1][0], lines[1][1], &intersec))
			{
				v=vec2(intersec.floats[0]-p.floats[0], intersec.floats[1]-p.floats[1]);
				c_sqrd_len=dot2(v, v);
				if(c_sqrd_len<sqrd_len)
				{
					sqrd_len=c_sqrd_len;
					index=i;
					*intersection=intersec;
				}
			}
			if(physics_Test2DSegmentSegment(p, to, lines[2][0], lines[2][1], &intersec))
			{
				v=vec2(intersec.floats[0]-p.floats[0], intersec.floats[1]-p.floats[1]);
				c_sqrd_len=dot2(v, v);
				if(c_sqrd_len<sqrd_len)
				{
					sqrd_len=c_sqrd_len;
					index=i;
					*intersection=intersec;
				}
			}
			if(physics_Test2DSegmentSegment(p, to, lines[3][0], lines[3][1], &intersec))
			{
				v=vec2(intersec.floats[0]-p.floats[0], intersec.floats[1]-p.floats[1]);
				c_sqrd_len=dot2(v, v);
				if(c_sqrd_len<sqrd_len)
				{
					sqrd_len=c_sqrd_len;
					index=i;
					*intersection=intersec;
				}
			}
		}
		
	}
	if(index>=0)
	{
		return &entity_a.entities[collider_a.colliders[index].assigned_entity_index];
	}
	return NULL;*/
}


/*
=============
physics_GetIntersectionPlaneLine
=============
*/
PEWAPI int physics_GetIntersectionPlaneLine(vec3_t a, vec3_t b, plane_t *plane, vec3_t *out)
{
	vec3_t dir=sub3(b, a);
	float t = (plane->d - dot3(plane->normal, a))/dot3(plane->normal, dir);
	
	if(t>=0.0  && t<=1.0)
	{
		*out=add3(a, mul3(dir, t));
		return 1; 
	}
	return 0;
}


/*
=============
physics_Signed2DTriangleArea
=============
*/
PEWAPI float physics_Signed2DTriangleArea(vec2_t a, vec2_t b, vec2_t c)
{
	return (a.floats[0] - c.floats[0]) * (b.floats[1]-c.floats[1]) - (a.floats[1] - c.floats[1]) * (b.floats[0] - c.floats[0]);
}


/*
=============
physics_Test2DSegmentSegment
=============
*/
PEWAPI int physics_Test2DSegmentSegment(vec2_t a, vec2_t b, vec2_t c, vec2_t d, vec2_t *intersection)
{
	float a1;
	float a2;
	float a3;
	float a4;
	float t;
	
	a1=physics_Signed2DTriangleArea(a, b, d);
	a2=physics_Signed2DTriangleArea(a, b, c);
	
	if(a1*a2<0.0)
	{
		a3=physics_Signed2DTriangleArea(c, d, a);
		a4=a3+a2-a1;
		if(a3*a4<0.0)
		{
			t=a3/(a3-a4);
			if(t>=0.0  && t<=1.0)
			{
				intersection->floats[0]=a.floats[0] + t*(b.floats[0]-a.floats[0]);
				intersection->floats[1]=a.floats[1] + t*(b.floats[1]-a.floats[1]);
				return 1;
			}
		}
	}
	return 0;
}

PEWAPI int physics_PointInside2DHalfSpaces(vec2_t point, vec2_t a, vec2_t b, vec2_t c, vec2_t d, vec2_t e, vec2_t f, vec2_t g, vec2_t h)
{
	vec3_t ab;
	vec3_t bp;
	
	vec3_t cd;
	vec3_t dp;
	
	vec3_t ef;
	vec3_t fp;
	
	vec3_t gh;
	vec3_t hp;
	
	ab.floats[0]=b.floats[0]-a.floats[0];
	ab.floats[1]=b.floats[1]-a.floats[1];
	ab.floats[2]=0.0;
	
	bp.floats[0]=point.floats[0]-b.floats[0];
	bp.floats[1]=point.floats[1]-b.floats[1];
	bp.floats[2]=0.0;
	
	cd.floats[0]=d.floats[0]-c.floats[0];
	cd.floats[1]=d.floats[1]-c.floats[1];
	cd.floats[2]=0.0;
	
	dp.floats[0]=point.floats[0]-d.floats[0];
	dp.floats[1]=point.floats[1]-d.floats[1];
	dp.floats[2]=0.0;
	
	
	
	ef.floats[0]=f.floats[0]-e.floats[0];
	ef.floats[1]=f.floats[1]-e.floats[1];
	ef.floats[2]=0.0;
	
	fp.floats[0]=point.floats[0]-f.floats[0];
	fp.floats[1]=point.floats[1]-f.floats[1];
	fp.floats[2]=0.0;
	
	gh.floats[0]=h.floats[0]-g.floats[0];
	gh.floats[1]=h.floats[1]-g.floats[1];
	gh.floats[2]=0.0;
	
	hp.floats[0]=point.floats[0]-h.floats[0];
	hp.floats[1]=point.floats[1]-h.floats[1];
	hp.floats[2]=0.0;
	
	if(dot3(cross(bp, ab), cross(dp, cd)) * dot3(cross(fp, ef), cross(hp, gh))>0.0) return 1;
	
	return 0;
	
}

#ifdef __cplusplus
}
#endif























