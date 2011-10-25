/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Http Module
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <semaphore.h>
#endif

#include <curl/curl.h>

#include "http_curl.h"
#include "http_manager.h"
#include "../common/xfer_debug.h"

#define HTTP_CURL_DETAIL_SHOW

#define HTTP_CURL_LOOP_TIME 1
#define HTTP_CURL_SELECT_MAX_TIMES 30
extern int s_http_mgr_redirs_max;
extern int s_http_mgr_timeout_max;
extern int s_http_mgr_connect_timeout_max;
extern int s_http_mgr_setup_demo;
extern http_mgr_status_t s_http_mgr_status;
extern http_mgr_list_t http_mgr_tasks;

static int http_curl_complete_cb(int taskid, int error);

xfer_errid_t 
http_curl_queue_start(transfer_taskid_t taskid)
{
	int res = 0;
	struct stat stat_buf;
	char host[256];
	transfer_task_stat_e state = DLST_NONE;
	http_async_context_t* cxt = NULL;

	debugf(XFER_LOG_HTTP, "http_curl_queue_start(), taskid=%d\n" , taskid);

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		cxt->errid = XFER_ERROR_HTTP_IN_PARAM_NULL;
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_IN_PARAM_NULL;
	}
	HTTP_DBG_MSG_SENT(cxt->task_id, 96);

	if(cxt->file_name != NULL)
		res = stat(cxt->file_name, &stat_buf);
	
	if (http_mgr_get_context(cxt->task_id) != cxt)
	{
		cxt->errid = XFER_ERROR_HTTP_IN_PARAM_NULL;
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_IN_PARAM_NULL;
	}

	cxt->state = DLST_NONE;
	cxt->errid = XFER_ERROR_OK;
	cxt->complete_callbacked = 0;
	
	/* for demo mode*/
#ifdef HAS_OFFLINE_DEMO
	if (s_http_mgr_setup_demo)
	{
		http_mgr_sem_unlock();
		res = http_demo_task_start(taskid);
		return http_mgr_task_complete_cb(taskid, res);
	}
