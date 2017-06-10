#include "particle.h"
#include "scenegraph.h"
#include "gmath/vector.h"
#include "particle_behaviour.h"

extern particle_system_array particle_system_a;
extern active_particle_system_array active_particle_system_a;

#define PARTICLE_GRAVITY -0.008


#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void particle_Init()
{
	/*particle_system_a.system_count=0;
	particle_system_a.systems=NULL;
	
	active_particle_system_a.systems=NULL;
	active_particle_system_a.active_count=0;
	active_particle_system_a.stack_top=-1;
	active_particle_system_a.free_positions_stack=NULL;
	
	particle_ResizeParticleSystemArray(16);
	particle_ResizeActiveParticleSystemArray(16);
	
	return;*/
}

PEWAPI void particle_Finish()
{
	/*free(particle_system_a.systems);
	free(active_particle_system_a.systems);
	free(active_particle_system_a.free_positions_stack);
	return;*/
}

PEWAPI void particle_ResizeParticleSystemArray(int new_size)
{
	/*particle_system *temp=(particle_system *)calloc(new_size, sizeof(particle_system));
	if(particle_system_a.systems)
	{
		memcpy(temp, particle_system_a.systems, sizeof(particle_system)*particle_system_a.system_count);
		free(particle_system_a.systems);
	}
	particle_system_a.systems=temp;
	particle_system_a.array_size=new_size;
	return;*/
}

PEWAPI void particle_ResizeActiveParticleSystemArray(int new_size)
{
	/*particle_system *temp=(particle_system *)calloc(new_size, sizeof(particle_system));
	int *stack_temp=(int *)calloc(new_size, sizeof(particle_system));
	if(active_particle_system_a.systems)
	{
		memcpy(temp, active_particle_system_a.systems, sizeof(particle_system)*active_particle_system_a.active_count);
		free(active_particle_system_a.systems);
		free(active_particle_system_a.free_positions_stack);
	}
	active_particle_system_a.systems=temp;
	active_particle_system_a.free_positions_stack=stack_temp;
	active_particle_system_a.array_size=new_size;
	return;*/
}

PEWAPI void particle_ProcessParticleSystems()
{
	/*int i;
	int c;
	register int j;
	register int k;
	particle_system *system;
	c=active_particle_system_a.active_count;
	
	for(i=0; i<c; i++)
	{
		system=&active_particle_system_a.systems[i];
		
		if(system->system_life==-2) continue;
		
		system->update_func(system);

		if(system->system_life>0) system->system_life--;
		if(system->system_life==0)
		{
			particle_DestroyParticleSystem(system);
		}
	}
	return;*/
}


PEWAPI int particle_CreateParticleSystem(particle_system *system)
{
	/*if(system)
	{
		if(particle_system_a.system_count>=particle_system_a.array_size)
		{
			particle_ResizeParticleSystemArray(particle_system_a.array_size<<2);
		}
		if(!system->update_func)
		{
			switch(system->type)
			{
				case SYSTEM_SMOKE:
					system->update_func=smoke_default;
				break;
			}
		}
		particle_system_a.systems[particle_system_a.system_count++]=*system;
		return particle_system_a.system_count-1;
	}
	return -1;*/
}

