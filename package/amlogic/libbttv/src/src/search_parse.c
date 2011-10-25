
#include "search_parse.h"
#include <ixml.h>
#include <xml_util.h>
//#include "Aw_windows.h"
#include "ioapi.h"
#include "net/ABoxBase/abx_error.h"
#include <AVmalloc.h>

#define		DLUI_MALLOC		AVMem_malloc
#define		DLUI_CALLOC		AVMem_calloc
#define		DLUI_REALLOC	AVMem_realloc
#define		DLUI_FREE		AVMem_free
#define 	_AVMem_free(p)  do{ if( p ){DLUI_FREE(p);p=NULL;}}while(0)

static char* DLUIUtils_strdup(const char *s)
{
	char *d = NULL;
	int len;
	if(s)
	{
		len = strlen(s);
		if( 0 == len )
			return NULL ;
		if(NULL != ( d = (char*)AVMem_malloc(len + 1) ) )
		{
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

void DLSrch_ReleaseXmlNodeCtx(dl_rcmd_xml_movie_node_type *node)
{
	if(node)
	{
		if( node->format )
			_AVMem_free( node->format ) ;
		if(node->name)
			_AVMem_free(node->name);
		if(node->mv_info)
			_AVMem_free(node->mv_info);
		if(node->desc)
			_AVMem_free(node->desc);
		if(node->pic)
			_AVMem_free(node->pic);
		if(node->cid)
			_AVMem_free(node->cid);
		if(node->torrent)
			_AVMem_free(node->torrent);
		if(node->xml)
			_AVMem_free(node->xml);
		if(node->c_size )
			_AVMem_free( node->c_size ) ;
	}
}

void DLSrch_ReleaseXmlCtx(dl_rcmd_xml_ctx_type *ctx)
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
				DLSrch_ReleaseXmlNodeCtx(&(ctx->nodes[i]));
			}

			_AVMem_free(ctx->nodes);
		}

		_AVMem_free(ctx);
	}
}


#define	SEARCHRESULT	"SearchResult"
#define SEARCHRESULT_LOW "searchresult"

#define	QUERY		"Query"
#define QUERY_LOW	"query"
//#define	ID			"id"
#define	COUNT		"Count"
#define COUNT_LOW   "count"
#define TOTAL		"total"

#define	RESULTSET	"ResultSet"
#define RESULTSET_LOW	"resultset"

#define	BTINFO		"BTInfo"
#define MOVIE_INFO  "movie_info"
//#define ISCHANNEL	"ischannel"
#define SIZE		"Size"
#define SIZE_LOW	"size"
#define DRM			"drm"
#define CONNECTED	"Connected"
#define CONNECTED_LOW	"connected"

#define NAME		"Name"
#define NAME_LOW 	"name"
#define TITLE 		"Title"
//#define MOVIEINFO	"movieinfo"
//#define DESC		"desc"
//#define PIC			"pic"
#define TORRENT		"torrent"
#define ENCRY       "encry_url"
#define SOURCE      "source"
#define ENCRY       "encry_url"
#define PROTOCOL    "protocol"

#define DIRECTOR 	"director"
#define ACTOR		"actor"
#define AREA		"area"
#define LANGUANGE	"languange"
#define PUBTIME		"pubTime"
#define PLAYTIME	"playTime"


