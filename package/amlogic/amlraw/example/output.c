//#include "scaler_common.h"
//#include "scaler.h"
#include "amlraw.h"
//#include "rawdec_type.h"
//#include "rawdec_ahd_cbrt.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
    int width, height ,mode,flag;    
    aml_image_info_t* image_info;    
    char* p;
    if(argc < 5){        
        printf("Amlogic raw decoder API \n");
        printf("usage: output [filename] [width] [height] [mode]\n");
        printf("options :\n");
        printf("         filename : raw url in your root fs\n");
        printf("         width    : output width\n");
        printf("         height   : output height\n");
        printf("         mode     : 0/keep ratio  1/crop image 2/stretch image\n");
        printf("         flag     : 0/disable display  1/enable display\n");
        return -1;    
    }else{
        printf("%s\n", argv[1]);
    }   
    image_info = read_raw_image(argc, argv);
    
    if(image_info){
        printf("output image width is %d\n", image_info->width);
        printf("output image height is %d\n", image_info->height);
        printf("output image depth is %d\n", image_info->depth);
        printf("output image bytes_per_line is %d\n", image_info->bytes_per_line);
        printf("output image nbytes   is %d\n", image_info->nbytes);
    }
    if(image_info){
        free(image_info);    
        image_info = NULL;
    }    
    return 0;
}
