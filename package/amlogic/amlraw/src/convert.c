/*   
    Copyright (C) 2010 amlogic.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/*
 *  convert.c
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
//#include "scaler_common.h"
#include "scaler.h"
#include "raw.h"
//#include "rawdec_type.h"
#include "convert.h"
#include <ge2d.h>
#include <amstream.h>
#include <vformat.h>
#include <cmem.h>
#define CANVAS_ALIGNED(x)	(((x) + 7) & ~7)
#define   FILE_NAME_GE2D		  "/dev/ge2d"
CMEM_AllocParams cmemParm;
unsigned char *planes[4];
unsigned char* plane;
static char* scan_line(aml_image_info_t* info , int i)
{
    if((!info)||(!info->data)||(i > 1080)){
        return NULL;    
    }
    return (info->data + i*info->bytes_per_line);
}
#if 1
int amlraw_init(int w ,int h,int flag)
{
    int ret = 0 ;
    unsigned char* p;
    int width =  CANVAS_ALIGNED(w);
    int height = h;
    plane = NULL;
    if(!flag){
       goto exit; //we don't need display the image 
    }
    cmemParm.type=CMEM_HEAP; cmemParm.flags=CMEM_NONCACHED; cmemParm.alignment=8;
    if(CMEM_init())
    {    
        ret = -1;
		printf("hw jpeg init decoder error---cmem init error\n");		
		goto exit;	
    }	 
    plane = (unsigned char *)CMEM_alloc(0,width*height*4,&cmemParm);
    printf("plane address is %x\n", plane);
    memset(plane,0,1280*720*4);
    p = (unsigned char*)CMEM_getPhys(plane);
    printf("physical address is %x\n", p);
    if(!plane){
        ret = -1;
        goto exit;	
    }   
//	if(plane){
//		CMEM_free(plane, &cmemParm);
//		plane = NULL;		
//		printf("free plane\n" );  		 
//	}
exit:		        
//    CMEM_exit();    
    return ret;
}
int amlraw_exit(int flag)
{
    if(flag){
        CMEM_exit();        
    }   
    return 0;
}
#endif
aml_image_info_t* convert_fb(int width , int height ,unsigned char* output ,int flag)
{
    int i ;
    int scale_w , scale_h ;
	int fd_ge2d=-1;
	char* p;
	config_para_t ge2d_config;   
	ge2d_op_para_t op_para; 
    aml_image_info_t* input_image_info;
    aml_image_info_t* output_image_info;    
    char* input_data;
    char* output_data;    
    scale_w = width;
    scale_h = height;    
    int input_image_width = CANVAS_ALIGNED(scale_w);
    cmemParm.type=CMEM_HEAP; cmemParm.flags=CMEM_NONCACHED; cmemParm.alignment=8;
	planes[0] = planes[1]= planes[2]= planes[3]=NULL;
//	plane = NULL;

	input_image_info = (aml_image_info_t*)malloc(sizeof(aml_image_info_t));
    output_image_info = (aml_image_info_t*)malloc(sizeof(aml_image_info_t));
    memset((char*)output_image_info , 0 , sizeof(aml_image_info_t));            
     memset((char*)input_image_info , 0 , sizeof(aml_image_info_t));
    
    output_image_info->width = scale_w;
    output_image_info->height = scale_h;
    output_image_info->depth = 32;
    output_image_info->bytes_per_line = ((output_image_info->width * output_image_info->depth + 31) >> 5 ) << 2 ;
    output_image_info->nbytes = output_image_info->bytes_per_line * height;
    output_image_info->data  = output  ;    

/*if flag is set , we need display the image directly to frame buffer*/    
    if(flag){
//      if(CMEM_init())
//      {    
//		    printf("hw jpeg init decoder error---cmem init error\n");		
//		    goto exit;	
//      }	
//      plane = (unsigned char *)CMEM_alloc(0,input_image_width*scale_h*4,&cmemParm);
        printf("plane address is %x\n", plane);
        p = (unsigned char*)CMEM_getPhys(plane);
        printf("physical address is %x\n", p);
        if(!plane){
            goto exit;	
        }           
        input_image_info->width = scale_w;
        input_image_info->height = scale_h;
        input_image_info->depth = 32;
        input_image_info->bytes_per_line = input_image_width << 2 ;
        input_image_info->nbytes = input_image_info->bytes_per_line * height;
        input_image_info->data  = (char*)plane ;
        printf("start test output image\n");
        fd_ge2d= open(FILE_NAME_GE2D, O_RDWR);    
    	if(fd_ge2d<0)
    	{	    
    		printf("can't open framebuffer device" );  			
    		goto exit;	
    	}    
        for(i = 0 ; i < input_image_info->height ; i++){
            input_data =   scan_line(output_image_info , i);
            output_data =  scan_line(input_image_info , i);       
            memcpy(output_data , input_data  , 4* output_image_info->width );
        } 
    	ge2d_config.src_format = GE2D_FORMAT_S32_ARGB;
    	ge2d_config.dst_format = GE2D_FORMAT_S32_ARGB;    
        ge2d_config.src_dst_type = ALLOC_OSD1;	
        ge2d_config.alu_const_color=0xff0000ff;
    	ge2d_config.src_planes[0].addr=CMEM_getPhys(plane);	
    	ge2d_config.src_planes[0].w=  input_image_width;
    	ge2d_config.src_planes[0].h = scale_h;  
        ge2d_config.dst_planes[0].addr=CMEM_getPhys(plane);	
    	ge2d_config.dst_planes[0].w=  input_image_width;
    	ge2d_config.dst_planes[0].h = scale_h;
    	ioctl(fd_ge2d, FBIOPUT_GE2D_CONFIG, &ge2d_config);  
    	
    	op_para.src1_rect.x = 0;
    	op_para.src1_rect.y = 0;
    	op_para.src1_rect.w = scale_w;
    	op_para.src1_rect.h = scale_h;
    	op_para.dst_rect.x = 0;
    	op_para.dst_rect.y = 0;
    	op_para.dst_rect.w = scale_w;
    	op_para.dst_rect.h = scale_h;	
        ioctl(fd_ge2d, FBIOPUT_GE2D_STRETCHBLIT_NOALPHA, &op_para);                    
    }
exit:
	if(plane){
		CMEM_free(plane, &cmemParm);
		plane = NULL;		
		printf("free plane\n" );  		 
	}	    
    if(!output_image_info->data){
        if(output_image_info){
            free(output_image_info);   
            output_image_info = NULL; 
        } 
    }
    if(input_image_info){
        free(input_image_info);   
        input_image_info = NULL;         
    }
	if(fd_ge2d >=0){
	    close(fd_ge2d);		
	    fd_ge2d = -1;
    }    
	return output_image_info;    
}