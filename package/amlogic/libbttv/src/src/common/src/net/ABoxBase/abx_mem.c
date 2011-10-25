/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the realization of ABoxBase memory functions
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/


#include "abx_common.h"
#include "abx_mem.h"
#include "abx_dbg.h"
#include <memory.h>
#include <stdio.h>
#ifdef AVOS
    #include "avmalloc.h"
#endif

#ifdef ABOX_MEM_CHECK_EN

u32_t g_abx_mem_used = 0;
int g_abx_mem_his_used = 0;

typedef struct abx_mem_struct_tag
{
	u32_t magic;
	u32_t size;
	char file[10];
	INT16U line;
}abx_mem_struct_type;


#define		ABX_MEM_HIS_SIZE		0x20000

abx_mem_struct_type *g_abx_mem_his[ABX_MEM_HIS_SIZE] = {0};
	
static u32_t mem_magic_head = 0x3e3d3d68; // h==>
static u32_t mem_magic_tail = 0x743d3d3c; // <==t

static u32_t mem_free_magic_head = 0x3e213d68; // h!=>
static u32_t mem_free_magic_tail = 0x743d213c; // <=!t

#define		ABX_MIN_MEM_PTR		0x2280000
#define		ABX_MAX_MEM_PTR		0x3000000

void *abx_mem_check_malloc( size_t size, const char* file, INT16U line)
{
	INT8U *ret;
	size_t new_size;
	char *sp;
	int i;
    DECLARE_CPU_SR;

	new_size = size + 24;
	
	ret = AVMem_kmalloc(new_size);

	if(ret)
	{
		if ((u32_t)ret % 4)
		{
			printf("abx_mem_malloc1\n");
			while(1);
		}
			
		if((u32_t)ret < ABX_MIN_MEM_PTR)
		{
			printf("abx_mem_malloc2\n");
			while(1);
		}
		
		if((u32_t)ret >= ABX_MAX_MEM_PTR)
		{
			printf("abx_mem_malloc3\n");
			while(1);
		}

		//his
		OS_ENTER_CRITICAL();
		for(i=0; i< ABX_MEM_HIS_SIZE; i++)
		{
			if(g_abx_mem_his[i] == NULL)
			{
				g_abx_mem_his[i] = (abx_mem_struct_type*)ret;
				g_abx_mem_his_used++;
				break;
			}
		}
		g_abx_mem_used += size;
		OS_EXIT_CRITICAL();
		
		if(i >= ABX_MEM_HIS_SIZE)
		{
			printf("abx_mem_malloc(his1)\n");
			while(1);
		}


		memcpy(ret, &mem_magic_head, 4);
		
		ret += 4;

		*(u32_t*)ret = size;

		ret += 4;
		
		sp = strrchr((char*)file, '/');
		if(!sp)
			sp = (char*)file;
		_rstrncpy((char*)ret, sp, 10);

		ret += 10;
		
		*(INT16U*)ret = line;

		ret += 2;
		
		memcpy(ret + size, &mem_magic_tail, 4);

	} 
	
	return ret;
}

void *abx_mem_check_calloc( size_t nmemb, size_t size, const char* file, INT16U line)
{
	void *ret;
	
	ret = abx_mem_check_malloc(nmemb * size, file, line);

	if(ret)
		memset(ret, 0, nmemb * size);

	return ret;
}

