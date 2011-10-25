/*****************************************************************
**                                                              **
**  Copyright (C) 2008 Amlogic,Inc.                             **
**  All rights reserved                                         **
**        Filename : T9_Event.c /Project:AVOS		            **
**        Revision : 1.0                                        **
**                                                              **
*****************************************************************/
//#include <includes.h>
//#include <sysdefine.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "../include/T9_Event.h"
#include "T9_mb.h"
#include "T9_Index.h"

#include "t9wordstring.h"
#include "t9wordstring_song.h"
#include "t9wordstring_cinema.h"
#include "stroke.h"

#define SPEC            "\n"
#define WORD_COUNT		60
#define BUFF_SIZE		1024 * 24
#define FCLOSE(f)       close(f);f=-1;
#define 	_AVMem_free(p)  do{ if( p ){AVMem_free(p);p=NULL;}}while(0)

static int f_IndexString = -1 ;
static int f_stroke_bin = -1 ; 

typedef struct
{
	char* data;
	int flag; // 1-need free memory
	char* version;
}  WORDS_LIB_INFO;

#define DEFAULT_WORDSLIB_VER "2009-05-26"

static WORDS_LIB_INFO g_words_libs[3] =	
{
	{(char*) t9wordstring_cinema_ary, 0,  DEFAULT_WORDSLIB_VER},
	{(char*)t9wordstring_song_ary, 0, DEFAULT_WORDSLIB_VER},
	{(char*)t9wordstring_ary, 0, DEFAULT_WORDSLIB_VER},
};

static char * s_cp_IndexString = NULL ;
static char * s_cp_IndexString_cur = NULL ;
static char * s_cp_stroke_bin = NULL ;
static char * s_cp_stroke_bin_cur = NULL ;
static INT8U	s_b_mem = 0 ;

void close_Index_file();

int  T9SetWordsLib(int lib_type, char* lib_data,  int data_len, int flag)
{
	if (lib_type < 0 || lib_type > 2 || data_len < 12)
		return -1;

	if (g_words_libs[lib_type].data == lib_data) //same lib
		return 0;

	if (g_words_libs[lib_type].flag != 0)
	{
		AVMem_free(g_words_libs[lib_type].data);
	}

	if (lib_data)
	{
		g_words_libs[lib_type].data = lib_data;
		g_words_libs[lib_type].flag = flag;
		g_words_libs[lib_type].version = &lib_data[data_len-11];
	}
	else
	{
		if (lib_type == 0)
			g_words_libs[lib_type].data = (char*)t9wordstring_cinema_ary;
		else if (lib_type == 1)
			g_words_libs[lib_type].data = (char*)t9wordstring_song_ary;
		else
			g_words_libs[lib_type].data = (char*)t9wordstring_ary;
		g_words_libs[lib_type].flag = 0;
		g_words_libs[lib_type].version = DEFAULT_WORDSLIB_VER;
	}
	close_Index_file(); //need reload lib
	return 0;
}

char*  T9GetWordsLibVer(int lib_type)
{
	if (lib_type < 0 || lib_type > 2)
		return NULL;
	
	return g_words_libs[lib_type].version;
}

void close_stroke_file()
{
	if( 0 <= f_stroke_bin )
	{
		f_stroke_bin = -1 ;
	}

	s_cp_stroke_bin = NULL ;
	s_cp_stroke_bin_cur = NULL ;
}

void close_Index_file()
{
	if( 0 <= f_IndexString )
	{
		f_IndexString = -1 ;
	}
	 
 	if(1 == s_b_mem &&s_cp_IndexString)
	{
		_AVMem_free( s_cp_IndexString ) ;
		s_cp_IndexString_cur = NULL ;
		s_b_mem = 0 ;
	}
	else
 	{
 		s_cp_IndexString = NULL ;
		s_cp_IndexString_cur = NULL ;
	}
 }

static void get_point( const int f , char ** cpBegingPos , char ** cpCurPos )
{
	if( f == f_IndexString )
	{
		*cpCurPos = s_cp_IndexString_cur ;
		*cpBegingPos = s_cp_IndexString ;
	}
	else if( f == f_stroke_bin )
	{
		*cpCurPos = s_cp_stroke_bin_cur ;
		*cpBegingPos = s_cp_stroke_bin ;
	}
}

