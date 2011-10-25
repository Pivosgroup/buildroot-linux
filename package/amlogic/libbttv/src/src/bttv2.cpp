/*****************************************************************
**                                                              **
**  Copyright (C) 2007 Amlogic,Inc.                             **
**  All rights reserved                                         **
**                                                              **
**                                                              **
*****************************************************************/

extern "C"
{
#include <includes.h>		
#include <af_engine.h>		

#include "BttvApp_cnt.h"
#include "xfer/transfer_manager.h"

#include "transfer_def.h"
#include <AVmalloc.h>
#include "navigate_parse.h"
#include "kankan_parse.h"
#include "search_parse.h"
#include "ref_update_manager.h"

#include "netdownload_comm.h"
#include "navigate_parse.h"
#include "search_parse.h"
#include "kankan_parse.h"
#include "kankan_channel_xml.h"
#include "movie_channel_xml.h"
#include "transfer_def.h"
}
#include "q_sem.h"
#include <QFile>
#include <QDir>
#include <QMutex>
#include "XmlThread.h"
#include "MBttvObject.h"

MBttvObject mbttvObject;

XMLThread::XMLThread(QObject *parent):QThread(parent)
{
}

XMLThread::~XMLThread()
{
}

XMLThread & XMLThread::Instance()
{
    static XMLThread b;
    return b;
}

void XMLThread::sleep(unsigned long ms)
{
    QThread::msleep(ms);
}
#ifndef FALSE
#define FALSE   (0)
#endif

static int dbg_level = 0;

static int s_finished_taskid = 0;
static INT8U s_finished_show_times = 0;
static int s_b_error_1025 = FALSE ;
static char  s_DownloadPath[ 128 ] = {0} ;
static char Licence[128] = {0} ;
typedef struct{
    control_t* cntl;
    INT8U task_table_pos;
    INT8U cb_handler_pos;
    INT8U last_error_pos;
    INT8U SetOption_pos;
    INT8U GetOption_pos;
    INT8U Init_Ex_pos;
    INT8U UpdateTaskInfo_pos;
    INT8U UnloadAllTasks_pos;
    INT8U LoadAllTasks_pos;
    INT8U AddTask_pos;
    INT8U GetResult_pos;
    INT8U AddThunderTask_pos;
    INT8U StartTask_pos;
    INT8U StartPrioTask_pos;
    INT8U PauseTask_pos;
    INT8U CancelTask_pos;
    INT8U CloseTask_pos;
    INT8U GetTaskList_pos;
    INT8U GetTaskInfo_pos;
    INT8U ReservedSpace_pos;
    INT8U SetInfoPath_pos;
    INT8U SetTempPath_pos;
    INT8U GetSeedInfo_pos;
    INT8U SetStatCallback_pos;
    INT8U EnableCacheFile_pos;
    INT8U SetCustomAlloc_pos;
    INT8U Fini_pos;
    INT8U SetLicense_pos;
    INT8U GetThdTaskid_pos;
    INT8U PauseAllTask_pos;
    INT8U RestartAllTask_pos;
    INT8U CloseAllTask_pos;
    INT8U CancelAllTask_pos;
    INT8U GetLastError_pos;
    /* Add more members here */

    /*end*/
}TransferInter_t;


typedef struct{
    control_t* cntl;
    INT8U ColNickNameList_pos;
    INT8U GetRowCount_pos;
    INT8U GetColCount_pos;
    INT8U GetString_pos;
    INT8U GetRowName_pos;
    INT8U GetRowIndex_pos;
    INT8U AddRow_pos;
    INT8U DelRow_pos;
    INT8U SetString_pos;
    INT8U DelString_pos;
    INT8U Release_pos;
    /* Add more members here */

    /*end*/
}StringTable_t;


typedef struct{
    control_t* cntl;
    TransferInter_t* TransferManager;
    StringTable_t* MountInfo;
    /* Add more members here */

    /*end*/
}bttvapp_t;



#define     MAX_GET_TIMES     3
#define 	_AVMem_free(p)  do{ if( p ){AVMem_free(p);p=NULL;}}while(0)
static char* get_value(const char *s , int i_len )
{
	char *d = NULL;
	if(s)
	{
		if( 0 == i_len )
			return NULL ;
		if(NULL != ( d = (char*)AVMem_malloc(i_len + 1) ) )
		{
			memcpy( d, s , i_len );
			d[i_len] = 0 ;
		}	
	}
	return d;
}

static dl_rcmd_xml_ctx_type ** s_current_pic_ctx = NULL ;
/******************* thunder download **********************/
#define MOVIE_VERSION 2  /* 1: old version;  2: new version */
#define HD_MOVIE_CH1 0
#define HD_MOVIE_CH2 1
#define HD_MOVIE_CH3 2
static dl_rcmd_xml_ctx_type * s_search_result = NULL;
static dl_rcmd_xml_ctx_type ** s_navigate = NULL ;
static dl_rcmd_xml_ctx_type * s_suggest = NULL ;
static INT8U s_navigate_page_count = 0 ;
static server_xml_type * s_server_list = NULL;
static MEM_PIC_INFO * s_mem_pic = NULL ;
static MEM_PIC_INFO * s_last_pic = NULL ;

typedef struct {
	int taskid;
	int datasrc_id;
	int parent_id;
	int item_id;
	char cid[256];
	char src_url[256];
} DLSeedInfo_t;


DLSeedInfo_t CurDLSeedInfo;

typedef struct {
	int taskid;
	int datasrc_id;
	int parent_id;
	int item_id;
	char torrent[256];
	char seed_path[256];	
} DLTorrentInfo_t;

DLTorrentInfo_t CurDLTorrentInfo;
/******************* end thunder download ******************/


/********************** kankan ****************************/
#define KANKAN_VERSION 2  /* 1: old version;  2: new version */
static dl_rcmd_xml_ctx_type * s_kankan_search_result = NULL ;
static INT8U s_kankan_page_count = 0 ;
static dl_rcmd_xml_ctx_type ** s_kankan = NULL ;
static KANKAN_PROGRAME * s_kankan_programe = NULL ;
static char bIPTV_kan=0;
static char bCopyright_kan=0;
/**********************  end kankan data struct **************/

/*******************************************/
#define BTTV_DEBUG 0
#if (BTTV_DEBUG == 0)
#define printf(x...)
#endif

#ifdef __ROM_
#define printf(x...)
#else
#define TEST_DEAD_LOCK
#ifdef TEST_DEAD_LOCK
static char am_run_line[512];
static int log_count = 0 ; 
static int lock_flag = 0;
#endif
#endif

static int s_http_sem_inited = 0;
QMutex * s_am_http_sem=NULL;

void SetKanKanCond(char bIPTV,char bCopyright)
{
   bIPTV_kan=bIPTV;
   bCopyright_kan=bCopyright;
}

static void sem_init()
{
	while(!s_http_sem_inited)
	{
		s_http_sem_inited=1; 
                s_am_http_sem = new QMutex();//AVSemCreate(1);
		break;
	}
#ifdef TEST_DEAD_LOCK
	lock_flag = 0;
#endif
}

#ifdef TEST_DEAD_LOCK
static void sem_lock_(int line, const char* file)
#else
static void sem_lock_()
#endif
{
#ifdef TEST_DEAD_LOCK
	log_count++; 
	sprintf(am_run_line+(log_count%5)*100, "@%d line:%d, %s", log_count, line, file); 
#endif
        if(s_am_http_sem != NULL)
        {
            s_am_http_sem->lock();
        }
        /*
	do{
		INT8U err; 
		AVSemPend(s_am_http_sem, 0, &err);
	}while(0);
        */
#ifdef TEST_DEAD_LOCK
	sprintf(am_run_line+(log_count%5)*100, "L@%d line:%d, %s", log_count, line, file); 
	lock_flag ++;
#endif
}

#ifdef TEST_DEAD_LOCK
static void sem_unlock_(int line, const char* file)
#else
static void sem_unlock_()
#endif
{
#ifdef TEST_DEAD_LOCK
	log_count++; 
	sprintf(am_run_line+(log_count%5)*100, "U@%d line:%d, %s", log_count, line, file); 
#endif
        s_am_http_sem->unlock();
        //AVSemPost(s_am_http_sem);
#ifdef TEST_DEAD_LOCK
	lock_flag --;
#endif
}

static void sem_close()
{
	while(s_http_sem_inited)
	{
                //INT8U err;
                //AVSemDel(s_am_http_sem, OS_DEL_ALWAYS, &err);
                delete s_am_http_sem;
		s_http_sem_inited=0;
		break;
	}
#ifdef TEST_DEAD_LOCK
	lock_flag = 0;
#endif
}

#ifdef TEST_DEAD_LOCK
#define http_sem_lock() 	sem_lock_(__LINE__, __FILE__)
#define http_sem_unlock()sem_unlock_(__LINE__, __FILE__)
#else
#define http_sem_lock	sem_lock_
#define http_sem_unlock	sem_unlock_
#endif

static int bttvapp_send_msg_net_datasrc_ready(control_t* cntl, handle_t handle, INT32U ret_value, char* err_msg, INT32U datasrc_id, INT32U parent_id, INT32U rawdata, INT32U count, INT32U stage) ;
static int bttvapp_send_msg_net_pic_download_ready(control_t* cntl, handle_t handle, INT32U ret_value, char* err_msg, INT32U parent_id, INT32U item_id, INT32U image_data, INT32U data_size, char* image_type, INT32U stage);

static void add_pic_http_task() ;

/****************************************************/

typedef enum
{
	DOWNLOAD_URL_NULL = -6 ,
	DOWNLOAD_DISK_UNMOUNT = -5 ,
	NO_ENOUGH_SPACE = -4 ,
	NO_SUPPORT_FORMAT = -3 ,
	VOD_OVER = -2,
	ERR_OK = 0,
	MALLOC_MEM_ERROR = 7000 ,
	NO_GETTING_TASK = 7001,
	NO_LAST_TASK = 7002 ,
	ROOT_URL_NULL = 7003 ,
	CREATE_TASK_STARTED = 7004 ,
	CREATE_TASK_ERR = 7005 ,
	NO_SEARCH_WORD = 7006 ,
	NO_SUPPORT_THE_FORMAT_PROGRAMME = 7007,
}SKYNET_ERROR_TYPE ;

typedef enum
{
	JPG,
	GIF,
	BMP,
}PIC_FORMAT ;

static char * pic_format[] = { "jpg" , "gif" , "bmp" } ;

typedef enum
{
	ROOT_CHANNEL_GET,
	MOVIE_GET,
	SEARCH_GET,
	PIC_GET,
	TORENT_GET,	
	KANKAN_ROOT_GET,
	KANKAN_MOVIE_CH_GET,
	KANKAN_VOD_GET,
	KANKAN_SEARCH_GET,
	SEED_SRC_URL_GET,
	HD_GET,
	TORRENT_GET,
	PINYIN_SUGGEST_GET,
	SERVER_GET,
}HTTP_GET_TYPE;

typedef enum
{
	HTTP_NONE,
	HTTP_GETTING,
	HTTP_FINISHED,
}TASK_STATE ;

typedef struct _HTTP_TASK
{
	int task_id ;
	int get_type ;
	int datasrc_id ;
	int parent_id ;
	int item_id ;
	int task_state ;
	int i_error ;
	int i_result_len ;
	int i_get_times ;
	char * p_result ;
	char * p_url;
	control_t* cntl;
	struct _HTTP_TASK * p_next;
}HTTP_TASK;

static HTTP_TASK * s_http_task = NULL ;
static HTTP_TASK * s_getting_task = NULL ;

static int add_http_task( control_t* cntl , int task_id , HTTP_GET_TYPE http_get_type , int data_src , int parent_id , int item_id , char * url , INT8U b_lock )
{
	HTTP_TASK * p_task = (HTTP_TASK *)AVMem_malloc( sizeof( HTTP_TASK ) ) ;
	if( !p_task )
	{
		return MALLOC_MEM_ERROR ;
	}
	memset( p_task ,0x00 , sizeof( HTTP_TASK ) );
	p_task->cntl = cntl ;
	p_task->task_id = task_id ;
	p_task->get_type = http_get_type ;
	p_task->datasrc_id = data_src ;
	p_task->parent_id = parent_id ;
	p_task->item_id = item_id ;
        if(url == 0)
            p_task->p_url = NULL;
        else
            p_task->p_url = get_value( url , strlen( url ) );


	if( b_lock )		
		http_sem_lock() ;
	HTTP_TASK * cur_task = s_http_task ; 
	while( cur_task && cur_task->p_next )
	{
		cur_task = cur_task->p_next ;
	}
	if( !cur_task )
		s_http_task = p_task ;
	else
		cur_task->p_next = p_task ;
	if( b_lock )
		http_sem_unlock() ;

	return 0;
}

static HTTP_TASK * get_http_task_head( int b_lock )
{
	HTTP_TASK * p_task = NULL ;
	if( b_lock )
		http_sem_lock() ;
	if( !s_http_task )
	{
		if( b_lock )
			http_sem_unlock() ;
		return NULL ;
	}
	p_task = s_http_task ;
	s_http_task = s_http_task->p_next ;
	p_task->p_next = NULL ;
	//s_getting_task = p_task ;
	if( b_lock )
		http_sem_unlock() ;

	return p_task ;
}

static void release_http_task( HTTP_TASK ** p_task )
{
	if( ( *p_task ))
	{
		_AVMem_free( (*p_task)->p_url ) ;
		_AVMem_free( (*p_task)->p_result ) ;
		_AVMem_free( (*p_task) );
		(*p_task) = NULL ;
	}	
}

static void release_all_http_task( INT8U b_lock )
{
	if( b_lock )
		http_sem_lock() ;
	HTTP_TASK * p_task = s_http_task ;
	HTTP_TASK * p_task_temp = NULL ;
	while( p_task )
	{
		p_task_temp = p_task->p_next ;
		release_http_task( &p_task ) ;
		p_task = p_task_temp ;
	}
	
	s_http_task = NULL ;
	if( b_lock )
		http_sem_unlock() ;
}

 char* TransferManager_GetResult(bttvapp_t* bttvapp , INT32U taskid ) ;
 int TransferManager_GetSeedInfo(bttvapp_t* bttvapp , char* seed_path ,char* src );
 int TransferManager_AddTask(bttvapp_t* bttvapp , char* url ,char* saved_path ,char* buff ,INT32U buff_len ,char* post_buff ,INT32U isCallback ,INT32U type ,INT32U mode );
 int TransferManager_Init_Ex(bttvapp_t* bttvapp , char* lic );

static int modify_http_task( int task_id , int i_error , int i_result_len , control_t* cntl )
{
	http_sem_lock() ;
	if( !s_getting_task )
	{
		http_sem_unlock() ;
		return NO_GETTING_TASK ;
	}

	if( task_id != s_getting_task->task_id )
	{
		http_sem_unlock() ;
		return NO_LAST_TASK ;
	}
	
    bttvapp_t* bttvapp = NULL;//(bttvapp_t*)(cntl->private_data);
	char * buff = NULL ;
	if( 0 == i_error && i_result_len > 0 )
		buff = TransferManager_GetResult( bttvapp ,task_id ) ;
#ifndef __ROM_
	else
	{
		printf( "\n%d task get result: %d  len: %d\n" , task_id , i_error , i_result_len );
	}
#endif
	s_getting_task->i_error = i_error ;
	s_getting_task->i_result_len = i_result_len ;
	s_getting_task->p_result = get_value( buff , i_result_len ) ;
	s_getting_task->task_state = HTTP_FINISHED ;
	//s_getting_task->cntl = cntl ;
	http_sem_unlock() ;
}

static MEM_PIC_INFO * add_mem_pic( int i_channel , int i_item , int i_len , char * pic , int pic_format)
{
        MEM_PIC_INFO * p_mem_pic = (MEM_PIC_INFO*)AVMem_malloc( sizeof( MEM_PIC_INFO ) ) ;
	if( !p_mem_pic )
		return NULL ;
	p_mem_pic->i_channel = i_channel ;
	p_mem_pic->i_item = i_item ;
	p_mem_pic->pic_buff_len = i_len ;
	p_mem_pic->pic_format = pic_format ;
	p_mem_pic->pic_buff = get_value( pic , i_len ) ;
	p_mem_pic->p_next_pic = NULL ;
	
	/*if( !s_mem_pic )
		s_mem_pic = p_mem_pic ;
	else
		s_last_pic->p_next_pic = p_mem_pic ;*/
	s_last_pic = p_mem_pic ;
	return p_mem_pic ;
}

