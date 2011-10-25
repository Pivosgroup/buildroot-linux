#ifndef SD_EMBEDTHUNDER_H_00138F8F2E70_200809081558
#define SD_EMBEDTHUNDER_H_00138F8F2E70_200809081558
/*--------------------------------------------------------------------------*/
/*                               IDENTIFICATION                             */
/*--------------------------------------------------------------------------*/
/*     Filename  : embed_thunder.h                                         */
/*     Author     : Li Feng                                              */
/*     Project    : EmbedThunder                                        */
/*     Version    : 1.3  													*/
/*--------------------------------------------------------------------------*/
/*                  Shenzhen XunLei Networks			                    */
/*--------------------------------------------------------------------------*/
/*                  - C (copyright) - www.xunlei.com -		     		    */
/*                                                                          */
/*      This document is the proprietary of XunLei                          */
/*                                                                          */
/*      All rights reserved. Integral or partial reproduction               */
/*      prohibited without a written authorization from the                 */
/*      permanent of the author rights.                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*                         FUNCTIONAL DESCRIPTION                           */
/*--------------------------------------------------------------------------*/
/* This file contains the interfaces of EmbedThunder                         */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                              HISTORY                                     */
/*--------------------------------------------------------------------------*/
/*   Date     |    Author   | Modification                                  */
/*--------------------------------------------------------------------------*/
/* 2008.09.08 | Li Feng  | Creation                                      */
/*--------------------------------------------------------------------------*/
/* 2009.01.19 | ZengYuqing  | Update to version 1.2                                     */
/*--------------------------------------------------------------------------*/
/* 2009.04.13 | ZengYuqing  | Update to version 1.3    

1.et1.3版本在et1.2版本的基础上增加了纯BT协议的下载和vod点播功能。
2.在结构体ET_TASK中的任务状态_task_status中增加了ET_TASK_VOD(注意该状态对应的值为2，而原来的状态ET_TASK_SUCCESS变为3，ET_TASK_FAILED变为4，ET_TASK_STOPPED变为5)，该状态下任务处于数据下载完毕，但是正在播放状态。在该状态下，用户可以调用et_stop_task强制停止任务，但不可以直接调用et_delete_task。
3.在结构体ET_TASK中，增加元素_valid_data_speed，该参数用于显示点播时实际的数据下载速度，用于调试用。注意，该参数在编译宏开关_CONNECT_DETAIL的控制之下。
4.在结构体ET_PEER_PIPE_INFO中修改元素_peerid的长度为21.
5.在结构体ET_BT_FILE中_file_status参数中，去除原来的“文件未完成”状态。
6.在注册license时，返回到错误码中增加了返回值21005，表示服务器繁忙，下载库会在一小时后自动重试。
7.增加用于得到下载库内当前所有的task的id的接口函数et_get_all_task_id。
8.增加用于得到task_id标识的BT任务的所有需要下载的文件的id的接口函数et_get_bt_download_file_index。
9.增加用于启动和关闭VOD点播用的http服务器的接口函数：et_start_http_server，et_stop_http_server。
10.增加VOD时用于读取任务数据的接口函数et_vod_read_file。
11.增加用于用户自定义设置socket相关参数的函数类型#define ET_SOCKET_IDX_SET_SOCKOPT (11)。
12.增加用迅雷看看VOD专用URL启动新任务和续传任务的功能，详细说明请看et_create_new_task_by_url的接口说明。
13.接口et_create_new_bt_task已被接口et_create_bt_task取代，虽然et_create_new_bt_task还可以用，但建议直接用et_create_bt_task。
14.现在下bt任务时，如果存在需要下载的跨文件的piece的话，会在下载目录那里生成info_hash.tmp文件和info_hash.tmp.cfg,在整个任务完成后这两个文件会被删掉，如果任务没有完成的话，是不会删除的。
15.在结构体ET_BT_FILE中，去除原来的参数：char *_file_name_str;u32 _file_name_len;BOOL _is_need_download;增加用于显示p2sp加速状况的参数：BOOL _has_record;u32 _accelerate_state。
16.增加获取BT任务文件路径和文件名的接口函数：et_get_bt_file_path_and_name。
17.增加用于用户自定义申请和释放内存的函数类型#define ET_MEM_IDX_GET_MEM (12) 和#define ET_MEM_IDX_FREE_MEM (13) 。
18.在结构体ET_TASK中，增加元素_ul_speed，该参数用于显示任务的上传速度。
19.在结构体ET_TASK中，增加元素_bt_dl_speed和_bt_ul_speed，这两个参数分别用于显示BT任务的纯BT协议的上传和下载速度，用于调试用。注意，该参数在编译宏开关_CONNECT_DETAIL的控制之下，且只对BT任务有效。
20.重要提示：编译宏开关_CONNECT_DETAIL控制之下的所有结构体和参数都只供迅雷公司内部调试用，暂时没有开放给各合作伙伴，因此请各合作伙伴不要在界面程序中定义编译宏开关_CONNECT_DETAIL，更不要尝试引用相关的结构体和参数，由此带来的不便，请见谅！	[2009.05.13]
21.将et_create_new_task_by_url接口中的参数char* file_name_for_user 改为 char* file_name。	[2009.05.14]
22.将et_create_continue_task_by_url接口中的参数char* cur_file_name 改为 char* file_name。	[2009.05.14]
23.增加接口et_create_task_by_tcid_file_size_gcid。	[2009.05.14]
24.et_create_continue_task_by_url和et_create_continue_task_by_tcid增加返回错误码4199，表示对应的.cfg文件不存在。	[2009.06.08]
25.增加接口et_get_current_upload_speed获得底层上传速度。	[2009.06.29]
26.增加接口et_set_download_record_file_path 用于设置下载记录文件的路径。	[2009.07.01]
27.增加接口et_set_task_no_disk 用于将vod任务设置为无盘下载模式。	[2009.07.22]
28.增加接口et_get_upload_pipe_info 用于读取上传pipe的信息(仅供迅雷内部调试用)。	[2009.07.23]
29.增加对接口et_set_license_callback 中关于回调函数中不能调用任何下载库的接口的重要说明。	[2009.07.27]
30.增加对接口et_create_task_by_tcid_file_size_gcid 中参数file_size切不可以等于0的重要更正。	[2009.07.27]
31.增加对接口et_set_limit_speed 中参数不可以等于0的说明。	[2009.08.27]
32.增加对接口et_set_max_task_connection 中参数取值范围的说明。	[2009.08.27]
33.增加设置缓冲时间接口et_vod_set_buffer_time [2009.08.27]
34.增加获取缓冲百分比接口et_vod_get_buffer_percent [2009.08.27]
35.增加对接口et_set_limit_speed 的注意说明。				[2009.09.01]
36.增加对接口et_get_current_download_speed 的注意说明。	[2009.09.01]
37.增加对接口et_get_current_upload_speed的注意说明。		[2009.09.01]
38.去除ET_PEER_PIPE_INFO结构体中元素_upload_speed的UPLOAD_ENABLE宏控制	[2009.09.01]
39.增加对接口et_get_bt_file_info的错误码说明:1174  文件信息正在更新中，暂时不可读，请稍候再试!	[2009.09.03]
40.更正用户自定义接口中typedef int32 (*et_fs_open)(char *filepath, int32 flag, u32 *file_id);的file_id为u64的重大说明错误。[2009.11.19]
41.增加对接口et_set_license的错误码说明:1925 	获取网卡MAC 出错!	[2009.11.23]
42.增加与vod 点播专用内存有关的四个接口:et_vod_get_vod_buffer_size,et_vod_set_vod_buffer_size,et_vod_is_vod_buffer_allocated,et_vod_free_vod_buffer [2009.12.12]
43.增加用于查询vod 点播任务的数据是否下载完毕的接口:et_vod_is_download_finished	 		[2009.12.12]
*/

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef WIN32
#ifndef WINCE
	#ifdef ETDLL_EXPORTS
	#define ETDLL_API __declspec(dllexport)
	#else
	#define ETDLL_API __declspec(dllimport)
	#endif
#else
	#define ETDLL_API
#endif
#else
	#define ETDLL_API
#endif

/************************************************************************/
/*                    TYPE DEFINE                                       */
/************************************************************************/
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef char				int8;
typedef short				int16;
typedef int				int32;
#if defined(LINUX)
	typedef unsigned long long	uint64;
	typedef long long			int64;
#else
	#ifdef  NOT_SUPPORT_LARGE_INT_64
		typedef unsigned int	uint64;
		typedef int			__int64;
	#else
        #if defined(WIN32)
		typedef unsigned __int64 uint64;
		//typedef long long			__int64;
         #else
		typedef unsigned long	long uint64;
		typedef long long			__int64;
         #endif
	#endif
#endif

#if defined(LINUX)
#ifndef NULL
#define NULL	((void*)(0))
#endif
#endif

#ifndef TRUE
typedef int32 BOOL;
#define TRUE	(1)
#define FALSE	(0)
#endif


/************************************************************************/
/*                    STRUCT DEFINE                                     */
/************************************************************************/

