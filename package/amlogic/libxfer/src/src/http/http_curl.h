/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Http Module
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/


#ifndef _HTTP_CURL_H__
#define _HTTP_CURL_H_

#include <curl/curl.h>

#include "http_manager.h"

#ifdef __WIN32__
DWORD WINAPI http_curl_run_win32( LPVOID lpParam );
#else
void *http_curl_run(void *data);
#endif

void *http_curl_perform(CURLM *multi);

size_t
http_curl_mwrite(void *ptr, size_t size, size_t nmemb, void *data);

size_t
http_curl_fwrite(void *ptr, size_t size, size_t nmemb, void *data);

int http_curl_progress(void *clientp,
		       double dltotal,
		       double dlnow,
		       double ultotal,
		       double ulnow);

#endif //_HTTP_CURL_H_
