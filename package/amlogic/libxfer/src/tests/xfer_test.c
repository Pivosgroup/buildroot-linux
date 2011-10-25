/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: Xfer Module Test
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transfer_manager.h"
#include "transfer_def.h"
#include "xfer_update.h"

#define XFER_MGR_TEST_ENABLE 1
#if XFER_MGR_TEST_ENABLE

#define XFER_TASK_MAX 10
#define DEMO_TASK_MAX 6
#define XFER_HTTP_TASK_MAX 3
#define HTTP_TASK_MAX 14
char s_license[] = "09092700020000000000001eheff19cu95p6gnuw42";

char s_path_prefix[] = "";
char s_load_path[] = "amlogic_download/";
char s_http_tmp_path[] = "http_tmp/";
char s_demo_path[] = "demo/";

//char s_vod_cid[] = "9208617047d679f50d62ff4c9ebc4e41b0748175";
//char s_vod_gcid[] = "54b73ad355bc4d926b33c00ae6d39f00ccb33f4b";
//char s_vod_size[] = "65925093";

char s_vod_cid[] = "989e0d34bb14556d65aee4ada20d55300ab8645e";
char s_vod_gcid[] = "1a4edaa29351d0b387657b9f6f4cf584c7a53582";
char s_vod_size[] = "92221225";

const char demo_urls[DEMO_TASK_MAX][512] = 
{	
	/* 6 */
	/* register.txt */
	"Http://tv1.dnethome.net/smartbox/servlet/boxinit/",
	/* login.txt */
	"Http://tv1.dnethome.net/smartbox/servlet/boxauth/",
	/* eladies.xml */
	"http://news.sina.com.cn/hisense/eladies.xml",

	/* health.xml */
	"http://news.sina.com.cn/hisense/health.xml",
	/* ent.xml */
	"http://news.sina.com.cn/hisense/ent.xml",
	/* sports.xml */
	"http://news.sina.com.cn/hisense/sports.xml",
};

const char demo_dests[DEMO_TASK_MAX][512] =
{
	"amlogic_download/register.txt",
	"amlogic_download/login.txt",
	"amlogic_download/eladies.xml",

	"amlogic_download/health.xml",
	"amlogic_download/ent.xml",
	"amlogic_download/sports.xml",
};

const char http_urls[HTTP_TASK_MAX][512] = 
{	
	/* 7 */
	"http://10.68.11.54:8080/Flashtest/zl/v9374_a1h_25632_0115.img",
	"http://g.cn",
	"http://www.baidu.com",

	"http://bbs.jiashan.gov.cn/music/name/bjhyn.mp3",
	"http://www.yn.xinhuanet.com/travel/2008-03/19/xin_52303051911205623137846.jpg",
	"http://flower.aweb.com.cn/UploadFiles/20073297974663.jpg",
	"http://www.tvtour.com.cn/article/upimg/070405/9_172042.jpg",

#if 0
	"http://down.sandai.net/Thunder5.8.4.572.exe",
	"http://d.kmplayer.cn/kmplayer2010.exe",
	"http://www.126blog.com/Up/2009-10/30199936097.mp3",
	"http://cachefile23.fs2you.com/zh-cn/download/9e5bc3516fd6162627a64ff80e241387/setup_2_7039build_NOD.rar", 
	"http://www.31shsx.com/wzczb/801/xiaohudui/Ð¡»¢¶Ó-°®.mp3",

	"http://aud01.p2v.tudou.com/170/120/090311191232_12919.mp3",
	"http://clubpic.vodone.com/attachments/2010/03/148409859_201003031137309639.mp3",
	"http://webftp.bbs.hnol.net/qyabl/201001/tianyan2cd/cd1/04.mp3",
	"http://www.dsq99.cn/upload/withyou.mp3",
#endif

	/* 7 */
	"http://pic.wallcoo.com/nature/Beach_Vacation/images/Maldives_beach_picture_WALLCOO_1EP009.JPG",
	"http://img.tostudy.com.cn/news/pics/54684/546845582.jpg",
	"http://blog.vsharing.com/Uploads/UserDirs/1/8/2809/%E9%B8%A1%E8%9B%8B%E8%8A%B1.jpg",
	"http://236.travel-web.com.tw/Show/images/Column/5564_2.jpg",
	"http://www.cdta.gov.cn/usersource/module/pic_zjcdgdtpz/007-%E8%8A%99%E8%93%89%E8%8A%B1.jpg",
	"http://news.xinhuanet.com/lady/2009-04/30/xinsrc_00204063008577181259118.jpg",
	"http://www.yn.xinhuanet.com/travel/2007-02/28/xin_1402042814579683088210.jpg",

};