//return 0 : success
int DLSrch_ParseXmlCtx( const char *xml, dl_rcmd_xml_ctx_type ** ctx)
{
	struct stat st = {0};
	//int fd;
	//char *buf;
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

	//read file to buffer
	/*if(stat(xml, &st) < 0 || st.st_size < 0)
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
	close(fd);*/

	ret = ixmlParseBufferEx((char *)xml, &doc);
	//DLUI_FREE((char *)xml);

	//start parse xml file
	if(ret != IXML_SUCCESS)
		return ABXERR_MOAP_PARSE_XML_ERR ;

	//SearchResult
	if( (node = ixmlNode_getFirstChild(&doc->n)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_CHILD;
	}
	
	if( (name = ixmlNode_getNodeName(node)) == NULL ||
		   strcmp(SEARCHRESULT, name))
	{
			if( strcmp( SEARCHRESULT_LOW , name ) )
			{
				ixmlDocument_free(doc);
				return DLXML_ERROR_NO_SEARCHRESULT;
			}
	}

	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_SEARCHRESULT_NO_CHILD;
	}

	if( (*ctx = (dl_rcmd_xml_ctx_type*)DLUI_CALLOC(1, sizeof(dl_rcmd_xml_ctx_type))) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_MEMORY_SEARCH;
	}

	int i_page_count = 0 ;
	do
	{
		name = ixmlNode_getNodeName(node);
		//Query
		if(name && ( 0 == strcmp( QUERY_LOW , name ) || strcmp(QUERY, name) == 0 ))
		{
			if( NULL != (attrs = ixmlNode_getAttributes(node)))
			{
				int iCount = 0 ;
				int iTotal = 0 ;
				//count
				if( NULL != (node2 = ixmlNamedNodeMap_getNamedItem(attrs, COUNT)))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)
						{
							iCount = n ;
						}
					}
				}
				//count
				if( NULL != (node2 = ixmlNamedNodeMap_getNamedItem(attrs, COUNT_LOW)))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)
						{
							iCount = n ;
						}
					}
				}
				//count
				if( NULL != (node2 = ixmlNamedNodeMap_getNamedItem(attrs, "PageSize")))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)
						{
							i_page_count = n ;
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
							iTotal = n ;
						}
					}
				}
				else
				{
					iTotal = iCount ;
				}
				ixmlNamedNodeMap_free(attrs);
				
				n = iTotal > iCount ? iCount : iTotal ;
				if( 0 == n )
					return ABXERR_MOAP_NO_SEARCH_RESULT ;
				if( n > 40 )
					n = 40 ;
				if( 0 != i_page_count )
					n = i_page_count ;
				if( NULL != ( (*ctx)->nodes = (dl_rcmd_xml_movie_node_type*)DLUI_CALLOC(n, sizeof(dl_rcmd_xml_movie_node_type))))
				{
					memset( (*ctx)->nodes , 0x00 , n * sizeof(dl_rcmd_xml_movie_node_type) ) ;
					(*ctx)->nodes_count = n;
				}
				else
					return DLXML_ERROR_NO_MEMORY;

			}
		}
		//ResultSet
		else if(name && ( 0 == strcmp( RESULTSET_LOW , name ) || strcmp(RESULTSET, name) == 0 ) )
		{
			int node_idx;
			
			node2 = ixmlNode_getFirstChild(node);

//			for(node_idx=0; node2 && node_idx<(*ctx)->nodes_count; node_idx++)
			for(node_idx=0; node2 && node_idx<(*ctx)->nodes_count; node2 = ixmlNode_getNextSibling(node2))
			{
				name = ixmlNode_getNodeName(node2);
				if(name && ( 0 == strcmp( MOVIE_INFO , name ) || strcmp(BTINFO, name) == 0 || 0 == strcmp( "FileInfo" , name ) ))
				{
					if( NULL != (attrs = ixmlNode_getAttributes(node2)))
					{
						//id
						/*if(node3 = ixmlNamedNodeMap_getNamedItem(attrs, ID))
						{
							if(value = ixmlNode_getNodeValue(node3))
							{
								if(sscanf(value, "%d", &n) == 1)
								{
									(*ctx)->nodes[node_idx].id = n;
								}
							}
						}
						//is channel
						if(node3 = ixmlNamedNodeMap_getNamedItem(attrs, ISCHANNEL))
						{
							if(value = ixmlNode_getNodeValue(node3))
							{
								if(sscanf(value, "%d", &n) == 1)
								{
									(*ctx)->nodes[node_idx].is_channel = n;
								}
							}
						}*/
						//size
						if( NULL != (node3 = ixmlNamedNodeMap_getNamedItem(attrs, SIZE)))
						{
							if( NULL != (value = ixmlNode_getNodeValue(node3)))
							{
								float float_size = 0;
								while(*value >= '0' && *value <= '9')
								{
									float_size = float_size * 10 + (*value - '0');
									value++;
								}
								(*ctx)->nodes[node_idx].size = float_size;
							}
						}	
						//size
						if( NULL != (node3 = ixmlNamedNodeMap_getNamedItem(attrs, SIZE_LOW)))
						{
							if( NULL != (value = ixmlNode_getNodeValue(node3)))
							{
								float float_size = 0;
								while(*value >= '0' && *value <= '9')
								{
									float_size = float_size * 10 + (*value - '0');
									value++;
								}
								(*ctx)->nodes[node_idx].size = float_size;
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
						//connect
						if( NULL != (node3 = ixmlNamedNodeMap_getNamedItem(attrs, CONNECTED)))
						{
							if( NULL != (value = ixmlNode_getNodeValue(node3)))
							{
								if(sscanf(value, "%d", &n) == 1)
								{
									(*ctx)->nodes[node_idx].connect = n;
								}
							}
						}
						//connect
						if( NULL != (node3 = ixmlNamedNodeMap_getNamedItem(attrs, CONNECTED_LOW )))
						{
							if( NULL != (value = ixmlNode_getNodeValue(node3)))
							{
								if(sscanf(value, "%d", &n) == 1)
								{
									(*ctx)->nodes[node_idx].connect = n;
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
						if(name && ( 0 == strcmp( NAME_LOW , name ) || strcmp(NAME, name) == 0 || 0 == strcmp( TITLE , name )))
						{
							if(NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].name = DLUIUtils_strdup(value);
									if( THUNDER_DL_PROTOCOL != (*ctx)->nodes[node_idx].iDownloadProtocol )
										(*ctx)->nodes[node_idx].iDownloadProtocol = BT_DL_PROTOCOL ;

									if( strstr(value, "RM") || strstr(value, "rm") )
										(*ctx)->nodes[node_idx].format = DLUIUtils_strdup("RM");
									else if( strstr(value, "AVI") || strstr(value, "avi") )
										(*ctx)->nodes[node_idx].format = DLUIUtils_strdup("AVI");
									else if( strstr(value, "MPG") || strstr(value, "mpg") )
										(*ctx)->nodes[node_idx].format = DLUIUtils_strdup("MPG");
									else if( strstr(value, "MPEG") || strstr(value, "mpeg") )
										(*ctx)->nodes[node_idx].format = DLUIUtils_strdup("MPEG");
									else if( strstr(value, "WMV") || strstr(value, "wmv") )
										(*ctx)->nodes[node_idx].format = DLUIUtils_strdup("WMV");
									else if( strstr(value, "MKV") || strstr(value, "mkv") )
										(*ctx)->nodes[node_idx].format = DLUIUtils_strdup("MKV");
									else if( strstr(value, "MP3") || strstr(value, "mp3") )
										(*ctx)->nodes[node_idx].format = DLUIUtils_strdup("MP3");
									else 
										(*ctx)->nodes[node_idx].format = DLUIUtils_strdup("--");
								}
							}
						}
						else if( name && 0 == strcmp( "Cid" , name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(  NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].cid = DLUIUtils_strdup(value);
									(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_DL_PROTOCOL ;
								}
							}							
						}
						else if( name && 0 == strcmp( "Torrent" , name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(  NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].torrent = DLUIUtils_strdup(value);
									(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_DL_PROTOCOL ;
								}
							}							
						}						
						else if( name && 0 == strcmp( "Format" , name ) )
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].format = DLUIUtils_strdup(value);
								}
							}							
						}
						else if( name && 0 == strcmp("Size", name))
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].c_size = DLUIUtils_strdup(value) ; 
									(*ctx)->nodes[node_idx].size = get_downdload_file_size( value ) ;
								}
							}
						}
						/*//movieinfo
						else if(name && strcmp(MOVIEINFO, name) == 0)
						{
							if(node4 = ixmlNode_getFirstChild(node3))
							{
								if(value = ixmlNode_getNodeValue(node4))
								{
									(*ctx)->nodes[node_idx].mv_info = DLUIUtils_strdup(value);
								}
							}
						}
						//desc
						else if(name && strcmp(DESC, name) == 0)
						{
							if(node4 = ixmlNode_getFirstChild(node3))
							{
								if(value = ixmlNode_getNodeValue(node4))
								{
									(*ctx)->nodes[node_idx].desc = DLUIUtils_strdup(value);
								}
							}
						}
						//pic
						else if(name && strcmp(PIC, name) == 0)
						{
							if(node4 = ixmlNode_getFirstChild(node3))
							{
								if(value = ixmlNode_getNodeValue(node4))
								{
									(*ctx)->nodes[node_idx].pic = DLUIUtils_strdup(value);
								}
							}
						}*/
						//torrent
						/*else if(name && strcmp(TORRENT, name) == 0 && (*ctx)->nodes[node_idx].iDownloadProtocol != THUNDER_DL_PROTOCOL )
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
								if(NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].torrent = DLUIUtils_strdup(value);
									(*ctx)->nodes[node_idx].iDownloadProtocol = BT_DL_PROTOCOL ;
								}
							}
						}*/
