
#include <ixml.h>
#include <xml_util.h>
#include "aw_windows.h"
#include "ioapi.h"
#include "kankan_parse.h"
#include "net/ABoxBase/abx_error.h"
#include "Unicode/inc/unicode.h"
#include "Unicode/inc/unicode.h"
#include <AVmalloc.h>


#define		DLUI_MALLOC		AVMem_malloc
#define		DLUI_CALLOC		AVMem_calloc
#define		DLUI_REALLOC	AVMem_realloc
#define		DLUI_FREE		AVMem_free
#define 	_AVMem_free(p)  do{ if( p ){AVMem_free(p);p=NULL;}}while(0)
#define 	LOCAL_FILE_DEV_ROOT  "/mnt/"

static char* DLUIUtils_strdup(char *s)
{
	char *d = NULL;
	int len;
	if(s)
	{
		len = strlen(s);
                int i = 0;
                for( i = len - 1 ; i >= 0 ; i-- )
		{
			if( ' ' == s[i] )
			{
				s[i] = '\0' ;
				len -- ;
			}
			else
				break ;
		}
		if( 0 == len )
			return NULL ;
		//if( len > 256 )
		//	len = 256 ;
		if(NULL != ( d = (char*)DLUI_MALLOC(len + 1) ) )
		{
			//strncpy( d , s , len ) ;
			strcpy(d,s);
			d[len] = 0 ;
		}	
	}
	return d;
}

#define MAX_DES_STRING_SIZE		1024

static char * AddString( char * cRes , const char * cTitle , const  char * cAddstr , const char * cUnit)
{
	char * d = NULL ;
	if( cAddstr )
	{
		int ilen = 0 ;
		int iResLen = 0 ;
		if( cRes )
		{
			ilen = strlen( cRes ) ;
			iResLen = ilen ;
		}
		
		ilen += strlen( cTitle ) ;
		ilen += strlen( cAddstr ) ;
		if( cUnit )
		{
			ilen += strlen( cUnit ) ;
			ilen += 1 ;
		}
		ilen += strlen("\n") ;

		if(ilen > MAX_DES_STRING_SIZE)
			ilen = MAX_DES_STRING_SIZE;
		
		if( NULL != ( d = (char * )DLUI_REALLOC( cRes , ilen + 2 ) ) )
		{
			if( cUnit )
				snprintf( d + iResLen , ilen+1 - iResLen, "%s%s %s\n" ,  cTitle , cAddstr , cUnit ) ;
			else
				snprintf( d + iResLen , ilen+1 - iResLen, "%s%s\n" ,  cTitle , cAddstr ) ;
			d[ilen+1] = 0;
		}
	}
	return d ;
}

//#define BEGIN_POS	"begin_pos"
//#define TOTAL		"total"
//#define DRM		"drm"
//#define MOVIEINFO	"movieinfo"
#define MOVIELIST	"movielist"
#define PARENT		"parent"
#define ID			"id"
#define TITLE		"title"
#define COUNT		"count"
#define MOVIESETS	"moviesets"
#define MOVIE		"movie"
#define ISCHANNEL	"ischannel"
#define SIZE		"size"
#define NAME		"name"
#define URL			"url"

#define FILMDATA	"FilmData"
#define FILMCOUNT	"FilmCount"
#define FILM		"Film"
#define DIRECTOR 	"Director"
#define ACTOR		"Actor"
#define CLASS		"Class"
#define TYPE 		"Type"
#define PUBTIME		"PubTime"
#define AREA		"Area"
#define LONGTIME	"LongTime"
#define PIC			"Pic"
#define PIC2		"BImgUrl"
#define DESCRIPTI	"Descripti"
#define DESCRIPTI2	"Description"
#define POPULARITY  "Popularity"
#define UPDATE      "Update"
#define DEFINITION  "Definition"
#define LINKURL     "LinkUrl"
#define LINKURL2    "Url"

#define UPDATE		"Update"
#define EPOSODES	"Eposodes"
#define LANG		"Lang"
#define VODFILE		"VODfile"
#define VOD			"VOD"
#define SUBID       "Subid"
#define CID			"CID"
#define GCID		"GCID"
#define VODSIZE     "VODsize"
#define VODNAME     "VODname"
#define VODFORMAT	"VODformat"
#define VODTIME     "VODtime"
/*
static char * getvalue_from_UTF8( const char * value ) 
{
	char *d = NULL;
	if( value )
	{
		int iLen = GetUTF8Len( value , strlen( value ) ) ;
		char * p_temp = ( char * )AVMem_malloc( (iLen << 2 ) + 2 ) ;
		if( NULL != p_temp )
		{
			iLen =  utf8_2_gb( p_temp  , (char *)value , strlen( value ) );
			p_temp[iLen] = '\0' ;
			d = (char * )AVMem_malloc( iLen + 1 ) ;
			if( d )
				strncpy( d , p_temp , iLen + 1 ) ;
			AVMem_free( p_temp ) ;
		}		
	}
	return d;
}	
*/
void KanKan_VodRelease( KANKAN_PROGRAME ** programe )
{
	if( !(*programe) )
		return ;
	if( !(*programe)->p_vod )
	{
		_AVMem_free( (*programe) ) ;
		return ;
	}
        int i = 0;
        for( i = 0 ; i < (*programe)->vod_count ; i++ )
	{
		_AVMem_free( (*programe)->p_vod[i].cid ) ;
		_AVMem_free( (*programe)->p_vod[i].g_cid ) ;
		_AVMem_free( (*programe)->p_vod[i].name ) ;
		_AVMem_free( (*programe)->p_vod[i].format ) ;
	}
	_AVMem_free( (*programe)->p_vod ) ;
	_AVMem_free( (*programe) ) ;
}

