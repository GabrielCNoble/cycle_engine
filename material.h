#ifndef MATERIAL_H
#define MATERIAL_H

#include "material_types.h"
#include "includes.h"
#include "conf.h"



#ifdef __cplusplus
extern "C"
{
#endif

PEWAPI void material_Init(char *path);

PEWAPI void material_Finish();

PEWAPI void material_ResizeMaterialArray(int new_size);

PEWAPI void material_CreateMaterialFromData(material_t *material);

PEWAPI void material_CreateMaterial(char *name, short shininess, float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a, float specular_r, float specular_g, float specular_b, float specular_intensity, int bm_flags, tex_info_t *ti);

PEWAPI void material_LoadMaterial(char *filename, char *name);

PEWAPI material_t *material_GetMaterialByIndex(int material_index);

PEWAPI int material_GetMaterialIndex(char *name);

PEWAPI void material_SetMaterialByIndex(int material_index);

PEWAPI color4_t material_FloatToColor4_t(float r, float g, float b, float a);

PEWAPI void material_FloatToBaseMultiplierPair(float r, float g, float b, float a, color4_t *pair);

PEWAPI void material_SetMaterialDiffuseColor(material_t *material, float r, float g, float b, float a);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL_H */














