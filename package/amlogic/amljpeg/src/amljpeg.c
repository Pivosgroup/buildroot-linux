/*
 * Copyright (c) 2009, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * cmem.c
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <ge2d.h>
#include <amstream.h>
#include <vformat.h>
#include "amljpeg.h"
#include <jpegdec.h>
#include <cmem.h>
//#define JPEG_DBG 
#undef JPEG_DBG
#define   FILE_NAME_AMPORT    "/dev/amstream_vbuf"
#define   FILE_NAME_JPEGDEC	  "/dev/amjpegdec"
#define   FILE_NAME_GE2D		  "/dev/ge2d"
#define   MAX_BUFFER_SIZE		  (32*1024)
#define   HEADER_SIZE				  (2048)	
#define CANVAS_ALIGNED(x)	(((x) + 7) & ~7)

enum {
	DEC_STAT_INFOCONFIG=0,
	DEC_STAT_INFO,
	DEC_STAT_DECCONFIG,
	DEC_STAT_RUN,
	DEC_STAT_MAX
};
enum{
    KEEPASPECTRATIO =0,    
    KEEPASPECTRATIOBYEXPANDING,
    IGNOREASPECTRATIO
};

typedef  struct{
 int  fd_amport;
 jpegdec_info_t info;
 char buffer[MAX_BUFFER_SIZE];
}jpeg_data_t;

#define Format_RGB32 0
#define Format_RGB16 1
int fd_amport = -1;
int scale_x;
int scale_y;
int scale_w;
int scale_h;	
CMEM_AllocParams cmemParm;
unsigned char *planes[4];
unsigned decoder_opt;
int sMode;
//to get a valid image for jpeg.
inline int nextMulOf8(int n)
{
    return ((n + 7) & 0xfffffff8);
}

int amljpeg_init()
{
    int i = 0;
#ifdef JPEG_DBG     
    printf("last fd_amport is (%d).\n",fd_amport);
	printf("------------HWJpegDec Init----------------\n");
#endif    
    fd_amport = open(FILE_NAME_AMPORT, O_RDWR);
	if(fd_amport<0) {
#ifdef JPEG_DBG 	    
		printf("hw jpeg init decoder error---amport access error\n");
#endif		
		return;
	}
	cmemParm.type=CMEM_HEAP; cmemParm.flags=CMEM_NONCACHED; cmemParm.alignment=8;
	planes[0] = planes[1]= planes[2]= planes[3]=NULL;
	if(CMEM_init())
    {
#ifdef JPEG_DBG         
		printf("hw jpeg init decoder error---cmem init error\n");
#endif  		
		return;	
    }	

	
    //init amport for write device data.	
    ioctl(fd_amport, AMSTREAM_IOC_VB_SIZE, 1024*1024);
    ioctl(fd_amport, AMSTREAM_IOC_VFORMAT, VFORMAT_JPEG); 
    ioctl(fd_amport, AMSTREAM_IOC_PORT_INIT);
    while(access(FILE_NAME_JPEGDEC, R_OK|W_OK)){	 //waitting for device created.
      	i ++;
		usleep(1000);
      	if(i>1000)
      	{
#ifdef JPEG_DBG       	    
			printf("hw jpeg init decoder error---hw jpeg device access error\n");
#endif			
			return;
      	}
    }    
}

void amljpeg_exit()
{
	if(fd_amport>=0)
	{
		close(fd_amport);
#ifdef JPEG_DBG 		
		printf("fd_amport (%d) closed.\n",fd_amport);
#endif		
		fd_amport=-1;
	}
	CMEM_exit();    
}
unsigned int  ImgFormat2Ge2dFormat(int img_format)
{
	unsigned int format=0xffffffff;
	
	 switch (img_format) {
	/* 32 bpp */
    	case Format_RGB32:
		format = GE2D_FORMAT_S32_ARGB;
		break;
	/* 16 bpp */
    	case Format_RGB16:
		format = GE2D_FORMAT_S16_RGB_565;
	    break;
	default:
#ifdef JPEG_DBG 	
    	printf("blit_32(): Image format %d not supported!", img_format);