static void KanKan_ReleasePic( MEM_PIC_INFO * p_mem_pic )
{
	if( !p_mem_pic )
		return ;
	MEM_PIC_INFO * p_temp_pic = p_mem_pic ;
	MEM_PIC_INFO * p_next_pic = NULL ;
	while( p_temp_pic)
	{
		p_next_pic = p_temp_pic->p_next_pic ;
		_AVMem_free( p_temp_pic->pic_buff ) ;
		_AVMem_free( p_temp_pic ) ;
		p_temp_pic = p_next_pic ;
	}
	
}

void KanKan_ReleaseXmlCtx(dl_rcmd_xml_ctx_type *ctx)
{
	int i;
	
	if(ctx)
	{
		if(ctx->title)
			_AVMem_free(ctx->title);
		
		if(ctx->nodes)
		{
			for(i=0; i<ctx->nodes_count; i++)
			{
				_AVMem_free( ctx->nodes[i].format ) ;
				_AVMem_free(ctx->nodes[i].name);
				_AVMem_free(ctx->nodes[i].mv_info);
				_AVMem_free(ctx->nodes[i].desc);
				_AVMem_free(ctx->nodes[i].pic);
				_AVMem_free(ctx->nodes[i].torrent);
				_AVMem_free(ctx->nodes[i].xml);
				_AVMem_free( ctx->nodes[i].c_size ) ;
				if( ctx->nodes[i].mem_pic )
				{
					_AVMem_free( ctx->nodes[i].mem_pic->pic_buff ) ;
					_AVMem_free( ctx->nodes[i].mem_pic ) ;
				}
				//KanKan_ReleasePic( ctx->nodes[i].mem_pic ) ;
				//ctx->nodes[i].mem_pic = NULL ;
				//KanKan_VodRelease( &(ctx->nodes[i].kankan_vod ) ) ;
			}

			_AVMem_free(ctx->nodes);
		}

		_AVMem_free(ctx);
	}
}


