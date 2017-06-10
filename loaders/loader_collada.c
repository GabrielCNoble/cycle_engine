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

extern int mesh_path_len;
extern char mesh_path[256];

/* maybe this should go to the file sub-system? */


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
	
	printf("%s\n", element.Name());

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


PEWAPI mesh_t loader_LoadCollada(char *file_name)
{
	char full_path[128];
	FILE *f;
	mesh_t mesh;
	tinyxml2 :: XMLDocument d;
	visitor v;
	strcpy(full_path, mesh_path);
	strcat(full_path, file_name);
	
	if((d.LoadFile(full_path)))
	{
		console_Print(MESSAGE_ERROR, "couldn't load file [%s]!", file_name);
		mesh.v_data = NULL;
		mesh.vert_count = 0;
		return mesh;
	}
	
	d.Accept(&v);
	
	resolve(&v, NULL, NULL, NULL, NULL, NULL);
	
	mesh.v_data = NULL;
	
	return mesh;
}