const char http_dests[HTTP_TASK_MAX][512] =
{
	"amlogic_download/",
	"amlogic_download/g.cn.html",
	"amlogic_download/baidu.html",

	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	
	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	
};

char test_urls[XFER_TASK_MAX][512] = 
{	
	"http://10.68.11.54:8080/Flashtest/zl/v9374_a1h_25632_0115.img",
	"http://g.cn",
	"http://www.baidu.com",

	"http://bbs.jiashan.gov.cn/music/name/bjhyn.mp3",
	//"http://get.qt.nokia.com/qt/source/qt-win-opensource-4.6.2-mingw.exe",
	//"http://down.sandai.net/Thunder5.8.4.572.exe",
	"http://hg.ysbird.com:105/data/www.ysbird.com影视鸟/越光宝盒-Blu[1280x720].rmvb",
	"http://cachefile23.fs2you.com/zh-cn/download/9e5bc3516fd6162627a64ff80e241387/setup_2_7039build_NOD.rar", 
	"http://d.kmplayer.cn/kmplayer2010.exe",

	"EF2E7D7294A308137B0FDEB4C20E0ACD103E4BCB",/* tcid task */
};

const char test_dests[XFER_TASK_MAX][512] =
{
	"amlogic_download/",
	"amlogic_download/g.cn.html",
	"amlogic_download/baidu.html",

	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	"amlogic_download/",
	
	"amlogic_download/live.rmvb",
};

char seed_path1[] = "bt/hotbt.torrent";
char seed_path2[] = "bt/jinwumen.torrent";

char bt_src[] = "http://chinabt.cn";
torrent_seed_info_t *seed_info = NULL;

int s_xfer_type = 0;

char get_choice();
void xfer_usage_show();
void demo_usage_show();
void http_usage_show();
void thunder_usage_show();
void xfer_type_show();
void vod_usage_show();


void xfer_file_cb(transfer_taskid_t taskid, const char* file_name, int len, int succ)
{
	static int count = 0;
	printf(">>>>[%d] XFER download finish, taskid=%d, errno=%d. (%s | %d).\n", ++count, taskid, succ, file_name, len);
}

void xfer_mem_cb(transfer_taskid_t taskid, const char* buff, int len, int succ)
{
	char buf[33];

	memset(buf, 0x0, 33);
	if(buff)
	{
		strncpy(buf, buff, 32);
		buf[32] = 0;
	}

	printf(">>>> XFER download finish, taskid=%d, errno=%d. (%s | %d).\n", taskid, succ, buf, len);
}

void test_type_select(void)
{
	int choice = 0;
	xfer_type_show();

	while((choice = get_choice()) != 'q')
	{
		switch(choice)
		{
			case '1':
				s_xfer_type = 1;
				return;
			case '2':
				s_xfer_type = 2;
				return;
			case '3':
				s_xfer_type = 3;
				return;
			case '4':
				s_xfer_type = 4;
				return;
			case 'h':
				xfer_type_show();
				break;
			case 'q':
				s_xfer_type = 0;
				return;

			default:
				printf("Input errror ch=%x \n", choice);
				break;
		}
	}
	return;
}

