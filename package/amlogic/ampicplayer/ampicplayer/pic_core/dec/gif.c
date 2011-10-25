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
#include "pic_app.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <setjmp.h>
#include <gif_lib.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define min(a,b) ((a) < (b) ? (a) : (b))
#define gflush return(FH_ERROR_FILE);
#define grflush { DGifCloseFile(gft); return(FH_ERROR_FORMAT); }
#define mgrflush { free(lb); free(slb); DGifCloseFile(gft); return(FH_ERROR_FORMAT); }
#define agflush return(FH_ERROR_FORMAT);
#define agrflush { DGifCloseFile(gft); return(FH_ERROR_FORMAT); }


int fh_gif_id(char *name)
{
    int fd;
    char id[4];
    fd=open(name,O_RDONLY); if(fd==-1) return(0);
    read(fd,id,4);
    close(fd);
    if(id[0]=='G' && id[1]=='I' && id[2]=='F') return(1);
    return(0);
}

inline void m_rend_gif_decodecolormap(unsigned char *cmb,unsigned char *rgbb,ColorMapObject *cm,int s,int l, int transparency)
{
    GifColorType *cmentry;
    int i;
    for(i=0;i<l;i++)
    {
	cmentry=&cm->Colors[cmb[i]];
	*(rgbb++)=cmentry->Red;
	*(rgbb++)=cmentry->Green;
	*(rgbb++)=cmentry->Blue;
    }
}


/* Thanks goes here to Mauro Meneghin, who implemented interlaced GIF files support */
//int fh_gif_load(char *name,unsigned char *buffer, unsigned char ** alpha, int x,int y)
int fh_gif_load(aml_dec_para_t* para , aml_image_info_t* image)
{
  int in_nextrow[4]={8,8,4,2};   //interlaced jump to the row current+in_nextrow
  int in_beginrow[4]={0,4,2,1};  //begin pass j from that row number
  int transparency=-1;  //-1 means not transparency present
    int px,py,i,ibxs;
    int j;
    char *fbptr;
    char *lb;
    char *slb;
    GifFileType *gft;
    GifByteType *extension;
    int extcode;
    GifRecordType rt;
    ColorMapObject *cmap;
    int cmaps;
    char *p,*q;
    int buf_len;
    char* name;
    char* buffer;
    char* alpha = NULL;
    int x,y;    
    name = para->fn;
    buffer = (unsigned char*) malloc(para->iwidth * para->iheight * 4);
    buf_len = para->iwidth * para->iheight;
    x = para->iwidth;
    y = para->iheight;
    gft=DGifOpenFileName(name);
    if(gft==NULL){printf("err5\n"); gflush;} //////////
    do
    {
	if(DGifGetRecordType(gft,&rt) == GIF_ERROR) grflush;
	switch(rt)
	{
	    case IMAGE_DESC_RECORD_TYPE:
		if(DGifGetImageDesc(gft)==GIF_ERROR) grflush;
		px=gft->Image.Width;
		py=gft->Image.Height;
		lb=(char*)malloc(px*3);
		slb=(char*) malloc(px);
//  printf("reading...\n");
		if(lb!=NULL && slb!=NULL)
		{
			unsigned char *alphaptr = NULL;
			
		    cmap=(gft->Image.ColorMap ? gft->Image.ColorMap : gft->SColorMap);
		    cmaps=cmap->ColorCount;

		    ibxs=ibxs*3;
		    fbptr=(char*)buffer;
			
			if(transparency != -1)
			{
				alphaptr = malloc(px * py);
				alpha = alphaptr;
			}
			
		    if(!(gft->Image.Interlace))
		    {
			for(i=0;i<py;i++,fbptr+=px*3)
			{
				int j;
			    if(DGifGetLine(gft,(GifPixelType*)slb,px)==GIF_ERROR) mgrflush;
			    m_rend_gif_decodecolormap((unsigned char*)slb,(unsigned char*)lb,cmap,cmaps,px,transparency);
			    memcpy(fbptr,lb,px*3);
				if(alphaptr)
					for(j = 0; j<px; j++) *(alphaptr++) = (((unsigned char*) slb)[j] == transparency) ? 0x00 : 0xff;
        		}
                    }
                    else
		    {
				unsigned char * aptr = NULL;
				
	               for(j=0;j<4;j++)
	               {
						int k;
				        if(alphaptr)
							aptr = alphaptr + (in_beginrow[j] * px);
							
					    fbptr=(char*)buffer + (in_beginrow[j] * px * 3);

					    for(i = in_beginrow[j]; i<py; i += in_nextrow[j], fbptr += px * 3 * in_nextrow[j], aptr += px * in_nextrow[j])
			    {
				if(DGifGetLine(gft,(GifPixelType*)slb,px)==GIF_ERROR) mgrflush; /////////////
				m_rend_gif_decodecolormap((unsigned char*)slb,(unsigned char*)lb,cmap,cmaps,px,transparency);
				memcpy(fbptr,lb,px*3);
				if(alphaptr)
					for(k = 0; k<px; k++) aptr[k] = (((unsigned char*) slb)[k] == transparency) ? 0x00 : 0xff;
          		    }
			}
		    }
		}
		if(lb) free(lb);
		if(slb) free(slb);
                break;
	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(gft,&extcode,&extension)==GIF_ERROR) grflush; //////////
		if(extcode==0xf9) //look image transparency in graph ctr extension
		{
			if(extension[1] & 1)
			{
				transparency = extension[4];

			}
//		    tran_off=(int)*extension;
//		    transparency=(int)*(extension+tran_off);
//			printf("transparency: %d\n", transparency);
                }
		while(extension!=NULL)
		    if(DGifGetExtensionNext(gft,&extension) == GIF_ERROR) grflush
		break;
	    default:
		break;
	}
    }
    while( rt!= TERMINATE_RECORD_TYPE );
    DGifCloseFile(gft);
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



int fh_gif_getsize(char *name,int *x,int *y)
{
    int px,py;
    GifFileType *gft;
    GifByteType *extension;
    int extcode;
    GifRecordType rt;

    gft=DGifOpenFileName(name);
    if(gft==NULL) gflush;
    do
    {
	if(DGifGetRecordType(gft,&rt) == GIF_ERROR) grflush;
	switch(rt)
	{
	    case IMAGE_DESC_RECORD_TYPE:

		if(DGifGetImageDesc(gft)==GIF_ERROR) grflush;
		px=gft->Image.Width;
		py=gft->Image.Height;
		*x=px; *y=py;
		DGifCloseFile(gft);
		return(FH_ERROR_OK);
		break;
	    case EXTENSION_RECORD_TYPE:
		if(DGifGetExtension(gft,&extcode,&extension)==GIF_ERROR) grflush;
		while(extension!=NULL)
		    if(DGifGetExtensionNext(gft,&extension)==GIF_ERROR) grflush;
		break;
	    default:
		break;
	}  
    }
    while( rt!= TERMINATE_RECORD_TYPE );
    DGifCloseFile(gft);
    return(FH_ERROR_FORMAT);
}