enum ET_TASK_STATUS {ET_TASK_IDLE = 0, ET_TASK_RUNNING, ET_TASK_VOD,  ET_TASK_SUCCESS, ET_TASK_FAILED, ET_TASK_STOPPED};
enum ET_TASK_FILE_CREATE_STATUS {ET_FILE_NOT_CREATED = 0, ET_FILE_CREATED_SUCCESS,  ET_FILE_CREATED_FAILED};
enum ET_ENCODING_SWITCH_MODE 
{ 
	ET_ENCODING_PROTO_MODE = 0, /* 返回原始字段 */
	ET_ENCODING_GBK_SWITCH = 1,/*  返回GBK格式编码 */
	ET_ENCODING_UTF8_SWITCH = 2,/* 返回UTF-8格式编码 */
	ET_ENCODING_BIG5_SWITCH = 3,/* 返回BIG5格式编码  */
	
	ET_ENCODING_UTF8_PROTO_MODE = 4,/* 返回种子文件中的utf-8字段  */
	ET_ENCODING_DEFAULT_SWITCH = 5/* 未设置输出格式(使用et_set_seed_switch_type的全局输出设置)  */
};


#ifdef _CONNECT_DETAIL
/* 迅雷公司内部调试用  */
typedef struct t_et_peer_pipe_info
{
	u32	_connect_type;
	char	_internal_ip[24];
	char	_external_ip[24];
	u16	_internal_tcp_port;
	u16	_internal_udp_port;
	u16	_external_port;
	char	_peerid[21];
    u32    _speed;
	
    u32    _upload_speed;

    u32    _score;
	
	/* pipe状态 
	0 空闲
	1 连接
	2 连接成功
	3 被choke
	4 开始下载数据
	5 发生错误
	6 下载成功(变成纯上传)
	*/
	u32    _pipe_state;
} ET_PEER_PIPE_INFO;

typedef struct t_et_peer_pipe_info_array
{
	 ET_PEER_PIPE_INFO _pipe_info_list[ 10 ];
    u32 _pipe_info_num;
} ET_PEER_PIPE_INFO_ARRAY;


#endif /*  _CONNECT_DETAIL  */


typedef struct t_et_download_task_info
{
	u32  _task_id;
	u32 _speed;    /*任务的下载速度*/
	u32 _server_speed;   /*任务server 资源的下载速度*/  
	u32 _peer_speed;   /*任务peer 资源的下载速度*/  
	u32 _ul_speed;    /*任务的上传速度*/
	u32 _progress;  /*任务进度*/  
	u32 _dowing_server_pipe_num; /*任务server 连接数*/  
	u32 _connecting_server_pipe_num;  /*任务server 正在连接数*/  
	u32 _dowing_peer_pipe_num;  /*任务peer 连接数*/  
	u32 _connecting_peer_pipe_num; /*任务pipe 正在连接数*/  

#ifdef _CONNECT_DETAIL
/* 迅雷公司内部调试用  */
       u32  _valid_data_speed;
	u32 _bt_dl_speed;   /*任务BT 资源的下载速度（只对BT任务有效）*/  
	u32 _bt_ul_speed;   /*任务BT 资源的上传速度（只对BT任务有效）*/  
	u32 _downloading_tcp_peer_pipe_num;
	u32 _downloading_udp_peer_pipe_num;
    
	u32 _downloading_tcp_peer_pipe_speed;
	u32 _downloading_udp_peer_pipe_speed;


	/* Server resource information */
	u32 _idle_server_res_num;
	u32 _using_server_res_num;
	u32 _candidate_server_res_num;
	u32 _retry_server_res_num;
	u32 _discard_server_res_num;
	u32 _destroy_server_res_num;

	/* Peer resource information */
	u32 _idle_peer_res_num;
	u32 _using_peer_res_num;

	u32 _candidate_peer_res_num;
	u32 _retry_peer_res_num;
	u32 _discard_peer_res_num;
	u32 _destroy_peer_res_num;
	u32 _cm_level;
    ET_PEER_PIPE_INFO_ARRAY _peer_pipe_info_array;    
#endif /*  _CONNECT_DETAIL  */
	uint64 _file_size;  /*任务文件大小*/  

      /*任务状态: 	ET_TASK_IDLE  		空闲
                                	ET_TASK_RUNNING 	正在运行
                                 	ET_TASK_VOD 		数据已下载完成，正在播放中
                                 	ET_TASK_SUCCESS 	任务成功
                                 	ET_TASK_FAILED 		任务失败，失败码为failed_code
                                 	ET_TASK_STOPPED 	任务已停止
	 */  
	enum ET_TASK_STATUS  _task_status;   

	 /*任务失败原因
              102  无法纠错
              103  无法获取cid
              104  无法获取gcid
              105  cid 校验错误
              106  gcid校验错误
              107  创建文件失败
              108  写文件失败
              109  读文件失败
              112  空间不足无法创建文件
              113 校验cid时读文件错误 
              130  无资源下载失败

              15400 子文件下载失败(bt任务)
              
         */  	  
	u32  _failed_code;
	 
	/* Time information  */
	/* 注意：1.这两个时间是获得自1970年1月1日开始的秒数
		 		2.考虑到在获取任务开始时间（_start_time）和完成时间（_finished_time）之间，系统的时间有可能被恶意更改而导致开始时间大于完成时间，
		 		或者完成时间远远大于开始时间（如大于1个月），这种情况下，用这两个时间直接作相减运算没有任何意义。
		 		因此，不能用这两个时间计算任务所用的时间或者计算平均速度
		 		3.对于续传任务，开始时间（_start_time）记录的是续传这个任务的开始时间   */
	u32 _start_time;	
	u32 _finished_time;

	enum ET_TASK_FILE_CREATE_STATUS _file_create_status;
	uint64 _downloaded_data_size; 		 /*已下载的数据大小*/  
	uint64 _written_data_size;  /*已写入磁盘的数据大小*/  
}ET_TASK;


typedef struct  t_et_bt_file
{
	u32 _file_index;
	uint64 _file_size;	
	uint64 _downloaded_data_size; 		 /*已下载的数据大小*/  
	uint64 _written_data_size; 			 /*已写进磁盘的数据大小*/  
	u32 _file_percent;/* 文件下载进度    */	

	/*文件状态:(改动说明:原来有 2:文件未完成 这个状态,现在去除)
		0:文件未处理
		1:文件正在下载
		2:文件下载完成
		3:文件下载失败 
	*/
	u32 _file_status;


	/*服务器查询状态:
		0:未查询，
		1:正在查询, 
		2:查询成功，
		3:查询失败 
	*/
	u32 _query_result;

	
	/*子任务失败原因错误码
		 15383  无法纠错
		 15393  cid 校验错误
		 15386  gcid校验错误
		 15394  创建文件失败
		 15395  数据下载成功,等待piece校验(临时状态,piece校验成功后子文件成功)
		 108  写文件失败
		 109  读文件失败
		 112  空间不足无法创建文件
		 113 校验cid时读文件错误
		 130  无资源下载失败
		 131  查询资源失败

		 15389 子文件速度太慢
	*/	   
	u32 _sub_task_err_code;

	BOOL _has_record; /* 为1 表示xunlei资源库中有该文件的记录，可以通过p2sp网络进行加速 */
	
	/*通过p2sp网络进行加速的状态:
		0:未加速，
		1:正在加速, 
		2:加速成功，
		3:加速失败，
		4:加速完成。意思是指不管加速成功或失败，总之这个文件已经被加速过，不会再被加速	*/
	u32 _accelerate_state; 
	
}ET_BT_FILE;



typedef struct  t_et_proxy_info
{
	char _server[32];
	char _user[32];
	char _password[32];

	u32 _port;

	int32 _proxy_type; /* 0 direct connect type, 1 socks5 proxy type, 	2 http_proxy type, 3 ftp_proxy type */
	int32 _server_len;
	int32 _user_len;
	int32 _password_len;
} ET_PROXY_INFO;

/* Structures for bt downloading */

#define ET_MAX_TD_CFG_SUFFIX_LEN (8)

typedef struct t_et_torrent_file_info
{
	u32 _file_index;
	char *_file_name;
	u32 _file_name_len;
	char *_file_path;
	u32 _file_path_len;
	uint64 _file_offset;
	uint64 _file_size;
} ET_TORRENT_FILE_INFO;

typedef struct t_et_torrent_seed_info
{
	char _title_name[256-ET_MAX_TD_CFG_SUFFIX_LEN];
	u32 _title_name_len;
	uint64 _file_total_size;
	u32 _file_num;
	u32 _encoding;//种子文件编码格式，GBK = 0, UTF_8 = 1, BIG5 = 2
	unsigned char _info_hash[20];
	ET_TORRENT_FILE_INFO *file_info_array_ptr;
} ET_TORRENT_SEED_INFO;


/*--------------------------------------------------------------------------*/
/*                Interfaces provid by EmbedThunder download library				        */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                下载库的初始化和反初始化接口				        */
/*--------------------------------------------------------------------------*/
/*
  * 初始化下载库 
  * 返回值: 0    成功
                      1025  初始分配内存失败
                      3672 下载库已经被初始化过了
                      4112 非法参数
                      其它初始化下载库线程失败错误码 
  * 如果连接hub不需要代理，proxy_for_hub设置成NULL或proxy_type为0
  *
  * 注意：迅雷下载库的所有其他所有接口都必须在这个函数之后才能调用，否则会出错（返回-1）！
  */
ETDLL_API int32 et_init(ET_PROXY_INFO* proxy_for_hub);

