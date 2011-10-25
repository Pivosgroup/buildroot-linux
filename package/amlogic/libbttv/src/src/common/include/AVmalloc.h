/*******************************************************************
 * 
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: 
 *
 *  Author: Amlogic Software
 *  Created: Fri Nov 11 00:58:05 2005
 *
 *******************************************************************/

#ifndef _AVMALLOC_H_

#ifndef AVMem_malloc
#include <memory.h>
#include <stdlib.h>
#define AVMem_malloc malloc
#define AVMem_free free
#define AVMem_calloc calloc
#define AVMem_realloc realloc
#endif

#endif /* _AVMALLOC_H_ */
