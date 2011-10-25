
#include <ixml.h>
#include <xml_util.h>
#include "aw_windows.h"
#include "ioapi.h"
#include "navigate_parse.h"
#include "net/ABoxBase/abx_error.h"
#include <AVmalloc.h>


#define		DLUI_MALLOC		AVMem_malloc
#define		DLUI_CALLOC		AVMem_calloc
#define		DLUI_REALLOC	AVMem_realloc
#define		DLUI_FREE		AVMem_free
#define 	_AVMem_free(p)  do{ if( p ){AVMem_free(p);p=NULL;}}while(0)
#define 	LOCAL_FILE_DEV_ROOT  "/mnt/"

static char* DLUIUtils_strdup(const char *s)
{
	char *d = NULL;
	int len;
	if(s)
	{
		len = strlen(s);
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
		
		if( NULL != ( d = (char * )DLUI_REALLOC( cRes , ilen + 1 ) ) )
		{
			if( cUnit )
				sprintf( d + iResLen , "%s%s %s\n" ,  cTitle , cAddstr , cUnit ) ;
			else
				sprintf( d +  iResLen , "%s%s\n" ,  cTitle , cAddstr ) ;
		}
	}
	return d ;
}

static float get_downdload_file_size( char * c_file_size )
{
	int i_strlen = strlen( c_file_size ) ;
        int i = 0;
        for( i = i_strlen - 1 ; i > 0 ; i-- )
	{
		if( c_file_size[i] >= 0x30 && c_file_size[i] <= 0x39 )
			break ;
	}
	
	char c_size[12] = {0} ;
	memcpy( c_size , c_file_size , i + 1 ) ;
	float f_size = atof( c_size ) ;
	int i_until_pos = i + 1 ;
	if( 0 == strcmp( c_file_size + i_until_pos , "GB" ) || 
		0 == strcmp( c_file_size + i_until_pos , "G" ))
	{
		f_size = f_size * (1 << 30) ;
	}
	else if( 0 == strcmp( c_file_size + i_until_pos , "MB" ) ||
		0 == strcmp( c_file_size + i_until_pos , "M" ))
	{
		f_size = f_size * (1 << 20) ;
	}
	else if( 0 == strcmp( c_file_size + i_until_pos , "KB" ) ||
		0 == strcmp( c_file_size + i_until_pos , "K" ))
	{
		f_size = f_size * (1 << 10) ;
	}
	return f_size ;
}

void DLRcmd_ReleaseXmlCtx(dl_rcmd_xml_ctx_type *ctx)
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
			}

			_AVMem_free(ctx->nodes);
		}

		_AVMem_free(ctx);
	}
}


#define MOVIELIST		"movielist"

#define PARENT		"parent"
#define ID			"id"
#define BEGIN_POS	"begin_pos"
#define COUNT		"count"
#define TOTAL		"total"

#define TITLE			"title"
#define MOVIESETS	"moviesets"

#define MOVIE		"movie"
#define ISCHANNEL	"ischannel"
#define DRM			"drm"
#define SIZE			"size"

#define NAME			"name"
#define MOVIEINFO	"movieinfo"
#define DESC			"desc"
#define PIC			"pic"
#define TORRENT		"torrent"

#define CHILD			"child"	//next level xml file name
#define SOURCE      	"source"
#define ENCRY        		"encry_url"
#define PROTOCOL         "protocol"

#define DIRECTOR 		"director"
#define ACTOR		"actor"
#define AREA			"area"
#define LANGUANGE	"languange"
#define PUBTIME		"pubTime"
#define PLAYTIME		"playTime"

#define MOVIE		"movie"

