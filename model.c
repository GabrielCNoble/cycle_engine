#include "model.h"
#include "console.h"
#include "macros.h"
#include "vcache.h"
#include "tri.h"
#include "tinyxml2.h"
#include "loader_wavefront.h"
#include "loader_collada.h"

//extern draw_info_array draw_info_a;
mesh_array mesh_a;
int mesh_path_len = 0;
char mesh_path[256];

//int mesh_count = 0;
//mesh_node_t *root;
//mesh_node_t *last;

int vertex_data_count = 0;
vertex_data_t *vdata_root;
vertex_data_t *vdata_last;

#define SKIP_SPACES while(file_str[str_cursor]==' ' || file_str[str_cursor]=='\n' && file_str[str_cursor]!='\0'){str_cursor++;}


enum EXTENTIONS
{
	WAVEFRONT = ('o')|('b'<<8)|('j'<<16),
	COLLADA = ('d')|('a'<<8)|('e'<<16),
};


#ifdef __cplusplus
extern "C"
{
#endif

/*
=============
model_Init
=============
*/
PEWAPI void model_Init(char *path)
{
	
	//root = (mesh_node_t *)malloc(sizeof(mesh_node_t));
	//root->next = NULL;
	//root->mesh.name = NULL;
	//root->mesh.draw_mode = 0;
	//root->mesh.n_data = NULL;
	//root->mesh.t_c_data = NULL;
	//root->mesh.t_data = NULL;
	//root->mesh.v_data = NULL;
	//root->mesh.vert_count = 0;
	//last = root;
	
	strcpy(mesh_path, path);
	mesh_path_len = strlen(mesh_path);
	
	/*vdata_root = (vertex_data_t *)malloc(sizeof(vertex_data_t));
	vdata_root->next = NULL;
	vdata_root->indexes = NULL;
	vdata_root->position = NULL;
	vdata_root->normal = NULL;
	vdata_root->tex_coord = NULL;
	vdata_root->tangent = NULL;
	vdata_last = vdata_root;*/
	
	
	/*draw_info_a.draw_infos=NULL;
	draw_info_a.draw_info_count=0;*/
	mesh_a.meshes=NULL;
	mesh_a.mesh_count=0;
	model_ResizeMeshArray(16);
	//model_ResizeDrawInfoArray(16);
	return;
}


/*
=============
model_Finish
=============
*/
PEWAPI void model_Finish()
{
	//mesh_node_t *n = root;
	//mesh_node_t *p;
	vertex_data_t *t;
	vertex_data_t *q = vdata_root;
	/*while(n)
	{
		p = n->next;
		free(n);
		n = p;
	}*/
	
	
	free(mesh_a.meshes);
	while(q)
	{
		if(q->position) free(q->position);
		if(q->normal) free(q->normal);
		if(q->tangent) free(q->tangent);
		if(q->tex_coord) free(q->tex_coord);
		if(q->edges) free(q->edges);
		if(q->indexes) free(q->indexes);
		if(q->name) free(q->name);
		
		t = q->next;
		free(q);
		q = t;
	}
	//free(draw_info_a.draw_infos);
	//free(mesh_a.meshes);
	/* free(mesh_a.meshes); */
	return;
}



/*
=============
model_ResizeMeshArray
=============
*/
PEWAPI void model_ResizeMeshArray(int new_size)
{
	mesh_t *temp=(mesh_t *)calloc(new_size, sizeof(mesh_t));
	if(mesh_a.meshes)
	{
		memcpy(temp, mesh_a.meshes, sizeof(mesh_t)*mesh_a.mesh_count);
		free(mesh_a.meshes);
	}
	mesh_a.meshes=temp;
	mesh_a.array_size=new_size;
	return;
}


PEWAPI int model_StoreMesh(mesh_t *mesh)
{
	mesh_t temp;
	float *buf=NULL;
	int gpu_buffer_size=0;
	int offset=0;
	int i;
	
	if(mesh)
	{
		if(!mesh->v_data) return -1;
		
		/* make a copy of the vertex data, so user won't bug it down by reusing
		the same variable to create another mesh_t */
		/*temp.name=mesh->name;
		temp.vert_count=mesh->vert_count;
		temp.draw_mode=GL_TRIANGLES;*/
		
		/*temp.v_data=(float *)calloc(mesh->vert_count, sizeof(float)*3);
		memcpy(temp.v_data, mesh->v_data, sizeof(float )*3*mesh->vert_count);*/
		//gpu_buffer_size+=sizeof(float)*3*mesh->vert_count;
		mesh->vcache_slot_id = -1;
		
		//if(mesh->n_data)
		//{
			//temp.n_data=(float *)calloc(mesh->vert_count, sizeof(float)*3);
			//memcpy(temp.n_data, mesh->n_data, sizeof(float )*3*mesh->vert_count);
		//	gpu_buffer_size+=sizeof(float)*3*mesh->vert_count;
		//}
		//else temp.n_data=NULL;
		
		//if(mesh->t_c_data)
		//{
			/*temp.t_c_data=(float *)calloc(mesh->vert_count, sizeof(float)*2);
			memcpy(temp.t_c_data, mesh->t_c_data, sizeof(float)*2);*/
		//	gpu_buffer_size+=sizeof(float)*2*mesh->vert_count;
		//}
		//else temp.t_c_data=NULL;
		
		//if(mesh->t_data)
		//{
			/*temp.t_data=(float *)calloc(mesh->vert_count, sizeof(float)*3);
			memcpy(temp.t_data, mesh->t_data, sizeof(float)*3);*/
		//	gpu_buffer_size+=sizeof(float)*3*mesh->vert_count;
		//}
		//else temp.t_data=NULL;
		
		/*if(mesh->b_data)
		{
			gpu_buffer_size += sizeof(float) * 3 * mesh->vert_count;
		}*/
		
		
		
		//glGenBuffers(1, &mesh->vertex_buffer);
		//glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
		//gpu_BindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
		//glBufferData(GL_ARRAY_BUFFER, gpu_buffer_size, NULL, GL_DYNAMIC_DRAW);
		//buf=(float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		//buf = (float *)gpu_MapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		
		//for(i=0; i<mesh->vert_count; i++)
		//{
		//	buf[i*3]=mesh->v_data[i*3];
		//	buf[i*3+1]=mesh->v_data[i*3+1];
		//	buf[i*3+2]=mesh->v_data[i*3+2];
		//}
		//offset+=mesh->vert_count*3;
		
		//if(mesh->n_data)
		//{
		//	for(i=0; i<mesh->vert_count; i++)
		//	{
		//		buf[offset+i*3]=mesh->n_data[i*3];
		//		buf[offset+i*3+1]=mesh->n_data[i*3+1];
		//		buf[offset+i*3+2]=mesh->n_data[i*3+2];		
		//	}
		//	offset+=mesh->vert_count*3;
		//}
		
		//if(mesh->t_data)
		//{
		//	for(i=0; i<mesh->vert_count; i++)
		//	{
		//		buf[offset+i*3]=mesh->t_data[i*3];
		//		buf[offset+i*3+1]=mesh->t_data[i*3+1];
		//		buf[offset+i*3+2]=mesh->t_data[i*3+2];		
		//	}
		//	offset+=mesh->vert_count*3;
		//}
		
		//if(mesh->t_c_data)
		//{
		//	for(i=0; i<mesh->vert_count; i++)
		//	{
		//		buf[offset+i*2]=mesh->t_c_data[i*2];
		//		buf[offset+i*2+1]=mesh->t_c_data[i*2+1];	
		//	}
		//	offset+=mesh->vert_count*2;
		//}
		
		
		
				
		//gpu_UnmapGPUBuffer(&temp.buffer);
		//glUnmapBuffer(GL_ARRAY_BUFFER);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//mesh->flags=0;
		//gpu_UnmapBuffer(GL_ARRAY_BUFFER);
		//gpu_BindBuffer(GL_ARRAY_BUFFER, 0);
		
		
		/*if(mesh_a.mesh_count>=mesh_a.array_size)
		{
			model_ResizeMeshArray(mesh_a.array_size*2);
		}
		mesh_a.meshes[mesh_a.mesh_count++]=temp;*/
		
		
		/* not feeling like messing around the code. This will be here for now... */
		mesh_a.meshes[mesh_a.mesh_count++] = *mesh;
		
		
		//model_AddMeshNode(mesh);
	
		return mesh_a.mesh_count-1;
	}
	return -1;
}


PEWAPI void model_StoreVertexData(char *name, float *position, float *normal, float *tangent, float *tex_coord, edge_t *edges, int *indexes, int index_count, int vertex_count, int edge_count)
{
	vertex_data_t *temp;
	//if(position && normal && tangent && tex_coord && indexes)
	{
		temp = (vertex_data_t *)malloc(sizeof(vertex_data_t ));
		temp->position = position;
		temp->normal = normal;
		temp->tangent = tangent;
		temp->tex_coord = tex_coord;
		temp->edges = edges;
		temp->indexes = indexes;
		temp->index_count = index_count;
		temp->vertex_count = vertex_count;
		temp->edge_count = edge_count;
		temp->name = strdup(name);
		temp->next = NULL;
		vdata_last->next = temp;
		vdata_last = temp;
		
		printf("vertex data [%s] has %d indexes, %d vertexes and %d edges...\n", name, vdata_last->index_count, vdata_last->vertex_count, vdata_last->edge_count);
	}
}

PEWAPI vertex_data_t *model_GetVertexData(char *name)
{
	vertex_data_t *temp = vdata_root->next;
	while(temp)
	{
		if(!strcmp(name, temp->name))
		{
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}


void model_AddMeshNode(mesh_t *mesh)
{
	/*mesh_node_t *temp;
	if(mesh)
	{
		temp = (mesh_node_t *)malloc(sizeof(mesh_node_t));
		temp->next = NULL;
		temp->mesh = *mesh;
		temp->mesh.id = mesh_count;
		last->next = temp;
		last = temp;
		
	}
	return;*/
}

void model_RemoveMeshNode(char *name)
{
	/*mesh_node_t *n = root->next;
	mesh_node_t *p = root;
	
	while(n)
	{

		if(!strcmp(name, n->mesh.name))
		{
			p->next = n->next;
			free(n);
			mesh_count--;
			return;
		}
		p = n;
		n = n->next;
	}
	return;*/
}

void model_FlushMeshList()
{
	/*mesh_node_t *n = root->next;
	mesh_node_t *p;
	
	while(n)
	{
		p = n->next;
		free(n);
		n = p;
	}
	mesh_count = 0;
	return;*/
}

PEWAPI mesh_t model_GetMesh(char *name, int b_full_copy)
{
	int i;
	int c;
	int offset;
	int buf_size=0;
	float *buf;
	mesh_t temp;
	mesh_t *mesh;
	c=mesh_a.mesh_count;
	/*for(i=0; i<c; i++)
	{
		if(!strcmp(name, mesh_a.meshes[i].name))
		{
			mesh=&mesh_a.meshes[i];
			temp.draw_mode=mesh->draw_mode;
			temp.vert_count=mesh->vert_count;
			temp.name=name;
			if(b_full_copy)
			{
				temp.v_data=(float *)calloc(mesh->vert_count, sizeof(float)*3);
				memcpy(temp.v_data, mesh->v_data, sizeof(float )*3*mesh->vert_count);
				buf_size+=sizeof(float)*3*mesh->vert_count;
				
				if(mesh->n_data)
				{
					temp.n_data=(float *)calloc(mesh->vert_count, sizeof(float)*3);
					memcpy(temp.n_data, mesh->n_data, sizeof(float )*3*mesh->vert_count);
					buf_size+=sizeof(float )*3*mesh->vert_count;
				}
				else temp.n_data=NULL;
				
				if(mesh->t_c_data)
				{
					temp.t_c_data=(float *)calloc(mesh->vert_count, sizeof(float)*2);
					memcpy(temp.t_c_data, mesh->t_c_data, sizeof(float)*2);
					buf_size+=sizeof(float )*2*mesh->vert_count;
				}
				else temp.t_c_data=NULL;
				
				if(mesh->t_data)
				{
					temp.t_data=(float *)calloc(mesh->vert_count, sizeof(float)*3);
					memcpy(temp.t_data, mesh->t_data, sizeof(float)*3);
					buf_size+=sizeof(float )*3*mesh->vert_count;
				}
				else temp.t_data=NULL;
				
				glGenBuffers(1, &temp.vertex_buffer);
				glBindBuffer(GL_ARRAY_BUFFER, temp.vertex_buffer);
				glBufferData(GL_ARRAY_BUFFER, buf_size, NULL, GL_DYNAMIC_DRAW);
				buf=(float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
				
				
				for(i=0; i<temp.vert_count; i++)
				{
					buf[i*3]=temp.v_data[i*3];
					buf[i*3+1]=temp.v_data[i*3+1];
					buf[i*3+2]=temp.v_data[i*3+2];
				}
				offset+=temp.vert_count*3;
					
				if(temp.n_data)
				{
					for(i=0; i<temp.vert_count; i++)
					{
						buf[offset+i*3]=temp.n_data[i*3];
						buf[offset+i*3+1]=temp.n_data[i*3+1];
						buf[offset+i*3+2]=temp.n_data[i*3+2];		
					}
					offset+=temp.vert_count*3;
				}
				
				if(temp.t_c_data)
				{
					for(i=0; i<temp.vert_count; i++)
					{
						buf[offset+i*2]=temp.t_c_data[i*2];
						buf[offset+i*2+1]=temp.t_c_data[i*2+1];	
					}
					offset+=temp.vert_count*2;
				}
				
				if(temp.t_data)
				{
					for(i=0; i<temp.vert_count; i++)
					{
						buf[offset+i*3]=temp.t_data[i*3];
						buf[offset+i*3+1]=temp.t_data[i*3+1];
						buf[offset+i*3+2]=temp.t_data[i*3+2];		
					}
					offset+=temp.vert_count*3;
				}
				//gpu_UnmapGPUBuffer(&temp.buffer);
				glUnmapBuffer(GL_ARRAY_BUFFER);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				//temp.flags=MESH_FULL_COPY;
			}
			else
			{
				temp.v_data=mesh->v_data;
				temp.n_data=mesh->n_data;
				temp.t_c_data=mesh->t_c_data;
				temp.t_data=mesh->t_data;
				//temp.buffer=mesh->buffer;
				temp.vertex_buffer=mesh->vertex_buffer;
				//temp.flags=0;
			}
			return temp;
		}
	}
	temp.vert_count=0;
	temp.v_data=NULL;
	temp.draw_mode=0;
	temp.name=(char *)"invalid";
	return temp;*/
}


PEWAPI mesh_t *model_GetMeshPtr(char *name)
{
	int i;
	int c = mesh_a.mesh_count;
	
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, mesh_a.meshes[i].name))
		{
			return &mesh_a.meshes[i];
		}
	}
	return NULL;
	/*mesh_node_t *n = root->next;
	while(n)
	{
		if(!strcmp(n->mesh.name, name))
		{
			return &n->mesh;
		}
		n = n->next;
	}
	return NULL;*/
}


