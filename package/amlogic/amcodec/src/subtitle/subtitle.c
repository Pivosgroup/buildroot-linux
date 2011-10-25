/************************************************
 * name	:subtitle.c
 * function	:decoder relative functions
 * data		:2010.8.11
 * author		:FFT
 * version	:1.0.0
 *************************************************/
 //header file
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "subtitle/subtitle.h"
#include "vob_sub.h"

static unsigned short DecodeRL(unsigned short RLData,unsigned short *pixelnum,unsigned short *pixeldata)
{
	unsigned short nData = RLData;
	unsigned short nShiftNum;
	unsigned short nDecodedBits;
	
	if(nData & 0xc000) 
		nDecodedBits = 4;
	else if(nData & 0x3000) 
		nDecodedBits = 8;
	else if(nData & 0x0c00) 
		nDecodedBits = 12;
	else 
		nDecodedBits = 16;
	
	nShiftNum = 16 - nDecodedBits;
	*pixeldata = (nData >> nShiftNum) & 0x0003;
	*pixelnum = nData >> (nShiftNum + 2);
	
	return nDecodedBits;	
}

static unsigned short GetWordFromPixBuffer(unsigned short bitpos, unsigned short *pixelIn)
{
	unsigned char hi=0, lo=0, hi_=0, lo_=0;
	char *tmp = (char *)pixelIn;

	hi = *(tmp+0);
	lo = *(tmp+1);
	hi_ = *(tmp+2);
	lo_ = *(tmp+3);

	if(bitpos == 0){
		return (hi<<0x8 | lo);
	}
	else {
		return(((hi<<0x8 | lo) << bitpos) | ((hi_<<0x8 | lo_)>>(16 - bitpos)));
	}
}

unsigned char spu_fill_pixel(unsigned short *pixelIn, char *pixelOut, AML_SPUVAR *sub_frame)
{
	unsigned short nPixelNum = 0,nPixelData = 0;
	unsigned short nRLData,nBits;
	unsigned short nDecodedPixNum = 0;
	unsigned short i, j;
	unsigned short PXDBufferBitPos	= 0,WrOffset = 16;
	unsigned short change_data = 0;
    unsigned short PixelDatas[4] = {0,1,2,3};
	unsigned short rownum = sub_frame->spu_width;
	unsigned short height = sub_frame->spu_height;
	
	static unsigned short *ptrPXDWrite;
        
	memset(pixelOut, 0, VOB_SUB_SIZE/2);
	ptrPXDWrite = (unsigned short *)pixelOut;

	for (j=0; j<height/2; j++) {
		while(nDecodedPixNum < rownum){
			nRLData = GetWordFromPixBuffer(PXDBufferBitPos, pixelIn);
			nBits = DecodeRL(nRLData,&nPixelNum,&nPixelData);

			PXDBufferBitPos += nBits;
			if(PXDBufferBitPos >= 16){
				PXDBufferBitPos -= 16;
				pixelIn++;
			}
			if(nPixelNum == 0){
				nPixelNum = rownum - nDecodedPixNum%rownum;
			}
            
    		if(change_data)
    		{
                nPixelData = PixelDatas[nPixelData];
    		}
            
			for(i = 0;i < nPixelNum;i++){
				WrOffset -= 2;
				*ptrPXDWrite |= nPixelData << WrOffset;
				if(WrOffset == 0){
					WrOffset = 16;
					ptrPXDWrite++;
				}
			}
			nDecodedPixNum += nPixelNum;
		}	

		if(PXDBufferBitPos == 4) {			 //Rule 6
			PXDBufferBitPos = 8;
		}
		else if(PXDBufferBitPos == 12){
			PXDBufferBitPos = 0;
			pixelIn++;
		}
		
		if (WrOffset != 16) {
		    WrOffset = 16;
		    ptrPXDWrite++;
		}

		nDecodedPixNum -= rownum;

	}

	return 0;
}


int get_spu(AML_SPUVAR *spu, int sub_fd)
{
	int ret, rd_oft, wr_oft, size;
	char *spu_buf=NULL;
	unsigned current_length, current_pts, current_type;

	ret = codec_poll_sub_fd(sub_fd, 10);
	if (ret == 0)
		goto error; 

	size = codec_get_sub_size_fd(sub_fd);
	if (size <= 0)
		goto error; 
	else
		spu_buf = malloc(size);	

	ret = codec_read_sub_data_fd(sub_fd, spu_buf, size);
	

	rd_oft = 0;
	if ((spu_buf[rd_oft++]!=0x41)||(spu_buf[rd_oft++]!=0x4d)||
		(spu_buf[rd_oft++]!=0x4c)||(spu_buf[rd_oft++]!=0x55)|| (spu_buf[rd_oft++]!=0xaa))
		goto error; 		// wrong head
		
	current_type = spu_buf[rd_oft++]<<16;
	current_type |= spu_buf[rd_oft++]<<8;
	current_type |= spu_buf[rd_oft++];

	current_length = spu_buf[rd_oft++]<<24;
	current_length |= spu_buf[rd_oft++]<<16;
	current_length |= spu_buf[rd_oft++]<<8;
	current_length |= spu_buf[rd_oft++];	
	
	current_pts = spu_buf[rd_oft++]<<24;
	current_pts |= spu_buf[rd_oft++]<<16;
	current_pts |= spu_buf[rd_oft++]<<8;
	current_pts |= spu_buf[rd_oft++];

	if (current_pts==0)
		goto error;

	switch (current_type) {
		case 0x17000:
			spu->spu_data = malloc(VOB_SUB_SIZE);
			spu->pts = current_pts;
			ret = get_vob_spu(spu_buf+rd_oft, current_length, spu); 
			break;

		default:
			break;
	}

error:
	if (spu_buf)
		free(spu_buf);
	
	return 0;
}

int release_spu(AML_SPUVAR *spu)
{
	if(spu->spu_data)
		free(spu->spu_data);
}

/*****test code*********
int test_sub_data(int fd)
{ 
	int ret, size; 
	char *buf=NULL; 

	if (fd >= 0){ 		
		ret = codec_poll_sub_fd(fd, 10); 
		if (ret != 0){ 
			size = codec_get_sub_size_fd(fd); 
			if (size > 0) 
				buf = malloc(size); 
			ret = codec_read_sub_data_fd(fd, buf, size); 
			if (buf)
				free(buf);
		}
		
		//codec_close_sub_fd(fd); 
	} 	
	return 0; 
}


int test_sub_data2(void)
{ 
	int fd = -1;
	char *buf=NULL; 

	fd = codec_open_sub_read(); 
	if (fd >= 0){ 
		codec_close_sub_fd(fd); 
	} 	
	return 0; 
}
*/