#endif    	
        format = GE2D_FORMAT_S32_ARGB;
        break;	
    	}
	 return format;

}
int clear_plane(int index,jpegdec_config_t *config)
{
	int fd_ge2d=-1;
	config_para_t ge2d_config;
    int format;
    ge2d_op_para_t op_para;
    memset((char*)&ge2d_config,0,sizeof(config_para_t));
    fd_ge2d= open(FILE_NAME_GE2D, O_RDWR);
	if(fd_ge2d<0)
	{
#ifdef JPEG_DBG 	    
		printf("can't open framebuffer device" );  	
#endif		
		goto exit;
	}
    format = Format_RGB32;
#ifdef JPEG_DBG 
	printf("start clean plane buffer %d!!!!!\n", index);
#endif	
	ge2d_config.src_dst_type = ALLOC_ALLOC;

//	qCritical("planes[3]addr : 0x%x-0x%x" ,planes[3],ge2d_config.dst_planes[0].addr);		
    
    switch(index){
        case 0:
        op_para.color=0x008080ff;
        ge2d_config.src_format = GE2D_FORMAT_S8_Y;
	    ge2d_config.dst_format = GE2D_FORMAT_S8_Y;
    	ge2d_config.dst_planes[0].addr = config->addr_y;
    	ge2d_config.dst_planes[0].w = config->canvas_width;
    	ge2d_config.dst_planes[0].h = config->dec_h;     
    	op_para.src1_rect.x = config->dec_x;
    	op_para.src1_rect.y = config->dec_y;
    	op_para.src1_rect.w = config->dec_w;
    	op_para.src1_rect.h = config->dec_h;      	   
    	op_para.dst_rect.x = config->dec_x;
    	op_para.dst_rect.y = config->dec_y;
    	op_para.dst_rect.w = config->dec_w;
    	op_para.dst_rect.h = config->dec_h;        	
        break;
        case 1:
        op_para.color=0x008080ff;
        ge2d_config.src_format = GE2D_FORMAT_S8_CB;
	    ge2d_config.dst_format = GE2D_FORMAT_S8_CB;
    	ge2d_config.dst_planes[0].addr = config->addr_u;
    	ge2d_config.dst_planes[0].w = config->canvas_width/2;
    	ge2d_config.dst_planes[0].h = config->dec_h / 2;      
    	op_para.src1_rect.x = config->dec_x/2;
    	op_para.src1_rect.y = config->dec_y/2;
    	op_para.src1_rect.w = config->dec_w/2;
    	op_para.src1_rect.h = config->dec_h/2;       	
    	op_para.dst_rect.x = config->dec_x/2;
    	op_para.dst_rect.y = config->dec_y/2;
    	op_para.dst_rect.w = config->dec_w/2;
    	op_para.dst_rect.h = config->dec_h/2;        	
        break;
        case 2:
        op_para.color=0x008080ff;
        ge2d_config.src_format = GE2D_FORMAT_S8_CR;
	    ge2d_config.dst_format = GE2D_FORMAT_S8_CR;    
    	ge2d_config.dst_planes[0].addr = config->addr_v;
    	ge2d_config.dst_planes[0].w = config->canvas_width/2;
    	ge2d_config.dst_planes[0].h = config->dec_h / 2;
    	op_para.src1_rect.x = config->dec_x/2;    
    	op_para.src1_rect.y = config->dec_y/2;    
    	op_para.src1_rect.w = config->dec_w/2;    
    	op_para.src1_rect.h = config->dec_h/2;    
    	op_para.dst_rect.x = config->dec_x/2;
    	op_para.dst_rect.y = config->dec_y/2;
    	op_para.dst_rect.w = config->dec_w/2;
    	op_para.dst_rect.h = config->dec_h/2;        		    
        break;
        case 3:
        op_para.color=0x000000ff;
        ge2d_config.src_format = ImgFormat2Ge2dFormat(format);
        ge2d_config.dst_format = ImgFormat2Ge2dFormat(format);
    	ge2d_config.dst_planes[0].addr=CMEM_getPhys(planes[3]);
    	ge2d_config.dst_planes[0].w= scale_w;
    	ge2d_config.dst_planes[0].h = scale_h;    
    	op_para.src1_rect.x = scale_x; 
    	op_para.src1_rect.y = scale_y; 
    	op_para.src1_rect.w = scale_w; 
    	op_para.src1_rect.h = scale_h;       	
    	op_para.dst_rect.x = scale_x;  
    	op_para.dst_rect.y = scale_y;  
    	op_para.dst_rect.w = scale_w;  
    	op_para.dst_rect.h = scale_h;    	    
        break;
        case 4:
        ge2d_config.src_dst_type = OSD0_OSD0;
        op_para.color=0x000000ff;
        ge2d_config.src_format = ImgFormat2Ge2dFormat(format);
        ge2d_config.dst_format = ImgFormat2Ge2dFormat(format);
    	op_para.src1_rect.x = scale_x; 
    	op_para.src1_rect.y = scale_y; 
    	op_para.src1_rect.w = scale_w; 
    	op_para.src1_rect.h = scale_h;       	
    	op_para.dst_rect.x = scale_x;  
    	op_para.dst_rect.y = scale_y;  
    	op_para.dst_rect.w = scale_w;  
    	op_para.dst_rect.h = scale_h;            
        break;
        default:
        break;                            
    }
	ioctl(fd_ge2d, FBIOPUT_GE2D_CONFIG, &ge2d_config);
   ioctl(fd_ge2d, FBIOPUT_GE2D_FILLRECTANGLE, &op_para);     
exit:       
	if(fd_ge2d >=0){
	    close(fd_ge2d);		
	    fd_ge2d = -1;
    }     
#ifdef JPEG_DBG       
    printf("finish clean plane buffer %d!!!!!\n", index);
#endif
    return 0;     
}