static void check_channel_id( dl_rcmd_xml_ctx_type * p_channel )
{
	if( !p_channel )
		return ;
	for( int i = 0 ; i < p_channel->nodes_count ; i++ )
	{
		if( i != p_channel->nodes[i].id )
		{
			p_channel->nodes[i].id = i ;
		}
	}
}

#define RA_DEPACK_TASK_STK_SIZE		(24*1024)
//static OS_STK * s_parse_xml_task = NULL ;
static void * s_parse_xml_task = NULL ;
typedef enum
{
	DOWNLOAD_STOPED = 1 ,
	DOWNLOAD_RUN=2 ,
}PASE_TASK_STATE;
static INT8U  s_download_run = DOWNLOAD_STOPED ;

static char s_error_msg[128] = {0} ;

static void parse_root_channel( HTTP_TASK * p_getting_task , INT8U * page_count )
{
	dl_rcmd_xml_ctx_type ** p_navigate = NULL ;
	if( 0 == s_getting_task->i_error )
	{
		p_navigate = ( dl_rcmd_xml_ctx_type** )AVMem_malloc( sizeof( dl_rcmd_xml_ctx_type* ));
		if( !p_navigate )
		{
			s_getting_task->i_error = MALLOC_MEM_ERROR ;
			return ;
		}
		p_navigate[0] = NULL ;
		if( ROOT_CHANNEL_GET == s_getting_task->get_type )
		{
			s_getting_task->i_error = DLRcmd_ParseXmlCtx( ( char * )s_getting_task->p_result , &p_navigate[0]) ;
		}
		else
		{
			s_getting_task->i_error = KanKan_ParseCH( ( char * )s_getting_task->p_result , &p_navigate[0]) ;
		}
		
		if( ERR_OK == s_getting_task->i_error )
		{
			dl_rcmd_xml_ctx_type ** new_p_navigate;
			new_p_navigate = ( dl_rcmd_xml_ctx_type** )AVMem_realloc(p_navigate, sizeof( dl_rcmd_xml_ctx_type* )*(p_navigate[0]->nodes_count+1));
			if( !new_p_navigate )
			{
				if( ROOT_CHANNEL_GET == s_getting_task->get_type )
					DLRcmd_ReleaseXmlCtx(p_navigate[0]) ;
				else
					KanKan_ReleaseXmlCtx(p_navigate[0]) ;
				s_getting_task->i_error = MALLOC_MEM_ERROR ;
				return ;
			}
			p_navigate = new_p_navigate;
			memset(&p_navigate[1], 0, sizeof( dl_rcmd_xml_ctx_type* )*p_navigate[0]->nodes_count);

			check_channel_id( p_navigate[0] ) ;
			(*page_count) = 1 ;
			s_error_msg[0] = '\0' ;
		}
		else if( MAX_GET_TIMES > s_getting_task->i_get_times )
		{
			_AVMem_free( p_navigate ) ;
			return ;
		}
		else
		{
#ifndef __ROM_
			printf( "\nparse channel page xml error , error no: %d! \n" , s_getting_task->i_error ) ;
#endif
			snprintf( s_error_msg , sizeof( s_error_msg ) , "get channel info error ! %d\n" , s_getting_task->i_error ) ;
			s_error_msg[127] = 0;
			_AVMem_free( p_navigate ) ;
		}
	}
	if( 0 == s_getting_task->i_error )
	{
		if( 0 == s_getting_task->i_error && ROOT_CHANNEL_GET == s_getting_task->get_type )
			s_navigate = p_navigate ;
		else if( 0 == s_getting_task->i_error && KANKAN_ROOT_GET == s_getting_task->get_type )		
			s_kankan = p_navigate ;
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error ,s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id , (INT32U)p_navigate[0] , p_navigate[0]->nodes_count , 0 ) ;
	}
	else if( MAX_GET_TIMES <= s_getting_task->i_get_times )
	{
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error ,s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id , (INT32U)0 , 0 , 0 ) ;
	}
}

static int parse_movie_root_channel(INT8U * page_count )
{
	int ret = 0;
	dl_rcmd_xml_ctx_type ** p_navigate = NULL ;
	
	p_navigate = ( dl_rcmd_xml_ctx_type** )AVMem_malloc( sizeof( dl_rcmd_xml_ctx_type* ));
	if( !p_navigate )
	{
		return -1;
	}
	p_navigate[0] = NULL ;
	ret = Download_ParseCH( ( char * )movie_channel_xml, &p_navigate[0]) ;

		
	if( ERR_OK == ret )
	{
		check_channel_id( p_navigate[0] ) ;
		(*page_count) = 1 ;
		s_error_msg[0] = '\0' ;
	}
	else
	{
#ifndef __ROM_
		printf( "\nparse channel page xml error , error no: %d! \n" , ret) ;
#endif
		snprintf( s_error_msg , sizeof( s_error_msg ) , "get channel info error ! %d\n" , ret) ;
		s_error_msg[127] = 0;
		_AVMem_free( p_navigate ) ;
	}
	
	if( 0 == ret )
	{
		s_navigate= p_navigate ;
	}
	else
	{
		s_navigate= NULL ;
	}
	return 0;
}


static int parse_kankan_root_channel(INT8U * page_count )
{
	int ret = 0;
	dl_rcmd_xml_ctx_type ** p_navigate = NULL ;
	
	p_navigate = ( dl_rcmd_xml_ctx_type** )AVMem_malloc( sizeof( dl_rcmd_xml_ctx_type* ));
	if( !p_navigate )
	{
		return -1;
	}
	p_navigate[0] = NULL ;
	ret = KanKan_ParseCH( ( char * )kankan_channel_xml, &p_navigate[0]) ;

		
	if( ERR_OK == ret )
	{
		dl_rcmd_xml_ctx_type ** new_p_navigate;
		new_p_navigate = ( dl_rcmd_xml_ctx_type** )AVMem_realloc(p_navigate, sizeof( dl_rcmd_xml_ctx_type* )*(p_navigate[0]->nodes_count+1));
		if( !new_p_navigate )
		{
			KanKan_ReleaseXmlCtx(p_navigate[0]) ;
			return -1;
		}
		p_navigate = new_p_navigate;
		memset(&p_navigate[1], 0, sizeof( dl_rcmd_xml_ctx_type* )*p_navigate[0]->nodes_count);
	
		check_channel_id( p_navigate[0] ) ;
		(*page_count) = 1 ;
		s_error_msg[0] = '\0' ;
	}
	else
	{
#ifndef __ROM_
		printf( "\nparse channel page xml error , error no: %d! \n" , ret) ;
#endif
		snprintf( s_error_msg , sizeof( s_error_msg ) , "get channel info error ! %d\n" , ret) ;
		s_error_msg[127] = 0;
		_AVMem_free( p_navigate ) ;
	}
	
	if( 0 == ret )
	{
		s_kankan = p_navigate ;
	}
	else
	{
		s_kankan = NULL ;
	}
	return 0;
}

static void parse_movie_channel( HTTP_TASK * p_getting_task )
{				
	if( 0 == s_getting_task->i_error )
	{
		if( 1 == s_navigate_page_count )
		{
			s_navigate = ( dl_rcmd_xml_ctx_type** )AVMem_realloc( s_navigate , ( s_navigate[0]->nodes_count + 1 ) * sizeof( dl_rcmd_xml_ctx_type* ));
			if( s_navigate )
			{
				for( int i = 1 ; i <= s_navigate[0]->nodes_count ; i++ )
				{
					s_navigate[i] = NULL ;
				}
			}
		}
		if( !s_navigate )
		{
			s_getting_task->i_error = MALLOC_MEM_ERROR ;
			return ;
		}
		#if (MOVIE_VERSION == 2)
			s_getting_task->i_error = Download_ParseMovie( ( char * )s_getting_task->p_result , &s_navigate[ s_getting_task->item_id + 1 ]) ;
		#else
			s_getting_task->i_error = DLRcmd_ParseXmlCtx( ( char * )s_getting_task->p_result , &s_navigate[ s_getting_task->item_id + 1 ]) ;
		#endif
	}

	if( ERR_OK == s_getting_task->i_error )
	{
		s_navigate[ s_getting_task->item_id + 1 ]->parent_id = s_getting_task->item_id ;
		s_navigate_page_count++ ;
		s_error_msg[0] = '\0' ;
	}
	else if( MAX_GET_TIMES > s_getting_task->i_get_times )
	{
		return ;
	}
	else
	{
#ifndef __ROM_
		printf( "\nparse %s movie page xml error , error no: %d! \n" , s_navigate[0]->nodes[s_getting_task->item_id].name , s_getting_task->i_error ) ;
#endif
		snprintf( s_error_msg , sizeof( s_error_msg ) , "get channel movie info error ! %d\n" , s_getting_task->i_error ) ;
		s_error_msg[127] = 0;
	}
	
	if( 0 == s_getting_task->i_error )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)s_navigate[ s_getting_task->item_id + 1 ] , s_navigate[ s_getting_task->item_id + 1 ]->nodes_count , 0) ;
	else if( MAX_GET_TIMES <= s_getting_task->i_get_times )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 0) ;
}

static void parse_hd_result( HTTP_TASK * p_getting_task )
{
	if( NULL != s_search_result)
	{
		DLSrch_ReleaseXmlCtx(s_search_result);
		s_search_result = NULL;
	}
	
	if( 0 == s_getting_task->i_error )
	{
		s_getting_task->i_error = Download_ParseHD( ( char * )s_getting_task->p_result , &s_search_result) ;
	}
	if( 0 == s_getting_task->i_error )
	{
		s_error_msg[0] = '\0' ;
	}
	else if( MAX_GET_TIMES > s_getting_task->i_get_times )
	{
		return ;
	}
	else
	{
#ifndef __ROM_
		printf( "\nparse hd result xml error , error no: %d! \n" , s_getting_task->i_error ) ;
#endif				
		snprintf( s_error_msg , sizeof( s_error_msg ) , "parse hd movie error ! %d\n" , s_getting_task->i_error ) ;
		s_error_msg[127] = 0;
	}
	if( 0 == s_getting_task->i_error )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id , (INT32U)s_search_result , s_search_result->nodes_count , 0 ) ;
	else if( MAX_GET_TIMES <= s_getting_task->i_get_times )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 0) ;
}

static void parse_search_result( HTTP_TASK * p_getting_task )
{
	if( NULL != s_search_result)
	{
		DLSrch_ReleaseXmlCtx(s_search_result);
		s_search_result = NULL;
	}
	
	if( 0 == s_getting_task->i_error )
	{
		s_getting_task->i_error = DLSrch_ParseXmlCtx( ( char * )s_getting_task->p_result , &s_search_result) ;
	}
	if( 0 == s_getting_task->i_error )
	{
		s_error_msg[0] = '\0' ;
	}
	else if( MAX_GET_TIMES > s_getting_task->i_get_times )
	{
		return ;
	}
	else
	{
#ifndef __ROM_
		printf( "\nparse search result xml error , error no: %d! \n" , s_getting_task->i_error ) ;
#endif				
		snprintf( s_error_msg , sizeof( s_error_msg ) , "search movie error ! %d\n" , s_getting_task->i_error ) ;
		s_error_msg[127] = 0;
	}
	if( 0 == s_getting_task->i_error )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id , (INT32U)s_search_result , s_search_result->nodes_count , 0 ) ;
	else if( MAX_GET_TIMES <= s_getting_task->i_get_times )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 0) ;
}

static void parse_pic_get( HTTP_TASK * p_getting_task )
{
#ifndef __ROM_
//	if( 0 != s_getting_task->i_error )
		printf( "\n\nget pic %s error no: %d! \n" , s_current_pic_ctx[s_getting_task->parent_id + 1]->nodes[s_getting_task->item_id].name , s_getting_task->i_error ) ;
#endif					
	MEM_PIC_INFO * p_mem_pic = NULL ;
	if( 0 == s_getting_task->i_error )
	{
		p_mem_pic = add_mem_pic( s_getting_task->parent_id , s_getting_task->item_id , s_getting_task->i_result_len , s_getting_task->p_result , JPG ) ;
	}
	else if( MAX_GET_TIMES > s_getting_task->i_get_times )
	{
		return ;
	}
	if( p_mem_pic )
		bttvapp_send_msg_net_pic_download_ready( s_getting_task->cntl , BROADCAST_ALL , 0 , NULL , s_getting_task->parent_id , s_getting_task->item_id , (INT32U)p_mem_pic , p_mem_pic->pic_buff_len , pic_format[ p_mem_pic->pic_format ] , 0 ) ;
	else
		add_http_task( s_getting_task->cntl ,0 , PIC_GET , 2 , s_getting_task->parent_id , s_getting_task->item_id , NULL , 0 ) ;
}	


static void parse_kankan_channel( HTTP_TASK * p_getting_task )
{				
	if( 0 == s_getting_task->i_error )
	{
		if( 1 == s_kankan_page_count )
		{
			s_kankan = ( dl_rcmd_xml_ctx_type** )AVMem_realloc( s_kankan , ( s_kankan[0]->nodes_count + 1 ) * sizeof( dl_rcmd_xml_ctx_type* ));
			if( s_kankan )
			{
				for( int i = 1 ; i <= s_kankan[0]->nodes_count ; i++ )
				{
					s_kankan[i] = NULL ;
				}
			}
		}
		if( !s_kankan )
		{
			s_getting_task->i_error = MALLOC_MEM_ERROR ;
			return ;
		}
		if( KANKAN_SEARCH_GET == s_getting_task->get_type && s_kankan[ s_getting_task->item_id + 1 ])
		{
			KanKan_ReleaseXmlCtx( s_kankan[ s_getting_task->item_id + 1 ] ) ;
			s_kankan[ s_getting_task->item_id + 1 ] = NULL ;
		}
		s_getting_task->i_error = KanKan_ParseMovies( ( char * )s_getting_task->p_result , &s_kankan[ s_getting_task->item_id + 1 ] ,bIPTV_kan,bCopyright_kan, s_getting_task->get_type==KANKAN_SEARCH_GET);
	}

	if( ERR_OK == s_getting_task->i_error )
	{
		s_kankan[ s_getting_task->item_id + 1 ]->parent_id = s_getting_task->item_id ;
		s_kankan_page_count++ ;
		s_error_msg[0] = '\0' ;
	}
	else if( MAX_GET_TIMES > s_getting_task->i_get_times )
	{
		return ;
	}
	else
	{
#ifndef __ROM_
		printf( "\nparse %s movie page xml error , error no: %d! \n" , s_kankan[0]->nodes[s_getting_task->item_id].name , s_getting_task->i_error ) ;
#endif
		snprintf( s_error_msg , sizeof( s_error_msg ) , "get channel movie info error ! %d\n" , s_getting_task->i_error ) ;
		s_error_msg[127] = 0;
	}
	
	if( 0 == s_getting_task->i_error )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)s_kankan[ s_getting_task->item_id + 1 ] , s_kankan[ s_getting_task->item_id + 1 ]->nodes_count , 0) ;
	else if( MAX_GET_TIMES <= s_getting_task->i_get_times )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 0) ;
}

static void parse_seed_src_url( HTTP_TASK * p_getting_task )
{
	if( 0 == s_getting_task->i_error )
	{
#ifndef __ROM_
		printf( "\n\nget src_url: %s error no: %d! \n" , s_getting_task->p_result, s_getting_task->i_error ) ;
#endif
	}
	else if( MAX_GET_TIMES > s_getting_task->i_get_times )
	{
		s_error_msg[0] = 0;	
		//bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 1) ;
		return ;
	}
	
	if( 0 == s_getting_task->i_error ) {
		snprintf( s_error_msg , sizeof( s_error_msg ) , "%s" , s_getting_task->p_result ) ;
		s_error_msg[127] = 0;
		if((CurDLSeedInfo.datasrc_id == s_getting_task->datasrc_id) 
				&& (CurDLSeedInfo.parent_id == s_getting_task->parent_id)
				&& (CurDLSeedInfo.item_id == s_getting_task->item_id)) {
			
			snprintf( CurDLSeedInfo.src_url, sizeof( CurDLSeedInfo.src_url ) , "%s" , s_getting_task->p_result ) ;
		}
		
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 1) ;
	}
	else if( MAX_GET_TIMES <= s_getting_task->i_get_times ) {
		snprintf( s_error_msg , sizeof( s_error_msg ) , "TIMEOUT" ) ;
		s_error_msg[127] = 0;
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 1) ;
	}
}	

