/*
 * HTTP protocol for ffmpeg client
 * Copyright (c) 2000, 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/base64.h"
#include "libavutil/avstring.h"
#include "avformat.h"
#include <unistd.h>
#include <strings.h>
#include "network.h"
#include "os_support.h"

/* XXX: POST protocol is not completely implemented because ffmpeg uses
   only a subset of it. */

/* used for protocol handling */
#define BUFFER_SIZE 2048
#define URL_SIZE    4096
#define MAX_REDIRECTS 8

#define MAX_RETRY	10
#define OPEN_RETRY 3
#define IPAD_IDENT	"AppleCoreMedia/1.0.0.8C148 (iPad; U; CPU OS 4_2_1 like Mac OS X; zh_cn)"


typedef struct {
    URLContext *hd;
    unsigned char buffer[BUFFER_SIZE], *buf_ptr, *buf_end;
    int line_count;
    int http_code;
    int64_t chunksize;      /**< Used if "Transfer-Encoding: chunked" otherwise -1. */
    int64_t off, filesize;
	int is_seek;
    char location[URL_SIZE];
} HTTPContext;

static int http_connect(URLContext *h, const char *path, const char *hoststr,
                        const char *auth, int *new_location);
static int http_write(URLContext *h, uint8_t *buf, int size);


/* return non zero if error */
static int http_open_cnx(URLContext *h)
{
    const char *path, *proxy_path;
    char hostname[1024], hoststr[1024];
    char auth[1024];
    char path1[1024];
    char buf[1024];
    int port, use_proxy, err, location_changed = 0, redirects = 0;
    HTTPContext *s = h->priv_data;
    URLContext *hd = NULL;

    proxy_path = getenv("http_proxy");
    use_proxy = (proxy_path != NULL) && !getenv("no_proxy") &&
        av_strstart(proxy_path, "http://", NULL);

    /* fill the dest addr */
 redo:
     av_log(NULL, AV_LOG_INFO, "http_open_cnx url=%s\n",s->location);
    /* needed in any case to build the host string */
    url_split(NULL, 0, auth, sizeof(auth), hostname, sizeof(hostname), &port,
              path1, sizeof(path1), s->location);
    if (port > 0) {
        snprintf(hoststr, sizeof(hoststr), "%s:%d", hostname, port);
    } else {
        av_strlcpy(hoststr, hostname, sizeof(hoststr));
    }

    if (use_proxy) {
        url_split(NULL, 0, auth, sizeof(auth), hostname, sizeof(hostname), &port,
                  NULL, 0, proxy_path);
        path = s->location;
    } else {
        if (path1[0] == '\0')
            path = "/";
        else
            path = path1;
    }
    if (port < 0)
        port = 80;

    snprintf(buf, sizeof(buf), "tcp://%s:%d", hostname, port);
    err = url_open(&hd, buf, URL_RDWR);
    if (err < 0)
        goto fail;

    s->hd = hd;
    if (http_connect(h, path, hoststr, auth, &location_changed) < 0)
        goto fail;
   av_log(NULL, AV_LOG_INFO, "http_open_cnx s->http_code=%d,location_changed=%d\n",s->http_code,location_changed);	
    if ((s->http_code == 301 ||s->http_code == 302 || s->http_code == 303)&& location_changed == 1) {
        /* url moved, get next */
        url_close(hd);
        if (redirects++ >= MAX_REDIRECTS)
            return AVERROR(EIO);
        location_changed = 0;
		s->filesize = -1;/*file changed*/
		s->chunksize = -1;/*chunk may changed also*/
		h->location=s->location;
        goto redo;
    }
    return 0;
 fail:
    if (hd)
        url_close(hd);
    return AVERROR(EIO);
}



static int http_reopen_cnx(URLContext *h,int64_t off)
{
    HTTPContext *s = h->priv_data;
    URLContext *old_hd = s->hd;
    int64_t old_off = s->off;
    int64_t old_chunksize=s->chunksize ;	

   if(off>=0)
    		s->off = off;
    /* if it fails, continue on old connection */
		/*reget it*/
    s->chunksize = -1;
    if (http_open_cnx(h) < 0) {
	 s->chunksize=old_chunksize;
        s->hd = old_hd;
        s->off = old_off;
        return -1;
    }
    url_close(old_hd);
    return off;
}

