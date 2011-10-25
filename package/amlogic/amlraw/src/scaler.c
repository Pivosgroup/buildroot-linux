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
 *  scaler.c
 */
#include <stdlib.h>
//#include "scaler_common.h"
#include "scaler.h"
#include "crop.h"

static int canvas_width = 1280 ;
static int canvas_height = 720 ;
#define _max(a,b) (a)>(b)?(a):(b)

void draw_pixel(simp_scaler_t *scaler , int  x, int y,unsigned char a,unsigned char r,unsigned char g,unsigned char b)
{

    int offset = y * scaler->input.bpp + x*4;
    unsigned char* p = &scaler->input.output[offset];
//    p[0] = a;
//    p[1] = r;
//    p[2] = g;
//    p[3] = b;
    p[0] = b;
    p[1] = g;
    p[2] = r;
    p[3] = a;
//    scaler->input.output[offset] = (a<<24)|(r<<16)|(g<<8)|b;
}

static void
build_scale_map(simp_scaler_t *scaler)
{
    int i;
    unsigned image_height,image_width,frame_height,frame_width,temp, scaled_top, scaled_left;
    unsigned crop_left = 0, crop_top = 0;
    float ratio_x, ratio_y, temp_f;
    int video_fd;
    int dynamic_frame_enable = 1;
    dimension_t image_dimension, frame_dimension;

    image_height = scaler->input.pic_height;
    image_width = scaler->input.pic_width;
    frame_height = scaler->input.frame_height;
    frame_width = scaler->input.frame_width;            
	scaler->input.config_width  = frame_width;
	scaler->input.config_height  = frame_height ;
	scaler->input.config_screen_width = INTI_SCREEN_WIDTH;
	scaler->input.config_screen_height = INIT_SCREEN_HEIGHT;
	
    kPixelAspectRatio = (((float)scaler->input.config_width/(float)scaler->input.config_screen_width)/((float)scaler->input.config_height/(float)scaler->input.config_screen_height));	
    
    kFrameAspectRatio = kPixelAspectRatio*frame_height/frame_width;

    PixelAspectHigh = scaler->input.config_screen_height * scaler->input.config_width ; 
    PixelAspectLow = scaler->input.config_screen_width*scaler->input.config_height ;
    
    kMaxCropAllowed = 0.26;
    
    kFramePixelWidth  = frame_width;
    kFramePixelHeight = frame_height;
    
    if ((scaler->input.rotation == PIC_DEC_DIR_90) || (scaler->input.rotation == PIC_DEC_DIR_270)) {
        temp = image_height;    /* swap width and height to calc the scale ratio */
        image_height = image_width;
        image_width = temp;
    }
    compute_full_image(image_width, image_height, scaler->input.rotation, PIC_DEC_DIR_0, &image_dimension, &frame_dimension, &ratio_x, &ratio_y, &scaler->input.ratio_method);
    
    if ((scaler->input.rotation == PIC_DEC_DIR_90) || (scaler->input.rotation == PIC_DEC_DIR_270))
    {
        temp_f = ratio_x;
        ratio_x = ratio_y;
        ratio_y = temp_f;

        temp = image_dimension.height;
        image_dimension.height = image_dimension.width;
        image_dimension.width  = temp;

        temp = image_dimension.top;
        image_dimension.top = image_dimension.left;
        image_dimension.left  = temp;     
        scaler->input.ratio_method |= 0x2; /*set bit 1*/
    }
    else
    {
        scaler->input.ratio_method &= ~0x2; /*clear bit 1*/
    }
    
    //ratio
    ratio_x = 1 / ratio_x;
    ratio_y = 1 / ratio_y;
    
    /* build X map */
    for (i=0;i<scaler->input.pic_width+1;i++){
        scaler->map_x[2*i+1] = (unsigned)(i * ratio_x + 0.5);       /* target X for each input */
    }

    for (i=0;i<scaler->input.pic_width;i++){
        scaler->map_x[2*i] = scaler->map_x[2*i+3] - scaler->map_x[2*i+1];
    }

    /* build Y map */
    for (i=0;i<scaler->input.pic_height+1;i++){
        scaler->map_y[2*i+1] = (unsigned)(i * ratio_y + 0.5);       /* target X for each input */
    }

    for (i=0;i<scaler->input.pic_height;i++){
        scaler->map_y[2*i] = scaler->map_y[2*i+3] - scaler->map_y[2*i+1];
    }

    /* note:
     * ratio_x/ratio_y is in the same space with the input image, so (x,y) works for the input
     * and the map_x/map_y also works for the input(x,y), and scaled_width/scaled_height
     * is same. The width/height is the same direction as the input, but scaled_top/scaled_left
     * is the screen based position.
     */
    scaler->input.scaled_width  = image_dimension.width  * ratio_x;
    scaler->input.scaled_height = image_dimension.height * ratio_y;
	scaler->input.pic_width = image_dimension.width;
	scaler->input.pic_height = image_dimension.height;
    if (image_dimension.left) scaler->input.pic_left  = image_dimension.left;
	else image_dimension.left = 0;
    if (image_dimension.top) scaler->input.pic_top    = image_dimension.top;
	else image_dimension.top = 0;

	if ((scaler->input.rotation == PIC_DEC_DIR_90) || (scaler->input.rotation == PIC_DEC_DIR_270)) {
		if(scaler->input.frame_height >= scaler->input.scaled_width) {
			scaler->input.scaled_top = scaler->input.frame_top + (scaler->input.frame_height - scaler->input.scaled_width) / 2;
		}
		else {
			scaler->input.scaled_top = scaler->input.frame_top;
		}
		
		if(scaler->input.frame_width >= scaler->input.scaled_height) {
			scaler->input.scaled_left = scaler->input.frame_left + (scaler->input.frame_width - scaler->input.scaled_height) / 2;
		}
		else {
			scaler->input.scaled_left = scaler->input.frame_left;
		}
	}
	else {
		if(scaler->input.frame_height >= scaler->input.scaled_height) {
			scaler->input.scaled_top = scaler->input.frame_top + (scaler->input.frame_height - scaler->input.scaled_height) / 2;
		}
		else {
			scaler->input.scaled_top = scaler->input.frame_top;
		}

		if(scaler->input.frame_width >= scaler->input.scaled_width) {
			scaler->input.scaled_left = scaler->input.frame_left + (scaler->input.frame_width - scaler->input.scaled_width) / 2;
		}
		else {
			scaler->input.scaled_left = scaler->input.frame_left;
		}

        scaler->input.scaled_top  = (scaler->input.scaled_top >> 1) << 1;
        scaler->input.scaled_left = (scaler->input.scaled_left >> 1) << 1;
    }
    scaler->input.display_ratio = (float)scaler->input.scaled_width/(float)scaler->input.scaled_height;
}