//return 0 : success
int DLRcmd_ParseXmlCtx( char *xml, dl_rcmd_xml_ctx_type ** ctx)
{
	struct stat st = {0};
	int fd;
	char *buf;
	int ret;

	IXML_Document* doc;
	IXML_Node* node;
	IXML_Node* node2;
	IXML_Node* node3;
	IXML_Node* node4;
	IXML_NamedNodeMap* attrs;
	int n;
	
	DOMString name;
	DOMString value;
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
				if( NULL != (node2 = ixmlNamedNodeMap_getNamedItem(attrs , BEGIN_POS )))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)
							(*ctx)->begin_pos = n;
					}
				}
				//count
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, COUNT)))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)
						{
							iCount = n ;
							/*if( NULL != ((*ctx)->nodes = (dl_rcmd_xml_movie_node_type*)DLUI_CALLOC(n, sizeof(dl_rcmd_xml_movie_node_type))))
							{
								(*ctx)->nodes_count = n;
								memset( (*ctx)->nodes ,  0x00 , n *  sizeof(dl_rcmd_xml_movie_node_type) ) ;
							}*/
						}
					}
				}
				//total
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, TOTAL)))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)
						{
							/*if( NULL != ((*ctx)->nodes = (dl_rcmd_xml_movie_node_type*)DLUI_CALLOC(n, sizeof(dl_rcmd_xml_movie_node_type))))
							{
								(*ctx)->nodes_count = n;
								memset( (*ctx)->nodes ,  0x00 , n *  sizeof(dl_rcmd_xml_movie_node_type) ) ;
							}*/
							iChannel_count = n ;
							(*ctx)->channel_count = iChannel_count ;
						}
					}
				}
				else
				{
					iChannel_count = iCount ;
				}
				//title
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, TITLE)))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						(*ctx)->title = DLUIUtils_strdup(value);
					}
				}
				ixmlNamedNodeMap_free(attrs);


				if( 0 == (*ctx)->channel_count &&  0 != iCount )
					(*ctx)->channel_count = iCount ;
				
				if( (*ctx)->channel_count < iCount )
					iCount = (*ctx)->channel_count  ;
				else if( (*ctx)->begin_pos + iCount > (*ctx)->channel_count  )
					iCount = (*ctx)->channel_count - (*ctx)->begin_pos  ;

				n = iCount ;
				if( 0 == n )
				{
					ixmlDocument_free(doc);
					if( b_mem_buf)
						DLUI_FREE(buf);
					return ABXERR_MOAP_NOT_ANY_PROGRAM;
				}
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
		// for thunder navigate!
		else if( name && !(*ctx)->nodes && 0 == strcmp( "total" , name ) )
		{
			if(NULL != (node2 = ixmlNode_getFirstChild(node)))
			{
				if( NULL != (value = ixmlNode_getNodeValue(node2)))
				{
					if(sscanf(value, "%d", &n) == 1)
					{
						(*ctx)->nodes_count = n;
						if( NULL != ((*ctx)->nodes = (dl_rcmd_xml_movie_node_type*)DLUI_CALLOC(n, sizeof(dl_rcmd_xml_movie_node_type))))
						{
							memset( (*ctx)->nodes ,  0x00 , n *  sizeof(dl_rcmd_xml_movie_node_type) ) ;
						}
						else
						{
							ixmlDocument_free(doc);
							if( b_mem_buf)
								DLUI_FREE(buf);
							return DLXML_ERROR_NO_MEMORY_RCMD ;
						}
						node2 = ixmlNode_getNextSibling(node);
						name = ixmlNode_getNodeName(node2);
						if( 0 == strcmp( name , "all" ) )
						{
							node2 = ixmlNode_getNextSibling(node2);
						}
						(*ctx)->channel_count = 50 ;
						goto MOVIE_BEGIN ;
					}
				}
			}	
		}
		//moviesets
		else if(name && strcmp(MOVIESETS, name) == 0)
		{
			int node_idx;
			
			node2 = ixmlNode_getFirstChild(node);
			
MOVIE_BEGIN:
			if( 0 == (*ctx)->channel_count )
				(*ctx)->channel_count = (*ctx)->nodes_count ;

			for(node_idx=0; node2 && node_idx<(*ctx)->nodes_count; node_idx++)
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
						//drm
						if( NULL != ( node3 = ixmlNamedNodeMap_getNamedItem(attrs, DRM)))
						{
							if( NULL != ( value = ixmlNode_getNodeValue(node3)))
							{
								if(sscanf(value, "%d", &n) == 1)
								{
									(*ctx)->nodes[node_idx].iMovie_drm = n;
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
						if(name && ( !strcmp( "key", name) || !strcmp(NAME, name )) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].name = DLUIUtils_strdup(value);
									(*ctx)->nodes[node_idx].iDownloadProtocol = BT_DL_PROTOCOL ;
								}
							}
						}
						//movieinfo
						else if(name && strcmp(MOVIEINFO, name) == 0)
						{
							if(NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = DLUIUtils_strdup(value);
									
								}
							}
						}
						//desc
						else if(name && strcmp(DESC, name) == 0)
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									//open by zhenlei
									(*ctx)->nodes[node_idx].desc = DLUIUtils_strdup(value);
								}
							}
						}
						//pic
						else if(name && strcmp(PIC, name) == 0)
						{
							//encry
							if( NULL != ( attrs = ixmlNode_getAttributes(node3) ) )
							{
								if( NULL != ( node4 = ixmlNamedNodeMap_getNamedItem(attrs, ENCRY)))
								{
									if( NULL != ( value = ixmlNode_getNodeValue(node4)))
									{
										if(sscanf(value, "%d", &n) == 1)
										{
											(*ctx)->nodes[node_idx].iPic_encry = n;
										}
									}
								}
								ixmlNamedNodeMap_free(attrs);
							}
							
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].pic = DLUIUtils_strdup(value);
								}
							}
	
						}
						//torrent
						else if(name && ( 0 == strcmp( "furlbt" , name ) || strcmp(TORRENT, name) == 0 || 0 == strcmp( "torrent0" , name )))
						{
							//encry
							if( NULL != ( attrs = ixmlNode_getAttributes(node3) ) )
							{
								if( NULL != ( node4 = ixmlNamedNodeMap_getNamedItem(attrs, ENCRY)))
								{
									if( NULL != ( value = ixmlNode_getNodeValue(node4)))
									{
										if(sscanf(value, "%d", &n) == 1)
										{
											(*ctx)->nodes[node_idx].iTorr_encry = n;
										}
									}
								}
								ixmlNamedNodeMap_free(attrs);
							}
							
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									/*char * p_last = strrchr( value , '/' ) ;
									char * p_dot = strrchr( value , '.' ) ;
									if( p_last && p_dot )
									{
										char  c_cid[64] = {0} ;
										memcpy( c_cid , p_last + 1 , p_dot - p_last - 1 ) ;
										(*ctx)->nodes[node_idx].torrent = DLUIUtils_strdup(c_cid );
										(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_DL_PROTOCOL ;
										//(*ctx)->nodes[node_idx].iDownloadProtocol = BT_DL_PROTOCOL;
									}*/
									(*ctx)->nodes[node_idx].torrent = DLUIUtils_strdup(value);
								}
							}
						}
						/*else if( name && 0 == strcmp( "httpcid" , name ) ) 
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].torrent = DLUIUtils_strdup(value);
									(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_DL_PROTOCOL ;
								}
							}
						}*/
						//child xml
						else if(name && 0 == strcmp(CHILD, name))
						{
							//encry
							if( NULL != ( attrs = ixmlNode_getAttributes(node3) ) )
							{
								if( NULL != ( node4 = ixmlNamedNodeMap_getNamedItem(attrs, ENCRY)))
								{
									if( NULL != ( value = ixmlNode_getNodeValue(node4)))
									{
										if(sscanf(value, "%d", &n) == 1)
										{
											(*ctx)->nodes[node_idx].iXml_encry = n;
										}
									}
								}
								ixmlNamedNodeMap_free(attrs);
							}
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].xml = DLUIUtils_strdup(value);
								}
							}
	
						}
						//source 
						else if( name && strcmp( SOURCE , name ) == 0 )
						{
							//encry
							if( NULL != ( attrs = ixmlNode_getAttributes(node3) ) )
							{
								if( NULL != ( node4 = ixmlNamedNodeMap_getNamedItem(attrs, ENCRY)))
								{
									if( NULL != ( value = ixmlNode_getNodeValue(node4)))
									{
										if(sscanf(value, "%d", &n) == 1)
										{
											if(  1 == (*ctx)->nodes[node_idx].is_channel )
												(*ctx)->nodes[node_idx].iXml_encry = n;
											else
												(*ctx)->nodes[node_idx].iTorr_encry = n;
										}
									}
								}
								ixmlNamedNodeMap_free(attrs);
							}
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									if(  1 == (*ctx)->nodes[node_idx].is_channel )
										(*ctx)->nodes[node_idx].xml = DLUIUtils_strdup(value);
									else
										(*ctx)->nodes[node_idx].torrent = DLUIUtils_strdup(value);				
								}
							}

						}
						//protocol
						else if(name && strcmp(PROTOCOL, name) == 0)
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != (value = ixmlNode_getNodeValue(node4)))
								{									
									if(sscanf(value, "%d", &n) == 1)
									{
										(*ctx)->nodes[node_idx].iDownloadProtocol = n ;
									}
								}
							}
						}
						//director
						else if(name && strcmp(DIRECTOR, name) == 0)
					{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "导演: " ,  value , NULL  ) ;
								}
							}
						}
						//actor
						else if(name && strcmp(ACTOR, name) == 0)
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "主演: " ,  value , NULL ) ;
								}
							}
						}
						//area
						else if(name && strcmp( AREA , name) == 0)
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "发行地: " ,  value , NULL ) ;
								}
							}
						}
						//languange
						else if(name && ( 0 == strcmp( "lang" , name ) || strcmp( LANGUANGE , name) == 0) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "语言: " ,  value , NULL ) ;
								}
							}
						}
						//pub time 
						else if(name && ( 0 == strcmp( "year" , name ) || strcmp( PUBTIME, name) == 0 ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "发行时间: " ,  value  , NULL ) ;
								}
							}
						}
						//play time 
						else if(name && strcmp( PLAYTIME , name) == 0)
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "时长: " ,  value  , "分钟") ;
								}
							}
						}
						/*else if( name && 0 == strcmp( "runtime" , name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "时长: " ,  value  , "分钟" ) ;
								}
							}
						}*/
						else if( name && 0 == strcmp( "time0" , name ) )
						//else if( name && 0 == strcmp( "httptime" , name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									int i_min = atoi(value)/60 ;
									char c_min[10] ;
									snprintf( c_min , sizeof( c_min ) , "%s" , i_min ) ;
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "时长: " ,  c_min  , "分钟" ) ;
								}
							}
						}
						/*else if( name && 0 == strcmp( "fmtbt" , name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "格式: " ,  value  , NULL ) ;
									(*ctx)->nodes[node_idx].format = DLUIUtils_strdup(value);
								}
							}
						}*/
						else if( name && 0 == strcmp( "fmt0" , name ) )
						//else if( name && 0 == strcmp( "httpfmt" , name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "格式: " ,  value  , NULL ) ;
									(*ctx)->nodes[node_idx].format = DLUIUtils_strdup(value);
								}
							}
						}
						else if( name && 0 == strcmp( "size0" , name ) )
						//else if( name && 0 == strcmp( "httpsize" , name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "文件大小: " ,  value  , NULL ) ;
									(*ctx)->nodes[node_idx].size = get_downdload_file_size( value ) ;
								}
							}
						}
						node3 = ixmlNode_getNextSibling(node3);
					}
				}
				node2 = ixmlNode_getNextSibling(node2);
			}

			(*ctx)->nodes_count = node_idx ;
		}
	}while( NULL != (node = ixmlNode_getNextSibling(node)));
	
	ixmlDocument_free(doc);
	if( b_mem_buf)
		DLUI_FREE(buf);
	if( 0 == (*ctx)->nodes_count  )
	{
		DLRcmd_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
		return 0;
}