PEWAPI void model_FreeMesh(mesh_t *mesh)
{
	if(mesh)
	{
		free(mesh->v_data);
		mesh->v_data=NULL;
			
		/*if(mesh->n_data)
		{
			free(mesh->n_data);
			mesh->n_data=NULL;
		}
			
		if(mesh->t_data)
		{
			free(mesh->t_data);
			mesh->t_data=NULL;
		}
			
		if(mesh->t_c_data)
		{
			free(mesh->t_c_data);
			mesh->t_c_data=NULL;	
		}*/
			
		//glDeleteBuffers(1, &mesh->buffer.buffer_ID);
		//glDeleteBuffers(1, &mesh->vertex_buffer);
		
	}
}


/*
=============
model_GetMaxMinsFromVertexData
=============
*/
PEWAPI void model_GetMaxMinsFromVertexData(float *vertex_data, float *maxmins, int vertex_count)
{
	float max_x=-9999999999999999.0;
	float min_x=9999999999999999.0;
	float max_y=-9999999999999999.0;
	float min_y=9999999999999999.0;
	float max_z=-9999999999999999.0;
	float min_z=9999999999999999.0;
	
	int i;
	for(i=0; i<vertex_count; i++)
	{
		if(vertex_data[i*3]>max_x)max_x=vertex_data[i*3];
		//else if(vertex_data[i*3]<min_x)min_x=vertex_data[i*3];
		if(vertex_data[i*3+1]>max_y)max_y=vertex_data[i*3+1];
		//else if(vertex_data[i*3+1]<min_y)min_y=vertex_data[i*3+1];
		if(vertex_data[i*3+2]>max_z)max_z=vertex_data[i*3+2];
		//else if(vertex_data[i*3+2]<min_z)min_z=vertex_data[i*3+2];
	}
	
	maxmins[0]=max_x;
	maxmins[1]=max_y;
	maxmins[2]=max_z;
	//maxmins[3]=min_x;
	//maxmins[4]=min_y;
	//maxmins[5]=min_z;
}