int compute_keep_ratio(jpeg_data_t  *jpeg_data,jpegdec_config_t *config)
{

    int image_width , image_height;
    int frame_left , frame_top ,frame_width, frame_height;
    int target_width , target_height;
    if(config->angle & 1){
        image_width = jpeg_data->info.height ;
        image_height = jpeg_data->info.width ;         
    }else{
        image_width = jpeg_data->info.width ;
        image_height = jpeg_data->info.height;          
    }
    frame_left = scale_x;
    frame_top =  scale_y;
    frame_width = scale_w;
    frame_height = scale_h ; 
    
    if((image_width * frame_height) > (image_height * frame_width)){
/*according with width*/        
        target_width = frame_width ;
        target_height = (float)(image_height*frame_width)/image_width;       
    }else{
        target_height = frame_height ;
        target_width = (float)(image_width*frame_height)/image_height;   
    }
    config->dec_x = 0 ; 
    config->dec_y = 0;
    config->dec_w = target_width ;
    config->dec_h = target_height ;
    if((image_width < frame_width)&&(image_height < frame_height)){        
        config->dec_w = image_width ;
        config->dec_h = image_height ;        
    }
    return 0;    
}
/**/
int compute_keep_ratio_by_expanding(jpeg_data_t  *jpeg_data,jpegdec_config_t *config)
{
    int image_width , image_height;
    int frame_left , frame_top ,frame_width, frame_height;
    int target_width , target_height;
    int w_limit ,h_limit;
    if(config->angle & 1){
        image_width = jpeg_data->info.height ;
        image_height = jpeg_data->info.width ;         
    }else{
        image_width = jpeg_data->info.width ;
        image_height = jpeg_data->info.height;          
    }
    frame_left = scale_x;
    frame_top =  scale_y;
    frame_width = scale_w;
    frame_height = scale_h ; 
    
    if((frame_width * image_height) > (frame_height * image_width)){
/*adjust height*/        
        target_width = frame_width ;
        target_height = (float)(image_height*frame_width)/image_width;       
    }else{
        target_height = frame_height ;
        target_width = (float)(image_width*frame_height)/image_height;   
    }    
    
    config->dec_x = 0 ; 
    config->dec_y = 0;
    config->dec_w = target_width ;
    config->dec_h = target_height ;
    if(config->angle & 1){
        w_limit = 1920;
        h_limit = 1920;   
    }else{
        w_limit = 1920;
        h_limit = 1920;      
    }
    if((target_width > w_limit)||(target_height > h_limit)){ 
#ifdef JPEG_DBG         
        printf("crop area exceed %d*%d!!!!!\n", target_width, target_height);       
#endif
        return -1;       
    }
    if((image_width <= frame_width)&&(image_height <= frame_height)){        
        config->dec_w = image_width ;
        config->dec_h = image_height ;        
    }else if((image_width < frame_width)||(image_height < frame_height)){
#ifdef JPEG_DBG         
        printf("crop function disable %d*%d!!!!!\n", image_width, image_height);       
#endif        
        return -1;    
    }    
    return 0;      
}
int rebuild_jpg_config_para(jpeg_data_t  *jpeg_data,jpegdec_config_t *config)
{
	int ret = 0;
	if((scale_w*jpeg_data->info.height)!= (scale_h*jpeg_data->info.width)){
	    sMode =  IGNOREASPECTRATIO;   	        
    }else{
        sMode =  KEEPASPECTRATIO;   	 
    }
#ifdef JPEG_DBG    
    printf("current sMode is %d\n",sMode);	
#endif    
	switch(sMode){
	    case KEEPASPECTRATIO:
	    ret = compute_keep_ratio(jpeg_data,config);
	    break;
	    case IGNOREASPECTRATIO:
	    ret = compute_keep_ratio_by_expanding(jpeg_data,config);
	    if(ret < 0){
	        ret = compute_keep_ratio(jpeg_data,config);    
	    }
	    break;
	    case KEEPASPECTRATIOBYEXPANDING:
	    ret = compute_keep_ratio_by_expanding(jpeg_data,config);
	    if(ret < 0){
	        ret = compute_keep_ratio(jpeg_data,config);    
	    }
	    break;
	    default:
	    break;    
	}  
	if(config->dec_h<2) {
		printf("too small to decode with hwjpeg decoder.\n");
		return -1;
	}	
	config->canvas_width = CANVAS_ALIGNED(config->dec_w);	
	planes[0] = (unsigned char *)CMEM_alloc(0, 
				 config->canvas_width * config->dec_h, &cmemParm);
	planes[1] = (unsigned char *)CMEM_alloc(0,
				CANVAS_ALIGNED((config->canvas_width/2)) *config->dec_h/2, &cmemParm);
	planes[2] = (unsigned char *)CMEM_alloc(0,
				CANVAS_ALIGNED((config->canvas_width/2)) * config->dec_h/2, &cmemParm);
	if ((!planes[0]) || (!planes[1]) || (!planes[2])) {
		printf("Not enough memory\n");
		if (planes[0])
			CMEM_free(planes[0], &cmemParm);
		if (planes[1])
			CMEM_free(planes[1], &cmemParm);
		if (planes[2])
			CMEM_free(planes[2], &cmemParm);
		return -1;
	}
	config->addr_y = CMEM_getPhys(planes[0]);
	config->addr_u = CMEM_getPhys(planes[1]);
	config->addr_v = CMEM_getPhys(planes[2]);
	
	if(config->dec_w==0 ||config->dec_h==0)
	{
		config->dec_w= jpeg_data->info.width;
		config->dec_h= jpeg_data->info.height;
	}
//	scaleSize(config->dec_w, config->dec_h, jpeg_data->info.width, jpeg_data->info.height, (Qt::AspectRatioMode)config->opt);
	config->opt = 0;
	config->dec_x = 0;
	config->dec_y = 0;
	config->angle = CLKWISE_0;
	clear_plane(0,config);
	clear_plane(1,config);
	clear_plane(2,config);
	return 0;	
}
unsigned int  get_decoder_state(int  handle)
{
	if(handle>0)
	{
		return ioctl(handle, JPEGDEC_IOC_STAT);
	}
	return JPEGDEC_STAT_ERROR;
}

char* scan_line(aml_image_info_t* info , int i)
{
    if((!info)||(!info->data)||(i > 1080)){
        return NULL;    
    }
    return (info->data + i*info->bytes_per_line);
}