static void parse_torrent_get (HTTP_TASK * p_getting_task )
{
	int ret = -1;
	
#ifndef __ROM_
		printf( "\n\nget torrent file: %s error no: %d! \n" , s_getting_task->p_url, s_getting_task->i_error ) ;
#endif	
	CurDLTorrentInfo.taskid = 0;

	if( 0 == s_getting_task->i_error ) {
		snprintf( s_error_msg , sizeof( s_error_msg ) , "SUCCESS" ) ;
		s_error_msg[127] = 0;
		if((CurDLTorrentInfo.datasrc_id == s_getting_task->datasrc_id) 
				&& (CurDLTorrentInfo.parent_id == s_getting_task->parent_id)
				&& (CurDLTorrentInfo.item_id == s_getting_task->item_id)) 
		{
			bttvapp_t* bttvapp = (bttvapp_t*)((s_getting_task->cntl)->private_data);
			ret = TransferManager_GetSeedInfo(bttvapp, CurDLTorrentInfo.seed_path, CurDLTorrentInfo.torrent);
			if(ret != 0)
			{
				snprintf( s_error_msg , sizeof( s_error_msg ) , "GET TORRENT INFO ERROR" ) ;
				s_error_msg[127] = 0;
			}
		}
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , ret , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 1) ;
	}
	else if( MAX_GET_TIMES <= s_getting_task->i_get_times ) {			
		snprintf( s_error_msg , sizeof( s_error_msg ) , "TIMEOUT" ) ;
		s_error_msg[127] = 0;
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , ret , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 1) ;	
	}
	else {
		snprintf( s_error_msg , sizeof( s_error_msg ) , "FAILED" ) ;
		s_error_msg[127] = 0;
	}
}

static void parse_pinyin_suggest_get(HTTP_TASK * p_getting_task) 
{
#ifndef __ROM_
	printf( "\n\nget pinyin suggest word,error no: %d! \n" , s_getting_task->i_error ) ;
#endif

	if( NULL != s_suggest)
	{
		DLSrch_ReleaseXmlCtx(s_suggest);
		s_suggest = NULL;
	}
	
	if( 0 == s_getting_task->i_error )
	{
		s_getting_task->i_error = Download_ParseSuggest( ( char * )s_getting_task->p_result , &s_suggest) ;
	}
	else
	{				
		snprintf( s_error_msg , sizeof( s_error_msg ) , "get pinyin suggest word,error no: %d! \n" , s_getting_task->i_error ) ;
	}
	
	if( 0 == s_getting_task->i_error )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id , (INT32U)s_suggest , s_suggest->nodes_count , 0) ;
	else if( MAX_GET_TIMES <= s_getting_task->i_get_times )
		bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 0) ;
}

static void parse_server_get(HTTP_TASK * p_getting_task) ;

static void parse_vod_get( HTTP_TASK * p_getting_task );
	
//static void parse_xml_pro(void *exit_sem)
void XMLThread::run()
{
	while( 1 )
	{
		http_sem_lock() ;
		if( s_getting_task && HTTP_FINISHED == s_getting_task->task_state )
		{
			switch( s_getting_task->get_type )
			{
				case ROOT_CHANNEL_GET :
					parse_root_channel( s_getting_task , &s_navigate_page_count ) ;
					break ;
				case MOVIE_GET :
					parse_movie_channel( s_getting_task ) ;
					s_current_pic_ctx = s_navigate ;
					break ;
				case SEARCH_GET :
					parse_search_result( s_getting_task ) ;
					break ;
				case PIC_GET :
					parse_pic_get( s_getting_task ) ;
					break ;
				case KANKAN_ROOT_GET :
					parse_root_channel( s_getting_task , &s_kankan_page_count ) ;
					break;
				case KANKAN_MOVIE_CH_GET :
				case KANKAN_SEARCH_GET :
					parse_kankan_channel( s_getting_task ) ;
					s_current_pic_ctx = s_kankan ;
 					break;
				case KANKAN_VOD_GET :
					parse_vod_get( s_getting_task ) ;
					break ;
				case SEED_SRC_URL_GET:
					parse_seed_src_url( s_getting_task );
					break;
				case HD_GET:
					parse_hd_result( s_getting_task );
					break;
				case TORRENT_GET:
					parse_torrent_get( s_getting_task );
					break;
				case PINYIN_SUGGEST_GET:
					parse_pinyin_suggest_get( s_getting_task );
					break;
				case SERVER_GET:
					parse_server_get( s_getting_task );
					break;
				default:
					break;
			}

			if( 0 != s_getting_task->i_error && MAX_GET_TIMES > s_getting_task->i_get_times)
			{
				bttvapp_t* bttvapp = NULL;//(bttvapp_t*)((s_getting_task->cntl)->private_data);
				int task_id = TransferManager_AddTask( bttvapp , s_getting_task->p_url , 
					(char*)NULL , (char*)NULL , 0 , (char*)NULL , 1 , XFER_TASK_HTTP , XFER_BUFF | XFER_NOCACHE );
				s_getting_task->i_get_times++ ;
				if( task_id > 0 )
				{
					_AVMem_free( s_getting_task->p_result );
					s_getting_task->task_state = HTTP_NONE ;
					s_getting_task->task_id = task_id ;
					s_getting_task->i_result_len = 0 ;
					s_getting_task->i_error = 0 ;
				}
				else
				{
				}	
			}
			else
			{
				release_http_task( &s_getting_task ) ;
				add_pic_http_task() ;
			}
			http_sem_unlock() ;
		}
		else
		{
			if(!s_getting_task)
			{
				add_pic_http_task();
			}
			http_sem_unlock() ;
            //AVTimeDly(200);
            sleep(200);
		}
	}

        //_AVMem_free( s_parse_xml_task ) ;
	s_download_run = DOWNLOAD_STOPED ;
	//moap_sem_close() ;
        //AVTaskDel(OS_ID_SELF);
}


int create_parse_xml_task( )
{
    int ret = -1 ;
    if( DOWNLOAD_STOPED == s_download_run )
    {
       //  char* licence = "09092700020000000000001eheff19cu95p6gnuw42";
          s_download_run = DOWNLOAD_RUN ;
           sem_init();
           ret = TransferManager_Init_Ex( NULL , Licence ) ;
          XMLThread::Instance().start();
    }
/*	INT16U taskid;
	if( DOWNLOAD_STOPED != s_download_run )
		return CREATE_TASK_STARTED ;
	s_parse_xml_task = (OS_STK *)AVMem_malloc( sizeof( OS_STK ) * RA_DEPACK_TASK_STK_SIZE ) ;
	if( !s_parse_xml_task )
		return  CREATE_TASK_ERR;
    if( OS_NO_ERR == AVTaskCreate(parse_xml_pro, 0, &s_parse_xml_task[RA_DEPACK_TASK_STK_SIZE - 1], 25, &taskid) )
	{
		s_download_run = DOWNLOAD_RUN ;
		sem_init();
		return ERR_OK ;
	}
	else
	{
		_AVMem_free( s_parse_xml_task ) ;
		return CREATE_TASK_ERR  ;
	}
        */
}

static int get_channel_type( char * p_channel_name )
{
	int channel_type = 5 ;

	if( !strcmp( p_channel_name , "热门电影" ))
		channel_type = 5 ;
   	else if( !strcmp( p_channel_name , "最新电影" ))
		channel_type = 54 ; 
   	else if( !strcmp( p_channel_name , "大陆电影" ))
		channel_type = 50 ;
   	else if( !strcmp( p_channel_name , "港台电影" ))
		channel_type = 51 ;
   	else if( !strcmp( p_channel_name , "日韩电影" ))
		channel_type = 52 ;
   	else if( !strcmp( p_channel_name , "欧美电影" ))
		channel_type = 53 ;
	else if( !strcmp( p_channel_name , "热门剧集" ))
		channel_type = 7 ;
   	else if( !strcmp( p_channel_name , "大陆剧集" ))
		channel_type = 14 ;
   	else if( !strcmp( p_channel_name , "港台剧集" ))
		channel_type = 17 ;
   	else if( !strcmp( p_channel_name , "日韩剧集" ))
		channel_type = 16 ;
   	else if( !strcmp( p_channel_name , "欧美剧集" ))
		channel_type = 15 ;
 	else if( !strcmp( p_channel_name , "热门卡通" ) )
		channel_type = 2 ; 
	else if( !strcmp( p_channel_name , "其它电影" ))
	{
		//AVMem_free( p_channel_name ) ;
		//p_channel_name = moap_getvalue( "综艺节目" ) ;
		channel_type = 30 ;
   	}
	else if( !strcmp( p_channel_name , "其它电视剧")) //"综艺节目" ))
	{
		//AVMem_free( p_channel_name ) ;
		//p_channel_name = moap_getvalue( "电视节目" ) ;
		channel_type = 31 ;
	}
	else if( !strcmp( p_channel_name , "其它动漫" ))
		channel_type = 99 ; 

	return channel_type ;
}

static float get_disk_freespace(char *disk_or_path)
{
    /*
	char buf[64] = {0}; 
	char *disk = "/mnt/";
	
	float freesize = 0 ;
	off_t disksize = 0 ;
	int secsize = 0 ;
	
	int i_count = 0 ;
	int len;
	len = strlen(disk);
	strncpy(buf, disk, len);	
	
	if(disk_or_path && !strncmp(buf, disk_or_path, len))
	{
		while( len < sizeof(buf)-1 && disk_or_path[len])
		{
			buf[len] = disk_or_path[len];
			if( '/' == disk_or_path[len] )
			i_count ++ ;
			if( i_count == 2 )
			{
				break ;
			}
			len++;
		}
	}
	else
	{
		strcat(buf, "UDISK/usba01/");
	}
	
	freesize = getFreeandDiskSize(buf, &disksize, &secsize);
	
	freesize *= (float)secsize;

	return freesize;
*/
    return 0x88888888;
}

static char * check_file_name( char * c_file_name )
{
	if( !c_file_name )
		return NULL ;
	const char c_invalid[] = "<>\\/|:*?\"";
		
	for( int i = 0 ; i < strlen( c_invalid ) ; i ++ )
	{
		for( int j = 0 ; j < strlen( c_file_name ) ; j ++ )
		{
			if( c_invalid[i] == c_file_name[j] )
			{
				c_file_name[j] = '_' ;
			}
		}
	}
	return c_file_name ;
}



/* access Handle/Array prop by these Macros*/
/*
#define TransferManager_task_table                   af_get_prop_val(bttvapp->TransferManager->cntl,bttvapp->TransferManager->task_table_pos)
#define TransferManager_task_table_Set(val)          af_set_prop_val(bttvapp->TransferManager->cntl,bttvapp->TransferManager->task_table_pos,val)
#define TransferManager_cb_handler                   af_get_prop_val(bttvapp->TransferManager->cntl,bttvapp->TransferManager->cb_handler_pos)
#define TransferManager_cb_handler_Set(val)          af_set_prop_val(bttvapp->TransferManager->cntl,bttvapp->TransferManager->cb_handler_pos,val)
#define TransferManager_last_error                   af_get_prop_val(bttvapp->TransferManager->cntl,bttvapp->TransferManager->last_error_pos)
#define TransferManager_last_error_Set(val)          af_set_prop_val(bttvapp->TransferManager->cntl,bttvapp->TransferManager->last_error_pos,val)

#define this_DownloadPath_str                af_get_prop_str(bttvapp->cntl,PROP_DOWNLOADPATH_POS)
#define this_DownloadPath_str_Set(str)       af_set_prop_str(bttvapp->cntl,PROP_DOWNLOADPATH_POS,str)
#define this_ServerURL_str                af_get_prop_str(bttvapp->cntl,PROP_SERVERURL_POS)
#define this_ServerURL_str_Set(str)       af_set_prop_str(bttvapp->cntl,PROP_SERVERURL_POS,str)
#define this_RootURL_str                af_get_prop_str(bttvapp->cntl,PROP_ROOTURL_POS)
#define this_RootURL_str_Set(str)       af_set_prop_str(bttvapp->cntl,PROP_ROOTURL_POS,str)
#define this_SearchURL_str                af_get_prop_str(bttvapp->cntl,PROP_SEARCHURL_POS)
#define this_SearchURL_str_Set(str)       af_set_prop_str(bttvapp->cntl,PROP_SEARCHURL_POS,str)
#define this_SearchBtURL_str                af_get_prop_str(bttvapp->cntl,PROP_SEARCHBTURL_POS)
#define this_SearchBtURL_str_Set(str)       af_set_prop_str(bttvapp->cntl,PROP_SEARCHBTURL_POS,str)
#define this_SearchType_str                af_get_prop_str(bttvapp->cntl,PROP_SEARCHTYPE_POS)
#define this_SearchType_str_Set(str)       af_set_prop_str(bttvapp->cntl,PROP_SEARCHTYPE_POS,str)
#define this_kankanNaviUrl_str                af_get_prop_str(bttvapp->cntl,PROP_KANKANNAVIURL_POS)
#define this_kankanNaviUrl_str_Set(str)       af_set_prop_str(bttvapp->cntl,PROP_KANKANNAVIURL_POS,str)
#define this_kankanSearUrl_str                af_get_prop_str(bttvapp->cntl,PROP_KANKANSEARURL_POS)
#define this_kankanSearUrl_str_Set(str)       af_set_prop_str(bttvapp->cntl,PROP_KANKANSEARURL_POS,str)
#define MountInfo_ColNickNameList_str(subindex)                af_get_array_str(bttvapp->MountInfo->cntl,bttvapp->MountInfo->ColNickNameList_pos, subindex)
#define MountInfo_ColNickNameList_str_Set(subindex,str)        af_set_array_str(bttvapp->MountInfo->cntl,bttvapp->MountInfo->ColNickNameList_pos, subindex,str)
#define MountInfo_ColNickNameList_str_Num                af_get_property_array_by_pos(bttvapp->MountInfo->cntl,bttvapp->MountInfo->ColNickNameList_pos, NULL)
#define MountInfo_ColNickNameList_str_Array_Resize(size)        af_resize_property_array_by_pos(bttvapp->MountInfo->cntl,bttvapp->MountInfo->ColNickNameList_pos, size)

#define this_ErrorNo                   af_get_prop_val(bttvapp->cntl,PROP_ERRORNO_POS)
#define this_ErrorNo_Set(val)          af_set_prop_val(bttvapp->cntl,PROP_ERRORNO_POS,val)
*/
/*Macros defines end*/
//#define G_DUPSTR(s1, s2)  \
//    do {if (s1) {delete s1; s1 = 0;} if (s2) {s1 = strdup(s2);} }while (0)

char *g_DownloadPath ; //malloc by InitTaskInfo()
char* g_ServerURL = "/home/download";
char* g_RootURL = "http://dy.n0808.com/rank_list/list.xml";
char* g_SearchURL = "http://dy.n0808.com/btinfo?num=30&amp;page=1&amp;format=rm|rmvb|avi|dat|mpg|vob|mpg|ts|m2ts&amp;restype=http&amp;keyword=";
char* g_SearchBtURL = "http://dy.n0808.com/btinfo?num=30&amp;page=1&amp;format=rm|rmvb|avi|dat|mpg|vob|mpg|ts&amp;restype=bt&amp;default=1&amp;keyword=";
char* g_SearchType = "";
char* g_kankanNaviUrl = "http://preview.kankan.xunlei.com/movie/list.xml";
char* g_kankanSearUrl = "http://dy.n0808.com/top_movie?num=12&amp;amp;vdkey=";

void  G_DUPSTR(char *s1, char *s2)
{
    if (s1)
    {
        delete s1; s1 = 0;
    }
    if (s2)
    {
        s1 = strdup(s2);
    }
      qDebug("%s,%s",s2,s1);
    qDebug("%s",g_DownloadPath);
}


#define this_DownloadPath_str                g_DownloadPath
#define this_DownloadPath_str_Set(str)       G_DUPSTR(g_DownloadPath, str)
#define this_ServerURL_str                g_ServerURL
#define this_ServerURL_str_Set(str) G_DUPSTR(g_ServerURL, str)
#define this_RootURL_str            g_RootURL
#define this_RootURL_str_Set(str)   G_DUPSTR(g_RootURL, str)
#define this_SearchURL_str          g_SearchURL
#define this_SearchURL_str_Set(str) G_DUPSTR(g_SearchURL, str)
#define this_SearchBtURL_str        g_SearchBtURL
#define this_SearchBtURL_str_Set(str)   G_DUPSTR(g_SearchBtURL, str)
#define this_SearchType_str         g_SearchType
#define this_SearchType_str_Set(str)    G_DUPSTR(g_SearchType, str)
#define this_kankanNaviUrl_str      g_kankanNaviUrl
#define this_kankanNaviUrl_str_Set(str) G_DUPSTR(g_kankanNaviUrl, str)
#define this_kankanSearUrl_str      g_kankanSearUrl
#define this_kankanSearUrl_str_Set(str) G_DUPSTR(g_kankanSearUrl, str)

