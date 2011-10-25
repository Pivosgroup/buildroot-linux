/*
    fbv  --  simple image viewer for the linux framebuffer
    Copyright (C) 2000, 2001, 2003  Mateusz Golicz

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

#include <config.h>
#include <png.h>
#include "pic_app.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PNG_BYTES_TO_CHECK 4
#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif

int fh_png_id(char *name)
{
    int fd;
    char id[4];
    fd=open(name,O_RDONLY); if(fd==-1) return(0);
    read(fd,id,4);
    close(fd);
    if(id[1]=='P' && id[2]=='N' && id[3]=='G') return(1);
    return(0);
}
			    

//int fh_png_load(char *name,unsigned char *buffer, unsigned char ** alpha,int x,int y)
int fh_png_load(aml_dec_para_t* para , aml_image_info_t* image)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height;
    int i;
    int bit_depth, color_type, interlace_type;
    int number_passes,pass, trans = 0;
    char *rp;
    png_bytep rptr[2];
    char *fbptr;
    FILE *fh;
    char *p,*q;
    int buf_len;
    char* name;
    char* buffer;
    char* alpha = NULL;
    int x,y;    
    name = para->fn;
    buffer = (unsigned char*) malloc(para->iwidth * para->iheight * 4);
    buf_len = para->iwidth * para->iheight  ;
    x = para->iwidth;
    y = para->iheight;
    if(!(fh=fopen(name,"rb"))) return(FH_ERROR_FILE);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (png_ptr == NULL) return(FH_ERROR_FORMAT);
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fh); return(FH_ERROR_FORMAT);
    }
    rp=0;
    if (setjmp(png_ptr->jmpbuf))
    {
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        if(rp) free(rp);
	fclose(fh); return(FH_ERROR_FORMAT);
    }

    png_init_io(png_ptr,fh);

    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,&interlace_type, NULL, NULL);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr); 
    if (bit_depth < 8) png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type== PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		trans = 1;
		png_set_tRNS_to_alpha(png_ptr);
	}

    if(bit_depth == 16) png_set_strip_16(png_ptr); 
    number_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr,info_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA || color_type == PNG_COLOR_TYPE_RGB_ALPHA || trans)
	{
		unsigned char * alpha_buffer = (unsigned char*) malloc(width * height);
		unsigned char * aptr;
		
    	rp = (char*) malloc(width * 4);
	    rptr[0] = (png_bytep) rp;
    
		alpha = alpha_buffer;
		
	    for (pass = 0; pass < number_passes; pass++)
    	{
			fbptr = buffer;
			aptr = alpha_buffer;
			
			for(i=0; i<height; i++)
    		{
				int n;
				unsigned char *trp = rp;
				
    	   		png_read_rows(png_ptr, rptr, NULL, 1);
				
				for(n = 0; n < width; n++, fbptr += 3, trp += 4)
				{
					memcpy(fbptr, trp, 3);
					*(aptr++) = trp[3];
				}
			}
    	}
	    free(rp);
	}
	else
	{
	    for (pass = 0; pass < number_passes; pass++)
    	{
			fbptr = buffer;
			for(i=0; i<height; i++, fbptr += width*3)
    		{
			    rptr[0] = (png_bytep) fbptr;
    	   		png_read_rows(png_ptr, rptr, NULL, 1);
			}
    	}
	}
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fh);
    p = buffer;
    q = alpha ;    
    for(i = buf_len ; i >0 ;i--){
        if(!q){
            p[4*i - 1] = 0xff; 
        }else{
            p[4*i - 1] = q[i-1];         //a
        }
        p[4*i - 2] = p[3*i - 3];     //r
        p[4*i - 3] = p[3*i - 2];     //g
        p[4*i - 4] = p[3*i - 1];     //b
    }      
    image->data = buffer;
    image->width = x;
    image->height = y;
    image->depth = 32;
    image->bytes_per_line = x << 2 ;
    image->nbytes = image->bytes_per_line * y;        
    return(FH_ERROR_OK);
}
int fh_png_getsize(char *name,int *x,int *y)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    char *rp;
    FILE *fh;

    if(!(fh=fopen(name,"rb"))) return(FH_ERROR_FILE);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (png_ptr == NULL) return(FH_ERROR_FORMAT);
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fh); return(FH_ERROR_FORMAT);
    }
    rp=0;
    if (setjmp(png_ptr->jmpbuf))
    {
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        if(rp) free(rp);
	fclose(fh); return(FH_ERROR_FORMAT);
    }
   
    png_init_io(png_ptr,fh);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,&interlace_type, NULL, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    *x=width;
    *y=height;
    fclose(fh);
    return(FH_ERROR_OK);
}