int read_jpeg_data(int fd,jpeg_data_t  *jpeg_data,int op_max,jpegdec_config_t *config)
{
    struct am_io_param vb_info;
	int  read_num;
	unsigned int decState;
	int fd_jpegdec;
	int fd_amport=jpeg_data->fd_amport;
	
	lseek(fd ,0, SEEK_SET);  //seek device head .
	if(jpeg_data->buffer==NULL||fd_amport<0) return 0;
   
    fd_jpegdec= open(FILE_NAME_JPEGDEC, O_RDWR);
#ifdef JPEG_DBG     
    printf("fd_jpegdec= open(%s, O_RDWR)\n",FILE_NAME_JPEGDEC); 
#endif
    if(fd_jpegdec <0 ){
        perror("open amjpec device error\r\n")	;
        // 	  	printf("can't open jpeg relative device %s",qPrintable(dev));  	
        return 0;
    }	
	int  op_step=DEC_STAT_INFOCONFIG ;
	int  result=0;;
	int  read_unit=HEADER_SIZE;
	int total_size =0 ;
	int file_read_end = 0;
//	QDateTime time1;
//	QDateTime time2;			
	int wait_info_count  =0 ;
	int wait_timeout = 0 ;
//    time1= QDateTime::currentDateTime();
#ifdef JPEG_DBG     
	printf("decoder start\n");
#endif
	while(op_step < op_max)
	{
		decState=get_decoder_state( fd_jpegdec);
		result = decState;
		if (decState & JPEGDEC_STAT_ERROR) {
#ifdef JPEG_DBG 		    
			printf("jpegdec error\n");
#endif
			break;
		}

		if (decState & JPEGDEC_STAT_UNSUPPORT) {
#ifdef JPEG_DBG 		    
			printf("jpegdec unsupported format\n");
#endif			
			break;
		}

		if (decState & JPEGDEC_STAT_DONE) {
#ifdef JPEG_DBG 		    
			printf("jpegdec done\n");
#endif			
			break;
		}
		ioctl(fd_amport, AMSTREAM_IOC_VB_STATUS,&vb_info);		
		if((!file_read_end)&&(vb_info.status.data_len < ((4*vb_info.status.size)/5))&&(decState & JPEGDEC_STAT_WAIT_DATA)){
			read_num=read(fd,jpeg_data->buffer,read_unit );
			total_size += read_num;
			if(read_num<0)
			{
#ifdef JPEG_DBG 			    
				printf("can't read data from jpeg device");  
#endif				
				result= 0;
				break;
			}
			read_unit=MAX_BUFFER_SIZE;//expand buffer size to read real data.
			if(read_num==0) //file end then fill padding data into buffer.
			{
			    file_read_end = 1;
				read_num=read_unit=HEADER_SIZE;
				memset(jpeg_data->buffer,0,read_unit);
			}
			int ret = write( fd_amport, jpeg_data->buffer, read_num); 
			
		}	
		switch(op_step)
		{
			case DEC_STAT_INFOCONFIG:
			ioctl( fd_jpegdec, JPEGDEC_IOC_INFOCONFIG, decoder_opt);
			op_step=DEC_STAT_INFO;
			break;
			case DEC_STAT_INFO:
			if (decState & JPEGDEC_STAT_INFO_READY) {
				ioctl( fd_jpegdec, JPEGDEC_IOC_INFO, &jpeg_data->info);
#ifdef JPEG_DBG 				
				printf("++jpeg informations:w:%d,h:%d\r\n",jpeg_data->info.width,jpeg_data->info.height);
#endif				
				op_step=DEC_STAT_DECCONFIG;
				
			}else{
			    wait_info_count++;
			    if(wait_info_count > 5){
			        wait_info_count = 0 ;
//			        time2= QDateTime::currentDateTime(); 
//			        wait_timeout = abs(time2.secsTo(time1));		
#ifdef JPEG_DBG 			        	
			        printf("current timeout is %d!!!\n",wait_timeout);	        		        
#endif			        	        		        
			    }
			    if(wait_timeout > 3){
			        op_step = op_max;  
#ifdef JPEG_DBG 			        
			        printf("timeout for get jpeg info!!!\n");
#endif			        
			        break;  			        
			    }
#ifdef JPEG_DBG 			    
				printf("in jpeg decoding process\n");
#endif				
				result =0;
			}
			break;
			case DEC_STAT_DECCONFIG:
			if(config)
			{
				// first request mem from cmem.
				if(rebuild_jpg_config_para(jpeg_data,config)<0)
				{
#ifdef JPEG_DBG 				    
					printf("rebuild_cmem_config_para error");
#endif					
					op_step = op_max;
					result=0;
					continue;
				}
//#ifdef JPEG_DBG 				
//				printf("sending jpeg decoding config (%d-%d-%d-%d), planes(0x%x, 0x%x, 0x%x).\n",
//					config->dec_x, config->dec_y, config->dec_w, config->dec_h,
//					config->addr_y, config->addr_u, config->addr_v);
//#endif	
				if (ioctl(fd_jpegdec, JPEGDEC_IOC_DECCONFIG, config)<0) {
#ifdef JPEG_DBG 				    
					printf("decoder config failed\n");
#endif					
					op_step = op_max;
					result =0;
					continue;
				}	
				
			}
			op_step =DEC_STAT_RUN;
			break;
			default:
			break;	
		}
	}
#ifdef JPEG_DBG 	
	printf("decoder exit\n");
	printf("total read bytes is %d",total_size);  
#endif
	if( fd_jpegdec >0)
	{
		close(  fd_jpegdec);
		fd_jpegdec=-1;
	}
	
	return result;
		
}
aml_image_info_t* read_jpeg_image_rgb_test(char* url , int width, int height,int mode , int flag)
{    
    aml_image_info_t* output_image_info;
    output_image_info = (aml_image_info_t*)malloc(sizeof(aml_image_info_t));
    memset((char*)output_image_info , 0 , sizeof(aml_image_info_t));   
/*default para*/	
    if((width <= 0)||(height <=0)||(mode > 2) ){
    	 width = 1280;
    	 height = 720;		
    	 mode = KEEPASPECTRATIO;
	}

     int wait_timeout = 0 ;
     char* outimage = NULL; 

     int fd = open(url,O_RDWR ) ; 
     if(fd < 0){
        goto exit;   
     }
     printf("open 15.jpeg sucessfuly\n");
	//we need some step to decompress jpeg image to output 
	// 1  request yuv space from cmem to store decompressed data
	// 2  config and decompress jpg picture.
	// 3	 request  new rgb space from cmem to store output rgb data.
	// 4  ge2d move yuv data to rgb space.
	// 5  release request mem to cmem module.
	jpegdec_config_t  config;
	jpeg_data_t  jpeg_data;
	int fd_ge2d=-1;
	config_para_t ge2d_config;
	int format;
    	ge2d_op_para_t op_para;
	int bpl 	 ;

	memset((char*)&ge2d_config,0,sizeof(config_para_t));	
	scale_x = 0;
	scale_y = 0;
	scale_w = width;
	scale_h = height;	
/*default value for no scaler input*/	
#if 1
	if(scale_w>0 && scale_w<=200 && scale_h>0 && scale_h<=200)  {
		decoder_opt=JPEGDEC_OPT_THUMBNAIL_PREFERED;
	} else {
		decoder_opt=0;
	}
#else
	decoder_opt=0;
#endif
	if((scale_w == 0)||(scale_h ==0)){
	    scale_w  = 160 ;
	    scale_h  = 100;     
	}
	config.opt=(unsigned)sMode ;
	jpeg_data.fd_amport=fd_amport;
	if(!(JPEGDEC_STAT_DONE&read_jpeg_data(fd,&jpeg_data,DEC_STAT_MAX,&config)))
	{
#ifdef JPEG_DBG 	    
		printf("can't decode jpg pic");	
#endif			
		goto exit;
	}

#ifdef JPEG_DBG 	
	printf("deocde jpg pic completed");
#endif
	planes[3]=(unsigned char *)CMEM_alloc(0,CANVAS_ALIGNED(scale_w)*scale_h*4,&cmemParm);
	if(!planes[3])
	{
#ifdef JPEG_DBG 	    
		printf("can't get rgb memory from heap");
#endif		
		goto exit;
	}
#ifdef JPEG_DBG 	
	printf("planes[3]=(unsigned char *)CMEM_alloc(0,%d * %d *4,&cmemParm)\n",scale_w ,scale_h); 
#endif	
 	clear_plane(3,&config);

	//open fb device to handle ge2d op FILE_NAME_GE2D
    fd_ge2d= open(FILE_NAME_GE2D, O_RDWR);
//#ifdef JPEG_DBG     
//    printf("fd_ge2d= open(%s, O_RDWR)\n",dev.toLatin1().constData());
//#endif    
	if(fd_ge2d<0)
	{
#ifdef JPEG_DBG 	    
		printf("can't open framebuffer device" );  	
#endif			
		goto exit;
	}
/*antiflicking setting*/	
    if(flag){
	    ioctl(fd_ge2d,FBIOPUT_GE2D_ANTIFLICKER_ENABLE,1);    
	}else{
	    ioctl(fd_ge2d,FBIOPUT_GE2D_ANTIFLICKER_ENABLE,0);   
	}
	if(jpeg_data.info.comp_num==3 ||jpeg_data.info.comp_num==4)
	{
		format = Format_RGB32;
	}else{
#ifdef JPEG_DBG 	
		printf("unsupported color format" );  	
#endif		
		goto exit;
	}
#ifdef JPEG_DBG 	
	printf("start ge2d image format convert!!!!!\n");
#endif	
	ge2d_config.src_dst_type = ALLOC_ALLOC;
//    ge2d_config.src_dst_type = ALLOC_OSD1;        //only for test
	ge2d_config.alu_const_color=0xff0000ff;
	ge2d_config.src_format = GE2D_FORMAT_M24_YUV420;
	ge2d_config.dst_format = ImgFormat2Ge2dFormat(format);
	if(0xffffffff==ge2d_config.dst_format)
	{
#ifdef JPEG_DBG 	    
		printf("can't get proper ge2d format" );  	
#endif			
		goto exit;
	}

	ge2d_config.src_planes[0].addr = config.addr_y;
	ge2d_config.src_planes[0].w =    config.canvas_width;
	ge2d_config.src_planes[0].h =    config.dec_h;
	ge2d_config.src_planes[1].addr = config.addr_u;
	ge2d_config.src_planes[1].w =    config.canvas_width/2;
	ge2d_config.src_planes[1].h =    config.dec_h / 2;

	ge2d_config.src_planes[2].addr = config.addr_v;
	ge2d_config.src_planes[2].w = config.canvas_width/2;
	ge2d_config.src_planes[2].h = config.dec_h / 2;
	ge2d_config.dst_planes[0].addr=CMEM_getPhys(planes[3]);	
	ge2d_config.dst_planes[0].w=  scale_w;
	ge2d_config.dst_planes[0].h = scale_h;
//#ifdef JPEG_DBG 	
//	printf("planes[3]addr : 0x%x-0x%x" ,planes[3],ge2d_config.dst_planes[0].addr);		
//#endif	
	ioctl(fd_ge2d, FBIOPUT_GE2D_CONFIG, &ge2d_config);
/*crop case*/	
	if((config.dec_w > scale_w )||(config.dec_h > scale_h)){
    	op_para.src1_rect.x = (config.dec_w - scale_w)>>1;
    	op_para.src1_rect.y = (config.dec_h - scale_h)>>1;
    	op_para.src1_rect.w = scale_w;
    	op_para.src1_rect.h = scale_h;
    	op_para.dst_rect.x = 0;
    	op_para.dst_rect.y = 0;
    	op_para.dst_rect.w = scale_w;
    	op_para.dst_rect.h = scale_h;		    
	}else{	
    	op_para.src1_rect.x = config.dec_x;
    	op_para.src1_rect.y = config.dec_y;
    	op_para.src1_rect.w = config.dec_w;
    	op_para.src1_rect.h = config.dec_h;
    	op_para.dst_rect.x = (scale_w - config.dec_w )>>1;
    	op_para.dst_rect.y = (scale_h - config.dec_h )>>1;
    	op_para.dst_rect.w = config.dec_w;
    	op_para.dst_rect.h = config.dec_h;
    }
#ifdef JPEG_DBG     
   printf("alloc_alloc:srcx :%d  : srcy :%d srcw :%d srch :%d" ,op_para.src1_rect.x,op_para.src1_rect.y,op_para.src1_rect.w,op_para.src1_rect.h);	
   printf("alloc_alloc:dstx :%d  : dsty :%d dstw :%d dsth :%d" ,op_para.dst_rect.x,op_para.dst_rect.y,op_para.dst_rect.w,op_para.dst_rect.h);	
#endif
    ioctl(fd_ge2d, FBIOPUT_GE2D_STRETCHBLIT_NOALPHA, &op_para); 
//    bpl = nextMulOf8(bytesPerPixel(format) *scale_w);
#if 0 
	ge2d_config.src_format = ImgFormat2Ge2dFormat(format);
	ge2d_config.dst_format = ImgFormat2Ge2dFormat(format);    
    ge2d_config.src_dst_type = ALLOC_OSD1;	
	ge2d_config.src_planes[0].addr=CMEM_getPhys(planes[3]);	
	ge2d_config.src_planes[0].w=  scale_w;
	ge2d_config.src_planes[0].h = scale_h;  
	ioctl(fd_ge2d, FBIOPUT_GE2D_CONFIG, &ge2d_config);  
	op_para.src1_rect.x = op_para.dst_rect.x;
	op_para.src1_rect.y = op_para.dst_rect.y;
	op_para.src1_rect.w = op_para.dst_rect.w ;
	op_para.src1_rect.h = op_para.dst_rect.h ;
#ifdef JPEG_DBG     
   printf("alloc_osd0: srcx :%d  : srcy :%d srcw :%d srch :%d" ,op_para.src1_rect.x,op_para.src1_rect.y,op_para.src1_rect.w,op_para.src1_rect.h);	
   printf("alloc_osd0:dstx :%d  : dsty :%d dstw :%d dsth :%d" ,op_para.dst_rect.x,op_para.dst_rect.y,op_para.dst_rect.w,op_para.dst_rect.h);	
#endif	
    ioctl(fd_ge2d, FBIOPUT_GE2D_STRETCHBLIT_NOALPHA, &op_para); 
#endif
	if(planes[3])
	{
		CMEM_free(planes[3], &cmemParm);
		planes[3] = NULL;
#ifdef JPEG_DBG 		
		printf("free planes[3]\n" );  
#endif		
	}
   
exit:	
	if (planes[0]){
			CMEM_free(planes[0], &cmemParm);
		planes[0] = NULL;
#ifdef JPEG_DBG 
		printf("free planes[0]\n" );  
#endif		
	}
	if (planes[1]){
			CMEM_free(planes[1], &cmemParm);
		planes[1] = NULL;
#ifdef JPEG_DBG 		
		printf("free planes[1]\n" );  
#endif		
	}
	if (planes[2]){	    
			CMEM_free(planes[2], &cmemParm);
		planes[2] = NULL;
#ifdef JPEG_DBG 		
		printf("free planes[2]\n" );  
#endif		
    }
	if(planes[3]){
		CMEM_free(planes[3], &cmemParm);
		planes[3] = NULL;
#ifdef JPEG_DBG 		
		printf("free planes[3]\n" );  
#endif			 
	}	
	if(fd_ge2d >=0){
	    close(fd_ge2d);		
	    fd_ge2d = -1;
    }
    if(fd >=0){
        close(fd);    
    }
    if(!output_image_info->data){
        if(output_image_info){
            free(output_image_info);   
            output_image_info = NULL; 
        } 
    }
	return output_image_info;
}

