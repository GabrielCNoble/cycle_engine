#include "armature.h"
#include "macros.h"
#include "console.h"
#include "file.h"
#include "gpu.h"
//#include "ozz\animation\runtime\animation.h"

#define FLOAT_BUFFER_SIZE 33554432				/* this should be enough floats... */

#define MAX_BONES 64

armature_list_t armature_list;
animation_list_t animation_list;
armdef_list_t armature_defs;
//skinning_queue_t skinning_queue;
wlist_t weight_sets;


int float_buffer_index;
float *float_buffer;

mat4_t transform_stack_base[MAX_BONES];
mat4_t *transform_stack = &transform_stack_base[1];

mat4_t inverse_transform_stack_base[MAX_BONES];
mat4_t *inverse_transform_stack = &inverse_transform_stack_base[1];



void armature_Init()
{
	//armature_a.armatures = (bone_t **)calloc(sizeof(bone_t *), 10);
	//armature_a.max_armatures = 10;
	//armature_a.armature_count = 0;
	
	
	transform_stack_base[0] = mat4_t_id();
	inverse_transform_stack[0] = mat4_t_id();
	
	
	animation_list.animations = NULL;
	armature_ResizeAnimationList(16);
	armature_list.armatures = NULL;
	armature_ResizeArmatureList(16);
	weight_sets.wsets = NULL;
	armature_ResizeWeightSetList(16);
	
	armature_defs.armdefs = NULL;
	armature_ResizeArmDefList(16);
	
	
	float_buffer = (float *)malloc(sizeof(float) * FLOAT_BUFFER_SIZE);
	//skinning_queue.command_buffers = NULL;
	//armature_ResizeSkinningQueue(16);
	
	return;
}

/*void armature_ResizeArmatureArray(int new_size)
{
	bone_t **temp;
	if(new_size > armature_a.max_armatures)
	{
		temp = (bone_t **)calloc(new_size, sizeof(bone_t *));
		memcpy(temp, armature_a.armatures, sizeof(bone_t *) * armature_a.armature_count);
		free(armature_a.armatures);
		armature_a.max_armatures = new_size;
		armature_a.armatures = temp;
	}
	return;
}*/



void armature_Finish()
{
	
	int i;
	int c = armature_list.count;
	
	for(i = 0; i < c; i++)
	{
		armature_DeleteBones(armature_list.armatures[i].bones);
		free(armature_list.armatures[i].name);
		free(armature_list.armatures[i].global_transform);
	}
	
	free(armature_list.armatures);
	//free(armature_list.extra);
	free(armature_list.free_stack);
	
	c = armature_defs.count;
	for(i = 0; i < c; i++)
	{
		armature_DeleteBones(armature_defs.armdefs[i].bones);
		free(armature_defs.armdefs[i].name);
	}
	
	free(armature_defs.armdefs);
	free(armature_defs.free_stack);
}

void armature_ProcessArmatures(float delta_time)
{
	armature_UpdatePoses(delta_time);
	armature_SkinMeshesCPU();
}

void armature_ResizeArmatureList(int new_size)
{
	armature_t *temp = (armature_t *)malloc(sizeof(armature_t) * new_size);
	armature_extra_t *etemp = (armature_extra_t *)malloc(sizeof(armature_extra_t) * new_size);
	int *itemp = (int *)malloc(sizeof(int) * new_size);
	if(armature_list.armatures)
	{
		memcpy(temp, armature_list.armatures, sizeof(armature_t) * armature_list.size);
		//memcpy(etemp, armature_list.extra, sizeof(armature_extra_t) * armature_list.size);
		free(armature_list.armatures);
		//free(armature_list.extra);
		free(armature_list.free_stack);
	}
	else
	{
		armature_list.count = 0;
		armature_list.stack_top = -1;
	}
	
	armature_list.armatures = temp;
	//armature_list.extra = etemp;
	armature_list.free_stack = itemp;
	armature_list.size = new_size;
}

void armature_ResizeArmDefList(int new_size)
{
	armdef_t *temp = (armdef_t *)malloc(sizeof(armdef_t) * new_size);
	int *itemp = (int *)malloc(sizeof(int) * new_size);
	if(armature_defs.armdefs)
	{
		memcpy(temp, armature_defs.armdefs, sizeof(armdef_t) * armature_defs.size);
		free(armature_defs.armdefs);
		free(armature_defs.free_stack);
	}
	else
	{
		armature_defs.count = 0;
		armature_defs.stack_top = -1;
	}
	
	armature_defs.armdefs = temp;
	armature_defs.free_stack = itemp;
	armature_defs.size = new_size;
}

void armature_ResizeAnimationList(int new_size)
{
	animation_t *temp = (animation_t *)malloc(sizeof(animation_t) * new_size);
	if(animation_list.animations)
	{
		memcpy(temp, animation_list.animations, sizeof(animation_t) * animation_list.size);
		free(animation_list.animations);
	}
	else
	{
		animation_list.count = 0;
	}
	
	animation_list.animations = temp;
	animation_list.size = new_size;
	
}

