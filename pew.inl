#ifndef PEW_INLINES_INL
#define PEW_INLINES_INL
#include "conf.h"

typedef union
{
	unsigned long long lp;
	unsigned int ip[2]; 
}ulong_union;

PEWAPI unsigned long long pew_GetCycleCount()
{
	ulong_union ll;
	asm(
			"rdtsc\n"
			"movl %%eax, %[p]\n"
			"movl %%edx, %[q]\n"
			: [p] "=m" (ll.ip[0]), [q] "=m" (ll.ip[1]):  
	   );
	return ll.lp;
}

#endif /* ifndef PEW_INLINES_INL */