/*
  * 反初始化下载库 
  * 返回值: 0    成功       
  * 
  * 注意：1.迅雷下载库的所有其他所有接口（除了et_init）都不能在这个函数之后调用，否则会出错（返回-1）！
 */
ETDLL_API int32 et_uninit(void);

/* Check the status of et */
ETDLL_API int32 et_check_critical_error(void);


/*--------------------------------------------------------------------------*/
/*              license相关接口			        
----------------------------------------------------------------------------*/
/* 设置此客户端的 license 
  * 返回值: 0    成功       
  			   1925 	获取网卡MAC 出错
  			   
*  注意：请务必在下载库初始化之后启动下载任务（et_start_task）之前调用此函数
* 以下是用于测试的license:
                               License                               |序号 |有效期                         |状态(服务器返回结果)
08083000021b29b27e57604f68bece019489747910|0000000|2008-08-13┄2009-08-06|00

*/
ETDLL_API  int32 	et_set_license(char *license, int32 license_size);

/* 设置license上报后服务器返回结果的的回调通知函数 
* 特别注意:在这个回调函数里面,代码应该尽量简洁，而且切不可以在这个函数里面调用任何下载库的接口!因为这样会导致下载库死锁!谨记!!! 
*
* 下载库会每隔一段时间（该间隔由服务器返回，一般为一小时）向license服务器
* 上报本地的license（由et_set_license设置），服务器会返回该license的状态(结果)
*  和过期时间.
*
*  注意：在下载库被初始化(调用et_init)后，界面程序调用et_set_license函数下载库
*  即向license服务器上报所设置的license，若界面程序在初始化下载库后没有调
*  用et_set_license去设置license，下载库也会在五分钟之后向服务器上报，而
*  这种情况下服务器将返回结果为21004(license是虚假的)，使下载库不能继续下载新任务!
*
*  另外，由于服务器的响应可能很快，所以请在设置license(调用et_set_license)后尽快
*  调用et_set_license_callback设置回调函数，否则下载库将无法把服务器返回的结果通
*  知给界面程序，一个比较保险的方法就是在界面程序中把调用et_set_license_callback
*  放在调用et_set_license之前
*
*  第一个参数表示上报返回的结果 result:
*    4096   表示通讯错误，原因可能是网络问题或服务器坏掉了,若另一个参数expire_time不为0，则为可能的网络错误码。
*    4118   表示license上报失败，原因同4096，而且所有的下载通道将被关闭，此后新建的任务将不可下载。
*    21000 表示 LICENSE 验证未通过，原因是 LICENSE 被冻结。
*    21001 表示 LICENSE 验证未通过，原因是 LICENSE 过期。
*    21002 表示 LICENSE 验证未通过，原因是 LICENSE 被回收。
*    21003 表示 LICENSE 验证未通协，原因是 LICENSE 被重复使用。
*    21004 表示 LICENSE 验证未通协，原因是 LICENSE 是虚假的。
*    21005 表示 服务器繁忙，下载库会在一小时后自动重试。
*    
*  第二个参数表示多长时间后过期 expireTime（单位为秒）.
*/
typedef int32 ( *ET_NOTIFY_LICENSE_RESULT)(u32 result, u32 expire_time);
ETDLL_API  int32 	et_set_license_callback( ET_NOTIFY_LICENSE_RESULT license_callback_ptr);


/*--------------------------------------------------------------------------*/
/*              下载库全局参数配置相关接口			        
----------------------------------------------------------------------------*/
/*
  * 获取下载库版本号, 返回值是版本号的字符串
  *                   
  */
ETDLL_API const char* et_get_version(void);

/*
  * 设置下载记录文件的路径，这个文件用于记录下载库所有成功下载过并且还没有被删除掉的文件的信息。
  *	注意：1.路径不可为空，长度不能为0且长度不超过255字节，要以'/'符号结束，可以是绝对路径，也可以是相对路径。
  *   			   2.该文件夹一定要存在并且是可写的，因为下载库需要在此文件夹创建文件。
  *                    3.这个函数必须在下载库被初始化后，创建任务之前调用。
  */
ETDLL_API  int32 et_set_download_record_file_path(const char *path,u32  path_len);


/*
  * 设置最大任务数,最少1个，最多16个，否则，返回4112错误
  *                   
  */
ETDLL_API int32 et_set_max_tasks(u32 task_num);

/*
  * 获取最大任务数
  *  
  * 注意：若返回-1，表示下载库还没有被初始化！
  */
ETDLL_API u32 et_get_max_tasks(void);


/*
  * 设置最大下载速度和上传速度,以KB为单位,-1表示不限速
  *                   
  * 注意：1.download_limit_speed和upload_limit_speed为下载库所有接收到和发出去的数据量，包括了与外面服务器的各种通信指令和要下载的文件内容，
  *			因此，这两个值均不能设得太小，因为upload_limit_speed设置得太小会严重影响下载速度，得不偿失!
  *			2.这两个值更不能设为0，否则返回4112 错误！
  *			3.如果没有特殊要求，建议不要调用这个接口，因为下载库会根据资源状况自行选择合适的速度。
  *			4.如果下载库是用root帐户运行的话，下载库还会根据网络状况自行调节下载和上传速度，达到智能限速，
  *			这样就会在保持合理的下载速度的同时又不影响同一局域网内其他电脑的网络速度。
 */
ETDLL_API int32 et_set_limit_speed(u32 download_limit_speed, u32 upload_limit_speed);

/*
  * 获取最大速度,以KB为单位
  *                   
  */
ETDLL_API int32 et_get_limit_speed(u32* download_limit_speed, u32* upload_limit_speed);


/*
  * 设置任务最大连接数
  *                   
  * 注意：connection_num 的取值范围为[1~200]，否则返回4112 错误！如果没有特殊要求，建议不要调用这个
  *       		接口，因为下载库会根据资源状况自行选择合适的连接数。
  */
ETDLL_API int32 et_set_max_task_connection(u32 connection_num);

/*
  * 获取任务最大连接数
  *                   
  * 注意：若返回-1，表示下载库还没有被初始化！
  */
ETDLL_API u32 et_get_max_task_connection(void);

/*
 * 获取底层下行速度
 * 注意：底层下行速度为下载库所有接收到的数据量，包括了与外面服务器的各种通信指令和要下载的文件内容，
 *			因此，这个速度通常是大于或等于所有任务在et_get_task_info的ET_TASK结构体中的下载速度(_speed)之和!                   
 *
 */
ETDLL_API u32 et_get_current_download_speed(void);


/*
 * 获取底层上传速度
  * 注意：底层上传速度为下载库所有发出去的数据量，包括了与外面服务器的各种通信指令和点对点协议里的文件内容上传，
  *			因此，这个速度通常是大于或等于所有任务在et_get_task_info的ET_TASK结构体中的上传速度(_ul_speed)之和!                   
 *
 */
ETDLL_API u32 et_get_current_upload_speed(void);


/*
  * 得到下载库内当前所有的task的id，*buffer_len为task_id_buffer的长度，当*buffer_len不够时，返回4115，并在*buffer_len带回正确的长度。 
  * 返回值: 0    成功
                        4112 参数错误，buffer_len和task_id_buffer不能为空
                        4113 当前没有任何任务
                        4115 buffer不够，需要更长的buffer size
  *
  * 注意：如果返回4115，表示所需的task_id_buffer不够长，需要重新传入更多的buffer！
  *                 
  */
ETDLL_API int32 et_get_all_task_id(  u32 *buffer_len, u32 *task_id_buffer );



/*--------------------------------------------------------------------------*/
/*              task相关接口			        
----------------------------------------------------------------------------*/
/*
  * 创建url下载的新任务
  * 返回值: 0    成功，task_id 标识成功创建的任务
                        4103 已经超过最大任务数
			   4119 操作冲突。由于迅雷下载库同一时间只能处理一个任务的创建或删除，如果用户想同时创建或/和删除两个或以上的任务，会有冲突！                       
			   4200  非法url
                        4201  非法path
                        4202 非法filename
                        4216 已经有另一个具有相同url的任务存在
                        4222 所要下载的文件已经存在

  *                   
  * 注意：1.参数url和file_path切不可为NULL，相应的url_length和file_path_len也不能为0，并且url_length最大
  *       不能超过511字节，file_path_len不能超过255字节,要确保file_path为已存在的路径(绝对路径和相对路径均可)，而且file_path的末尾要有字符'/'。
  *       参数ref_url和description要是没有的话可等于NULL,相应的ref_url_length和description_len也可为0，有的话ref_url_length和description_len不能大于511；
  *       另外file_name也可为空或file_name_length等于0，这种情况下，迅雷将从url中解析出文件名，如果有的话file_name要符合linux文件名的命名规则，file_name_length不能大于255。
  *       2.目前只能接受"http://","https://","ftp://"和"thunder://"开头的url.
  *       3.支持创建迅雷看看vod专用url的任务，这种url的格式如下（以/分割字段）：
		版本：0
			http://host:port/version/gcid/cid/file_size/head_size/index_size/bitrate/packet_size/packet_count/tail_pos/url_mid/filename
		版本：1
			http://host:port/version/gcid/cid/file_size/head_size/index_size/bitrate/packet_size/packet_count/tail_pos/扩展项列表/url_mid/filename

		host:port			为CDN集群的域名，如pubnet1.sandai.net:80
		version			是vod url的版本号， 当前0/1
		gcid				文件的gcid信息，十六进制编码
		cid				文件的cid信息，十六进制编码
		file_size 			文件大小, 十六进制编码
		head_size 		头块数据大小, 十六进制编码
		index_size 		索引块大小, 十六进制编码
		bitrate			比特率(bit) , 十六进制编码
		packet size 	包大小 , 十六进制编码 
		packet_count 	包个数，十六进制编码
		tail_pos   		尾部起始位置，十六进制编码
		url_mid 		校验码
		url_mid 表示gcid[20] + cid[20] + file_size[8]，用MD5哈希，再进行十六进制编码得到
		filename 			文件名, 如1.wmv
		扩展项列表 		扩展项/扩展项/.../ext_mid, 以/分割扩展项
		第一个扩展项是custno（客户号），该url是为客户发行的
		ext_mid 表示所有扩展项+url_mid，用MD5哈希，再进行十六进制编码得到
  */
