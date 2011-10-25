/***************************************************
 * name	: player.c
 * function:  player thread, include all player functions
 * date		:  2010.2.2
 ***************************************************/

 //header file
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h> 

#include <codec.h>
#include <message.h>
#include <player_set_sys.h>

//private
#include "player_priv.h"
#include "player_av.h"
#include "player_update.h"
#include "player_hwdec.h"
#include "thread_mgt.h"
#include "stream_decoder.h"

#define BREAK_FLAG      0x01
#define CONTINUE_FLAG   0x02
#define NONO_FLAG       0x00

static int player_para_release(play_para_t *para)
{
    int i;
    
    if(para->vstream_info.has_video)
    {
        for(i = 0; i < para->media_info.stream_info.total_video_num; i ++)
        {
            if(para->media_info.video_info[i] != NULL)
        	{
        		FREE(para->media_info.video_info[i]);
                para->media_info.video_info[i] = NULL;
        	}
        }
	
    }
   
    if(para->astream_info.has_audio)
    {
        for(i = 0; i < para->media_info.stream_info.total_audio_num; i ++)
        {
            if(para->media_info.audio_info[i] != NULL)
        	{
                if(para->stream_type == STREAM_AUDIO)
                {
                    if(para->media_info.audio_info[i]->audio_tag)
                    {
                        FREE(para->media_info.audio_info[i]->audio_tag);
                        para->media_info.audio_info[i]->audio_tag = NULL;
                    }
                }
        		FREE(para->media_info.audio_info[i]);
                para->media_info.audio_info[i] = NULL;
        	}
        }
    }
    
    if(para->sstream_info.has_sub)
    {
        for(i = 0; i < para->media_info.stream_info.total_sub_num; i ++)
        {
            if(para->media_info.sub_info[i] != NULL)
        	{                
        		FREE(para->media_info.sub_info[i]);
                para->media_info.sub_info[i] = NULL;
        	}
        }
    }    	
    if(para->astream_info.adts_header != NULL)
    {
        FREE(para->astream_info.adts_header);
        para->astream_info.adts_header = NULL;
    }
	if(para->pFormatCtx!=NULL)
	{
		av_close_input_file(para->pFormatCtx);	
		para->pFormatCtx = NULL; 
	}     
    if(para->decoder && para->decoder->release)
	{
    	para->decoder->release(para);
    	para->decoder=NULL;
	}      
    return PLAYER_SUCCESS;
}

static int check_decoder_worksta(play_para_t *para)
{      
    #define PARSER_ERROR_WRONG_PACKAGE_SIZE 0x80
    #define PARSER_ERROR_WRONG_HEAD_VER     0x40	
    codec_para_t *codec;
    struct vdec_status vdec;   
    static unsigned int last_time=0;
    int ret;
    if ( para->vstream_info.video_format == VFORMAT_SW )
    	return PLAYER_SUCCESS;
    if(para->vstream_info.has_video)
    {
        if(para->vcodec)
            codec = para->vcodec;
        else if(para->codec)
            codec = para->codec;
        if(codec)
        {
            ret = codec_get_vdec_state(codec, &vdec);
            if(ret!=0)    	
        	{	
        		log_error("pid:[%d]::codec_get_vdec_state error: %x\n",para->player_id, -ret);				
        		return PLAYER_CHECK_CODEC_ERROR;
        	}
            else
            {             
                if(vdec.status & (0x20) && 
                    ((para->state.current_time - last_time)>=1) && 
                    (!para->playctrl_info.read_end_flag))
                {        
                    last_time = para->state.current_time;
                    if(vdec.status & (PARSER_ERROR_WRONG_PACKAGE_SIZE|PARSER_ERROR_WRONG_HEAD_VER))
                    {     
                        if(para->playctrl_info.need_reset)
                        {
                            para->playctrl_info.time_point = para->state.current_time;                          
                            para->playctrl_info.need_reset = 0; 
                            log_print("pid[%d]::[%s:%d]time=%d ret=%d\n",para->player_id,__FUNCTION__, __LINE__,para->playctrl_info.time_point,ret);

                        }
                        else
                        {
                            para->playctrl_info.time_point = para->state.current_time+1;   //avoid reset more times at the same position
                        }
                        para->playctrl_info.reset_flag = 1;
                        para->playctrl_info.end_flag = 1;                          
                        log_print("pid[%d}::[%s:%d]time=%d ret=%d\n",para->player_id,__FUNCTION__, __LINE__,para->playctrl_info.time_point,ret);
                    }                                        
                }
            }
        }
    }
    return PLAYER_SUCCESS;
}

