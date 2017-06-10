#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader_collada.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision\CollisionShapes\btShapeHull.h"
#include "vector.h"
#include "matrix.h"
#include "console.h"
#include "tinyxml2.h"
#include "file.h"
#include "scene.h"
#include "armature.h"

extern int mesh_path_len;
extern char mesh_path[256];

/* maybe this should go to the file sub-system? */

static int unnamed_animation_len = 0;
static char *unnamed_animation = NULL;
using namespace tinyxml2;

enum VISITOR_STATE
{
	VISITOR_NONE = 0,
	VISITOR_ENTER_GEOMETRY,
	VISITOR_EXIT_GEOMETRY,
	VISITOR_ENTER_SOURCE,
	VISITOR_EXIT_SOURCE,
	VISITOR_ENTER_FLOAT_ARRAY,
	VISITOR_EXIT_FLOAT_ARRAY,
	VISITOR_ENTER_VERTICES,
	VISITOR_EXIT_VERTICES,
	VISITOR_ENTER_POLYLIST,
	VISITOR_EXIT_POLYLIST,
	VISITOR_ENTER_INPUT,
	VISITOR_EXIT_INPUT,
	VISITOR_ENTER_VCOUNT,
	VISITOR_EXIT_VCOUNT,
	VISITOR_ENTER_P,
	VISITOR_EXIT_P,
	VISITOR_ENTER_MESH,
	VISITOR_EXIT_MESH
};

enum DNODE_TYPE
{
	DNODE_NONE = 0,
	DNODE_FLOAT_ARRAY,
	DNODE_SOURCE,
	DNODE_ACCESSOR,
	DNODE_VERTICES,
	DNODE_INPUT,
	DNODE_GEOMETRY,
	DNODE_MESH,
	DNODE_POLYLIST,
	DNODE_VCOUNT,
	DNODE_P
};


typedef struct float_data
{
	int count;
	float *data;
	char *name;
	struct float_data *next;
}float_data;

/* base node */
typedef struct dnode_t
{
	char *name;
	int type;
	struct dnode_t *next;
	struct dnode_t *prev;
}dnode_t;

/* float array node */
typedef struct fanode_t
{
	char *name;
	int type;
	struct dnode_t *next;
	struct dnode_t *prev;
	/****************/
	int count;
	float *data;
}fanode_t;

/* source node */
typedef struct snode_t
{
	char *name;
	int type;
	struct dnode_t *next;
	struct dnode_t *prev;
	/****************/
	char *src;
}snode_t;

/* input node */
typedef struct inode_t
{
	char *name;
	int type;
	struct dnode_t *next;
	struct dnode_t *prev;
	/****************/
	int offset;
	char *semantic;
	char *src;
}inode_t;

/* polylist node */
typedef struct plnode_t
{
	char *name;
	int type;
	struct dnode_t *next;
	struct dnode_t *prev;
	/****************/
	int input_count;
	int count;
	char *material;
}plnode_t;

/* vcount node */
typedef struct vcnode_t
{
	char *name;
	int type;
	struct dnode_t *next;
	struct dnode_t *prev;
	/****************/
	int vert_count;
	int count;
	int *data;
}vcnode_t;

/* p node */
typedef struct pnode_t
{
	char *name;
	int type;
	struct dnode_t *next;
	struct dnode_t *prev;
	/****************/
	int count;
	int *data;
}pnode_t;


using namespace tinyxml2;

class visitor : public XMLVisitor
{
	public:
	
	dnode_t *droot;
	dnode_t *dlast;
	dnode_t *curmsh;
	plnode_t *curpln;
	vcnode_t *curvcn;
	
	
	int state;
	
	visitor();
	
	~visitor();
	
	bool VisitEnter( const XMLElement& element, const XMLAttribute* first_attrib);
	
	bool VisitExit( const XMLElement& element);
};