/*						//child xml
						else if(name && strcmp(CHILD, name) == 0)
						{
							if(node4 = ixmlNode_getFirstChild(node3))
							{
								if(value = ixmlNode_getNodeValue(node4))
								{
									(*ctx)->nodes[node_idx].xml = DLUIUtils_strdup(value);
								}
							}
						}
*/						//source 
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
										//(*ctx)->nodes[node_idx].iDownloadProtocol = 0 ;
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
						else if(name && strcmp( LANGUANGE , name) == 0)
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
						else if(name && strcmp( PUBTIME, name) == 0)
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
						else if(name && ( 0 == strcmp( "PlayTime" , name ) || strcmp( PLAYTIME , name) == 0 ))
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									if( THUNDER_DL_PROTOCOL ==(*ctx)->nodes[node_idx].iDownloadProtocol )
									{
										int i_play_time = atoi( value ) ;
										i_play_time = i_play_time/60 ;
										char c_play_time[12] ;
										snprintf( c_play_time , sizeof( c_play_time ) , "%d" , i_play_time ) ;
										(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "时长: " ,  c_play_time  , "分钟") ;
									}
									else
										(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "时长: " ,  value  , "分钟") ;
								}
							}
						}
						else if( name && 0 == strcmp("PicCount", name))
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].quality = atoi( value ) ;
								}
							}
						}
						node3 = ixmlNode_getNextSibling(node3);
					}

					//check for 2G
					if((*ctx)->nodes[node_idx].size < 2.0*1024*1024*1024)
					{
						node_idx ++;
					}
					else
					{
						DLSrch_ReleaseXmlNodeCtx(&((*ctx)->nodes[node_idx]));
					}
				}