void demo_test(void)
{
	int errid = 0;
	int taskid[DEMO_TASK_MAX] = {0};
	int choice = 0;
	int i = 0;

#ifndef HAS_OFFLINE_DEMO
	printf("This version has not offline demo.\n");
	return;
#endif

	demo_usage_show();

	while((choice = get_choice()) != 'q')
	{
		switch(choice)
		{
			case 'i':
				errid = transfer_mgr_init();
				printf(">>>> Init Xfer, err=%d \n", errid);
				break;
			case '1':
				printf(">>>> Register HTTP Module start\n");
				errid = transfer_mgr_module_register(XFER_MODULE_HTTP, NULL);
				printf(">>>> Register HTTP Module, err=%d \n", errid);
				errid = transfer_mgr_setopt(DLOPT_SETUP_DEMO, XFER_TASK_HTTP, 1);
				printf(">>>> transfer_mgr_setopt, err=%d \n", errid);
				errid = transfer_mgr_set_demo_path(s_demo_path);
				printf(">>>> transfer_mgr_set_demo_path=%s, err=%d \n", s_demo_path, errid);
				break;
			case 'a':
				printf("-------------- http task add ---------------\n");
				for(i = 0; i < DEMO_TASK_MAX; i++)
				{
					//errid = transfer_mgr_task_add(demo_urls[i], demo_dests[i], XFER_TASK_HTTP, XFER_NULL, NULL, xfer_file_cb, &taskid[i]);
					errid = transfer_mgr_task_add(demo_urls[i], NULL, XFER_TASK_HTTP, XFER_BUFF, NULL, xfer_mem_cb, &taskid[i]);
					printf(">>>>[%d] Add Task, err=%d, id=%d\n", i+1, errid, taskid[i]);
				}

				break;
			case 's':
				/* start or restart HTTP task */
				for(i = 0; i < DEMO_TASK_MAX; i++)
				{
					errid = transfer_mgr_task_start(taskid[i]);
					printf(">>>>[%d] Start Task, err=%d, id=%d\n", i+1, errid, taskid[i]);
				}
				break;
			case 'p':

				/* pause HTTP task */
				for(i = 0; i < DEMO_TASK_MAX; i++)
				{
					errid = transfer_mgr_task_pause(taskid[i]);
					printf(">>>> Pause Task, err=%d, id=%d\n", errid, taskid[i]);
				}
				break;
			case 'c':
				/* close HTTP task */
				for(i = 0; i < DEMO_TASK_MAX; i++)
				{
					transfer_mgr_task_close(taskid[i]);
					printf(">>>> Close Task, id=%d\n", taskid[i]);
				}
				break;
			case 'u':
				printf(">>>> Uninit ...\n");
				transfer_mgr_fini();
				break;
			case 'h':
				demo_usage_show();
				break;
			case 'q':
				printf(">>>> Quit ...\n");
				return;

			default:
				printf("Input errror ch=%x \n", choice);
				break;
		}
	}
	return;
}

void http_test(void)
{
	int errid = 0;
	int taskid[HTTP_TASK_MAX] = {0};
	int choice = 0;
	int i = 0;

	http_usage_show();

	while((choice = get_choice()) != 'q')
	{
		switch(choice)
		{
			case 'i':
				errid = transfer_mgr_init();
				printf(">>>> Init Xfer, err=%d \n", errid);
				//break;
			case '1':
				errid = transfer_mgr_module_register(XFER_MODULE_HTTP, NULL);
				printf(">>>> Register HTTP Module, err=%d \n", errid);
				//errid = transfer_mgr_create_tmp_file(s_http_tmp_path);
				//printf(">>>> Create tmp file, path=%s, err=%d \n", s_http_tmp_path, errid);
				break;
			case 'a':
				printf("-------------- http task add ---------------\n");
				for(i = 0; i < HTTP_TASK_MAX; i++)
				{
					errid = transfer_mgr_task_add(http_urls[i], http_dests[i], XFER_TASK_HTTP, XFER_NULL, NULL, xfer_file_cb, &taskid[i]); 
					printf(">>>>[%d] Add Task, err=%d, id=%d\n", i+1, errid, taskid[i]);
				}

				break;
			case 's':
				/* start or restart HTTP task */
				for(i = 0; i < HTTP_TASK_MAX; i++)
				{
					errid = transfer_mgr_task_start(taskid[i]);
					printf(">>>>[%d] Start Task, err=%d, id=%d\n", i+1, errid, taskid[i]);
				}
				break;
			case 'p':

				/* pause HTTP task */
				for(i = 0; i < HTTP_TASK_MAX; i++)
				{
					errid = transfer_mgr_task_pause(taskid[i]);
					printf(">>>> Pause Task, err=%d, id=%d\n", errid, taskid[i]);
				}
				break;
			case 'c':
				/* close HTTP task */
				for(i = 0; i < HTTP_TASK_MAX; i++)
				{
					transfer_mgr_task_close(taskid[i]);
					printf(">>>> Close Task, id=%d\n", taskid[i]);
				}
				break;
			case 'u':
				printf(">>>> Uninit ...\n");
				transfer_mgr_fini();
				break;
			case 'h':
				http_usage_show();
				break;
			case 'q':
				printf(">>>> Quit ...\n");
				return;

			default:
				printf("Input errror ch=%x \n", choice);
				break;
		}
	}
	return;
}