static void bttvapp_release(control_t* cntl)
{
    bttvapp_t* bttvapp = NULL ;// (bttvapp_t *)(cntl->private_data);
    if(bttvapp)
    {
        if(bttvapp->TransferManager)
        {
            AVMem_free(bttvapp->TransferManager);
            bttvapp->TransferManager = NULL;
        }
        if(bttvapp->MountInfo)
        {
            AVMem_free(bttvapp->MountInfo);
            bttvapp->MountInfo = NULL;
        }
        AVMem_free(bttvapp);
    }

}

static bttvapp_t* bttvapp_init(control_t* cntl)
{
    int i=0;
    cond_item_t* temp_ptr=NULL;
    bttvapp_t* bttvapp= (bttvapp_t*)AVMem_calloc(sizeof(bttvapp_t),1);
    if(bttvapp)
    {
/*        if(af_get_prop_val(cntl, PROP_TRANSFERMANAGER_POS))
        {
            bttvapp->TransferManager = AVMem_calloc(sizeof(TransferInter_t),1);
            if(bttvapp->TransferManager)
            {
                control_t* tmp_cntl;
                tmp_cntl = af_find_ctrl_by_handle(af_get_prop_val(cntl,PROP_TRANSFERMANAGER_POS));
                if(tmp_cntl)
                {
                    bttvapp->TransferManager->cntl = tmp_cntl;
                    bttvapp->TransferManager->task_table_pos = af_get_property_pos_by_name(tmp_cntl, TransferInter_PROP_task_table);
                    bttvapp->TransferManager->cb_handler_pos = af_get_property_pos_by_name(tmp_cntl, TransferInter_PROP_cb_handler);
                    bttvapp->TransferManager->last_error_pos = af_get_property_pos_by_name(tmp_cntl, TransferInter_PROP_last_error);
                    bttvapp->TransferManager->SetOption_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_SetOption);
                    bttvapp->TransferManager->GetOption_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_GetOption);
                    bttvapp->TransferManager->Init_Ex_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_Init_Ex);
                    bttvapp->TransferManager->UpdateTaskInfo_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_UpdateTaskInfo);
                    bttvapp->TransferManager->UnloadAllTasks_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_UnloadAllTasks);
                    bttvapp->TransferManager->LoadAllTasks_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_LoadAllTasks);
                    bttvapp->TransferManager->AddTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_AddTask);
                    bttvapp->TransferManager->GetResult_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_GetResult);
                    bttvapp->TransferManager->AddThunderTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_AddThunderTask);
                    bttvapp->TransferManager->StartTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_StartTask);
                    bttvapp->TransferManager->StartPrioTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_StartPrioTask);
                    bttvapp->TransferManager->PauseTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_PauseTask);
                    bttvapp->TransferManager->CancelTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_CancelTask);
                    bttvapp->TransferManager->CloseTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_CloseTask);
                    bttvapp->TransferManager->GetTaskList_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_GetTaskList);
                    bttvapp->TransferManager->GetTaskInfo_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_GetTaskInfo);
                    bttvapp->TransferManager->ReservedSpace_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_ReservedSpace);
                    bttvapp->TransferManager->SetInfoPath_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_SetInfoPath);
                    bttvapp->TransferManager->SetTempPath_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_SetTempPath);
                    bttvapp->TransferManager->GetSeedInfo_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_GetSeedInfo);
                    bttvapp->TransferManager->SetStatCallback_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_SetStatCallback);
                    bttvapp->TransferManager->EnableCacheFile_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_EnableCacheFile);
                    bttvapp->TransferManager->SetCustomAlloc_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_SetCustomAlloc);
                    bttvapp->TransferManager->Fini_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_Fini);
                    bttvapp->TransferManager->SetLicense_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_SetLicense);
                    bttvapp->TransferManager->GetThdTaskid_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_GetThdTaskid);
                    bttvapp->TransferManager->PauseAllTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_PauseAllTask);
                    bttvapp->TransferManager->RestartAllTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_RestartAllTask);
                    bttvapp->TransferManager->CloseAllTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_CloseAllTask);
                    bttvapp->TransferManager->CancelAllTask_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_CancelAllTask);
                    bttvapp->TransferManager->GetLastError_pos = af_get_method_pos_by_name(tmp_cntl, TransferInter_METHOD_GetLastError);
                }
            }
        }
        if(af_get_prop_val(cntl, PROP_MOUNTINFO_POS))
        {
            bttvapp->MountInfo = AVMem_calloc(sizeof(StringTable_t),1);
            if(bttvapp->MountInfo)
            {
                control_t* tmp_cntl;
                tmp_cntl = af_find_ctrl_by_handle(af_get_prop_val(cntl,PROP_MOUNTINFO_POS));
                if(tmp_cntl)
                {
                    bttvapp->MountInfo->cntl = tmp_cntl;
                    bttvapp->MountInfo->ColNickNameList_pos = af_get_property_pos_by_name(tmp_cntl, StringTable_PROP_ColNickNameList);
                    bttvapp->MountInfo->GetRowCount_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_GetRowCount);
                    bttvapp->MountInfo->GetColCount_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_GetColCount);
                    bttvapp->MountInfo->GetString_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_GetString);
                    bttvapp->MountInfo->GetRowName_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_GetRowName);
                    bttvapp->MountInfo->GetRowIndex_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_GetRowIndex);
                    bttvapp->MountInfo->AddRow_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_AddRow);
                    bttvapp->MountInfo->DelRow_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_DelRow);
                    bttvapp->MountInfo->SetString_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_SetString);
                    bttvapp->MountInfo->DelString_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_DelString);
                    bttvapp->MountInfo->Release_pos = af_get_method_pos_by_name(tmp_cntl, StringTable_METHOD_Release);
                }
            }
        }
        bttvapp->cntl=cntl;
*/    }
    return bttvapp;
}


/* Handle Prop' method functions, #define HANDLE_METHOD_FUN to enable below code, remove functions that never been used */
#define HANDLE_METHOD_FUN
#ifdef HANDLE_METHOD_FUN
 int TransferManager_SetOption(bttvapp_t* bttvapp , INT32U opt ,INT32U type ,INT32U value )
{
    int ret=-1;
    cond_item_t param[3] = {
        (cond_item_t)opt,
        (cond_item_t)type,
        (cond_item_t)value,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->SetOption(0x%x,0x%x,0x%x)\n",opt,type,value);
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->SetOption_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"SetOption()=>%d\n",ret);
    return ret;
}

 int TransferManager_GetOption(bttvapp_t* bttvapp , INT32U opt ,INT32U type )
{
     /*
    int ret=-1;
    cond_item_t param[2] = {
        (cond_item_t)opt,
        (cond_item_t)type,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->GetOption(0x%x,0x%x)\n",opt,type);
    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->GetOption_pos].func(bttvapp->TransferManager->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"GetOption()=>%d\n",ret);
    return ret;
    */
     return 0;
}

 int TransferManager_Init_Ex(bttvapp_t* bttvapp , char* lic )
{
    int ret=-1;
    ret = transfer_mgr_init() ;
    if(!ret )
            ret = transfer_mgr_module_register( XFER_MODULE_HTTP , NULL ) ;

    if( lic )
    {
            ret = transfer_mgr_module_register( XFER_MODULE_THUNDER , lic ) ;
    }

#ifndef __ROM_
       printf( " TransferManager_Init_Ex ret :%d!\n" , ret ) ;
#endif

    return ret;
}




static void SetDLTaskRow( int i_row , transfer_taskid_t taskid  )
{
	transfer_task_info_t * taskinfo;
	int percent = 0;
	char c_taskid[64]	 = {0};
	char c_state[64]	 = {0};
	//char c_taskinfo[512] = {0};
	char c_name[256]	 = {0};
	char c_percent[64] 	 = {0};
	char c_size[64] 	 = {0};
	char c_rate[64] 	 = {0};
	char c_errno[64] 	 = {0};
	char c_path_file_name[256] = {0}	;
	char c_task_path[256]  = {0};
	char c_disk[24] = {0};
	char c_srcurl[256]	 = {0};

        if(transfer_mgr_get_task_info(taskid, &taskinfo) != 0)
		return;

        snprintf(c_taskid , sizeof(c_taskid), "%d", taskinfo->task_id);

        memset(TaskInfoList[i_row].c_taskid,0,64*sizeof(char));
        memcpy(TaskInfoList[i_row].c_taskid,c_taskid,strlen(c_taskid));


	switch (taskinfo->task_stat){
		case DLST_STOPPED:
		case DLST_NONE:
			snprintf(c_state , sizeof( c_state ), "%s", "pausing");
			break;
		case DLST_DOWNLOADING:
                        //s_downloading_task_count ++ ;
			snprintf(c_state , sizeof( c_state ), "%s", "running");
			break;
		case DLST_PENDING:
		case DLST_CREATING:
			snprintf(c_state , sizeof( c_state ), "%s", "waiting");
			break;
		case DLST_FINISHED:
			snprintf(c_state , sizeof( c_state ), "%s", "done");
			break;
		default://case DLST_ERROR:
			snprintf(c_state , sizeof( c_state ), "%s", "error");
	}
	//task_table_SetString(transfermgr, c_taskid, COL_STATE, c_state);

        memset(TaskInfoList[i_row].c_state,0,64*sizeof(char));
        memcpy(TaskInfoList[i_row].c_state,c_state,strlen(c_state));



	snprintf(c_name , sizeof( c_name ), "%s", taskinfo->task_name);
	c_name[strlen(c_name)] = 0;
	//task_table_SetString(transfermgr, c_taskid, COL_FILENAME, c_name);
        memset(TaskInfoList[i_row].c_name,0,256*sizeof(char));
        memcpy(TaskInfoList[i_row].c_name,c_name,strlen(c_name));


	//snprintf(c_srcurl, sizeof( c_srcurl), "%s", taskinfo->task_src_url);
	//c_srcurl[strlen(c_srcurl)] = 0;
        //task_table_SetString(transfermgr, c_taskid, COL_SRCURL, c_srcurl);
	
	if (taskinfo->total_size > 0)
		percent = (int)((float)taskinfo->downloaded_size * 100 / (float)taskinfo->total_size);
	else
		percent = 0;


        sprintf( c_percent , "%d%%%" , percent );
	//add by zhenlei
	sprintf( c_percent , "%d%%" , percent );
	c_percent[5] = 0;
        //add by zhenlei

	if( DLST_FINISHED == taskinfo->task_stat || 100 == percent )
	{
		if( s_finished_taskid == taskinfo->task_id )
			s_finished_show_times ++ ;
//		else if( 0 == s_finished_taskid  && ( 0 == s_kankan_task_id || s_kankan_task_id != taskinfo->thunder_task_id ))
                else if( 0 == s_finished_taskid  )
                    s_finished_taskid =  taskinfo->task_id ;
		if( 100 != percent )
			percent = 100 ;
		else
			taskinfo->task_stat = DLST_FINISHED ;
		if( 1 == s_finished_show_times )
		{
		    int len_tmp=strlen(taskinfo->task_path) + strlen(taskinfo->task_name) ;
		    char* newitem=AVMem_malloc(4+len_tmp+1);
		    if( newitem )
			{
//				sprintf(newitem,"%x%03x%s%s", REFRESH_FILE_NEW, len_tmp, taskinfo->task_path , taskinfo->task_name );
//		        TransferMgr_send_msg_devst_dev_refresh(cntl, BROADCAST_ALL, DEV_REFRESH_DIR, 1, (char *)newitem);
		        AVMem_free(newitem);
		   }	
		}	
	}
        memset(TaskInfoList[i_row].c_percent,0,64*sizeof(char));
        memcpy(TaskInfoList[i_row].c_percent,c_percent,strlen(c_percent));

//	task_table_SetString(transfermgr, c_taskid, COL_PROGRESS, c_percent);

	sprintf(c_rate, "%.2fKB", (float)taskinfo->speed/1024);

        memset(TaskInfoList[i_row].c_rate,0,64*sizeof(char));
        memcpy(TaskInfoList[i_row].c_rate,c_rate,strlen(c_rate));
//	task_table_SetString(transfermgr, c_taskid, COL_RATE, c_rate);

	if (taskinfo->total_size >= 1024*1024*1024)
		sprintf(c_size, "%.1fGB", taskinfo->total_size/1024.0/1024/1024);
	else if(taskinfo->total_size >= 1024*1024) 
		sprintf(c_size, "%.1fMB", taskinfo->total_size/1024.0/1024);
	else 
		sprintf(c_size, "%.1fKB", taskinfo->total_size/1024.0);
//	task_table_SetString(transfermgr, c_taskid, COL_FILESIZE, c_size);
        memset(TaskInfoList[i_row].c_size,0,64*sizeof(char));
        memcpy(TaskInfoList[i_row].c_size,c_size,strlen(c_size));

     /*
	snprintf( c_task_path , sizeof( c_task_path ) , "%s%s" , taskinfo->task_path , taskinfo->task_name ) ;
	int i_times = 0 ;
	for( int i= 0 ; i < strlen( c_task_path ) ; i++ )
	{
		if( '/' == c_task_path[i] )
			i_times ++ ;
		c_disk[i] = c_task_path[i] ;
		if( 4 == i_times )
		      break ;
	}
	snprintf( c_path_file_name , sizeof( c_path_file_name ) , "%s" , c_task_path + i ) ;	
	task_table_SetString(transfermgr, c_taskid, COL_DISK ,  c_disk );
	task_table_SetString(transfermgr, c_taskid, COL_PATH ,  c_path_file_name);

	if (taskinfo->speed != 0)
	{
		char c_time[16] = {0};
		int remain_time = (taskinfo->total_size - taskinfo->downloaded_size) / taskinfo->speed;
		if (remain_time < 120)
		{
			task_table_SetString(transfermgr, c_taskid, COL_REMAINTIME, "<2分钟");
		}
		else if (remain_time < 3600)
		{
			sprintf(c_time, "%d:%d",  remain_time/60, remain_time%60);
			task_table_SetString(transfermgr, c_taskid, COL_REMAINTIME, c_time);
		}
		else if (remain_time < 24 * 3600)
		{
			sprintf(c_time, "%d:%d:%d", remain_time/3600, remain_time%3600/60, remain_time%60);
			task_table_SetString(transfermgr, c_taskid, COL_REMAINTIME, c_time);
		}
		else
		{
			task_table_SetString(transfermgr, c_taskid, COL_REMAINTIME, ">1天");
		}
	}
	else
		task_table_SetString(transfermgr, c_taskid, COL_REMAINTIME, " --");

	sprintf(c_errno, "%d", taskinfo->error_id);
	task_table_SetString(transfermgr, c_taskid, COL_ERRORNO, c_errno);
	if( 1025 == taskinfo->error_id )
	{
		s_b_error_1025 = TRUE ;
	}
	transfer_mgr_release_task_info(taskinfo);
*/
	
}

 int TransferManager_UpdateTaskInfo(bttvapp_t* bttvapp )
{
    int ret=-1;
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->UpdateTaskInfo()\n");
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->UpdateTaskInfo_pos].func(bttvapp->TransferManager->cntl,NULL));
    //AF_METHOD_RET_LOG(dbg_level,"UpdateTaskInfo()=>%d\n",ret);
    
    //transfermgr_lock();

	transfer_task_list_t*	tasklist = NULL;
	ret = transfer_mgr_get_task_list(&tasklist) ;
        if( ret != 0 )
	{
		
              // transfermgr_unlock();
		return -1;
	}

	for(int i = 0; i < tasklist->task_count; i++)
	{
		SetDLTaskRow( i , tasklist->task_ids[i]);
	}	
	

	
        emit MBttvObject().Instance()->RequestUpdataTask((INT32U)TaskInfoList,tasklist->task_count);
    //transfermgr_unlock();
    return ret;
}

 int TransferManager_UnloadAllTasks(bttvapp_t* bttvapp , char* path )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)path,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->UnloadAllTasks(%s)\n",path);
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->UnloadAllTasks_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"UnloadAllTasks()=>%d\n",ret);
    return ret;
}

 int TransferManager_LoadAllTasks(bttvapp_t* bttvapp , char* path )
{
    int ret=-1;
    ret = transfer_mgr_load( path ) ;
    transfer_taskid_t task_id = -1 ;
    qDebug("%d",ret);
    qDebug("%s",path);
    if( ret == 0 )
    {
          ret =  transfer_mgr_task_start( task_id ) ;
           if(0 != ret )
           {
               transfer_mgr_task_close( task_id ) ;
               task_id = -1 ;
           }
           else
               ret = (int)task_id ;
    }
    return ret;
}

 void transfer_complete_cb_proc(transfer_taskid_t taskid, const char* result, int len, int errno)
 {
      //modify_http_task( taskid , errno , len , result ) ;
     http_sem_lock() ;
     if( !s_getting_task )
     {
                 http_sem_unlock() ;
                 return  ;
     }

     if( taskid != s_getting_task->task_id )
     {
                 http_sem_unlock() ;
                 return  ;
     }

// bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(cntl->private_data);
 char * buff = (char *)result ;

     //s_getting_task->i_error = i_error ;
     s_getting_task->i_result_len = len ;
     s_getting_task->p_result = get_value( buff , len ) ;
     s_getting_task->task_state = HTTP_FINISHED ;
 //s_getting_task->cntl = cntl ;
     http_sem_unlock() ;
}

 int TransferManager_AddTask(bttvapp_t* bttvapp , char* url ,char* saved_path ,char* buff ,INT32U buff_len ,char* post_buff ,INT32U isCallback ,INT32U type ,INT32U mode )
{
    int ret=-1;
    transfer_taskid_t task_id = -1 ;
    cond_item_t param[8] = {
        (cond_item_t)url,
        (cond_item_t)saved_path,
        (cond_item_t)buff,
        (cond_item_t)buff_len,
        (cond_item_t)post_buff,
        (cond_item_t)isCallback,
        (cond_item_t)type,
        (cond_item_t)mode,
        };
    ret = transfer_mgr_task_add( url , saved_path , (transfer_type_e)type , mode , post_buff , ( transfer_complete_cb )transfer_complete_cb_proc , &task_id ) ;
    if( ret == 0 )
    {
          ret =  transfer_mgr_task_start( task_id ) ;
           if(0 != ret )
           {
               transfer_mgr_task_close( task_id ) ;
               task_id = -1 ;
           }
           else
           {
               ret = task_id ;

           }
    }
    return ret;
}

 char* TransferManager_GetResult(bttvapp_t* bttvapp , INT32U taskid )
{
    char* retstr=NULL;
    cond_item_t param[1] = {
        (cond_item_t)taskid,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->GetResult(0x%x)\n",taskid);
    //retstr = (char*)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->GetResult_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"GetResult()=>%s\n",retstr);
    return retstr;
}
 char Thunderpath[256];
 int TransferManager_AddThunderTask(bttvapp_t* bttvapp , char* src_path ,char* gcid ,char* saved_path ,INT32U task_len ,INT32U type ,char* file_name )
{
    int ret=-1;
    cond_item_t param[6] = {
        (cond_item_t)src_path,
        (cond_item_t)gcid,
        (cond_item_t)saved_path,
        (cond_item_t)task_len,
        (cond_item_t)type,
        (cond_item_t)file_name,
        };

    transfer_taskid_t task_id = -1 ;

    sprintf(Thunderpath,"%s%s",saved_path,file_name);

//  QString string(QByteArray());
//  qDebug()<<string;
     qDebug("%s",Thunderpath);
    ret = transfer_mgr_task_add( src_path , Thunderpath, (transfer_type_e)type , 0 , NULL , ( transfer_complete_cb )transfer_complete_cb_proc , &task_id ) ;

    if( ret == 0 )
    {
          ret =  transfer_mgr_task_start( task_id ) ;
           if(0 != ret )
           {
               transfer_mgr_task_close( task_id ) ;
               task_id = -1 ;
           }
           else
               ret = (int)task_id ;
    }
    return ret;
}

 int TransferManager_StartTask(bttvapp_t* bttvapp , INT32U taskid )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)taskid,
        };
    ret =  transfer_mgr_task_start( taskid ) ;
    return ret;
}

 int TransferManager_StartPrioTask(bttvapp_t* bttvapp , INT32U taskid )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)taskid,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->StartPrioTask(0x%x)\n",taskid);
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->StartPrioTask_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"StartPrioTask()=>%d\n",ret);
    return ret;
}

 int TransferManager_PauseTask(bttvapp_t* bttvapp , INT32U taskid )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)taskid,
        };
    ret = transfer_mgr_task_pause( taskid ) ;
    return ret;
}

 int TransferManager_CancelTask(bttvapp_t* bttvapp , INT32U isCallback ,INT32U taskid )
{
    int ret=-1;
    cond_item_t param[2] = {
        (cond_item_t)isCallback,
        (cond_item_t)taskid,
        };
    transfer_mgr_task_close( taskid ) ;
    return ret;
}

 int TransferManager_CloseTask(bttvapp_t* bttvapp , INT32U taskid )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)taskid,
        };
    transfer_mgr_task_close( taskid ) ;
    return ret;
}

 int TransferManager_GetTaskList(bttvapp_t* bttvapp )
{
    int ret=-1;
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->GetTaskList()\n");
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->GetTaskList_pos].func(bttvapp->TransferManager->cntl,NULL));
    //AF_METHOD_RET_LOG(dbg_level,"GetTaskList()=>%d\n",ret);
   
   
   transfer_task_list_t*  tasklist = NULL;
   ret = transfer_mgr_get_task_list(&tasklist);
    
   
    return ret;
}

 int TransferManager_GetTaskInfo(bttvapp_t* bttvapp , INT32U taskid )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)taskid,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->GetTaskInfo(0x%x)\n",taskid);
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->GetTaskInfo_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"GetTaskInfo()=>%d\n",ret);
    
    return ret;
}

 int TransferManager_ReservedSpace(bttvapp_t* bttvapp )
{
    int ret=-1;
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->ReservedSpace()\n");
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->ReservedSpace_pos].func(bttvapp->TransferManager->cntl,NULL));
    //AF_METHOD_RET_LOG(dbg_level,"ReservedSpace()=>%d\n",ret);
    return ret;
}

 int TransferManager_SetInfoPath(bttvapp_t* bttvapp , char* path )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)path,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->SetInfoPath(%s)\n",path);
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->SetInfoPath_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"SetInfoPath()=>%d\n",ret);
    return ret;
}

 int TransferManager_SetTempPath(bttvapp_t* bttvapp , char* path )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)path,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->SetTempPath(%s)\n",path);
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->SetTempPath_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"SetTempPath()=>%d\n",ret);
    return ret;
}

 int TransferManager_GetSeedInfo(bttvapp_t* bttvapp , char* seed_path ,char* src )
{
    int ret=-1;
    cond_item_t param[2] = {
        (cond_item_t)seed_path,
        (cond_item_t)src,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->GetSeedInfo(%s,%s)\n",seed_path,src);
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->GetSeedInfo_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"GetSeedInfo()=>%d\n",ret);
    return ret;
}

 int TransferManager_SetStatCallback(bttvapp_t* bttvapp )
{
    int ret=-1;
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->SetStatCallback()\n");
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->SetStatCallback_pos].func(bttvapp->TransferManager->cntl,NULL));
    //AF_METHOD_RET_LOG(dbg_level,"SetStatCallback()=>%d\n",ret);
    return ret;
}

 int TransferManager_EnableCacheFile(bttvapp_t* bttvapp , INT32U bEnable )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)bEnable,
        };
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->EnableCacheFile(0x%x)\n",bEnable);
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->EnableCacheFile_pos].func(bttvapp->TransferManager->cntl,param));
    //AF_METHOD_RET_LOG(dbg_level,"EnableCacheFile()=>%d\n",ret);
    return ret;
}

 int TransferManager_SetCustomAlloc(bttvapp_t* bttvapp )
{
    int ret=-1;
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->SetCustomAlloc()\n");
    //ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->SetCustomAlloc_pos].func(bttvapp->TransferManager->cntl,NULL));
    //AF_METHOD_RET_LOG(dbg_level,"SetCustomAlloc()=>%d\n",ret);
    return ret;
}

 int TransferManager_Fini(bttvapp_t* bttvapp )
{
    int ret=-1;
    //AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->Fini()\n");
//    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->Fini_pos].func(bttvapp->TransferManager->cntl,NULL));
//    AF_METHOD_RET_LOG(dbg_level,"Fini()=>%d\n",ret);
    return ret;
}

 int TransferManager_SetLicense(bttvapp_t* bttvapp , char* lic )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)lic,
        };
