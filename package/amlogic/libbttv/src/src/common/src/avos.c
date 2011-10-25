#include "includes.h"
#include <sys/time.h>
#include <malloc.h>

INT32U  AVTimeGet (void)
{
     struct timeval tv;
     gettimeofday(&tv, NULL);
     return tv.tv_sec*1000L+tv.tv_usec/1000L;
}

void *AVMem_malloc(size_t size)
{
    return malloc(size);
}

/*void * AVMem_malloc(size_t size , size_t size_1 )
{
	return malloc( size * size_1) ;
}

void AVMem_realloc( void * mem , size_t size )
{
        return relloc( mem , size ) ;
}*/

void AVMem_free(void * buf)
{
    free(buf);
}