void thunder_vod_test(void)
{
	int errid = 0;
	int taskid = 0;
	int choice = 0;
	int percent;
	char buf[1024] = {0};
	static int vod_read = 0;

#ifndef HAS_MODULE_THUNDER
	printf("This version has not thunder module.\n");
	return;
#endif

	vod_usage_show();

	errid = transfer_mgr_init();
	printf(">>>> Init Xfer, err=%d \n", errid);

	errid = transfer_mgr_module_register(XFER_MODULE_THUNDER, (void *)s_license);
	printf(">>>> Register THUNDRE Module, err=%d \n", errid);

	errid = transfer_mgr_task_add_vod(s_vod_cid, s_vod_gcid, s_load_path, (int)atoi(s_vod_size), XFER_TASK_XLGCID, "vod.rmvb", &taskid);
	printf(">>>> Add thunde VOD Task, err=%d, id=%d\n", errid, taskid);

	while((choice = get_choice()) != 'q')
	{
		switch(choice)
		{
			case 's':
				errid = transfer_mgr_task_start(taskid);
				printf(">>>> Start Task, err=%d, id=%d\n", errid, taskid);
				break;
			case 'p':
				errid = transfer_mgr_task_pause(taskid);
				printf(">>>> Pause Task, err=%d, id=%d\n", errid, taskid);
				break;
			case 'u':
				printf(">>>> Uninit ...\n");
				transfer_mgr_fini();
				break;
			case 'q':
				printf(">>>> Quit ...\n");
				return;
			case '7':
				if (vod_read <= 1)
				{
					errid = transfer_mgr_read_kankan_file(taskid, 0, 1024, buf, 200);
					vod_read++;
				}
				errid = transfer_mgr_get_kankan_buffer_percent(taskid, &percent);
				printf(">>>> Read kankan_buffer err=%d, <%d%%>\n", errid, percent);
				break;
				/*
			case '8':
				errid = transfer_mgr_read_kankan_file(taskid, 0, 1024, buf, 200);
				printf(">>>> Read kankan_file, err=%d \n", errid);
				//printf("[%s]\n", buf);
				break;
				*/
			case '9':
				errid = xfer_update_task_init();
				printf(">>>> Show task list detail, err=%d \n", errid);
				break;
			case '0':
				xfer_update_task_fini();
				break;
			case 'h':
				vod_usage_show();
				break;

			default:
				printf("Input errror ch=%x \n", choice);
				break;
		}
	}
	return;
}