void armature_ResizeWeightSetList(int new_size)
{
	wset_t *temp = (wset_t *)malloc(sizeof(wset_t) * new_size);
	if(weight_sets.wsets)
	{
		memcpy(temp, weight_sets.wsets, sizeof(wset_t) * weight_sets.size);
		free(weight_sets.wsets);
	}
	else
	{
		weight_sets.count = 0;
	}
	
	weight_sets.wsets = temp;
	weight_sets.size = new_size;
}

void armature_CopyBoneData(bone_t **out, bone_t *in)
{
	int children[128];
	int stack_top = 0;
	bone_t *b;
	bone_t *p;
	bone_t *r = NULL;
	bone_t *parent;
	b = in;
		
	children[stack_top] = 0;
	
	do
	{
		if(!children[stack_top])
		{
			p = armature_CreateBone(b->name, b->position, &b->orientation);
			p->global_id = b->global_id;
			p->tip = b->tip;
			
			if(!r)
			{
				r = p;
				parent = r;
			}
			else
			{
				armature_AddBoneChild(&parent, p);
				//parent->tip = p->position;
				parent = p;
			}

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
			b = b->parent;
			parent = parent->parent;
			stack_top--;
		}
	}while(parent);
	
	*out = r;
}

void armature_UpdatePoses(float delta_time)
{
	int i;
	int c = armature_list.count;
	int j;
	int k;
	int m;
	//int n;
	int ia;
	int ib;
	float p;
	float t;
	float d;
	bone_t *b;
	mat4_t local_bone_transform;
	mat4_t global_bone_transform;
	animation_t *a;
	armature_t *q;
	//aframe_t *fra;
	
	int stack_top = -1;
	int bone_count;
	char children[128];
	aframe_t *fa;
	aframe_t *fb;
	quaternion_t qc;
	
	for(i = 0; i < c; i++)
	{
		if(!(armature_list.armatures[i].flags & ARMATURE_PLAYING)) continue;
		
		q = &armature_list.armatures[i];
		b = q->bones;
		a = &animation_list.animations[q->c0];
		k = q->bone_count;
		
		d = a->duration;

		p = q->time;
		p -= ((int)(p / d)) * d;
		m = a->frame_count;
		j = m >> 1;
		k = j;
		
		if(m > 2)
		{
			while(1)
			{
				if(a->frames[j].time <= p && a->frames[j + 1].time >= p)
				{
					ia = j;
					ib = j + 1;
					break;
				}
				else
				{
					if(k > 1) k >>= 1;
					if(a->frames[j].time > p) j -= k;
					else j += k;
				}
			}
		}
		else
		{
			ia = 0;
			ib = 1;
		}

		stack_top = 0;
		children[stack_top] = 0;
		transform_stack[stack_top - 1] = mat4_t_id();
		k = a->frame_count;
		while(b)
		{
			if(!children[stack_top])
			{
				fa = &a->frames[ia + k * b->global_id];
				fb = &a->frames[ib + k * b->global_id];	
				t = (p - fa->time) / (fb->time - fa->time);
				//qc = slerp(&fa->rotation, &fb->rotation, t);
				qc = lerp4(&fa->rotation, &fb->rotation, t);
				quat_to_mat3_t(&b->orientation, &qc);
				b->position = lerp3(fa->position, fb->position, t);
				
				mat4_t_compose(&local_bone_transform, &b->orientation, b->position);
				mat4_t_mult(&q->global_transform[b->global_id], &local_bone_transform, &transform_stack[stack_top - 1]);
				memcpy(&transform_stack[stack_top], &q->global_transform[b->global_id], sizeof(mat4_t));
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
				b = b->parent;
				stack_top--;
			}

		}
		q->time += delta_time;
	}
}