//    AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->SetLicense(%s)\n",lic);
//    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->SetLicense_pos].func(bttvapp->TransferManager->cntl,param));
//    AF_METHOD_RET_LOG(dbg_level,"SetLicense()=>%d\n",ret);
    return ret;
}

 int TransferManager_GetThdTaskid(bttvapp_t* bttvapp , INT32U taskid )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)taskid,
        };
//    AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->GetThdTaskid(0x%x)\n",taskid);
//    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->GetThdTaskid_pos].func(bttvapp->TransferManager->cntl,param));
//    AF_METHOD_RET_LOG(dbg_level,"GetThdTaskid()=>%d\n",ret);
    return ret;
}

 int TransferManager_PauseAllTask(bttvapp_t* bttvapp )
{
    int ret=-1;
//    AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->PauseAllTask()\n");
//    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->PauseAllTask_pos].func(bttvapp->TransferManager->cntl,NULL));
//    AF_METHOD_RET_LOG(dbg_level,"PauseAllTask()=>%d\n",ret);
    return ret;
}

 int TransferManager_RestartAllTask(bttvapp_t* bttvapp )
{
    int ret=-1;
//    AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->RestartAllTask()\n");
//    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->RestartAllTask_pos].func(bttvapp->TransferManager->cntl,NULL));
//    AF_METHOD_RET_LOG(dbg_level,"RestartAllTask()=>%d\n",ret);
    return ret;
}

 int TransferManager_CloseAllTask(bttvapp_t* bttvapp )
{
    int ret=-1;
//    AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->CloseAllTask()\n");
//    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->CloseAllTask_pos].func(bttvapp->TransferManager->cntl,NULL));
//    AF_METHOD_RET_LOG(dbg_level,"CloseAllTask()=>%d\n",ret);
    return ret;
}

 int TransferManager_CancelAllTask(bttvapp_t* bttvapp )
{
    int ret=-1;
//    AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->CancelAllTask()\n");
//    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->CancelAllTask_pos].func(bttvapp->TransferManager->cntl,NULL));
//    AF_METHOD_RET_LOG(dbg_level,"CancelAllTask()=>%d\n",ret);
    return ret;
}

 int TransferManager_GetLastError(bttvapp_t* bttvapp )
{
    int ret=-1;
//    AF_METHOD_LOG(dbg_level,"BttvApp->TransferManager->GetLastError()\n");
//    ret = (int)(bttvapp->TransferManager->cntl->cntl_info->method[bttvapp->TransferManager->GetLastError_pos].func(bttvapp->TransferManager->cntl,NULL));
//    AF_METHOD_RET_LOG(dbg_level,"GetLastError()=>%d\n",ret);
    return ret;
}

 int MountInfo_GetRowCount(bttvapp_t* bttvapp )
{
    int ret=-1;
//    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->GetRowCount()\n");
//    ret = (int)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->GetRowCount_pos].func(bttvapp->MountInfo->cntl,NULL));
//    AF_METHOD_RET_LOG(dbg_level,"GetRowCount()=>%d\n",ret);
    return ret;
}
/*
 int MountInfo_GetColCount(bttvapp_t* bttvapp , char* row_name )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)row_name,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->GetColCount(%s)\n",row_name);
    ret = (int)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->GetColCount_pos].func(bttvapp->MountInfo->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"GetColCount()=>%d\n",ret);
    return ret;
}

static char* MountInfo_GetString(bttvapp_t* bttvapp , char* row_name ,INT32U col_index )
{
    char* retstr=NULL;
    cond_item_t param[2] = {
        (cond_item_t)row_name,
        (cond_item_t)col_index,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->GetString(%s,0x%x)\n",row_name,col_index);
    retstr = (char*)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->GetString_pos].func(bttvapp->MountInfo->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"GetString()=>%s\n",retstr);
    return retstr;
}

static char* MountInfo_GetRowName(bttvapp_t* bttvapp , INT32U row_index )
{
    char* retstr=NULL;
    cond_item_t param[1] = {
        (cond_item_t)row_index,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->GetRowName(0x%x)\n",row_index);
    retstr = (char*)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->GetRowName_pos].func(bttvapp->MountInfo->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"GetRowName()=>%s\n",retstr);
    return retstr;
}

static int MountInfo_GetRowIndex(bttvapp_t* bttvapp , char* row_name )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)row_name,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->GetRowIndex(%s)\n",row_name);
    ret = (int)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->GetRowIndex_pos].func(bttvapp->MountInfo->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"GetRowIndex()=>%d\n",ret);
    return ret;
}

static int MountInfo_AddRow(bttvapp_t* bttvapp , char* row_name )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)row_name,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->AddRow(%s)\n",row_name);
    ret = (int)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->AddRow_pos].func(bttvapp->MountInfo->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"AddRow()=>%d\n",ret);
    return ret;
}

static int MountInfo_DelRow(bttvapp_t* bttvapp , char* row_name )
{
    int ret=-1;
    cond_item_t param[1] = {
        (cond_item_t)row_name,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->DelRow(%s)\n",row_name);
    ret = (int)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->DelRow_pos].func(bttvapp->MountInfo->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"DelRow()=>%d\n",ret);
    return ret;
}

static int MountInfo_SetString(bttvapp_t* bttvapp , char* row_name ,INT32U col_index ,char* string )
{
    int ret=-1;
    cond_item_t param[3] = {
        (cond_item_t)row_name,
        (cond_item_t)col_index,
        (cond_item_t)string,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->SetString(%s,0x%x,%s)\n",row_name,col_index,string);
    ret = (int)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->SetString_pos].func(bttvapp->MountInfo->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"SetString()=>%d\n",ret);
    return ret;
}

static int MountInfo_DelString(bttvapp_t* bttvapp , char* row_name ,INT32U col_index )
{
    int ret=-1;
    cond_item_t param[2] = {
        (cond_item_t)row_name,
        (cond_item_t)col_index,
        };
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->DelString(%s,0x%x)\n",row_name,col_index);
    ret = (int)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->DelString_pos].func(bttvapp->MountInfo->cntl,param));
    AF_METHOD_RET_LOG(dbg_level,"DelString()=>%d\n",ret);
    return ret;
}

static int MountInfo_Release(bttvapp_t* bttvapp )
{
    int ret=-1;
    AF_METHOD_LOG(dbg_level,"BttvApp->MountInfo->Release()\n");
    ret = (int)(bttvapp->MountInfo->cntl->cntl_info->method[bttvapp->MountInfo->Release_pos].func(bttvapp->MountInfo->cntl,NULL));
    AF_METHOD_RET_LOG(dbg_level,"Release()=>%d\n",ret);
    return ret;
}
*/
#endif /*HANDLE_METHOD_FUN*/


/* message send functions, #define MSG_SEND_FUN to enable below code, remove functions that never been used */
#define MSG_SEND_FUN
#ifdef MSG_SEND_FUN

static int bttvapp_send_msg_net_datasrc_ready(control_t* cntl, handle_t handle, INT32U ret_value, char* err_msg, INT32U datasrc_id, INT32U parent_id, INT32U rawdata, INT32U count, INT32U stage)
{
    int ret=0;
    //retValue;
    //err_msg;
    //datasrc_id;
    //parent_id;
    //rawdata;
    //count;
    //stage;
    MBttvObject().Instance()->RequestDataReady(ret_value, err_msg, datasrc_id, parent_id, rawdata, count, stage);
    return ret;
}

static int bttvapp_send_msg_net_vob_ready(control_t* cntl, handle_t handle, INT32U ret_value, char* err_msg, INT32U datasrc_id, INT32U parent_id, INT32U rawdata, INT32U count, INT32U stage)
{
    int ret=0;
    //retValue;
    //err_msg;
    //datasrc_id;
    //parent_id;
    //rawdata;
    //count;
    //stage;
    //MBttvObject().Instance()->RequestDataReady(ret_value, err_msg, datasrc_id, parent_id, rawdata, count, stage);
    return ret;
}

static int bttvapp_send_msg_net_pic_download_ready(control_t* cntl, handle_t handle, INT32U ret_value, char* err_msg, INT32U parent_id, INT32U item_id, INT32U image_data, INT32U data_size, char* image_type, INT32U stage)
{
    int ret=0;
    //ret_value;
    //err_msg;
    //parent_id;
    //item_id;
    //image_data;
    //data_size;
    //image_type;
    //stage;
    MBttvObject().Instance()->RequestPicReady(ret_value, err_msg, parent_id, item_id, image_data, data_size, image_type, stage);
    return ret;
}

static int bttvapp_send_msg_netst_netif_off(control_t* cntl, handle_t handle)
{
    int ret=0;

    return ret;
}

#endif /*MSG_SEND_FUN*/


/* Above code is created by AfControlWizard */

/* Add User Code here */
static int s_pic2file_taskid = -1;

/*end*/