//				node2 = ixmlNode_getNextSibling(node2);
			}
			(*ctx)->nodes_count = node_idx ;
		}
	}while( NULL != (node = ixmlNode_getNextSibling(node)));
	
	ixmlDocument_free(doc);
	return 0;
}

int Download_ParseHD( const char *xml, dl_rcmd_xml_ctx_type ** ctx)
{
	struct stat st = {0};
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
	int node_idx = 0 ;

	ret = ixmlParseBufferEx((char *)xml, &doc);

	//start parse xml file
	if(ret != IXML_SUCCESS)
	{
		return ABXERR_MOAP_PARSE_XML_ERR;
	}
	//booklist/booksets
	if( (node = ixmlNode_getFirstChild(&doc->n)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_CHILD;
	}
	
	if( (name = ixmlNode_getNodeName(node)) == NULL )
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_MOVIELIST;
	}

	if( strcmp( "booklist" , name ) )
	{
		if( strcmp("booksets", name ) )
		{
			ixmlDocument_free(doc);
			return DLXML_ERROR_NO_MOVIELIST;
		}
	}
	
	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_MOVIELIST_NO_CHILD;
	}

	if( (*ctx = (dl_rcmd_xml_ctx_type*)DLUI_CALLOC(1, sizeof(dl_rcmd_xml_ctx_type))) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_MEMORY_RCMD;
	}

	memset( *ctx , 0x00 , sizeof(dl_rcmd_xml_ctx_type) ) ;
	
	do
	{
		name = ixmlNode_getNodeName(node);
		//parent
		if(name && strcmp("parent", name) == 0)
		{
			if( NULL != ( attrs = ixmlNode_getAttributes(node) ) )
			{
				int iCount = 0 ;
				int iChannel_count = 0 ;
				//id
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, "id")))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						if(sscanf(value, "%d", &n) == 1)					
							(*ctx)->parent_id = n;
					}
				}
				//title
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, "title")))
				{
					if( NULL != (value = ixmlNode_getNodeValue(node2)))
					{
						(*ctx)->title = DLUIUtils_strdup(value);
					}
				}
				//count
				if( NULL != ( node2 = ixmlNamedNodeMap_getNamedItem(attrs, "count")))
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
								return DLXML_ERROR_NO_MEMORY_RCMD ;
							}
						}
					}
				}
				ixmlNamedNodeMap_free(attrs);
			}
		}
		//booksets
		else if(name && strcmp("booksets", name) == 0)
		{
			int node_idx = 0 ;
			node2 = ixmlNode_getFirstChild(node);
			if( 0 == (*ctx)->channel_count )
				(*ctx)->channel_count = (*ctx)->nodes_count ;

			for(node_idx=0; node2 && node_idx+1<(*ctx)->nodes_count; node_idx++)
			{
				name = ixmlNode_getNodeName(node2);
				//hd
				if(name && strcmp("hd", name) == 0)
				{
					node3 = ixmlNode_getFirstChild(node2);
					while(node3)
					{
						name = ixmlNode_getNodeName(node3);
						//title
						if(name && ((strcmp("Title", name ) == 0 )||(strcmp("title", name ) == 0 )))
						{
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].name = DLUIUtils_strdup(value);
								}
							}				
						}
						//size
						else if(name && strcmp("size", name ) == 0 )
						{
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "文件大小: " ,  value  , NULL ) ;
									(*ctx)->nodes[node_idx].c_size = DLUIUtils_strdup(value) ; 
									(*ctx)->nodes[node_idx].size = get_downdload_file_size( value ) ;						
								}				
							}				
						}
						//pub time 
						else if(name && strcmp( "pubtime", name) == 0)
						{
							if( NULL != (node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "发行时间: " ,  value  , NULL ) ;
								}
							}
						}				
						//format
						else if(name && strcmp("fmt", name ) == 0 )
						{
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if(NULL != (value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].format = DLUIUtils_strdup(value);
								}
							}				
						}				
						//cid
						else if(name && strcmp("cid", name ) == 0 )
						{
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].cid = DLUIUtils_strdup(value);
									(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_DL_PROTOCOL ;
								}
							}
						}
						//torrent
						else if(name && strcmp("url", name ) == 0 )
						{
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].torrent = DLUIUtils_strdup(value);
									(*ctx)->nodes[node_idx].iDownloadProtocol = THUNDER_DL_PROTOCOL ;
								}
							}
						}						
						//hdtag
						else if(name && strcmp("hdtat", name ) == 0 )
						{
							if( NULL != ( node4 = ixmlNode_getFirstChild(node3)))
							{
								if( NULL != ( value = ixmlNode_getNodeValue(node4)))
								{
									(*ctx)->nodes[node_idx].mv_info = AddString( (*ctx)->nodes[node_idx].mv_info , "分辨率: " ,  value , NULL  ) ;						
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

	if( 0 == (*ctx)->nodes_count  )
	{
		DLSrch_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
	{
		return 0;
	}
}

int Download_ParseSuggest( const char *xml, dl_rcmd_xml_ctx_type ** ctx)
{
	struct stat st = {0};
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
	int node_idx = 0 ;

	ret = ixmlParseBufferEx((char *)xml, &doc);

	//start parse xml file
	if(ret != IXML_SUCCESS)
	{
		return ABXERR_MOAP_PARSE_XML_ERR;
	}
	//suggest
	if( (node = ixmlNode_getFirstChild(&doc->n)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_CHILD;
	}
	
	if( (name = ixmlNode_getNodeName(node)) == NULL )
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_MOVIELIST;
	}

	if( strcmp( "Suggest" , name ) )
	{
		return DLXML_ERROR_NO_MOVIELIST;
	}
	
	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_MOVIELIST_NO_CHILD;
	}

	if( (*ctx = (dl_rcmd_xml_ctx_type*)DLUI_CALLOC(1, sizeof(dl_rcmd_xml_ctx_type))) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_MEMORY_RCMD;
	}

	memset( *ctx , 0x00 , sizeof(dl_rcmd_xml_ctx_type) ) ;

	do
	{
		name = ixmlNode_getNodeName(node);
		//SuggestWord
		if(name && strcmp("SuggestWord", name) == 0)
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
					DLSrch_ReleaseXmlCtx(*ctx);
					(*ctx)->nodes = NULL;
					(*ctx)->nodes_count = 0;
					ixmlDocument_free(doc);
					return DLXML_ERROR_NO_MEMORY_RCMD ;
				}
			}

			if( NULL != ( node2 = ixmlNode_getFirstChild(node)))
			{
				if(NULL != (value = ixmlNode_getNodeValue(node2)))
				{
					(*ctx)->nodes[node_idx].name = DLUIUtils_strdup(value);
				}
			}				
		}
		node_idx++;
	}while( NULL != (node = ixmlNode_getNextSibling(node)));
	
	if( 0 == (*ctx)->channel_count )
		(*ctx)->channel_count = (*ctx)->nodes_count ;

	ixmlDocument_free(doc);

	if( 0 == (*ctx)->nodes_count  )
	{
		DLSrch_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
	{
		return 0;
	}
}