/* this function should avoid skinning objects that are
not visible... */
void armature_SkinMeshesCPU()
{
	int i;
	int c = armature_list.count;
	int j;
	int k;

	short bm_attribs = 0;
	int byte_count;
	int byte_offset = 0;
	int float_offset = 0;
	int weight_count;
	int start;
	int index;
	
	mesh_t *mesh;
	armature_t *a;
	wset_t *set;
	mat4_t local_bone_transform;
	mat4_t bone_transform;
	mat4_t inverse_bone_transform;
	int stack_top = -1;
	int children[64];
	//vec3_t translation_base[129];
	//vec3_t *translation = &translation_base[1];
	vec3_t *inverse_translation;
	mat4_t *global_transform;
	bone_t *root;
	//wset_t *set;
	vec4_t p;
	vec4_t n;
	vec4_t t;
	
	vec4_t temp4;
	vec3_t temp3;
	
	vec4_t ap;
	vec4_t an;
	vec4_t at;
	
	for(i = 0; i < c; i++)
	{
		
		mesh = armature_list.armatures[i].mesh;
		a = &armature_list.armatures[i];
		
		if(!mesh || (!(a->flags & ARMATURE_PLAYING))) continue;
		float_buffer_index = 0;
		
		
		root = a->bones;
		stack_top = 0;
		
		start = armature_list.armatures[i].start;
		//set = &weight_sets.wsets[armature_list.armatures[i].weight_set];
		//k = set->count;
		
		//mat4_t_compose(&local_bone_transform, &root->orientation, root->position);
		
		//processed_child_stack[0] = 0;
		//transform_stack[-1] = mat4_t_id();
		//MatrixCopy4(&transform_stack[0], &local_bone_transform);
		
		
		//children[0] = 1;
		//children[root->global_id] = 0;
		//translation[-1] = vec3(0.0, 0.0, 0.0);
		//k = -1;
		/*while(root)
		{

			if(!children[root->global_id])
			{
				j = root->global_id;
				temp3 = a->bind_pose[j];
				
				translation[j] = temp3;
			}
			
			if(root->child_count > children[root->global_id])
			{
				children[root->global_id]++;
				root = root->children[children[root->global_id] - 1];
				children[root->global_id] = 0;
			}
			else
			{
				root = root->parent;
			}
		}*/
		
		set = &a->weights;
		k = set->count;
		index = -1;
		float_offset = mesh->vert_count * 3;
		mesh = a->mesh;
		global_transform = a->global_transform;
		inverse_translation = a->inverse_bind_pose;
		
		bm_attribs = 0;
		
		if(mesh->n_data)
		{
			bm_attribs |= 1;
		}
		if(mesh->t_data)
		{
			bm_attribs |= 2;
		}

		for(j = 0; j < k;)
		{
				
			index = set->weights[j].index;
				
			p.x = mesh->v_data[index * 3];
			p.y = mesh->v_data[index * 3 + 1];
			p.z = mesh->v_data[index * 3 + 2];
			
			//if(a->mesh->n_data)
			if(bm_attribs & 1)
			{
				n.x = mesh->n_data[index * 3];
				n.y = mesh->n_data[index * 3 + 1];
				n.z = mesh->n_data[index * 3 + 2];
			}
			
			//if(a->mesh->t_data)
			if(bm_attribs & 2)
			{
				t.x = mesh->t_data[index * 3];
				t.y = mesh->t_data[index * 3 + 1];
				t.z = mesh->t_data[index * 3 + 2];
			}
				
			ap.x = 0.0;
			ap.y = 0.0;
			ap.z = 0.0;
					
			an.x = 0.0;
			an.y = 0.0;
			an.z = 0.0;
			
			at.x = 0.0;
			at.y = 0.0;
			at.z = 0.0;
				
			while(index == set->weights[j].index && j < k)
			{
				temp4.x = p.x + inverse_translation[set->weights[j].global_id].x;
				temp4.y = p.y + inverse_translation[set->weights[j].global_id].y;
				temp4.z = p.z + inverse_translation[set->weights[j].global_id].z;
				temp4.w = set->weights[j].weight;
				//temp4 = MultiplyVector4(&global_transform[set->weights[j].global_id], temp4);
				
				mat4_t_vec4_t_mult(&global_transform[set->weights[j].global_id], &temp4);
				
				ap.x += temp4.x * set->weights[j].weight;
				ap.y += temp4.y * set->weights[j].weight;
				ap.z += temp4.z * set->weights[j].weight;
					
				if(bm_attribs & 1)
				{
					temp4.x = n.x;
					temp4.y = n.y;
					temp4.z = n.z;
					temp4.w = 0.0;
					//temp4 = MultiplyVector4(&global_transform[set->weights[j].global_id], temp4);
					mat4_t_vec4_t_mult(&global_transform[set->weights[j].global_id], &temp4);
					an.x += temp4.x * set->weights[j].weight;
					an.y += temp4.y * set->weights[j].weight;
					an.z += temp4.z * set->weights[j].weight;
				}	
				
				if(bm_attribs & 2)
				{
					temp4.x = t.x;
					temp4.y = t.y;
					temp4.z = t.z;
					temp4.w = 0.0;
					//temp4 = MultiplyVector4(&global_transform[set->weights[j].global_id], temp4);
					mat4_t_vec4_t_mult(&global_transform[set->weights[j].global_id], &temp4);
					at.x += temp4.x * set->weights[j].weight;
					at.y += temp4.y * set->weights[j].weight;
					at.z += temp4.z * set->weights[j].weight;
				}
				
					
				j++;
			}
			
			float_buffer[index * 3] = ap.x;
			float_buffer[index * 3 + 1] = ap.y;
			float_buffer[index * 3 + 2] = ap.z;
					
			if(bm_attribs & 1)
			{
				float_buffer[float_offset + index * 3] = an.x;
				float_buffer[float_offset + index * 3 + 1] = an.y;
				float_buffer[float_offset + index * 3 + 2] = an.z;
			}
			
			if(bm_attribs & 2)
			{
				float_buffer[float_offset * 2 + index * 3] = an.x;
				float_buffer[float_offset * 2 + index * 3 + 1] = an.y;
				float_buffer[float_offset * 2 + index * 3 + 2] = an.z;
			}
				
		}

		//printf("\n");
		/* basic framework (to remember): translation contain the
		inverse translation (inverse bind "transform"), and can
		be accessed randomly. Each armature contains a array
		of transforms, which are the concatened transforms
		from all the parent bones plus its own, and
		also can be accessed randomly. The idea is to
		sort the armature's weights by vertice index,
		and then iterate over all the weights
		that affect that vertex... */
		
		
		
		
		
		
		byte_count = sizeof(float) * 3 * mesh->vert_count;
	
		if(bm_attribs & 1)
		{
			byte_count += byte_count;
		}
		if(bm_attribs & 2)
		{
			byte_count += byte_count;
		}
		/*if(mesh->t_c_data)
		{
			byte_count += sizeof(float) * 2 * mesh->vert_count;
		}*/
		
		gpu_Write(start, 0, float_buffer, byte_count, 1);

	}
}