visitor :: visitor()
{

	droot = (dnode_t *)malloc(sizeof(dnode_t));
	droot->name = strdup("root");
	droot->type = DNODE_NONE;
	droot->next = NULL;
	dlast = droot;
	curpln = NULL;
	curvcn = NULL;
	state = VISITOR_NONE;
}
	
visitor :: ~visitor()
{
	
	while(droot)
	{
		dlast = droot->next;
		switch(droot->type)
		{
			case DNODE_GEOMETRY:
				free(droot->name);
			break;
			
			case DNODE_SOURCE:
				free(droot->name);
				free(((snode_t*)droot)->src);
			break;
			
			case DNODE_FLOAT_ARRAY:
				free(droot->name);
				free(((fanode_t *)droot)->data);
			break;
			
			case DNODE_INPUT:
				free(droot->name);
				free(((inode_t *)droot)->src);
				free(((inode_t *)droot)->semantic);
			break;
			
			case DNODE_POLYLIST:
				free(droot->name);
				free(((plnode_t *)droot)->material);
			break;
			
			case DNODE_VCOUNT:
				free(droot->name);
				free(((vcnode_t *)droot)->data);
			break;
			
			case DNODE_P:
				free(droot->name);
				free(((pnode_t *)droot)->data);
			break;
		}
		free(droot);
		droot = dlast;
	}
}