#define MountInfo_ColNickNameList_str(subindex)                af_get_array_str(uskywnetapp->MountInfo->cntl,uskywnetapp->MountInfo->ColNickNameList_pos, subindex)
#define MountInfo_ColNickNameList_str_Set(subindex,str)        af_set_array_str(uskywnetapp->MountInfo->cntl,uskywnetapp->MountInfo->ColNickNameList_pos, subindex,str)
#define MountInfo_ColNickNameList_str_Num                af_get_property_array_by_pos(uskywnetapp->MountInfo->cntl,uskywnetapp->MountInfo->ColNickNameList_pos, NULL)
#define MountInfo_ColNickNameList_str_Array_Resize(size)        af_resize_property_array_by_pos(uskywnetapp->MountInfo->cntl,uskywnetapp->MountInfo->ColNickNameList_pos, size)

int g_sky_ErrorNo = 0;
#define this_ErrorNo                   g_sky_ErrorNo
#define this_ErrorNo_Set(val)          g_sky_ErrorNo=val

/*****************************************************************
**                                                              **
**    Method  functions                                         **
**                                                              **
**                                                              **
*****************************************************************/
static int BttvApp_Init(control_t* cntl, cond_item_t* param)
{
    int ret=-1;
    bttvapp_t* bttvapp= bttvapp_init(cntl);
    if(bttvapp)
    {
        ret=0;
        /*  Add Initialize Code here , set ret to be -1 if fail*/

        /* end */
        cntl->private_data=bttvapp;
    }
    if(ret<0)
        bttvapp_release(cntl);

    return ret;
}


static int BttvApp_UnInit(control_t* cntl, cond_item_t* param)
{
    bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(cntl->private_data);
    /* Add UnInitialize code here */

    /* end */
    bttvapp_release(cntl);
    return 0;
}

static int BttvApp_MsgProcess(control_t* cntl, cond_item_t* param)
{
#if 0
    bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(cntl->private_data);
    message_t* msg = (message_t *)param;
    switch(msg->id){
        case AFMSG_XFER_GOT_CALLBACK:
            AF_MSG_LOG(dbg_level,"BttvApp<=AFMSG_XFER_GOT_CALLBACK(0x%x,%s,0x%x,0x%x)\n",msg->param[0],(char*)(msg->param[1]),msg->param[2],msg->param[3]);
            /* Add Message Process code here */
			create_parse_xml_task();
			int task_id , len , error_no ;
			task_id = msg->param[0] ;
			char * buff = NULL ;
			len = msg->param[2] ;
			error_no = msg->param[3] ;
			modify_http_task( task_id , error_no , len , cntl ) ;

            /*end*/
            break;
        case AFMSG_DEVST_POST_MOUNT:
            AF_MSG_LOG(dbg_level,"BttvApp<=AFMSG_DEVST_POST_MOUNT(%s,%s,%s,%s,0x%x,0x%x)\n",(char*)(msg->param[0]),(char*)(msg->param[1]),(char*)(msg->param[2]),(char*)(msg->param[3]),msg->param[4],msg->param[5]);
            /* Add Message Process code here */
			{
				char * dev_name = (char *)msg->param[0] ;
				char * dev_type_str = (char *)msg->param[1] ;
				char * mount_point = (char *)msg->param[2] ;			
				char * mount_label = (char *)msg->param[3] ;
				char * download_path = this_DownloadPath_str ;
				if( strstr( download_path , mount_point ))
				{
					TransferManager_LoadAllTasks( bttvapp , this_DownloadPath_str ) ;
				}
			}
			/*end*/
            break;
        case AFMSG_DEVST_POST_UNMOUNT:
        	AF_MSG_LOG(dbg_level,"BttvApp<=AFMSG_DEVST_POST_UNMOUNT(%s,%s,%s,%s,0x%x,0x%x)\n",(char*)(msg->param[0]),(char*)(msg->param[1]),(char*)(msg->param[2]),(char*)(msg->param[3]),msg->param[4],msg->param[5]);
            /* Add Message Process code here */
			{
				char * dev_name = (char *)msg->param[0] ;
				char * dev_type_str = (char *)msg->param[1] ;
				char * mount_point = (char *)msg->param[2] ;			
				char * mount_label = (char *)msg->param[3] ;
				
				char * download_path = this_DownloadPath_str ;
				if( strstr( download_path , mount_point))
				{
					TransferManager_CancelAllTask( bttvapp ) ;
				}
            }

            /*end*/
            break;
        case AFMSG_NETST_NETIF_READY:
            AF_MSG_LOG(dbg_level,"BttvApp<=AFMSG_NETST_NETIF_READY()\n");
            /* Add Message Process code here */
			TransferManager_RestartAllTask( bttvapp ) ;
            /*end*/
            break;
        case AFMSG_NETST_NETIF_OFF:
            AF_MSG_LOG(dbg_level,"BttvApp<=AFMSG_NETST_NETIF_OFF()\n");
            /* Add Message Process code here */
			TransferManager_PauseAllTask( bttvapp ) ;

            /*end*/
            break;
        default:
            break;
    }
#endif
    return 0;
}
static INT8U s_test_ = 0 ;
static int current_dldisk_mount( bttvapp_t* bttvapp , char * c_download_path ) ;