void Server_ReleaseXmlCtx(server_xml_type *ctx)
{
	if(ctx)
	{
            int i = 0;
                for(i=0; i < ctx->count; i++)
		{
			if(ctx->nodes && ctx->nodes[i].type)
				_AVMem_free(ctx->nodes[i].type) ;
			if(ctx->nodes && ctx->nodes[i].url)
				_AVMem_free(ctx->nodes[i].url);
		}
	}
}

int Server_ParseXmlCtx( const char *xml, server_xml_type ** ctx)
{
	struct stat st = {0};
	int ret;

	IXML_Document* doc = NULL ;
	IXML_Node* node = NULL ;
	IXML_Node* node2 = NULL ;
	IXML_Node* node3 = NULL ;
	IXML_NamedNodeMap* attrs = NULL ;
	int n = 0 ;
	
	DOMString name = NULL ;
	DOMString value = NULL ;
	int node_idx = 0 ;

	ret = ixmlParseBufferEx((char *)xml, &doc);

	if(ret != IXML_SUCCESS)
	{
		return ABXERR_MOAP_PARSE_XML_ERR;
	}
	if( (node = ixmlNode_getFirstChild(&doc->n)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_CHILD;
	}
	
	if( (name = ixmlNode_getNodeName(node)) == NULL )
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_MOVIELIST;
	}

	if( strcmp( "serverlist" , name ) )
	{
		return DLXML_ERROR_NO_MOVIELIST;
	}
	
	if( (node = ixmlNode_getFirstChild(node)) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_MOVIELIST_NO_CHILD;
	}

	if( (*ctx = (server_xml_type*)DLUI_CALLOC(1, sizeof(server_xml_type))) == NULL)
	{
		ixmlDocument_free(doc);
		return DLXML_ERROR_NO_MEMORY_RCMD;
	}

	memset( *ctx , 0x00 , sizeof(server_xml_type) ) ;

	do
	{
		name = ixmlNode_getNodeName(node);
		//Server
		if(name && strcmp("server", name) == 0)
		{
			server_node_type *nodes;
			
			if( (*ctx)->count == 0 )
			{
				if( NULL != ((*ctx)->nodes = (server_node_type*)DLUI_CALLOC(1, sizeof(server_node_type))))
				{
					(*ctx)->count = 1;
					memset( (*ctx)->nodes ,  0x00 , n *  sizeof(server_node_type) ) ;
				}
				else
				{
					ixmlDocument_free(doc);
					return DLXML_ERROR_NO_MEMORY_RCMD ;
				}
			}			
			else if(( (*ctx)->count <= node_idx) && (node_idx > 0))
			{
				if( NULL != (nodes = (server_node_type*)DLUI_REALLOC((*ctx)->nodes,(node_idx+1)*sizeof(server_node_type))))
				{
					(*ctx)->nodes = nodes;
					(*ctx)->count = node_idx+1;
					memset( &(*ctx)->nodes[node_idx] ,  0x00 , sizeof(server_node_type) ) ;
				}
				else
				{
					Server_ReleaseXmlCtx(*ctx);
					(*ctx)->nodes = NULL;
					(*ctx)->count = 0;
					ixmlDocument_free(doc);
					return DLXML_ERROR_NO_MEMORY_RCMD ;
				}
			}

			node2 = ixmlNode_getFirstChild(node);
			while(node2)
			{
				name = ixmlNode_getNodeName(node2);
				//type
				if(name && ((strcmp("type", name ) == 0 )||(strcmp("Type", name ) == 0 )))
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != (value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].type = DLUIUtils_strdup(value);
						}
					}				
				}
				//url
				if(name && ((strcmp("url", name ) == 0 )||(strcmp("Url", name ) == 0 )))
				{
					if( NULL != ( node3 = ixmlNode_getFirstChild(node2)))
					{
						if(NULL != (value = ixmlNode_getNodeValue(node3)))
						{
							(*ctx)->nodes[node_idx].url = DLUIUtils_strdup(value);
						}
					}				
				}

				node2 = ixmlNode_getNextSibling(node2);
			}				
		}
		node_idx++;
	}while( NULL != (node = ixmlNode_getNextSibling(node)));
	

	ixmlDocument_free(doc);

	if( 0 == (*ctx)->count  )
	{
		Server_ReleaseXmlCtx(*ctx) ;
		(*ctx) = NULL ;
		return DLXML_ERROR_NO_CHILD;
	}
	else
	{
		return 0;
	}
}