bool visitor :: VisitEnter( const XMLElement& element, const XMLAttribute* first_attrib)
{

	char *dstr;
	int index;
	int *t;
	char str[128];
	float_data *q;
	int i;
	int j;
	XMLAttribute *a;
	dnode_t *dn;
	//gnode_t *gn;
	snode_t *sn;
	fanode_t *fn;
	//vnode_t *vn;
	inode_t *in;
	plnode_t *pln;
	vcnode_t *vcn;
	pnode_t *pn;
	
	//printf("%s\n", element.Name());

	if(!strcmp(element.Name(), "source"))
	{
		state = VISITOR_ENTER_SOURCE;
	}
	else if(!strcmp(element.Name(), "mesh"))
	{
		state = VISITOR_ENTER_MESH;
	}
	else if(!strcmp(element.Name(), "float_array"))
	{
		state = VISITOR_ENTER_FLOAT_ARRAY;
	}
	else if(!strcmp(element.Name(), "vertices"))
	{
		state = VISITOR_ENTER_VERTICES;
	}
	else if(!strcmp(element.Name(), "input"))
	{
		state = VISITOR_ENTER_INPUT;
	}
	else if(!strcmp(element.Name(), "polylist"))
	{
		state = VISITOR_ENTER_POLYLIST;
	}
	else if(!strcmp(element.Name(), "vcount"))
	{
		state = VISITOR_ENTER_VCOUNT;
	}
	else if(!strcmp(element.Name(), "p"))
	{
		state = VISITOR_ENTER_P;
	}
	
		
	switch(state)
	{
		case VISITOR_ENTER_MESH:
			dn = (dnode_t *)malloc(sizeof(dnode_t));
			dn->next = NULL;
			dn->prev = NULL;
			dn->name = strdup("mesh");
			dn->type = DNODE_MESH;
			
			if(!droot)
			{
				droot = dn;
				dlast = dn;
			}
			else
			{
				dlast->next = dn;
				dn->prev = dlast;
				dlast = dn;
			}
		break;
		
		case VISITOR_ENTER_SOURCE:
			sn = (snode_t *)malloc(sizeof(snode_t));
			a = (XMLAttribute *) first_attrib;
			while(a && strcmp("id", a->Name()))
			{
				a = (XMLAttribute *)a->Next();
			}
			if(a)
			{
				sn->name = strdup(a->Value());
			}
			sn->next = NULL;
			sn->type = DNODE_SOURCE;
			dlast->next = (dnode_t *)sn;
			sn->prev = dlast;
			dlast = (dnode_t *)sn;
		break;
			
		case VISITOR_ENTER_FLOAT_ARRAY:
			fn = (fanode_t *)malloc(sizeof(fanode_t));
			a = (XMLAttribute *) first_attrib;
			while(a)
			{
				if(!strcmp(a->Name(), "id"))
				{
					fn->name = strdup(a->Value());
				}
				else if(!strcmp(a->Name(), "count"))
				{
					fn->count = a->IntValue();
				}
				a = (XMLAttribute *)a->Next();
			}
				
			dstr = (char *)element.GetText();
			fn->data = (float *)malloc(sizeof(float) * fn->count);
			
			j = 0;
			for(i = 0; i < fn->count; i++)
			{
				index = 0;
				while(dstr[j] == ' ') j++;
				while(dstr[j] != '\0' && dstr[j] != ' ')
				{
					str[index++] = dstr[j++];
				}
				str[index] = '\0';
				fn->data[i] = atof(str);
			}
			
			fn->next = NULL;
			fn->type = DNODE_FLOAT_ARRAY;
			dlast->next = (dnode_t *)fn;
			fn->prev = dlast;
			dlast = (dnode_t *)fn;
		break;
		
		case VISITOR_ENTER_VERTICES:
			dn = (dnode_t *)malloc(sizeof(dnode_t));
			a = (XMLAttribute *) first_attrib;
			while(a && strcmp("id", a->Name()))
			{
				a = (XMLAttribute *)a->Next();
			}
			if(a)
			{
				dn->name = strdup(a->Value());
			}
			dn->next = NULL;
			dn->type = DNODE_VERTICES;
			dlast->next = dn;
			dn->prev = dlast;
			dlast = dn;
		break;
		
		case VISITOR_ENTER_INPUT:
			in = (inode_t *)malloc(sizeof(inode_t));
			in->name = strdup("input");
			in->semantic = NULL;
			in->offset = -1;
			in->src = NULL;
			
			a = (XMLAttribute *) first_attrib;
			while(a)
			{
				if(!strcmp(a->Name(), "semantic"))
				{
					in->semantic = strdup(a->Value());
				}
				else if(!strcmp(a->Name(), "offset"))
				{
					in->offset = a->IntValue();
				}
				else if(!strcmp(a->Name(), "source"))
				{
					in->src = strdup(a->Value());
				}
				a = (XMLAttribute *)a->Next();
			}
			in->type = DNODE_INPUT;
			in->next = NULL;
			
			dlast->next = (dnode_t *)in;
			in->prev = dlast;
			dlast = (dnode_t *)in;
			if(curpln)
				curpln->input_count++;
		break;
		
		case VISITOR_ENTER_POLYLIST:
			pln = (plnode_t *)malloc(sizeof(plnode_t));
			pln->name = strdup("polylist");
			pln->material = NULL;
			pln->count = -1;
			a = (XMLAttribute *) first_attrib;
			while(a)
			{
				if(!strcmp(a->Name(), "material"))
				{
					pln->material = strdup(a->Value());
				}
				else if(!strcmp(a->Name(), "count"))
				{
					pln->count = a->IntValue();
				}

				a = (XMLAttribute *)a->Next();
			}
			pln->type = DNODE_POLYLIST;
			pln->next = NULL;
			
			dlast->next = (dnode_t *)pln;
			pln->prev = dlast;
			dlast = (dnode_t *)pln;
			pln->input_count = 0;
			curpln = pln;
		break;
		
		case VISITOR_ENTER_VCOUNT:
			
			vcn = (vcnode_t *)malloc(sizeof(vcnode_t));
			vcn->count = curpln->count;
			vcn->name = strdup("vcount");
			vcn->vert_count = 0;
			vcn->next = NULL;
			vcn->data = (int *)malloc(sizeof(int) * vcn->count);
			dstr = (char *)element.GetText();
			
			j = 0;
			for(i = 0; i < vcn->count; i++)
			{
				index = 0;
				while(dstr[j] == ' ') j++;
				while(dstr[j] != ' ' && dstr[j] != '\0')
				{
					str[index++] = dstr[j++];
				}
				str[index] = '\0';
				vcn->data[i] = atoi(str);
				vcn->vert_count += vcn->data[i];
			}
			vcn->type = DNODE_VCOUNT;
			
			dlast->next = (dnode_t *)vcn;
			vcn->prev = dlast;
			dlast = (dnode_t *)vcn;
			
			curvcn = vcn;
			
		break;
		
		case VISITOR_ENTER_P:
			pn = (pnode_t *)malloc(sizeof(pnode_t));
			pn->count = 0;
			pn->name = strdup("p");
			pn->data = NULL;
			pn->next = NULL;
			dstr = (char *)element.GetText();
			
			j = 0;
			while(dstr[j] != '\0')
			{
				index = 0;
				while(dstr[j] == ' ') j++;
				if(dstr[j] == '\0') break;
				while(dstr[j] != ' ' && dstr[j] != '\0')
				{
					str[index++] = dstr[j++];
				}
				str[index] = '\0';
				
				/* uhg... */
				t = (int *)malloc(sizeof(int) * (pn->count + 1));
				if(!pn->data)
				{
					pn->data = t;	
				}
				else
				{
					/* uuhhhg... */
					memcpy(t, pn->data, sizeof(int) * pn->count);
					/* >:( */
					free(pn->data);
					pn->data = t;
				}
				pn->data[pn->count] = atoi(str);
				pn->count++;

			}
			pn->type = DNODE_P;
			
			dlast->next = (dnode_t *)pn;
			pn->prev = dlast;
			dlast = (dnode_t *)pn;
		break;
	}
	state = VISITOR_NONE;
	return true;
}