static int http_open(URLContext *h, const char *uri, int flags)
{
    HTTPContext *s;
    int ret;
	int open_retry=0;
    s = av_malloc(sizeof(HTTPContext));
    if (!s) {
        return AVERROR(ENOMEM);
    }
    h->priv_data = s;
    s->filesize = -1;
    s->chunksize = -1;
    s->off = 0;
    av_strlcpy(s->location, uri+1, URL_SIZE);
	h->location=s->location;
	s->is_seek=0;/*for first rang=0*/
    ret = http_open_cnx(h);
    while(ret<0 && open_retry++<OPEN_RETRY && !url_interrupt_cb()){
		s->is_seek=!s->is_seek;
    	ret = http_open_cnx(h);
    }
	s->is_seek=0;
    if (ret != 0)
        av_free (s);
	//h->is_streamed=1;
	h->is_slowmedia=1;/*make sure we are slowmedia,to let less seek.*/
    return ret;
}
static int http_getc(HTTPContext *s)
{
    int len;
    if (s->buf_ptr >= s->buf_end) {
        len = url_read(s->hd, s->buffer, BUFFER_SIZE);
        if (len < 0) {
            return AVERROR(EIO);
        } else if (len == 0) {
            return -1;
        } else {
            s->buf_ptr = s->buffer;
            s->buf_end = s->buffer + len;
        }
    }
    return *s->buf_ptr++;
}

static int http_get_line(HTTPContext *s, char *line, int line_size)
{
    int ch;
    char *q;

    q = line;
    for(;;) {
        ch = http_getc(s);
        if (ch < 0)
            return AVERROR(EIO);
        if (ch == '\n') {
            /* process line */
            if (q > line && q[-1] == '\r')
                q--;
            *q = '\0';
		
            return 0;
        } else {
            if ((q - line) < line_size - 1)
                *q++ = ch;
        }
    }
}

static int process_line(URLContext *h, char *line, int line_count,
                        int *new_location)
{
    HTTPContext *s = h->priv_data;
    char *tag, *p;

    /* end of header */
    if (line[0] == '\0')
        return 0;
	av_log(NULL, AV_LOG_INFO, "process_line =%s\n",line);
    p = line;
    if (line_count == 0) {
        while (!isspace(*p) && *p != '\0')
            p++;
        while (isspace(*p))
            p++;
        s->http_code = strtol(p, NULL, 10);

        dprintf(NULL, "http_code=%d\n", s->http_code);

        /* error codes are 4xx and 5xx */
        if (s->http_code >= 400 && s->http_code < 600)
            return -1;
    } else {
        while (*p != '\0' && *p != ':')
            p++;
        if (*p != ':')
            return 1;

        *p = '\0';
        tag = line;
        p++;
        while (isspace(*p))
            p++;
        if (!strcasecmp(tag, "Location")) {
            strcpy(s->location, p);
            *new_location = 1;
        } else if (!strcasecmp (tag, "Content-Length") && s->filesize == -1) {
            s->filesize = atoll(p);
        } else if (!strcasecmp (tag, "Content-Range")) {
            /* "bytes $from-$to/$document_size" */
            const char *slash;
            if (!strncasecmp (p, "bytes ", 6)) {
                p += 6;
                s->off = atoll(p);
                if ((slash = strchr(p, '/')) && strlen(slash) > 0)
                    s->filesize = atoll(slash+1);
            }
            h->is_streamed = 0; /* we _can_ in fact seek */
        } else if (!strcasecmp (tag, "Transfer-Encoding") && !strncasecmp(p, "chunked", 7)) {
            s->filesize = -1;
            s->chunksize = 0;
        }
    }
    return 1;
}

static int http_connect(URLContext *h, const char *path, const char *hoststr,
                        const char *auth, int *new_location)
{
    HTTPContext *s = h->priv_data;
    int post, err;
    char line[1024];
    char *auth_b64;
    int auth_b64_len = (strlen(auth) + 2) / 3 * 4 + 1;
    int64_t off = s->off;
    int len, wrote;

    /* send http header */
    post = h->flags & URL_WRONLY;
    if ((wrote = snprintf(s->buffer, sizeof(s->buffer),
                          "%s %s HTTP/1.1\r\n",
                          post ? "POST" : "GET",
                          path)) < 0)
        return AVERROR(EINVAL);
    len = wrote;
    if ((wrote = snprintf(s->buffer+len, sizeof(s->buffer) - len,
                          "User-Agent: %s\r\n",
                          IPAD_IDENT)) < 0)
        return AVERROR(EINVAL);
    len += wrote;
    if (h->headers) {
        if ((wrote = snprintf(s->buffer + len, sizeof(s->buffer) - len, "%s", h->headers)) < 0)
            return AVERROR(EINVAL);
        len += wrote;
    }
    if(s->is_seek>0 ||  s->off>0){
        if ((wrote = snprintf(s->buffer+len, sizeof(s->buffer) - len,
                              "Range: bytes=%"PRId64"-\r\n",
                              s->off)) < 0)
            return AVERROR(EINVAL);
        len += wrote;
    }
    auth_b64 = av_malloc(auth_b64_len);
    av_base64_encode(auth_b64, auth_b64_len, auth, strlen(auth));
    wrote = snprintf(s->buffer+len, sizeof(s->buffer) - len,
                     "Accept: */*\r\n"
                     "Host: %s\r\n"
                     "Authorization: Basic %s\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     hoststr,
                     auth_b64);
    len += wrote;
    av_freep(&auth_b64);
    if (wrote < 0)
        return AVERROR(EINVAL);

    if (http_write(h, s->buffer, strlen(s->buffer)) < 0)
        return AVERROR(EIO);

    /* init input buffer */
    s->buf_ptr = s->buffer;
    s->buf_end = s->buffer;
    s->line_count = 0;
    s->off = 0;
    if (s->filesize <= 0)
        s->filesize = -1;
    if (post) {
        return 0;
    }

    /* wait for header */
    for(;;) {
        if (http_get_line(s, line, sizeof(line)) < 0)
            return AVERROR(EIO);

        dprintf(NULL, "header='%s'\n", line);

        err = process_line(h, line, s->line_count, new_location);
        if (err < 0)
            return err;
        if (err == 0)
            break;
        s->line_count++;
    }

    return (off == s->off) ? 0 : -1;
}