codec_para_t *get_audio_codec(play_para_t *player)
{
    if(player->stream_type == STREAM_ES ||
       player->stream_type == STREAM_AUDIO)
    {        
        return player->acodec;
    }
    else
    {
        return player->codec;
    }        
}

codec_para_t *get_video_codec(play_para_t *player)
{
    if(player->stream_type == STREAM_ES ||
       player->stream_type == STREAM_VIDEO)
    {
        return player->vcodec;
    }
    else
    {
        return player->codec;
    }        
}

static void check_msg(play_para_t *para,player_cmd_t *msg)
{
    if((msg->ctrl_cmd & CMD_EXIT) || (msg->ctrl_cmd & CMD_STOP))
    {
    	para->playctrl_info.end_flag = 1;
        para->playctrl_info.loop_flag = 0;
        para->playctrl_info.search_flag = 0;    	
    	para->playctrl_info.pause_flag = 0;   
        para->playctrl_info.fast_forward = 0;
        para->playctrl_info.fast_backward = 0;
    }
    else if(msg->ctrl_cmd & CMD_SEARCH)
    {
        if(msg->param < para->state.full_time && msg->param >= 0)
        {
            para->playctrl_info.search_flag = 1;
            para->playctrl_info.time_point = msg->param;      
            para->playctrl_info.end_flag = 1;
            para->playctrl_info.need_reset = 0; 
        }
        else
        {
            log_print("pid[%d]::seek time overspill!\n",para->player_id);
            set_player_error_no(para,PLAYER_SEEK_OVERSPILL);
        }
    }
    else if(msg->ctrl_cmd & CMD_PAUSE)
    {   
        para->playctrl_info.pause_flag = 1;       
    }
    else if(msg->ctrl_cmd & CMD_RESUME)
    {
       para->playctrl_info.pause_flag = 0;                      
    }
    else if(msg->set_mode & CMD_LOOP)
    {
       para->playctrl_info.loop_flag = 1;                      
    }
    else if(msg->set_mode & CMD_NOLOOP)
    {
       para->playctrl_info.loop_flag = 0;                      
    }
    else if(msg->ctrl_cmd & CMD_FF)
    {
		para->playctrl_info.init_ff_fr = 0;
        if (msg->param == 0)
        {
            para->playctrl_info.f_step = 0;            
        }
        else
        {
            para->playctrl_info.f_step = msg->param * FF_FB_BASE_STEP;  
            para->playctrl_info.fast_forward = 1;
            para->playctrl_info.fast_backward = 0;
        }
    }
    else if(msg->ctrl_cmd & CMD_FB)
    {
		para->playctrl_info.init_ff_fr = 0;
        if (msg->param == 0)
        {
            para->playctrl_info.f_step = 0;
        }
        else
        {
            para->playctrl_info.f_step = msg->param * FF_FB_BASE_STEP;  
            para->playctrl_info.fast_backward = 1;
            para->playctrl_info.fast_forward = 0;
        }
    }
    else if(msg->ctrl_cmd & CMD_SWITCH_AID)
    {
        para->playctrl_info.audio_switch_flag = 1;
        para->playctrl_info.switch_audio_id = msg->param;
    }
    else if(msg->ctrl_cmd & CMD_SWITCH_SID)
    {
        para->playctrl_info.switch_sub_id = msg->param;
        player_switch_sub(para);
    }
}

