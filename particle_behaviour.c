#include "particle_behaviour.h"



#define PARTICLE_GRAVITY -0.008


void smoke_default(particle_system *system)
{
	register int k;
	register int j;
	j=system->max_particles;
	system->last_spawn++;
	for(k=0; k<j; k++)
	{			
		if(system->particles[k].b_exist)
		{
						
			if(system->particles[k].life<system->max_particle_life)
			{
				system->particles[k].origin.floats[0]+=system->particles[k].velocity.floats[0];
				system->particles[k].origin.floats[1]+=system->particles[k].velocity.floats[1];
				system->particles[k].velocity.floats[1]-=PARTICLE_GRAVITY;
				system->particles[k].life++;
			}
			else
			{
				particle_RemoveParticle(system, k);
			}
					
			if(system->last_spawn>=system->spawn_delay && system->particle_count<system->max_particles)
			{
				particle_SpawnParticle(system);
			}
		}
					
	}
}


void spark_default(particle_system *system)
{
	
	register int k;
	register int j;
	j=system->max_particles;
	system->last_spawn++;
	for(k=0; k<j; k++)
	{			
		if(system->particles[k].b_exist)
		{
						
			if(system->particles[k].life<system->max_particle_life)
			{
				system->particles[k].origin.floats[0]+=system->particles[k].velocity.floats[0];
				system->particles[k].origin.floats[1]+=system->particles[k].velocity.floats[1];
				system->particles[k].velocity.floats[1]+=PARTICLE_GRAVITY*5;
				system->particles[k].life++;
			}
			else
			{
				particle_RemoveParticle(system, k);
			}
					
			if(system->last_spawn>=system->spawn_delay && system->particle_count<system->max_particles)
			{
				particle_SpawnParticle(system);
			}
		}
					
	}
}


void behaviour_1(particle_system *system)
{
	int j;
	int k;
	j=system->max_particles;
	system->last_spawn++;
	for(k=0; k<j; k++)
	{		
		if(system->particles[k].b_exist)
		{
				
			if(system->particles[k].life<system->max_particle_life)
			{
				system->particles[k].origin.floats[0]+=system->particles[k].velocity.floats[0];
				system->particles[k].origin.floats[1]+=system->particles[k].velocity.floats[1];
				system->particles[k].life++;
				
				if(!(system->particles[k].life%2))
				{
					system->particles[k].texture_layer++;
					if(system->particles[k].texture_layer>13)
					{
						system->particles[k].texture_layer=0;
					}
				}
			}
			else
			{
				particle_RemoveParticle(system, k);
			}
					
			if(system->last_spawn>=system->spawn_delay && system->particle_count<system->max_particles)
			{
				particle_SpawnParticle(system);
			}
		}
					
	}

}

void behaviour_2(particle_system *system)
{
	int j;
	int k;
	j=system->max_particles;
	system->last_spawn++;
	for(k=0; k<j; k++)
	{		
		if(system->particles[k].b_exist)
		{
				
			if(system->particles[k].life<system->max_particle_life)
			{
				/*system->particles[k].origin.floats[0]+=system->particles[k].velocity.floats[0];
				system->particles[k].origin.floats[1]+=system->particles[k].velocity.floats[1];*/
				system->particles[k].life++;
				
				if(!(system->particles[k].life%3))
				{
					system->particles[k].texture_layer++;
					if(system->particles[k].texture_layer>15)
					{
						system->particles[k].texture_layer=0;
					}
				}
			}
			else
			{
				particle_RemoveParticle(system, k);
			}
					
			if(system->last_spawn>=system->spawn_delay && system->particle_count<system->max_particles)
			{
				particle_SpawnParticle(system);
			}
		}
					
	}
}