/* TODO: change vertex_data param to uv_data. */
PEWAPI void model_CalculateTangents(float *vertex_data, float *uv_data, float *normal_data, float **tangent_data, int vert_count)
{
	int i;
	int count = vert_count;
	//float *tangent_data = NULL;
	
	vec3_t a;
	vec3_t b;
	vec3_t c;
	vec3_t ab;
	vec3_t ac;
	vec3_t t;
	vec3_t bt;
	vec3_t t1;
	vec3_t bt1;
	
	vec2_t duv1;
	vec2_t duv2;
	
	
	
	float x;
	float y;
	float z;
	float w;
	
	float q;
	
	if(vert_count < 3)
	{
		return;
	}
	
	//*tangent_data = (float *)calloc(vert_count, sizeof(float) * 3);
	//*bitangent_data = (float *)calloc(vert_count, sizeof(float) * 3);
	
	for(i = 0; i < count;)
	{
		
		duv1.floats[0] = uv_data[(i+1) * 2] - uv_data[i * 2];
		duv1.floats[1] = uv_data[(i+1) * 2 + 1] - uv_data[i * 2 + 1];
		
		duv2.floats[0] = uv_data[(i+2) * 2] - uv_data[i * 2];
		duv2.floats[1] = uv_data[(i+2) * 2 + 1] - uv_data[i * 2 + 1];
		
		q = 1.0 / (duv1.floats[0] * duv2.floats[1] - duv1.floats[1] * duv2.floats[0]);
		
		/*x = duv2.floats[1] / q;
		y = -duv1.floats[1] / q;
		z = -duv2.floats[0] / q;
		w = duv1.floats[0] / q;*/
		
		
		a.floats[0] = vertex_data[i*3];
		a.floats[1] = vertex_data[i*3+1];
		a.floats[2] = vertex_data[i*3+2];
	
		
		b.floats[0] = vertex_data[(i+1)*3]; 
		b.floats[1] = vertex_data[(i+1)*3+1];
		b.floats[2] = vertex_data[(i+1)*3+2];
		
		
		c.floats[0] = vertex_data[(i+2)*3]; 
		c.floats[1] = vertex_data[(i+2)*3+1];
		c.floats[2] = vertex_data[(i+2)*3+2];
		
		ab = sub3(b, a);
		ac = sub3(c, a);
		
		t1.floats[0] = (ab.floats[0] * duv2.floats[1] - ac.floats[0] * duv1.floats[1])*q;
		t1.floats[1] = (ab.floats[1] * duv2.floats[1] - ac.floats[1] * duv1.floats[1])*q;
		t1.floats[2] = (ab.floats[2] * duv2.floats[1] - ac.floats[2] * duv1.floats[1])*q;
		
		
		/*bt1.floats[0] = (ac.floats[0] * duv1.floats[0] - ab.floats[0] * duv2.floats[0])*q;
		bt1.floats[1] = (ac.floats[1] * duv1.floats[0] - ab.floats[1] * duv2.floats[0])*q;
		bt1.floats[2] = (ac.floats[2] * duv1.floats[0] - ab.floats[2] * duv2.floats[0])*q;*/
		
		t = gs_orthg(vec3(normal_data[i*3], normal_data[i*3+1], normal_data[i*3+2]), t1);
		(*tangent_data)[i*3] = t.floats[0];
		(*tangent_data)[i*3+1] = t.floats[1];
		(*tangent_data)[i*3+2] = t.floats[2];
		
		/*(*bitangent_data)[i*3] = 1.0;
		(*bitangent_data)[i*3+1] = 0.0;
		(*bitangent_data)[i*3+2] = 0.0;*/
		
		i++;
		
		t = gs_orthg(vec3(normal_data[i*3], normal_data[i*3+1], normal_data[i*3+2]), t1);
		(*tangent_data)[i*3] = t.floats[0];
		(*tangent_data)[i*3+1] = t.floats[1];
		(*tangent_data)[i*3+2] = t.floats[2];
		
		/*(*bitangent_data)[i*3] = 1.0;
		(*bitangent_data)[i*3+1] = 0.0;
		(*bitangent_data)[i*3+2] = 0.0;*/
		
		i++;
		
		t = gs_orthg(vec3(normal_data[i*3], normal_data[i*3+1], normal_data[i*3+2]), t1);
		(*tangent_data)[i*3] = t.floats[0];
		(*tangent_data)[i*3+1] = t.floats[1];
		(*tangent_data)[i*3+2] = t.floats[2];
		
		/*(*bitangent_data)[i*3] = 1.0;
		(*bitangent_data)[i*3+1] = 0.0;
		(*bitangent_data)[i*3+2] = 0.0;*/
		
		i++;

		
	}
	return;
}



