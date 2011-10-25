/************************************************
 * name	:vob_sub.c
 * function	:decoder relative functions
 * data		:2010.8.10
 * author		:FFT
 * version	:1.0.0
 *************************************************/
 //header file
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//#include "codec.h"
#include "subtitle/subtitle.h"
#include "vob_sub.h"


unsigned short doDCSQC(unsigned char *pdata,unsigned char *pend)
{
    unsigned short cmdDelay,cmdDelaynew;
    unsigned short temp;
    unsigned short cmdAddress;
    int Done,stoped;
    
    cmdDelay = *pdata++;
    cmdDelay <<= 8;
    cmdDelay += *pdata++;
    
    cmdAddress = *pdata++;
    cmdAddress <<= 8;
    cmdAddress += *pdata++;   
    
    cmdDelaynew = 0;
    
    Done = 0;
    stoped = 0;
    
    while(!Done)
    {
        switch(*pdata)
        {
            case FSTA_DSP:
                pdata++;
            	break;
            case STA_DSP:
                pdata++;
            	break;
            case STP_DSP:
                pdata++;
                stoped = 1;
            	break;
            case SET_COLOR:
                pdata+=3;
            	break;
            case SET_CONTR:
                pdata+=3;
            	break;
            case SET_DAREA:
				pdata+=7;
            	break;
            case SET_DSPXA:
                pdata += 7;
            	break;
            case CHG_COLCON:
                temp = *pdata++;
            	temp = temp<<8;
                temp += *pdata++;    
                pdata += temp;
            	break;
            case CMD_END:
                pdata++;
                Done = 1;
                break;
            default:
                pdata = pend;
                Done = 1;
                break;
         }
    }

    if((pdata < pend) && (stoped==0))
        cmdDelaynew = doDCSQC(pdata,pend);

    return cmdDelaynew>cmdDelay?cmdDelaynew:cmdDelay;
}

static int get_spu_cmd(AML_SPUVAR *sub_frame)
{
	unsigned short temp;
    unsigned char *pCmdData;
    unsigned char *pCmdEnd;
    unsigned char data_byte0, data_byte1;
    unsigned char spu_cmd;    


    if(sub_frame->cmd_offset >= sub_frame->length)
        return -1;		//cmd offset > frame size
    
    pCmdData = (unsigned char*)(sub_frame->spu_data);

    pCmdEnd = pCmdData+sub_frame->length;
    pCmdData += sub_frame->cmd_offset;
    pCmdData += 4;
    
    while(pCmdData<pCmdEnd)
    {
        spu_cmd = *pCmdData++;
        
        switch(spu_cmd)
        {
            case FSTA_DSP:
                sub_frame->display_pending = 2;
            	break;
            case STA_DSP:
            	sub_frame->display_pending = 1;
            	break;
            case STP_DSP:
            	sub_frame->display_pending = 0;
            	break;
            case SET_COLOR:
                temp = *pCmdData++;
            	sub_frame->spu_color = temp<<8;
                temp = *pCmdData++;
                sub_frame->spu_color += temp;
            	break;
            case SET_CONTR:
                temp = *pCmdData++;
            	sub_frame->spu_alpha = temp<<8;
                temp = *pCmdData++;
                sub_frame->spu_alpha += temp;
            	break;
            case SET_DAREA:
            	data_byte0 = *pCmdData++;
            	data_byte1 = *pCmdData++;
                sub_frame->spu_start_x = ((data_byte0 & 0x3f) << 4)|(data_byte1 >> 4);
    	        data_byte0 = *pCmdData++;
                sub_frame->spu_width = ((data_byte1 & 0x03) << 8)|(data_byte0);
                sub_frame->spu_width = sub_frame->spu_width - sub_frame->spu_start_x+1;
            	data_byte0 = *pCmdData++;
            	data_byte1 = *pCmdData++;
                sub_frame->spu_start_y = ((data_byte0 & 0x3f) << 4)|(data_byte1 >> 4);
    	        data_byte0 = *pCmdData++;
                sub_frame->spu_height= ((data_byte1 & 0x03) << 8) |(data_byte0);
                sub_frame->spu_height = sub_frame->spu_height - sub_frame->spu_start_y+1;
            
            	if((sub_frame->spu_width > 720) ||
                    (sub_frame->spu_height > 576)
                    )
            	{
                    sub_frame->spu_width = 720;
                    sub_frame->spu_height = 576;
            	}
             
            	break;
            case SET_DSPXA:
                temp = *pCmdData++;
            	sub_frame->top_pxd_addr = temp<<8;
                temp = *pCmdData++;
                sub_frame->top_pxd_addr += temp;
                
                temp = *pCmdData++;
            	sub_frame->bottom_pxd_addr = temp<<8;
                temp = *pCmdData++;
                sub_frame->bottom_pxd_addr += temp;

            	break;
            case CHG_COLCON:
                temp = *pCmdData++;
            	temp = temp<<8;
                temp += *pCmdData++;    
                pCmdData += temp;
                /*
		            	uVobSPU.disp_colcon_addr = uVobSPU.point + uVobSPU.point_offset;
		            	uVobSPU.colcon_addr_valid = 1;
		                temp = uVobSPU.disp_colcon_addr + temp - 2;	

		            	uSPU.point = temp & 0x1fffc;
		            	uSPU.point_offset = temp & 3;
				*/
            	break;
            case CMD_END:
                if(pCmdData<=(pCmdEnd-6))
                {
                    if((sub_frame->m_delay = doDCSQC(pCmdData,pCmdEnd-6))>0)
                        sub_frame->m_delay = sub_frame->m_delay*1024+sub_frame->pts;
                }
                return 0;
                break;
            default:
                return -1;
    	 }
    }
    return -1;
}

