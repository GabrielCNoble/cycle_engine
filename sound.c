#include "sound.h"
#include "console.h"
#include "log.h"

sound_system sound;
source_array source_a;
abuffer_array abuffer_a;
pcm_array pcm_a;

#ifdef __cplusplus
extern "C"
{
#endif


/* TODO: use a single thread for the sound system */

/*
=============
sound_Init
=============
*/
PEWAPI void sound_Init()
{
	
	sound.sound_device=alcOpenDevice(NULL);
	if(!sound.sound_device)
	{
		log_LogMessage("OpenAL: error initializing OpenAL!");
		exit(-4);
		//printf("What a terrible fate. OpenAL didn't cooperate. Bailing out.\n");
		//exit(-4);
	}
	sound.sound_context=alcCreateContext(sound.sound_device, NULL);
	alcMakeContextCurrent(sound.sound_context);
	
	sound.sound_thread = SDL_CreateThread(sound_SoundThread, "sound thread", NULL);
	sound.bm_state = SOUND_SYSTEM_RUNNING;
	
	alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
	alListener3f(AL_VELOCITY, 0.0, 0.0, 0.0);

	sound_InitSourceArray(80);
	
	abuffer_a.buffers=NULL;
	abuffer_a.buffer_count=0;
	sound_ResizeABufferArray(16);

	
}


/*
=============
sound_Finish
=============
*/
PEWAPI void sound_Finish()
{
	int i;
	int c;
	
	sound_StopAllSources();
	sound_WaitForSoundThreads();
	sound_ReleaseABufferArray();
	sound_ReleaseSourceArray();
	alcDestroyContext(sound.sound_context);
	alcCloseDevice(sound.sound_device);
	free(source_a.sources);
	free(source_a.busy_positions_stack);
	free(source_a.free_positions_stack);
	free(abuffer_a.buffers);
}


/*
=============
sound_InitSourceArray
=============
*/
static void sound_InitSourceArray(int size)
{
	int i;
	
	source_a.sources=(source_t *)calloc(size, sizeof(source_t));
	source_a.busy_positions_stack=(int *)calloc(size, sizeof(int));
	source_a.free_positions_stack=(int *)calloc(size, sizeof(int));

	source_a.busy_stack_top=-1;
	source_a.stack_top=0;
	source_a.array_size=size;
	
	for(i=size-1; i>=0; i--)
	{
		source_a.sources[i]=sound_CreateSource(vec2(0.0, 0.0), vec2(0.0, 0.0), 1.0, AL_FALSE);
		*(source_a.free_positions_stack+i)=i;	
	}
	return;
	
	
}


/*
=============
sound_ReleaseSourceArray
=============
*/
static void sound_ReleaseSourceArray()
{
	int i;
	int c;
	c=source_a.array_size;
	for(i=0; i<c; i++)
	{
		alDeleteSources(1, &source_a.sources[i].source_ID);
	}
}

/*
=============
sound_ResizeABufferArray
=============
*/
PEWAPI void sound_ResizeABufferArray(int new_size)
{
	abuffer_t *temp=(abuffer_t *)calloc(new_size, sizeof(abuffer_t));
	if(abuffer_a.buffers)
	{
		memcpy(temp, abuffer_a.buffers, sizeof(abuffer_t)*abuffer_a.buffer_count);
		free(abuffer_a.buffers);
	}
	abuffer_a.buffers=temp;
	abuffer_a.array_size=new_size;
	return;
}


/*
=============
sound_ReleaseABufferArray
=============
*/
static void sound_ReleaseABufferArray()
{
	int i;
	int c;
	c=abuffer_a.array_size;
	for(i=0; i<c; i++)
	{
		alDeleteBuffers(1, &abuffer_a.buffers[i].buffer_ID);
	}
}


/*
=============
sound_ResizePCMArray
=============
*/
PEWAPI void sound_ResizePCMArray(int new_size)
{
	
}



wav_header sound_ParseWAV(FILE *file)
{
	if(file)
	{
		
	}
}


/*
=============
sound_LoadWAV
=============
*/
PEWAPI pcm_t sound_LoadWAV(char *file_name)
{
	FILE *file;							/* TODO: handle extra parameters passed in the file. (maybe Valve's WAV files has some of these...) */
	pcm_t file_data;
	int file_read_index;
	int read_bytes_index;
	int data_size;
	short compression=0;
	short channels=0;
	short bits_per_sample=0;
	int frequency;
	int index=0;
	file_data.size=0;
	file_data.data=NULL;
	
	if(!(file=fopen(file_name, "rb")))
	{
		//printf("Couldn't load [%s]!\n", file_name);
		console_Print(MESSAGE_ERROR, "sound [%s] could not be loaded\n", file_name);
		file_data.size=0;
		return file_data;
	}
	fseek(file, 20, SEEK_CUR);
	
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		
		*((char *)&compression)=fgetc(file);	//any compression?
		*((char *)&compression+1)=fgetc(file);
	
		*((char *)&channels)=fgetc(file);		//how many channels.
		*((char *)&channels+1)=fgetc(file);
	
		*((char *)&frequency)=fgetc(file);		//sample rate
		*((char *)&frequency+1)=fgetc(file);
		*((char *)&frequency+2)=fgetc(file);
		*((char *)&frequency+3)=fgetc(file);
	
		fseek(file, 6, SEEK_CUR);
		*((char *)&bits_per_sample)=fgetc(file);
		*((char *)&bits_per_sample+1)=fgetc(file);
	
		fseek(file, 4, SEEK_CUR);
		*((char *)&data_size)=fgetc(file);		
		*((char *)&data_size+1)=fgetc(file);
		*((char *)&data_size+2)=fgetc(file);
		*((char *)&data_size+3)=fgetc(file);
		
	#else
		
		*((char *)&compression+1)=fgetc(file);	//any compression?
		*((char *)&compression)=fgetc(file);
	
		*((char *)&channels+1)=fgetc(file);		//how many channels.
		*((char *)&channels)=fgetc(file);
	
		*((char *)&frequency+3)=fgetc(file);	//sample rate
		*((char *)&frequency+2)=fgetc(file);
		*((char *)&frequency+1)=fgetc(file);
		*((char *)&frequency)=fgetc(file);
	
		fseek(file, 6, SEEK_CUR);
		*((char *)&bits_per_sample+1)=fgetc(file);
		*((char *)&bits_per_sample)=fgetc(file);
	
		fseek(file, 4, SEEK_CUR);
		*((char *)&data_size+3)=fgetc(file);		
		*((char *)&data_size+2)=fgetc(file);
		*((char *)&data_size+1)=fgetc(file);
		*((char *)&data_size)=fgetc(file);
	
	#endif 	
	
	file_data.frequency=frequency;
	file_data.bits_per_sample=bits_per_sample;
	file_data.size=data_size;
	file_data.data=(int *)calloc(data_size*2, 1); 
	//printf("%s %d\n", file_name, data_size*channels);
	//printf("freq: %d   bps: %d    size: %d    chs: %d\n", file_data.frequency, file_data.bits_per_sample, file_data.size, channels);
	
	if(!file_data.data)
	{
		//printf("something went wrong when alloc'ing memory in LoadWAV()\n");
		console_Print(MESSAGE_ERROR, "not enough memory for the sound [%s]\n", file_name);
		file_data.size=0;
		return file_data;
	}
	switch(channels)
	{
		case 1:
			file_data.num_channels=AL_FORMAT_MONO16;
		break;
		
		case 2:
			file_data.num_channels=AL_FORMAT_STEREO16;
		break;
	}
	
	while(!feof(file))
	{
		#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			*((char *)file_data.data+index*4)=fgetc(file);
			*((char *)file_data.data+index*4+1)=fgetc(file);
			*((char *)file_data.data+index*4+2)=fgetc(file);
			*((char *)file_data.data+index*4+3)=fgetc(file);
		#else
			*((char *)file_data.data+index*4+3)=fgetc(file);
			*((char *)file_data.data+index*4+2)=fgetc(file);
			*((char *)file_data.data+index*4+1)=fgetc(file);
			*((char *)file_data.data+index*4)=fgetc(file);	
		#endif
		index++;
	}
	rewind(file);
	fclose(file);
	return file_data;
}


PEWAPI int sound_LoadSound(char *file_name, char *name)
{
	pcm_t rtrn;
	int abuff_index;
	abuffer_t *abuff;
	rtrn=sound_LoadWAV(file_name);
	if(rtrn.size>0)
	{
		abuff_index=sound_CreateABuffer(name);
		abuff=sound_GetABufferByIndex(abuff_index);
		sound_FillABuffer(abuff, &rtrn);
		sound_DeletePCM_T(&rtrn);
		return abuff_index;
	}
	return -1;
}


/*
=============
sound_DeletePCM_T
=============
*/
PEWAPI void sound_DeletePCM_T(pcm_t *pcm)
{
	if(pcm)
	{
		free(pcm->data);
	}
	return;
}



/*
=============
sound_CreateSource
=============
*/
PEWAPI source_t sound_CreateSource(vec2_t position, vec2_t velocity, float pitch, int b_loop)
{
	source_t source;
	alGenSources(1, &source.source_ID);
	source.position=position;
	source.velocity=velocity;
	source.gain=0.0;
	source.pitch=pitch;
	source.bm_status=0;
	source.write_lock=SDL_CreateMutex();
	
	alSourcef(source.source_ID, AL_PITCH, pitch);
	alSourcef(source.source_ID, AL_GAIN, 1.0);
	alSource3f(source.source_ID, AL_POSITION, source.position.floats[0], source.position.floats[1], 0.0);
	alSource3f(source.source_ID, AL_VELOCITY, source.velocity.floats[0], source.velocity.floats[1], 0.0);
	alSourcei(source.source_ID, AL_LOOPING, 0);
	return source;
}


/*
=============
sound_CreateABuffer
=============
*/
PEWAPI int sound_CreateABuffer(char *name)
{
	abuffer_t audio_buffer;
	alGenBuffers(1, &audio_buffer.buffer_ID);
	audio_buffer.linked_data=NULL;
	audio_buffer.name=name;
	
	if(abuffer_a.buffer_count>=abuffer_a.array_size)
	{
		sound_ResizeABufferArray(abuffer_a.array_size+16);
	}
	abuffer_a.buffers[abuffer_a.buffer_count++]=audio_buffer;
	return abuffer_a.buffer_count-1;
}



PEWAPI int sound_GetAudioBufferIndex(char *name)
{
	int i;
	int c;
	c=abuffer_a.buffer_count;
	for(i=0; i<c; i++)
	{
		if(!strcmp(name, abuffer_a.buffers[i].name))
		{
			return i;
		}
	}
	return -1;
}

PEWAPI abuffer_t *sound_GetAudioBuffer(char *name)
{
	int i;
	int c;
	c=abuffer_a.buffer_count;
	for(i=0; i<c; i++)
	{
		if(!strcmp(name, abuffer_a.buffers[i].name))
		{
			return &abuffer_a.buffers[i];
		}
	}
	return NULL;
}



/*
=============
sound_GetABufferByIndex
=============
*/
PEWAPI abuffer_t *sound_GetABufferByIndex(int abuffer_index)
{
	if(abuffer_index>=0)
	{
		return &abuffer_a.buffers[abuffer_index];
	}
	return NULL;
}


/*
=============
sound_FillABuffer
=============
*/
PEWAPI void sound_FillABuffer(abuffer_t *buffer, pcm_t *data)
{
	if(buffer && data)
	{
		alBufferData(buffer->buffer_ID, data->num_channels, data->data, data->size, data->frequency);
		buffer->linked_data=NULL;
	}
}


/*
=============
sound_AttachABufferToSource
=============
*/
PEWAPI void sound_AttachABufferToSource(source_t *source, abuffer_t *buffer)
{
	if(source && buffer)
	{
		alSourcei(source->source_ID, AL_BUFFER, buffer->buffer_ID);
	}
	return;
}



/*
=============
sound_PlayAudioSource
=============
*/
PEWAPI void sound_PlayAudioSource(source_t *source)
{
	if(source)
	{
		source->bm_status|=SOURCE_PLAYING|SOURCE_ASSIGNED;
		SDL_DetachThread(SDL_CreateThread(sound_PlayAudioSourceDirectThreadFn, "sound_thread", source));
	}
 	
}


/*
=============
sound_ResumeAudioSource
=============
*/
PEWAPI void sound_ResumeAudioSource(source_t *source)
{
	if(source)
	{
		SDL_LockMutex(source->write_lock);
		source->bm_status&= ~SOURCE_PAUSED;
		source->bm_status|=SOURCE_PLAYING;
		alSourcePlay(source->source_ID);
		SDL_UnlockMutex(source->write_lock);
	}
}


/*
=============
sound_PauseAudioSource
=============
*/
PEWAPI void sound_PauseAudioSource(source_t *source)
{
	if(source)
	{
		SDL_LockMutex(source->write_lock);
		source->bm_status&= ~SOURCE_PLAYING;
		source->bm_status|=SOURCE_PAUSED;
		alSourcePause(source->source_ID);
		SDL_UnlockMutex(source->write_lock);
	}
	return;
}



/*
=============
sound_StopAudioSource
=============
*/
PEWAPI void sound_StopAudioSource(source_t *source)
{
	SDL_LockMutex(source->write_lock);
	alSourceStop(source->source_ID);
	SDL_UnlockMutex(source->write_lock);
	return;
}


/*
=============
sound_PauseAllSources
=============
*/
PEWAPI void sound_PauseAllSources()
{
	int i;
	int c;
	c=source_a.busy_stack_top+1;
	
	for(i=0; i<c; i++)
	{
		sound_PauseAudioSource(&source_a.sources[source_a.busy_positions_stack[i]]);
	}
}



/*
=============
sound_ResumeAllSources
=============
*/
PEWAPI void sound_ResumeAllSources()
{
	int i;
	int c;
	c=source_a.busy_stack_top+1;
	
	for(i=0; i<c; i++)
	{
		sound_ResumeAudioSource(&source_a.sources[source_a.busy_positions_stack[i]]);
	}
}



/*
=============
sound_StopAllSources
=============
*/
PEWAPI void sound_StopAllSources()
{
	int i;
	int c;
	c=source_a.busy_stack_top+1;
	
	for(i=0; i<c; i++)
	{
		sound_StopAudioSource(&source_a.sources[source_a.busy_positions_stack[i]]);
	}
}


/*
=============
sound_PlayAudioBufferDirect
=============
*/
PEWAPI source_t *sound_PlayAudioBufferDirect(abuffer_t *buffer, vec2_t position, int b_loop)
{
	register int stack_top;
	register int source_index;
	source_t *source;
	
	if(buffer)
	{
		if(source_a.stack_top>=source_a.array_size)
		{
			printf("no free source to use. Fuck you\n");
			return NULL;	//free_positions_stack is depleted, no free source to use.
		}
	
		stack_top=source_a.stack_top;
		source_index=source_a.free_positions_stack[stack_top++];		//free_positions_stack stack top went down
		source=&source_a.sources[source_index];
		source_a.busy_positions_stack[source_a.busy_stack_top++]=source_index;	//busy_positions_stack stack top went up
		source_a.stack_top=stack_top;

	
		sound_AttachABufferToSource(source, buffer);
		/* no need to lock write_lock here, this source is not in use... */
		source->position=position;
		//source->bm_status|=SOURCE_ASSIGNED;
		alSource3f(source->source_ID, AL_POSITION, source->position.floats[0], source->position.floats[1], 0.0);
		alSourcei(source->source_ID, AL_LOOPING, b_loop);
		sound_PlayAudioSource(source);
		return source;
	}
	return NULL;
	
}


/*
=============
sound_CheckBusySources
=============
*/
static void sound_CheckBusySources()
{
	register int index;
	register int count=source_a.busy_stack_top;
	register int still_busy_index=-1;
	int still_busy[count];
	
	if(count<0) return;
	
	for(index=0; index<=count; index++)
	{
		if(!(source_a.sources[source_a.busy_positions_stack[index]].bm_status&SOURCE_ASSIGNED))
		{
			source_a.free_positions_stack[--source_a.stack_top]=source_a.busy_positions_stack[index];
		}
		else
		{
			still_busy[++still_busy_index]=source_a.busy_positions_stack[index];
		}
	}
	count=still_busy_index;
	for(index=0; index<=count; index++)	//this reorganizes the stack, retightening the indexes.
	{
		source_a.busy_positions_stack[index]=still_busy[index];
	}
	source_a.busy_stack_top=count;
}


/*
=============
sound_WaitForSoundThreads
=============
*/
static void sound_WaitForSoundThreads()
{
	int d_count=0;
	int c;
	int i;
	
	c=source_a.array_size;
	while(d_count<c)
	{
		d_count=0;
		for(i=0; i<c; i++)
		{
			if(!SDL_TryLockMutex(source_a.sources[i].write_lock))
			{
				if(!(source_a.sources[i].bm_status&SOURCE_ASSIGNED)) d_count++;
				SDL_UnlockMutex(source_a.sources[i].write_lock);
			}
		}
		
	}
}


/*
=============
sound_ProcessSound
=============
*/
PEWAPI void sound_ProcessSound()
{
	sound_CheckBusySources();
}


/*
=============
sound_PlayAudioSourceDirectThreadFn
=============
*/
static int sound_PlayAudioSourceDirectThreadFn(void *source)
{
	source_t *src=(source_t *)source;
	
	
	SDL_LockMutex(src->write_lock);
	src->gain=1.0;
	alSourcef(src->source_ID, AL_GAIN, 1.0);
	alSourcePlay(src->source_ID);
	SDL_UnlockMutex(src->write_lock);
	
	ALenum source_state;
	alGetSourcei(src->source_ID, AL_SOURCE_STATE, &source_state);
	
	while(source_state!=AL_STOPPED) 
	{
		SDL_Delay(200);			/* check every 200 ms to avoid busy wait... */
		if(!SDL_TryLockMutex(src->write_lock))
		{
			alGetSourcei(src->source_ID, AL_SOURCE_STATE, &source_state);
			SDL_UnlockMutex(src->write_lock);
		}
		
	}
	SDL_LockMutex(src->write_lock);
	src->bm_status&=  ~SOURCE_ASSIGNED;		//this causes a crash if the application returns before this thread...
	SDL_UnlockMutex(src->write_lock);

	return 0;
}


static int sound_SoundThread(void *args)
{
/*	while(*((int *)args) & SOUND_SYSTEM_RUNNING)
	{
		SDL_Delay(500);
		printf("sound thread\n");
	}*/
}

#ifdef __cplusplus
}
#endif












