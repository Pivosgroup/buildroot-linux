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
 *  crop.c
 */
#include "crop.h"

/*frame resolution*/
int kFramePixelWidth =0;
int kFramePixelHeight =0;

/*frame size ratio*/
float kFrameAspectRatio =0;
float kMaxCropAllowed = 0.5;
int   kTallerLossPolicy  = 20;

float kPixelAspectRatio =0;
unsigned PixelAspectHigh =0;
unsigned PixelAspectLow=0;
static int pixel_adjust = 1;
float RATIO_TARGET = 1.0;

/* stretch policy controller */ 
float kMaxStretchAllowed = 0.3;

/*Compute dimension_t to display image with no cropping of image*/
int compute_full_image(int image_width, int image_height, int image_rotation,int frame_rotation, dimension_t *image_dimension,
 dimension_t *frame_dimension,float* ratio_x,float* ratio_y,unsigned char* ratio_method)
{
    float image_aspect_ratio;
    frame_rotation = 0 ;

    image_aspect_ratio = ((float) image_height) / ((float) image_width);

    if (image_aspect_ratio >= kFrameAspectRatio)
    {
        // Image is taller than display
        image_dimension->height = image_height;
        image_dimension->width = image_width;

        *ratio_y = (float)image_height/kFramePixelHeight;
		*ratio_x =  (float)(image_height*PixelAspectLow)/(kFramePixelHeight*PixelAspectHigh) ;

		if((image_dimension->width*kFramePixelHeight) < (0xffffffff/(float)PixelAspectHigh)){				
//		if(image_width < 1000){
		    frame_dimension->width =   ((image_dimension->width * kFramePixelHeight*PixelAspectHigh)+ (image_height*PixelAspectLow -1 ))/(image_height*PixelAspectLow);
	    }else{
        frame_dimension->width =   image_dimension->width/(*ratio_x);
	    }
		frame_dimension->height =  (image_dimension->height*kFramePixelHeight +(image_height -1))/image_height;
        *ratio_method |= 0x1;/*set bit 0*/
    }
    else
    {
        // Image is wider than display
        image_dimension->height = image_height;
        image_dimension->width = image_width;

        *ratio_x =  (float)image_width/kFramePixelWidth ;
		*ratio_y = (float)(image_width*PixelAspectHigh)/(kFramePixelWidth*PixelAspectLow);
		frame_dimension->width =   (image_dimension->width*kFramePixelWidth + (image_width - 1))/image_width;

		if((image_dimension->height*kFramePixelWidth) < (0xffffffff/(float)PixelAspectLow)){		
//		if(image_height < 1000){
		    frame_dimension->height =  ((image_dimension->height*kFramePixelWidth*PixelAspectLow)+(image_width*PixelAspectHigh -1))/(image_width*PixelAspectHigh);	
		}else{
        frame_dimension->height =  image_dimension->height/(*ratio_y);
	    }
        *ratio_method &= ~0x1 ;
    } // else
	
    image_dimension->top = 0;
    image_dimension->left = 0;
    frame_dimension->top = 0;
    frame_dimension->left = 0;
    return 0;
}