bool visitor :: VisitExit(const XMLElement& element)
{
	if(!strcmp(element.Name(), "polylist"))
	{
		curpln = NULL;
	}
	else if(!strcmp(element.Name(), "float_array"))
	{
		curmsh = NULL;
	}

	return true;
} 


void resolve(visitor *v, float **v_data, float **n_data, float **t_data, float **t_c_data, int *vert_count)
{
	dnode_t *root = v->droot;
	dnode_t *d = root;
	dnode_t *q;
	dnode_t *t;
	dnode_t *curmsh;
	plnode_t *curpln;
	vcnode_t *curvcn;
	char *source;
	
	fanode_t *vd = NULL;
	fanode_t *nd = NULL;
	fanode_t *tcd = NULL;
	
	//float *vd = NULL;
	int voffset;
	//float *nd = NULL;
	int noffset;
	//float *t_cd = NULL;
	int tcoffset;
	int *indexes;
	int count;
	int remove = 0;
	int input = 0;
	int size;
	
	
	while(d)
	{
		if(d->type == DNODE_MESH)
		{
			curmsh = d;
			d = d->next;
			input = 0;
			
			while(d)
			{
				if(d->type == DNODE_POLYLIST)
				{
					curpln = (plnode_t *)d;
					break;
				}
				d = d->next;
			}
			
			while(d)
			{
				switch(d->type)
				{
					case DNODE_SOURCE:
						
					break;
					
					case DNODE_FLOAT_ARRAY:
						switch(input)
						{
							case 1:
								vd = ((fanode_t *)d);
								d = (dnode_t *)curpln;
								input = 0;
							break;	
							
							case 2:
								nd = ((fanode_t *)d);
								d = (dnode_t *)curpln;
								input = 0;
							break;
							
							/* TODO: Increase robustness. This should
							be able to handle multiple tex_coord sets
							per mesh... */
							case 3:
								tcd = ((fanode_t *)d);
								d = (dnode_t *)curpln;
								input = 0;
							break;
						}
					break;
					
					case DNODE_INPUT:
						
						if(!strcmp(((inode_t *)d)->semantic, "VERTEX"))
						{
							voffset = ((inode_t *)d)->offset;
							input = 1;
							remove = 1;
						}
						else if(!strcmp(((inode_t *)d)->semantic, "NORMAL"))
						{
							noffset = ((inode_t *)d)->offset;
							input = 2;
							remove = 1;
						}
						else if(!strcmp(((inode_t *)d)->semantic, "TEXCOORD"))
						{
							tcoffset = ((inode_t *)d)->offset;
							input = 3;
							remove = 1;
						}
						if(input)
						{
								
							q = curmsh->next;
							source = ((inode_t *)d)->src;
							if(source[0] == '#') source++;
							
							while(q)
							{
								if(!strcmp(q->name, source))
								{
									break;
								}
								q = q->next;
							}
							
							if(remove)
							{
								d->next->prev = d->prev;
								d->prev->next = d->next;
								
								free(d->name);
								free(((inode_t *)d)->src);
								free(((inode_t *)d)->semantic);
								free(d);
								remove = 0;
							}
						
							d = q;
							continue;	
						}
					break;	
					
					case DNODE_VCOUNT:
						curvcn = (vcnode_t *)d;
					break;
				
				}
				d = d->next;
			}
			if(!d) break;
		}
		d = d->next;
	}
}