int BttvApp_RequestData(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    bttvapp_t* bttvapp = NULL ;// NULL ;// NULL ;// (bttvapp_t*)(cntl->private_data);
    int datasrc_id = (int)(param[0]);
    int parent_id = (int)(param[1]);
    int item_id = (int)(param[2]);
    char* keyword = (char*)(param[3]);
    /* Add Method code here, Set ret to be -1 if fail */
	int task_id = -1 ;
	int http_get_type ;
            create_parse_xml_task();
	http_sem_lock() ;
	if( s_getting_task )
	{
		TransferManager_CancelTask( bttvapp , 0 , s_getting_task->task_id ) ;
		release_http_task( &s_getting_task ) ;
	}

	char c_url[256] = {0} ;
	if( 0 == datasrc_id )
	{
		if( s_navigate && s_navigate[0] )
		{
			bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 ,NULL , datasrc_id , item_id , (INT32U)s_navigate[0] , s_navigate[0]->nodes_count , 0 ) ;
			http_sem_unlock() ; 
			return 0 ;
		}
		else
		{
			#if(MOVIE_VERSION == 2)
			{
				ret = parse_movie_root_channel(&s_navigate_page_count);
				if( s_navigate && s_navigate[0] )
				{
					bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 ,NULL , datasrc_id , item_id , (INT32U)s_navigate[0] , s_navigate[0]->nodes_count , 0 ) ;
					http_sem_unlock() ; 
					return 0 ;
				}
				else
				{
					bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 ,NULL , datasrc_id , item_id , 0, 0 , 0 ) ;				
					http_sem_unlock() ; 
					return ret;
				}
			}
			#endif
					
			snprintf( c_url , sizeof( c_url ) , "%s" , this_RootURL_str ) ;
			http_get_type = ROOT_CHANNEL_GET ;
		}	
	}
	else if( 1 == datasrc_id )
	{
		if( s_navigate && s_navigate_page_count > 1 && s_navigate[ item_id + 1] )
		{
			s_current_pic_ctx = s_navigate ;
			bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 ,NULL , datasrc_id , item_id , (INT32U)s_navigate[ item_id + 1] , s_navigate[ item_id + 1]->nodes_count , 0 ) ;
			http_sem_unlock() ; 
			return 0 ;
		}
		else
		{
			#if(MOVIE_VERSION == 2)
			{
				if(s_navigate && s_navigate[0] && item_id >= 0 && item_id < s_navigate[0]->nodes_count &&
					s_navigate[0]->nodes[item_id].xml)
				{
					snprintf( c_url , sizeof( c_url ) , "%s" , s_navigate[0]->nodes[item_id].xml );			
				}
			#ifdef MOVIE_HAS_HD
				if((parent_id == HD_MOVIE_CH1) ||(parent_id == HD_MOVIE_CH2) || (parent_id == HD_MOVIE_CH3))
					http_get_type = HD_GET ;	
				else
					http_get_type = MOVIE_GET ;	
			#else
				http_get_type = MOVIE_GET ;
			#endif
			}
			#else
			{
				snprintf( c_url , sizeof( c_url ) , "%s" , this_RootURL_str ) ;
				char * c_str = strstr( c_url , ".com" ) ;
				if( c_str && s_navigate && s_navigate[0])
				{
					*( c_str + 4 ) = '\0' ;
					int i_channel_type = get_channel_type( s_navigate[0]->nodes[item_id].name ) ;
					//For xunlei rank_list bug. by Peifu
					if(i_channel_type == 99)
						snprintf( c_url + strlen( c_url ) , sizeof( c_url ) - strlen( c_url ) , "/top_movie?rtype=2&num=36&st=36" ) ;
					else
						snprintf( c_url + strlen( c_url ) , sizeof( c_url ) - strlen( c_url ) , "/top_movie?rtype=%d&num=36&st=0" , i_channel_type ) ;
				}
				http_get_type = MOVIE_GET ;				
			}
			#endif 
		}	
	}
	else if( keyword && 2 == datasrc_id )
	{
		int i_len = 0;
		if(strcmp(this_SearchType_str, "BT") == 0)
			i_len = snprintf( c_url , sizeof( c_url ) , "%s" , this_SearchBtURL_str ) ;
		else
			i_len = snprintf( c_url , sizeof( c_url ) , "%s" , this_SearchURL_str ) ;			
		if( c_url[0] )
		{
			char * p_url = c_url + i_len ; 
			if( 0 == strlen(keyword) && s_navigate && s_navigate[parent_id+1] && s_navigate[parent_id + 1]->nodes_count > item_id )
			{
				keyword = s_navigate[parent_id+1]->nodes[item_id].name ;
			}

			if( strlen(keyword) > 0)
			{
				while( *keyword )
				{
					if( *keyword > 127 )
						p_url += snprintf( p_url,  sizeof( c_url ) - strlen( c_url ) ,"%%%02X", *keyword++ );
					else
						{
							//p_url += snprintf( p_url,  sizeof( c_url ) - strlen( c_url ) ,"%s", keyword++ );	//modified by zhenlei
							snprintf( p_url,  sizeof( c_url ) - strlen( c_url ) ,"%s", keyword++ );
							p_url += 1;
						}
				}
			}
			else
			{
				http_sem_unlock() ; 
				return NO_SEARCH_WORD ;
			}
		}
		
		http_get_type = SEARCH_GET ;
	}
	else if( 3 == datasrc_id )
	{
		char * c_cid = s_search_result->nodes[ item_id ].cid ;
		char c_file_name[256] ;
		snprintf( c_file_name , sizeof( c_file_name ) , "%s.%s" , s_search_result->nodes[item_id].name , s_search_result->nodes[item_id].format ) ;
		check_file_name( c_file_name ) ;
		int task_id = 0 ;
		if( c_cid )
		{
			char * download_path = this_DownloadPath_str ;
			if( !current_dldisk_mount( bttvapp , download_path ) )
			{
				task_id = DOWNLOAD_DISK_UNMOUNT ;
			}
			else
			{
				float f_free_size = get_disk_freespace( download_path ) ;
				//by Peifu Jiang
                                float f_reserved_size = 1024 * 1024 * 1024;//(float)transfer_mgr_reserved_space() * 1024 * 1024;
				float f_seed_size = s_search_result->nodes[ item_id ].size;
						
				//if( f_free_size < s_search_result->nodes[ item_id ].size )
				if( f_seed_size + f_reserved_size + 10.0*1024*1024 > f_free_size )
				{
					task_id = NO_ENOUGH_SPACE ;
				}
				else
				{
					task_id = TransferManager_AddThunderTask( bttvapp ,c_cid , NULL ,download_path , (INT32U)s_search_result->nodes[ item_id ].size , 5 , c_file_name);
					if( task_id < 0 )
					{
						int i_error = TransferManager_GetLastError( bttvapp ) ;
						this_ErrorNo_Set( i_error ) ;
						task_id = i_error;
					}
					else
					{
						this_ErrorNo_Set( 0 ) ;
						if((CurDLSeedInfo.parent_id == parent_id)
								&& (CurDLSeedInfo.item_id == item_id)
								&& (0 == strcmp(CurDLSeedInfo.cid, c_cid))
								&&(CurDLSeedInfo.src_url)) 
						{						
							transfer_mgr_set_task_src_url(task_id, CurDLSeedInfo.src_url);
						}
					}
				}	
			}
		}
		else
			task_id = DOWNLOAD_URL_NULL ;
		http_sem_unlock() ; 
		return task_id ;
	}
	if( 4 == datasrc_id )
	{
		if( s_kankan && s_kankan[0] )
		{
			bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 ,NULL , datasrc_id , item_id , (INT32U)s_kankan[0] , s_kankan[0]->nodes_count , 0 ) ;
			http_sem_unlock() ; 
			return 0 ;
		}
		else
		{
			#if(KANKAN_VERSION == 2)
			{
				ret = parse_kankan_root_channel(&s_kankan_page_count);
				if( s_kankan && s_kankan[0] )
				{
					bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 ,NULL , datasrc_id , item_id , (INT32U)s_kankan[0] , s_kankan[0]->nodes_count , 0 ) ;
					http_sem_unlock() ; 
					return 0 ;
				}
				else
				{
					bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 ,NULL , datasrc_id , item_id , 0, 0 , 0 ) ;				
					http_sem_unlock() ; 
					return ret;
				}
			}
			#endif
			
			if(this_kankanNaviUrl_str)
				snprintf( c_url , sizeof( c_url ) , "%s" , this_kankanNaviUrl_str ) ;
			else
				snprintf( c_url , sizeof( c_url ) , "%s" , "http://preview.kankan.xunlei.com/movie/list.xml" ) ;
			http_get_type = KANKAN_ROOT_GET ;				
		}	
	}
	else if( 5 == datasrc_id )
	{	
		if( s_kankan && s_kankan[0] && keyword && (*keyword))
		{
			int i_len;
			if(this_kankanSearUrl_str)
				i_len = snprintf( c_url , sizeof( c_url ) , "%s" , this_kankanSearUrl_str ) ;
			else
				i_len = snprintf( c_url , sizeof( c_url ) , "%s" , "http://movie.em.n0808.com/ce/search?num=50&keyword=" ) ;
			if( c_url[0] )
			{
				char * p_url = c_url + i_len ;	
				if( strlen(keyword) > 0)
				{
					while( *keyword )
					{
						if( *keyword > 127 )
							p_url += snprintf( p_url,  sizeof( c_url ) - strlen( c_url ) ,"%%%02X", *keyword++ );
						else
							p_url += snprintf( p_url,  sizeof( c_url ) - strlen( c_url ) ,"%s", keyword++ );
					}
				}
			}			
			http_get_type = KANKAN_SEARCH_GET ;
		}
		else if( s_kankan && s_kankan_page_count > 1 && s_kankan[ item_id + 1] )
		{
			s_current_pic_ctx = s_kankan ;
			bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 ,NULL , datasrc_id , item_id , (INT32U)s_kankan[ item_id + 1] , s_kankan[ item_id + 1]->nodes_count , 0 ) ;
			http_sem_unlock() ; 
			return 0 ;
		}
		else
		{
			if( s_kankan && s_kankan[0]->channel_count > item_id && s_kankan[0]->nodes[item_id].xml )
			{
				snprintf( c_url , sizeof( c_url ) , "%s" , s_kankan[0]->nodes[item_id].xml ) ;
				http_get_type = KANKAN_MOVIE_CH_GET ;
			}
		}
	}
	else if( 6 == datasrc_id )//for storage picture to file and download the movie(same as 3 == datasrc_id)
	{
		char * c_cid = s_search_result->nodes[ item_id ].cid ;
		char c_file_name[256] ;
		//int task_id = 0 ;
		snprintf( c_file_name , sizeof( c_file_name ) , "%s.%s" , s_search_result->nodes[item_id].name , s_search_result->nodes[item_id].format ) ;
		check_file_name( c_file_name ) ;
		
		if( c_cid )
		{
			char * download_path = this_DownloadPath_str ;
			if(download_path && download_path[0])
			{
				float f_free_size = get_disk_freespace( download_path ) ;
				//by Peifu Jiang
                                float f_reserved_size = 1024*1024*1024;//(float)transfer_mgr_reserved_space() * 1024 * 1024;
				float f_seed_size = s_search_result->nodes[ item_id ].size;
						
				//if( f_free_size < s_search_result->nodes[ item_id ].size )
				if( f_seed_size + f_reserved_size + 10.0*1024*1024 > f_free_size )
				{
					task_id = NO_ENOUGH_SPACE ;
				}
				else
				{
					task_id = TransferManager_AddThunderTask( bttvapp ,c_cid , NULL ,download_path , (INT32U)s_search_result->nodes[ item_id ].size , 5 , c_file_name);
					if( task_id < 0 )
					{
						int i_error = TransferManager_GetLastError( bttvapp ) ;
						this_ErrorNo_Set( i_error ) ;
						task_id = i_error;
					}
					else
					{
						this_ErrorNo_Set( 0 ) ;
						if((CurDLSeedInfo.parent_id == parent_id)
								&& (CurDLSeedInfo.item_id == item_id)
								&& (0 == strcmp(CurDLSeedInfo.cid, c_cid))
								&&(CurDLSeedInfo.src_url)) 
						{						
							transfer_mgr_set_task_src_url(task_id, CurDLSeedInfo.src_url);
						}
						
						//save the picture to file.
						if(parent_id >= 0 && s_navigate && s_navigate[0] &&
							parent_id+1 < s_navigate[0]->nodes_count && keyword && keyword[0])
						{
							int item_rcmd = atoi(keyword);
							if(item_rcmd >= 0 && s_navigate[parent_id + 1] && item_rcmd < s_navigate[parent_id + 1]->nodes_count)
							{
								char pic_file[256];
								MEM_PIC_INFO * mem_pic ; //the movie picture of memory type.
						
								snprintf( pic_file , sizeof( pic_file ) , "%s%s.%s" , download_path, s_search_result->nodes[item_id].name , pic_format[JPG] ) ;
						
								mem_pic = s_navigate[parent_id+1]->nodes[item_rcmd].mem_pic ;
								if(mem_pic && mem_pic->pic_buff && mem_pic->pic_buff_len)
								{
									int fd;
									if((fd = open(pic_file, O_CREAT|O_WRONLY)) != -1)
									{
										write(fd, mem_pic->pic_buff, mem_pic->pic_buff_len);
										close(fd);
									}
								}
								else
								{
									char* pic;				//the url of the picture
									pic = s_navigate[parent_id+1]->nodes[item_rcmd].pic ;
									if(s_pic2file_taskid >= 0)
									{
										TransferManager_CancelTask( bttvapp , 0 , s_pic2file_taskid) ;
										s_pic2file_taskid = -1;
									}
									s_pic2file_taskid = TransferManager_AddTask( bttvapp , pic, pic_file , NULL , 0 , NULL , 1 , XFER_TASK_HTTP , XFER_NOCACHE );
									if( s_pic2file_taskid < 0 )
									{
										int i_error = TransferManager_GetLastError( bttvapp ) ;
										this_ErrorNo_Set( i_error ) ;
									}
									else
										this_ErrorNo_Set( 0 ) ;
								}
							}
						}		
					}
				}
			}
			else
			{
				task_id = DOWNLOAD_DISK_UNMOUNT ;
			}
		}
		else
		{
			task_id = DOWNLOAD_URL_NULL ;
		}
		http_sem_unlock() ; 
		return task_id ;
	}
	else if( 7 == datasrc_id )
	{
		if( s_kankan && s_kankan[ parent_id + 1] && s_kankan[ parent_id + 1]->nodes[item_id].xml )
		{
			snprintf( c_url , sizeof( c_url ) , "%s" , s_kankan[ parent_id + 1]->nodes[item_id].xml ) ;
			http_get_type = KANKAN_VOD_GET ;
		}
	}
	else if( 8 == datasrc_id )
	{
		int task_id = 0 ;
		if( s_kankan_programe && s_kankan_programe->vod_count >= item_id/* && s_kankan_programe->p_vod[item_id]*/ )
		{
			int j = -1;
			for( int i = 0 ; i < s_kankan_programe->vod_count ; i++ )
			{
				if( s_kankan_programe->p_vod[i].be_new_part )
					j++ ;
				if( j == item_id )
				{
					//modify by peifu, fix a bug
					if(! (strstr( s_kankan_programe->p_vod[i].format ,"rm"  ) || strstr( s_kankan_programe->p_vod[i].format ,"RM"  ) ))
					{
						task_id = NO_SUPPORT_FORMAT ;
						break ;
					}
					char c_file_name[256] ;
					char * download_path = this_DownloadPath_str ;
					snprintf( c_file_name , sizeof( c_file_name ) , "%s.%s" , s_kankan_programe->p_vod[i].name , s_kankan_programe->p_vod[i].format ) ;
					check_file_name( c_file_name ) ;
					unlink( c_file_name ) ;
					TransferManager_PauseAllTask( bttvapp ) ;
					task_id = TransferManager_AddThunderTask( bttvapp ,s_kankan_programe->p_vod[i].cid , s_kankan_programe->p_vod[i].g_cid , download_path , (INT32U)s_kankan_programe->p_vod[i].size , 7 , c_file_name);
					/*
					#ifndef __ROM_
					printf( "\nadd kankan task %s  task id \n!" , c_file_name , task_id ) ;
					#endif
					*/
					if( task_id > 0 )
					{
						this_ErrorNo_Set( 0 ) ;
						//TransferManager_StartPrioTask( bttvapp , task_id ) ;
						s_kankan_programe->cur_download_blok = item_id;//0 ;
						s_kankan_programe->cur_download_part = 0;//item_id ;
					}
					else
					{
						TransferManager_RestartAllTask( bttvapp ) ;
						int i_error = TransferManager_GetLastError( bttvapp ) ;
						this_ErrorNo_Set( i_error ) ;
					}
					break ;
				}
			}
		}	
		http_sem_unlock() ; 
		return task_id ;
	}
	else if( 9 == datasrc_id )
	{
		int task_id = 0 ;
		if( s_kankan_programe && 1 == s_kankan_programe->part_count && s_kankan_programe->cur_download_blok == s_kankan_programe->vod_count - 1 )
		{
			task_id = VOD_OVER ;
		}
		else if( s_kankan_programe && s_kankan_programe->part_count >= parent_id )
		{
			int j = -1;
			for( int i = 0 ; i < s_kankan_programe->vod_count ; i++ )
			{
				if( s_kankan_programe->p_vod[i].be_new_part )
					j++ ;
				if( j == parent_id )
				{
					/*for( int k = 0 ; k + i < s_kankan_programe->vod_count ; k++ )
					{
						if( k == s_kankan_programe->cur_download_blok + 1 )
							break ;
					}*/
					int k = s_kankan_programe->cur_download_blok + 1;
					if( k < s_kankan_programe->vod_count )
					{
						char c_file_name[256] ;
						//modify by peifu, fix a bug
						if(! (strstr( s_kankan_programe->p_vod[k].format ,"rm"  ) || strstr( s_kankan_programe->p_vod[k].format ,"RM"  ) ))
						{
							task_id = NO_SUPPORT_FORMAT ;
							break ;
						}
						char * download_path = this_DownloadPath_str ;
						snprintf( c_file_name , sizeof( c_file_name ) , "%s.%s" , s_kankan_programe->p_vod[k].name , s_kankan_programe->p_vod[k].format ) ;
						check_file_name( c_file_name ) ;
						unlink( c_file_name ) ;
						TransferManager_PauseAllTask( bttvapp ) ;	
						task_id = TransferManager_AddThunderTask( bttvapp ,s_kankan_programe->p_vod[k].cid , s_kankan_programe->p_vod[k].g_cid , download_path , (INT32U)s_kankan_programe->p_vod[k].size , 7 , c_file_name);
						/*
						#ifndef __ROM_
						printf( "\nadd kankan task %s  task id %d \n!" , c_file_name , task_id ) ;
						#endif
						*/
						if( task_id > 0 )
						{
							this_ErrorNo_Set( 0 ) ;
							//TransferManager_StartPrioTask( bttvapp , task_id ) ;
							s_kankan_programe->cur_download_blok ++ ;
						}
						else
						{
							TransferManager_RestartAllTask( bttvapp ) ;
							int i_error = TransferManager_GetLastError( bttvapp ) ;
							this_ErrorNo_Set( i_error ) ;
						}	
						http_sem_unlock() ; 
						return task_id ;
					}
					else
						task_id = VOD_OVER ;
				}
			}
		}	
		else
		{			
			task_id = VOD_OVER ;
		}
		http_sem_unlock() ; 
		return task_id ;
	}
	else if( 10 == datasrc_id ) //add by peifu for prev vod
	{
		int task_id = 0 ;
		if( s_kankan_programe && 1 == s_kankan_programe->part_count && s_kankan_programe->cur_download_blok == 0 )
		{
			task_id = VOD_OVER ;
		}
		else if( s_kankan_programe && s_kankan_programe->part_count >= parent_id )
		{
			int j = -1;
			for( int i = 0 ; i < s_kankan_programe->vod_count ; i++ )
			{
				if( s_kankan_programe->p_vod[i].be_new_part )
					j++ ;
				if( j == parent_id )
				{
					int k = s_kankan_programe->cur_download_blok - 1;
					if(( k >= 0) && (k < s_kankan_programe->vod_count) )
					{
						char c_file_name[256] ;
						//modify by peifu, fix a bug
						if(! (strstr( s_kankan_programe->p_vod[k].format ,"rm"  ) || strstr( s_kankan_programe->p_vod[k].format ,"RM"  ) ))
						{
							task_id = NO_SUPPORT_FORMAT ;
							break ;
						}
						char * download_path = this_DownloadPath_str ;
						snprintf( c_file_name , sizeof( c_file_name ) , "%s.%s" , s_kankan_programe->p_vod[k].name , s_kankan_programe->p_vod[k].format ) ;
						check_file_name( c_file_name ) ;
						unlink( c_file_name ) ;
						TransferManager_PauseAllTask( bttvapp ) ;	
						task_id = TransferManager_AddThunderTask( bttvapp ,s_kankan_programe->p_vod[k].cid , s_kankan_programe->p_vod[k].g_cid , download_path , (INT32U)s_kankan_programe->p_vod[k].size , 7 , c_file_name);
						/*
						#ifndef __ROM_
						printf( "\nadd kankan task %s  task id %d \n!" , c_file_name , task_id ) ;
						#endif
						*/
						if( task_id > 0 )
						{
							this_ErrorNo_Set( 0 ) ;
							//TransferManager_StartPrioTask( bttvapp , task_id ) ;
							s_kankan_programe->cur_download_blok -- ;
						}
						else
						{
							TransferManager_RestartAllTask( bttvapp ) ;
							int i_error = TransferManager_GetLastError( bttvapp ) ;
							this_ErrorNo_Set( i_error ) ;
							task_id = i_error;
						}	
						http_sem_unlock() ; 
						return task_id ;
					}
					else
						task_id = VOD_OVER ;
				}
			}
		}	
		else
		{			
			task_id = VOD_OVER ;
		}
		http_sem_unlock() ; 
		return task_id ;
	}
	else if( 11 == datasrc_id )//get the source url of the download seed
	{
		char * c_cid = s_search_result->nodes[ item_id ].cid ;
		char * src_url = NULL;

		if( c_cid )	 {
			if((0 == strcmp(c_cid, CurDLSeedInfo.cid)) && CurDLSeedInfo.src_url ) {
				src_url = CurDLSeedInfo.src_url;
				snprintf( s_error_msg , sizeof( s_error_msg ) , "%s" , src_url ) ;
				s_error_msg[127] = 0;				
				bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , 0 , s_error_msg , datasrc_id , item_id ,(INT32U)0 , 0 , 1) ;				
				http_sem_unlock() ; 
				return 0 ;
			}
			
			memset(CurDLSeedInfo.cid, 0, sizeof(CurDLSeedInfo.cid));
			memset(CurDLSeedInfo.src_url, 0, sizeof(CurDLSeedInfo.src_url));
			CurDLSeedInfo.datasrc_id = datasrc_id;
			CurDLSeedInfo.parent_id = parent_id;
			CurDLSeedInfo.item_id = item_id;
			snprintf( CurDLSeedInfo.cid , sizeof( CurDLSeedInfo.cid  ) , "%s" , c_cid ) ;
			snprintf( c_url , sizeof( c_url ) , "%s%s" , "http://dy.n0808.com/furl?cid=", c_cid ) ;		
			http_get_type = SEED_SRC_URL_GET;
		}
		else {
			http_sem_unlock() ;
			return -1;
		}
	}
	else if( 12 == datasrc_id && keyword)//get pinyin suggest words
	{
		int i_len = 0;
		i_len = snprintf( c_url , sizeof( c_url ) , "%s" , "http://ac.em.n0808.com/ac_box?en=gbk&ac=" ) ;
		
		if( c_url[0] )
		{
			char * p_url = c_url + i_len ; 
			if( strlen(keyword) > 0)
			{
				while( *keyword )
				{
					if( *keyword > 127 )
						p_url += snprintf( p_url,  sizeof( c_url ) - strlen( c_url ) ,"%%%02X", *keyword++ );
					else
					{
						snprintf( p_url,  sizeof( c_url ) - strlen( c_url ) ,"%s", keyword++ );
						p_url += 1;
					}
				}
			}
			else
			{
				http_sem_unlock() ; 
				return NO_SEARCH_WORD ;
			}
		}		
		http_get_type = PINYIN_SUGGEST_GET;
	}	
	else if( 13 == datasrc_id )//save bt torrent file
	{
		char * download_path = this_DownloadPath_str ;
		char * c_torrent = s_search_result->nodes[ item_id ].torrent ;
		char c_file_name[256] ;
		int task_id = 0 ;
		int ret = 0;
		int i_len = snprintf( c_file_name , sizeof( c_file_name ) , "%s" , download_path ) ;
		snprintf( &c_file_name[i_len] , sizeof( c_file_name )-i_len , "%s.%s" , s_search_result->nodes[item_id].name , "torrent" ) ;
		check_file_name( &c_file_name[i_len] ) ;
		
		if( c_torrent )
		{
                        if(download_path && download_path[0]/*&& FileNavDirExists(download_path)*/)
			{
				/*if(CurDLTorrentInfo.taskid != 0)
				{	
					task_id = NO_ENOUGH_SPACE;
					http_sem_unlock() ; 
					return task_id ;
				}
				*/
				//delete by zhenlei at 2010/01/25

				CurDLTorrentInfo.datasrc_id = datasrc_id;
				CurDLTorrentInfo.parent_id= parent_id;				
				CurDLTorrentInfo.item_id = item_id;
				snprintf(CurDLTorrentInfo.torrent, sizeof(CurDLTorrentInfo.torrent) , "%s" , c_torrent) ;
				snprintf(CurDLTorrentInfo.seed_path, sizeof(CurDLTorrentInfo.seed_path) , "%s" , c_file_name) ;

				struct stat torrent_stat;
				if(stat(c_file_name,&torrent_stat)==0)
				{
					CurDLTorrentInfo.taskid = 0;
					ret = TransferManager_GetSeedInfo(bttvapp, CurDLTorrentInfo.seed_path, CurDLTorrentInfo.torrent);
					if(ret != 0)
					{
						snprintf( s_error_msg , sizeof( s_error_msg ) , "GET TORRENT INFO ERROR" ) ;
						s_error_msg[127] = 0;
					}
					else
					{
						snprintf( s_error_msg , sizeof( s_error_msg ) , "GET TORRENT SUCCESS" ) ;
						s_error_msg[127] = 0;
					}	
					bttvapp_send_msg_net_datasrc_ready( cntl , BROADCAST_ALL , ret , s_error_msg , datasrc_id, item_id ,(INT32U)0 , 0 , 1) ;
					http_sem_unlock() ; 
					this_ErrorNo_Set( ret ) ;					
					return ret;
				}
				
				task_id = TransferManager_AddTask( bttvapp , c_torrent, c_file_name, NULL , 0 , NULL , 1 , XFER_TASK_HTTP , XFER_NOCACHE );
				if( task_id > 0 )
				{
					this_ErrorNo_Set( 0 ) ;
                                        s_getting_task = (HTTP_TASK*)AVMem_malloc( sizeof( HTTP_TASK ) ) ;
					if( !s_getting_task )
					{
						http_sem_unlock() ; 
						this_ErrorNo_Set( MALLOC_MEM_ERROR ) ;
						return MALLOC_MEM_ERROR ;
					}
					memset( s_getting_task , 0x00 , sizeof( HTTP_TASK ) );
					s_getting_task->task_id = task_id ;
					s_getting_task->get_type = TORRENT_GET;
					s_getting_task->datasrc_id = datasrc_id ;
					s_getting_task->parent_id = parent_id ;
					s_getting_task->item_id = item_id ;
					s_getting_task->p_url = get_value( c_torrent , strlen( c_torrent ) ) ;
					s_getting_task->task_state = HTTP_GETTING ;
					ret = task_id ;

					CurDLTorrentInfo.taskid = task_id;
				}
				else
				{
					int i_error = TransferManager_GetLastError( bttvapp ) ;
					this_ErrorNo_Set( i_error ) ;

					CurDLTorrentInfo.taskid = 0;
				}
			}
			else
			{
				task_id = DOWNLOAD_DISK_UNMOUNT ;
			}
		}
		else
		{
			task_id = DOWNLOAD_URL_NULL ;
		}
		http_sem_unlock() ; 
		return task_id ;
	}	
	else if( 14 == datasrc_id )//get server list
	{
		int i_len = 0;
		char *server_url = this_ServerURL_str;
		
		if(server_url && server_url[0])
		{
			i_len = snprintf( c_url , sizeof( c_url ) , "%s" , server_url);
			http_get_type = SERVER_GET;
		}
		else
		{
			http_sem_unlock() ; 
			return DOWNLOAD_URL_NULL ;
		}
	}	
	else if( 77 == datasrc_id ) //for kankan title
	{
		http_sem_unlock() ; 
		if(s_kankan && s_kankan[0])
			return (INT32U)s_kankan[0]->title;
		return (INT32U)NULL;
	}

	if( !c_url )
	{
		http_sem_unlock() ;	
		return ROOT_URL_NULL ;
	}
	
	task_id = TransferManager_AddTask( bttvapp , c_url , NULL , NULL , 0 , NULL , 1 , XFER_TASK_HTTP , XFER_BUFF | XFER_NOCACHE );
	if( task_id > 0 )
	{
		this_ErrorNo_Set( 0 ) ;
                s_getting_task = (HTTP_TASK*)AVMem_malloc( sizeof( HTTP_TASK ) ) ;
		if( !s_getting_task )
		{
			http_sem_unlock() ; 
			this_ErrorNo_Set( MALLOC_MEM_ERROR ) ;
			return MALLOC_MEM_ERROR ;
		}
		memset( s_getting_task , 0x00 , sizeof( HTTP_TASK ) );
		s_getting_task->task_id = task_id ;
		s_getting_task->get_type = http_get_type ;
		s_getting_task->datasrc_id = datasrc_id ;
		s_getting_task->parent_id = parent_id ;
		s_getting_task->item_id = item_id ;
		s_getting_task->p_url = get_value( c_url , strlen( c_url ) ) ;
		s_getting_task->task_state = HTTP_GETTING ;
		ret = task_id ;
	}
	else
	{
		int i_error = TransferManager_GetLastError( bttvapp ) ;
		this_ErrorNo_Set( i_error ) ;
	}
	http_sem_unlock() ;	
    /* end */
    return ret;
}