int parse_register_xml( char * c_xml  , char * p_key , char ** p_root_url )
{
	struct stat st = {0};
	int fd;
	char *buf ;
	int ret;

	IXML_Document* doc;
	IXML_Node* node;
	IXML_Node* node2;
	
	DOMString name;
	DOMString value;

	
	//read file to buffer
	if( ( 0 == strncmp( c_xml , LOCAL_FILE_DEV_ROOT , strlen( LOCAL_FILE_DEV_ROOT ) )) )
	{
		if(stat(c_xml, &st) < 0 || st.st_size < 0)
		{
			return ABXERR_FILENOTEXIST;
		}
		
		if( (buf = (char*)AVMem_malloc(st.st_size+2)) == NULL)
			return ABXERR_MALLOC_FAILURE;

		if((fd = open(c_xml, O_RDONLY)) < 0)
		{
			AVMem_free(buf);
			return ABXERR_FAILOPENFILE;
		}

		int iReadLen = 0 ;
		while( iReadLen < st.st_size )
		{
			int iRead =  read(fd , buf + iReadLen , st.st_size - iReadLen) ;
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
			AVMem_free(buf);
			close(fd);
			return ABXERR_FAILREADFILE;
		}
		buf[iReadLen] = '\0' ;
		close(fd);
	}
	else
		buf = c_xml ;
	
	ret = ixmlParseBufferEx(buf, &doc);
	if( ( 0 == strncmp( c_xml , LOCAL_FILE_DEV_ROOT , strlen( LOCAL_FILE_DEV_ROOT ) )) )
		AVMem_free(buf);

	//start parse xml file
	if(ret != IXML_SUCCESS)
		return ABXERR_MOAP_PARSE_XML_ERR ;

	if( (node = ixmlNode_getFirstChild(&doc->n)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_CHILD;
	}

	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_CHILD ;
	}

	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_CHILD ;
	}

	for( ; node ; node = ixmlNode_getNextSibling( node ) )
	{
		name = ixmlNode_getNodeName(node);
		if( name && 0 == strcmp( "key" , name ) )
		{
			if( NULL != ( node2 = ixmlNode_getFirstChild(node)))
			{
				if(NULL != (value = ixmlNode_getNodeValue(node2)))
				{
					memcpy( p_key , value , 8 ) ;
				}
			}
		}
		else if( name && 0 == strcmp( "value" , name ) )
		{
			if( NULL != ( node2 = ixmlNode_getFirstChild(node)))
			{
				if(NULL != (value = ixmlNode_getNodeValue( node2 )))
				{
					if( *p_root_url )
						AVMem_free( *p_root_url ) ;
					(*p_root_url ) = DLUIUtils_strdup(value);
				}
			}			
		}
	}

	ixmlDocument_free( doc ) ;
	return ABXERR_OK  ;
}