#endif
	
	if (cxt->file_name != NULL && res != -1 && cxt->saved_len != 0 && (cxt->file_len >= cxt->saved_len || cxt->file_len == 0))
	{
		if (stat_buf.st_size == 0)
			cxt->saved_len = 0;
		else if (stat_buf.st_size < cxt->saved_len)
			cxt->saved_len = stat_buf.st_size;
	}

	if (cxt->save_file && cxt->file_name != NULL && !(cxt->mode&XFER_SHOW) && cxt->is_cache)
	{
		if (res != -1 && stat_buf.st_size != 0)
		{
			cxt->file_len = stat_buf.st_size;
		}
	}
	strcpy(host, cxt->host);
	state = cxt->state;

	if(cxt->curl == NULL)
	{
		cxt->curl = curl_easy_init( );
		if(cxt->curl == NULL)
		{
			http_mgr_sem_unlock();
			return XFER_ERROR_HTTP_STATUS_ERROR;
		}
		
		if(cxt->url)
		{
			curl_easy_setopt(cxt->curl, CURLOPT_URL, cxt->url);
			curl_easy_setopt(cxt->curl, CURLOPT_PRIVATE, cxt->task_id);

			curl_easy_setopt(cxt->curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(cxt->curl, CURLOPT_MAXREDIRS, s_http_mgr_redirs_max);
			curl_easy_setopt(cxt->curl, CURLOPT_NOSIGNAL, 1L);
			curl_easy_setopt(cxt->curl, CURLOPT_TIMEOUT, s_http_mgr_timeout_max);
			curl_easy_setopt(cxt->curl, CURLOPT_CONNECTTIMEOUT, s_http_mgr_connect_timeout_max);

			curl_easy_setopt(cxt->curl, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(cxt->curl, CURLOPT_PROGRESSFUNCTION, http_curl_progress);
			curl_easy_setopt(cxt->curl, CURLOPT_PROGRESSDATA, cxt->task_id);

			curl_easy_setopt(cxt->curl, CURLOPT_HTTPHEADER, cxt->slist);

			if(cxt->save_file == 1)
			{
				curl_easy_setopt(cxt->curl, CURLOPT_WRITEFUNCTION, http_curl_fwrite);
				curl_easy_setopt(cxt->curl, CURLOPT_WRITEDATA, cxt->task_id);
			}
			else
			{
				curl_easy_setopt(cxt->curl, CURLOPT_WRITEFUNCTION, http_curl_mwrite);
				curl_easy_setopt(cxt->curl, CURLOPT_WRITEDATA, cxt->task_id);
			}
		}
		else
		{
			curl_easy_cleanup(cxt->curl);
			cxt->curl = NULL;
			curl_slist_free_all(cxt->slist); 
			cxt->slist = NULL;
			http_mgr_sem_unlock();
			return XFER_ERROR_HTTP_STATUS_ERROR;
		}

		/* for debug curl */
#ifdef HTTP_DEBUG_ENABLE
		curl_easy_setopt(cxt->curl, CURLOPT_VERBOSE, 1L);
#endif

		res = curl_multi_add_handle(s_http_mgr_status.curl_multi_handle, cxt->curl);
	}
	else
	{
		res = curl_easy_pause(cxt->curl, CURLPAUSE_CONT);
	}
	state = DLST_DOWNLOADING;
	
	if (http_mgr_get_context(cxt->task_id) != cxt)
	{
		res = XFER_ERROR_HTTP_IN_PARAM_NULL;
	}
	else
	{
		cxt->errid = res;
		cxt->state = state;
	}

	http_mgr_sem_unlock();
	return res;
}

xfer_errid_t http_curl_queue_pause(transfer_taskid_t taskid)
{
	http_async_context_t* cxt = NULL;
	
	debugf(XFER_LOG_HTTP, "http_curl_queue_pause(), taskid=%d\n" , taskid);

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}

	if(cxt->curl)
	{
		curl_easy_pause(cxt->curl, CURLPAUSE_ALL);
	}
	cxt->state = DLST_STOPPED;

	http_mgr_sem_unlock();
	
	return XFER_ERROR_HTTP_SUCCESS;
}

xfer_errid_t http_curl_queue_close(transfer_taskid_t taskid)
{
	struct llhead		*lp, *tmp;
	http_mgr_list_t *curr = NULL;
	http_mgr_list_t *find = NULL;

	debugf(XFER_LOG_HTTP, "http_curl_queue_close(), taskid=%d\n" , taskid);
	http_mgr_sem_lock();
	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		curr = LL_ENTRY( lp, http_mgr_list_t, link);
		if (curr->cxt && curr->cxt->task_id == taskid)
		{
			LL_DEL(&curr->link);
			http_mgr_sub_count(curr->cxt->mode);

			find = curr;
			break;
		}
	}
	http_mgr_sem_unlock();

	if (find)
	{
		if(find->cxt->save_file && find->cxt->fd)
		{
			fclose(find->cxt->fd);
			find->cxt->fd = NULL;
		}
		if(find->cxt->curl)
		{
			curl_multi_remove_handle(s_http_mgr_status.curl_multi_handle, find->cxt->curl);
			curl_easy_cleanup(find->cxt->curl);
			find->cxt->curl = NULL;
		}
		if(find->cxt->slist)
		{
			curl_slist_free_all(find->cxt->slist); 
			find->cxt->slist = NULL;
		}

		http_mgr_task_free(find->cxt);

		free(find);
	}

	return XFER_ERROR_HTTP_SUCCESS;
}