static void set_point( const int f , int iOffset , int iBeginPos )
{
	if( f == f_IndexString )
	{
		if( SEEK_SET == iBeginPos )
		{
			s_cp_IndexString_cur = s_cp_IndexString ;
			s_cp_IndexString_cur += iOffset ;
		}
		else if( SEEK_CUR == iBeginPos )
			s_cp_IndexString_cur += iOffset;
	}
	else if( f == f_stroke_bin )
	{
		if( SEEK_SET == iBeginPos )
		{
			s_cp_stroke_bin_cur = s_cp_stroke_bin ;
			s_cp_stroke_bin_cur += iOffset ;
		}
		else if( SEEK_CUR == iBeginPos )
			s_cp_stroke_bin_cur += iOffset ;
	}
	return ;
}


static int T9_read( int f , void * cpOut , int iReadLen )
{
	char * cpBegingPos = NULL , * cpCurPos = NULL ;
	get_point( f , &cpBegingPos , &cpCurPos ) ;
	memcpy( cpOut , cpCurPos , iReadLen ) ;
	set_point( f , iReadLen , SEEK_CUR  ) ;
	return iReadLen;	
}


static int T9_lseek( int f , long lSeekPos , long lSeekBegingPos )
{
	set_point( f , lSeekPos , lSeekBegingPos ) ;
	return 0 ;
}


void free_mb( struct T9_INDEX * T9_Index_Out , INT8U unBeginPos )
{
	for( INT8U i = unBeginPos ; i < MB_MAX_COUNT ; i++ )
	{
		if( T9_Index_Out[i].Chinese_mb )
		{
			_AVMem_free( T9_Index_Out[i].Chinese_mb ) ;
		}
	}
}

const unsigned char INDEX_FLAG = 0x00 ;

int uncompress_mb( const char * p , char * * pp_mb )
{
	if( NULL != *pp_mb )
	{
		_AVMem_free( *pp_mb ) ;
	}
	
	INT8U iCHCount = *p ;
	p++ ;

	char * cMbBuf = AVMem_malloc( iCHCount * 2 + 1 ) ;
	if( NULL == cMbBuf )
		return -1 ;
	
	char * cBUf = cMbBuf;
	memset( cMbBuf , 0x00 , iCHCount * 2 + 1 ) ;
	
	INT16S iIndex = 0 ;
	INT8U bPre = 1 ; 

	for( INT8U j = 0 ; j < iCHCount ; j++ )
	{
		if( INDEX_FLAG == *p )
		{
			if( 0 == bPre )
				bPre = 1 ;
			else
				bPre = 0 ;
			
			p++ ;
		}

		if( 1 == bPre )
		{
			INT8U piCNH = *p ;
			*cBUf = piCNH ;
			p++ ;
			cBUf++ ;
			INT8U piCNL = *p ;
			*cBUf = piCNL ;
			p++ ;
			cBUf++ ;
			iIndex = ((piCNH&0x00ff)<<8);
			iIndex += piCNL ;
		}
		else
		{
			INT16U iCh = iIndex + (INT8S)(*p) ; 
			*cBUf = iCh>>8 ;
			cBUf ++ ;
			*cBUf = iCh ;
			cBUf ++ ;
			p++ ;
		}
	}
	*pp_mb = cMbBuf ;
	return 1 ;
}