void armature_SkinMeshesGPU()
{
	
}

int armature_StartWeightSet(int size)
{
	int set_index = weight_sets.count;
	
	if(likely(set_index < weight_sets.size))
	{
		_add_weight_set:
		weight_sets.wsets[set_index].weights = (weight_t *)malloc(sizeof(weight_t) * size);
		weight_sets.wsets[set_index].size = size;
		weight_sets.wsets[set_index].count = 0;
		weight_sets.count++;
		return set_index;
	}
	else
	{
		armature_ResizeWeightSetList(weight_sets.size + 16);
		goto _add_weight_set;
	}
	
}

void armature_AddWeightToSet(int set_index, int vertex_index, int bone_id, float weight)
{
	wset_t *set;
	weight_t *t;
	if(set_index > -1)
	{
		set = &weight_sets.wsets[set_index];
		
		if(set->count < set->size)
		{
			_add_weight_data:
			set->weights[set->count].global_id = bone_id;
			set->weights[set->count].index = vertex_index;
			set->weights[set->count].weight = weight;
			set->count++;
		}
		else
		{
			t = (weight_t *)malloc(sizeof(weight_t) * (set->size + 16));
			memcpy(t, set->weights, sizeof(weight_t) * set->size);
			free(set->weights);
			set->weights = t;
			set->size += 16;
			goto _add_weight_data;
		}
	}
	return;
}

void armature_SortWeightSet(wset_t *set)
{
	//wset_t *set = &weight_sets.wsets[set_index];
	armature_SortIndices(set, 0, set->count - 1);
	//armature_SortIds(set, 0, set->count - 1);
}

void armature_SortIds(wset_t *set, int left, int right)
{
	int i = left;
	int j = right;
	int m = (right + left) / 2;
	weight_t t;
	weight_t q = set->weights[m];
	while(i <= j)
	{
		for(; set->weights[i].global_id < q.global_id && i < right; i++);
		for(; set->weights[j].global_id > q.global_id && j > left; j--);
		
		if(i <= j)
		{
			t = set->weights[i];
			set->weights[i] = set->weights[j];
			set->weights[j] = t;
			i++;
			j--;
		} 
	}
	if(j > left) armature_SortIds(set, left, j);
	if(i < right) armature_SortIds(set, i, right);
	
	return;
}

void armature_SortIndices(wset_t *set, int left, int right)
{
	int i = left;
	int j = right;
	int m = (right + left) / 2;
	weight_t t;
	weight_t q = set->weights[m];
	while(i <= j)
	{
		for(; set->weights[i].index < q.index && i < right; i++);
		for(; set->weights[j].index > q.index && j > left; j--);
		
		if(i <= j)
		{
			t = set->weights[i];
			set->weights[i] = set->weights[j];
			set->weights[j] = t;
			i++;
			j--;
		} 
	}
	if(j > left) armature_SortIndices(set, left, j);
	if(i < right) armature_SortIndices(set, i, right);
	
	return;
}

bone_t *armature_CreateBone(char *name, vec3_t position, mat3_t *orientation)
{
	bone_t *temp;
	temp = (bone_t *)malloc(sizeof(bone_t ) + sizeof(bone_t *)*10);
	temp->max_children = 10;
	temp->name = strdup(name);
	temp->parent = NULL;
	temp->child_count = 0;
	temp->bone_id = -1;										/* root bone */
	temp->orientation = *orientation;
	temp->position = position;
	temp->children = ((bone_t **)&temp->children)+1;		/* temp->children contains the address of the start of its children list, so
															   temp->children[0] is the first child bone */
	//temp->world_orientation = temp->local_orientation;
	//temp->world_position = temp->local_position;
	
	return temp;
}

void armature_DeleteBones(bone_t *root)
{
	int stack_top = 0;
	int children[64];
	bone_t *b = root;
	bone_t *p;
	
	
	children[stack_top] = 0;
	
	while(b)
	{
		if(children[stack_top] < b->child_count)
		{
			b = b->children[children[stack_top]];
			children[stack_top]++;
			stack_top++;
			children[stack_top] = 0;
		}
		else
		{
			p = b->parent;
			free(b->name);
			free(b);
			b = p;
			stack_top--;
		}
	}
	return;
}

