#ifndef MATERIAL_H
#define MATERIAL_H

#include "material_types.h"
#include "includes.h"
#include "conf.h"



#ifdef __cplusplus
extern "C"
{
#endif

void material_Init(char *path);

void material_Finish();

void material_ResizeMaterialArray(int new_size);

/*PEWAPI void material_CreateMaterialFromData(material_t *material);*/

PEWAPI void material_CreateMaterial(char *name, float glossiness, float metallic, vec4_t color, float emissive, int bm_flags, tex_info_t *ti);

PEWAPI void material_LoadMaterial(char *filename, char *name);

PEWAPI material_t *material_GetMaterialByIndex(int );

PEWAPI int material_GetMaterialIndex(char *name);

PEWAPI void material_UpdateGPUMaterial(material_t *material);

extern PEWAPI void (*material_SetMaterialByIndex)(int material_index);

/*PEWAPI color4_t material_FloatToColor4_t(float r, float g, float b, float a);

PEWAPI void material_FloatToBaseMultiplierPair(float r, float g, float b, float a, color4_t *pair);

PEWAPI void material_SetMaterialDiffuseColor(material_t *material, float r, float g, float b, float a);*/

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL_H */














