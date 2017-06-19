
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>

#ifdef _DEBUG

#define nerr   printf
#define nwarn  printf
#define ninfo  printf
#define DEBUGASSERT

#else

#define nerr   
#define nwarn 
#define ninfo
#define DEBUGASSERT

#endif


#endif