ETDLL_API int32 et_create_new_task_by_url(char* url, u32 url_length, 
							  char* ref_url, u32 ref_url_length,
							  char* description, u32 description_len,
							  char* file_path, u32 file_path_len,
							  char* file_name, u32 file_name_length, 
							  u32* task_id);


/*
  * 创建url下载的断点续传任务，要求cfg配置文件存在，否则创建任务失败
  * 返回值: 0    成功，task_id 标识成功创建的任务       
                        4103 已经超过最大任务数
			   4119 操作冲突。由于迅雷下载库同一时间只能处理一个任务的创建或删除，如果用户想同时创建或/和删除两个或以上的任务，会有冲突！                       
                        4199  对应的.cfg文件不存在
                        4200  非法url
                        4201  非法path
                        4202 非法filename
                        4216 已经有另一个具有相同url的任务存在
                        4222 所要下载的文件已经存在，没必要续传
                        6159 cfg文件已经被毁坏，创建断点续传任务失败
  * 
  * 注意：1.参数url,file_path和file_name切不可为NULL，相应的url_length，file_path_len和file_name_length
  *       也不能为0，并且url_length最大不能超过511字节，file_path_len和file_name_length不能超过255字节,要确保file_name要符合linux文件名的命名规则，file_path为已存在的路径(绝对路径和相对路径均可)，而且file_path的末尾要有字符'/'。
  *       参数ref_url和description要是没有的话可等于NULL,相应的ref_url_length和description_len也可为0，有的话ref_url_length和description_len不能大于511；
  *       另外，file_name对应的td文件和td.cfg文件一定要存在。
  *       2.目前只能接受"http://","https://","ftp://"和"thunder://"开头的url.
  *       3.另外需要特别注意的是，参数file_name要与该任务在用et_create_new_task_by_url创建时传入的file_name_for_user的文件名部分一致，但由于文件的扩展名可能被下载库自动修改，
  		因此为了确保下载库在启动该续传任务时能准确地找到与之对应的.td和.td.cfg文件,一个比较保险的方法就是在用et_create_new_task_by_url创建任务的时候，当任务信息中的
  		ET_TASK结构体中的_file_create_status为ET_FILE_CREATED_SUCCESS的时候，调用et_get_task_file_name函数得到任务的最终文件名，再去除后缀.td，则得到启动续传任务时的正确文件名.
  *       4.支持续传用迅雷看看vod专用url创建的任务。
 */
ETDLL_API int32 et_create_continue_task_by_url(char* url, u32 url_length, 
								   char* ref_url, u32 ref_url_length,
								   char* description, u32 description_len,
								   char* file_path, u32 file_path_len,
								   char* file_name, u32 file_name_length,
								   u32* task_id);



/*
  * 创建cid下载的新任务
  * 返回值: 0    成功，task_id 标识成功创建的任务
                        4103 已经超过最大任务数
 			   4119 操作冲突。由于迅雷下载库同一时间只能处理一个任务的创建或删除，如果用户想同时创建或/和删除两个或以上的任务，会有冲突！                       
                        4201  非法path
                        4202 非法filename
                        4205 非法cid
                        4216 已经有另一个相同的任务存在
                        4222 所要下载的文件已经存在
  *                   
  * 注意：1.参数tcid(content-id)为20字节的十六进制数字串，不可为NULL;file_name和file_path也不可为NULL，相应的 
  *       file_name_length和file_path_len也不能为0，并且file_path_len和file_name_length不能超过255字节,要确保file_name要符合linux文件名的命名规则，file_path为已存在的路径(绝对路径和相对路径均可)，而且file_path的末尾要有字符'/'。
  *       参数file_size要是不知道的话可等于0,但切不可胡乱填一个错误的值，因为这样会导致无法下载！
  */
ETDLL_API int32 et_create_new_task_by_tcid(u8 *tcid, uint64 file_size, char *file_name, u32 file_name_length, char *file_path, u32 file_path_len, u32* task_id );

/*
  * 创建cid下载的续传任务
  * 返回值: 0    成功，task_id 标识成功创建的任务
                        4103 已经超过最大任务数
 			   4119 操作冲突。由于迅雷下载库同一时间只能处理一个任务的创建或删除，如果用户想同时创建或/和删除两个或以上的任务，会有冲突！                       
                           4199  对应的.cfg文件不存在
                    4201  非法path
                        4202 非法filename
                        4205 非法cid
                         4216 已经有另一个相同的任务存在
                        4219 获取cid出错，有可能是从.cfg中得不到cid或得到的cid跟输入的cid不一致
                       4222 所要下载的文件已经存在，没必要续传
                        6159 cfg文件已经被毁坏，创建断点续传任务失败
  *                   
  * 注意：1.参数tcid(content-id)为20字节的十六进制数字串，不可为NULL;file_name,file_path也不可为NULL，相应的  
  *       file_name_length和file_path_len也不能为0，并且file_path_len和file_name_length不能超过255字节,要确保file_path为已存在的路径(绝对路径和相对路径均可)，而且file_path的末尾要有字符'/'。
  *       另外，file_name对应的td文件和td.cfg文件一定要存在。
  *       2.另外需要特别注意的是，参数file_name要与该任务在用et_create_new_task_by_tcid创建时传入的file_name的文件名部分一致，但由于文件的扩展名可能被下载库自动修改，
  		因此为了确保下载库在启动该续传任务时能准确地找到与之对应的.td和.td.cfg文件,一个比较保险的方法就是在用et_create_new_task_by_tcid创建任务的时候，当任务信息中的
  		ET_TASK结构体中的_file_create_status为ET_FILE_CREATED_SUCCESS的时候，调用et_get_task_file_name函数得到任务的最终文件名，再去除后缀.td，则得到启动续传任务时的正确文件名.
  */
ETDLL_API int32 et_create_continue_task_by_tcid(u8 *tcid, char*file_name, u32 file_name_length, char*file_path, u32 file_path_len, u32* task_id );


/*
  * 创建cid+file_size+gcid下载的任务
  * 返回值: 0    成功，task_id 标识成功创建的任务
                        4103 已经超过最大任务数
 			   4119 操作冲突。由于迅雷下载库同一时间只能处理一个任务的创建或删除，如果用户想同时创建或/和删除两个或以上的任务，会有冲突！                       
                        4201  非法path
                        4202 非法filename
                        4205 非法cid
                         4216 已经有另一个相同的任务存在
                        4222 所要下载的文件已经存在
  *                   
  * 注意：1.参数tcid(content-id)为20字节的十六进制数字串，不可为NULL;file_name和file_path也不可为NULL，相应的 
  *       file_name_length和file_path_len也不能为0，并且file_path_len和file_name_length不能超过255字节,要确保file_name要符合linux文件名的命名规则，file_path为已存在的路径(绝对路径和相对路径均可)，而且file_path的末尾要有字符'/'。
  *       参数file_size一定要正确的值，切不可等于0,也不可胡乱填一个错误的值，因为这样会导致无法下载！
  */
ETDLL_API int32 et_create_task_by_tcid_file_size_gcid(u8 *tcid, uint64 file_size, u8 *gcid,char *file_name, u32 file_name_length, char *file_path, u32 file_path_len, u32* task_id );


/*
 * 创建bt下载任务(该接口已经被et_create_bt_task取代)
*/
ETDLL_API int32 et_create_new_bt_task(char* seed_file_full_path, u32 seed_file_full_path_len, 
								char* file_path, u32 file_path_len,
								u32* download_file_index_array, u32 file_num,
								u32* task_id);