void *http_curl_perform(CURLM *multi)
{
	CURLM *multi_handle = multi;
	int still_running = -1; /* keep number of running handles */

	if(multi_handle == NULL)
	{
		debugf(XFER_LOG_HTTP, "http_curl_perform() error, param NULL\n");
		return NULL;
	}

#ifdef HTTP_CURL_DETAIL_SHOW
	debugf(XFER_LOG_MAIN, ">>>> [C1] CURL perform, still_running=%d\n",still_running);
#endif //HTTP_CURL_DETAIL_SHOW
	while(CURLM_CALL_MULTI_PERFORM ==
		  curl_multi_perform(multi_handle, &still_running));

#ifdef HTTP_CURL_DETAIL_SHOW
	debugf(XFER_LOG_MAIN, ">>>> [C2] CURL perform, still_running=%d\n",still_running);
#endif //HTTP_CURL_DETAIL_SHOW
	return NULL;
}

void *http_curl_run(void *data)
{
	CURLM *multi_handle = NULL;
	CURLMsg *msg;
	int i;
	int ret = 0;
	double val;
	long curl_timeout = -1;
	int msgs_in_queue = 0; /* messages in queue */
	static int still_running = -1; /* keep number of running handles */
	http_mgr_status_t *mgr;
	http_async_context_t *cxt;
	xfer_errid_t errid = XFER_ERROR_OK;
	transfer_taskid_t *task_id = NULL;
	transfer_task_stat_e task_state = 0;
	transfer_task_list_t *tasklist = NULL;

	if(data == NULL)
	{
		debugf(XFER_LOG_HTTP, "http_curl_run() error, param NULL\n");
		return NULL;
	}

	mgr = (http_mgr_status_t *)data;
	multi_handle = mgr->curl_multi_handle;

	while(multi_handle)
	{
		/* Add created http tasks into curl multi queue */
		errid = http_mgr_get_all_task_list(&tasklist);
		if(errid == XFER_ERROR_OK)
		{
			task_id = tasklist->task_ids;
			for(i = 0; i < tasklist->task_count; i++)
			{
				errid = http_mgr_get_task_state(*task_id, &task_state);
				debugf(XFER_LOG_HTTP, "~~~~~~~~~ task count:%d ~~~~~~ task_id:%d, task_state:%d!\n", 
						tasklist->task_count, 
						*task_id, 
						task_state);
				debugf(XFER_LOG_HTTP, "~~~~~~~~~~~~~~~~ task_id:%d, task_state:%d!\n", *task_id, task_state);
				switch(task_state)
				{
					case DLST_STARTING:
						if((errid = http_curl_queue_start(*task_id)) != XFER_ERROR_OK)
							http_curl_complete_cb(*task_id, errid);
						break;
					case DLST_STOPPING:
						if((errid = http_curl_queue_pause(*task_id)) != XFER_ERROR_OK)
							http_curl_complete_cb(*task_id, errid);
						break;
					case DLST_CLOSING:
						if((errid = http_curl_queue_close(*task_id)) != XFER_ERROR_OK)
							http_curl_complete_cb(*task_id, errid);
						break;

					case DLST_DOWNLOADING:
						/* get task download progress */ 
#ifdef HTTP_CURL_DETAIL_SHOW
						http_mgr_sem_lock();
						cxt = http_mgr_get_context(*task_id);
						http_mgr_sem_unlock();
						if (cxt == NULL)
						{
							cxt->errid = XFER_ERROR_HTTP_IN_PARAM_NULL;
							debugf(XFER_LOG_MAIN, "ERROR: get cxt error, taskid=%d\n", *task_id);
						}
						else if (cxt->curl)
						{
							ret = curl_easy_getinfo(cxt->curl, CURLINFO_SPEED_DOWNLOAD, &val);
							if((CURLE_OK == ret) && val)
							{
								cxt->speed = (int)val;
								debugf(XFER_LOG_MAIN, "\t[%d] task speed: %0.3f kbyte/sec.\n", *task_id, val / 1024);
							}
							ret = curl_easy_getinfo(cxt->curl, CURLINFO_SIZE_DOWNLOAD, &val);
							if((CURLE_OK == ret) && val && cxt->file_len > 0)
							{
								debugf(XFER_LOG_MAIN, "\t[%d] ==============> \033[7m< %d%% > ( %d/%d ) \033[0m bytes.\n", 
										*task_id, 
										(int)(val * 100 / (float)cxt->file_len),
										(int)val,
										cxt->file_len);
							}
							debugf(XFER_LOG_MAIN, "\t[%d] task url: %s\n", *task_id, cxt->url);
						}
#endif //HTTP_CURL_DETAIL_SHOW
						break;

					default:
						break;
				}
				task_id++;
			}
		}
		http_mgr_release_task_list(tasklist);

		/* perform curl queue, read or write data */
#ifdef HTTP_CURL_DETAIL_SHOW
		if(still_running != 0)
			debugf(XFER_LOG_MAIN, "$$$$$$\t\tCURL perform, still_running=%d\n",still_running);
		ret = curl_multi_perform(multi_handle, &still_running);
		//ret = curl_multi_socket_action(multi_handle, CURL_POLL_INOUT, 0, &still_running);
		if(still_running > 0)
			debugf(XFER_LOG_MAIN, "$$$$$$$$\tCURL perform, still_running=%d, ret=%d\n",still_running, ret);
#endif //HTTP_CURL_DETAIL_SHOW
		
		while(CURLM_CALL_MULTI_PERFORM ==
			  curl_multi_perform(multi_handle, &still_running));

		debugf(XFER_LOG_HTTP, ">>>> [A1] CURL perform, still_running=%d\n",still_running);
		while ((msg = curl_multi_info_read(multi_handle, &msgs_in_queue))) {
			if (msg->msg == CURLMSG_DONE) {
				long taskid;
				int res;
				long response = 0;
				int error = (int)(msg->data.result);
				//CURL *curl = msg->easy_handle;
				res = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &taskid);
				debugf(XFER_LOG_MAIN, ">>>> CURL download: %d - %s taskid=%d\n",
						msg->data.result, curl_easy_strerror(msg->data.result), (int)taskid);
#ifdef HTTP_CURL_DETAIL_SHOW
				/* check for bytes downloaded */ 
				res = curl_easy_getinfo(msg->easy_handle, CURLINFO_SIZE_DOWNLOAD, &val);
				if((CURLE_OK == res) && val)
					debugf(XFER_LOG_MAIN, "\t\tData downloaded: %0.0f bytes.\n", val);

				/* check for total download time */ 
				res = curl_easy_getinfo(msg->easy_handle, CURLINFO_TOTAL_TIME, &val);
				if((CURLE_OK == res) && val)
					debugf(XFER_LOG_MAIN, "\t\tTotal download time: %0.3f sec.\n", val);

				/* check for average download speed */ 
				res = curl_easy_getinfo(msg->easy_handle, CURLINFO_SPEED_DOWNLOAD, &val);
				if((CURLE_OK == res) && val)
					debugf(XFER_LOG_MAIN, "\t\tAverage download speed: %0.3f kbyte/sec.\n", val / 1024);
#endif //HTTP_CURL_DETAIL_SHOW

				/* check for response code  */
				res = curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response);
				if((CURLE_OK == res) && response)
				{
					debugf(XFER_LOG_MAIN, "\t\tResponse code: %ld, data.result: %d\n", response, error);
					if(response >= 400)
						error = (int)response;
				}

				/* complete callback */
				http_curl_complete_cb((int)taskid, error);
			}
			else {
				debugf(XFER_LOG_MAIN, ">>>> Erroe: CURLMsg (%d)\n", msg->msg);
			}
		}

		ret = curl_multi_timeout(multi_handle, &curl_timeout);
		debugf(XFER_LOG_HTTP, ">>>> multi_timeout: ret=%d, timeout=%ld-------------------\n", ret, curl_timeout);

		if(still_running)
		{
			struct timeval timeout;
			int rc; /* select() return code */
			char * mesg = NULL;
			//static int select_loop = 0;

			fd_set fdread;
			fd_set fdwrite;
			fd_set fdexcep;
			int maxfd;

			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdexcep);

			/* set a suitable timeout to play around with */
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			/* get file descriptors from the transfers */
			curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

			rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

			switch(rc) {
				case -1:
					/* select error */
#ifdef HTTP_CURL_DETAIL_SHOW
					debugf(XFER_LOG_MAIN, ">>>> [D1] select err=-1, still_running=%d\n",still_running);

					debugf(XFER_LOG_MAIN, "\t\terrno=%d\n",errno);
					mesg = strerror(errno);
					debugf(XFER_LOG_MAIN, "\t\tMesg:%s\n",mesg); 
#endif //HTTP_CURL_DETAIL_SHOW
					break;
				case 0:
					/* timeout */
#ifdef HTTP_CURL_DETAIL_SHOW
					debugf(XFER_LOG_HTTP, ">>>> [D0] select timeout, still_running=%d\n",still_running);
#endif //HTTP_CURL_DETAIL_SHOW
					while(CURLM_CALL_MULTI_PERFORM ==
						curl_multi_perform(multi_handle, &still_running));
					break;
				default:
					/* readable/writable sockets */
					debugf(XFER_LOG_HTTP, ">>>> [D2] select ok, rc=%d, still_running=%d\n", rc, still_running);
					while(CURLM_CALL_MULTI_PERFORM ==
						curl_multi_perform(multi_handle, &still_running));
					break;
			}
			
			while ((msg = curl_multi_info_read(multi_handle, &msgs_in_queue))) {
				long taskid;
				int res;
				double val;
				long response = 0;
				int error = (int)(msg->data.result);
				//CURL *curl = msg->easy_handle;
				if (msg->msg == CURLMSG_DONE) {
					res = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &taskid);
					debugf(XFER_LOG_HTTP, ">>>> CURL complete.Result: %d - %s taskid=%d\n",
							msg->data.result, curl_easy_strerror(msg->data.result), (int)taskid);

#ifdef HTTP_CURL_DETAIL_SHOW
					debugf(XFER_LOG_MAIN, ">>>> [B] CURL complete.Result: %d - %s taskid=%d\n",
							msg->data.result, curl_easy_strerror(msg->data.result), (int)taskid);
					/* check for bytes downloaded */ 
					res = curl_easy_getinfo(msg->easy_handle, CURLINFO_SIZE_DOWNLOAD, &val);
					if((CURLE_OK == res) && val)
						debugf(XFER_LOG_MAIN, "\t\tData downloaded: %0.0f bytes.\n", val);

					/* check for total download time */ 
					res = curl_easy_getinfo(msg->easy_handle, CURLINFO_TOTAL_TIME, &val);
					if((CURLE_OK == res) && val)
						debugf(XFER_LOG_MAIN, "\t\tTotal download time: %0.3f sec.\n", val);

					/* check for average download speed */ 
					res = curl_easy_getinfo(msg->easy_handle, CURLINFO_SPEED_DOWNLOAD, &val);
					if((CURLE_OK == res) && val)
						debugf(XFER_LOG_MAIN, "\t\tAverage download speed: %0.3f kbyte/sec.\n", val / 1024);
#endif //HTTP_CURL_DETAIL_SHOW

					/* check for response code  */
					res = curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response);
					if((CURLE_OK == res) && response)
					{
						debugf(XFER_LOG_MAIN, "\t\tResponse code: %ld, data.result: %d\n", response, error);
						if(response >= 400)
							error = (int)response;
					}

					/* complete callback */
					http_curl_complete_cb((int)taskid, error);
				}
				else {
					res = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &taskid);
					debugf(XFER_LOG_MAIN, ">>>> Erroe: CURLMsg (%d)\n", msg->msg);
					debugf(XFER_LOG_MAIN, ">>>> CURL complete.Result: %d - %s taskid=%d\n",
							msg->data.result, curl_easy_strerror(msg->data.result), (int)taskid);
					/* complete callback */
					http_curl_complete_cb((int)taskid, -1);
				}

				//select_loop = 0;
			}

			/*
			if(select_loop++ > HTTP_CURL_SELECT_MAX_TIMES)
			{
#ifdef HTTP_CURL_DETAIL_SHOW
				//debugf(XFER_LOG_MAIN, ">>>> select loop max, still_running=%ld---------------\n", still_running);
#endif //HTTP_CURL_DETAIL_SHOW
				select_loop = 0;
				break;
			}
			*/
		} 