void *abx_mem_check_realloc( void *buf, size_t size, const char* file, INT16U line)
{
	size_t old_size;
	INT8U* ptr = buf;
	INT8U* ret;
	char *sp;
	int i;
    DECLARE_CPU_SR;

	if(ptr)
	{
		if ((u32_t)ptr % 4)
		{
			printf("abx_mem_realloc1\n");
			while(1);
		}

		ptr -= 20;
			
		if((u32_t)ptr < ABX_MIN_MEM_PTR)
		{
			printf("abx_mem_realloc2\n");
			while(1);
		}
		
		if((u32_t)ptr >= ABX_MAX_MEM_PTR)
		{
			printf("abx_mem_realloc3\n");
			while(1);
		}
		
		if(memcmp(ptr, &mem_magic_head, 4))
		{
			printf("abx_mem_realloc4\n");
			while(1);
		}

		old_size = *(u32_t*)(ptr+4);
		
		if(memcmp(ptr + 20 + old_size, &mem_magic_tail, 4))
		{
			printf("abx_mem_realloc5\n");
			while(1);
		}

		if(g_abx_mem_used < old_size)
		{
			printf("abx_mem_realloc6\n");
			while(1);
		}

		//his
		OS_ENTER_CRITICAL();
		for(i=0; i< ABX_MEM_HIS_SIZE; i++)
		{
			if(g_abx_mem_his[i] == (abx_mem_struct_type*)ptr)
			{
				break;
			}
		}
		OS_EXIT_CRITICAL();
		if(i >= ABX_MEM_HIS_SIZE)
		{
			printf("abx_mem_realloc(his1)\n");
			while(1);
		}
	}

	ret = AVMem_krealloc(ptr, size);

	if(ret)
	{
		if ((u32_t)ret % 4)
		{
			printf("abx_mem_realloc7\n");
			while(1);
		}
			
		if((u32_t)ret < ABX_MIN_MEM_PTR)
		{
			printf("abx_mem_realloc8\n");
			while(1);
		}
		
		if((u32_t)ret >= ABX_MAX_MEM_PTR)
		{
			printf("abx_mem_realloc9\n");
			while(1);
		}
		
		//his
		OS_ENTER_CRITICAL();
		if(ptr)
		{
			if(i >= ABX_MEM_HIS_SIZE)
			{
				printf("abx_mem_realloc(his2)\n");
				while(1);
			}
			g_abx_mem_his[i] = (abx_mem_struct_type*)ret;
			g_abx_mem_used += size - old_size;
			OS_EXIT_CRITICAL();
		}
		else
		{
			for(i=0; i< ABX_MEM_HIS_SIZE; i++)
			{
				if(g_abx_mem_his[i] == NULL)
				{
					g_abx_mem_his[i] = (abx_mem_struct_type*)ret;
					g_abx_mem_his_used++;
					break;
				}
			}
			g_abx_mem_used += size;
			OS_EXIT_CRITICAL();
		
			if(i >= ABX_MEM_HIS_SIZE)
			{
				printf("abx_mem_realloc(his3)\n");
				while(1);
			}
		}
		//


		memcpy(ret, &mem_magic_head, 4);
		
		ret += 4;

		*(u32_t*)ret = size;

		ret += 4;
		
		sp = strrchr((char*)file, '/');
		if(!sp)
			sp = (char*)file;
		_rstrncpy((char*)ret, sp, 10);

		ret += 10;
		
		*(INT16U*)ret = line;

		ret += 2;
		
		memcpy(ret + size, &mem_magic_tail, 4);

	}

	return ret;
}

void abx_mem_check_free( void *buf)
{
	size_t old_size;
	INT8U* ptr = buf;
	int i;
    DECLARE_CPU_SR;

	if(ptr == NULL)
	{
		printf("abx_mem_free1\n");
		while(1);
	}

	if ((u32_t)ptr % 4)
	{
		printf("abx_mem_free2\n");
		while(1);
	}

	ptr -= 20;
		
	if((u32_t)ptr < ABX_MIN_MEM_PTR)
	{
		printf("abx_mem_free3\n");
		while(1);
	}
	
	if((u32_t)ptr >= ABX_MAX_MEM_PTR)
	{
		printf("abx_mem_free4\n");
		while(1);
	}
	
	if(memcmp(ptr, &mem_magic_head, 4))
	{
		printf("abx_mem_free5\n");
		while(1);
	}
	
	*(u32_t*)(ptr) = mem_free_magic_head;

	old_size = *(u32_t*)(ptr+4);
	
	if(memcmp(ptr + 20 + old_size, &mem_magic_tail, 4))
	{
		printf("abx_mem_free6\n");
		while(1);
	}
	
	memcpy(ptr + 20 + old_size, &mem_free_magic_tail, 4);

	if(g_abx_mem_used < old_size)
	{
		printf("abx_mem_free7\n");
		while(1);
	}
	
	//his
	OS_ENTER_CRITICAL();
	for(i=0; i< ABX_MEM_HIS_SIZE; i++)
	{
		if(g_abx_mem_his[i] == (abx_mem_struct_type*)ptr)
		{
			g_abx_mem_his[i] = NULL;
			g_abx_mem_his_used--;
			break;
		}
	}
	g_abx_mem_used -= old_size;
	OS_EXIT_CRITICAL();
	
	if(i >= ABX_MEM_HIS_SIZE)
	{
		printf("abx_mem_free(his1)\n");
		while(1);
	}

	AVMem_free(ptr);

}