int KanKan_ParseCH(char *xml, dl_rcmd_xml_ctx_type ** ctx)
{
	struct stat st = {0};
	int fd;
	char *buf= NULL ;
	int ret;

	IXML_Document* doc = NULL ;
	IXML_Node* node = NULL ;
	IXML_Node* node2 = NULL ;
	IXML_Node* node3 = NULL ;
	IXML_Node* node4 = NULL ;
	IXML_NamedNodeMap* attrs = NULL ;
	int n = 0 ;
	
	DOMString name = NULL ;
	DOMString value = NULL ;
	INT8U b_mem_buf = 0 ;

	if(  0 == strncmp( xml , LOCAL_FILE_DEV_ROOT , strlen( LOCAL_FILE_DEV_ROOT ) ) )
	{
		//read file to buffer
		if(stat(xml, &st) < 0 || st.st_size < 0)
			return DLXML_ERROR_FILE_NOT_EXIST;

		if( (buf = (char*)DLUI_CALLOC(st.st_size+2, 1)) == NULL)
			return DLXML_ERROR_NO_MEMORY;

		if((fd = open(xml, O_RDONLY)) < 0)
		{
			DLUI_FREE(buf);
			return DLXML_ERROR_OPEN_FAILED;
		}

		int iReadLen = 0 ;
		while( iReadLen < st.st_size )
		{
			int iRead =  read(fd , buf + iReadLen , st.st_size - iReadLen) ;
			if( 0 ==  buf[0] && 0 == buf[27])
			{
				lseek( fd , 0 , SEEK_SET) ;
				iReadLen = 0 ;
				continue ;
			}

			if( iRead > 0 )
			{
				iReadLen += iRead ;
			}
			else
			{
				break ;
			}
		}
		
		if( iReadLen != st.st_size )
		{
			DLUI_FREE(buf);
			close(fd);
			return DLXML_ERROR_READ_FAILED;
		}
		buf[iReadLen] = '\0' ;
		close(fd);
		b_mem_buf = TRUE ;
	}
	else
		buf = xml ;
	ret = ixmlParseBufferEx(buf, &doc);
	//DLUI_FREE(buf);

	//start parse xml file
	if(ret != IXML_SUCCESS)
	{
		if( b_mem_buf)
			DLUI_FREE(buf);
		return ABXERR_MOAP_PARSE_XML_ERR;
	}
	//movielist
	if( (node = ixmlNode_getFirstChild(&doc->n)) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_CHILD;
	}
	
	if( (name = ixmlNode_getNodeName(node)) == NULL )
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_MOVIELIST;
	}

	if( strcmp(MOVIELIST, name))
	{
		if( strcmp( MOVIESETS , name ) )
		{
			ixmlDocument_free(doc);
			if( b_mem_buf)
				DLUI_FREE(buf);
			return DLXML_ERROR_NO_MOVIELIST;
		}
	}	
	
	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_MOVIELIST_NO_CHILD;
	}

	if( (*ctx = (dl_rcmd_xml_ctx_type*)DLUI_CALLOC(1, sizeof(dl_rcmd_xml_ctx_type))) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_MEMORY_RCMD;
	}

	memset( *ctx , 0x00 , sizeof(dl_rcmd_xml_ctx_type) ) ;
	
	do
	{
		name = ixmlNode_getNodeName(node);
		//parent
		if(name && strcmp(PARENT, name) == 0)
		{
			if( NULL != ( attrs = ixmlNode_getAttributes(node) ) )
			{
				int iCount = 0 ;
				int iChannel_count = 0 ;
				//id
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, ID)))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)
							(*ctx)->parent_id = n;
					}
				}
				//title
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, TITLE)))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						(*ctx)->title = DLUIUtils_strdup(value);
					}
				}
				//count
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, COUNT)))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)
						{
						 	n += 1 ;// for kankan search.
							if( NULL != ((*ctx)->nodes = (dl_rcmd_xml_movie_node_type*)DLUI_CALLOC(n, sizeof(dl_rcmd_xml_movie_node_type))))
							{
								(*ctx)->nodes_count = n;
								memset( (*ctx)->nodes ,  0x00 , n *  sizeof(dl_rcmd_xml_movie_node_type) ) ;
							}
							else
							{
								ixmlDocument_free(doc);
								if( b_mem_buf)
									DLUI_FREE(buf);
								return DLXML_ERROR_NO_MEMORY_RCMD ;
							}
						}
					}
				}
				ixmlNamedNodeMap_free(attrs);
			}
		}
		//moviesets
		else if(name && strcmp(MOVIESETS, name) == 0)
		{
			int node_idx = 0 ;
			node2 = ixmlNode_getFirstChild(node);
			if( 0 == (*ctx)->channel_count )
				(*ctx)->channel_count = (*ctx)->nodes_count ;

			for(node_idx=0; node2 && node_idx+1<(*ctx)->nodes_count; node_idx++)
			{
				name = ixmlNode_getNodeName(node2);
				if(name && strcmp(MOVIE, name) == 0)
				{
					if( NULL != ( attrs = ixmlNode_getAttributes(node2)))
					{
						//id
						if( NULL != ( node3 = ixmlNamedNodeMap_getNamedItem(attrs, ID )))
						{
							if( NULL != (value = ixmlNode_getNodeValue(node3)))
							{
								if(sscanf(value, "%d", &n) == 1)
								{
									(*ctx)->nodes[node_idx].id = n;
								}
							}
						}
						//is channel
						if( NULL != ( node3 = ixmlNamedNodeMap_getNamedItem(attrs, ISCHANNEL)))
						{
							if( NULL != (value = ixmlNode_getNodeValue(node3)))
							{
								if(sscanf(value, "%d", &n) == 1)
								{
									(*ctx)->nodes[node_idx].is_channel = n;
									if( 0 == (*ctx)->nodes[node_idx].is_channel )
										(*ctx)->nodes[node_idx].iDownloadProtocol = BT_DL_PROTOCOL;
								}

							}
						}
						//size
						if( NULL != ( node3 = ixmlNamedNodeMap_getNamedItem(attrs, SIZE)))
						{
							if( NULL != ( value = ixmlNode_getNodeValue(node3)))
							{
								if(sscanf(value, "%d", &n) == 1)
								{
									(*ctx)->nodes[node_idx].size = n;
								}
							}
						}
						ixmlNamedNodeMap_free(attrs);
					}
					
					node3 = ixmlNode_getFirstChild(node2);
					while(node3)
					{
						name = ixmlNode_getNodeName(node3);
						//name
						if(name && !strcmp(NAME, name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].name = DLUIUtils_strdup(value);
									(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_KANKAN_PROTOCOL ;
								}
							}
						}
						//child xml
						else if(name && 0 == strcmp(URL, name))
						{
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].xml = DLUIUtils_strdup(value);
								}
							}
						}
						//pic
						else if(name && 0 == strcmp(PIC, name))
						{
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].pic = DLUIUtils_strdup(value);
								}
							}
						}
						node3 = ixmlNode_getNextSibling(node3);
					}
				}
				node2 = ixmlNode_getNextSibling(node2);
			}
			(*ctx)->nodes[node_idx].id = 10 ;
			(*ctx)->nodes[node_idx].name = DLUIUtils_strdup(SEARCH_STRING);
			(*ctx)->nodes[node_idx].xml = DLUIUtils_strdup( "http://dy.gougou.com/top_movie?vdkey=%s&num=12" ) ;
			node_idx += 1 ;
			(*ctx)->nodes_count = node_idx ;
		}
	}while( NULL != (node = ixmlNode_getNextSibling(node)));
	
	ixmlDocument_free(doc);
	if( b_mem_buf)
		DLUI_FREE(buf);
	if( 0 == (*ctx)->nodes_count  )
	{
		KanKan_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
		return 0;
}

