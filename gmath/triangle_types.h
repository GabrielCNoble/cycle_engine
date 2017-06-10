#ifndef TRIANGLE_TYPES_H
#define TRIANGLE_TYPES_H

typedef struct
{
	float a[3];
	float b[3];
	float c[3];
}triangle_t;



typedef struct
{
	int vert0;
	int vert1;
}edge_t;			/* indexes */



#endif /* TRIANGLE_TYPES_H */
