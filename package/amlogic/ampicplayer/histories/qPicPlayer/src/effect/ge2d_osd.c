/******************************************************
*
*	operation for ge2d utils.
*
*******************************************************/
#include <unistd.h>
#include <stdio.h>      // jpeglib needs this to be pre-included
#include <sys/ioctl.h>
#include <setjmp.h>
#include <fcntl.h>
#include "./amllib/includes.h"
#include "ge2d_osd.h"

#define CANVAS_ALIGNED(x)	(((x) + 7) & ~7)
#define   FILE_NAME_GE2D		  "/dev/ge2d"

static config_para_t ge2d_config={0};
static ge2d_op_para_t op_para={0};
static CMEM_AllocParams cmemParm = {CMEM_HEAP, CMEM_NONCACHED, 8};
static int ge2d_dev_fd=-1;
static inline unsigned blendop(unsigned color_blending_mode,
						   unsigned color_blending_src_factor,
						   unsigned color_blending_dst_factor,
						   unsigned alpha_blending_mode,
						   unsigned alpha_blending_src_factor,
						   unsigned alpha_blending_dst_factor)
{
	return (color_blending_mode << 24) |
		   (color_blending_src_factor << 20) |
		   (color_blending_dst_factor << 16) |
		   (alpha_blending_mode << 8) |
		   (alpha_blending_src_factor << 4) |
		   (alpha_blending_dst_factor << 0);
};

int init_ge2d() {
	ge2d_config.src_dst_type = OSD1_OSD1;
	if(ge2d_dev_fd>0) return -1;
	ge2d_dev_fd=open(FILE_NAME_GE2D,O_RDWR);
	if(ge2d_dev_fd<0) printf("----------------ge2d for transition error---------------------\n");
}

void uninit_ge2d() {
	if(ge2d_dev_fd>=0) {		
		close(ge2d_dev_fd);
		ge2d_dev_fd=-1;
	}
}

void fb_blend(screen_area_t* src1_pos, screen_area_t* src2_pos, screen_area_t* dst_pos,int factor) {
	if(ge2d_dev_fd<0) return;
	
	/* set configuration of ge2d device. */
	ge2d_config.alu_const_color=(unsigned)factor;
	ioctl(ge2d_dev_fd, FBIOPUT_GE2D_CONFIG, &ge2d_config);
	
	op_para.src1_rect.x	=	src1_pos->x;
	op_para.src1_rect.y	=	src1_pos->y;
	op_para.src1_rect.w	=	src1_pos->width;
	op_para.src1_rect.h	=	src1_pos->height;

	op_para.src2_rect.x	=	src2_pos->x;
	op_para.src2_rect.y	=	src2_pos->y;
	op_para.src2_rect.w	=	src2_pos->width;
	op_para.src2_rect.h	=	src2_pos->height;

	op_para.dst_rect.x	=	dst_pos->x;
	op_para.dst_rect.	y=	dst_pos->y;
	op_para.dst_rect.	w=	dst_pos->width;
	op_para.dst_rect.	h=	dst_pos->height;

	op_para.op=blendop( 
			OPERATION_ADD,
                    /* color blending src factor */
                    COLOR_FACTOR_CONST_ALPHA,
                    /* color blending dst factor */
                    COLOR_FACTOR_ONE_MINUS_CONST_ALPHA,
                    /* alpha blending_mode */
                    OPERATION_ADD,
                    /* alpha blending src factor */
                    ALPHA_FACTOR_ONE,
                    /* color blending dst factor */
                    ALPHA_FACTOR_ONE);
	
	ioctl(ge2d_dev_fd, FBIOPUT_GE2D_BLEND, &op_para); 
	
}

void fb_bitbld(screen_area_t* src_pos, screen_area_t* dst_pos) {
	if(ge2d_dev_fd<0) return;
	
	/* set configuration of ge2d device. */
	ge2d_config.alu_const_color	=	0xff;
	ioctl(ge2d_dev_fd, FBIOPUT_GE2D_CONFIG, &ge2d_config);
	
	op_para.src1_rect.x	=	src_pos->x;
	op_para.src1_rect.y	=	src_pos->y;
	op_para.src1_rect.w	=	src_pos->width;
	op_para.src1_rect.h	=	src_pos->height;
	
	op_para.dst_rect.x	=	dst_pos->x;
	op_para.dst_rect.	y=	dst_pos->y;
	op_para.dst_rect.	w=	dst_pos->width;
	op_para.dst_rect.	h=	dst_pos->height;

	if(op_para.dst_rect.w==op_para.src1_rect.w&&op_para.dst_rect.h==op_para.src1_rect.h)
		ioctl(ge2d_dev_fd, FBIOPUT_GE2D_BLIT, &op_para); 
	else
		ioctl(ge2d_dev_fd, FBIOPUT_GE2D_STRETCHBLIT, &op_para); 
}

void fb_fillrect(screen_area_t* dst_pos, unsigned argb) {
	if(ge2d_dev_fd<0) return;
	
	/* set configuration of ge2d device. */
	ge2d_config.alu_const_color	=	0xff;
	ioctl(ge2d_dev_fd, FBIOPUT_GE2D_CONFIG, &ge2d_config);
	
	op_para.dst_rect.x	=	dst_pos->x;
	op_para.dst_rect.y=	dst_pos->y;
	op_para.dst_rect.w=	dst_pos->width;
	op_para.dst_rect.h=	dst_pos->height;
	op_para.src1_rect.x	=	dst_pos->x;
	op_para.src1_rect.y=	dst_pos->y;
	op_para.src1_rect.w=	dst_pos->width;
	op_para.src1_rect.h=	dst_pos->height;
	
	ioctl(ge2d_dev_fd, FBIOPUT_GE2D_FILLRECTANGLE, &op_para); 
}