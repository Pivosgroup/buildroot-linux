/*****************************************************************
**                                                          									**
**  Copyright (C) 2008 Amlogic,Inc.                             						**
**  All rights reserved                                         							**
**        Filename : search_comm.h /Project:AVOS          						** 
**        Revision : 1.0                                        								**
**                                                              									**
*****************************************************************/


#ifndef _MOAP_COMM_H_
#define _MOAP_COMM_H_

//search type define
typedef enum
{
	MOVIE_NAME_TYPE = 0,
	DIRECROT_NAME_TYPE = 1,
	SONG_NAME_TYPE=2,
	SONGER_NAME_TYPE=3,
	OTHER_TYPE=9,	
}Search_type_menu ;


//search content type define
typedef enum
{
	ALL_CONT = 0,
	MOVIE_CONT = 1 ,
	MUSI_CCONT=2,
	PICTURE_CONT=3,
	OTHER_CONTENT=9,
}Search_conternt_menu ;

//the download protocol .
typedef enum
{
	FTP_DL_PROTOCOL = 0 ,
	HTTP_DL_PROTOCOL = 1 ,
	BT_DL_PROTOCOL = 2 ,
	EMULE_DL_PROTOCOL =  3,
	THUNDER_DL_PROTOCOL = 4,
	THUNDER_KANKAN_PROTOCOL = 5 ,
}download_protocol_menu ;

//the type of the navigate and search.
typedef enum
{
	AMLOGIC_MOAP,
	CHANGHONG_MOAP,
	NET_MOVIE_MOAP,
	HAIER_MOAP,
	AOC_MOAP ,
	HA_TH_MOAP,
}moap_type_menu ;

typedef enum 
{
	CH_NONE = 0 ,
	CH_TYPE = 1 ,
	CH_MOVIE = 2 ,
	CH_TELEPLAY = 3 ,
	CH_CARTON = 4 ,
	CH_TV_SHOW = 5 ,
}CHANNEL_TYPE ;

typedef struct _MEM_PIC_INFO
{
	char * pic_buff ;
	int    pic_format ;
	int    pic_buff_len ;
	int    i_channel ;
	int    i_item ;
	struct _MEM_PIC_INFO * p_next_pic ;
}MEM_PIC_INFO ;

typedef struct _VODFiles
{
	int sub_id ;
	int be_new_part ;
	char * cid ;
	char * g_cid ;
	int size ;
	char * name ;
	char * format ;
	//int play_time ;
	char * play_time;
}VODFiles;

typedef struct _KANKAN_PROGRAME
{
	int part_count ;
	int vod_count ;
	int cur_download_part ;
	int cur_download_blok ;
	int cur_play_blok ;
	CHANNEL_TYPE programe_type ;
	VODFiles * p_vod ;
}KANKAN_PROGRAME ;


typedef struct{
	int id;					//the id of the movie.
	int is_channel;			//whether the channel  1: yes ; 0:no
	int iMovie_drm ;		//the movie file local file drm .
	float size;             //the movie file size .
	char * c_size ;			//the movie file size of string
	int len;				//the duration of the movie  minute.
	int quality;			//the movie quality.
	int connect;			//the count of the connect .by BT
	int iDownloadProtocol ;	//the download protocol of the movie
	char *format;			//the file format of the movie .example rmvb , rm ...
	char* name;				//the movie name.
	char* mv_info;			//the director and the actors describe.
	char* desc;				//the describe of the movie
	int   iPic_encry ;		//the pic url encry type
	char* pic;				//the url of the log picture
	MEM_PIC_INFO * mem_pic ;//the movie log picture of memory type.
	int   iTorr_encry ;  	//the torrent url encry type 
	char* cid;			//the cid	
	char* torrent;			//the torrent url
	int   iXml_encry ;   	//the xml url encry type 
	char* xml;				//the xml url of the next level navigate info .
	KANKAN_PROGRAME* kankan_vod ; //the kankan program 
}dl_rcmd_xml_movie_node_type;

typedef struct {
	int parent_id;							//the parent node id;
	char *c_parent_id;
	char *title;							// the title of the node.
	int iSelected_item ;						//current selected intem of the page.
	int nodes_count;						// the current page nodes count .
	int begin_pos ;							//the node begin postion
	int channel_count ;						//current channel node count.
	dl_rcmd_xml_movie_node_type *nodes;	// the point of the nodes.
}dl_rcmd_xml_ctx_type;

typedef struct {
    char *type;
    char *url;
} server_node_type;

typedef struct {
    int count;
    server_node_type *nodes;
} server_xml_type;

/* the call back of Download_request 
@param iResult         [in]       the result of download  .
@param iTaskId        [in]       the request return task id .
@param cpDesc        [in]       download finished return describe .
@pNaviNodeInfo	 [in] 	   the request return result info.
*/
typedef void( * moap_Callback )( int iResult , int iTaskId , const char * cpDesc , dl_rcmd_xml_ctx_type * pNaviNodeInfo ) ;


typedef struct{
	char * c_navi_server ; 			//the navigate server url
	char * c_search_server ;			//the search server url
}MOAP_CFG_INFO ;


#endif