int check_flag(play_para_t *p_para)
{
    player_cmd_t *msg = NULL;	
  
    msg = get_message(p_para);	//msg: pause, resume, timesearch,add file, rm file, move up, move down,... 
	if(msg)
	{
        log_print("pid[%d]:: [check_flag:%d]cmd=%x set_mode=%x info=%x param=%d\n",p_para->player_id,__LINE__,msg->ctrl_cmd,msg->set_mode, msg->info_cmd, msg->param);
        check_msg(p_para, msg);
        message_free(msg);
		msg=NULL;                
	}     
    if(p_para->playctrl_info.end_flag)                              
    {
        if(!p_para->playctrl_info.search_flag &&
           !p_para->playctrl_info.fast_forward &&
           !p_para->playctrl_info.fast_backward)
        {          
            set_black_policy(p_para->playctrl_info.black_out);
            set_player_state(p_para,PLAYER_STOPED); 
        }
        return BREAK_FLAG;
    }
           
    if((p_para->playctrl_info.fast_forward || 
        p_para->playctrl_info.fast_backward)
        && (!p_para->playctrl_info.init_ff_fr))
    {
        if (p_para->playctrl_info.f_step != 0)
        {            
            p_para->astream_info.has_audio = 0;
            p_para->playctrl_info.init_ff_fr = 1;
            p_para->playctrl_info.time_point = p_para->state.current_time;
            set_black_policy(0);
            set_cntl_mode(p_para, TRICKMODE_FFFB);
        }
        else
        {            
            p_para->playctrl_info.fast_forward = 0;
            p_para->playctrl_info.fast_backward = 0;
            p_para->playctrl_info.search_flag = 1;
            p_para->astream_info.has_audio = p_para->astream_info.resume_audio;
            set_cntl_mode(p_para, TRICKMODE_NONE);
        }  
         log_print("pid[%d]::[%s:%d]ff=%d fb=%d step=%d curtime=%d timepoint=%d\n",p_para->player_id,__FUNCTION__, __LINE__, 
            p_para->playctrl_info.fast_forward,p_para->playctrl_info.fast_backward,
            p_para->playctrl_info.f_step,p_para->state.current_time,p_para->playctrl_info.time_point);
        return BREAK_FLAG;
    }
    if(p_para->playctrl_info.pause_flag)
    {        
        if(get_player_state(p_para)== PLAYER_RUNNING)
        {
            if(p_para->astream_info.has_audio)
                codec_pause(NULL);        
			/* only commit video pause here */
            if (p_para->astream_info.has_audio == 0)
            {
                if (p_para->vcodec)
                    codec_pause(p_para->vcodec);
                else
                    codec_pause(p_para->codec);
            }           
            set_player_state(p_para,PLAYER_PAUSE);

            update_playing_info(p_para); 
            update_player_states(p_para,1);       
        }  
        return CONTINUE_FLAG;
    }
    else
    {        
        if(get_player_state(p_para) == PLAYER_PAUSE)
         {   
            if(p_para->astream_info.has_audio)
				codec_resume(NULL);
			/* only commit video resume here */
            if (p_para->astream_info.has_audio == 0)
            {
                if (p_para->vcodec)
                    codec_resume(p_para->vcodec);
                else
                    codec_resume(p_para->codec);
            }
            set_player_state(p_para,PLAYER_RUNNING);
            update_playing_info(p_para); 
            update_player_states(p_para,1);  
        }
    }

    if(p_para->playctrl_info.audio_switch_flag)
    {
        player_switch_audio(p_para);
        p_para->playctrl_info.audio_switch_flag = 0;
    }    
    return NONO_FLAG;
}

void set_player_error_no(play_para_t *player,int error_no)
{
    player->state.error_no = error_no;
}