int get_vob_spu(char *spu_buf, unsigned length, AML_SPUVAR *spu)
{
	int rd_oft, wr_oft, i;
	unsigned current_length = length;
	
	char *pixDataOdd = NULL;
	char *pixDataEven = NULL;
	unsigned short *ptrPXDRead;

	rd_oft = 0;
	spu->length = spu_buf[0]<<8;
	spu->length |= spu_buf[1];
	spu->cmd_offset = spu_buf[2]<<8;
	spu->cmd_offset |= spu_buf[3];

	memset(spu->spu_data, 0, VOB_SUB_SIZE);
	wr_oft = 0;
	
	while (spu->length-rd_oft > 0){
		if (!current_length) {
			if ((spu_buf[rd_oft++]!=0x41)||(spu_buf[rd_oft++]!=0x4d)||
				(spu_buf[rd_oft++]!=0x4c)||(spu_buf[rd_oft++]!=0x55)|| (spu_buf[rd_oft++]!=0xaa))
				goto error; 		// wrong head				

				rd_oft += 3; 			// 3 bytes for type				
				current_length = spu_buf[rd_oft++]<<24;
				current_length |= spu_buf[rd_oft++]<<16;
				current_length |= spu_buf[rd_oft++]<<8;
				current_length |= spu_buf[rd_oft++];	
			
				rd_oft += 4;			// 4 bytes for pts
		}
		if ((wr_oft+current_length) <= spu->length){
			memcpy(spu->spu_data+wr_oft, spu_buf+rd_oft, current_length);
			rd_oft += current_length;
			wr_oft += current_length;
			current_length = 0;
		}
		if (wr_oft == spu->length){
			get_spu_cmd(spu);
			spu->frame_rdy = 1;
			break;
		}			
	}


	// if one frame data is ready, decode it.
	if (spu->frame_rdy == 1){
		pixDataOdd = malloc(VOB_SUB_SIZE/2);
		if (!pixDataOdd)
			goto error; 		//not enough memory
		ptrPXDRead = (unsigned short *)(spu->spu_data + spu->top_pxd_addr);
		spu_fill_pixel(ptrPXDRead, pixDataOdd, spu);	

		pixDataEven = malloc(VOB_SUB_SIZE/2);
		if (!pixDataEven)
			goto error; 		//not enough memory
		ptrPXDRead = (unsigned short *)(spu->spu_data + spu->bottom_pxd_addr);
		spu_fill_pixel(ptrPXDRead, pixDataEven, spu); 

		memset(spu->spu_data, 0, VOB_SUB_SIZE);
		for (i=0; i<VOB_SUB_SIZE; i+=spu->spu_width/2){
			memcpy(spu->spu_data+i, pixDataOdd+i/2, spu->spu_width/4);
			memcpy(spu->spu_data+i+spu->spu_width/4, pixDataEven+i/2, spu->spu_width/4);
		}
	}

error:
	if (pixDataOdd)
		free(pixDataOdd);
	if (pixDataEven)
		free(pixDataEven);

	return 0;
}