void thunder_test(void)
{
	int errid = 0;
	int taskid[XFER_TASK_MAX] = {0};
	transfer_task_list_t* tasklist = NULL;
	int choice = 0;
	int i = 0;
	int j = 0;

#ifndef HAS_MODULE_THUNDER
	printf("This version has not thunder module.\n");
	return;
#endif

	thunder_usage_show();

	while((choice = get_choice()) != 'q')
	{
		switch(choice)
		{
			case 'i':
				errid = transfer_mgr_init();
				printf(">>>> Init Xfer, err=%d \n", errid);
				//break;
			case '2':
				errid = transfer_mgr_module_register(XFER_MODULE_THUNDER, (void *)s_license);
				printf(">>>> Register THUNDRE Module, err=%d \n", errid);
				errid = transfer_mgr_set_path_prefix(s_path_prefix);
				printf(">>>> Set Path Prefix[%s], err=%d \n", s_path_prefix, errid);
				break;
			case 'a':
#if 0 //thunder cid task
				printf("-------------- tunder url task add ---------------\n");
				{
					/* thunder cid task */
					errid = transfer_mgr_task_add(test_urls[7], test_dests[7], XFER_TASK_XLTCID, XFER_NULL, NULL, xfer_file_cb, &taskid[7]); 
					printf(">>>> Add thunde tcid Task, err=%d, id=%d\n", errid, taskid[7]);
				}
#endif
#if 0 //thunder bt task
				{
					/* thunder BT task */
					errid = transfer_mgr_get_seed_info(bt_src, seed_path1, &seed_info); 
					printf("\n------------------------------------------------\n");
					printf("BT name=%s\n", seed_info->name);
					printf("BT saved_path=%s\n", seed_info->saved_path);
					printf("BT seed_file_path=%s\n", seed_info->seed_file_path);
					printf("BT total_size=%d\n", (int)seed_info->total_size);
					printf("BT file_num=%d\n", seed_info->file_num);
					for(j = 0; j < seed_info->file_num; j++)
					{
						printf("\tBT file_index[%d]=%d\n", j, seed_info->download_file_index_array[j]);
					}
					printf("BT download_num=%d\n", seed_info->download_num);
					printf("\n------------------------------------------------\n");

					errid = transfer_mgr_task_add_bt(bt_src, 
								seed_info->seed_file_path,
								seed_info->saved_path,
								seed_info->name,
								seed_info->download_file_index_array,
								seed_info->file_num,
								&taskid[8]); 
					printf(">>>> Add thunde BT Task, err=%d, id=%d\n", errid, taskid[8]);
					transfer_mgr_release_seed_info(&seed_info);
				}
				{
					/* thunder BT task */
					errid = transfer_mgr_get_seed_info(bt_src, seed_path2, &seed_info); 
					printf("\n------------------------------------------------\n");
					printf("BT name=%s\n", seed_info->name);
					printf("BT saved_path=%s\n", seed_info->saved_path);
					printf("BT seed_file_path=%s\n", seed_info->seed_file_path);
					printf("BT total_size=%d\n", (int)seed_info->total_size);
					printf("BT file_num=%d\n", seed_info->file_num);
					for(j = 0; j < seed_info->file_num; j++)
					{
						printf("\tBT file_index[%d]=%d\n", j, seed_info->download_file_index_array[j]);
					}
					printf("BT download_num=%d\n", seed_info->download_num);
					printf("\n------------------------------------------------\n");

					errid = transfer_mgr_task_add_bt(bt_src, 
								seed_info->seed_file_path,
								seed_info->saved_path,
								seed_info->name,
								seed_info->download_file_index_array,
								seed_info->file_num,
								&taskid[9]); 
					printf(">>>> Add thunde BT Task, err=%d, id=%d\n", errid, taskid[9]);
					transfer_mgr_release_seed_info(&seed_info);
				}
#endif
#if 1 //thunder url task
				/* url task*/
				{
					errid = transfer_mgr_task_add(test_urls[4], test_dests[4], XFER_TASK_XLURL, XFER_NULL, NULL, xfer_file_cb, &taskid[4]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[4]);
				}
#endif
#if 0
				{
					errid = transfer_mgr_task_add(test_urls[5], test_dests[5], XFER_TASK_XLURL, XFER_NULL, NULL, xfer_file_cb, &taskid[5]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[5]);
				}
				{
					errid = transfer_mgr_task_add(test_urls[6], test_dests[6], XFER_TASK_XLURL, XFER_NULL, NULL, xfer_file_cb, &taskid[6]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[6]);
				}
				{
					errid = transfer_mgr_task_add(test_urls[7], test_dests[7], XFER_TASK_XLTCID, XFER_NULL, NULL, xfer_file_cb, &taskid[7]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[7]);
				}
#endif

				break;
			case 's':
				errid = transfer_mgr_get_task_list(&tasklist);

				/* start or restart THUNDER task */
				if((errid != 0) || (tasklist == NULL))
				{
					printf("----------tasklist is NULL,errid=%d----------\n", errid);
				}
				else
				{
					printf("----------tasklist->count=%d----------\n", tasklist->task_count);
					for(i = 0; i < tasklist->task_count; i++)
					{
						errid = transfer_mgr_task_start(tasklist->task_ids[i]);
						printf(">>>> Start Task, err=%d, id=%d\n", errid, tasklist->task_ids[i]);
					}

					transfer_mgr_release_task_list(tasklist);
				}

				break;
			case 'p':
				errid = transfer_mgr_get_task_list(&tasklist);

				/* pause THUNDER task */
				if((errid != 0) || (tasklist == NULL))
				{
					printf("----------tasklist is NULL,errid=%d----------\n", errid);
				}
				else
				{
					printf("----------tasklist->count=%d----------\n", tasklist->task_count);
					for(i = 0; i < tasklist->task_count; i++)
					{
						errid = transfer_mgr_task_pause(tasklist->task_ids[i]);
						printf(">>>> Pause Task, err=%d, id=%d\n", errid, tasklist->task_ids[i]);
					}

					transfer_mgr_release_task_list(tasklist);
				}
				
				break;
			case 'c':
				errid = transfer_mgr_get_task_list(&tasklist);

				/* close THUNDER task */
				if((errid != 0) || (tasklist == NULL))
				{
					printf("----------tasklist is NULL,errid=%d----------\n", errid);
				}
				else
				{
					printf("----------tasklist->count=%d----------\n", tasklist->task_count);
					for(i = 0; i < tasklist->task_count; i++)
					{
						errid = transfer_mgr_task_check_busy(tasklist->task_ids[i]);
						printf(">>>> Check Task, id=%d, errid=%d\n", tasklist->task_ids[i], errid);
						if (errid == 0)
						{
							transfer_mgr_task_close(tasklist->task_ids[i]);
							printf(">>>> Close Task, id=%d\n", tasklist->task_ids[i]);
						}
					}

					transfer_mgr_release_task_list(tasklist);
				}
				break;
			case 'l':
				errid = transfer_mgr_load(s_load_path);
				printf(">>>> Load task, err=%d\n", errid);
				break;
			case 'u':
				printf(">>>> Uninit ...\n");
				transfer_mgr_fini();
				xfer_update_task_fini();
				break;
			case 'h':
				thunder_usage_show();
				break;
			case 'q':
				printf(">>>> Quit ...\n");
				return;
			case '9':
				errid = xfer_update_task_init();
				printf(">>>> Show task list detail, err=%d \n", errid);
				break;
			case '0':
				xfer_update_task_fini();
				break;

			default:
				printf("Input errror ch=%x \n", choice);
				break;
		}
	}
	return;
}