INT32U abx_mem_get_used()
{
	INT32U ret;
    DECLARE_CPU_SR;
	
	OS_ENTER_CRITICAL();
	ret = g_abx_mem_used;
	OS_EXIT_CRITICAL();

	return ret;
}

#else //ABOX_MEM_CHECK_EN

INT32U abx_mem_get_used()
{
	return 0;
}

#define		DYNC_MEM_INIT


#define	MEM_SMALL			64
#define	MEM_SMALL_CT		256
	
#define	MEM_MID				256		
#define	MEM_MID_CT			64
	
#define	MEM_LARGE			1024		
#define	MEM_LARGE_CT		16

#define	MEM_SUPPER_SIZE	    (1024 * 512)


#define	ALINE4BYTE(nbyte)	( ((nbyte) + 3) & (~3) )
#define	OPYDWORDS(nbyte)	( ((nbyte) + 3) >> 2 )

#ifndef uint32
#define uint32 unsigned int
#endif

typedef struct mem_supper_blk{
	uint32 used;
	struct mem_supper_blk* next;
	struct mem_supper_blk* prv;
	size_t size;
}msblk;




//----------------------------//
#ifdef	DYNC_MEM_INIT

static uint32 (*mem_small)[OPYDWORDS(MEM_SMALL)] = NULL;

static uint32 (*mem_mid)[OPYDWORDS(MEM_MID)] = NULL;

static uint32 (*mem_large)[OPYDWORDS(MEM_LARGE)] = NULL;

static uint32 *mem_supper = NULL;

#else
	
static uint32 mem_small[MEM_SMALL_CT][OPYDWORDS(MEM_SMALL)];

static uint32 mem_mid[MEM_MID_CT][OPYDWORDS(MEM_MID)];

static uint32 mem_large[MEM_LARGE_CT][OPYDWORDS(MEM_LARGE)];

static uint32 mem_supper[OPYDWORDS(MEM_SUPPER_SIZE)];

#endif

static uint32* free_large[MEM_LARGE_CT] = {NULL};
static uint32* free_mid[MEM_MID_CT] = {NULL};
static uint32* free_small[MEM_SMALL_CT] = {NULL};

static int free_small_ct = 0;
static int free_mid_ct = 0;
static int free_large_ct = 0;

static msblk * mshead = NULL;

/////////////////////////////////////////////////////////////////////



static size_t getmblksize(void *ptr)
{
	if((uint32*)ptr >= mem_small[0] && (uint32*)ptr <= mem_small[MEM_SMALL_CT-1])
	{
		return ALINE4BYTE(MEM_SMALL);
	}

	if((uint32*)ptr >= mem_mid[0] && (uint32*)ptr <= mem_mid[MEM_MID_CT-1])
	{
		return ALINE4BYTE(MEM_MID);
	}

	if((uint32*)ptr >= mem_large[0] && (uint32*)ptr <= mem_large[MEM_LARGE_CT-1])
	{
		return ALINE4BYTE(MEM_LARGE);
	}

	if((char*)ptr >= (char*)mem_supper + ALINE4BYTE(sizeof(msblk)) && (uint32*)ptr <= &mem_supper[OPYDWORDS(MEM_SUPPER_SIZE)-1])
	{
		return ((msblk *)((char*)ptr - ALINE4BYTE(sizeof(msblk))))->size;
	}

	return -1;
		
}