void update_player_start_paras(play_para_t *p_para, play_control_t *c_para)
{
	p_para->file_name = c_para->file_name;	
	p_para->vstream_info.video_index = c_para->video_index;
	p_para->astream_info.audio_index = c_para->audio_index;
    p_para->sstream_info.sub_index = c_para->sub_index;
	p_para->playctrl_info.no_audio_flag = c_para->nosound;
	p_para->playctrl_info.no_video_flag = c_para->novideo;
    p_para->playctrl_info.has_sub_flag = c_para->hassub;
	p_para->playctrl_info.loop_flag = c_para->loop_mode;
    //if(!p_para->playctrl_info.no_audio_flag)
    //    p_para->playctrl_info.audio_mute= codec_get_mutesta(NULL);
    p_para->update_state = c_para->callback_fn;
    p_para->playctrl_info.black_out = get_black_policy();    
    log_print("pid[%d]::Init State: mute_on=%d black=%d\n",p_para->player_id,p_para->playctrl_info.audio_mute,p_para->playctrl_info.black_out);
}
static int check_start_cmd(play_para_t *player)
{
    int flag = -1;
    player_cmd_t *msg = NULL;
    msg = get_message(player);	//msg: pause, resume, timesearch,add file, rm file, move up, move down,... 
	if(msg )
	{
        log_print("pid[%d]::[check_flag:%d]ctrl=%x mode=%x info=%x param=%d\n",player->player_id,__LINE__,msg->ctrl_cmd,msg->set_mode, msg->info_cmd,msg->param);
        if(msg->ctrl_cmd & CMD_START)                
           flag = 1;   
		check_msg(player, msg);
        message_free(msg);
		msg=NULL;                
	}   
	log_print("===========search=%d pos=%d\n",player->playctrl_info.search_flag,player->playctrl_info.time_point);
    return flag;
}
static int check_stop_cmd(play_para_t *player)
{    
    player_cmd_t *msg = NULL;   
	int ret = -1;
    msg = get_message(player);
    if(msg)
    {
        log_print("pid[%d]::[player_thread:%d]cmd=%x set_mode=%x info=%x param=%d\n",player->player_id,__LINE__,msg->ctrl_cmd,msg->set_mode, msg->info_cmd, msg->param);
        if(msg->ctrl_cmd & CMD_STOP)        
            ret = 1;    
        message_free(msg);
		msg=NULL;          	
    }   
	return ret;
}

int decodeble_wmv3(play_para_t *player)
{
	return (player->vstream_info.extradata[3] & 1);
}

