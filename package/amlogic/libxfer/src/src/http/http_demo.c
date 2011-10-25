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

#include "xfer_common.h"
#include "xfer_error.h"
#include "http_manager.h"
#include "../common/xfer_debug.h"
#include "../common/transfer_utils.h"
#include "../common/listop.h"

#ifdef HAS_OFFLINE_DEMO
#include "ixml.h"

typedef struct demo_node_{
	list_t	link;

	char* 	flag;
	char* 	url;
	char* 	file_path;
}demo_node_t;

typedef struct demo_list_
{
	int count;
	
	demo_node_t node;
}demo_list_t;

static demo_list_t s_demo_list;
static int s_demo_init = 0;
static IXML_Document* demo_doc = NULL;

static xfer_errid_t demo_list_init()
{
	if (!s_demo_init)
	{
		s_demo_init = 1;
		s_demo_list.count = 0;
		INIT_LIST_HEAD(&(s_demo_list.node.link));
	}

	return XFER_ERROR_OK;
}

static xfer_errid_t demo_list_pushback(const char* flag, const char* url, const char* file_path)
{
	demo_node_t* node = NULL;
	
	if (flag == NULL || url == NULL || file_path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	node = (demo_node_t*)xfer_malloc(sizeof(demo_node_t));
	if (node == NULL)
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	memset(node, 0, sizeof(demo_node_t));
	INIT_LIST_HEAD(&(node->link));

	node->flag = (char*)xfer_malloc(strlen(flag)+1);
	node->url = (char*)xfer_malloc(strlen(url)+1);
	node->file_path = (char*)xfer_malloc(strlen(file_path)+1);
	if (node->flag == NULL || node->url == NULL || node->file_path == NULL)
	{
		if (node->flag)
		{
			xfer_free(node->flag);
			node->flag = NULL;
		}
		if (node->url)
		{
			xfer_free(node->url);
			node->url = NULL;
		}
		if (node->file_path)
		{
			xfer_free(node->file_path);
			node->file_path = NULL;
		}
		xfer_free(node);

		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	strcpy(node->flag, flag);
	strcpy(node->url, url);
	strcpy(node->file_path, file_path);

	list_add_tail(&(node->link), &(s_demo_list.node.link));
	s_demo_list.count++;

	return XFER_ERROR_OK;
}

static xfer_errid_t demo_list_clear()
{
	list_t *next, *cur;
	demo_node_t* node = NULL;

	if (!s_demo_init)
		return XFER_ERROR_HTTP_UNINITIAL;

	list_for_each_safe(cur, next, &(s_demo_list.node.link)) 
	{
		node = list_entry(cur, demo_node_t, link);
		list_del(cur);
		if (node->flag)
		{
			xfer_free(node->flag);
			node->flag = NULL;
		}
		if (node->url)
		{
			xfer_free(node->url);
			node->url = NULL;
		}
		if (node->file_path)
		{
			xfer_free(node->file_path);
			node->file_path = NULL;
		}
		xfer_free(node);
	}
	s_demo_list.count = 0;

	return XFER_ERROR_OK;
}

int 
http_demo_read_file(const char* file_name, char** buff)
{
	int 	fd = -1;
	int 	read_len = 0;
	int 	curr_read_len = 0;
	int		need_read = 0;
	struct stat stat_buf;

	if ((fd = xfer_fileopen(file_name, XFER_O_RDONLY | XFER_O_BINARY)) == -1
		|| fstat(fd, &stat_buf) == -1)
	{
		if (fd != -1)
			xfer_fileclose(fd);
		
		return -1;
	}

	need_read = stat_buf.st_size;
	if ((*buff = (char*)xfer_malloc(need_read+1)) == NULL)
	{
		xfer_fileclose(fd);
		return -1;
	}

	curr_read_len = 0;
	do{
		curr_read_len = xfer_fileread(fd, *buff+read_len, need_read);
		if (curr_read_len <= 0)
		{
			xfer_free(*buff);
			*buff = NULL;
			xfer_fileclose(fd);
			return -1;
		}
		read_len += curr_read_len;
		need_read -= curr_read_len;
	}while(need_read);
	(*buff)[read_len] = 0;
	
	xfer_fileclose(fd);

	return read_len;
}

extern char xfer_demo_path[DL_MAX_PATH];

xfer_errid_t 
http_demo_task_start(transfer_taskid_t taskid)
{
	list_t *next, *cur;
	http_async_context_t* cxt = NULL;
	xfer_errid_t errid = XFER_ERROR_OK;

	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL || cxt->url == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	/*while(s_demo_list.count == 0)  // test code ONLY
	{
		sleep(8000);
		http_demo_xml_parse((const char*)("/mnt/D/demo/demo.xml"));
	}*/

	if (!s_demo_init || s_demo_list.count == 0)
		return XFER_ERROR_HTTP_UNINITIAL;

	list_for_each_safe(cur, next, &(s_demo_list.node.link)) 
	{
		demo_node_t* demo_node = list_entry(cur, demo_node_t, link);

		if (demo_node->url && 0 == strncmp(demo_node->url, cxt->url, strlen(demo_node->url)))
		{
			char file_name[512];

			strcpy(file_name, xfer_demo_path);
			if(file_name[strlen(file_name) - 1] == '/')
				strcat(file_name, demo_node->file_path + 1);
			else
				strcat(file_name, demo_node->file_path);

			if (demo_node->flag && 0 == strcmp(demo_node->flag, "1"))
			{
				char* pos = NULL;
				
				if (strlen(cxt->url) <= strlen(demo_node->url))
					return XFER_ERROR_HTTP_URL_ERR;

				pos = cxt->url + strlen(demo_node->url);
				strcat(file_name, pos);
			}
			
			if (cxt->mode & XFER_BUFF)
			{
				int file_len = 0;
				char* buff = NULL;
				if ((file_len = http_demo_read_file(file_name, &buff)) == -1)
				{
					errid = XFER_ERROR_HTTP_STD_404;
					printf("http_demo_read_file()%s\n error\n", file_name);
				}
				cxt->buff = buff;
				cxt->buff_len = file_len;
			}
			else
			{
				int file_len = 0;
				struct stat st;
				if (stat(file_name, &st) == -1)
				{
					errid = XFER_ERROR_HTTP_STD_404;
					printf("http_demo_read_file()%s\n error\n", file_name);
				}
				else
				{
					file_len = st.st_size;
				}
				if(cxt->file_name)
					xfer_free(cxt->file_name);
				cxt->file_name = xfer_malloc(strlen(file_name) + 1); 
				if(cxt->file_name == NULL)
				{
					printf("http malloc error\n");
					return XFER_ERROR_MALLOC_FAILURE;
				}
				else
					strcpy(cxt->file_name, file_name);

				cxt->saved_len = file_len;
				//node->cb(node->taskid, file_name, file_len, err);
			}
			return errid;
		}
	}

	return errid;
}

#define ERROR_RETURN(a) \
	if (node) \
		ixmlNode_free(node); \
	if (demo_doc) \
		ixmlDocument_free(demo_doc); \
	if (xml) \
		xfer_free(xml); \
	return a;
	
xfer_errid_t
http_demo_xml_parse(const char* file_name)
{
	IXML_Node* node = NULL;
	IXML_Node* child = NULL;
	char* node_name = NULL;
	char* flag = NULL;
	char* url  = NULL;
	char* file_path = NULL;
	char* xml = NULL;

	demo_list_init();
	demo_list_clear();
	
	if(http_demo_read_file(file_name, &xml) == -1)
		return XFER_ERROR_HTTP_READ_FILE_ERR;
	
	if(ixmlParseBufferEx(xml, &demo_doc))
	{
		ERROR_RETURN(XFER_ERROR_HTTP_FILE_CONTENT_ERROR);
	}
	
	node = ixmlNode_getFirstChild(&demo_doc->n);
	if(NULL == node)
	{
		ERROR_RETURN(XFER_ERROR_HTTP_FILE_CONTENT_ERROR);
	}

	node_name = ixmlNode_getNodeName(node);
	if(NULL == node_name)
	{
		ERROR_RETURN(XFER_ERROR_HTTP_FILE_CONTENT_ERROR);
	}

	if(!ixmlNode_hasChildNodes(node))
	{
		ERROR_RETURN(XFER_ERROR_HTTP_FILE_CONTENT_ERROR);
	}

	// start to get information node by node
	node = ixmlNode_getFirstChild(node);
	if(NULL == node)
	{
		ERROR_RETURN(XFER_ERROR_HTTP_FILE_CONTENT_ERROR);
	}

	do
	{
		node_name = ixmlNode_getNodeName(node);
		if( 0 == strncmp(node_name, "resource", strlen("resource")) )
		{
			flag = NULL;
			url = NULL;
			file_path = NULL;
			child = ixmlNode_getFirstChild(node);
			do
			{
				node_name = ixmlNode_getNodeName(child);
				
				if(0 == strncmp(node_name, "path_flag", strlen("path_flag")))
					flag =  ixmlNode_getNodeValue(ixmlNode_getFirstChild(child)) ;
				else if(0 == strncmp(node_name , "url", strlen("url")) ) 
					url = ixmlNode_getNodeValue(ixmlNode_getFirstChild(child)) ;
				else if(0 == strncmp(node_name, "path_file", strlen("path_file")))
					file_path = ixmlNode_getNodeValue(ixmlNode_getFirstChild(child));

				child = ixmlNode_getNextSibling(child);
			}while(child);

			demo_list_pushback(flag, url, file_path);
		}
		node = ixmlNode_getNextSibling(node);
	}while(node);

	ERROR_RETURN(XFER_ERROR_OK);

	return XFER_ERROR_OK;
}
#undef ERROR_RETURN

xfer_errid_t
http_demo_xml_release()
{
	demo_list_clear();
	
	return XFER_ERROR_OK;
}
#endif