/* 
 * 创建bt下载任务的新接口(新任务和断点续传任务均使用此接口，并取代et_create_new_bt_task接口
 *                        多传入一个参数seed_switch_type )
 * 返回值: 0     成功，task_id 标识成功创建的任务
 *		   15361 种子文件不存在
 * 		   15362 种子文件解析失败
 *		   15363 种子文件过大不支持解析(种子文件大小大于2G)
 *		   15364 种子文件下载文件序号非法
 *         15365 bt下载被禁用
 *         15367 种子文件读取错误
 *         15368 不支持的种子文件信息输出类型，只支持gbk和utf-8，big5
 *		   15369 底层不支持gbk转utf-8
 *		   15370 底层不支持gbk转big5
 *		   15371 底层不支持utf-8转gbk
 *		   15372 底层不支持utf-8转big5
 *		   15373 底层不支持big5转gbk
 *		   15374 底层不支持big5转utf-8
 *		   15375 种子文件没有utf-8字段。(仅当编码输出格式设置为ET_ENCODING_UTF8_PROTO_MODE时产生
 *		   15376 重复的文件下载序号
                 15400 子文件下载失败
 *		   4112  非法参数，种子文件全路径或文件下载目录路径为空或太长
                 4201  file_path非法
                4216 已经有另一个相同的任务存在
  *		   2058  该版本的下载库不支持BT下载
 * 注意：1.参数seed_file_full_path是*.torrent文件的完全路径，绝对路径和相对路径均可，不可为NULL, 长度不可大于256+255，
            file_path是文件下载后存放的路径，也不可为NULL, 长度不可大于255
  *         相应的seed_file_full_path_len和file_path_len也不能为0，同时要确保file_path为已存在的路径(绝对路径和相对路径均可)，
  *         而且file_path的末尾要有字符'/'。
  *        2.download_file_index_array是用户选择的需要下载的文件序号数组，file_num为数组内包含的文件个数。文件序号不能超过或等于种子的文件个数。
  *        3.seed_switch_type用来设置单个任务的种子文件编码输出格式
  *                          0 返回原始字段
  *                          1 返回GBK格式编码 
  *                          2 返回UTF-8格式编码
  *                          3 返回BIG5格式编码 
  *				  4 返回种子文件中的utf-8字段
  *				  5 未设置输出格式(使用et_set_seed_switch_type的全局输出设置)  
  */

ETDLL_API int32 et_create_bt_task(char* seed_file_full_path, u32 seed_file_full_path_len, 
								char* file_path, u32 file_path_len,
								u32* download_file_index_array, u32 file_num,
								enum ET_ENCODING_SWITCH_MODE encoding_switch_mode, u32* task_id);

/* 
 * 创建emule下载任务的新接口(新任务和断点续传任务均使用此接口 )
 * 返回值: 0     成功，task_id 标识成功创建的任务
 *		   4103 已经超过最大任务数
 *         4112  非法参数，emule链接为空或太长,全路径或文件下载目录路径为空或太长
           4201  file_path非法
           4216 已经有另一个相同的任务存在
  *		   2058  该版本的下载库不支持emule下载
 *         20482 非法的ed2k_link
 * 注意：1 ed2k_link是emule下载的链接,不可为NULL,长度不可超过2048
 *       2 path是文件下载后存放的路径，也不可为NULL, 长度不可大于255
  *         相应的seed_file_full_path_len和file_path_len也不能为0，同时要确保file_path为已存在的路径(绝对路径和相对路径均可)，
  *         而且file_path的末尾要有字符'/'。
  */

ETDLL_API int32 et_create_emule_task(const char* ed2k_link, u32 ed2k_link_len, char* path, u32 path_len, 
    char* file_name, u32 file_name_length, u32* task_id );


/*
  * 设置任务为无盘任务，所谓无盘任务，就是该任务虽然下载数据，但却不写进磁盘中
   * 返回值: 0    成功
                        4107  非法task_id
                        4113 当前没有可运行的任务
                        4116 任务类型不正确，该任务不能为BT任务，也不能为续传任务
			   4117  任务状态不正确，有可能是该任务已经start了
 *                   
 * 注意：1.该接口一定要在et_start_task之前调用否则无效。
 			2.该接口只用于vod点播任务。
 			3.任务一旦被设置为无盘任务，将不能重设为有盘。
 			4.该接口只能用于新创建的任务，而不能用于续传任务。
  */
ETDLL_API int32 et_set_task_no_disk(u32 task_id);

/*
  * 设置任务为需要校验完数据才给播放器返回数据
   * 返回值: 0    成功
                        4107  非法task_id
                        4113 当前没有可运行的任务
                        4116 任务类型不正确，该任务不能为BT任务，也不能为续传任务
			   4117  任务状态不正确，有可能是该任务已经start了
 *                   
 * 注意：1.该接口一定要在et_start_task之前调用否则无效。
 			2.该接口只用于vod点播任务。
  */
ETDLL_API int32 et_vod_set_task_check_data(u32 task_id);

/*
  * 启动task_id 标识的任务
  * 返回值: 0    成功
                        4107  非法task_id
                        4113 当前没有可运行的任务
			   4117  任务状态不正确，有可能是该任务已经start过了
                        4211  非法GCID,该cid任务无法运行
  *                   
  */
ETDLL_API int32 et_start_task(u32 task_id);


/*
  * 停止task_id 标识的任务
  * 返回值: 0    成功
                        4107  非法task_id
                        4110  task_id标识的任务不是正在运行的任务,无法停止
                        4113  当前没有可停止的任务
  *                   
  * 注意：1.调用et_stop_task停掉task之后不能再调用et_start_task尝试再次启动task，必须在
  *       调用et_delete_task之后通过调用et_create_continue_task_by_xxx启动续传任务！
  *       2.只要是调用et_start_task启动的任务，无论状态是什么（RUNNING，VOD,SUCCESS, FAILED），在调用
  *       et_delete_task之前都必须调用et_stop_task把任务停止掉
*/
ETDLL_API int32 et_stop_task(u32 task_id);

/*
  * 删除task_id 标识的任务
  * 返回值: 0    成功
                        4107  非法task_id
                        4109  task_id标识的任务还没有停止，无法删除
                        4113  当前没有可删除的任务
 			   4119 操作冲突。由于迅雷下载库同一时间只能处理一个任务的创建或删除，如果用户想同时创建或/和删除两个或以上的任务，会有冲突！                       
  *                   
  * 注意：调用et_delete_task之前要确保任务已经用et_stop_task停止掉
 */
ETDLL_API int32 et_delete_task(u32 task_id);


/*
  * 获取task_id 标识的任务的任务信息 
  * 返回值: 0    成功
                        4107  非法task_id
                        4113 当前没有运行的任务
                        4112  无效参数，ET_TASK *info 不能为空
  */
ETDLL_API int32 et_get_task_info(u32 task_id, ET_TASK *info);

/*
  * 获取task_id 标识的任务的文件名 （注意，不适合BT任务）
  * 返回值: 0    成功
                        4107  非法task_id
			   4110  任务还没有运行
 			   4112  非法参数，file_name_buffer不能为空
                        4113 当前没有运行的任务
                        4116 任务类型不对，任务的类型不能为BT任务
                        4202 无法获得任务文件名,有可能是因为文件未创建成功(任务信息中的_file_create_status 不等于ET_FILE_CREATED_SUCCESS)
                        4215 buffer_size过小
  *
  * 注意：1.如果在ET_TASK_RUNNING状态下调用该接口，返回的为临时文件名(*.td)，在任务成功后调用则返回最终文件名。
  *	  		buffer_size为输入输出参数，当输入的buffer size过小时（接口返回4215），此参数返回正确的所需buffer size。
  *	  		2.任务由于意外事件发生中断（如停电，死机等）而需要续传时，为确保续传时传进的文件名是正确的，
  *			建议在用et_create_new_task_by_xxx启动新任务时，当任务信息中的_file_create_status 一旦等于ET_FILE_CREATED_SUCCESS，
  *			调用该接口获得任务的正确文件名，并保存起来以被续传用。注意界面程序要去除文件名后缀.td得到正确的文件名。 
   *	  		3.关于迅雷下载库对任务文件名处理的一点重要说明（注意，不适合BT任务）：当界面程序调用et_create_new_task_by_url或et_create_new_task_by_tcid接口创建一个新的下载任务时（注意，不是BT任务），
  *			如果传进来的文件名（参数file_name_for_user或file_name）是完整且扩展名正确（如my_music.mp3）的，则下载库会把下回来的文件严格按照传进去的名字命名（如my_music.mp3）。
  *			但如果传进来的文件名是空的（file_name_for_user或file_name等于NULL，不建议这样用）或扩展名没有或错误（如my_music或my_music.m3p）,则迅雷下载库会根据网络上找到的正确文件名和扩展名自动对文件进行重命名（如最终文件名为my_music.mp3），
  *			这种情况下，为确保任务由于意外中断而需要续传时传进的文件名正确无误，则需要在用et_create_new_task_by_url或et_create_new_task_by_tcid接口创建新任务时调用et_get_task_file_name来获得正确文件名以备不时之需。
 */
ETDLL_API int32 et_get_task_file_name(u32 task_id, char *file_name_buffer, int32* buffer_size);
ETDLL_API int32 et_get_task_tcid(u32 task_id, u8 * tcid);



/*
  * 删除task_id 标识的任务的相关文件，这些文件有可能是临时文件（任务失败或运行中被中止），也有可能是已经下载到的目标文件（任务成功）。 
  * 返回值: 0    成功
                        4107  非法task_id
                        4109 当前任务正在运行
                        4113 当前没有可执行的任务
  *
  * 注意：如果想要用此函数删除任务的临时文件的话，一定要在et_stop_task之后，et_delete_task之前调用此函数！
  *                  当然，如果你不想删除任务的临时文件，可不调用此函数。
  */
ETDLL_API int32 et_remove_task_tmp_file(u32 task_id);

/*
  * 删除指定的文件所匹配的临时文件，如 file_name.td和file_name.td.cfg文件。 
  * 返回值: 0    成功
  *
  * 注意：这个函数的调用与任务无关。
  */
ETDLL_API int32 et_remove_tmp_file(char* file_path, char* file_name);