INT8S  Spell_Process( char * InputNumStr, struct T9_INDEX * T9_Index_Out )
{
	if( '\0' == *InputNumStr ||
		NULL == T9_Index_Out )
	{
		return -1 ;
	}

	INT8U Out_Number = 0 ;

	struct T9_INDEX  const *cpHZ,*cpHZedge;
    	char i,j,cInputStrLength;

    j=0; 
    cInputStrLength = strlen(InputNumStr);
    cpHZ=&(T9_Index[0]); 
    cpHZedge=T9_Index+sizeof(T9_Index)/sizeof(T9_Index[0]);

	INT8U b_matching = FALSE ;
    while(cpHZ < cpHZedge) 
    {
        for(i=0;i<cInputStrLength;i++)
        {
            if(*(InputNumStr+i)!=*((*cpHZ).NumberKey_Seq+i))
            {
                if (i+1 > j)
                {
                    j=i+1; 
                }
                break; 
            }           
        }
        if( ( i == cInputStrLength ) && ( Out_Number < MB_MAX_COUNT ) && ( strlen( cpHZ->Chinese_Spell ) >= cInputStrLength ) )
        {
			T9_Index_Out[Out_Number].Chinese_Spell = cpHZ->Chinese_Spell;
			T9_Index_Out[Out_Number].NumberKey_Seq = cpHZ->NumberKey_Seq ;
			if( 1 == uncompress_mb( cpHZ->Chinese_mb , &T9_Index_Out[Out_Number].Chinese_mb ) )
            	Out_Number++;
        }
        cpHZ++;
    }	

	if( Out_Number > 0 )
		free_mb( T9_Index_Out , Out_Number ) ;
	return (Out_Number);
}


INT8U GetNextCH( char * cpFind , char * cpFindeds , char * cpAllFindeds , INT8U iCount )
{
	
	if( 0 > f_IndexString )
	{
		INT8U i_word_lib_type = get_words_lib_type() ;
		s_cp_IndexString = g_words_libs[i_word_lib_type].data;
		s_cp_IndexString_cur = s_cp_IndexString ;
		f_IndexString = 255 ;
	}
	else
	{
		if( -1 == T9_lseek( f_IndexString , (long)0 , SEEK_SET ) )
		{
			close_Index_file() ;
			return 0 ;
		}
	}

	INT32U iSize = 0 ;
	int iReadLen = T9_read( f_IndexString , (void *)&iSize , 2 ) ;
	if( 2 != iReadLen )
	{
		close_Index_file() ;
		return 0 ;
	}

	INT32U iStrLen = iSize * sizeof(struct WORD_INDEX) ;
	char * cBuff = AVMem_malloc( iStrLen + 1) ; 
	if( NULL == cBuff )
		return 0 ;
	
	iReadLen = 0 ;
	while( iReadLen < iSize * sizeof( struct WORD_INDEX ) )
	{
		int iRead = T9_read( f_IndexString , (void *)( cBuff + iReadLen ) , iSize * sizeof( struct WORD_INDEX) - iReadLen ) ;
		if( iRead > 0 )
			iReadLen += iRead ;
		else
			break ;
	}

	if( iReadLen <= 0 )
	{
		close_Index_file() ;
		_AVMem_free( cBuff ) ;
		return 0 ;
	}

	*( cBuff + iReadLen ) = 0 ;
	
	struct WORD_INDEX * pIndex = (struct WORD_INDEX *)cBuff ;
        int i = 0;
        for( i = 0 ; i < iSize ; i++ )
	{
		if( 0 == memcmp( pIndex->cCh , cpFind , 2 ) )
		{
			break ;
		}
		pIndex++ ;
	}
	
	int iFinded = 0 ;

	if( i >= iSize )
	{
		_AVMem_free( cBuff ) ;
		if( iFinded < iCount && strlen( cpFind ) > 2 )
		{
			GetNextCH( cpFind + strlen( cpFind ) -2 , cpFindeds + iFinded , cpFindeds , iCount - iFinded  ) ;
			close_Index_file() ;
		}
		else
		{
			return 0;
		}
	}

	if( 0 > f_IndexString )
	{	
		if( NULL != cBuff )
		{
			_AVMem_free( cBuff ) ;
		}
		return strlen( cpAllFindeds )/2 ;		
	}
	// be finded
	if( -1 == T9_lseek( f_IndexString, (long)pIndex->ioffset , SEEK_SET) )
	{
		if( -1 == T9_lseek( f_IndexString, (long)pIndex->ioffset , SEEK_SET) )
			_AVMem_free( cBuff ) ;
		return 0 ;
	}
	
	iReadLen = 0 ;
	INT32U iIndexLen = pIndex->iStrlen ;
	_AVMem_free( cBuff ) ;
	cBuff = AVMem_malloc( iIndexLen + 1 ) ;
	if( NULL == cBuff )
		return 0 ;
	
	while( iReadLen < iIndexLen )
	{
		int iRead = T9_read( f_IndexString , (void *)( cBuff + iReadLen ) , iIndexLen - iReadLen ) ;
		if( iRead > 0 )
			iReadLen += iRead ;
		else
			break ;
	}

	if( iReadLen <= 0 )
	{
		if( -1 == T9_lseek( f_IndexString, (long)pIndex->ioffset , SEEK_SET) )
		_AVMem_free( cBuff ) ;
		return 0 ;
	}
	  
	*( cBuff + iReadLen ) = 0 ;

	//read success
	char * cpStoken = NULL ;		
	cpStoken = strtok( cBuff , SPEC ) ;
	*cpFindeds = 0x00 ;
	while( NULL != cpStoken )
	{
		char * cDest = strstr( cpStoken , cpFind ) ;
		if( cpStoken == cDest 
			&& strlen( cpFind ) < strlen( cpStoken ) 
			&& iFinded + 2 < iCount )
		{
                    int i = 0;
                        for( i = 0 ; i < strlen( cpAllFindeds ) ; i+=2 )
			{
				if( 0 == memcmp( cpStoken + strlen( cpFind ) , cpAllFindeds + i , 2 ) )
				{
					break ;
				}
			}

			if( i >= strlen( cpAllFindeds ) )
			{
				memcpy( cpFindeds + iFinded , cpStoken + strlen( cpFind ) , 2 );
				iFinded += 2 ;
				*( cpFindeds + iFinded ) = 0x00 ;
			}

			if( iFinded + 2 > iCount )
				break ;
		}
	
		cpStoken = strtok( NULL , SPEC ) ;
	}

	_AVMem_free( cBuff ) ;
	cBuff = NULL ;
	
	if( iFinded < iCount && strlen( cpFind ) > 2 )
	{
		GetNextCH( cpFind + strlen( cpFind ) -2 , cpFindeds + iFinded , cpFindeds , iCount - iFinded  ) ;
	}

	return strlen( cpAllFindeds )/2 ;		
}

