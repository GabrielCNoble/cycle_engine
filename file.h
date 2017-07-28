#ifndef FILE_H
#define FILE_H
//#include "cimport.h"

typedef struct
{
	int size;
	char *buf;
}file_t;

file_t file_LoadFile(char *file_name, int bin_load);

unsigned int file_GetFileSize(FILE *f);

//aiScene *file_OpenFile(char *file_name, int flags);





#endif 