///////////////////*main function *//////////////////////////////////////
void player_thread(play_para_t *player)
 {  
	am_packet_t am_pkt;
	AVPacket avpkt; 		   
	am_packet_t *pkt = NULL;
	int ret, pkt_count = 0;
    unsigned int exit_flag = 0;
	pkt = &am_pkt;
        
//#define SAVE_YUV_FILE

#ifdef SAVE_YUV_FILE
	int out_fp = -1;
#endif

    AVCodecContext *ic = NULL;
    AVCodec *codec = NULL;	
    AVFrame *picture = NULL;
    int got_picture = 0;        

    update_player_start_paras(player, player->start_param);      
    av_packet_init(pkt);  
    pkt->avpkt = &avpkt;    
	av_init_packet(pkt->avpkt);   
	
    player_thread_wait(player, 100*1000);      //wait pid send finish	
    set_player_state(player,PLAYER_INITING);   
    update_playing_info(player); 
    update_player_states(player,1);
    ret = player_dec_init(player);  
	if(ret != PLAYER_SUCCESS)
    {
        if(check_stop_cmd(player) == 1)
			set_player_state(player,PLAYER_STOPED);
		else                  
            set_player_state(player,PLAYER_ERROR); 
        goto release0;
	}    
	if ( player->vstream_info.video_format == VFORMAT_VC1 && player->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_WMV3 )
		log_print("*** wmv3 type = %d, video duration = %d ***\n", decodeble_wmv3(player), (int)(player->pFormatCtx->video_avg_frame_time*96/10000));
    ret = set_media_info(player);
    if(ret != PLAYER_SUCCESS)
    {
        log_error("pid[%d::player_set_media_info failed!\n",player->player_id);
        set_player_state(player,PLAYER_ERROR);        	
        goto release0;
	}
    set_player_state(player,PLAYER_INITOK);   
    update_playing_info(player); 
    update_player_states(player,1);
    
    if(player->start_param->need_start)
    {        
        int flag = 0;
        do
        { 
            flag = check_start_cmd(player);
            if(flag == 1)
                break;            
            else if(player->playctrl_info.end_flag == 1 && (!player->playctrl_info.search_flag))
            {
                set_player_state(player,PLAYER_STOPED);   
                update_playing_info(player); 
                update_player_states(player,1);
                goto release0;
            }
            player_thread_wait(player,100*1000);                    
        }while(1);
    }
    ret = player_decoder_init(player);
    if(ret != PLAYER_SUCCESS)
    {
        log_error("pid[%d]::player_decoder_init failed!\n",player->player_id);
        set_player_state(player,PLAYER_ERROR);        	
        goto release;
	}
	  
    set_cntl_mode(player, TRICKMODE_NONE);
	set_cntl_avthresh(player, AV_SYNC_THRESH);
    set_cntl_syncthresh(player); 
    
	set_player_state(player,PLAYER_START);   
    update_playing_info(player); 
    update_player_states(player,1);

    if ( player->vstream_info.video_format == VFORMAT_SW )
    {
    	log_print("Use SW video decoder\n");

#ifdef SAVE_YUV_FILE
        out_fp = open("./output.yuv", O_CREAT|O_RDWR);
        if(out_fp < 0)
            log_print("Create output file failed! fd=%d\n", out_fp);
#endif

		av_register_all();

	    ic = avcodec_alloc_context();
	    if (!ic) 
	    {
	        log_print("AVCodec Memory error\n");
	        ic = NULL;
        	goto release;
	    }

	    ic->codec_id = player->pFormatCtx->streams[player->vstream_info.video_index]->codec->codec_id;
	    ic->codec_type = CODEC_TYPE_VIDEO;
	    ic->pix_fmt = PIX_FMT_YUV420P;

	    codec = avcodec_find_decoder(ic->codec_id);
	    if (!codec) 
	    {
	        log_print("Codec not found\n");
        	goto release;
	    }

	    if (avcodec_open(ic, codec) < 0) 
	    {
	        log_print("Could not open codec\n");
        	goto release;
	    }

		picture = avcodec_alloc_frame();
	    if (!picture) 
	    {
	        log_print("Could not allocate picture\n");
        	goto release;
	    }
    }
    
    //player loop
	do
	{   
		if ( !(player->vstream_info.video_format == VFORMAT_SW)
				&& !(player->vstream_info.video_format == VFORMAT_VC1 && player->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_WMV3) )
		{
			pre_header_feeding(player, pkt);	
		}
		do
		{    
            ret = check_flag(player);     
            if (ret == BREAK_FLAG)
            {
                log_print("pid[%d]::[player_thread:%d]end=%d valid=%d new=%d pktsize=%d\n",player->player_id,
					__LINE__,player->playctrl_info.end_flag,pkt->avpkt_isvalid, pkt->avpkt_newflag,pkt->data_size);
				
                if(!player->playctrl_info.end_flag)
                {
                    if(pkt->avpkt_isvalid)
                    {                         
                        //player->playctrl_info.read_end_flag = 1;                        
                        goto write_packet;
                    }
                    else
                    {                   
                        player->playctrl_info.end_flag = 1;                        
                    }
                }
                break;
            }
            else if (ret == CONTINUE_FLAG)
            {
                player_thread_wait(player,100*1000);  //100ms
                continue;
            } 
            ret = read_av_packet(player, pkt);      
			if(ret!= PLAYER_SUCCESS && ret != PLAYER_RD_AGAIN)							
			{
				log_error("pid[%d]::read_av_packet failed!\n",player->player_id);
                set_player_state(player,PLAYER_ERROR);
			    goto release;
			}
			
            if(player->playctrl_info.f_step == 0)
            {                     
                if(ret == PLAYER_SUCCESS)
                {
                    if(get_player_state(player) != PLAYER_RUNNING)
                    {
                        set_player_state(player,PLAYER_RUNNING);
                        update_playing_info(player); 
                        update_player_states(player,1);
                    }
                }
                else if(ret == PLAYER_RD_AGAIN)
                {      
                    if(get_player_state(player) != PLAYER_BUFFERING)
                    {
                        set_player_state(player,PLAYER_BUFFERING);
                        update_playing_info(player); 
                        update_player_states(player,1);
                    }
                }
            } 
            if ( pkt->avpkt_newflag )
            {
            	if ( player->vstream_info.video_format == VFORMAT_VC1 && player->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_WMV3 )
            	{
	            	if ( !pkt->avpkt->flags && (pkt_count % 15 == 0) )
	            		pkt->avpkt->flags |= 2;
            	}

            	//log_print("pkt_count = %d, pkt->avpkt->flags = %d\n", pkt_count, pkt->avpkt->flags);
            	pkt_count++;
            }
            ret = set_header_info(player, pkt);
			if(ret!= PLAYER_SUCCESS)
			{
                log_error("pid[%d]::set_header_info failed! ret=%x\n",player->player_id,-ret);
                set_player_state(player,PLAYER_ERROR);
                goto release;
			}
write_packet:
			if ( (player->vstream_info.video_format == VFORMAT_SW) && (pkt->type == CODEC_VIDEO) )
			{
        		avcodec_decode_video2(ic, picture, &got_picture, pkt->avpkt);
				pkt->data_size = 0;
        		
        		if ( got_picture )
        		{        			
#ifdef SAVE_YUV_FILE
					if ( out_fp >= 0 )
					{
	        			int i;
	        			
		        		for ( i = 0 ; i < ic->height ; i++ )
		        			write(out_fp, picture->data[0]+i*picture->linesize[0], ic->width);
		        		for ( i = 0 ; i < ic->height/2 ; i++ )
		        			write(out_fp, picture->data[1]+i*picture->linesize[1], ic->width/2);
		        		for ( i = 0 ; i < ic->height/2 ; i++ )
		        			write(out_fp, picture->data[2]+i*picture->linesize[2], ic->width/2);
					}
#endif
        		}	
			}
			else
			{
	            ret = write_av_packet(player, pkt);           
	            if(ret == PLAYER_WR_FINISH)
	            {   
	               if(player->playctrl_info.f_step == 0)
	                    break;
	            }
				else if( ret != PLAYER_SUCCESS)                                     
				{
					log_print("pid[%d]::write_av_packet failed!\n",player->player_id);
	                set_player_state(player,PLAYER_ERROR);
				    goto release;
				}           
			}
            update_playing_info(player); 
            update_player_states(player,0);    
            if(check_decoder_worksta(player)!=PLAYER_SUCCESS)
            {
                log_error("pid[%d]::check decoder work status error!\n",player->player_id);
                set_player_state(player,PLAYER_ERROR);
			    goto release;
            }              
            if ( (player->vstream_info.video_format == VFORMAT_SW) && (pkt->type == CODEC_VIDEO) )
            {
			    player->state.current_time = (int)(pkt->avpkt->dts/1000);
			    if(player->state.current_time > player->state.full_time)   
			        player->state.current_time = player->state.full_time;  
			    if(player->state.current_time < player->state.last_time)   
			        player->state.current_time = player->state.last_time;  
		        player->state.last_time = player->state.current_time;  
            }
                 
            if( player->vstream_info.has_video
		        && (player->playctrl_info.fast_forward 
                || player->playctrl_info.fast_backward))
            {
            	if ( player->vstream_info.video_format != VFORMAT_SW )
            	{
	                ret = get_cntl_state(pkt);
	                if (ret == 0)
	                {
	                    //log_print("more data needed, data size %d\n", pkt->data_size);
	                    continue;
	                }
	                else if (ret < 0)
	                {
	                    log_error("pid[%d]::get state exception\n",player->player_id);
	                    continue;
	                }
	                else
	                {
	                    player->playctrl_info.end_flag = 1;
	                    break;
	                }
	            } 
            } 
		} while(!player->playctrl_info.end_flag);
      
        //wait for play end...
        while(!player->playctrl_info.end_flag)
		{   
			player_thread_wait(player,100*1000);
            
            ret = check_flag(player);
            if (ret == BREAK_FLAG)
                break;
            else if (ret == CONTINUE_FLAG)
                continue;

            if(update_playing_info(player) != PLAYER_SUCCESS)				
				break;
            
			update_player_states(player,0);
		}	
        
        log_print("pid[%d]::loop=%d search=%d ff=%d fb=%d reset=%d step=%d\n",
				player->player_id,
                player->playctrl_info.loop_flag,player->playctrl_info.search_flag,
                player->playctrl_info.fast_forward,player->playctrl_info.fast_backward,
                player->playctrl_info.reset_flag,player->playctrl_info.f_step);
        
        exit_flag = (!player->playctrl_info.loop_flag)   &&
                    (!player->playctrl_info.search_flag) &&
                    (!player->playctrl_info.fast_forward)&&
                    (!player->playctrl_info.fast_backward)&&
                    (!player->playctrl_info.reset_flag);
        if(exit_flag)           
        {
            break;
        }
        else           
        {       
            if(get_player_state(player) != PLAYER_SEARCHING)
            {
                set_player_state(player,PLAYER_SEARCHING);
                update_playing_info(player); 
                update_player_states(player,1);
            }
            ret= player_reset(player,pkt);              
            if(ret != PLAYER_SUCCESS)
            {
                log_error("pid[%d]::player reaset failed(%d)!",player->player_id,ret);
                break;
            }              
            if(player->playctrl_info.end_flag)
            {  
                set_player_state(player,PLAYER_PLAYEND);
                break;            
            }
            if(player->playctrl_info.search_flag)
            {
                set_player_state(player,PLAYER_SEARCHOK);   
                update_playing_info(player); 
                update_player_states(player,1);
                
                if(player->playctrl_info.f_step == 0)
                    set_black_policy(player->playctrl_info.black_out);
            }
           
            player->playctrl_info.search_flag = 0;
            player->playctrl_info.reset_flag = 0;
            player->playctrl_info.end_flag = 0;            
            av_packet_release(pkt);   
        }
	}while(1);    
release:    
    if ( player->vstream_info.video_format == VFORMAT_SW )
    {
#ifdef SAVE_YUV_FILE
        printf("Output file closing\n");
		if ( out_fp >= 0 )
			close(out_fp);
       	printf("Output file closed\n");
#endif
		if ( picture )
			av_free(picture);
		if ( ic )
		{
			log_print("AVCodec close\n");
    		avcodec_close(ic);
    		av_free(ic);
    	}
    }
    else
    	set_cntl_mode(player, TRICKMODE_NONE);
    
release0:
    if(get_player_state(player) == PLAYER_ERROR)
        set_player_error_no(player,ret);   
	update_playing_info(player);  
	av_packet_release(&am_pkt);	
    update_player_states(player,1);     
    log_print("\npid[%d]::stop play, exit player thead!(sta:%d)\n",player->player_id,player->state.status);
    player_para_release(player);
	pthread_exit(0);    
 }