aml_image_info_t* read_jpeg_image(char* url , int width, int height,int mode , int flag)
{    
    char* outimage = NULL; 
    aml_image_info_t* input_image_info;
    aml_image_info_t* output_image_info;   
    int input_image_width;    
    char* input_data;
    char* output_data;
    int i;
/*default para*/	
    if((width <= 0)||(height <=0)||(mode > 2) ){
    	 width = 1280;
    	 height = 720;		
    	 mode = KEEPASPECTRATIO;
	}
/*RGB antiflicking test*/	
	if(flag == 2){
	    output_image_info = read_jpeg_image_rgb_test(url,width, height, mode ,flag); 
	    return    output_image_info;
	}
	input_image_info = (aml_image_info_t*)malloc(sizeof(aml_image_info_t));
    output_image_info = (aml_image_info_t*)malloc(sizeof(aml_image_info_t));
    memset((char*)output_image_info , 0 , sizeof(aml_image_info_t));	
     int wait_timeout = 0 ;
     int fd = open(url,O_RDWR ) ; 
     if(fd < 0){
        goto exit;   
     }
     printf("open %s sucessfuly\n",url);
	//we need some step to decompress jpeg image to output 
	// 1  request yuv space from cmem to store decompressed data
	// 2  config and decompress jpg picture.
	// 3	 request  new rgb space from cmem to store output rgb data.
	// 4  ge2d move yuv data to rgb space.
	// 5  release request mem to cmem module.
	jpegdec_config_t  config;
	jpeg_data_t  jpeg_data;
	int fd_ge2d=-1;
	config_para_t ge2d_config;
	int format;
    	ge2d_op_para_t op_para;
	int bpl 	 ;

	memset((char*)&ge2d_config,0,sizeof(config_para_t));	
	scale_x = 0;
	scale_y = 0;
	scale_w = width;
	scale_h = height;	
/*default value for no scaler input*/	
#if 1
	if(scale_w>0 && scale_w<=200 && scale_h>0 && scale_h<=200)  {
		decoder_opt=JPEGDEC_OPT_THUMBNAIL_PREFERED;
	} else {
		decoder_opt=0;
	}
#else
	decoder_opt=0;
#endif
	if((scale_w == 0)||(scale_h ==0)){
	    scale_w  = 160 ;
	    scale_h  = 100;     
	}
	config.opt=(unsigned)sMode ;
	jpeg_data.fd_amport=fd_amport;
	if(!(JPEGDEC_STAT_DONE&read_jpeg_data(fd,&jpeg_data,DEC_STAT_MAX,&config)))
	{
#ifdef JPEG_DBG 	    
		printf("can't decode jpg pic\n");	
#endif			
		goto exit;
	}

#ifdef JPEG_DBG 	
	printf("deocde jpg pic completed\n");
#endif
    input_image_width = CANVAS_ALIGNED(scale_w);
	planes[3]=(unsigned char *)CMEM_alloc(0,input_image_width*scale_h*4,&cmemParm);
	if(!planes[3])
	{
#ifdef JPEG_DBG 	    
		printf("can't get rgb memory from heap\n");
#endif		
		goto exit;
	}
#ifdef JPEG_DBG 	
	printf("planes[3]=(unsigned char *)CMEM_alloc(0,%d * %d *4,&cmemParm)\n",scale_w ,scale_h); 
#endif	
 	clear_plane(3,&config);

	//open fb device to handle ge2d op FILE_NAME_GE2D
    fd_ge2d= open(FILE_NAME_GE2D, O_RDWR);
//#ifdef JPEG_DBG     
//    printf("fd_ge2d= open(%s, O_RDWR)\n",dev.toLatin1().constData());
//#endif    
	if(fd_ge2d<0)
	{
#ifdef JPEG_DBG 	    
		printf("can't open framebuffer device\n" );  	
#endif			
		goto exit;
	}
/*antiflicking setting*/	
    if(flag){
	    ioctl(fd_ge2d,FBIOPUT_GE2D_ANTIFLICKER_ENABLE,1);    
	}else{
	    ioctl(fd_ge2d,FBIOPUT_GE2D_ANTIFLICKER_ENABLE,0);   
	}
	if(jpeg_data.info.comp_num==3 ||jpeg_data.info.comp_num==4)
	{
		format = Format_RGB32;
	}else{
#ifdef JPEG_DBG 	
		printf("unsupported color format\n" );  	
#endif		
		goto exit;
	}
#ifdef JPEG_DBG 	
	printf("start ge2d image format convert!!!!!\n");
#endif	
	ge2d_config.src_dst_type = ALLOC_ALLOC;
//    ge2d_config.src_dst_type = ALLOC_OSD1;        //only for test
	ge2d_config.alu_const_color=0xff0000ff;
	ge2d_config.src_format = GE2D_FORMAT_M24_YUV420;
	ge2d_config.dst_format = ImgFormat2Ge2dFormat(format);
	if(0xffffffff==ge2d_config.dst_format)
	{
#ifdef JPEG_DBG 	    
		printf("can't get proper ge2d format\n" );  	
#endif			
		goto exit;
	}

	ge2d_config.src_planes[0].addr = config.addr_y;
	ge2d_config.src_planes[0].w =    config.canvas_width;
	ge2d_config.src_planes[0].h =    config.dec_h;
	ge2d_config.src_planes[1].addr = config.addr_u;
	ge2d_config.src_planes[1].w =    config.canvas_width/2;
	ge2d_config.src_planes[1].h =    config.dec_h / 2;

	ge2d_config.src_planes[2].addr = config.addr_v;
	ge2d_config.src_planes[2].w = config.canvas_width/2;
	ge2d_config.src_planes[2].h = config.dec_h / 2;
	ge2d_config.dst_planes[0].addr=CMEM_getPhys(planes[3]);	
//	ge2d_config.dst_planes[0].w=  scale_w;
    ge2d_config.dst_planes[0].w= input_image_width ;
	ge2d_config.dst_planes[0].h = scale_h;
//#ifdef JPEG_DBG 	
//	printf("planes[3]addr : 0x%x-0x%x" ,planes[3],ge2d_config.dst_planes[0].addr);		
//#endif	
	ioctl(fd_ge2d, FBIOPUT_GE2D_CONFIG, &ge2d_config);
/*crop case*/	
	if((config.dec_w > scale_w )||(config.dec_h > scale_h)){
    	op_para.src1_rect.x = (config.dec_w - scale_w)>>1;
    	op_para.src1_rect.y = (config.dec_h - scale_h)>>1;
    	op_para.src1_rect.w = scale_w;
    	op_para.src1_rect.h = scale_h;
    	op_para.dst_rect.x = 0;
    	op_para.dst_rect.y = 0;
    	op_para.dst_rect.w = scale_w;
    	op_para.dst_rect.h = scale_h;		    
	}else{	
    	op_para.src1_rect.x = config.dec_x;
    	op_para.src1_rect.y = config.dec_y;
    	op_para.src1_rect.w = config.dec_w;
    	op_para.src1_rect.h = config.dec_h;
    	op_para.dst_rect.x = (scale_w - config.dec_w )>>1;
    	op_para.dst_rect.y = (scale_h - config.dec_h )>>1;
    	op_para.dst_rect.w = config.dec_w;
    	op_para.dst_rect.h = config.dec_h;
    }
#ifdef JPEG_DBG     
   printf("alloc_alloc:srcx :%d  : srcy :%d srcw :%d srch :%d\n" ,op_para.src1_rect.x,op_para.src1_rect.y,op_para.src1_rect.w,op_para.src1_rect.h);	
   printf("alloc_alloc:dstx :%d  : dsty :%d dstw :%d dsth :%d\n" ,op_para.dst_rect.x,op_para.dst_rect.y,op_para.dst_rect.w,op_para.dst_rect.h);	
#endif
    ioctl(fd_ge2d, FBIOPUT_GE2D_STRETCHBLIT_NOALPHA, &op_para); 
//    bpl = nextMulOf8(bytesPerPixel(format) *scale_w);

    printf("start generate output image\n");
    input_image_info->width = scale_w;
    input_image_info->height = scale_h;
    input_image_info->depth = 32;
    input_image_info->bytes_per_line = input_image_width << 2 ;
    input_image_info->nbytes = input_image_info->bytes_per_line * height;
    input_image_info->data  = (char*)planes[3] ;
    
    output_image_info->width = scale_w;
    output_image_info->height = scale_h;
    output_image_info->depth = 32;
    output_image_info->bytes_per_line = ((output_image_info->width * output_image_info->depth + 31) >> 5 ) << 2 ;
    output_image_info->nbytes = output_image_info->bytes_per_line * height;
    output_image_info->data  = malloc(output_image_info->nbytes);
    if(!output_image_info->data){
        goto exit;    
    }else{
        for(i = 0 ; i < output_image_info->height ; i++){
            input_data =   scan_line(input_image_info , i);
            output_data =  scan_line(output_image_info , i);       
            memcpy(output_data, input_data , 4* output_image_info->width );
        }                
    }   
#if 0 
    if(flag){  //enable display
    #ifdef JPEG_DBG 
        printf("start test output image\n");
    #endif    
        int length = 4* output_image_info->width;
        for(i = 0 ; i < input_image_info->height ; i++){
            input_data =   scan_line(output_image_info , i);
            output_data =  scan_line(input_image_info , i);       
            memcpy(output_data , input_data  , 4* output_image_info->width );
            if(i&8){
                printf("src is %x, dst is %x, length is %d\n",output_data,input_data,length) ;   
            }
        } 
    	ge2d_config.src_format = ImgFormat2Ge2dFormat(format);
    	ge2d_config.dst_format = ImgFormat2Ge2dFormat(format);    
        ge2d_config.src_dst_type = ALLOC_OSD1;	
    	ge2d_config.src_planes[0].addr=CMEM_getPhys(planes[3]);	
    	ge2d_config.src_planes[0].w=  input_image_width;
    	ge2d_config.src_planes[0].h = scale_h;  
    	ioctl(fd_ge2d, FBIOPUT_GE2D_CONFIG, &ge2d_config);  
    	op_para.src1_rect.x = op_para.dst_rect.x;
    	op_para.src1_rect.y = op_para.dst_rect.y;
    	op_para.src1_rect.w = op_para.dst_rect.w ;
    	op_para.src1_rect.h = op_para.dst_rect.h ;	
        ioctl(fd_ge2d, FBIOPUT_GE2D_STRETCHBLIT_NOALPHA, &op_para);      
    }   
#endif   
//#ifdef JPEG_DBG     
//    printf("outimage address is %x\n" ,(unsigned)outImage);  
//#endif    
	if(planes[3])
	{
		CMEM_free(planes[3], &cmemParm);
		planes[3] = NULL;
#ifdef JPEG_DBG 		
		printf("free planes[3]\n" );  
#endif		
	}
   
exit:	
	if (planes[0]){
			CMEM_free(planes[0], &cmemParm);
		planes[0] = NULL;
#ifdef JPEG_DBG 
		printf("free planes[0]\n" );  
#endif		
	}
	if (planes[1]){
			CMEM_free(planes[1], &cmemParm);
		planes[1] = NULL;
#ifdef JPEG_DBG 		
		printf("free planes[1]\n" );  
#endif		
	}
	if (planes[2]){	    
			CMEM_free(planes[2], &cmemParm);
		planes[2] = NULL;
#ifdef JPEG_DBG 		
		printf("free planes[2]\n" );  
#endif		
    }
	if(planes[3]){
		CMEM_free(planes[3], &cmemParm);
		planes[3] = NULL;
#ifdef JPEG_DBG 		
		printf("free planes[3]\n" );  
#endif			 
	}	
	if(fd_ge2d >=0){
	    close(fd_ge2d);		
	    fd_ge2d = -1;
    }
    if(fd >=0){
        close(fd);    
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
	return output_image_info;
}
