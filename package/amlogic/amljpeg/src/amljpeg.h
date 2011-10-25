#if defined (__cplusplus)
extern "C" {
#endif
#include "aml_common.h"
//typedef struct aml_image{
//    int width;
//    int height;
//    int depth;
//    int bytes_per_line;
//    int nbytes;  
//    char* data;  
//}aml_image_info_t;
int amljpeg_init();
void amljpeg_exit();
aml_image_info_t* read_jpeg_image(char* url , int width, int height,int mode, int flag);
#if defined (__cplusplus)
}
#endif