PEWAPI void model_LoadModel(char *file_name, char *name)
{
	mesh_t m;
	mesh_t *t;
	int i = strlen(file_name);
	float *f;
	while(file_name[i] != '.') i--;
	i++;
	
	switch(*((int *)&file_name[i]))
	{
		case WAVEFRONT:
			m = loader_LoadWavefront(file_name);
		break;
		
		/*case COLLADA:
			m = loader_LoadCollada(file_name);
		break;*/
	}
		

	if(m.v_data)
	{
		m.name = strdup(name);
		i = model_StoreMesh(&m);
		#ifdef PARANOID
		if(i < 0)
		{
			printf("invalid mesh index for mesh_t [%s]!\n\n", name);
		}
		#endif
		
		if(i >= 0)
		{
			t = &mesh_a.meshes[i];
			vcache_CacheMeshData(t);
		}
		else
		{
			printf("invalid mesh index for mesh_t [%s]!\n\n", name);
		}
		
		
		//model_FreeMesh(&m);
	}
	#ifdef PARANOID
	else
	{
		printf("null ptr for mesh_t [%s]\n", name);
	}
	#endif 
	//return m;
}


PEWAPI void model_GenerateIcoSphere(float radius, int sub_divs, float **verts, int *face_count)
{
	int i;
	int j;
	int k;
	mesh_t m;
	float *a = (float *)malloc(sizeof(float) * 3 * 3 * 20);
	float *b;
	float v_offset = cos(DegToRad(60.0)) * radius;
	float c;
	int f_count = 20;
	int angle_increments = 4;
	float len;
	int src;
	int dst;
	
	
	for(i = 0; i < 5 * 3;)
	{
		a[i * 3] = 0.0;
		a[i * 3 + 1] = radius;
		a[i * 3 + 2] = 0.0;
		
		i++;
		
		a[i * 3] = sin(3.14159265 * (2.0 / 5.0) * (angle_increments)) * radius;
		a[i * 3 + 1] = v_offset;
		a[i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * (angle_increments)) * radius;
		
		i++;
		angle_increments--;
		
		a[i * 3] = sin(3.14159265 * (2.0 / 5.0) * angle_increments) * radius;
		a[i * 3 + 1] = v_offset;
		a[i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * angle_increments) * radius;
			
		i++;
		//angle_increments++;
	}
	
	j = i;
	angle_increments = 0;
	for(i = 0; i < 10 * 3;)
	{
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		
		i++;
		angle_increments++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		
		i++;
		angle_increments++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		
		i++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		
		i++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((angle_increments - 1) * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((angle_increments - 1) * 0.5 + 0.5)) * radius;
		
		i++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((angle_increments + 1) * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((angle_increments + 1) * 0.5 + 0.5)) * radius;
		
		i++;
			
	}
	
	j += i;
	angle_increments = 0;
	for(i = 0; i < 5 * 3;)
	{
		a[j * 3 + i * 3] = 0.0;
		a[j * 3 + i * 3 + 1] = -radius;
		a[j * 3 + i * 3 + 2] = 0.0;
		
		i++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((float)angle_increments + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((float)angle_increments + 0.5)) * radius;
		
		i++;
		angle_increments++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((float)angle_increments + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((float)angle_increments + 0.5)) * radius;
			
		i++;
	}
	
	
	if(sub_divs > 0)
	{
		for(k = 0; k < sub_divs; k++)
		{

				
			
				dst = 0;
				//src = 0;
				
				b = (float *)malloc(sizeof(float) * 3 * 3 * f_count * 4);
				
				for(i = 0; i < f_count * 3;)
				{
					/* v0 */
					b[dst * 3] = a[i * 3];
					b[dst * 3 + 1] = a[i * 3 + 1];
					b[dst * 3 + 2] = a[i * 3 + 2];		
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					
					dst++;
					
					/* v1 */
					b[dst * 3] = (a[i * 3] + a[(i + 1) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 1) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 1) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v2 */
					b[dst * 3] = (a[i * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					
					
					/* v3 */
					b[dst * 3] = (a[i * 3] + a[(i + 1) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 1) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 1) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v4 */
					b[dst * 3] = a[(i + 1)  * 3];
					b[dst * 3 + 1] = a[(i + 1) * 3 + 1];
					b[dst * 3 + 2] = a[(i + 1) * 3 + 2];
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					/* v5 */
					b[dst * 3] = (a[(i + 1) * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[(i + 1) * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[(i + 1) * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					
					
					/* v6 */
					b[dst * 3] = (a[(i + 1) * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[(i + 1) * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[(i + 1) * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v7 */
					b[dst * 3] = a[(i + 2) * 3];
					b[dst * 3 + 1] = a[(i + 2) * 3 + 1];
					b[dst * 3 + 2] = a[(i + 2) * 3 + 2];
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v8 */
					b[dst * 3] = (a[i * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					
					
					
					/* v9 */
					b[dst * 3] = (a[(i + 1) * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[(i + 1) * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[(i + 1) * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v10 */
					b[dst * 3] = (a[i * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v11 */
					b[dst * 3] = (a[i * 3] + a[(i + 1) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 1) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 1) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					i+=3;
				}
				
				f_count *= 4;

				free(a);
				a = b;		

		}
	}
	
	*verts = a;
	*face_count = f_count;
	
	return;
	
}

PEWAPI void model_GenerateIcoSphereMesh(float radius, int sub_divs)
{
	int face_count;
	float *verts;
	
	model_GenerateIcoSphere(radius, sub_divs, &verts, &face_count);
	model_StoreVertexData(strdup("_ico_"), verts, NULL, NULL, NULL, NULL, NULL, 0, face_count * 3, 0);
}


PEWAPI void model_GenerateCone(float length, float angle, int base_vert_count, float **verts, int *face_count)
{
	int i;
	int j;
	int f_count = 0;
	float step = (2.0 * 3.14159265) / (float)base_vert_count;
	float a = 0.0;
	//float o = cos(DegToRad(angle)) * length;
	float o = tan(DegToRad(angle)) * length;
	 
	float *b = (float *)malloc(sizeof(float) * 3 * 3 * (base_vert_count + 2) * 2);

	angle = 0.0;
	for(i = 0; i < base_vert_count * 3;)
	{
		b[i * 3] = 0.0;
		b[i * 3 + 1] = 0.0;
		b[i * 3 + 2] = 0.0;
		
		i++;
		
		b[i * 3] = cos(a) * o;
		b[i * 3 + 1] = sin(a) * o;
		b[i * 3 + 2] = -length;
		
		i++;
		a -= step;
		
		b[i * 3] = cos(a) * o;
		b[i * 3 + 1] = sin(a) * o;
		b[i * 3 + 2] = -length;
		
		i++;
		//a += step;
		
		f_count++;
	}
	
	j = i;
	a = 0.0;
	for(i = 0; i < base_vert_count * 3;)
	{
		b[j * 3 + i * 3] = 0.0;
		b[j * 3 + i * 3 + 1] = 0.0;
		b[j * 3 + i * 3 + 2] = -length;
		
		i++;
		
		b[j * 3 + i * 3] = cos(a) * o;
		b[j * 3 + i * 3 + 1] = sin(a) * o;
		b[j * 3 + i * 3 + 2] = -length;
		
		i++;
		a += step;
		
		b[j * 3 + i * 3] = cos(a) * o;
		b[j * 3 + i * 3 + 1] = sin(a) * o;
		b[j * 3 + i * 3 + 2] = -length;
		
		i++;
		//a += step;
		f_count++;
	}
	
	*face_count = f_count;
	*verts = b;
	
	return;
}

PEWAPI void model_GenerateConeMesh(float length, float angle, int base_vert_count)
{
	int face_count;
	float *verts;
	
	model_GenerateCone(length, angle, base_vert_count, &verts, &face_count);
	model_StoreVertexData(strdup("_cone_"), verts, NULL, NULL, NULL, NULL, NULL, 0, face_count * 3, 0);
}

PEWAPI void model_GenerateQuadMesh(float size)
{
	float *verts = (float *)malloc(sizeof(float) * 3 * 4);
	
	verts[0] = 1.0 * size;
	verts[1] = 1.0 * size;
	verts[2] = 0.0;
	
	verts[3] = 1.0 * size;
	verts[4] = -1.0 * size;
	verts[5] = 0.0;
	
	verts[6] = -1.0 * size;
	verts[7] = -1.0 * size;
	verts[8] = 0.0;
	
	verts[9] = -1.0 * size;
	verts[10] = 1.0 * size;
	verts[11] = 0.0;
	
	vec3_t *out;
	int out_c;
	
	triangulate((vec3_t *)verts, 4, &out, &out_c);
	
	model_StoreVertexData(strdup("_quad_"), (float *)out, NULL, NULL, NULL, NULL, NULL, 0, out_c, 0);
	
	free(verts);
	
}

PEWAPI void model_GenerateConvexPoly(float size, int vert_count)
{
	vec3_t *in;
	vec3_t *out;
	int out_c;
	int i;
	float step = (M_PI * 2.0) / (float)vert_count;
	float angle = 0.0;
	
	in = (vec3_t *)malloc(sizeof(vec3_t) * vert_count * 3);
	
	for(i = 0; i < vert_count; i++)
	{
		in[i].x = sin(angle);
		in[i].y = cos(angle);
		in[i].z = 0.0;
		
		angle += step;
	}
	
	
	triangulate(in, vert_count, &out, &out_c);
	
	model_StoreVertexData(strdup("_convex_polygon_"), (float *)out, NULL, NULL, NULL, NULL, NULL, 0, out_c, 0);
	
	free(in);
	
	
}

vec2_t model_GetVec2(char *file_str, int *str_cursor)
{
	int aux_cursor;
	int cursor=*str_cursor;
	char float_str[30];
	vec2_t v2;
	
	
	while(file_str[cursor]==' ' || file_str[cursor]=='\n' && file_str[cursor]!='\0'){cursor++;}
	aux_cursor=0;
	while(file_str[cursor]!=' ' && file_str[cursor]!='\n' && file_str[cursor]!='\0')
	{
		float_str[aux_cursor++]=file_str[cursor++];
	}
	float_str[aux_cursor]='\0';
	v2.floats[0]=atof(float_str);
			
			
	while(file_str[cursor]==' ' || file_str[cursor]=='\n' && file_str[cursor]!='\0'){cursor++;}
	aux_cursor=0;
	while(file_str[cursor]!=' ' && file_str[cursor]!='\n' && file_str[cursor]!='\0')
	{
		float_str[aux_cursor++]=file_str[cursor++];
	}
	float_str[aux_cursor]='\0';
	v2.floats[1]=atof(float_str);

	*str_cursor=cursor;
	return v2;
}



vec3_t model_GetVec3(char *file_str, int *str_cursor)
{
	int aux_cursor;
	int cursor=*str_cursor;
	char float_str[30];
	vec3_t v3;
	
	
	while(file_str[cursor]==' ' || file_str[cursor]=='\n' && file_str[cursor]!='\0'){cursor++;}
	aux_cursor=0;
	while(file_str[cursor]!=' ' && file_str[cursor]!='\n' && file_str[cursor]!='\0')
	{
		float_str[aux_cursor++]=file_str[cursor++];
	}
	float_str[aux_cursor]='\0';
	v3.floats[0]=atof(float_str);
			
			
	while(file_str[cursor]==' ' || file_str[cursor]=='\n' && file_str[cursor]!='\0'){cursor++;}
	aux_cursor=0;
	while(file_str[cursor]!=' ' && file_str[cursor]!='\n' && file_str[cursor]!='\0')
	{
		float_str[aux_cursor++]=file_str[cursor++];
	}
	float_str[aux_cursor]='\0';
	v3.floats[1]=atof(float_str);
			
			
	while(file_str[cursor]==' ' || file_str[cursor]=='\n' && file_str[cursor]!='\0'){cursor++;}
	aux_cursor=0;
	while(file_str[cursor]!=' ' && file_str[cursor]!='\n' && file_str[cursor]!='\0')
	{
		float_str[aux_cursor++]=file_str[cursor++];
	}
	float_str[aux_cursor]='\0';
	v3.floats[2]=atof(float_str);
	*str_cursor=cursor;
	return v3;
	
}


face_indexes model_GetFaceData(char *file_str, int *str_cursor)
{
	face_indexes f;
	int cursor=*str_cursor;
	int allocd_index_count=4;
	int index_count=0;
	int *indexes;
	int *copy;
	int aux_index;
	char int_str[30];
	indexes=(int *)malloc(sizeof(int)*allocd_index_count*3);
	
	while(file_str[cursor]!='f' && file_str[cursor]!='\0')
	{
		if(file_str[cursor]>='0' && file_str[cursor]<='9')
		{	 	
			aux_index=0;
			while(file_str[cursor]!='/' && file_str[cursor]!=' ' && file_str[cursor]!='\0' && file_str[cursor]!='f')
			{
				int_str[aux_index++]=file_str[cursor++];
			}
			int_str[aux_index]='\0';
			indexes[index_count*3]=atoi(int_str)-1;
			cursor++;
			
			aux_index=0;
			while(file_str[cursor]!='/' && file_str[cursor]!=' ' && file_str[cursor]!='\0' && file_str[cursor]!='f')
			{
				int_str[aux_index++]=file_str[cursor++];
			}
			if(aux_index>0)
			{
				int_str[aux_index]='\0';
				indexes[index_count*3+1]=atoi(int_str)-1;
			}
			else indexes[index_count*3+1]=-1;
			cursor++;
			
			aux_index=0;
			while(file_str[cursor]!='/' && file_str[cursor]!=' ' && file_str[cursor]!='\0' && file_str[cursor]!='f')
			{
				int_str[aux_index++]=file_str[cursor++];
			}
			int_str[aux_index]='\0';
			indexes[index_count*3+2]=atoi(int_str)-1;
			
			index_count++;
			
			if(index_count>=allocd_index_count)
			{
				copy=(int *)malloc(sizeof(int)*(allocd_index_count+3)*3);
				memcpy(copy, indexes, sizeof(int)*index_count*3);
				free(indexes);
				indexes=copy;
				copy=NULL;
				allocd_index_count+=3;
			}
		}
		else cursor++;
	}
	*str_cursor=cursor;
	f.indexes=indexes;
	f.vert_count=index_count;
	return f;
}


void model_TriangulateFace(float **verts, int *vert_count)
{
	int z_vert_count=(*vert_count+1)/2;
	int t_vert_count=0;
	int tri_count=*vert_count-2;
	float *output=(float *)calloc(tri_count*3, sizeof(float)*3);
	float *temp=(float *)calloc(*vert_count*2, sizeof(float)*3);
	int  *z_verts=(int *)calloc(z_vert_count, sizeof(int));
	register int i;
	int c;
	c=*vert_count;
	z_vert_count=0;

	for(i=0; i<c; i++)
	{
		*(temp+i*3)=*((*verts)+i*3);
		*(temp+i*3+1)=*((*verts)+i*3+1);
		*(temp+i*3+2)=*((*verts)+i*3+2);
	}
	
	while(c>1)
	{
		for(i=0; i<c-1; )
		{
			*(z_verts+z_vert_count++)=i;
		
			*(output+t_vert_count*3)=*(temp+i*3);
			*(output+t_vert_count*3+1)=*(temp+i*3+1);
			*(output+t_vert_count*3+2)=*(temp+i*3+2);
		
			i++;
			t_vert_count++;
		
			*(output+t_vert_count*3)=*(temp+i*3);
			*(output+t_vert_count*3+1)=*(temp+i*3+1);
			*(output+t_vert_count*3+2)=*(temp+i*3+2);
		
			i++;
			t_vert_count++;
			
			*(output+t_vert_count*3)=*(temp+i*3);
			*(output+t_vert_count*3+1)=*(temp+i*3+1);
			*(output+t_vert_count*3+2)=*(temp+i*3+2);
		
			t_vert_count++;
		}
		
		c=z_vert_count;
		
		z_vert_count=0;
		for(i=0; i<c; i++)
		{
			*(temp+i*3)=*((*verts)+z_verts[i]*3);
			*(temp+i*3+1)=*((*verts)+z_verts[i]*3+1);
			*(temp+i*3+2)=*((*verts)+z_verts[i]*3+2);
		}
	}
	
	free(z_verts);
	free(temp);
	free(*verts);
	(*verts)=output;
	*vert_count=tri_count*3;	
}


void model_TriangulateFaceIndexes(int **indexes, int *vert_count)
{
	int z_vert_count=(*vert_count+1)/2;
	int t_vert_count=0;
	int tri_count=(*vert_count)-2;
	int *output=(int *)calloc(tri_count*3, sizeof(int)*3);
	int *temp=(int *)calloc((*vert_count)*2, sizeof(int)*3);
	int *z_verts=(int *)calloc(z_vert_count, sizeof(int));
	int b_break=0;
	int i;
	int c;
	c=*vert_count;
	//if(vert_count%2)vert_count--;
	z_vert_count=0;
	if(*vert_count <= 3) return;
	for(i=0; i<c; i++)
	{
		*(temp+i*3)=*((*indexes)+i*3);
		
		//printf("%d ", *((*indexes)+i*3));
		*(temp+i*3+1)=*((*indexes)+i*3+1);
		*(temp+i*3+2)=*((*indexes)+i*3+2);
	}
	while(c>1)
	{

		b_break=0;
		//printf("   start of loop\n");
		for(i=0; i<c; )
		{
			*(z_verts+z_vert_count++)=*(temp+i*3);	/* save the first index of this triangle */
			//*(z_verts+z_vert_count++)=i;
			//printf("\nzero vertex is %d\n", *(temp+i*3));
			//printf("\nzero vertex is %d\n", i);
			
			*(output+t_vert_count*3)=*(temp+i*3);
			*(output+t_vert_count*3+1)=*(temp+i*3+1);
			*(output+t_vert_count*3+2)=*(temp+i*3+2);
			
			//printf("%d ", *(temp+i*3));
			
			
			
			i++;
			t_vert_count++;
		
			*(output+t_vert_count*3)=*(temp+i*3);
			*(output+t_vert_count*3+1)=*(temp+i*3+1);
			*(output+t_vert_count*3+2)=*(temp+i*3+2);
			
			//printf("%d ", *(temp+i*3));
			
			i++;
			
			if(c%2)
			{
				if(i+1==c)
				{

					b_break=1;
					*(z_verts+z_vert_count++)=*(temp+i*3);
					//*(z_verts+z_vert_count++)=i;
				}
			}
			else
			{
				if(i==c)
				{
					i=0;
					b_break=1;
				}	
			}	
			
			//printf("%d ", *(temp+i*3));
			
			t_vert_count++;
			
			*(output+t_vert_count*3)=*(temp+i*3);
			*(output+t_vert_count*3+1)=*(temp+i*3+1);
			*(output+t_vert_count*3+2)=*(temp+i*3+2);
		
			t_vert_count++;
			
			if(b_break) break;
			
		}
		c=z_vert_count;
		//printf("\n   zero vertex count is %d\n", c);
		if(c<3) break;
		//printf("   continue\n");
		z_vert_count=0;
		for(i=0; i<c; i++)
		{
			*(temp+i*3)=*((*indexes)+z_verts[i]*3);
			*(temp+i*3+1)=*((*indexes)+z_verts[i]*3+1);
			*(temp+i*3+2)=*((*indexes)+z_verts[i]*3+2);
		}
	}
	
	free(z_verts);
	free(temp);
	free(*indexes);
	(*indexes)=output;
	*vert_count=tri_count*3;
	//printf("this face now has %d vertices==========\n\n", *vert_count);
}


void model_LoadMaterialFromWavefront(char *file_name)
{
	
}

#ifdef __cplusplus
}
#endif

