//if bIPTV is 1 ,result only include iptv right is 1,else include all iptv right 1 and 0
//if bCopyright is 1,result only include copyright is 1,else include all copyright 1 and 0
int KanKan_ParseMovies(char *xml, dl_rcmd_xml_ctx_type ** ctx,char bIPTV,char bCopyright, char bSearch)
{
	struct stat st = {0};
	int fd = -1 ;
	char *buf = NULL ;
	int ret ;

	IXML_Document* doc = NULL ;
	IXML_Node* node = NULL ;
	IXML_Node* node2 = NULL ;
//	IXML_Node* node3;
	IXML_Node* node4 = NULL ;
//	IXML_NamedNodeMap* attrs;
	int n = 0 ;
	int node_idx = 0;
	
	DOMString name = NULL ;
	DOMString value = NULL ;
	INT8U b_mem_buf = 0 ;
	int bIncludeFilm=1;

	if(  0 == strncmp( xml , LOCAL_FILE_DEV_ROOT , strlen( LOCAL_FILE_DEV_ROOT ) ) )
	{
		//read file to buffer
		if(stat(xml, &st) < 0 || st.st_size < 0)
			return DLXML_ERROR_FILE_NOT_EXIST;

		if( (buf = (char*)DLUI_CALLOC(st.st_size+2, 1)) == NULL)
			return DLXML_ERROR_NO_MEMORY;

		if((fd = open(xml, O_RDONLY)) < 0)
		{
			DLUI_FREE(buf);
			return DLXML_ERROR_OPEN_FAILED;
		}

		int iReadLen = 0 ;
		while( iReadLen < st.st_size )
		{
			int iRead =  read(fd , buf + iReadLen , st.st_size - iReadLen) ;
			if( 0 ==  buf[0] && 0 == buf[27])
			{
				lseek( fd , 0 , SEEK_SET) ;
				iReadLen = 0 ;
				continue ;
			}

			if( iRead > 0 )
			{
				iReadLen += iRead ;
			}
			else
			{
				break ;
			}
		}
		
		if( iReadLen != st.st_size )
		{
			DLUI_FREE(buf);
			close(fd);
			return DLXML_ERROR_READ_FAILED;
		}
		buf[iReadLen] = '\0' ;
		close(fd);
		b_mem_buf = TRUE ;
	}
	else
		buf = xml ;
	ret = ixmlParseBufferEx(buf, &doc);
	//DLUI_FREE(buf);

	//start parse xml file
	if(ret != IXML_SUCCESS)
	{
		if( b_mem_buf)
			DLUI_FREE(buf);
		return ABXERR_MOAP_PARSE_XML_ERR;
	}
	//movielist
	if( (node = ixmlNode_getFirstChild(&doc->n)) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_CHILD;
	}
	
	if( (name = ixmlNode_getNodeName(node)) == NULL )
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_MOVIELIST;
	}

	if( strcmp("SearchResult", name) == 0)
	{
		if( (node = ixmlNode_getFirstChild(node)) == NULL ||
			(node = ixmlNode_getNextSibling(node)) == NULL ||
			(name = ixmlNode_getNodeName(node)) == NULL ) 
		{
			ixmlDocument_free(doc);
			if( b_mem_buf)
				DLUI_FREE(buf);
			return DLXML_ERROR_NO_MOVIELIST;
		}
	}
	
	if( strcmp(FILMDATA, name))
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_MOVIELIST;
	}	
	
	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_MOVIELIST_NO_CHILD;
	}

	if( (*ctx = (dl_rcmd_xml_ctx_type*)DLUI_CALLOC(1, sizeof(dl_rcmd_xml_ctx_type))) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_MEMORY_RCMD;
	}
	memset( *ctx , 0x00 , sizeof(dl_rcmd_xml_ctx_type) ) ;
	
	do
	{
		name = ixmlNode_getNodeName(node);
/*		//Filmcount
		if(name && ( strcmp("total", name) == 0 || strcmp(FILMCOUNT, name) == 0 ) )
		{
			if( NULL != (node4 = ixmlNode_getFirstChild(node)))
			{
				if( NULL != ( value = ixmlNode_getNodeValue(node4)))
				{
					if(sscanf(value, "%d", &n) == 1)
					{
						if( NULL != ((*ctx)->nodes = (dl_rcmd_xml_movie_node_type*)DLUI_CALLOC(n, sizeof(dl_rcmd_xml_movie_node_type))))
						{
							(*ctx)->nodes_count = n;
							memset( (*ctx)->nodes ,  0x00 , n *  sizeof(dl_rcmd_xml_movie_node_type) ) ;
						}
						else
						{
							ixmlDocument_free(doc);
							if( b_mem_buf)
								DLUI_FREE(buf);
							return DLXML_ERROR_NO_MEMORY_RCMD ;
						}
					}
				}
			}
		}
		//Film
		else */if(name && strcmp(FILM, name) == 0)
		{
			bIncludeFilm=1;
			node2 = ixmlNode_getFirstChild(node);
			if( (*ctx)->nodes_count <= node_idx )
			{
				dl_rcmd_xml_movie_node_type *nodes;
				if( NULL != (nodes = (dl_rcmd_xml_movie_node_type*)DLUI_REALLOC((*ctx)->nodes,(node_idx+1)*sizeof(dl_rcmd_xml_movie_node_type))))
				{
					(*ctx)->nodes = nodes;
					(*ctx)->nodes_count = node_idx+1;
					memset( &(*ctx)->nodes[node_idx] ,  0x00 , sizeof(dl_rcmd_xml_movie_node_type) ) ;
				}
				else
				{
					KanKan_ReleaseXmlCtx(*ctx);
					(*ctx)->nodes = NULL;
					(*ctx)->nodes_count = 0;
					ixmlDocument_free(doc);
					if( b_mem_buf)
						DLUI_FREE(buf);
					return DLXML_ERROR_NO_MEMORY_RCMD ;
				}
//				break ;
			}
//			if( 0 == (*ctx)->channel_count )
				(*ctx)->channel_count = (*ctx)->nodes_count ;
			for(; node2/* && node_idx<(*ctx)->nodes_count*/; )
			{
				name = ixmlNode_getNodeName(node2);
				//Id
				if(name && !strcmp("Id", name ) )
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							(*ctx)->nodes[node_idx].id = atoi(value);
							//(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_KANKAN_PROTOCOL ;
						}
					}
				}
				else if(name && !strcmp("Title", name ) )
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							(*ctx)->nodes[node_idx].name = DLUIUtils_strdup(value);
							//(*ctx)->nodes[node_idx].name = getvalue_from_UTF8(value);
						}
					}
				}
				//director
				else if(name && ( 0 == strcmp( "director" , name ) || strcmp(DIRECTOR, name) == 0) )
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "导演: " ,  value , NULL  ) ;
						}
					}
				}
				//actor
				else if(name && ( 0 == strcmp( "actor" , name ) || strcmp(ACTOR, name) == 0))
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "主演: " ,  value , NULL ) ;
						}
					}
				}				
				else if(name && strcmp(CLASS, name) == 0)
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							//char * p_temp = getvalue_from_UTF8( value ) ;
							char * p_temp = DLUIUtils_strdup( value ) ;
							if( strstr( p_temp , "电影" ) )
								(*ctx)->nodes[node_idx].is_channel = CH_MOVIE ;
							else if( strstr( p_temp , "电视剧" ) )
								(*ctx)->nodes[node_idx].is_channel = CH_TELEPLAY ;								
							else if( strstr( p_temp , "动漫" ) )
								(*ctx)->nodes[node_idx].is_channel = CH_CARTON ;								
							else if( strstr( p_temp , "综艺" ) )
								(*ctx)->nodes[node_idx].is_channel = CH_TV_SHOW ;								
							_AVMem_free( p_temp ) ;
						}
					}
				}					//languange
				else if(name && 0 == strcmp( AREA , name))
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "语言: " ,  value , NULL ) ;
						}
					}
				}
				//format 
				else if(name && 0 == strcmp( "Format" , name ))
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
#if 1	//only get rm format					
							if((*value!='R' && *value!='r' )||(*(value+1)!='m' && *(value+1)!='M'))//filter on rmvb
								bIncludeFilm=0;