static int http_read(URLContext *h, uint8_t *buf, int size)
{
    HTTPContext *s = h->priv_data;
    int len=AVERROR(EIO);
	int err_retry=MAX_RETRY;
retry:
    if (s->chunksize >= 0) {
        if (!s->chunksize) {
            char line[32];

            for(;;) {
                do {
                    if (http_get_line(s, line, sizeof(line)) < 0)
                    	{
			   goto errors;
                    	}
                } while (!*line);    /* skip CR LF from last chunk */
                s->chunksize = strtoll(line, NULL, 16);
				//av_log(NULL, AV_LOG_INFO, "Chunked encoding data size: %"PRId64"',str=%s\n", s->chunksize,line);

                if (!s->chunksize)
                   return 0;/*eof*/
                break;
            }
        }
        size = FFMIN(size, s->chunksize);
    }
    /* read bytes from input buffer first */
    len = s->buf_end - s->buf_ptr;
    if (len > 0) {
        if (len > size)
            len = size;
        memcpy(buf, s->buf_ptr, len);
        s->buf_ptr += len;
    } else {
        len = url_read(s->hd, buf, size);
    }
    if (len > 0) {
        s->off += len;
        if (s->chunksize > 0)
            s->chunksize -= len;
    }
errors:
	if(len<0 && len!=AVERROR(EAGAIN) && err_retry-->0 && !url_interrupt_cb())
	{
		av_log(NULL, AV_LOG_INFO, "http_read failed err try=%d\n", err_retry);
		http_reopen_cnx(h,-1);
		goto retry;
	}
	
    	return len;

}

/* used only when posting data */
static int http_write(URLContext *h, uint8_t *buf, int size)
{
    HTTPContext *s = h->priv_data;
    return url_write(s->hd, buf, size);
}

static int http_close(URLContext *h)
{
    HTTPContext *s = h->priv_data;
    url_close(s->hd);
    av_free(s);
    return 0;
}

static int64_t http_seek(URLContext *h, int64_t off, int whence)
{
    HTTPContext *s = h->priv_data;
    int ret=-1;
	int open_retry=0;

    if (whence == AVSEEK_SIZE)
        return s->filesize;
    else if ((s->filesize == -1 && whence == SEEK_END) || h->is_streamed)
        return -1;
    /* we save the old context in case the seek fails */
	av_log(NULL, AV_LOG_INFO, "http_seek:seek to %lld,whence=%d\n",off,whence);
    if (whence == SEEK_CUR)
        off += s->off;
    else if (whence == SEEK_END)
        off += s->filesize;
	s->is_seek=1;
    /* if it fails, continue on old connection */
   ret=http_reopen_cnx(h,off);
	while(ret<0 && open_retry++<OPEN_RETRY&& !url_interrupt_cb())
    {
     	if(off<0 || (s->filesize >0 && off>=s->filesize))
     	{
     		/*try once,if,out of range,we return now;*/
		break;
     	}
		ret=http_reopen_cnx(h,off);
    }
	s->is_seek=0;
    if(ret<0)
		return -1;
    else
    	return off;
}

static int
http_get_file_handle(URLContext *h)
{
    HTTPContext *s = h->priv_data;
    return url_get_file_handle(s->hd);
}

URLProtocol shttp_protocol = {
    "shttp",
    http_open,
    http_read,
    http_write,
    http_seek,
    http_close,
    .url_get_file_handle = http_get_file_handle,
};

