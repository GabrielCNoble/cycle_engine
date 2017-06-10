#ifndef  SOUND_H
#define SOUND_H

#include "conf.h"
#include "includes.h"
#include "gmath/vector.h"




enum SOUND_SYSTEM_STATUS
{
	SOUND_SYSTEM_RUNNING = 1,
	SOUND_SYSTEM_STOP = 2
};

enum SOURCE_STATUS
{
	SOURCE_FADING_IN=1,
	SOURCE_FADING_OUT=2,
	SOURCE_PLAYING=4,
	SOURCE_PAUSED=8,
	SOURCE_STOPPED=16,
	SOURCE_JUST_RESUMED=32,
	SOURCE_JUST_PAUSED=64,
	SOURCE_JUST_STOPPED=128,
	SOURCE_ASSIGNED=256
};

typedef struct wav_header
{
	int size;
	int frequency;
	int channels;
	int bps;
	short *data;
}wav_header;


typedef struct pcm_t
{
	int size;
	int frequency;
	int num_channels;
	int bits_per_sample;
	char *name;
	int *data;
}pcm_t;

typedef struct source_t
{
	unsigned int source_ID;
	int bm_status;
	float gain;
	float max_gain;
	float pitch;
	vec2_t position;
	vec2_t velocity;
	SDL_mutex *write_lock;
}source_t;

typedef struct abuffer_t
{
	unsigned int buffer_ID;
	char *name;
	pcm_t *linked_data;
}abuffer_t;

typedef struct source_array
{
	int array_size;
	int cursor;
	int busy_stack_top;
	int *busy_positions_stack;	//keeps the assigned sources inside the array. This is a inverse stack. It's top go from lower to higher addresses.
	int stack_top;
	int *free_positions_stack;	//keeps free audio sources inside the array
	source_t *sources;
}source_array;

typedef struct abuffer_array
{
	int array_size;
	int buffer_count;
	abuffer_t *buffers;
}abuffer_array;

typedef struct pcm_array
{
	int array_size;
	int pcm_count;
	pcm_t *data;
}pcm_array;

typedef struct sound_system
{
	ALCdevice *sound_device;
	ALCcontext *sound_context;
	SDL_Thread *sound_thread;
	int bm_state;
	
}sound_system;

#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void sound_Init();

PEWAPI void sound_Finish();

static void sound_InitSourceArray(int size);

static void sound_ReleaseSourceArray();

PEWAPI void sound_ResizeABufferArray(int new_size);

static void sound_ReleaseABufferArray();

PEWAPI void sound_ResizePCMArray(int new_size);

wav_header sound_ParseWAV(FILE *file);

pcm_t sound_LoadWAV(char *file_name);

PEWAPI int sound_LoadSound(char *file_name, char *name);

PEWAPI void sound_DeletePCM_T(pcm_t *pcm);

PEWAPI source_t sound_CreateSource(vec2_t position, vec2_t velocity, float pitch, int b_loop);

PEWAPI int sound_CreateABuffer(char *name);

PEWAPI int sound_GetAudioBufferIndex(char *name);

PEWAPI abuffer_t *sound_GetAudioBuffer(char *name);

PEWAPI abuffer_t *sound_GetABufferByIndex(int abuffer_index);

PEWAPI void sound_FillABuffer(abuffer_t *buffer, pcm_t *data);

PEWAPI void sound_AttachABufferToSource(source_t *source, abuffer_t *buffer);

PEWAPI void sound_PlayAudioSource(source_t *source);

PEWAPI void sound_ResumeAudioSource(source_t *source);

PEWAPI void sound_PauseAudioSource(source_t *source);

PEWAPI void sound_StopAudioSource(source_t *source);

PEWAPI void sound_PauseAllSources();

PEWAPI void sound_ResumeAllSources();

PEWAPI void sound_StopAllSources();

PEWAPI source_t *sound_PlayAudioBufferDirect(abuffer_t *buffer, vec2_t position, int b_loop);

static void sound_CheckBusySources();

static void sound_WaitForSoundThreads();

PEWAPI void sound_ProcessSound();

static int sound_PlayAudioSourceDirectThreadFn(void *source);

static int sound_SoundThread(void *args);

#ifdef __cplusplus
}
#endif


#endif /* SOUND_H */



