static void
pixel_write_hd_t( simp_scaler_t *scaler, unsigned char r, 
                    unsigned char g,unsigned char b, unsigned char a , int image_x, int image_y)
{
    int x,y,temp_x, temp_y;  
    for (y=0; y<scaler->map_y[2*(image_y-scaler->input.pic_top)]; y++){

        unsigned screen_y = scaler->map_y[2*(image_y-scaler->input.pic_top)+1] + y;
        for (x=0; x<scaler->map_x[2*(image_x-scaler->input.pic_left)]; x++){

            unsigned screen_x = scaler->map_x[2*(image_x-scaler->input.pic_left)+1] + x;
            unsigned char *p;

			switch (scaler->input.rotation) {
				default:
				case PIC_DEC_DIR_0:
					temp_x = screen_x;
					temp_y = screen_y;
					break;

				case PIC_DEC_DIR_90:
					temp_x = scaler->input.scaled_height - screen_y - 1;
					temp_y = screen_x;
					break;

				case PIC_DEC_DIR_180:
					temp_x = scaler->input.scaled_width - screen_x - 1;
					temp_y = scaler->input.scaled_height - screen_y - 1;
					break;

				case PIC_DEC_DIR_270:
					temp_x = screen_y;
					temp_y = scaler->input.scaled_width - screen_x - 1;
					break;
			}

			temp_x += scaler->input.scaled_left;
			temp_y += scaler->input.scaled_top;

			draw_pixel(scaler ,temp_x,temp_y,255,r,g,b);
        }
    }
}