void Download_ReleaseXmlCtx(dl_rcmd_xml_ctx_type *ctx)
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
				_AVMem_free(ctx->nodes[i].cid);
				_AVMem_free(ctx->nodes[i].torrent);				
				_AVMem_free(ctx->nodes[i].xml);
				_AVMem_free( ctx->nodes[i].c_size ) ;
				if( ctx->nodes[i].mem_pic )
				{
					_AVMem_free( ctx->nodes[i].mem_pic->pic_buff ) ;
					_AVMem_free( ctx->nodes[i].mem_pic ) ;
				}
			}

			_AVMem_free(ctx->nodes);
		}

		_AVMem_free(ctx);
	}
}

int Download_ParseCH(char *xml, dl_rcmd_xml_ctx_type ** ctx)
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
									(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_DL_PROTOCOL ;
								}
							}
						}
						//child xml
						else if(name && 0 == strcmp(CHILD, name))
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
		}
	}while( NULL != (node = ixmlNode_getNextSibling(node)));
	
	ixmlDocument_free(doc);
	if( b_mem_buf)
		DLUI_FREE(buf);
	if( 0 == (*ctx)->nodes_count  )
	{
		Download_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
		return 0;
}

int Download_ParseMovie(char *xml, dl_rcmd_xml_ctx_type ** ctx)
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
	int node_idx = 0 ;

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
	//FilmData /moviesets
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

	if( strcmp( "FilmData" , name ) )
	{
		if( strcmp(MOVIESETS, name ) )
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
		//Filmcount
		if(name && ( strcmp("total", name) == 0 || strcmp("FilmCount", name) == 0 ) )
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
		else if(name && strcmp("Film", name) == 0)
		{
			dl_rcmd_xml_movie_node_type *nodes;
			
			if( (*ctx)->nodes_count == 0 )
			{
				if( NULL != ((*ctx)->nodes = (dl_rcmd_xml_movie_node_type*)DLUI_CALLOC(1, sizeof(dl_rcmd_xml_movie_node_type))))
				{
					(*ctx)->nodes_count = 1;
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
			else if(( (*ctx)->nodes_count <= node_idx) && (node_idx > 0))
			{
				if( NULL != (nodes = (dl_rcmd_xml_movie_node_type*)DLUI_REALLOC((*ctx)->nodes,(node_idx+1)*sizeof(dl_rcmd_xml_movie_node_type))))
				{
					(*ctx)->nodes = nodes;
					(*ctx)->nodes_count = node_idx+1;
					memset( &(*ctx)->nodes[node_idx] ,  0x00 , sizeof(dl_rcmd_xml_movie_node_type) ) ;
				}
				else
				{
					Download_ReleaseXmlCtx(*ctx);
					(*ctx)->nodes = NULL;
					(*ctx)->nodes_count = 0;
					ixmlDocument_free(doc);
					if( b_mem_buf)
						DLUI_FREE(buf);
					return DLXML_ERROR_NO_MEMORY_RCMD ;
				}
			}
	
			node2 = ixmlNode_getFirstChild(node);
			while(node2)
			{
				name = ixmlNode_getNodeName(node2);
				//Title
				if(name && strcmp("Title", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != (value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].name = DLUIUtils_strdup(value);
						}
					}				
				}
				//Id
				else if(name && strcmp("Id", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != (value = ixmlNode_getNodeValue(node3)))
						{
							//if(sscanf(value, "%d", &n) == 1)						
							//	(*ctx)->nodes[node_idx].id = n;
							(*ctx)->nodes[node_idx].id = atoi(value);
						}
					}				
				}
				//Pic
				else if(name && strcmp("Pic", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != (value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].pic = DLUIUtils_strdup(value);
						}
					}				
				}				
				//Director
				else if(name && strcmp("Director", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != (value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "导演: " ,  value , NULL  ) ;						
						}
					}		
				}
				//Actor
				else if(name && strcmp("Actor", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "主演: " ,  value , NULL  ) ;						
						}
					}
				}
				//Lang
				else if(name && strcmp("Lang", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "语言: " ,  value , NULL  ) ;						
						}
					}
				}
				//LongTime
				else if(name && strcmp("LongTime", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "时长:" ,  value , NULL  ) ;						
						}
					}
				}
				//Type
				else if(name && strcmp("Type", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "类别:" ,  value , NULL  ) ;						
						}
					}
				}				
				//Area
				else if(name && strcmp("Area", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "地区:" ,  value , NULL  ) ;						
						}
					}
				}
				//PubTime
				else if(name && strcmp("PubTime", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "发行时间:" ,  value , NULL  ) ;						
						}
					}
				}
				//Descripti
				else if(name && strcmp("Descripti", name ) == 0 )
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if( NULL != ( value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].desc = DLUIUtils_strdup(value);							
							//(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "影片简介:" ,  value , NULL  ) ;						
						}
					}
				}			
				node2 = ixmlNode_getNextSibling(node2);	
			}
			node_idx++;
		}
	}while( NULL != (node = ixmlNode_getNextSibling(node)));

	ixmlDocument_free(doc);
	if( b_mem_buf)
		DLUI_FREE(buf);

	if( 0 == (*ctx)->channel_count )
		(*ctx)->channel_count = (*ctx)->nodes_count ;

	if( 0 == (*ctx)->nodes_count  )
	{
		Download_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
	{
		return 0;
	}
}

