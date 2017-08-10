#include "matrix_types.h"


PEWAPI void mat4_t_mult_fast(mat4_t *result, mat4_t *mat1, mat4_t *mat2)
{
	asm
	(
		"movl %0, %%ebx\n"
		"movl %1, %%esi\n"
		"movl %2, %%edi\n"
		
		".intel_syntax noprefix\n"
		
		"mov ecx, 4\n"
		
		"movups  xmm0, [edi]\n"
		"movups  xmm1, [edi + 16]\n"
		"movups  xmm2, [edi + 32]\n"
		"movups  xmm3, [edi + 48]\n"
		
		"_mat4_t_mult_fast_internal_loop:\n"
			"movups xmm4, [esi]\n"
			"add esi, 16\n"
			"dec ecx\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0\n"
			"mulps xmm5, xmm0\n"
			"movups xmm6, xmm5\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0x55\n"
			"mulps xmm5, xmm1\n"
			"addps xmm6, xmm5\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0xaa\n"
			"mulps xmm5, xmm2\n"
			"addps xmm6, xmm5\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0xff\n"
			"mulps xmm5, xmm3\n"
			"addps xmm6, xmm5\n"
		
			"movups [ebx], xmm6\n"
			"add ebx, 16\n"
			"test ecx, ecx\n"
			
			"jnz _mat4_t_mult_fast_internal_loop\n"
		
		".att_syntax prefix\n"
		:: "mr" (result), "mr" (mat1), "mr" (mat2)
		
	);
}


PEWAPI void amat4_t_mult_fast(amat4_t *result, amat4_t *mat1, amat4_t *mat2)
{
	asm
	(
		"movl %0, %%ebx\n"
		"movl %1, %%esi\n"
		"movl %2, %%edi\n"
		
		".intel_syntax noprefix\n"
		
		"mov ecx, 4\n"
		
		"movaps  xmm0, [edi]\n"
		"movaps  xmm1, [edi + 16]\n"
		"movaps  xmm2, [edi + 32]\n"
		"movaps  xmm3, [edi + 48]\n"
		
		"_loop0:\n"
			"movaps xmm4, [esi]\n"
			"add esi, 16\n"
			"dec ecx\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0\n"
			"mulps xmm5, xmm0\n"
			"movups xmm6, xmm5\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0x55\n"
			"mulps xmm5, xmm1\n"
			"addps xmm6, xmm5\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0xaa\n"
			"mulps xmm5, xmm2\n"
			"addps xmm6, xmm5\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0xff\n"
			"mulps xmm5, xmm3\n"
			"addps xmm6, xmm5\n"
		
			"movaps [ebx], xmm6\n"
			"add ebx, 16\n"
			"test ecx, ecx\n"
			
			"jnz _loop0\n"
		
		".att_syntax prefix\n"
		:: "mr" (result), "mr" (mat1), "mr" (mat2)
		
	);
}

PEWAPI void MultiplyVector4(mat4_t *mat, vec4_t *vec)
{
	vec4_t result;
	result.floats[0] = 0.0;
	result.floats[1] = 0.0;
	result.floats[2] = 0.0;
	result.floats[3] = 0.0;
	register int i;
	for(i=0; i<4; i++)
	{
		result.floats[0] += mat->floats[i][0] * vec->floats[i];
		result.floats[1] += mat->floats[i][1] * vec->floats[i];
		result.floats[2] += mat->floats[i][2] * vec->floats[i];
		result.floats[3] += mat->floats[i][3] * vec->floats[i];
	}
	
	*vec = result;
}


PEWAPI void mat4_t_vec4_t_mult(mat4_t *mat, vec4_t *vec)
{
	asm
	(
		"movl %1, %%esi\n"
		"movups (%%esi), %%xmm2\n"
		"movups 16(%%esi), %%xmm3\n"
		"movups 32(%%esi), %%xmm4\n"
		"movups 48(%%esi), %%xmm5\n"
		
		"movl %0, %%esi\n"
		"movups (%%esi), %%xmm0\n"
		
		".intel_syntax noprefix\n"
		"movups xmm1, xmm0\n"
		
		"shufps xmm0, xmm0, 0\n"
		"mulps xmm2, xmm0\n"
		
		"movups xmm0, xmm1\n"
		"shufps xmm0, xmm0, 0x55\n"
		"mulps xmm3, xmm6\n"
		
		"movups xmm0, xmm1\n"
		"shufps xmm0, xmm0, 0xaa\n"
		"mulps xmm4, xmm0\n"
		
		"shufps xmm1, xmm1, 0xff\n"
		"mulps xmm5, xmm0\n"
		
		"addps xmm2, xmm3\n"
		"addps xmm5, xmm4\n"
		"addps xmm2, xmm5\n"
		
		".att_syntax prefix\n"
		
		"movups %%xmm2, (%%esi)\n"
		
		: :"mr" (vec), "mr" (mat)
	);
}

/*PEWAPI void MatrixCopy4(mat4_t *out, mat4_t *in)
{
	asm
	(	
		
		"movl %0, %%edi\n"
		"movl %1, %%esi\n"
		
		".intel_syntax noprefix\n"
		"mov ecx, 16\n"
		"rep stosd\n"
		".att_syntax prefix"
		:: "mr" (out), "mr" (in)
	);
}*/