int build_armature(aiNode *root, aiBone **bone, int bone_count)
{
	int i;
	int c;
	int j;
	int k;
	int m;
	int stack_top = 0;
	int b_new_root = 0;
	int children[64];
	int wcount = 0;
	aiNode *q;
	aiNode *f;
	bone_t *r = NULL;
	bone_t *b;
	bone_t *parent;
	mat3_t o = mat3_t_id();
	vec3_t p;
	wset_t set;
	char new_root[64];
	q = root;
	children[stack_top] = 0;
	//while(q)
	
	
	c = bone_count;
	
	for(i = 0; i < c; i++)
	{
		wcount += bone[i]->mNumWeights;
		
	}
	
	if(wcount)
	{
		set.weights = (weight_t *)malloc(sizeof(weight_t ) * wcount);
		set.count = wcount;
		set.size = wcount;
	}
	
	q = root->FindNode(bone[0]->mName.C_Str());
	
	j = 0;
	i = 0;
	c = 0;
	m = 0;
	do
	{
		if(!children[stack_top])
		{
			/* no rotation is read because in this engine
			the orientation of the bones on bind pose
			is assumed to be the identity... */

			m++;
			p.floats[0] = q->mTransformation.a4;
			p.floats[1] = q->mTransformation.b4;
			p.floats[2] = q->mTransformation.c4;
			
			b = armature_CreateBone((char *)q->mName.C_Str(), p, &o);
			//printf("%s\n", q->mName.C_Str());
			
			c += bone[j]->mNumWeights;
			k = i;
			for(;i < c; i++)
			{
				set.weights[i].global_id = j;
				set.weights[i].index = bone[j]->mWeights[i - k].mVertexId;
				set.weights[i].weight = bone[j]->mWeights[i - k].mWeight;
			}
			j++;
			
			//b->wset_offset = k;
			//b->wcount = i - k;
			
			
			if(!r)
			{
				r = b;
				parent = r;
			}
			else
			{
				armature_AddBoneChild(&parent, b);
				parent->tip = b->position;
				parent = b;
			}
			
		}
		if(children[stack_top] < q->mNumChildren)
		{
			q = q->mChildren[children[stack_top]];
			children[stack_top]++;
			stack_top++;
			children[stack_top] = 0;
		}
		else
		{
			q = q->mParent;
			parent = parent->parent;
			stack_top--;
		}
		
		if(!parent)
		{
			if(m < bone_count)
			{
				q = root->FindNode(bone[m]->mName.C_Str());
				if(!b_new_root)
				{
					strcpy(new_root, r->name);
					strcat(new_root, ".root");
					b = armature_CreateBone(new_root, vec3(0.0, 0.0, 0.0), &o);
					armature_AddBoneChild(&b, r);
					r = b;
					children[0] = 1;
					b_new_root = 1;
				}
				children[0]++;
				stack_top = 1;
				children[stack_top] = 0;
				parent = r;
			}
		}
		
	}while(parent);
	
	//if(!armature_TestDuplicateBoneChain(r))
	{
		return armature_CreateArmDef(r, &set);
	}
	printf("duplicated bone chain would yeld duplicated armdef! deleting...\n");
	if(wcount) free(set.weights);
	armature_DeleteBones(r);
	
	return -1;
}


