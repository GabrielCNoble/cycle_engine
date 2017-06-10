#ifndef PARTICLE_H
#define PARTICLE_H

#include "conf.h"
#include "includes.h"
#include "draw.h"
#include "entity.h"
#include "scenegraph.h"
#include "sound.h"


enum PARTICLE_SYSTEM_TYPES
{
	SYSTEM_EXPLOSION,
	SYSTEM_SPARKS,
	SYSTEM_SMOKE
};


typedef struct particle_t
{
	vec2_t origin;
	vec2_t velocity;
	int texture_layer;
	int life;
	int b_exist;
}particle_t;

typedef struct particle_system
{
	//draw_info_t draw_info;
	mesh_t mesh;
	vec2_t local_position;
	vec2_t world_position;
	node_t *assigned_node;
	int material_index;
	int system_index;
	int max_particles;
	int max_particle_life;
	int particle_count;
	int system_life;
	int spawn_delay;					/* delay between one particle spawn and the next */
	int last_spawn;
	int stack_top;
	int layer;
	int type;
	int *free_positions_stack;
	float pos_dx;						/* horizontal randomness */
	float pos_dy;
	float vel_dx;
	float vel_dy; 
	particle_t *particles;
	void (*update_func)(struct particle_system *);			/* a update function for especific particle behaviour */
	abuffer_t sound;										/* the sound to be played when this particle effect is spawned */
	char *name;
}particle_system;


typedef struct particle_system_array
{
	int array_size;
	int system_count;
	particle_system *systems;
}particle_system_array;

typedef struct active_particle_system_array
{
	int array_size;
	int active_count;
	int stack_top;
	int *free_positions_stack;
	particle_system *systems;
}active_particle_system_array;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void particle_Init();

PEWAPI void particle_Finish();

PEWAPI void particle_ResizeParticleSystemArray(int new_size);

PEWAPI void particle_ResizeActiveParticleSystemArray(int new_size);

PEWAPI void particle_ProcessParticleSystems();

PEWAPI int particle_CreateParticleSystem(particle_system *system);

PEWAPI void particle_SpawnParticleSystem(particle_system *system, vec2_t position, int layer);

PEWAPI particle_system *particle_GetParticleSystemByIndex(int particle_system_index);

PEWAPI particle_system *particle_GetParticleSystem(char *name);

PEWAPI void particle_DestroyParticleSystem(particle_system *system);

PEWAPI void particle_SpawnParticle(particle_system *system);

PEWAPI void particle_RemoveParticle(particle_system *system, int index);

PEWAPI void particle_AttachParticleSystemToEntity(particle_system *system, entity_t *entity);

#ifdef __cplusplus
}
#endif

#endif /* PARTICLE_H */