static void
pixel_write_hd_b( simp_scaler_t *scaler, unsigned char r, 
                    unsigned char g,unsigned char b,unsigned char a , int image_x, int image_y)
{
    int x,y,temp_x, temp_y;
    for (y=0; y<scaler->map_y[2*(image_y-scaler->input.pic_top)]; y++){

        unsigned screen_y = scaler->map_y[2*(image_y-scaler->input.pic_top)+1] + y;
        for (x=0; x<scaler->map_x[2*(image_x-scaler->input.pic_left)]; x++){

            unsigned screen_x = scaler->map_x[2*(image_x-scaler->input.pic_left)+1] + x;
            unsigned char *p;

			switch (scaler->input.rotation) {
				default:
				case PIC_DEC_DIR_0:
					temp_x = screen_x;
					temp_y = scaler->input.scaled_height - screen_y - 1;
					break;

				case PIC_DEC_DIR_270:
					temp_x = scaler->input.scaled_height - screen_y - 1;
					temp_y = scaler->input.scaled_width - screen_x - 1;
					break;

				case PIC_DEC_DIR_180:
					temp_x = scaler->input.scaled_width - screen_x - 1;
					temp_y = screen_y;
					break;

				case PIC_DEC_DIR_90:
					temp_x = screen_y;
					temp_y = screen_x;
					break;
			}

			temp_x += scaler->input.scaled_left;
			temp_y += scaler->input.scaled_top;
			
			draw_pixel(scaler , temp_x,temp_y,255,r,g,b);
        }
    }
}

int
simp_scaler_init(simp_scaler_t *scaler)
{
    unsigned size;

    size = _max(scaler->input.pic_width, scaler->input.pic_height);
    scaler->map_x = (unsigned short *)malloc((size * 2 + 2) * 2);
    if (!scaler->map_x) {
        return -1;
    }

    scaler->map_y = (unsigned short *)malloc((size * 2 + 2) * 2);
    if (!scaler->map_y) {
        free(scaler->map_x);
        scaler->map_x = NULL;
        return -1;
    }

    build_scale_map(scaler);

	if (scaler->input.output_bt) {
		scaler->pixel_write = pixel_write_hd_b;
	}
	else{
		scaler->pixel_write = pixel_write_hd_t;
	}

    return 0;    
}

void
simp_scaler_output_pixel(   simp_scaler_t *scaler, unsigned char r, unsigned char g,
                                    unsigned char b, unsigned char a , int image_x, int image_y)
{
    scaler->pixel_write(scaler, r,g,b, a ,image_x, image_y);
}

void
simp_scaler_flush(simp_scaler_t *scaler)
{
    return;
}

void
simp_scaler_destroy(simp_scaler_t *scaler)
{
    if (scaler->map_x) {
        free(scaler->map_x);
        scaler->map_x = NULL;
    }
    if (scaler->map_y) {
        free(scaler->map_y);
        scaler->map_y = NULL;
    }
}

void 	get_canvas(int* w,int* h) {
   *w = canvas_width ;
   *h = canvas_height ;
}
void set_canvas(int w , int h)
{
    canvas_width = w ;
    canvas_height = h;   
}
void pre_out_scaler() {
//	pre_out();
} 

void post_out_scaler() {
//	post_out();
}