void xfer_test(void)
{
	int errid = 0;
	int taskid[XFER_TASK_MAX] = {0};
	transfer_task_list_t* tasklist = NULL;
	int choice = 0;
	int i = 0;

	xfer_usage_show();

	while((choice = get_choice()) != 'q')
	{
		switch(choice)
		{
			case 'i':
				errid = transfer_mgr_init();
				printf(">>>> Init Xfer, err=%d \n", errid);
				break;
			case '1':
				errid = transfer_mgr_module_register(XFER_MODULE_HTTP, NULL);
				printf(">>>> Register HTTP Module, err=%d \n", errid);
				break;
			case '2':
				errid = transfer_mgr_module_register(XFER_MODULE_THUNDER, (void *)s_license);
				printf(">>>> Register THUNDRE Module, err=%d \n", errid);
				break;
			case 'a':
				//for(i = 0; i < XFER_TASK_MAX; i++)
				printf("-------------- http task add ---------------\n");
				{
					errid = transfer_mgr_task_add(test_urls[0], test_dests[0], XFER_TASK_HTTP, XFER_NULL, NULL, xfer_file_cb, &taskid[0]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[0]);
				}
				{
					errid = transfer_mgr_task_add(test_urls[1], test_dests[1], XFER_TASK_HTTP, XFER_NULL, NULL, xfer_file_cb, &taskid[1]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[1]);
				}
				{
					errid = transfer_mgr_task_add(test_urls[2], test_dests[2], XFER_TASK_HTTP, XFER_NULL, NULL, xfer_mem_cb, &taskid[2]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[2]);
				}

				printf("-------------- tunder url task add ---------------\n");
				{
					errid = transfer_mgr_task_add(test_urls[3], test_dests[3], XFER_TASK_XLTCID, XFER_NULL, NULL, xfer_file_cb, &taskid[3]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[3]);
				}
				{
					errid = transfer_mgr_task_add(test_urls[4], test_dests[4], XFER_TASK_XLURL, XFER_NULL, NULL, xfer_file_cb, &taskid[4]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[4]);
				}
				{
					errid = transfer_mgr_task_add(test_urls[5], test_dests[5], XFER_TASK_XLURL, XFER_NULL, NULL, xfer_file_cb, &taskid[5]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[5]);
				}
				{
					errid = transfer_mgr_task_add(test_urls[6], test_dests[6], XFER_TASK_XLURL, XFER_NULL, NULL, xfer_file_cb, &taskid[6]); 
					printf(">>>> Add Task, err=%d, id=%d\n", errid, taskid[6]);
				}
				printf("-------------- tunder tcid task add ---------------\n");
				{
					errid = transfer_mgr_task_add(test_urls[7], test_dests[7], XFER_TASK_XLTCID, XFER_NULL, NULL, xfer_file_cb, &taskid[7]); 
					/* thunder cid task */
					printf(">>>> Add thunder tcid Task, err=%d, id=%d\n", errid, taskid[7]);
				}

				break;
			case 's':
				errid = transfer_mgr_get_task_list(&tasklist);

				/* start or restart THUNDER task */
				if((errid != 0) || (tasklist == NULL))
				{
					printf("----------tasklist is NULL,errid=%d----------\n", errid);
				}
				else
				{
					printf("----------tasklist->count=%d----------\n", tasklist->task_count);
					for(i = 0; i < tasklist->task_count; i++)
					{
						errid = transfer_mgr_task_start(tasklist->task_ids[i]);
						printf(">>>> Start Task, err=%d, id=%d\n", errid, tasklist->task_ids[i]);
					}

					transfer_mgr_release_task_list(tasklist);
				}

				/* start or restart HTTP task */
				for(i = 0; i < XFER_HTTP_TASK_MAX; i++)
				{
					errid = transfer_mgr_task_start(taskid[i]);
					printf(">>>> Start Task, err=%d, id=%d\n", errid, taskid[i]);
				}
				break;
			case 'p':
				errid = transfer_mgr_get_task_list(&tasklist);

				/* pause THUNDER task */
				if((errid != 0) || (tasklist == NULL))
				{
					printf("----------tasklist is NULL,errid=%d----------\n", errid);
				}
				else
				{
					printf("----------tasklist->count=%d----------\n", tasklist->task_count);
					for(i = 0; i < tasklist->task_count; i++)
					{
						errid = transfer_mgr_task_pause(tasklist->task_ids[i]);
						printf(">>>> Pause Task, err=%d, id=%d\n", errid, tasklist->task_ids[i]);
					}

					transfer_mgr_release_task_list(tasklist);
				}

				/* pause HTTP task */
				for(i = 0; i < XFER_HTTP_TASK_MAX; i++)
				{
					errid = transfer_mgr_task_pause(taskid[i]);
					printf(">>>> Pause Task, err=%d, id=%d\n", errid, taskid[i]);
				}
				break;
			case 'c':
				errid = transfer_mgr_get_task_list(&tasklist);

				/* close THUNDER task */
				if((errid != 0) || (tasklist == NULL))
				{
					printf("----------tasklist is NULL,errid=%d----------\n", errid);
				}
				else
				{
					printf("----------tasklist->count=%d----------\n", tasklist->task_count);
					for(i = 0; i < tasklist->task_count; i++)
					{
						transfer_mgr_task_close(tasklist->task_ids[i]);
						printf(">>>> Close Task, id=%d\n", tasklist->task_ids[i]);
					}

					transfer_mgr_release_task_list(tasklist);
				}

				/* close HTTP task */
				for(i = 0; i < XFER_HTTP_TASK_MAX; i++)
				{
					transfer_mgr_task_close(taskid[i]);
					printf(">>>> Close Task, id=%d\n", taskid[i]);
				}
				break;
			case 'u':
				printf(">>>> Uninit ...\n");
				transfer_mgr_fini();
				break;
			case 'l':
				errid = transfer_mgr_load(s_load_path);
				printf(">>>> Load task, err=%d\n", errid);
				break;
			case 'h':
				xfer_usage_show();
				break;
			case 'q':
				printf(">>>> Quit ...\n");
				return;
			case '9':
				errid = xfer_update_task_init();
				printf(">>>> Show task list detail, err=%d \n", errid);
				break;
			case '0':
				xfer_update_task_fini();
				break;

			default:
				printf("Input errror ch=%x \n", choice);
				break;
		}
	}
	return;
}

