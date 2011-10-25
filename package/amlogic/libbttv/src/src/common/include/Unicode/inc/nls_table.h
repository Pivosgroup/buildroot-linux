#ifndef NLS_TABLE__H
#define NLS_TABLE__H

#include "Unicode/inc/nls_uni.h"

/*
中文繁体
charset :  "BIG5"
*/
extern struct nls_table nls_table_big5;

/*
This Codepage is designed for Central Europe languages: Albanian, Croatian, Czech, Hungarian, Polish,
	Romanian, Serbian (Latin), Slovak, Slovenian.
charset: "cp1250"
*/
extern struct nls_table nls_table_cp1250;

/*
This Codepage is designed for Cyrillic languages: Azeri, Belarusian, Bulgarian, FYRO Macedonian, Kazakh, 
	Kyrgyz, Mongolian, Russian, Serbian, Tatar, Ukrainian, Uzbek. 
charset: "cp1251"
*/
extern struct nls_table nls_table_cp1251;

/*
This Codepage is designed for Greek. 
charset: "cp1253"
*/
extern struct nls_table nls_table_cp1253;

/*
This Codepage is designed for Turkic languages: Azeri (Latin), Turkish, Uzbek (Latin). 
charset: "cp1254"
*/
extern struct nls_table nls_table_cp1254;

/*
This Codepage is designed for Hebrew. 
charset: "cp1255"
*/
extern struct nls_table nls_table_cp1255;

/*
This Codepage is designed for Arabic, Farsi, Urdu. 
charset: "cp1256"
*/
extern struct nls_table nls_table_cp1256;

/*
Cyrillic code page to be used with MS-DOS
charset: "cp866"
*/
extern struct nls_table nls_table_cp866;

/*
This Codepage is designed for JIS 
charset: "cp932"
*/
extern struct nls_table nls_table_cp932;

/*
This Codepage is designed for Korea. 
charset: "cp949"
*/
extern struct nls_table nls_table_cp949;

/*
中文简体
charset: "GB2312"
*/
extern struct nls_table nls_table_gb2312;

/*
west europe
charset: "iso8859-1"
*/
extern struct nls_table nls_table_iso8859_1;

/*
Central Europe languages (iso)
charset: "iso8859-2"
*/
extern struct nls_table nls_table_iso8859_2;

/*
Cyrillic languages (iso)
charset: "iso8859-5"
*/
extern struct nls_table nls_table_iso8859_5;

/*
Arabic languages (iso)
charset: "iso8859-5"
*/
extern struct nls_table nls_table_iso8859_6;

/*
Greek languages (iso)
charset: "iso8859-7"
*/
extern struct nls_table nls_table_iso8859_7;

/*
Turkic languages (iso)
charset: "iso8859-9"
*/
extern struct nls_table nls_table_iso8859_9;

/*
俄语的缺省支持
charset: "koi8-r"
*/
extern struct nls_table nls_table_koi8r;

/*
This Codepage is designed for Thailand. 
charset: "cp874"
*/
extern struct nls_table nls_table_cp874;

/*
This Codepage is designed for Vietnam. 
charset: "cp1258"
*/
extern struct nls_table nls_table_cp1258;

/*
This Codepage is designed for Latin 1 languages: Afrikaans, Basque, C
atalan, Danish, Dutch, English, Faroese, Finnish, French, Galician, German, Icelandic, Indonesian, Italian, Malay, Norwegian, Portuguese, Spanish, Swahili, Swedish. 
charset: "cp1252"
*/
extern struct nls_table nls_table_cp1252;

/*
This Codepage is designed for hindu. 
charset: "hindi"
*/
extern struct nls_table nls_table_hindi;

extern void register_nls_table(struct nls_table * nls);
extern void set_nls_table(struct nls_table * nls);
#endif
