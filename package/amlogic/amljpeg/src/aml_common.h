#ifndef AML_COMMON_H_
#define AML_COMMON_H_
typedef struct aml_dec_para{
    char* fn;
    int width;          /* scaled image width info */
    int height ;         /* scaled image height info */
    int iwidth;         /* original image width info */
    int iheight;        /* original image height info */
    int flag;
    int mode;    
	unsigned char *rgb;
	unsigned char *alpha;    
    unsigned pic_type;		/* pointer to load function. */
}aml_dec_para_t;

typedef struct aml_image{
    int width;          /* scaled image width info */
    int height;         /* scaled image height info */
    int iwidth;         /* original image width info */
    int iheight;        /* original image height info */
    int depth;
    int bytes_per_line;
    int nbytes;  
    int do_free;
	unsigned char *rgb;
	unsigned char *alpha;      
    char* data;         /*scaled image data */
}aml_image_info_t;
#endif