int armature_CreateArmature(armdef_t *armdef, char *name, vec3_t position, mat3_t *orientation)
{
	int index;
	int i;
	int c;
	bone_t *b;
	armature_t *a;
	if(armdef)
	{
		if(armature_list.stack_top > -1)
		{
			index = armature_list.free_stack[armature_list.stack_top--];
		}
		else
		{
			if(armature_list.count >= armature_list.size)
			{
				armature_ResizeArmatureList(armature_list.size + 16);
			}
			index = armature_list.count++;
		}
		
		armature_CopyBoneData(&b, armdef->bones);
		
		armature_list.armatures[index].bones = b;
		armature_list.armatures[index].flags = 0;
		armature_list.armatures[index].name = strdup(name);
		armature_list.armatures[index].time = 0.0;
		armature_list.armatures[index].global_transform = (mat4_t *)malloc(sizeof(mat4_t) * armdef->bone_count);
		armature_list.armatures[index].inverse_bind_pose = armdef->inverse_bind_pose;
		//armature_list.armatures[index].weight_set = -1;
		armature_list.armatures[index].weights = armdef->weights;
		armature_list.armatures[index].bone_count = armdef->bone_count;
		
		armature_list.armatures[index].c0 = -1;
		armature_list.armatures[index].mesh = NULL;
		
		//MatrixCopy3(&armature_list.armatures[index].orientation, orientation);
		
		memcpy(&armature_list.armatures[index].local_orientation, orientation, sizeof(mat3_t));
		memcpy(&armature_list.armatures[index].world_orientation, orientation, sizeof(mat3_t));
		
		armature_list.armatures[index].local_position = position;
		armature_list.armatures[index].world_position = position;
		armature_list.armatures[index].current_frame = 0;
		armature_list.armatures[index].assigned_node = scenegraph_AddNode(NODE_ARMATURE, index, -1, armature_list.armatures[index].name);
		
		c = armdef->bone_count;
		a = &armature_list.armatures[index];
		for(i = 0; i < c; i++)
		{
			a->global_transform[i] = mat4_t_id();
		}
		
		return index;
	}
	return -1;
}

int armature_StartBoneChain(char *name, vec3_t position, mat3_t *orientation)
{
	/*bone_t *temp = armature_CreateBone(name, position, orientation);
	bone_t *b;
	armature_t a;
	
	if(armature_list.count >= armature_list.size)
	{
		armature_ResizeArmatureList(armature_list.size + 10);
	}
	armature_list.armatures[armature_list.count++] = temp;
	
	return armature_list.armature_count - 1;*/
}