/*--------------------------------------------------------------------------*/
/*              BT下载专用接口			
 *
 *              注意：如果这部分的接口函数返回错误码为2058，表示该版本的下载库不支持BT下载！	
 *
----------------------------------------------------------------------------*/
/* 传入种子文件全路径(包括路径和种子文件名，长度最大不能超过256+255字节)，生成种子文件信息: pp_seed_info 
 * 返回值: 0    成功
 *		   15361 种子文件不存在
 * 		   15362 种子文件解析失败
 *		   15363 种子文件过大不支持解析(种子文件大小大于2G)
 *		   15364 种子文件下载文件序号非法
 *         15365 bt下载被禁用
 *         15366 bt下载不支持的转换类型
 *         15367 种子文件读取错误
 *         15368 不支持的种子文件信息输出类型，只支持gbk和utf-8，big5
 *		   15369 底层不支持gbk转utf-8
 *		   15370 底层不支持gbk转big5
 *		   15371 底层不支持utf-8转gbk
 *		   15372 底层不支持utf-8转big5
 *		   15373 底层不支持big5转gbk
 *		   15374 底层不支持big5转utf-8
 *		   15375 种子文件没有utf-8字段。(仅当编码输出格式设置为ET_ENCODING_UTF8_PROTO_MODE时产生
 *         4112  非法参数，种子文件全路径大于256+255字节或传入空字符串
 * 注意：1info_hash为2进值编码，显示需自己转换为hex形式。
 *       2  必须和et_release_torrent_seed_info成对使用。
 *       3  _encoding字段意义: GBK = 0, UTF_8 = 1, BIG5 = 2
 *       4  相当于调用et_get_torrent_seed_info_with_encoding_mode传入encoding_switch_mode=ET_ENCODING_DEFAULT_SWITCH
 */
 
ETDLL_API int32 et_get_torrent_seed_info( char *seed_path, ET_TORRENT_SEED_INFO **pp_seed_info );




/* 在et_get_torrent_seed_info基础上，传入encoding_switch_mode字段，用来控制种子编码输出格式
 * 传入种子文件全路径(包括路径和种子文件名，长度最大不能超过256+255字节)，生成种子文件信息: pp_seed_info 
 * 返回值: 0    成功
 *		   15361 种子文件不存在
 * 		   15362 种子文件解析失败
 *		   15363 种子文件过大不支持解析(种子文件大小大于2G)
 *		   15364 种子文件下载文件序号非法
 *         15365 bt下载被禁用
 *         15366 bt下载不支持的转换类型
 *         15367 种子文件读取错误
 *         15368 不支持的种子文件信息输出类型，只支持gbk和utf-8，big5
 *		   15369 底层不支持gbk转utf-8
 *		   15370 底层不支持gbk转big5
 *		   15371 底层不支持utf-8转gbk
 *		   15372 底层不支持utf-8转big5
 *		   15373 底层不支持big5转gbk
 *		   15374 底层不支持big5转utf-8
 *		   15375 种子文件没有utf-8字段。(仅当编码输出格式设置为ET_ENCODING_UTF8_PROTO_MODE时产生
 *         4112  非法参数，种子文件全路径大于256+255字节或传入空字符串
 * 注意：1)info_hash为2进值编码，显示需自己转换为hex形式。
 *       2)  必须和et_release_torrent_seed_info成对使用。
 *       3)  _encoding字段意义: GBK = 0, UTF_8 = 1, BIG5 = 2
 *       4)  seed_switch_type用来设置单个任务的种子文件编码输出格式?
 * 				0 返回原始字段
 * 				1 返回GBK格式编码 
 * 				2 返回UTF-8格式编码
 * 				3 返回BIG5格式编码 
 * 				4 返回种子文件中的utf-8字段
 * 				5 未设置输出格式(使用et_set_seed_switch_type的全局输出设置)  
*/

ETDLL_API int32 et_get_torrent_seed_info_with_encoding_mode( char *seed_path, enum ET_ENCODING_SWITCH_MODE encoding_switch_mode, ET_TORRENT_SEED_INFO **pp_seed_info );


/* 释放通过et_get_torrent_seed_info接口得到的种子文件信息。
 * 注意：必须和et_get_torrent_seed_info成对使用。
 */
ETDLL_API int32 et_release_torrent_seed_info( ET_TORRENT_SEED_INFO *p_seed_info );


/* torrent文件有GBK,UTF8,BIG5编码方式，默认情况下switch_type=0下载库不对种子编码进行转换，用户可以根据
 * et_get_torrent_seed_info接口得到种子文件编码格式，做相应的文字显示处理。
 * 当用户需要指定编码输出方式时使用此接口，比如若是种子文件格式是UTF8,
 * 用户希望下载库以GBK格式输出，需要调用et_set_enable_utf8_switch(1)
 * switch_type: 
 * 				0 返回原始字段
 * 				1 返回GBK格式编码 
 * 				2 返回UTF-8格式编码
 * 				3 返回BIG5格式编码 
 * 				4 返回种子文件中的utf-8字段
 * 				5 未设置输出格式(使用系统默认输出设置:GBK_SWITCH) 
 * 返回值: 0    成功
 *         15366 操作系统不支持(系统不支持ICONV函数调用)
 *         15368 不支持的编码格式转换，即switch_type不是0，1，2
 */
ETDLL_API int32 et_set_seed_switch_type( enum ET_ENCODING_SWITCH_MODE switch_type );


/*
  * 得到task_id 标识的BT任务的所有需要下载的文件的id，*buffer_len为file_index_buffer的长度，当*buffer_len不够时，返回4115，并在*buffer_len带回正确的长度。 
  * 返回值: 0    成功
                        4107  非法task_id
                        4112 参数错误，buffer_len和file_index_buffer不能为空
                        4113 当前没有可执行的任务
                        4115 buffer不够，需要更长的buffer size
                        4116 错误的任务类型，task_id 标识的任务不是BT任务
  *
  * 注意：如果返回4115，表示所需的file_index_buffer不够长，需要重新传入更多的buffer！
  *                 
  */
ETDLL_API int32 et_get_bt_download_file_index( u32 task_id, u32 *buffer_len, u32 *file_index_buffer );

/*
  * 获取task_id 标识的bt任务中，文件序号为file_index的信息，填充在 ET_BT_FILE结构中。
  * 返回值: 0    成功
                        1174  文件信息正在更新中，暂时不可读，请稍候再试!
                        4107  非法task_id
                        4113 当前没有运行的任务
                        4112  无效参数，ET_BT_FILE *info 不能为空    
                        15364 种子文件序号非法
  */
ETDLL_API int32 et_get_bt_file_info(u32 task_id, u32 file_index, ET_BT_FILE *info);

/*
  * 获取task_id 标识的bt任务中，文件序号为file_index的文件的路径（任务目录内）和文件名。
  * 返回值: 0    成功
                        4107  非法task_id
                        4112  无效参数，file_path_buffer和file_name_buffer 不能为空,*file_path_buffer_size和*file_name_buffer_size 不能小于256    
                        4113 当前没有运行的任务
                        4116 任务类型不对，任务的类型应该为BT任务
                        1424 buffer_size过小
                        15364 种子文件序号非法
  * 注意: 1.两个buffer的长度*buffer_size 均不能小于256。
	  	    2.返回的路径为任务目录内的路径，如该文件在磁盘的路径为c:/tddownload/abc/abc_cd1.avi ,其中用户指定的下载目录为c:/tddownload，则返回的路径为abc/  
          	    3.返回的路径和文件名的编码方式跟用户在创建该任务时传入的encoding_switch_mode相同。
	  	    4.如果函数返回1424，则界面程序需要检查*file_path_buffer_size或*file_name_buffer_size是否小于256，纠正后重新调用该函数即可。
  */
ETDLL_API int32 et_get_bt_file_path_and_name(u32 task_id, u32 file_index,char *file_path_buffer, int32 *file_path_buffer_size, char *file_name_buffer, int32* file_name_buffer_size);


/*--------------------------------------------------------------------------*/
/*              VOD点播专用接口			        
 *
 *              注意：如果这部分的接口函数返回错误码为2058，表示该版本的下载库不支持VOD功能！	
 *
----------------------------------------------------------------------------*/
/*
  * 启动HTTP方式点播需要的HTTP服务，参数为端口号。
  * 返回值: 0    成功
                        4107  非法task_id
     此HTTP SERVER启动后，可供播放器点播，点播URL格式是http://ip:port/task_id_file_index.rmvb
     ip 即运行此程序的IP
     port 即传入的端口号
     task_id 即要点播的任务ID
     file_index即任务中的文件索引，BT任务有效，普通任务忽略
     rmvb 即媒体文件类型，可以为wmv avi等，主要供某些播放器需要文件名后缀使用
  */
ETDLL_API int32 et_start_http_server(u16 port);

/*
  * 停止HTTP方式点播需要的HTTP服务。
  * 返回值: 0    成功
                        4107  非法task_id
  */
ETDLL_API int32 et_stop_http_server(void);

/*
  * API方式获取VOD数据。参数为任务ID，开始位置，获取长度，缓冲区地址，阻塞时间(单位毫秒)。
  建议获取长度16*1024,达到最优获取速度,超时500,1000都可
  * 返回值: 0    成功
                        4107  非法task_id
                        18534 获取数据在进行中
                        18535 操作被中断
                        18536 获取数据超时
                        18537 未知的任务类型
                        18538 内存太大
                        18539 不在获取数据状态
                        18540 任务已经停止
                        18541 已有索引
  */
