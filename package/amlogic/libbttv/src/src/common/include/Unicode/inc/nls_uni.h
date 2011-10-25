#ifndef NLS_UNI_H
#define NLS_UNI_H
#ifndef NULL
#define NULL ((void *)0)
#endif
typedef unsigned short u_int16_t;
struct nls_table {
	char *charset;
	int flag;
	u_int16_t (*uni2char) (u_int16_t unicode);
	u_int16_t (*char2uni) (u_int16_t othercode);
	
	const unsigned char * cs2low;
	const unsigned char * cs2up;
	
	struct nls_table *next;
};
typedef enum unicode_occupy_types_s
{
    UNICODE_NULL,
    UNICODE_ONE_BYTE,
    UNICODE_TWO_BYTES,
   
} unicode_occupy_types_t;
int register_nls(struct nls_table * nls);
int unregister_nls(struct nls_table * nls);
#endif 