#endif							
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "格式: " ,  value  , NULL ) ;
						}
					}
				}				
				//pub time 
				else if(name && 0 == strcmp( PUBTIME , name ))
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "发行时间: " ,  value  , NULL ) ;
						}
					}
				}
				//play time 
				else if(name && strcmp( LONGTIME , name) == 0)
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							char minute[20];
							(*ctx)->nodes[node_idx].len = atoi(value);
							if(bSearch)
								(*ctx)->nodes[node_idx].len /= 60;
							sprintf(minute, "%d", (*ctx)->nodes[node_idx].len);
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "时长: " ,  minute  , "分钟") ;
						}
					}
				}
				else if(name && (strcmp(PIC, name) == 0 || strcmp(PIC2, name) == 0))
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != (value = ixmlNode_getNodeValue(node4)))
						{
							(*ctx)->nodes[node_idx].pic = DLUIUtils_strdup(value);
						}
					}				
				}
				else if(name && (strcmp(DESCRIPTI, name) == 0 ||  strcmp(DESCRIPTI2, name) == 0))
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != ( value = ixmlNode_getNodeValue(node4)))
						{
							//(*ctx)->nodes[node_idx].desc = DLUIUtils_strdup(value);
						}
					}
				}
				//child xml				
				else if(name && ( 0 == strcmp( "url", name ) || 0 == strcmp(LINKURL, name) || 0 == strcmp(LINKURL2, name)))
				{
					if( NULL != ( node4 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != (value = ixmlNode_getNodeValue(node4)))
						{
							(*ctx)->nodes[node_idx].xml = DLUIUtils_strdup(value);
						}
					}

				}
				else if(1==bIPTV && name && (strcmp("IPTV", name) == 0 || strcmp("iptv", name) == 0))
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != (value = ixmlNode_getNodeValue(node4)))
						{
							if('0'==*value)
								bIncludeFilm=0;
						}
					}				
				}
				else if(1==bCopyright&& name && (strcmp("CopyRight", name) == 0 || strcmp("Copyright", name) == 0))
				{
					if( NULL != (node4 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != (value = ixmlNode_getNodeValue(node4)))
						{
							if('0'==*value)
								bIncludeFilm=0;
						}
					}				
				}

				node2 = ixmlNode_getNextSibling(node2);
			}
			if(bIncludeFilm)
 					node_idx++;
 			else
				{
				dl_rcmd_xml_movie_node_type * p=&(*ctx)->nodes[node_idx];
				_AVMem_free(p->format ) ;
				_AVMem_free(p->name);
				_AVMem_free(p->mv_info);
				_AVMem_free(p->desc);
				_AVMem_free(p->pic);
				_AVMem_free(p->torrent);
				_AVMem_free(p->xml);
				_AVMem_free(p->c_size ) ;
				}
		}
	}while( NULL != (node = ixmlNode_getNextSibling(node)));

	(*ctx)->nodes_count=node_idx;
	
	ixmlDocument_free(doc);
	if( b_mem_buf)
		DLUI_FREE(buf);
	if( 0 == (*ctx)->nodes_count  )
	{
		KanKan_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
	{
		(*ctx)->nodes_count = node_idx ;
		return 0;
	}	
}