ETDLL_API int32 et_vod_read_file(int32 task_id, uint64 start_pos, uint64 len, char *buf, int32 block_time );

/*
  * API方式获取BT VOD数据。参数为任务ID，开始位置，获取长度，缓冲区地址，阻塞时间(单位毫秒)。
  * 返回值: 0    成功
                        4107  非法task_id
                        18534 获取数据在进行中
                        18535 操作被中断
                        18536 获取数据超时
                        18537 未知的任务类型
                        18538 内存太大
                        18539 不在获取数据状态
                        18540 任务已经停止
                        18541 已有索引
*/
ETDLL_API int32 et_vod_bt_read_file(int32 task_id, u32 file_index, uint64 start_pos, uint64 len, char *buf, int32 block_time );

/*
设置缓冲时间，单位秒,默认为30秒缓冲,不建议设置该值,以保证播放流畅
 * 返回值: 0    成功
                       2058 无此功能
*/
ETDLL_API int32 et_vod_set_buffer_time(int32 buffer_time );


/*
得到缓冲百分比
需在start任务之后,再调用et_vod_read_file后，调用该函数才能得到结果
 * 返回值: 0    成功
                       2058 无此功能
                        4107  非法task_id
                        18537 未知的任务类型
                        18540 任务已经停止
*/
ETDLL_API int32 et_vod_get_buffer_percent(int32 task_id,  int32* percent );


/*
返回点播内存大小
 * 返回值: 0    成功
                       2058 无此功能
*/
ETDLL_API int32 et_vod_get_vod_buffer_size(int32* buffer_size);


/*
设置点播内存大小,必须在点播内存还未分配时进行设置，否则返回失败
 * 返回值: 0    成功
                       2058 无此功能
                       18401 还有点播任务
                       18411 点播内存已经分配
                       18412 点播内存设置过小,不能小于2M
*/
ETDLL_API int32 et_vod_set_vod_buffer_size(int32 buffer_size);

/*
返回点播内存是否已经分配
 * 返回值: 0    成功
                       2058 无此功能
*/
ETDLL_API int32 et_vod_is_vod_buffer_allocated(int32* allocated);

/*
释放点播内存,必须在所有点播任务都退出删除之后,否则返回错误
 * 返回值: 0    成功
                       2058 无此功能
                       18401 还有点播任务
*/
ETDLL_API int32 et_vod_free_vod_buffer(void);

/*
判断点播任务下载是否完成，用以判断是否可以开始下载下一个电影
 * 返回值: 0    成功
                       2058 无此功能
                        4107  非法task_id
                        18537 未知的任务类型
                        18540 任务已经停止
*/
ETDLL_API int32 et_vod_is_download_finished(int32 task_id, BOOL* finished );

/*
设置log配置文件路径,要在et_init调用之前调用
 * 返回值: 0    成功
                       2058 无此功能
*/
ETDLL_API int32 et_set_log_conf_path(const char* path);

/*
获取所有ThreadID
*/
ETDLL_API int32 et_get_task_ids(int32* p_task_count, int32 task_array_size, char* task_array);

/*--------------------------------------------------------------------------*/
/*             设置用户自定义的底层接口
----------------------------------------------------------------------------*/
#define ET_FS_IDX_OPEN           (0)
#define ET_FS_IDX_ENLARGE_FILE   (1)
#define ET_FS_IDX_CLOSE          (2)
#define ET_FS_IDX_READ           (3)
#define ET_FS_IDX_WRITE          (4)
#define ET_FS_IDX_PREAD          (5)
#define ET_FS_IDX_PWRITE         (6)
#define ET_FS_IDX_FILEPOS        (7)
#define ET_FS_IDX_SETFILEPOS     (8)
#define ET_FS_IDX_FILESIZE       (9)
#define ET_FS_IDX_FREE_DISK      (10)

#define ET_SOCKET_IDX_SET_SOCKOPT (11)

#define ET_MEM_IDX_GET_MEM           (12)
#define ET_MEM_IDX_FREE_MEM          (13)
#define ET_ZLIB_UNCOMPRESS      (14)
#define ET_FS_IDX_GET_FILESIZE_AND_MODIFYTIME   (15)
#define ET_FS_IDX_DELETE_FILE     (16)
#define ET_FS_IDX_RM_DIR              (17)
#define ET_FS_IDX_MAKE_DIR              (18)
#define ET_FS_IDX_RENAME_FILE              (19)
#define ET_SOCKET_IDX_CREATE         (20)
#define ET_SOCKET_IDX_CLOSE         (21)
#define ET_FS_IDX_FILE_EXIST      (22)
#define ET_DNS_GET_DNS_SERVER   (23)
#define ET_LOG_WRITE_LOG            (24)
/*
 * 用来设置用户自定义的底层接口，比如用户可以自己实现文件读写、创建，然后调用这个接口设置进来；
 * 可以设置范围内的任意函数实现。如果不做设置，下载库会调用默认的系统接口来实现。
 * 必须在调用et_init之前设置完毕，否则会设置失败
 *
 * 返回值：0      成功
 *         3272   序号非法
 *         3273   函数指针无效
 *         3672   下载库已经初始化（即此函数调用要在et_init之前进行）
 *
 * 参数列表说明：
 * int32 fun_idx    接口函数的序号
 * void *fun_ptr    接口函数指针
 *
 *
 *  目前支持的接口函数序号以及相对应的函数类型说明：
 *
 *  序号：0 (ET_FS_IDX_OPEN)      函数说明：typedef int32 (*et_fs_open)(char *filepath, int32 flag, u32 *file_id);
 *   说明：打开文件，需要以读写方式打开。成功时返回0，否则返回错误码
 *   参数说明：
 *	 filepath：需要打开文件的全路径； 
 *	 flag：    当(flag & 0x01) == 0x01时，文件不存在创建文件，否则文件不存在时打开失败
                                                                       如果文件存在则以读写权限打开文件
                      当(flag & 0x02) == 0x02时，表示打开只读文件
                      当(flag & 0x04) == 0x04时，表示打开只写
                      当(flag ) = 0x 0时，表示读写权限打开文件
 *	 file_id： 文件打开成功，返回文件句柄
 *
 *  序号：1 (ET_FS_IDX_ENLARGE_FILE)  函数说明：typedef int32 (*et_fs_enlarge_file)(u32 file_id, uint64 expect_filesize, uint64 *cur_filesize);
 *   说明：重新改变文件大小（目前只需要变大）。一般用于打开文件后，进行预创建文件。 成功返回0，否则返回错误码
 *   参数说明：
 *   file_id：需要更改大小的文件句柄
 *   expect_filesize： 希望更改到的文件大小
 *   cur_filesize： 实际更改后的文件大小（注意：当设置大小成功后，一定要正确设置此参数的值!）
 *
 *  序号：2 (ET_FS_IDX_CLOSE)  函数说明：typedef int32 (*et_fs_close)(u32 file_id);
 *   说明：关闭文件。成功返回0，否则返回错误码
 *   参数说明：
 *   file_id：需要关闭的文件句柄
 *
 *  序号：3 (ET_FS_IDX_READ)  函数说明：typedef int32 (*et_fs_read)(u32 file_id, char *buffer, int32 size, u32 *readsize);
 *   说明：读取当前文件指针文件内容。成功返回0，否则返回错误码
 *   参数说明：
 *   file_id： 需要读取的文件句柄
 *   buffer：  存放读取内容的buffer指针
 *   size：    需要读取的数据大小（调用者可以保证不会超过buffer的大小）
 *   readsize：实际读取的文件大小（注意：文件读取成功后，一定要正确设置此参数的值!）
 *
 *  序号：4 (ET_FS_IDX_WRITE)  函数说明：typedef int32 (*et_fs_write)(u32 file_id, char *buffer, int32 size, u32 *writesize);
 *   说明：从当前文件指针处写入内容。成功返回0，否则返回错误码
 *   参数说明：
 *   file_id：  需要写入的文件句柄
 *   buffer：   存放写入内容的buffer指针
 *   size：     需要写入的数据大小（调用者可以保证不会超过buffer的大小）
 *   writesize：实际写入的文件大小（注意：文件写入成功后，一定要正确设置此参数的值!）
 *
 *  序号：5 (ET_FS_IDX_PREAD)  函数说明：typedef int32 (*et_fs_pread)(u32 file_id, char *buffer, int32 size, uint64 filepos, u32 *readsize);
 *   说明：读取指定偏移处的文件内容。成功返回0，否则返回错误码
 *   参数说明：
 *   file_id： 需要读取的文件句柄
 *   buffer：  存放读取内容的buffer指针
 *   size：    需要读取的数据大小（调用者可以保证不会超过buffer的大小）
 *   filepos： 需要读取的文件偏移
 *   readsize：实际读取的文件大小（注意：文件读取成功后，一定要正确设置此参数的值!）
 *
 *  序号：6 (ET_FS_IDX_PWRITE)  函数说明：typedef int32 (*et_fs_pwrite)(u32 file_id, char *buffer, int32 size, uint64 filepos, u32 *writesize);
 *   说明：从指定偏移处写入内容。成功返回0，否则返回错误码
 *   参数说明：
 *   file_id：  需要写入的文件句柄
 *   buffer：   存放写入内容的buffer指针
 *   size：     需要写入的数据大小（调用者可以保证不会超过buffer的大小）
 *   filepos：  需要读取的文件偏移
 *   writesize：实际写入的文件大小（注意：文件写入成功后，一定要正确设置此参数的值!）
 *
 *  序号：7 (ET_FS_IDX_FILEPOS)  函数说明：typedef int32 (*et_fs_filepos)(u32 file_id, uint64 *filepos);
 *   说明：获得当前文件指针位置。成功返回0，否则返回错误码
 *   参数说明：
 *   file_id： 文件句柄
 *   filepos： 从文件头开始计算的文件偏移
 *
 *  序号：8 (ET_FS_IDX_SETFILEPOS)  函数说明：typedef int32 (*et_fs_setfilepos)(u32 file_id, uint64 filepos);
 *   说明：设置当前文件指针位置。成功返回0，否则返回错误码
 *   参数说明：
 *   file_id： 文件句柄
 *   filepos： 从文件头开始计算的文件偏移
 *
 *  序号：9 (ET_FS_IDX_FILESIZE)  函数说明：typedef int32 (*et_fs_filesize)(u32 file_id, uint64 *filesize);
 *   说明：获得当前文件大小。成功返回0，否则返回错误码
 *   参数说明：
 *   file_id： 文件句柄
 *   filepos： 当前文件大小
 *
 *  序号：10 (ET_FS_IDX_FREE_DISK)  函数说明：typedef int32 (*et_fs_get_free_disk)(const char *path, u32 *free_size);
 *   说明：获得path路径所在磁盘的剩余空间，一般用作是否可以创建文件的判断依据。成功返回0，否则返回错误码
 *   参数说明：
 *   path：     需要获取剩余磁盘空间磁盘上的任意路径
 *   free_size：指定路径所在磁盘的当前剩余磁盘空间（注意：此参数值单位是 KB(1024 bytes) !）
 *
 *  序号：11 (ET_SOCKET_IDX_SET_SOCKOPT)  函数说明：typedef _int32 (*et_socket_set_sockopt)(u32 socket, int32 socket_type); 
 *   说明：设置socket的相关参数，目前只支持协议簇PF_INET。成功返回0，否则返回错误码
 *   参数说明：
 *   socket：     需要设置的socket句柄
 *   socket_type：此socket的类型，目前有效的值有2个：SOCK_STREAM  SOCK_DGRAM。这2个宏的取值和所在的OS一致。
 *
 *  序号：12 (ET_MEM_IDX_GET_MEM)  函数说明：typedef int32 (*et_mem_get_mem)(u32 memsize, void **mem);
 *   说明：从操作系统分配固定大小的连续内存块。成功返回0，否则返回错误码
 *   参数说明：
 *   memsize：     需要分配的内存大小
 *   mem： 成功分配之后，内存块首地址放在*mem中返回。
 *
 *  序号：13 (ET_MEM_IDX_FREE_MEM)  函数说明：typedef int32 (*et_mem_free_mem)(void* mem, u32 memsize);
 *   说明：释放指定内存块给操作系统。成功返回0，否则返回错误码
 *   参数说明：
 *   mem：     需要释放的内存块首地址
 *   memsize：需要释放的内存块的大小
 *
 *  序号：14 (ET_ZLIB_UNCOMPRESS)  函数说明：typedef _int32 (*et_zlib_uncompress)( unsigned char *p_out_buffer, int *p_out_len, const unsigned char *p_in_buffer, int in_len );
*   说明：指定zlib库的解压缩函数进来,便于kad网络中找源包的解压,提高emule找源数量
*   参数说明：
*   p_out_buffer：解压后数据缓冲区
*   p_out_len：   解压后数据长度
*   p_in_buffer： 待解压数据缓冲区
*   in_len：      待解压数据长度
*/
ETDLL_API int32 et_set_customed_interface(int32 fun_idx, void *fun_ptr);