#ifdef __WIN32__
		Sleep(HTTP_CURL_LOOP_TIME * 1000);
#else
		sleep(HTTP_CURL_LOOP_TIME);
#endif
		debugf(XFER_LOG_HTTP, ">>>> [A2] CURL perform, still_running=%ld\n",still_running);
	}

	return NULL;
}

#ifdef __WIN32__
DWORD WINAPI http_curl_run_win32( LPVOID lpParam )
{
	http_curl_run((void *)lpParam);

	return TRUE;
}
#endif

static int http_curl_complete_cb(int taskid, int error)
{
	return http_mgr_task_complete_cb(taskid, error);
}

static void *http_curl_realloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
	if(ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}

size_t
http_curl_mwrite(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	http_async_context_t* cxt = NULL;
	transfer_taskid_t taskid = (transfer_taskid_t)data;

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	http_mgr_sem_unlock();

	if(cxt == NULL)
	{
		debugf(XFER_LOG_MAIN, "\t\thttp mwrite ERROR, cxt = NULL\n");
		return 0;
	}

	cxt->buff = http_curl_realloc(cxt->buff, cxt->buff_len + realsize + 1);
	if (cxt->buff) {
		memcpy(&(cxt->buff[cxt->buff_len]), ptr, realsize);
		cxt->buff_len += realsize;
		cxt->buff[cxt->buff_len] = 0;
	}
	else
	{
		debugf(XFER_LOG_MAIN, "\t\trealloc() error\n");
		return 0;
	}

	return realsize;
}

