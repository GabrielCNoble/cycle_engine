#ifndef SCENEGRAPH_TYPES_H
#define SCENEGRAPH_TYPES_H

enum NODE_TYPE
{
	NODE_ROOT=0,
	NODE_ENTITY,
	NODE_PROJECTILE,
	//NODE_ENTITY_TO_ROOT,
	//NODE_ENTITY_TO_ENTITY,
	NODE_CAMERA,
	NODE_LIGHT,
	NODE_PARTICLE,
	NODE_COLLIDER,
	NODE_ARMATURE,
	NODE_BONE
};

enum SCENEGRAPH_STATUS
{
	SCENEGRAPH_RECOMPACT_NEEDED=1,
	SCENEGRAPH_HAS_BLOCKS=2,
	SCENEGRAPH_
};

enum NODE_HINT
{
	HINT_NONE=0,
	HINT_STATIC=1,	/* this node won't be parented, nor any node will become its children at runtime */
	HINT_DYNAMIC=2  
};

enum PICK_RESULT_TYPE
{
	PICK_ENTITY = 1,
	PICK_LIGHT,
	PICK_BMODEL,
};

typedef struct node_t
{
	int index;				/* entry in one of the arrays in the system. This node can refer to cameras, lights, entities... This value is constant
							   for the whole life time of this node. */
	char *name;
	struct node_t *parent;
	short node_index;			/* the position inside the parent's child node array */
	short max_children;		/* this number may increase as this node gets more child nodes attatched to it. */
	short sub_index;
	short type;		
	//short hint;				/* hint for the scenegraph on how to process this node. */
	short parent_type;
	short children_count;
	void *children;
}node_t;					/* Variable size node. Not safe to assume anything about its size */


typedef struct
{
	int graph_size;
	int node_count;
	int bm_status;
	node_t *root;
}scenegraph_t;


typedef struct
{
	int type;
	int index;
}pick_record_t;



#endif /* SCENEGRAPH_TYPES_H */