void abx_mem_init()
{
#ifdef ABOX_MEM_ALLOC
    register int i;

#ifdef	DYNC_MEM_INIT
#ifdef AVOS

	mem_small = AVMem_kmalloc(MEM_SMALL_CT * ALINE4BYTE(MEM_SMALL));
	ABX_ASSERT(mem_small);
	mem_mid = AVMem_kmalloc(MEM_MID_CT * ALINE4BYTE(MEM_MID));
	ABX_ASSERT(mem_mid);
	mem_large = AVMem_kmalloc(MEM_LARGE_CT * ALINE4BYTE(MEM_LARGE));
	ABX_ASSERT(mem_large);
	mem_supper = AVMem_kmalloc(ALINE4BYTE(MEM_SUPPER_SIZE));
	ABX_ASSERT(mem_supper);
	
#else

	mem_small = malloc(MEM_SMALL_CT * ALINE4BYTE(MEM_SMALL));
	ABX_ASSERT(mem_small);
	mem_mid = malloc(MEM_MID_CT * ALINE4BYTE(MEM_MID));
	ABX_ASSERT(mem_mid);
	mem_large = malloc(MEM_LARGE_CT * ALINE4BYTE(MEM_LARGE));
	ABX_ASSERT(mem_large);
	mem_supper = malloc(ALINE4BYTE(MEM_SUPPER_SIZE));
	ABX_ASSERT(mem_supper);
	
#endif
#endif

	for(i=0; i<MEM_SMALL_CT; i++)
	{
		free_small[i] = mem_small[i];
	}
	free_small_ct = MEM_SMALL_CT;
	
	for(i=0; i<MEM_MID_CT; i++)
	{
		free_mid[i] = mem_mid[i];
	}
	free_mid_ct = MEM_MID_CT;
	
	for(i=0; i<MEM_LARGE_CT; i++)
	{
		free_large[i] = mem_large[i];
	}
	free_large_ct = MEM_LARGE_CT;

	mshead = (msblk*)mem_supper;
	mshead->used = 0;
	mshead->size = ALINE4BYTE(MEM_SUPPER_SIZE) - ALINE4BYTE(sizeof(msblk));
	mshead->next = NULL;
	mshead->prv = NULL;
#endif
}

void *abx_mem_calloc( size_t nmemb, size_t size) {
    void *region = abx_malloc(nmemb * size);
	
	if( region )
		memset(region, 0, nmemb * size);
	
    return region;
}

void *abx_mem_malloc( size_t size) {
	void *region;
	msblk* b;

#ifdef	DYNC_MEM_INIT
	ABX_ASSERT(mem_small && mem_mid && mem_large && mem_supper);
#endif

	size = ALINE4BYTE(size);

	if(size <= ALINE4BYTE(MEM_SMALL))
	{
		if(free_small_ct)
		{
			region = free_small[--free_small_ct];
			ABX_ASSERT(region);
			free_small[free_small_ct] = NULL;
			return region;
		}
	}
	
	if(size <= ALINE4BYTE(MEM_MID))
	{
		if(free_mid_ct)
		{
			region = free_mid[--free_mid_ct];
			ABX_ASSERT(region);
			free_mid[free_mid_ct] = NULL;
			return region;
		}
	}
	
	if(size <= ALINE4BYTE(MEM_LARGE))
	{
		if(free_large_ct)
		{
			region = free_large[--free_large_ct];
			ABX_ASSERT(region);
			free_large[free_large_ct] = NULL;
			return region;
		}
	}

	for(b=mshead; b; b=b->next)
	{
		if(!b->used && b->size >= size)
		{
			if(b->size > size + ALINE4BYTE(sizeof(msblk)))
			{
				msblk *n = (msblk*)((char*)b + size + ALINE4BYTE(sizeof(msblk)));
				n->used = 0;
				n->size = b->size - size - ALINE4BYTE(sizeof(msblk));
				n->next = b->next;
				if(	b->next )
					b->next->prv=n;
				n->prv = b;
				b->next = n;
				b->size = size;
			}
			b->used = 1;
			return (char*)b + ALINE4BYTE(sizeof(msblk));
		}
	}
	
    return NULL;
}

