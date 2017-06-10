#ifndef ARMATURE_H
#define ARMATURE_H

#include "conf.h"
#include "includes.h"
#include "matrix.h"
#include "vector.h"
#include "model.h"
#include "scenegraph.h"


enum ARMATURE_FLAGS
{
	ARMATURE_PLAYING = 1,
};

typedef struct
{
	float weight;	
	int index;						/* this would, theoretically, allow the use of an unlimitted number of weigths per vertex... */		
	int global_id;					/* global id of the bone inside the armature... */
}weight_t;							/* each of these refers to a vertex... */

typedef struct
{
	int count;
	int size;
	weight_t *weights;	
	mesh_t *mesh;
}wset_t;

typedef struct bone_t
{
	mat3_t orientation;
	vec3_t position;
	vec3_t tip;
	//int wcount;						/* how many vertices this bone affects */
	//int wset_offset;				/* offset to the wset array attached to the armature that contains this bone */
	char *name;
	char bone_id;					/* its id inside its parent's children list. Could be a short or a char */
	char max_children;				
	char child_count;				
	char align;
	short global_id;				/* unique id inside the whole armature this bone is in... */
	short align2;				
	struct bone_t *parent;
	struct bone_t **children;
}bone_t;							/* variable size struct, no field shall be added after **children... */


typedef struct
{
	mat4_t global_transform;	
	vec3_t tip;
	int armature_id;				/* used to reference the armature this bone belongs to... */
	char *name;
}armature_extra_t;

typedef struct
{
	mat3_t world_orientation;
	mat3_t local_orientation;			
	vec3_t world_position;	
	vec3_t local_position;				
	int current_frame;				/* this makes no sense... */
	float time;						
	int bone_count;					
	mesh_t *mesh;					/* mesh this armature will deform */
	wset_t weights;
	int start;						/* where to put the skinned mesh... */
	bone_t *bones;					/* 'them' bones... */
	mat4_t *global_transform;		/* accumulated transforms */
	vec3_t *inverse_bind_pose;				/* just translation, since rotation in bind pose is the identity */
	//mat4_t *bind_pose;
	node_t *assigned_node;
	char *name;
	short flags;
	short c0;						 
}armature_t;

typedef struct
{
	int size;
	int count;
	int stack_top;
	int *free_stack;
	armature_t *armatures;
	//armature_extra_t *extra;
}armature_list_t;

typedef struct
{
	bone_t *bones;
	vec3_t *inverse_bind_pose;
	char *name;
	wset_t weights;
	//wset_t *weights;
	short bone_count;
	//short wset_index;
}armdef_t;

typedef struct
{
	int size;
	int count;
	int stack_top;
	int *free_stack;
	armdef_t *armdefs;
}armdef_list_t;

typedef struct aframe_t
{
	//mat3_t rotation; 
	quaternion_t rotation;
	vec3_t position;
	float time;
}aframe_t;

typedef struct
{
	float fps;			
	float duration;			
	int frame_count;
	int frame_size;											/* in transforms */
	aframe_t *frames;
	char *name;
}animation_t;


typedef struct
{
	int size;
	int count;
	animation_t *animations;
}animation_list_t;

typedef struct
{
	int count;
	int size;
	wset_t *wsets;
}wlist_t;


typedef struct
{
	mesh_t *mesh;
	int start;
	int frame;
	short animation;
	short armature;
	short weight_set;
}skinning_command_buffer_t;

typedef struct
{
	int count;
	int size;
	skinning_command_buffer_t *command_buffers;
}skinning_queue_t;

void armature_Init();

void armature_Finish();

void armature_ProcessArmatures(float delta_time);

void armature_ResizeArmatureList(int new_size);

void armature_ResizeArmDefList(int new_size);

void armature_ResizeAnimationList(int new_size);

void armature_ResizeWeightSetList(int new_size);

void armature_CopyBoneData(bone_t **out, bone_t *in);

void armature_UpdatePoses(float delta_time);

void armature_SkinMeshesCPU();

void armature_SkinMeshesGPU();



int armature_StartWeightSet(int size);

void armature_AddWeightToSet(int set_index, int vertex_index, int bone_id, float weight);

void armature_SortWeightSet(wset_t *set);

void armature_SortIds(wset_t *set, int left, int right);

void armature_SortIndices(wset_t *set, int left, int right);



int armature_StartBoneChain(char *name, vec3_t position, mat3_t *orientation);

int armature_CreateArmDef(bone_t *root, wset_t *set);

bone_t *armature_CreateBone(char *name, vec3_t position, mat3_t *orientation);

void armature_DeleteBones(bone_t *root);

int armature_CreateArmature(armdef_t *armdef, char *name, vec3_t position, mat3_t *orientation);

void armature_ExpandBoneChildList(bone_t **bone, int new_size);

void armature_AddBoneChild(bone_t **parent, bone_t *child);

void armature_RemoveBoneChild(bone_t *parent, int child_id);

PEWAPI bone_t *armature_FindBone(bone_t *root, char *name);




PEWAPI void armature_LoadAnimation(char *file_name, char *name);

PEWAPI void armature_StoreAnimation(animation_t *animation);

void armature_LoadBVH(char *file_name, armature_t *armature, animation_t *animation);

void armature_GetBone(bone_t **parent, char *istr, int *cur_pos, int *channel_count);

void armature_TransformVertex(vec4_t *v, mat4_t *transform, float weight);

PEWAPI animation_t *armature_GetAnimation(char *name);

PEWAPI int armature_GetAnimationIndex(char *name);

PEWAPI int armature_TestDuplicateBoneChain(bone_t *b);

PEWAPI armature_t *armature_GetArmature(char *name);

PEWAPI armature_t *armature_GetArmatureByIndex(int index);

PEWAPI armdef_t *armature_GetArmDef(char *name);

PEWAPI armdef_t *armature_GetArmDefByIndex(int index);

PEWAPI int armature_GetArmDefIndex(char *name);
 
PEWAPI int armature_GetArmatureIndex(char *name);

PEWAPI void armature_PlayAnimation(armature_t *armature, int animation_index);

PEWAPI int armature_test_GenerateAnimation(int bone_count, float fps, int frame_count);



//PEWAPI void armature_SkinMesh(mesh_t *mesh, bone_t *armature);



#endif /* ARMATURE_H */