char get_choice()
{
	char ch[10] = {'\0'};

	printf("Enter the letter of your choice: \n");
	scanf("%s", ch);
	while(getchar() != '\n')
		continue;
	
	return ch[0];
}

void demo_usage_show()
{
	printf("\n-----------------------------------------\n");
	printf("------------- XFER DEMO TEST -------------\n");
	printf("  i. Init xfer \n");
	printf("  1. Register http module and set demo path\n");
	printf("  a. Add task\n");
	printf("  s. Start or restart task\n");
	printf("  p. Pause task\n");
	printf("  c. Close task\n");
	printf("  u. Uninit xfer \n");
	printf("  q. Quit \n");
	printf("  h. Help \n");
	printf("------------------------------------------\n");

}

void http_usage_show()
{
	printf("\n-----------------------------------------\n");
	printf("--------------- HTTP TEST ---------------\n");
	printf("  i. Init xfer \n");
	//printf("  1. Register http module \n");
	printf("  a. Add task\n");
	printf("  s. Start or restart task\n");
	printf("  p. Pause task\n");
	printf("  c. Close task\n");
	printf("  u. Uninit xfer \n");
	printf("  q. Quit \n");
	printf("  h. Help \n");
	printf("------------------------------------------\n");

}

void thunder_usage_show()
{
	printf("\n-----------------------------------------\n");
	printf("--------------- THUNDER TEST ---------------\n");
	printf("  i. Init xfer \n");
	//printf("  2. Register thunder module \n");
	printf("  l. Load thunder tasks\n");
	printf("  a. Add task\n");
	printf("  s. Start or restart task\n");
	printf("  p. Pause task\n");
	printf("  c. Close task\n");
	printf("  u. Uninit xfer \n");
	printf("  9. Show task detail \n");
	printf("  0. Close task detail show \n");
	printf("  q. Quit \n");
	printf("  h. Help \n");
	printf("------------------------------------------\n");

}

