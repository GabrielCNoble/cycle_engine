#ifndef SHADER_H
#define SHADER_H

#include "shader_types.h"
#include "draw_types.h"

#include "conf.h"
#include "includes.h"


//extern renderer_t renderer;
//extern shader_array shader_a;



#ifdef __cplusplus
extern "C"
{
#endif


PEWAPI void shader_Init(char *path);

PEWAPI void shader_Finish();

PEWAPI int shader_GetShaderFileSize(FILE *shader_file);

PEWAPI char *shader_GetShaderString(FILE *shader_file);

PEWAPI int shader_LoadShader(char *vertex_shader_name, char *fragment_shader_name, char *name);

PEWAPI int shader_GetShaderIndex(char *name);

PEWAPI static inline shader_t *shader_GetActiveShader();

PEWAPI void shader_SetShaderByIndex(int shader_index);

PEWAPI static inline void shader_SetCurrentShaderUniform1i(int uniform, int value);

PEWAPI static inline void shader_SetCurrentShaderUniform1f(int uniform, float value);

PEWAPI static inline void shader_SetCurrentShaderUniform4fv(int uniform, float *value);

PEWAPI static inline void shader_SetCurrentShaderUniformMatrix4fv(int uniform, float *value);

PEWAPI static inline void shader_SetCurrentShaderVertexAttribArray(int vertex_array);




void shader_ResizeShaderArray(int new_size);

void shader_ParseShaderAttributes(char *shader_str, int *attribs_found, int *attrib_count);

int shader_Preprocess(char **shader_str, int *flags);

int shader_ExpandInclude(char **shader_str, int start_index, int cur_index, int old_len, int *new_len, int line);

int shader_AddDefine(char *shader_str, define_t **root, int *cur_index);

int shader_CheckDefine(define_t *root, char *shader_str, int *cur_index);

int shader_FindEndif(char *shader_str, int str_len, int cur_index);

void shader_EraseDirectivesOnly(char *shader_str, int start_pos, int end_pos);

void shader_EraseInBetweenDirectives(char *shader_str, int start_pos, int end_pos);

int shader_DoPragma(char *shader_str, int str_len, int start_pos, int cur_pos, int *flags);

void shader_ReleaseDefines(define_t *root);



#ifdef __cplusplus
}
#endif


#ifndef SHADER_INL
#include "shader.inl"
#endif /* SHADER_INL */


#endif /* SHADER_H */