static char * d1="，";
static char * d2=",";

void ParseCID_GCID(char * value,VODFiles * pVodFile)
{
   char * d,tmp;
   int d_size;
   char * p=strstr(value,d1);
   if(p)
   	{
   	d=d1;
	d_size=strlen(d1);
   	}
   else 
   	{
   	p=strstr(value,d2);
	if(p)
	{
	d=d2;
	d_size=strlen(d2);
	}
	else
		return;
   	}
   
  if(p)
  {
  	tmp=*p;
  	*p=0;
	pVodFile->cid=DLUIUtils_strdup(value);
	*p=tmp;
	value=p+d_size;
  }
  else
  	return;

  p=strstr(value,d);
  if(p)
  {
  	tmp=*p;
  	*p=0;
	pVodFile->g_cid=DLUIUtils_strdup(value);
	*p=tmp;
	value=p+d_size;
  }
  else
  	return;

  pVodFile->size=atoi(value);
}

int GetVodCount(IXML_Node * node,char * name)
{
	int i=0;
	for(; node && !strcmp( "Url", name );)
	{
	node = ixmlNode_getNextSibling(node);
	i++;
	}
	return i;
}

int KanKan_ParseVodInfo(char *xml, KANKAN_PROGRAME ** vod_info,char bIPTV,char bCopyright )
{
	struct stat st = {0};
	int fd = -1 ;
	char *buf = NULL ;
	int ret = 0 ;
	CHANNEL_TYPE channe_type = CH_NONE ;
	char * p_vod_name = NULL ;
	IXML_Document* doc = NULL ;
	IXML_Node* node = NULL ;
	IXML_Node* node2 = NULL ;
	IXML_Node* node3 = NULL ;
	IXML_Node* node4 = NULL ;
	//IXML_NamedNodeMap* attrs;
	//int n;
	int node_idx = 0;
	
	DOMString name = NULL ;
	DOMString value = NULL ;
	INT8U b_mem_buf = 0 ;
	
	if(  0 == strncmp( xml , LOCAL_FILE_DEV_ROOT , strlen( LOCAL_FILE_DEV_ROOT ) ) )
	{
		//read file to buffer
		if(stat(xml, &st) < 0 || st.st_size < 0)
			return DLXML_ERROR_FILE_NOT_EXIST;

		if( (buf = (char*)DLUI_CALLOC(st.st_size+2, 1)) == NULL)
			return DLXML_ERROR_NO_MEMORY;

		if((fd = open(xml, O_RDONLY)) < 0)
		{
			DLUI_FREE(buf);
			return DLXML_ERROR_OPEN_FAILED;
		}

		int iReadLen = 0 ;
		while( iReadLen < st.st_size )
		{
			int iRead =  read(fd , buf + iReadLen , st.st_size - iReadLen) ;
			if( 0 ==  buf[0] && 0 == buf[27])
			{
				lseek( fd , 0 , SEEK_SET) ;
				iReadLen = 0 ;
				continue ;
			}

			if( iRead > 0 )
			{
				iReadLen += iRead ;
			}
			else
			{
				break ;
			}
		}
		
		if( iReadLen != st.st_size )
		{
			DLUI_FREE(buf);
			close(fd);
			return DLXML_ERROR_READ_FAILED;
		}
		buf[iReadLen] = '\0' ;
		close(fd);
		b_mem_buf = TRUE ;
	}
	else
		buf = xml ;
	ret = ixmlParseBufferEx(buf, &doc);
	//DLUI_FREE(buf);

	//start parse xml file
	if(ret != IXML_SUCCESS)
	{
		if( b_mem_buf)
			DLUI_FREE(buf);
		return ABXERR_MOAP_PARSE_XML_ERR;
	}
	//movielist
	if( (node = ixmlNode_getFirstChild(&doc->n)) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_CHILD;
	}
	
	if( (name = ixmlNode_getNodeName(node)) == NULL )
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_MOVIELIST;
	}

	if( strcmp(FILMDATA, name))
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_NO_MOVIELIST;
	}	
	
	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_MOVIELIST_NO_CHILD;
	}
	
	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		if( b_mem_buf)
			DLUI_FREE(buf);
		return DLXML_ERROR_MOVIELIST_NO_CHILD;
	}

	do
	{
		name = ixmlNode_getNodeName(node);
		//Film
		if(name && 0 == strcmp( UPDATE , name ) )
		{
			
		}
		if( name && 0 == strcmp( "Title" , name ) )
		{
			if( NULL != (node4 = ixmlNode_getFirstChild(node)))
			{
				if( NULL != ( value = ixmlNode_getNodeValue(node4)))
				{
					//p_vod_name = getvalue_from_UTF8(value);		
					p_vod_name = DLUIUtils_strdup(value);					
				}
			}
		}
		if( name && 0 == strcmp( CLASS , name ) )
		{
			if( NULL != (node4 = ixmlNode_getFirstChild(node)))
			{
				if(NULL != ( value = ixmlNode_getNodeValue(node4)))
				{
					//char * p_temp = getvalue_from_UTF8( value ) ;
					char * p_temp = DLUIUtils_strdup( value ) ;
					if( strstr( p_temp , "电影" ) )
						channe_type = CH_MOVIE ;
					else if( strstr( p_temp , "电视剧" ) )
						channe_type = CH_TELEPLAY ;								
					else if( strstr( p_temp , "动漫" ) )
						channe_type = CH_CARTON ;								
					else if( strstr( p_temp , "综艺" ) )
						channe_type = CH_TV_SHOW ;
					_AVMem_free( p_temp ) ;
				}
			}
		}
		else if(1==bIPTV && name && (strcmp("IPTV", name) == 0 || strcmp("iptv", name) == 0))
		{
					if( NULL != (node4 = ixmlNode_getFirstChild(node)))
					{
						if( NULL != (value = ixmlNode_getNodeValue(node4)))
						{
							if('0'==*value)
							{
								_AVMem_free(p_vod_name);
								ixmlDocument_free(doc);
								if( b_mem_buf)
									DLUI_FREE(buf);
								return DLXML_ERROR_NO_MOVIELIST;
							}
						}
					}				
		}
		else if(1==bCopyright&& name && (strcmp("CopyRight", name) == 0 || strcmp("Copyright", name) == 0))
		{
					if( NULL != (node4 = ixmlNode_getFirstChild(node)))
					{
						if( NULL != (value = ixmlNode_getNodeValue(node4)))
						{
							if('0'==*value)
							{
								_AVMem_free(p_vod_name);
								ixmlDocument_free(doc);
								if( b_mem_buf)
									DLUI_FREE(buf);
								return DLXML_ERROR_NO_MOVIELIST;
							}
						}
					}				
		}
		else if( name && strcmp( EPOSODES , name ) == 0 )
		{
			int i_vod_count = 0 ;
			if( NULL != (node4 = ixmlNode_getFirstChild(node)))
			{
				if( NULL != ( value = ixmlNode_getNodeValue(node4)))
				{
					i_vod_count = atoi(value);
				}
			}
			
			(*vod_info) = (KANKAN_PROGRAME *)DLUI_CALLOC( 1 , sizeof(KANKAN_PROGRAME) ) ;
			if( !(*vod_info) )
			{				
				ixmlDocument_free(doc);
				if( b_mem_buf)
					DLUI_FREE(buf);
				return DLXML_ERROR_NO_MEMORY_RCMD ;
			}
			memset( (*vod_info) , 0x00 , sizeof( KANKAN_PROGRAME ) ) ;
			(*vod_info)->vod_count = i_vod_count ;
			(*vod_info)->programe_type = channe_type ;

			(*vod_info)->p_vod = ( VODFiles * )DLUI_CALLOC( i_vod_count , sizeof( VODFiles ) ) ;
			if( !(*vod_info)->p_vod )
			{
				_AVMem_free( (*vod_info) ) ;
				ixmlDocument_free(doc);
				if( b_mem_buf)
					DLUI_FREE(buf);
				return DLXML_ERROR_NO_MEMORY_RCMD ;
			}
			
			memset( (*vod_info)->p_vod , 0x00 , i_vod_count * sizeof( VODFiles ) ) ;
		}

		
		else if(name && strcmp("Down", name) == 0)
		{
			int i_vod_count = 0 ;
			node2 = ixmlNode_getFirstChild(node);
			name = ixmlNode_getNodeName(node2);
			i_vod_count= GetVodCount(node2,"Url");
			(*vod_info) = (KANKAN_PROGRAME *)DLUI_CALLOC( 1 , sizeof(KANKAN_PROGRAME) ) ;
			if( !(*vod_info) )
			{				
				ixmlDocument_free(doc);
				if( b_mem_buf)
					DLUI_FREE(buf);
				return DLXML_ERROR_NO_MEMORY_RCMD ;
			}
			memset( (*vod_info) , 0x00 , sizeof( KANKAN_PROGRAME ) ) ;
			(*vod_info)->vod_count = i_vod_count ;
			(*vod_info)->programe_type = channe_type ;

			(*vod_info)->p_vod = ( VODFiles * )DLUI_CALLOC( i_vod_count , sizeof( VODFiles ) ) ;
			if( !(*vod_info)->p_vod )
			{
				_AVMem_free( (*vod_info) ) ;
				ixmlDocument_free(doc);
				if( b_mem_buf)
					DLUI_FREE(buf);
				return DLXML_ERROR_NO_MEMORY_RCMD ;
			}
			
			memset( (*vod_info)->p_vod , 0x00 , i_vod_count * sizeof( VODFiles ) ) ;

			 
			for( ; node2 && !strcmp( "Url", name ) &&  node_idx<(*vod_info)->vod_count ; )
			{
				node3 = ixmlNode_getFirstChild(node2);

				for(; node3/* && node_idx<(*vod_info)->vod_count*/; )
				{
					name = ixmlNode_getNodeName(node3);
					//Subid
					if(name && !strcmp("ContentIndex", name ) )
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if( NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								(*vod_info)->p_vod[node_idx].sub_id = atoi(value);
							}
						}
					}
					else if(name &&!strcmp("DownUrl",name))
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if( NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								ParseCID_GCID(value,&((*vod_info)->p_vod[node_idx]));
							}
						}
					}
					else if(name && !strcmp(CID, name ) )
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if( NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								(*vod_info)->p_vod[node_idx].cid = DLUIUtils_strdup(value);
							}
						}
					}
					else if(name && strcmp(GCID, name) == 0)
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if(NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								(*vod_info)->p_vod[node_idx].g_cid = DLUIUtils_strdup(value);
							}
						}
					}
					else if(name && strcmp("Size", name) == 0)
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if(NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								char c_vod_size[24] ;
								memset( c_vod_size , 0x00 , sizeof( c_vod_size ) ) ;
								memcpy( c_vod_size , value , strlen( value ) - 1 ) ;
								(*vod_info)->p_vod[node_idx].size = atoi( c_vod_size ) ;
							}
						}
					}
					else if( name && strcmp( "PartIndex" , name) == 0 )
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if(NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								int i_vodsequence = atoi( value ) ;
								(*vod_info)->p_vod[node_idx].be_new_part = 1 ;
								(*vod_info)->part_count ++ ;
/*								if( 1 == i_vodsequence )
								{
									(*vod_info)->p_vod[node_idx].be_new_part = 1 ;
									(*vod_info)->part_count ++ ;
								}
								else
								{
									(*vod_info)->p_vod[node_idx].be_new_part = 0 ;
								}*/
							}
						}					
					}
					else if(name && strcmp("ContentName", name) == 0)
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if(NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								char * p_name_temp = DLUIUtils_strdup( value ) ;
/*								if( CH_TELEPLAY == channe_type || CH_CARTON == channe_type  || CH_TV_SHOW == channe_type)
								{
									if((*vod_info)->p_vod[node_idx].be_new_part == 0)
									{
										(*vod_info)->part_count ++ ;
										(*vod_info)->p_vod[node_idx].be_new_part = 1;
									}
								}
								else if( CH_MOVIE == channe_type )
								{
									if( 0 == node_idx )
									{
										(*vod_info)->part_count = 1 ;
										(*vod_info)->p_vod[node_idx].be_new_part = 1 ;
									}
									else 
										(*vod_info)->p_vod[node_idx].be_new_part = 0 ;
								}*/
								char c_temp_buff[256] = {0};
								snprintf( c_temp_buff , sizeof( c_temp_buff ) , "%s_%s" , p_vod_name , p_name_temp ) ;
								(*vod_info)->p_vod[node_idx].name = DLUIUtils_strdup( c_temp_buff ) ;
								_AVMem_free( p_name_temp ) ;
							}
						}
					}
					//VODformat
					else if(name && 0 == strcmp( "Format" , name))
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if(NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								(*vod_info)->p_vod[node_idx].format = DLUIUtils_strdup(value);
							}
						}
					}
					//play time 
					else if(name && strcmp( "LongTime" , name) == 0)
					{
						if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
						{
							if(NULL != ( value = ixmlNode_getNodeValue(node4)))
							{
								//(*vod_info)->p_vod[node_idx].play_time = atoi( value )/60 ;
								(*vod_info)->p_vod[node_idx].play_time = DLUIUtils_strdup(value);
							}
						}
					}
					node3 = ixmlNode_getNextSibling(node3);
				}
				
				node2 = ixmlNode_getNextSibling(node2);
				if( node2 )
					name = ixmlNode_getNodeName(node2);
				node_idx++ ;
			}
			//(*ctx)->nodes_count = node_idx ;
		}
	}while( NULL != (node = ixmlNode_getNextSibling(node)));
	
	ixmlDocument_free(doc);
	_AVMem_free( p_vod_name ) ;
	if( b_mem_buf)
		DLUI_FREE(buf);
	/*if( 0 == (*ctx)->nodes_count  )
	{
		KanKan_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
	*/
	if( (*vod_info) )
		(*vod_info)->vod_count = node_idx ;
	return 0;
}

