/*****************************************************************
**                                                             	                                             **
**  Copyright (C) 2008 Sympeer,Inc.                            	                                **
**  All rights reserved                                        	                                             **
**        Filename : T9_Event.h /Project:AVOS		                                             **
**        Revision : 1.0                                       	                                             **
**                                                             	                                             **
*****************************************************************/
#ifndef T9_ENENT_H
#define T9_ENENT_H

#include "af_engine.h"

struct T9_INDEX
{
	char *NumberKey_Seq;
	char *Chinese_Spell;
	char *Chinese_mb;
};

struct WORD_INDEX
{
	char cCh[4] ;
	INT32U ioffset ;
	INT32U iStrlen ;
} ;

#define MB_MAX_COUNT	12

void free_mb( struct T9_INDEX * T9_Index_Out , INT8U unBeginPos) ;
INT8S Spell_Process( char * InputNumStr, struct T9_INDEX * T9_Index_Out );
INT8U GetNextCH( char * cpFind , char * cpFindeds , char * cpAllFindeds , INT8U iCount ) ;
//words_lib_type: 0-movie 1-music 2-stock
char * get_word( char ** p_word , char * c_version, int words_lib_type );
INT8U get_words_lib_type( );

INT8U get_stroke_ch( char * cpInput , char ** cpFindeds ) ;
int open_storke_file() ;
void close_stroke_file() ;
void  close_assn_file() ;
#endif