int armature_CreateArmDef(bone_t *root, wset_t *set)
{
	bone_t *b;
	armdef_t a;
	int index;
	int i;
	int j;
	int k;
	int bone_count = 0;
	int stack_top = 0;
	char children[128];
	vec3_t v;
	vec3_t *inverse_bind_pose;
	//vec3_t translation_base[129];
	//vec3_t *translation = &translation_base[1];
	if(likely(root))
	{
		
		
		if(armature_defs.stack_top > -1)
		{
			index = armature_defs.free_stack[armature_defs.stack_top--];
		}
		else
		{
			if(armature_defs.count >= armature_defs.size)
			{
				armature_ResizeArmDefList(armature_defs.count + 16);
			}
			index = armature_defs.count++;
		}
		
		b = root;
		children[stack_top] = 0;
		while(b)
		{
			if(!children[stack_top])
			{
				b->global_id = bone_count;
				bone_count++;
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
		inverse_bind_pose = (vec3_t *)malloc(sizeof(vec3_t) * bone_count);	
		
		
		b = root;
		stack_top = 0;
		children[stack_top] = 0;
		//bind_pose++;
		 
		while(b)
		{
			if(!children[stack_top])
			{
				inverse_bind_pose[b->global_id] = b->position;
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
		
		//inverse_bind_pose[-1] = vec3(0.0, 0.0, 0.0);
		//armature_defs.armdefs[index].bind_pose = (vec3_t *)malloc(sizeof(vec3_t) * bone_count);
		
		b = root;
		stack_top = 0;
		children[stack_top] = 0;
		k = -1;
		while(b)
		{
			if(!children[stack_top])
			{
				v = inverse_bind_pose[b->global_id];
				j = b->global_id;
				inverse_bind_pose[j].x = -v.x;
				inverse_bind_pose[j].y = -v.y;
				inverse_bind_pose[j].z = -v.z;
				if(b->parent)
				{
					k = b->parent->global_id;
					inverse_bind_pose[j].x += inverse_bind_pose[k].x;
					inverse_bind_pose[j].y += inverse_bind_pose[k].y;
					inverse_bind_pose[j].z += inverse_bind_pose[k].z;
				}
				
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
		
		
		/*for(i = 0; i < bone_count; i++)
		{
			
		}*/
		
		
		
		
		
		armature_defs.armdefs[index].inverse_bind_pose = inverse_bind_pose;
		armature_defs.armdefs[index].bones = root;
		armature_defs.armdefs[index].bone_count = bone_count;
		armature_defs.armdefs[index].name = strdup(root->name);
		
		if(set)
		{
			armature_defs.armdefs[index].weights.count = set->count;
			armature_defs.armdefs[index].weights.size = set->size;
			armature_defs.armdefs[index].weights.weights = set->weights;
			armature_defs.armdefs[index].weights.mesh = set->mesh;
			armature_SortWeightSet(&armature_defs.armdefs[index].weights);
			
		/*	for(i = 0; i < armature_defs.armdefs[index].weights.count; i++)
			{
				printf("%d  %d\n", i, armature_defs.armdefs[index].weights.weights[i].index);
			}*/
			
			//armature_SortIndices(&armature_defs.armdefs[index].weights, 0, set->count);
		}
		else
		{
			armature_defs.armdefs[index].weights.count = 0;
			armature_defs.armdefs[index].weights.size = 0;
			armature_defs.armdefs[index].weights.weights = NULL;
			armature_defs.armdefs[index].weights.mesh = NULL;
		}
		
		//armature_defs.armdefs[index].wset_index = wset_index;
		
		
		
		//printf("armdef %s\n", armature_defs.armdefs[index].name);
		return index;
	}
	
	return -1;
}

void armature_ExpandBoneChildList(bone_t **bone, int new_size)
{
	bone_t *temp;
	int i;
	if(bone)
	{
		if((*bone)->max_children < new_size)
		{
			temp = (bone_t *)malloc(sizeof(bone_t) + sizeof(bone_t *) * new_size);
			temp->max_children = new_size;
			temp->child_count = (*bone)->child_count;
			temp->name = (*bone)->name;
			temp->orientation = (*bone)->orientation;
			temp->position = (*bone)->position;
			temp->bone_id = (*bone)->bone_id;
			temp->global_id = (*bone)->global_id;
			temp->parent = (*bone)->parent;
			temp->children = ((bone_t **)&temp->children)+1;
			memcpy(&temp->children[0], &(*bone)->children[0], sizeof(bone_t *) * (*bone)->child_count);
			
			/* this bone is a child of another bone */
			if(temp->bone_id>=0)
			{
				temp->parent->children[temp->bone_id] = temp;
			}
			/* make the child bones of the old bone point to the new parent */
			for(i = 0; i < temp->child_count; i++)
			{
				temp->children[i]->parent = temp;
			}
			
			free(*bone);
			*bone = temp;
			
			printf("bone children list expanded\n");
		}
	}
	return;
}

void armature_AddBoneChild(bone_t **parent, bone_t *child)
{
	mat3_t new_child_orientation;
	if(parent && child)
	{
		if((*parent)->child_count >= (*parent)->max_children)
		{
			armature_ExpandBoneChildList(parent, (*parent)->max_children + 1); 
		}
		
		child->bone_id = (*parent)->child_count;
		(*parent)->children[(*parent)->child_count] = child;
		child->parent = *parent;
		(*parent)->child_count++;
		
	}
	return;
}

void armature_RemoveBoneChild(bone_t *parent, int child_id)
{
	
}

PEWAPI bone_t *armature_FindBone(bone_t *root, char *name)
{
	int i;
	int c;
	int stack_top = 0;
	int visited_child_stack[64];
	bone_t *b = root;
	
	visited_child_stack[stack_top] = 0;
	while(b)
	{
		if(!strcmp(b->name, name))
		{
			return b;
		}
		else
		{
			if(visited_child_stack[stack_top] < b->child_count)
			{
				b = b->children[visited_child_stack[stack_top]];
				visited_child_stack[stack_top]++;
				stack_top++;
				visited_child_stack[stack_top] = 0;
			}
			else
			{
				stack_top--;
				b = b->parent;
			}
		}
	}
	return NULL;
}

PEWAPI void armature_LoadAnimation(char *file_name, char *name)
{
	
}

PEWAPI void armature_StoreAnimation(animation_t *animation)
{
	if(animation)
	{
		if(animation_list.count < animation_list.size)
		{
			_add_animation_data:
				
			animation_list.animations[animation_list.count] = *animation;
			animation_list.count++;	
		}
		else
		{
			armature_ResizeAnimationList(animation_list.count + 16);
			goto _add_animation_data;
		}
	}
}



enum BVH_LOADER_STATE
{
	BVH_NONE = 0,
	BVH_GET_BONE,
};

enum BVH_CHANNELS
{
	BVH_X_POSITION = 1,
	BVH_Y_POSITION,
	BVH_Z_POSITION,
	BVH_X_ROTATION,
	BVH_Y_ROTATION,
	BVH_Z_ROTATION,
};


void armature_LoadBVH(char *file_name, armature_t *armature, animation_t *animation)
{
	char *fstr;
	int i = 0;
	int j;
	file_t f;
	int index;
	char str[128];
	char *bone_name;
	int channel_count = 0;
	int channels;
	//int bone_count = 0;
	bone_t *b = NULL;
	
	mat3_t r = mat3_t_id();
	vec3_t t;
	
	int state;
	
	f = file_LoadFile(file_name, 0);
	fstr = f.buf;
	
	
	if(!fstr)
	{
		console_Print(MESSAGE_ERROR, "couldn't load BHV file [%s]!", file_name);
		//armature = NULL;
		//animation = NULL;
		return;
	}
	
	while(fstr[i] != '\0')
	{
		if(fstr[i] 	   == 'H' &&
		   fstr[i + 1] == 'I' &&
		   fstr[i + 2] == 'E' &&
		   fstr[i + 3] == 'R' &&
		   fstr[i + 4] == 'A' &&
		   fstr[i + 5] == 'R' &&
		   fstr[i + 6] == 'C' &&
		   fstr[i + 7] == 'H' &&
		   fstr[i + 8] == 'Y')
		{
			i += 9;
			armature_GetBone(&b, fstr, &i, &channel_count);
			armature_CreateArmDef(b, NULL);
			
		}
		else if(fstr[i] == 	   'M' &&
				fstr[i + 1] == 'O' &&
				fstr[i + 2] == 'T' &&
				fstr[i + 3] == 'I' &&
				fstr[i + 4] == 'O' &&
				fstr[i + 5] == 'N')
		{
			i += 6;
		}
		else
		{
			i++;
		}
	}
	
	free(fstr);
	return;
}

void armature_GetBone(bone_t **parent, char *istr, int *cur_pos, int *channel_count)
{
	int i = 0;
	int j;
	int index;
	char str[128];
	bone_t *b;
	//char *fstr = istr + *cur_pos;
	char *fstr = istr;
	char *bone_name;
	int c_count;
	int channels;
	int root = 0;
	int end = 0;
	static int bone_id = 0;
	
	mat3_t r = mat3_t_id();
	vec3_t t;
	while(fstr[*cur_pos] != '\0')
	{
		
		if((fstr[*cur_pos] 	   	 == 'J' || fstr[*cur_pos] 		== 'R') &&
		    fstr[(*cur_pos) + 1] == 'O' && 
		   (fstr[(*cur_pos) + 2] == 'I' || fstr[(*cur_pos) + 2] == 'O') &&
		   (fstr[(*cur_pos) + 3] == 'N' || fstr[(*cur_pos) + 3] == 'T'))
		   //str[i + 4] == 'T')
		{
			(*cur_pos) += 4;
			if(fstr[*cur_pos] == 'T') (*cur_pos)++;
			else
			{
				root = 1;
				//bone_id = 0;
			}
			
			while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
			index = 0;
			if(fstr[*cur_pos] != '{')
			{
				while(fstr[*cur_pos] != ' ' && fstr[*cur_pos] != '\n' && fstr[*cur_pos] != '\t' && fstr[*cur_pos] != '\0') 
				{
					str[index++] = fstr[(*cur_pos)++];
				}
				str[index] = '\0';
			}
			while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
				
			if(fstr[*cur_pos] != '{')
			{
				 //error... missing left curly brace...
			}
			(*cur_pos)++;
			bone_name = strdup(str);
				
			while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
			
			_end_site_offset:
				
			if(fstr[(*cur_pos)]     == 'O' &&
			   fstr[(*cur_pos) + 1] == 'F' &&
			   fstr[(*cur_pos) + 2] == 'F' &&
			   fstr[(*cur_pos) + 3] == 'S' &&
			   fstr[(*cur_pos) + 4] == 'E' &&
			   fstr[(*cur_pos) + 5] == 'T')
			{
				*cur_pos += 6;
					
				for(j = 0; j < 3; j++)
				{
					while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
					index = 0;
					while(fstr[*cur_pos] != ' ' && fstr[*cur_pos] != '\n' && fstr[*cur_pos] != '\t')
					{
						str[index++] = fstr[(*cur_pos)++];
					}
					str[index] = '\0';
					t.floats[j] = atof(str);
				}
				
				if(end)
				{
					(*parent)->tip = t;
					while(fstr[*cur_pos] != '}')(*cur_pos)++;
					(*cur_pos)++;
					continue;
				}
			}
				
			while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
				
			if(fstr[*cur_pos] 	  == 'C' &&
			   fstr[(*cur_pos) + 1] == 'H' &&
			   fstr[(*cur_pos) + 2] == 'A' &&
			   fstr[(*cur_pos) + 3] == 'N' &&
			   fstr[(*cur_pos) + 4] == 'N' &&
			   fstr[(*cur_pos) + 5] == 'E' &&
			   fstr[(*cur_pos) + 6] == 'L' &&
			   fstr[(*cur_pos) + 7] == 'S')
			{
				(*cur_pos) += 8;
				while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
				index = 0;
				while(fstr[*cur_pos] != ' ' && fstr[*cur_pos] != '\n' && fstr[*cur_pos] != '\t' && fstr[*cur_pos] != '\0') 
				{
					str[index++] = fstr[(*cur_pos)++];
				}
				str[index] = '\0';
					
				c_count = atoi(str);
				*channel_count += c_count;
				channels = 0;
				for(j = 0; j < c_count; j++)
				{
					while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
					index = 0;
					while(fstr[*cur_pos] != ' ' && fstr[*cur_pos] != '\n' && fstr[*cur_pos] != '\t' && fstr[*cur_pos] != '\0') 
					{
						str[index++] = fstr[(*cur_pos)++];
					}
					str[index] = '\0';
					
					channels <<= 4;
					
					if(!strcmp(str, "Xposition"))
					{
						channels |= BVH_X_POSITION;						
					}
					else if(!strcmp(str, "Yposition"))
					{
						channels |= BVH_Y_POSITION;	
					}
					else if(!strcmp(str, "Zposition"))
					{
						channels |= BVH_Z_POSITION;	
					}
					else if(!strcmp(str, "Xrotation"))
					{
						channels |= BVH_X_ROTATION;	
					}
					else if(!strcmp(str, "Yrotation"))
					{
						channels |= BVH_Y_ROTATION;	
					}
					else if(!strcmp(str, "Zrotation"))
					{
						channels |= BVH_Z_ROTATION;	
					}
						
				}
					
			}
				
			b = armature_CreateBone(bone_name, t, &r);
			//*cur_pos += i;
			
			if(root)
			{
				*parent = b;
				//b->bone_id = bone_id;
				//bone_id++;
				
				armature_GetBone(parent, istr, cur_pos, channel_count);
			}
			else
			{
				(*parent)->tip = t;	/* this could be set just once... */
				armature_GetBone(&b, istr, cur_pos, channel_count);
				armature_AddBoneChild(parent, b);	
			}
			
			//i = *cur_pos;
			
			
		}
		else if(fstr[(*cur_pos)] 	 == 'E' &&
				fstr[(*cur_pos) + 1] == 'n' &&
				fstr[(*cur_pos) + 2] == 'd' &&
				fstr[(*cur_pos) + 3] == ' ' &&
				fstr[(*cur_pos) + 4] == 'S' &&
				fstr[(*cur_pos) + 5] == 'i' &&
				fstr[(*cur_pos) + 6] == 't' &&
				fstr[(*cur_pos) + 7] == 'e')
		{
			(*cur_pos) += 8;
			while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
			if(fstr[*cur_pos] != '{')
			{
				/* error: missing left curly brace... */
			}
			(*cur_pos)++;
			while(fstr[*cur_pos] == ' ' || fstr[*cur_pos] == '\n' || fstr[*cur_pos] == '\t')(*cur_pos)++;
			end = 1;
			goto _end_site_offset;
			 
			break;
		}
		else if(fstr[*cur_pos] == '}')
		{
			(*cur_pos)++;
			break;
		}
		else
		{
			(*cur_pos)++;
		}
	}
	
	return;
}

void armature_TransformVertex(vec4_t *v, mat4_t *transform, float weight)
{

	
}

PEWAPI animation_t *armature_GetAnimation(char *name)
{
	int i;
	int c = animation_list.count;
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, animation_list.animations[i].name))
		{
			return &animation_list.animations[i];
		}
	}
	return NULL;
}

PEWAPI int armature_GetAnimationIndex(char *name)
{
	int i;
	int c = animation_list.count;
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, animation_list.animations[i].name))
		{
			return i;
		}
	}
	return -1;
}

