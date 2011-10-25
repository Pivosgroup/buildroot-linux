
#ifndef DLSEARCHXML_H
#define DLSEARCHXML_H


#include "netdownload_comm.h"

int DLSrch_ParseXmlCtx( const char *xml, dl_rcmd_xml_ctx_type ** ctx);
void DLSrch_ReleaseXmlCtx(dl_rcmd_xml_ctx_type *ctx);
int Download_ParseHD( const char *xml, dl_rcmd_xml_ctx_type ** ctx);
int Download_ParseSuggest( const char *xml, dl_rcmd_xml_ctx_type ** ctx);
int Server_ParseXmlCtx( const char *xml, server_xml_type** ctx);
void Server_ReleaseXmlCtx(server_xml_type *ctx);

#endif // DLSEARCHXML_H


