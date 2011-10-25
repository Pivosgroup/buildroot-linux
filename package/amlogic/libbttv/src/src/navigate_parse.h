
#ifndef DLRECOMMENDXML_H
#define DLRECOMMENDXML_H

#include "netdownload_comm.h"

int DLRcmd_ParseXmlCtx(char *xml, dl_rcmd_xml_ctx_type ** ctx);
void DLRcmd_ReleaseXmlCtx(dl_rcmd_xml_ctx_type *ctx);
int parse_register_xml( char * c_xml  , char * p_key , char ** p_root_url ) ;
int Download_ParseCH(char *xml, dl_rcmd_xml_ctx_type ** ctx);
int Download_ParseMovie(char *xml, dl_rcmd_xml_ctx_type ** ctx);

#endif // DLRECOMMENDXML_H