PEWAPI void particle_SpawnParticleSystem(particle_system *system, vec2_t position, int layer)
{
	/*node_t node;
	particle_system s;
	int system_index;
	if(system)
	{
		s.local_position=position;
		s.world_position=position;
		s.free_positions_stack=(int *)calloc(system->max_particles, sizeof(int));
		s.stack_top=-1;
		s.last_spawn=system->spawn_delay;
		s.system_life =system->system_life;
		s.particle_count=0;
		s.particles=(particle_t *)calloc(system->max_particles, sizeof(particle_t));
		s.max_particles=system->max_particles;
		s.max_particle_life=system->max_particle_life;
		s.material_index=system->material_index;
		s.mesh=system->mesh;
		s.spawn_delay=system->spawn_delay;
		s.sound=system->sound;
		s.pos_dx=system->pos_dx;
		s.pos_dy=system->pos_dy;
		s.vel_dx=system->vel_dx;
		s.vel_dy=system->vel_dy;
		s.type=system->type;
		s.layer=layer;
		s.update_func=system->update_func;
		
		if(active_particle_system_a.stack_top>=0)
		{
			system_index=active_particle_system_a.free_positions_stack[active_particle_system_a.stack_top];
			active_particle_system_a.stack_top--;
		}
		else
		{
			if(active_particle_system_a.active_count>=active_particle_system_a.array_size)
			{
				particle_ResizeActiveParticleSystemArray(active_particle_system_a.array_size<<1);
			}
			system_index=active_particle_system_a.active_count;
			active_particle_system_a.active_count++;
		}
		
		
		s.assigned_node=scenegraph_AddNode(NODE_PARTICLE, system_index, "wow");
		s.system_index=system_index;
		active_particle_system_a.systems[system_index]=s;
		particle_SpawnParticle(&active_particle_system_a.systems[system_index]);
		
		if(s.sound.buffer_ID>0) sound_PlayAudioBufferDirect(&s.sound, vec2(0.0, 0.0), 0);

	}
	return;*/
}


PEWAPI particle_system *particle_GetParticleSystemByIndex(int particle_system_index)
{
	/*if(particle_system_index>=0)
	{
		return &particle_system_a.systems[particle_system_index];
	}*/
}

PEWAPI particle_system *particle_GetParticleSystem(char *name)
{
	/*register int i;
	register int c;
	
	c=particle_system_a.array_size;
	for(i=0; i<c; i++)
	{
		if(particle_system_a.systems[i].name)
		{
			if(!strcmp(name, particle_system_a.systems[i].name))
			{
				return &particle_system_a.systems[i];
			}
		}
	}
	return NULL;*/
}


PEWAPI void particle_DestroyParticleSystem(particle_system *system)
{
	/*if(system)
	{
		//active_particle_system_a.active_count--;
		active_particle_system_a.free_positions_stack[++active_particle_system_a.stack_top]=system->system_index;
		system->system_life=-2;
		free(system->particles);
		free(system->free_positions_stack);
		scenegraph_RemoveNode(system->assigned_node, 0);
	}*/
}

PEWAPI void particle_SpawnParticle(particle_system *system)
{
	/*particle_t p;
	vec2_t v;
	float x;
	float y;
	if(system)
	{

		x=(float)(rand()%100);
		y=(float)(rand()%100);
		v=vec2(system->world_position.floats[0] + system->pos_dx*(-0.5+(x/100.0)), system->world_position.floats[1]+system->pos_dy*(-0.5+(y/100.0)));
		p.b_exist=1;
		p.life=0;
		p.texture_layer=0;
		p.origin=v;
			
		x=(float)(rand()%100);
		y=(float)(rand()%100);
			
		p.velocity=vec2(system->vel_dx*(-0.5+(x/100.0)), system->vel_dy*(-0.5+(y/100.0)));
			
		if(system->stack_top>=0)
		{
			system->particles[system->free_positions_stack[system->stack_top--]]=p;
			system->particle_count++;
			system->last_spawn=0;
		}
		else 
		{
			system->particles[system->particle_count++]=p;
			system->last_spawn=0;
		}
			
	}*/
}


PEWAPI void particle_RemoveParticle(particle_system *system, int index)
{
	/*if(system)
	{
		system->free_positions_stack[++system->stack_top]=index;
		system->particle_count--;
		system->particles[index].b_exist=0;
	}*/
}


PEWAPI void particle_AttachParticleSystemToEntity(particle_system *system, entity_t *entity)
{
	//scenegraph_SetParent(system->assigned_node, entity->assigned_node, 0);
	return;
}


#ifdef __cplusplus
}
#endif






























