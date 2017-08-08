#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include "scenegraph_types.h"

#include "configuration\conf.h"
#include "includes.h"

#include "vector.h"
#include "matrix.h"
#include "plane.h"

#define DEBUG_CODE_ENABLED



enum PLANES_INTERSECTION
{
	TOP_INTERSECTED = 1,
	BOTTOM_INTERSECTED = 2,
	LEFT_INTERSECTED = 4,
	RIGHT_INTERSECTED = 8,
	FRONT_NEAR_INTERSECTED = 16,
	FRONT_FAR_INTERSECTED = 32,
	BACK_NEAR_INTERSECTED = 64,
	BACK_FAR_INTERSECTED = 128,
	LEFT_NEAR_INTERSECTED = 256,
	LEFT_FAR_INTERSECTED = 512,
	RIGHT_NEAR_INTERSECTED = 1024,
	RIGHT_FAR_INTERSECTED = 2048,
	TOP_NEAR_INTERSECTED = 4096,
	TOP_FAR_INTERSESCTED = 8192,
	BOTTOM_NEAR_INTERSECTED = 16384,
	BOTTOM_FAR_INTERSECTED = 32768
};


#ifdef __cplusplus
extern "C"
{
#endif

void scenegraph_Init();

void scenegraph_Finish();

void scenegraph_ProcessScenegraph();

void scenegraph_FreeGraph(node_t *node);

PEWAPI void scenegraph_PurgeAllNodes();

/* Adds a node to the graph. The added node is a child of the root node. */
node_t *scenegraph_AddNode(int type, int index, short sub_index, char *name);

/* remove a node from the graph. */
void scenegraph_RemoveNode(node_t *node, int b_recursive);

/* set the child node's parent node */
PEWAPI void scenegraph_SetParent(node_t *node, node_t *parent, int b_keep_transform);
/* remove the parent from a node. After the call, the parent node
of node is the root node. */
PEWAPI void scenegraph_RemoveParent(node_t *node, int b_keep_transform);

/* set the parent node's child node*/
PEWAPI void scenegraph_SetChild(node_t *node, node_t *child);




PEWAPI void scenegraph_PrintChildsNames(node_t *node);

int scenegraph_GetEntityUnderMouse();

pick_record_t scenegraph_Pick();

#ifdef __cplusplus
}
#endif


#endif /* SCENEGRAPH_H */