ETDLL_API int32 et_set_customed_interface_mem(int32 fun_idx, void *fun_ptr);

#ifdef _CONNECT_DETAIL
/* 迅雷公司内部调试用  */

/*
  * 获取上传pipe的信息 
  * 返回值: 0    成功
                        4112  无效参数，p_upload_pipe_info_array 不能为空
  */
ETDLL_API int32 et_get_upload_pipe_info(ET_PEER_PIPE_INFO_ARRAY * p_upload_pipe_info_array);

#endif /*  _CONNECT_DETAIL  */


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/*
 * 以下为一个最简单的demo示例，更多复杂的操作，请参考demo.c文件
 */
/////////////////////////////////////////////////////////////////
//#include <stdio.h>
//#include <unistd.h>
//#include <string.h> 
//
//#include "embed_thunder.h"
//
//    
///*  This is the call back function of license report result
//	Please make sure keep it in this type:
//	typedef int32 ( *ET_NOTIFY_LICENSE_RESULT)(u32 result, u32 expire_time);
//*/
//int notify_license_result(unsigned int  result, unsigned int  expire_time)
//{
//	/* Add your code here ! */
//	
//	printf("\n******************* notify_license_result ******************\n");
//	printf("result=%u,expire_time=%u",result,expire_time);
//	printf("\n*************** End of notify_license_result ****************\n\n");
//
//	return 0;
//}
// 
///* Main of the demo */
//int main()
//{
//	ET_TASK info;
//	int ret_val = 0;
//	unsigned int task_id = 0;
//	char * url = "http://down.sandai.net/Thunder5.8.6.600.exe";
//
//	/* Initiate the Embed Thunder download library without proxy */
//	 ret_val = et_init( NULL );
//        if(ret_val != 0)
//       {
//              printf(" init error, ret = %d \n", ret_val);
//		goto ErrHandle;
//       }
//
//	/* Set license to  Embed Thunder download library */
//	ret_val=et_set_license("08083000021b29b27e57604f68bece019489747910", strlen("08083000021b29b27e57604f68bece019489747910"));
//	if(ret_val!=0)		
//       {
//           printf(" set_license error, ret = %d \n", ret_val);
//   		goto ErrHandle2;
//       }
//
//	/* Set license callback function to  Embed Thunder download library */
//	ret_val=et_set_license_callback( notify_license_result);
//	if(ret_val!=0)		
//       {
//           printf(" set_license_callback error, ret = %d \n", ret_val);
//   		goto ErrHandle2;
//       }
//	
//	/* Create task */
//       ret_val = et_create_new_task_by_url(url, strlen(url),			 /* url */
//       								NULL, 0,						 /* ref_url */
//       								NULL, 0,						 /* description */
//       								"./", strlen("./"), 			 /* file path */
//       								"Thunder5.8.6.600.exe", strlen("Thunder5.8.6.600.exe"),    /* user file name */
//       								&task_id);
//       if(ret_val != 0)
//	{
//		printf(" create_new_task  error, ret = %d \n",  ret_val);
//		goto ErrHandle3;  
//	}
//	   
//	/* Start task */
//       ret_val = et_start_task(task_id);
//       if(ret_val != 0)
//       {
//           printf(" start_task %u, error, ret = %d \n", task_id, ret_val);
//           goto ErrHandle4;    
//       }
//
//	/* Get the task information */
//	while(TRUE)
//	{
//   		memset(&info,0,sizeof(ET_TASK));
//     	       ret_val = et_get_task_info( task_id, &info);
//		if(ret_val != 0)
//		{
//		    printf("\n get_task_info %u, error, ret = %d \n", task_id, ret_val);
//		    goto ErrHandle4;    
//		}
//		else
//		{
//	        	printf("task id: %u, task_status=%d, task_speed=%7u (%7u,%7u),task_progress=%3u,pipes=(%3u,%3u,%3u,%3u), filesize=%llu\r", 
//	               	task_id,info._task_status,info._speed,info._server_speed, info._peer_speed, info._progress, info._dowing_server_pipe_num, 
//	               	info._connecting_server_pipe_num, info._dowing_peer_pipe_num, info._connecting_peer_pipe_num, info._file_size);
//	        	fflush(stdout);
//		}
//
//		/* Check if the task has finished */
//              if(info._task_status==ET_TASK_SUCCESS ||info._task_status ==ET_TASK_FAILED)
//              {
//                  if((ret_val == 0)&&(info._task_status ==ET_TASK_SUCCESS ))
//                  {
//                      printf("\ntask id:%u, Down load task success!\n", task_id);
//                  }
//                  else 
//                  if((ret_val == 0)&&(info._task_status ==ET_TASK_FAILED))
//                  {
//                      printf("\ntask_id:%u, Down load task failed! \n", task_id);
//                  }
//                      
//      		    /* Stop the task */
//                  et_stop_task( task_id);
//                          
//       	    /* Delete the task */
//                  et_delete_task( task_id);
//      		
//      		    break;
//              }
//					 
//		usleep(2000*1000);
//	}
//
//    /* Uninitiate the Embed Thunder download library */
//    et_uninit();
//
//    return 0;
//
///* Error handler */
//
//ErrHandle4:
//
//    et_stop_task( task_id);
//
//ErrHandle3:
//
//    et_delete_task(task_id);
//
//ErrHandle2:
//	
//    et_uninit();
//
//ErrHandle:
//
//    return 1;
//}

#ifdef __cplusplus
}
#endif
#endif