PEWAPI int armature_TestDuplicateBoneChain(bone_t *b)
{
	int i;
	int c = armature_defs.count;
	
	for(i = 0; i < c; i++)
	{
		if(!strcmp(b->name, armature_defs.armdefs[i].name))
		{
			return 1;
		}
	}
	return 0;
}

PEWAPI armature_t *armature_GetArmature(char *name)
{
	int i;
	int c = armature_list.count;
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, armature_list.armatures[i].name))
		{
			return &armature_list.armatures[i];
		}
	}
	return NULL;
}

PEWAPI armature_t *armature_GetArmatureByIndex(int index)
{
	if(index > -1 && index < armature_list.count)
	{
		return &armature_list.armatures[index];
	}
	return NULL;
}

PEWAPI armdef_t *armature_GetArmDef(char *name)
{
	register int i;
	int c = armature_defs.count;
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, armature_defs.armdefs[i].name))
		{
			return &armature_defs.armdefs[i];
		}
	}
	return NULL;
}

PEWAPI armdef_t *armature_GetArmDefByIndex(int index)
{
	if(index > -1 && index < armature_defs.count)
	{
		return &armature_defs.armdefs[index];
	}
	return NULL;
}

PEWAPI int armature_GetArmDefIndex(char *name)
{
	register int i;
	int c = armature_defs.count;
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, armature_defs.armdefs[i].name))
		{
			return i;
		}
	}
	return -1;
}

