#ifndef DRAW_H
#define DRAW_H

#include "draw_types.h"

#include "conf.h"
#include "includes.h"
#include "model.h"
#include "entity.h"
#include "text.h"
#include "gui.h"
#include "console.h"
#include "framebuffer.h"
#include "light_types.h"

#define DEFAULT_SMALL_BLOOM_RADIUS 14
#define DEFAULT_MEDIUM_BLOOM_RADIUS 16
#define DEFAULT_LARGE_BLOOM_RADIUS 18

#define DEFAULT_BLOOM_INTENSITY 5

#define DEFAULT_SMALL_BLOOM_ITERATIONS 2
#define DEFAULT_MEDIUM_BLOOM_ITERATIONS 2
#define DEFAULT_LARGE_BLOOM_ITERATIONS 2





#ifdef __cplusplus
extern "C"
{
#endif

void draw_Init(int resolution, int mode);

void draw_Finish();

PEWAPI int draw_GetResolutionMode(int width, int height);

int draw_SetResolution(int width, int height, int mode);

void draw_Fullscreen(int enable);

void draw_OpenFrame();

//void draw_DrawFrameDebug();

void draw_DrawFrame();

//static inline void draw_DrawFrame();

void draw_CloseFrame();

PEWAPI void draw_GetScreenResolution(int *width, int *height);

inline void draw_ResizeRenderQueue(render_queue *r_queue, int new_size);

inline void draw_ResetRenderQueue();

inline void draw_ResetShadowQueue();

void draw_SortRenderQueue(render_queue *queue, int left, int right);

//void draw_ExpandScreenTileList(screen_tile *tile, int new_size);

//screen_tile *draw_GetAffectedTile(int x, int y);



void draw_FillCommandBuffer128(command_buffer_t *cb, mesh_t *mesh, int entity_index, int material_index, mat4_t *model_view_matrix, mat4_t *last_model_view_matrix);

void draw_FillShadowCommandBuffer(command_buffer_t *scb, mat4_t *matrix, int vert_count, short material_index, int gpu_buffer, int draw_mode);

//void draw_DispatchCommandBuffer(render_queue *rqueue, command_buffer_t *cb);

//void draw_DispatchCommandBuffer(render_queue *rqueue, int entity_index, int material_index, mesh_t *mesh, mat4_t *model_view_matrix);

void draw_DispatchCommandBuffer(render_queue *rqueue, int entity_index, int material_index, int affecting_lights_index, int draw_flags, int start, int vert_count, mat4_t *model_view_matrix);

int draw_DispatchShadowCommandBuffer(command_buffer_t *cb);

int draw_FillAndDispatchShadowCommandBuffer(int vert_count, short material_index, int start, int draw_mode, mat4_t *matrix);





PEWAPI void draw_SetRenderDrawMode(int draw_mode);

PEWAPI void draw_SetRenderFlags(int flags);

//PEWAPI void draw_SetDebugLevel(int level);

PEWAPI void draw_SetDebugFlag(int flag);

PEWAPI void draw_ResetDebugFlag(int flag);



//PEWAPI void draw_EnterIdentityMode();

//PEWAPI void draw_ExitIdentityMode();



void draw_DrawWireframe();

void draw_DrawFlat();

void draw_DrawLit();

void draw_DrawEmissive();

void draw_DrawTranslucent();

void draw_ResolveGBuffer();

void draw_ResolveTranslucent();

void draw_DrawShadowMaps();

void draw_DrawLightVolumes();

void draw_DrawBloom();


void draw_AutoAdjust();

void draw_Dummy();


void draw_Compose();

void draw_ComposeVolNoBloom();

void draw_ComposeNoVolBloom();

void draw_ComposeNoVolNoBloom();



void draw_FillStencilBuffer();

void draw_BlitToScreen();

PEWAPI void draw_SetBloomParam(int param, int value);

PEWAPI int draw_GetBloomParam(int param);

PEWAPI unsigned int draw_GetCompositeBufferTexture();

PEWAPI void draw_EnableOutputToBackbuffer(int enable);

//PEWAPI void draw_SetBloomRadius(int bloom, float radius);

//PEWAPI void draw_SetBloomIntensity(float intensity);

//void draw_DrawScreenQuadNoVol();

//PEWAPI void draw_DrawTextToTexture(char *str, int x, int y);

PEWAPI __stdcall void draw_DrawString(int font_index, int size, int x, int y, int line_length, vec3_t color, char *str, ...);

void draw_DrawWidgets();

//PEWAPI void draw_DrawBitmap();

//PEWAPI void draw_DrawChar(char c, font_t *font, int x, int y);

//PEWAPI void draw_DrawLine(vec3_t from, vec3_t to, vec3_t color);

//PEWAPI void draw_Draw2DLine(vec2_t from, vec2_t to, vec3_t color);

//PEWAPI void draw_DrawPoint(vec3_t position, vec3_t color, float point_size);

//PEWAPI void draw_DrawPointHomogeneous(vec3_t position, vec3_t color, float point_size);

//PEWAPI void draw_DrawLineHomogeneous(vec3_t a, vec3_t b, vec3_t color);

//PEWAPI void draw_Draw2DPoint(vec2_t position, vec3_t color, float point_size);

//PEWAPI void draw_debug_DrawEntityAABB(entity_p *entity);

//PEWAPI void draw_DrawEntityOBB(entity_p *entity);

//PEWAPI void draw_DrawAABB(float *maxmins, vec3_t position);

//void draw_test_DrawVertexData(vec3_t position, vertex_data_t *data);

void draw_DrawConsole();

//void draw_DrawConsoleChar(char c, vec2_t pos);

//void draw_DrawLight(light_position_data *position_data, light_params *lparams, int draw_limits);

//void draw_debug_DrawLights(int b_draw_limits);

//void draw_DrawScreenTiles();

//void draw_ClearScreenTiles();

//void draw_DrawFrustum(frustum_t *frustum);

//void draw_DrawArmature(vec3_t position);

//void draw_DrawArmatures();

//void draw_SetDebugDrawBuffer();

PEWAPI void draw_Enable(int param);

PEWAPI void draw_Disable(int param);

//PEWAPI void draw_SetBlendMode(int blend_mode);

#include "draw.inl"

#ifdef __cplusplus
}
#endif




#endif /* DRAW_H */















