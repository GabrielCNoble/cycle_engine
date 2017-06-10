#include <stdio.h>
#include "file.h"
#include "console.h"



file_t file_LoadFile(char *file_name, int bin_load)
{
	FILE *f;
	int i;
	char *buf;
	char b;
	
	file_t q;
	q.buf = NULL;
	q.size = -1;
	
	if(!bin_load)
	{
		f = fopen(file_name, "r");
	}
	else
	{
		f = fopen(file_name, "rb");
	}

	if(!f)
	{
		console_Print(MESSAGE_ERROR, "couldn't load file [%s]!", file_name);
		return q;
	}
	
	q.size = file_GetFileSize(f) + 1;

	q.buf = (char *)malloc(q.size);
	i = 0;
	while(1)
	{
		b = fgetc(f);
		if(!feof(f))
		{
			q.buf[i] = b;
			i++;
		}
		else
		{
			break;
		}
		
	}
	fclose(f);
	
	/* Won't make difference for binary files. This is
	only here to allow this function to load text files... */
	q.buf[i] = '\0';
	
	return q;	
}

unsigned int file_GetFileSize(FILE *f)
{
	long cur;
	long size;
	
	cur = ftell(f);
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, cur, SEEK_SET);
	
	return size;
}

aiScene *file_OpenFile(char *file_name, int flags)
{
	aiScene *s = (aiScene *)aiImportFile(file_name, 0);
	if(!s) return NULL;
	return s;
	//return  Assimp :: ReadFile(file_name, 0);
}