void *abx_mem_realloc( void *ptr, size_t size) {
	size_t blksize;
    void *region;
	
#ifdef	DYNC_MEM_INIT
	ABX_ASSERT(mem_small && mem_mid && mem_large && mem_supper);
#endif
	
	if(ptr == NULL)
		return abx_malloc(size);
	
	blksize = getmblksize(ptr);
	ABX_ASSERT(blksize != -1);
	
	if(blksize >= size)
		return ptr;

	if((region = abx_malloc(size * 4 / 3)) != NULL)
		memcpy(region, ptr, blksize);

	abx_free(ptr);
    
    return region;
}

void abx_mem_free( void *ptr) {

#ifdef	DYNC_MEM_INIT
	ABX_ASSERT(mem_small && mem_mid && mem_large && mem_supper);
#endif

	if((uint32*)ptr >= mem_small[0] && (uint32*)ptr <= mem_small[MEM_SMALL_CT-1])
	{
		ABX_ASSERT(free_small_ct < MEM_SMALL_CT);
		free_small[free_small_ct++] = (uint32*)ptr;
		return;
	}

	if((uint32*)ptr >= mem_mid[0] && (uint32*)ptr <= mem_mid[MEM_MID_CT-1])
	{
		ABX_ASSERT(free_mid_ct < MEM_MID_CT);
		free_mid[free_mid_ct++] = (uint32*)ptr;
		return;
	}

	if((uint32*)ptr >= mem_large[0] && (uint32*)ptr <= mem_large[MEM_LARGE_CT-1])
	{
		ABX_ASSERT(free_large_ct < MEM_LARGE_CT);
		free_large[free_large_ct++] = (uint32*)ptr;
		return;
	}

	if((char*)ptr >= (char*)mem_supper + ALINE4BYTE(sizeof(msblk)) && (uint32*)ptr <= &mem_supper[OPYDWORDS(MEM_SUPPER_SIZE)-1])
	{
		msblk *b,*p,*n;

		b = (msblk *)((char*)ptr - ALINE4BYTE(sizeof(msblk)));
		b->used = 0;
		
		if((n = b->next) != NULL && !n->used)
		{
			b->size += ALINE4BYTE(sizeof(msblk)) + n->size;
			b->next = n->next;			
			if((n = n->next) != NULL)
				n->prv = b;
		}
		if((p = b->prv) != NULL && !p->used)
		{
			if((n = b->next) != NULL)
				n->prv = p;
			p->next = n;
			p->size += ALINE4BYTE(sizeof(msblk)) + b->size;
		}
		return;
	}

	ABX_ASSERT(0);
}

void abx_mem_exit()
{
#ifdef ABOX_MEM_ALLOC
    //check for memory leak


	//free the dync memory
#ifdef	DYNC_MEM_INIT
#ifdef AVOS
	if(mem_small)
	{
		AVMem_free(mem_small);
		mem_small = NULL;
	}
	if(mem_mid)
	{
		AVMem_free(mem_mid);
		mem_mid = NULL;
	}
	if(mem_large)
	{
		AVMem_free(mem_large);
		mem_large = NULL;
	}
	if(mem_supper)
	{
		AVMem_free(mem_supper);
		mem_supper = NULL;
	}
#else
	if(mem_small)
	{
		free(mem_small);
		mem_small = NULL;
	}
	if(mem_mid)
	{
		free(mem_mid);
		mem_mid = NULL;
	}
	if(mem_large)
	{
		free(mem_large);
		mem_large = NULL;
	}
	if(mem_supper)
	{
		free(mem_supper);
		mem_supper = NULL;
	}

#endif

#endif

#endif
}


#endif //ABOX_MEM_CHECK_EN