size_t
http_curl_fwrite(void *ptr, size_t size, size_t nmemb, void *data)
{
	static int fwrite_hit = 0;
	int write_len = 0;
	http_async_context_t* cxt = NULL;
	transfer_taskid_t taskid = (transfer_taskid_t)data;

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	http_mgr_sem_unlock();

	if(!cxt || !cxt->file_name)
	{
		debugf(XFER_LOG_MAIN, "\t\thttp fwrite ERROR, cxt = NULL\n");
		return -1;
	}

	if(!cxt->fd)
	{
		cxt->fd = fopen(cxt->file_name, "wb");
		if(!cxt->fd)
			return -1;
	}

	fwrite_hit ++;
	write_len = fwrite(ptr, size, nmemb, cxt->fd);
	cxt->saved_len += write_len; 

	return write_len;
}

int http_curl_progress(void *clientp,
		       double dltotal,
		       double dlnow,
		       double ultotal,
		       double ulnow)

{
	http_async_context_t* cxt = NULL;
	transfer_taskid_t taskid = (transfer_taskid_t)clientp;

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);

	if(cxt)
	{
		cxt->file_len = (int)dltotal;
		cxt->curr_len = (int)dlnow;
		debugf(XFER_LOG_HTTP, "\t\tdltotal: %.02f,  dlnow: %0.2f\n", cxt->file_len, cxt->curr_len);
	}
	http_mgr_sem_unlock();

	return 0;
}
