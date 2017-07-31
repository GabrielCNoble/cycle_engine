#include <stdio.h>
#include <stdlib.h>
#include "loader_wavefront.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision\CollisionShapes\btShapeHull.h"
#include "vector.h"
#include "matrix.h"
#include "console.h"

extern int mesh_path_len;
extern char mesh_path[256];


mesh_t loader_LoadWavefront(char *file_name)
{
	mesh_t mesh;
	FILE *file;
	
	int vertex_count=0;
	int allocd_vertexes=3;
	float *v_data=(float *)malloc(sizeof(float)*3*allocd_vertexes);
	
	int normal_count=0;
	int allocd_normals=3;
	float *n_data=(float *)malloc(sizeof(float)*3*allocd_normals);
	
	int t_c_count=0;
	int allocd_t_c=3;
	float *t_c_data=(float *)malloc(sizeof(float)*2*allocd_t_c);
	
	int tangent_count = 0;
	int allocd_tangent = 3;
	float *t_data = NULL;
	
	int index_count=0;
	int allocd_indexes=4;
	int *i_data=(int *)malloc(sizeof(int )*3*allocd_indexes);
	
	int oindex_count = 0;
	int allocd_oindexes = 4;
	int *oi_data = (int *)malloc(sizeof(int) * 3 * allocd_oindexes);
	
	int edge_count = 0;
	int allocd_edges = 2;
	edge_t *edges = (edge_t *)malloc(sizeof(edge_t) * allocd_edges);
	
	
	float *temp;
	int *itemp;
	edge_t *etemp;
	
	char full_path[256];
	strcpy(full_path, mesh_path);
	strcat(full_path, file_name);
	
	//int *indexes;
	char *file_str;
	char float_str[30];
	int str_cursor;
	int aux_cursor;
	int vbuf_size;
	int i;
	int j;
	int c;
	face_indexes f;
	vec3_t v3;
	vec2_t v2;
	
	mesh.vert_count=0;
	mesh.v_data=NULL;
	mesh.n_data=NULL;
	mesh.t_data=NULL;
	mesh.t_c_data=NULL;
	
	btConvexHullShape *chtemp;
	btShapeHull *htemp;
	//mesh.b_data = NULL;
	
	if(!(file=fopen(full_path, "rb")))
	{
		console_Print(MESSAGE_ERROR, "couldn't open wavefront file [%s]!\n", file_name);
		mesh.name=(char *)"invalid";
		
		free(v_data);
		free(n_data);
		free(i_data);
		
		return mesh;
	}
	
	str_cursor=0;
	while(!feof(file))
	{
		fgetc(file);
		str_cursor++;
	}
	file_str=(char *)calloc(str_cursor+2, 1);
	rewind(file);
	
	str_cursor=0;
	while(!feof(file))
	{
		file_str[str_cursor++]=fgetc(file);
	}
	rewind(file);
	fclose(file);
	str_cursor=0;
	while(!file_str[str_cursor]=='\0')
	{

		
		while(file_str[str_cursor] == ' ') str_cursor++;
		
		if(file_str[str_cursor]=='v')
		{
			str_cursor++;
			if(file_str[str_cursor]=='n')
			{
				str_cursor++;
				v3=model_GetVec3(file_str, &str_cursor);
				
				n_data[normal_count*3]=v3.floats[0];
				n_data[normal_count*3+1]=v3.floats[1];
				n_data[normal_count*3+2]=v3.floats[2];
				
				normal_count++;
				
				if(normal_count>=allocd_normals)
				{
					temp=(float *)malloc(sizeof(float)*3*(allocd_normals+3));
					memcpy(temp, n_data, sizeof(float)*3*normal_count);
					free(n_data);
					n_data=temp;
					temp=NULL;
					allocd_normals+=3;
				}
				
				continue;
			}
			
			else if(file_str[str_cursor]=='t')
			{
				str_cursor++;
				v2=model_GetVec2(file_str, &str_cursor);
				
				if(t_c_count >= allocd_t_c)
				{
					temp = (float *) malloc(sizeof(float) * 2 * (allocd_t_c+3));
					memcpy(temp, t_c_data, sizeof(float) * 2 * allocd_t_c);
					free(t_c_data);
					t_c_data = temp;
					temp = NULL;
					allocd_t_c += 3;
				}
				
				t_c_data[t_c_count*2] = v2.floats[0];
				t_c_data[t_c_count*2+1] = v2.floats[1];
				
				t_c_count++;
				continue;
			}
			
			str_cursor++;
			v3=model_GetVec3(file_str, &str_cursor);
			
			v_data[vertex_count*3]=v3.floats[0];
			v_data[vertex_count*3+1]=v3.floats[1];
			v_data[vertex_count*3+2]=v3.floats[2];
				
			vertex_count++;
			
			if(vertex_count>=allocd_vertexes)
			{
				temp=(float *)malloc(sizeof(float)*3*(allocd_vertexes+3));
				memcpy(temp, v_data, sizeof(float)*3*vertex_count);
				free(v_data);
				v_data=temp;
				temp=NULL;
				allocd_vertexes+=3;
			}
			

		}
		else if(file_str[str_cursor]=='f')
		{
			str_cursor++;
			f = model_GetFaceData(file_str, &str_cursor);
		
			/*for(i = 0; i < f.vert_count; i++)
			{
				if(oindex_count + i >= allocd_oindexes)
				{
					itemp = (int *)malloc(sizeof(int) * 3 *(allocd_oindexes + 10));
					memcpy(itemp, oi_data, sizeof(int) * (oindex_count + i) * 3);
					free(oi_data);
					oi_data = itemp;
					itemp = NULL;
					allocd_oindexes += 10;
				}
				oi_data[(oindex_count + i) * 3] = f.indexes[i * 3];
				oi_data[(oindex_count + i) * 3 + 1] = f.indexes[i * 3 + 1];
				oi_data[(oindex_count + i) * 3 + 2] = f.indexes[i * 3 + 2];
			}
			oindex_count += i;*/
			
			
			/*for(i = 0, j = 0; i < f.vert_count; i++)
			{
				if(edge_count + j >= allocd_edges)
				{
					etemp = (edge_t *)malloc(sizeof(edge_t) * (allocd_edges + 10));
					memcpy(etemp, edges, sizeof(edge_t) * (edge_count + j));
					free(edges);
					edges = etemp;
					etemp = NULL;
					allocd_edges += 10;
				}
				
				edges[edge_count + j].vert0 = f.indexes[i * 3];
				
				if(i + 1 >= f.vert_count)
				{
					edges[edge_count + j].vert1 = f.indexes[0];
				}
				else
				{
					edges[edge_count + j].vert1 = f.indexes[(i + 1) * 3];
				}
				
				j++;


			}
			edge_count += j;*/
			
			//printf("%s  %d\n", filename, f.vert_count);
			
			/* triangulate properly... */
			model_TriangulateFaceIndexes(&f.indexes, &f.vert_count);
			
			
			for(i=0; i<f.vert_count; i++)
			{
					
				if(index_count+i>=allocd_indexes)
				{
					itemp=(int *)malloc(sizeof(int)*3*(allocd_indexes+10));
					memcpy(itemp, i_data, sizeof(int )*3*(index_count+i));
					free(i_data);
					i_data=itemp;
					itemp=NULL;
					allocd_indexes+=10;
				}
				
				i_data[(index_count+i)*3]=f.indexes[i*3];
				i_data[(index_count+i)*3+1]=f.indexes[i*3+1];
				i_data[(index_count+i)*3+2]=f.indexes[i*3+2];
				
			}
			index_count+=i;
			free(f.indexes);
		}
		
		else if(file_str[str_cursor] == 'o')
		{
			/* skip the name */
			while(file_str[str_cursor] != '\n' && file_str[str_cursor] != '\0') str_cursor++;
		}
		
		else if(file_str[str_cursor] == 'u')
		{
			if(file_str[str_cursor + 1] == 's' && 
			   file_str[str_cursor + 2] == 'e' &&
			   file_str[str_cursor + 3] == 'm' && 
			   file_str[str_cursor + 4] == 't' && 
			   file_str[str_cursor + 5] == 'l')
			{
				str_cursor += 6;
				/* ignore the usemtl command for now... */
				while(file_str[str_cursor] == ' ') str_cursor++;
				while(file_str[str_cursor] != ' ' && file_str[str_cursor] != '\n' && file_str[str_cursor] != '\0') str_cursor++;
			}
		}
		
		else if(file_str[str_cursor] == 'm')
		{
			if(file_str[str_cursor + 1] == 't' && 
			   file_str[str_cursor + 2] == 'l' &&
			   file_str[str_cursor + 3] == 'l' && 
			   file_str[str_cursor + 4] == 'i' && 
			   file_str[str_cursor + 5] == 'b')
			{
				str_cursor += 6;
				/* ignore import of materials for now... */
				while(file_str[str_cursor] == ' ') str_cursor++;
				while(file_str[str_cursor] != ' ' && file_str[str_cursor] != '\n' && file_str[str_cursor] != '\0') str_cursor++;
			}
		}
		
		else if(file_str[str_cursor] == '#')
		{
			/* skip comments */
			while(file_str[str_cursor] != '\n' && file_str[str_cursor] != '\0') str_cursor++;
		}
		
		else if(file_str[str_cursor] == 's')
		{
			while(file_str[str_cursor] == ' ') str_cursor++;
			while(file_str[str_cursor] != '\n' && file_str[str_cursor] != '\0') str_cursor++;
		}
		
		else str_cursor++;
	}
	
	/* this seems to be causing crashes on gpu_Write()... */
	vbuf_size = sizeof(float) * 3 * index_count * 2;
	
	if(t_c_count)
	{
		vbuf_size += sizeof(float) * 2 * index_count + sizeof(float) * 3 * index_count;
	}
	
	mesh.v_data = (float *)malloc(vbuf_size);
	mesh.n_data = mesh.v_data + index_count * 3;
	
	if(t_c_count)
	{
		mesh.t_data = mesh.n_data + index_count * 3;
		mesh.t_c_data = mesh.t_data + index_count * 3;
		
		
	}
	for(i=0; i<index_count; i++)
	{
		mesh.v_data[i * 3]=v_data[i_data[i * 3] * 3];
		mesh.v_data[i * 3 + 1]=v_data[i_data[i * 3] * 3 + 1];
		mesh.v_data[i * 3 + 2]=v_data[i_data[i * 3] * 3 + 2];
	}

	for(i=0; i<index_count; i++)
	{
		mesh.n_data[i * 3]=n_data[i_data[i * 3 + 2] * 3];
		mesh.n_data[i * 3 + 1]=n_data[i_data[i * 3 + 2] * 3 + 1];
		mesh.n_data[i * 3 + 2]=n_data[i_data[i * 3 + 2] * 3 + 2];
	}
	if(t_c_count > 0)
	{
		for(i = 0; i < index_count; i++)
		{
			mesh.t_c_data[i * 2] = t_c_data[i_data[i * 3 + 1] * 2];
			mesh.t_c_data[i * 2 + 1] = t_c_data[i_data[i * 3 + 1] * 2 + 1];
		}
		model_CalculateTangents(mesh.v_data, mesh.t_c_data, mesh.n_data, &mesh.t_data, index_count);
	}

	chtemp = new btConvexHullShape(mesh.v_data, index_count, sizeof(float) * 3);
	htemp = new btShapeHull(chtemp);
	htemp->buildHull(chtemp->getMargin());
	mesh.collision_shape = new btConvexHullShape((btScalar *)htemp->getVertexPointer(), htemp->numVertices(), sizeof(float )*4);
	delete chtemp;
	delete htemp;
	
	
	//printf("q\n");
	
	
	/*for(i=0; i<index_count; i++)
	{
		mesh.v_data[i*11] = v_data[i_data[i*3]*3];
		mesh.v_data[i*11+1] = v_data[i_data[i*3]*3+1];
		mesh.v_data[i*11+2] = v_data[i_data[i*3]*3+2];
	}
	
	for(i=0; i<index_count; i++)
	{
		mesh.v_data[3 + i*11]=n_data[i_data[i*3+2]*3];
		mesh.v_data[3 + i*11+1]=n_data[i_data[i*3+2]*3+1];
		mesh.v_data[3 + i*11+2]=n_data[i_data[i*3+2]*3+2];
	}*/
	
	/*for(i = 0; i < index_count; i++)
	{
		printf("v: [%f %f %f]    n: [%f %f %f]\n", mesh.v_data[i * 9], mesh.v_data[i * 9 + 1], mesh.v_data[i * 9 + 2],
												   mesh.v_data[3 + i * 9], mesh.v_data[3 + i * 9 + 1], mesh.v_data[3 + i * 9 + 2]);
	}*/
	
	
	mesh.draw_mode=GL_TRIANGLES;
	mesh.vert_count=index_count;
	
	free(file_str);
	free(v_data);
	free(n_data);
	free(i_data);
	free(t_c_data);
	
	printf("mesh %s has %d vertexes\n",file_name, mesh.vert_count);
	
	return mesh;
}