int BttvApp_RequestPicture(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    bttvapp_t* bttvapp = NULL ;// NULL ;// (bttvapp_t*)(cntl->private_data);
    int parent_id = (int)(param[0]);
    int item_id = (int)(param[1]);
    /* Add Method code here, Set ret to be -1 if fail */
	add_http_task( cntl , 0 , PIC_GET , 2 , parent_id , item_id , NULL , 1 ) ;
    /* end */
    return ret;
}

int BttvApp_CancelRequest(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(cntl->private_data);
    int task_id = (int)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */
    if(-1 == task_id  &&s_getting_task == NULL)
    {
        return ret;
    }
	http_sem_lock() ;	
	if( -1 == task_id )
    {
		release_all_http_task( 0 );
		if( PIC_GET == s_getting_task->get_type )
		{
			TransferManager_CancelTask( bttvapp,0,s_getting_task->task_id ) ;
			release_http_task( &s_getting_task );
		}
	}
	else
	{
		TransferManager_CancelTask( bttvapp,0,task_id ) ;
	}
	
	http_sem_unlock() ;	
    /* end */
    return ret;
}

int BttvApp_GetErrorMsg(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(cntl->private_data);
    char* ret_str = NULL;
    int ErrorNo = (int)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */

    /* end */
    return (int)ret_str;
}

 int BttvApp_SetDemoModel(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(cntl->private_data);
    int ModelType = (int)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */
	
    /* end */
    return ret;
}

int BttvApp_SetDownlodPath(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(cntl->private_data);
    char* DownloadPath = (char*)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */
	char* c_dlpath = this_DownloadPath_str;
        qDebug("%s ,%s",c_dlpath,DownloadPath);

        if(strcmp(DownloadPath, c_dlpath) != 0)
	{
                /*if(current_dldisk_mount(bttvapp , this_DownloadPath_str) > 0)
		{
			this_DownloadPath_str_Set(DownloadPath);
                }*/

                this_DownloadPath_str_Set(DownloadPath);
	}
    /* end */
    return ret;
}

 int BttvApp_CurDLPathSpace(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(cntl->private_data);
    int bFreeSpace = (int)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */
	int mount_disk_count = MountInfo_GetRowCount( bttvapp ) ;
	char* c_dlpath = this_DownloadPath_str;
	for( int i = 0 ; i < mount_disk_count ; i++ )
	{
                        /*char * c_disk = MountInfo_GetRowName( bttvapp , i ) ;
		if( strstr(c_dlpath, c_disk ) )
		{
			ret = (unsigned)(get_disk_freespace(c_dlpath)/1024.0);
			break ;	
                        }
                }*/
	}
    /* end */
    return ret;
}

 int BttvApp_SetSearchType(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    bttvapp_t* bttvapp = (bttvapp_t*)(cntl->private_data);
    char* SearchType = (char*)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */
	this_SearchType_str_Set(SearchType);
    /* end */
    return ret;
}


static void add_pic_http_task()
{	
	int task_id = -1 ;
	s_getting_task = get_http_task_head( 0 ) ;
	if( !s_getting_task )
		return ;
	int i_channel = s_getting_task->parent_id ;
	int i_item = s_getting_task->item_id ;
	char * p_url = NULL ;	
	if( s_current_pic_ctx && s_current_pic_ctx[ i_channel + 1 ] && s_current_pic_ctx[ i_channel + 1 ]->channel_count >= i_item && 
		NULL == (s_current_pic_ctx[i_channel+1]->nodes[i_item]).mem_pic )
		p_url = (s_current_pic_ctx[i_channel+1]->nodes[i_item]).pic ;
	else
	{
		release_http_task( &s_getting_task ) ;
		return ;
	}

	if( p_url )
	{
		s_getting_task->p_url = get_value( p_url , strlen( p_url ) ) ;
                bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)((s_getting_task->cntl)->private_data);
		task_id = TransferManager_AddTask( bttvapp , p_url , NULL , NULL , 0 , NULL , 1 , XFER_TASK_HTTP , XFER_BUFF | XFER_NOCACHE );
		if( task_id > 0 )
		{		
			this_ErrorNo_Set( 0 ) ;
			s_getting_task->task_id = task_id ;			
			s_getting_task->task_state = HTTP_GETTING ;
		}
		else
		{		
			int i_error = TransferManager_GetLastError( bttvapp ) ;
			this_ErrorNo_Set( i_error ) ;
			add_http_task( s_getting_task->cntl ,0 , PIC_GET , 2 , s_getting_task->parent_id , s_getting_task->item_id , NULL , 0 ) ;
			release_http_task( &s_getting_task ) ;
		}	
		return ;
	}
	else
		release_http_task( &s_getting_task ) ;
}

static void parse_server_get(HTTP_TASK * p_getting_task) 
{
#ifndef __ROM_
	printf( "\n\nget server list,error no: %d! \n" , s_getting_task->i_error ) ;
#endif

	if( NULL != s_server_list)
	{
		Server_ReleaseXmlCtx(s_server_list);
		s_server_list = NULL;
	}
	
	if( 0 == s_getting_task->i_error )
	{
		bttvapp_t* bttvapp = (bttvapp_t*)((s_getting_task->cntl)->private_data);
		s_getting_task->i_error = Server_ParseXmlCtx( ( char * )s_getting_task->p_result , &s_server_list) ;
		
		if(NULL == s_server_list || 0 == s_server_list->count)
			return;
	
	for(int i = 0; i < s_server_list->count; i++)
	{
		char *type = s_server_list->nodes[i].type;
		char *url = s_server_list->nodes[i].url;
		
		if(type && url && (0 == strcmp(type, "download_channel")))
			this_RootURL_str_Set(url);
		else if(type && url && (0 == strcmp(type, "download_search")))
			this_SearchURL_str_Set(url);
		else if(type && url && (0 == strcmp(type, "online_channel")))
			this_kankanNaviUrl_str_Set(url);
		else if(type && url && (0 == strcmp(type, "online_search")))
			this_kankanSearUrl_str_Set(url);
		else if(type && url && (0 == strcmp(type, "upgrade")))
                        ;//SetRefUpdateURL(url);
	}
		
	}
	else
	{				
		snprintf( s_error_msg , sizeof( s_error_msg ) , "get server list, error no: %d! \n" , s_getting_task->i_error ) ;
	}
	
	bttvapp_send_msg_net_datasrc_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 0) ;
}

static void parse_vod_get( HTTP_TASK * p_getting_task )
{
	if( NULL != s_kankan_programe )
	{
		KanKan_VodRelease( &s_kankan_programe );
	}
	
	if( 0 == s_getting_task->i_error )
	{
		s_getting_task->i_error = KanKan_ParseVodInfo( ( char * )s_getting_task->p_result , &s_kankan_programe,bIPTV_kan,bCopyright_kan);
	}
	
	if( 0 == s_getting_task->i_error )
	{
		s_error_msg[0] = '\0' ;
	}
	else if( MAX_GET_TIMES > s_getting_task->i_get_times )
	{
		snprintf( s_error_msg , sizeof( s_error_msg ) , "get vod info error %d !",  s_getting_task->i_error) ;
                        bttvapp_send_msg_net_vob_ready( s_getting_task->cntl , BROADCAST_ALL , s_getting_task->i_error , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 1 ) ;
		return ;
	}
	else
	{
#ifndef __ROM_
		printf( "\nparse kankan vod xml error , error no: %d! \n" , s_getting_task->i_error ) ;
#endif				
		snprintf( s_error_msg , sizeof( s_error_msg ) , "parse kankan vod error ! %d\n" , s_getting_task->i_error ) ;
		s_error_msg[127] = 0;
	}
	if( 0 == s_getting_task->i_error )
	{
		if(/* CH_MOVIE == s_kankan_programe->programe_type &&*/ 1 == s_kankan_programe->part_count )
		{
			if( strstr( s_kankan_programe->p_vod[0].format ,"rm"  ) || strstr( s_kankan_programe->p_vod[0].format ,"RM"  ) )
			{
				char c_file_name[256] ;
				int task_id = 0 ;
                                                bttvapp_t* bttvapp = NULL ;// (bttvapp_t*)(s_getting_task->cntl->private_data);
				char * download_path = this_DownloadPath_str ;
				snprintf( c_file_name , sizeof( c_file_name ) , "%s.%s" , s_kankan_programe->p_vod[0].name , s_kankan_programe->p_vod[0].format ) ;
				check_file_name( c_file_name ) ;
				unlink( c_file_name ) ;
				TransferManager_PauseAllTask( bttvapp ) ;
				task_id = TransferManager_AddThunderTask( bttvapp ,s_kankan_programe->p_vod[0].cid , s_kankan_programe->p_vod[0].g_cid , download_path , (INT32U)s_kankan_programe->p_vod[0].size , 7 , c_file_name);
				/*
				#ifndef __ROM_
				printf( "\nadd kankan task %s task id %d !\n" , c_file_name , task_id ) ;
				#endif
				*/
				if(task_id > 0)
				{
					this_ErrorNo_Set( 0 ) ;
					//printf( "\n     add kankan task task id %d !  \n" ,  task_id ) ;
					//AVTimeDly(400);
					//TransferManager_StartPrioTask( bttvapp , task_id ) ;
					s_kankan_programe->cur_download_part = 0 ;
					s_kankan_programe->cur_download_blok = 0 ;
					snprintf( s_error_msg , sizeof( s_error_msg ) , "%d" , s_kankan_programe->vod_count ) ;
                                                            bttvapp_send_msg_net_vob_ready( s_getting_task->cntl , BROADCAST_ALL , 0 , s_error_msg , s_getting_task->datasrc_id , /*s_getting_task->item_id*/s_kankan_programe->cur_download_blok ,(INT32U)task_id , s_kankan_programe->part_count, 1) ;
				}
				else
				{
					TransferManager_RestartAllTask( bttvapp ) ;				
					int i_error = TransferManager_GetLastError( bttvapp ) ;
					this_ErrorNo_Set( i_error ) ;
					snprintf( s_error_msg , sizeof( s_error_msg ) , "add kankan task failed, error id %d !" , i_error ) ;
                                                            bttvapp_send_msg_net_vob_ready( s_getting_task->cntl , BROADCAST_ALL , task_id , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 1 ) ;
				}	
			}
			else
			{
				snprintf( s_error_msg , sizeof( s_error_msg ) , "no support the format programme !" ) ;
				s_error_msg[127] = 0;
				int i_value = NO_SUPPORT_THE_FORMAT_PROGRAMME ;
                                                bttvapp_send_msg_net_vob_ready( s_getting_task->cntl , BROADCAST_ALL , i_value , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id ,(INT32U)0 , 0 , 1 ) ;
			}	
		}
		else
		{
                                    bttvapp_send_msg_net_vob_ready( s_getting_task->cntl , BROADCAST_ALL , 0 , s_error_msg , s_getting_task->datasrc_id , s_getting_task->item_id , (INT32U)s_kankan_programe , s_kankan_programe->part_count , 0 ) ;
		}
	}
	else
	{
	}
}

static int current_dldisk_mount( bttvapp_t* bttvapp , char * c_download_path )
{
	int i_ret = 0 ;
	int mount_disk_count = MountInfo_GetRowCount( bttvapp ) ;
	
	for( int i = 0 ; i < mount_disk_count ; i++ )
	{
                        /*char * c_disk = MountInfo_GetRowName( bttvapp , i ) ;
		if( strstr( c_download_path , c_disk ) )
		{
			i_ret = 1 ;
			break ;	
                        }	*/
	}
        return 1 ;
}


MBttvObject::MBttvObject(QObject *parent) :
    QObject(parent)
{
}
MBttvObject* MBttvObject::Instance(QObject *parent)
{
    return &mbttvObject;
}
void MBttvObject::mbInit()
{
    //
}

void MBttvObject::mbUnInit()
{
    //
}

void MBttvObject::mbSerialNumCheck()
{
    //
}

void MBttvObject::mbRequestData()
{
    //
}

void MBttvObject::mbRequestPic()
{
    //
}

void MBttvObject::mbCancelRequest()
{
    //
}

void MBttvObject::mbSetDownloadPath()
{
    //
}

void MBttvObject::mbCurDLPathSpace()
{
    //
}

void MBttvObject::RequestDataReady(INT32U ret_value, char* err_msg, INT32U datasrc_id, INT32U parent_id, INT32U rawdata, INT32U count, INT32U stage)
{
    emit requestDataGot(ret_value, err_msg, datasrc_id, parent_id, rawdata, count, stage);
}

void MBttvObject::RequestPicReady(INT32U ret_value, char* err_msg, INT32U parent_id, INT32U item_id, INT32U image_data, INT32U data_size, char* image_type, INT32U stage)
{
    int edf = 12;
    emit requestPicGot(ret_value, err_msg, parent_id, item_id, image_data, data_size, image_type,stage);
}
void MBttvObject::RequestUpdataTask(INT32U addr,int count )
{

    emit updatataskinfo(addr,count);
}
 void  MBttvObject::SendTaskListInfo()
 {
     TransferManager_UpdateTaskInfo(NULL);
 }

 
void  MBttvObject::LoadAllTask( char* path)
{
      TransferManager_LoadAllTasks(NULL, path );
}

void  MBttvObject::PauseTask(INT32U taskid )
{
      TransferManager_PauseTask(0, taskid );
}

void  MBttvObject::CancelTask(INT32U isCallback ,INT32U taskid )
{
      TransferManager_CancelTask(0,  isCallback , taskid );
}


void  MBttvObject::CloseTask(INT32U taskid )
{
      TransferManager_CloseTask(0, taskid );
}

void  MBttvObject::StartTask(INT32U taskid )
{
      TransferManager_StartTask(0, taskid );
}

void  MBttvObject::SetLicence(char *data)
{
/**********************************************************/
    if(data == 0)
        return;

    memcpy(Licence,data,strlen(data));
}


void  MBttvObject::InitTaskInfo()
{
/**********************************************************/
     char *DownloadPath = "/download/";
     g_DownloadPath = (char*)malloc(sizeof(char*));
     this_DownloadPath_str_Set(DownloadPath);

      create_parse_xml_task();
}

extern "C" int BttvApp_SerialNumberCheck(control_t* cntl, cond_item_t* param)
{
    return 5;
}
