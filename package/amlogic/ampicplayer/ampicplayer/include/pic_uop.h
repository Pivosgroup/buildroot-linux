
/* picture operation. */

#ifndef __PIC_UOP_H___
#define __PIC_UOP_H___

/* from the pic core. */
unsigned char * simple_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
unsigned char * alpha_resize(unsigned char * alpha,int ox,int oy,int dx,int dy);
unsigned char * color_average_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
unsigned char * rotate(unsigned char *i, int ox, int oy, int rot);
unsigned char * alpha_rotate(unsigned char *i, int ox, int oy, int rot);

static inline void do_rotate(aml_image_info_t *i, int rot)
{
	if(rot)
	{
		unsigned char *image, *alpha = NULL;
		int t;
		
		image = rotate(i->rgb, i->width, i->height, rot);
		if(i->alpha)
			alpha = alpha_rotate(i->alpha, i->width, i->height, rot);
		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}
		
		i->rgb = image;
		i->alpha = alpha;
		i->do_free = 1;
		
		if(rot & 1)
		{
			t = i->width;
			i->width = i->height;
			i->height = t;
		}
	}
}


static inline void do_enlarge(aml_image_info_t *i, int screen_width, int screen_height, int ignoreaspect)
{
	if(((i->width > screen_width) || (i->height > screen_height)) && (!ignoreaspect))
		return;
	if((i->width < screen_width) || (i->height < screen_height))
	{
		int xsize = i->width, ysize = i->height;
		unsigned char * image, * alpha = NULL;
		
		if(ignoreaspect)
		{
			if(i->width < screen_width)
				xsize = screen_width;
			if(i->height < screen_height)
				ysize = screen_height;
			
			goto have_sizes;
		}
		
		if((i->height * screen_width / i->width) <= screen_height)
		{
			xsize = screen_width;
			ysize = i->height * screen_width / i->width;
			goto have_sizes;
		}
		
		if((i->width * screen_height / i->height) <= screen_width)
		{
			xsize = i->width * screen_height / i->height;
			ysize = screen_height;
			goto have_sizes;
		}
		return;
have_sizes:
		image = simple_resize(i->rgb, i->width, i->height, xsize, ysize);
		if(i->alpha)
			alpha = alpha_resize(i->alpha, i->width, i->height, xsize, ysize);
		
		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}
		
		i->rgb = image;
		i->alpha = alpha;
		i->do_free = 1;
		i->width = xsize;
		i->height = ysize;
	}
}


static inline void do_fit_to_screen(aml_image_info_t *i, int screen_width, int screen_height, int ignoreaspect, int cal)
{
	if((i->width > screen_width) || (i->height > screen_height))
	{
		unsigned char * new_image, * new_alpha = NULL;
		int nx_size = i->width, ny_size = i->height;
		
		if(ignoreaspect)
		{
			if(i->width > screen_width)
				nx_size = screen_width;
			if(i->height > screen_height)
				ny_size = screen_height;
		}
		else
		{
			if((i->height * screen_width / i->width) <= screen_height)
			{
				nx_size = screen_width;
				ny_size = i->height * screen_width / i->width;
			}
			else
			{
				nx_size = i->width * screen_height / i->height;
				ny_size = screen_height;
			}
		}
		
		if(cal)
			new_image = color_average_resize(i->rgb, i->width, i->height, nx_size, ny_size);
		else
			new_image = simple_resize(i->rgb, i->width, i->height, nx_size, ny_size);
		
		if(i->alpha)
			new_alpha = alpha_resize(i->alpha, i->width, i->height, nx_size, ny_size);
		
		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}
		
		i->rgb = new_image;
		i->alpha = new_alpha;
		i->do_free = 1;
		i->width = nx_size;
		i->height = ny_size;
	}
}

#endif