PEWAPI int armature_GetArmatureIndex(char *name)
{
	register int i;
	int c = armature_list.count;
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, armature_list.armatures[i].name))
		{
			return i;
		}
	}
	return -1;
}

PEWAPI void armature_PlayAnimation(armature_t *armature, int animation_index)
{
	if(animation_index > -1)
	{
		armature->c0 = animation_index;
		armature->flags |= ARMATURE_PLAYING;
	}
	else
	{
		printf("invalid animation index\n");
	}
	
}

PEWAPI int armature_test_GenerateAnimation(int bone_count, float fps, int frame_count)
{
	float ao = 5.0 / (float)bone_count;
	float ai = 6.3 / (float)frame_count;
	float angle = 0.0;
	int i;
	int j;
	animation_t a;
	mat3_t r;
	
	a.fps = fps;
	a.frame_count = frame_count;
	a.frame_size = bone_count;
	a.frames = (aframe_t *)malloc(sizeof(aframe_t) * bone_count * frame_count);
	
	for(i = 0; i < frame_count; i++)
	{/*
		
		for(j = 0; j < bone_count; j++)
		{
			mat3_t_rotate(&r, vec3(1.0, 0.0, 0.0), sin(angle + ao * j) * 0.1, 1);
			a.frames[i * bone_count + j].rotation = r;
			a.frames[i * bone_count + j].position = vec3(0.0, 0.0, 0.0);
		}
		angle += ai;*/
	}
	
	a.name = "_test_";
	
	animation_list.animations[animation_list.count++] = a;
	return animation_list.count - 1;
}




/*PEWAPI void armature_SkinMesh(mesh_t *mesh, bone_t *armature)
{
	
}*/















