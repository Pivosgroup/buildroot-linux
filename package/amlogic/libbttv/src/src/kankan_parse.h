
#ifndef KANKAN_PARSE_H
#define KANKAN_PARSE_H

#include "netdownload_comm.h"
#define SEARCH_STRING    "Ó°Æ¬ËÑË÷"

int KanKan_ParseCH(char *xml, dl_rcmd_xml_ctx_type ** ctx);
int KanKan_ParseMovies( char * xml , dl_rcmd_xml_ctx_type **ctx,char bIPTV,char bCopyright, char bSearch);
int KanKan_ParseVodInfo(char *xml, KANKAN_PROGRAME ** vod_info,char bIPTV,char bCopyright);
void KanKan_VodRelease( KANKAN_PROGRAME ** programe );
void KanKan_ReleaseXmlCtx(dl_rcmd_xml_ctx_type *ctx);

#endif // KANKAN_PARSE_H