INT8U get_stroke_ch( char * cpInput , char ** cpFindeds )
{
	if( f_stroke_bin < 0 )
	{
		if( open_storke_file() < 0)
			return 0 ;
	}
	else
	{
		if( -1 == T9_lseek( f_stroke_bin , (long)0 , SEEK_SET) ) 
		{
			close_stroke_file() ;
			return 0 ;
		}
	}

	INT32U iSize = 0 ;
	int iReadLen = T9_read( f_stroke_bin , (void *)&iSize , 2 ) ;
	if( 2 != iReadLen )
	{
		close_stroke_file() ;
		return 0 ;
	}

	INT32U iStrLen = iSize * sizeof(struct WORD_INDEX) ;
	char * cBuff = (char * )AVMem_malloc( iStrLen + 1) ; 
	if( NULL == cBuff )
		return 0 ;
	
	iReadLen = 0 ;
	while( iReadLen < iStrLen )
	{
		int iRead = T9_read( f_stroke_bin , (void *)( cBuff + iReadLen ) , iSize * sizeof( struct WORD_INDEX ) - iReadLen ) ;
		if( iRead > 0 )
			iReadLen += iRead ;
		else
			break ;
	}

	if( iReadLen <= 0 )
	{
		close_stroke_file() ;
		_AVMem_free( cBuff ) ;
		return 0 ;
	}
	*( cBuff + iReadLen ) = 0 ;
	
	char cFirst[4] ;
	if( strlen( cpInput ) < 3 )
	{
		sprintf( cFirst , "%s", "1_2" ) ;
	}
	else
	{
		memcpy( cFirst , cpInput , 3 ) ;
		cFirst[3] = 0 ;
	}

	struct WORD_INDEX * pIndex = (struct WORD_INDEX *)cBuff ;
	struct WORD_INDEX Index_1 , Index_2 ;
        int i;
        for( i = 0 ; i < iSize ; i++ )
	{
		if( 0 == memcmp( pIndex->cCh , cFirst , 3 ) )
		{
			Index_1 = *pIndex ; 
			if( strlen( cpInput ) > 2 )
				break ;
		}
		else if( strlen( cpInput ) < 3 && 0 == memcmp( pIndex->cCh , cpInput , strlen( cpInput) ) )
		{
			Index_2 = *pIndex ;
			break ;
		}

		pIndex++ ;
	}

	_AVMem_free( cBuff ) ;
	int iFinded = 0 ;

	if( i >= iSize )
	{
		return 0;
	}

	// be finded
	if( -1 == T9_lseek( f_stroke_bin , (long)Index_1.ioffset , SEEK_SET ) )
	{
		close_stroke_file() ;
		return 0 ;
	}
	
	iReadLen = 0 ;
	INT32U iIndexLen = Index_1.iStrlen ;
	cBuff = (char * )AVMem_malloc( iIndexLen + 1) ;
	if( NULL == cBuff )
		return 0 ;

	while( iReadLen < iIndexLen )
	{
		int iRead = T9_read( f_stroke_bin , (void *)( cBuff + iReadLen ) , iIndexLen - iReadLen ) ;
		if( iRead > 0 )
			iReadLen += iRead ;
		else
			break ;
	}

	if( iReadLen <= 0 )
	{
		close_stroke_file() ;
		_AVMem_free( cBuff ) ;
		return 0 ;
	}
	
	*( cBuff + iReadLen ) = 0 ;

	//read success
	if( NULL == *cpFindeds)
	{
		*cpFindeds = ( char * )AVMem_malloc( WORD_COUNT + 1 ) ;
		if( NULL == *cpFindeds )
		{
			_AVMem_free( cBuff ) ;
			return 0 ;
		}
		**cpFindeds = 0 ;
	}

	char * cpStoken = NULL ;		
	cpStoken = strtok( cBuff , SPEC ) ;
	while( NULL != cpStoken )
	{
		int iStokenLen = strlen( cpStoken ) - 2 ;
		char * cDest = strstr( cpStoken , cpInput ) ;
		if( cpStoken == cDest && 
			strlen( cpInput ) <= iStokenLen
			&& iFinded < WORD_COUNT )
		{
			memcpy( *cpFindeds + iFinded , cpStoken + iStokenLen , 2 );
			iFinded += 2 ;
			*( *cpFindeds + iFinded ) = 0 ;
		}
		else if( iFinded > WORD_COUNT )
		{
			break ;
		}
	
		cpStoken = strtok( NULL , SPEC ) ;

		if( NULL == cpStoken && iFinded < WORD_COUNT && strlen( cpInput ) < 3 )
		{
			// be finded
			if( -1 == T9_lseek( f_stroke_bin , (long)Index_2.ioffset , SEEK_SET ) ) 
			{
				close_stroke_file() ;
				break ;
			}
			
			iReadLen = 0 ;
			INT32U iIndexLen = Index_2.iStrlen ;
			_AVMem_free( cBuff  ) ;
			cBuff = (char * )AVMem_malloc( iIndexLen + 1) ;
			if( NULL == cBuff )
				break ;
			
			while( iReadLen < iIndexLen )
			{
				int iRead = T9_read( f_stroke_bin , (void *)( cBuff + iReadLen ) , iIndexLen - iReadLen ) ;
				if( iRead > 0 )
					iReadLen += iRead ;
				else
					break ;
			}
			
			if( iReadLen <= 0 )
			{
				close_stroke_file() ;
				_AVMem_free( cBuff ) ;
				break ;
			}
			
			*( cBuff + iReadLen ) = 0 ;
			cpStoken = strtok( cBuff , SPEC ) ;
		}
	}
	
	_AVMem_free( cBuff ) ;	

	return iFinded/2 ;
}


int open_storke_file()
{
	s_cp_stroke_bin = (char *)stroke_ary ;
	s_cp_stroke_bin_cur = s_cp_stroke_bin ;
	f_stroke_bin = 256 ;
	return f_stroke_bin ;
}

void close_assn_file()
{
	close_Index_file();
}