PEWAPI mesh_t loader_LoadCollada(char *file_name)
{
	struct aiScene *s;// = (aiScene*)aiImportFile(file_name, 0);
	mesh_t mesh;
	int i;
	int j;
	int k;
	int anim_byte_count;
	int h;
	int a_index;
	aiMesh *m;// = s->mMeshes[0]; 
	aiFace *f;
	aiNode *n;
	aiNode *p;
	bone_t *b;
	armdef_t *q;
	aiAnimation *a;
	animation_t an;
	int size;
	btConvexHullShape *chtemp;
	btShapeHull *htemp;
	
	if(!unnamed_animation)
	{
		unnamed_animation = strdup("unnamed_animation.0000");
		unnamed_animation_len = strlen(unnamed_animation);
	}
	
	char full_path[256];
	strcpy(full_path, mesh_path);
	strcat(full_path, file_name);
	
	s = (aiScene*)aiImportFile(full_path, 0);
	
	m = s->mMeshes[0];
	
	size = sizeof(float) * 3 * m->mNumVertices;
	
	if(m->mNormals)
	{
		size += size;
	}
	if(m->mTextureCoords[0])
	{
		size += sizeof(float) * 2 * m->mNumVertices + sizeof(float) * 3 * m->mNumVertices;
	}
	
	mesh.v_data = (float *)malloc(size);
	mesh.n_data = NULL;
	mesh.t_data = NULL;
	mesh.t_c_data = NULL;
	
	k = 0;
	for(i = 0; i < m->mNumFaces; i++)
	{
		f = &m->mFaces[i];
		for(j = 0; j < f->mNumIndices; j++)
		{
			mesh.v_data[k * 3] = m->mVertices[f->mIndices[j]].x;
			mesh.v_data[k * 3 + 1] = m->mVertices[f->mIndices[j]].y;
			mesh.v_data[k * 3 + 2] = m->mVertices[f->mIndices[j]].z;
			k++;
		}
	}
	if(m->mNormals)
	{
		mesh.n_data = mesh.v_data + m->mNumVertices * 3;
		k = 0;
		for(i = 0; i < m->mNumFaces; i++)
		{
			f = &m->mFaces[i];
			for(j = 0; j < f->mNumIndices; j++)
			{
				mesh.n_data[k * 3] = m->mNormals[f->mIndices[j]].x;
				mesh.n_data[k * 3 + 1] = m->mNormals[f->mIndices[j]].y;
				mesh.n_data[k * 3 + 2] = m->mNormals[f->mIndices[j]].z;
				k++;
			}
		}
	}
	if(m->mTextureCoords[0])
	{
		mesh.t_data = mesh.n_data + m->mNumVertices * 3;
		mesh.t_c_data = mesh.t_data + m->mNumVertices * 3;
		
		k = 0;
		for(i = 0; i < m->mNumFaces; i++)
		{
			f = &m->mFaces[i];
			for(j = 0; j < f->mNumIndices; j++)
			{
				mesh.t_c_data[k * 2] = m->mTextureCoords[0][f->mIndices[j]].x;
				mesh.t_c_data[k * 2 + 1] = m->mTextureCoords[0][f->mIndices[j]].y;
				k++;
			}
		}

		model_CalculateTangents(mesh.v_data, mesh.t_c_data, mesh.n_data, &mesh.t_data, m->mNumVertices);
	}
	/*else
	{
		mesh.t_data = NULL;
		mesh.t_c_data = NULL;
	}*/
	mesh.vert_count = m->mNumVertices;
	mesh.draw_mode = GL_TRIANGLES;
	mesh.flags = 0;
	
	
	if(m->mNumBones)
	{
		n = s->mRootNode;
		//p = n->FindNode(m->mBones[0]->mName.C_Str());
		
		if(p)
		{
			a_index = build_armature(n, m->mBones, m->mNumBones);
		}
	}
	
	if(s->mAnimations)
	{
		for(i = 0; i < s->mNumAnimations; i++)
		{
			a = s->mAnimations[i];		
			
			if(!a->mName.length)
			{
				an.name = strdup(unnamed_animation);
				if(unnamed_animation[unnamed_animation_len - 1] == '9')
				{
					unnamed_animation[unnamed_animation_len - 1] = '0';
					
					if(unnamed_animation[unnamed_animation_len - 2] == '9')
					{
						unnamed_animation[unnamed_animation_len - 2] = '0';
						
						if(unnamed_animation[unnamed_animation_len - 3] == '9')
						{
							unnamed_animation[unnamed_animation_len - 3] = '0';
							
							if(unnamed_animation[unnamed_animation_len - 4] == '9')
							{
								unnamed_animation[unnamed_animation_len - 4] = '0';
							}
							else
							{
								unnamed_animation[unnamed_animation_len - 4]++;
							}
						}
						else
						{
							unnamed_animation[unnamed_animation_len - 3]++;
						}
					}
					else
					{
						unnamed_animation[unnamed_animation_len - 2]++;
					}
				}
				else
				{
					unnamed_animation[unnamed_animation_len - 1]++;
				}
			}
			
			anim_byte_count = 0;
			if(!a->mTicksPerSecond) an.fps = 24.0;
			else an.fps = a->mTicksPerSecond;
			
			
			h = 0;
			for(j = 0; j < a->mNumChannels; j++)
			{
				if(a->mChannels[j]->mNumRotationKeys > h) h = a->mChannels[j]->mNumRotationKeys;
				if(a->mChannels[j]->mNumPositionKeys > h) h = a->mChannels[j]->mNumPositionKeys; 
			}

			an.frames = (aframe_t *)malloc(sizeof(aframe_t) * h * a->mNumChannels);
			an.frame_count = h;
			an.frame_size = a->mNumChannels;
			an.duration = a->mDuration * 1000.0;
			
			for(j = 0; j < a->mNumChannels; j++)
			{		
				for(k = 0; k < h; k++)
				{

					an.frames[k + h * j].position.x = a->mChannels[j]->mPositionKeys[k].mValue.x;
					an.frames[k + h * j].position.y = a->mChannels[j]->mPositionKeys[k].mValue.y;
					an.frames[k + h * j].position.z = a->mChannels[j]->mPositionKeys[k].mValue.z;
	
					an.frames[k + h * j].rotation.x = a->mChannels[j]->mRotationKeys[k].mValue.x;
					an.frames[k + h * j].rotation.y = a->mChannels[j]->mRotationKeys[k].mValue.y;
					an.frames[k + h * j].rotation.z = a->mChannels[j]->mRotationKeys[k].mValue.z;
					an.frames[k + h * j].rotation.w = a->mChannels[j]->mRotationKeys[k].mValue.w;
					an.frames[k + h * j].time = a->mChannels[j]->mRotationKeys[k].mTime * 1000.0;

				}
			}

			armature_StoreAnimation(&an);
		}
	}
	
	aiReleaseImport(s);
	
	chtemp = new btConvexHullShape(mesh.v_data, mesh.vert_count, sizeof(float) * 3);
	htemp = new btShapeHull(chtemp);
	htemp->buildHull(chtemp->getMargin());
	mesh.collision_shape = new btConvexHullShape((btScalar *)htemp->getVertexPointer(), htemp->numVertices(), sizeof(float )*4);
	delete chtemp;
	delete htemp;

	return mesh;
}