void xfer_usage_show()
{
	printf("\n-----------------------------------------\n");
	printf("--------------- XFER TEST ---------------\n");
	printf("  i. Init xfer \n");
	printf("  1. Register http module \n");
	printf("  2. Register thunder module \n");
	printf("  a. Add task\n");
	printf("  s. Start or restart task\n");
	printf("  p. Pause task\n");
	printf("  c. Close task\n");
	printf("  u. Uninit xfer \n");
	printf("  9. Show task detail \n");
	printf("  0. Close task detail show \n");
	printf("  q. Quit \n");
	printf("  h. Help \n");
	printf("------------------------------------------\n");

}

void vod_usage_show()
{
	printf("\n-----------------------------------------\n");
	printf("-------------- XFER VOD TEST --------------\n");
	printf("  s. Start task\n");
	printf("  p. Pause task\n");
	printf("  c. Close task\n");
	printf("  u. Uninit xfer \n");
	printf("  7. Get buff percent \n");
	printf("  9. Show task detail \n");
	printf("  0. Close task detail show \n");
	printf("  q. Quit \n");
	printf("  h. Help \n");
	printf("------------------------------------------\n");

}

void xfer_type_show()
{
	printf("\n-----------------------------------------\n");
	printf("--------------- XFER TEST ---------------\n");
	printf("  1. HTTP TEST \n");
	printf("  2. THUNDER URL, CID OR BT TEST\n");
	printf("  3. THUNDER VOD TEST\n");
	printf("  4. OFFLINE DEMO TEST\n\n");
	printf("  q. Quit \n");
	printf("  h. Help \n");
	printf("------------------------------------------\n");

}

int main(int argc, char **argv)
{
	test_type_select();

	if(s_xfer_type == 1)
		http_test();
	else if(s_xfer_type == 2)
	{
		if (argc >= 2) {
			strcpy(test_urls[4], argv[1]);
		}
		if (argc == 3) {
			strcpy(s_path_prefix, argv[2]);
		}

		printf("-------------THUNDER DWONLOAD TEST--------------\n");
		printf("URL:%s\n", test_urls[4]);
		printf("path:%s\n", s_path_prefix);

		thunder_test();
	}
	else if(s_xfer_type == 3)
	{
		if (argc == 4) {
			strcpy(s_vod_cid, argv[1]);
			strcpy(s_vod_gcid, argv[2]);
			strcpy(s_vod_size, argv[3]);
		}

		printf("-------------THUNDER VOD TEST--------------\n");
		printf("CID:%s\n", s_vod_cid);
		printf("GCID:%s\n", s_vod_gcid);
		printf("SIZE:%d\n", s_vod_size);

		thunder_vod_test();
	}
	else if(s_xfer_type == 4)
		demo_test();

	return 0;
}

#endif //XFER_MGR_TEST_ENABLE
