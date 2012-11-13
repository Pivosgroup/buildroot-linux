/**
 * \file adec-internal-mgt.c
 * \brief  Audio Dec Message Loop Thread
 * \version 1.0.0
 * \date 2011-03-08
 */
/* Copyright (C) 2007-2011, Amlogic Inc.
 * All right reserved
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <audio-dec.h>
#include <adec-pts-mgt.h>

#ifdef ANDROID
#include <cutils/properties.h>
#endif

/**
 * \brief set audio output mode.
 * \param cmd control commands
 * \return 0 on success otherwise -1
 */
 #if 0
#define ACODEC_FMT_NULL 	0
#define ACODEC_FMT_AAC		1
#define ACODEC_FMT_AC3		2
#define ACODEC_FMT_DTS		3
#define ACODEC_FMT_FLAC		4
#define ACODEC_FMT_COOK		5
#define ACODEC_FMT_AMR		6
#define ACODEC_FMT_RAAC		7
#define ACODEC_FMT_ADPCM	8
#define ACODEC_FMT_WMA		9
#define ACODEC_FMT_PCM		10
#define ACODEC_FMT_WMAPRO	11
#define ACODEC_FMT_ALAC		12
#define ACODEC_FMT_VORBIS	13
#define ACODEC_FMT_AAC_LATM	14
#define ACODEC_FMT_APE		15
#define ACODEC_FMT_MPEG123 	16
#endif

typedef struct {
//	int no;
	int audio_id;
	char type[16];
}audio_type_t;

audio_type_t audio_type[] = {
    {ACODEC_FMT_AAC, "aac"},
    {ACODEC_FMT_AC3, "ac3"},
    {ACODEC_FMT_DTS, "dts"},
    {ACODEC_FMT_FLAC, "flac"},
    {ACODEC_FMT_COOK, "cook"},
    {ACODEC_FMT_AMR, "arm"},
    {ACODEC_FMT_RAAC, "raac"},
    {ACODEC_FMT_ADPCM, "adpcm"},
    {ACODEC_FMT_WMA, "wma"},
    {ACODEC_FMT_WMAPRO, "wmapro"},
    {ACODEC_FMT_ALAC, "alac"},
    {ACODEC_FMT_VORBIS, "vorbis"},
    {ACODEC_FMT_AAC_LATM, "aac_latm"},
    {ACODEC_FMT_APE, "ape"},
    {ACODEC_FMT_MPEG, "mp3"},
};

static int audio_decoder = AUDIO_ARC_DECODER;
static int audio_hardware_ctrl(hw_command_t cmd)
{
    int fd;

    fd = open(AUDIO_CTRL_DEVICE, O_RDONLY);
    if (fd < 0) {
        adec_print("Open Device %s Failed!", AUDIO_CTRL_DEVICE);
        return -1;
    }

    switch (cmd) {
    case HW_CHANNELS_SWAP:
        ioctl(fd, AMAUDIO_IOC_SET_CHANNEL_SWAP, 0);
        break;

    case HW_LEFT_CHANNEL_MONO:
        ioctl(fd, AMAUDIO_IOC_SET_LEFT_MONO, 0);
        break;

    case HW_RIGHT_CHANNEL_MONO:
        ioctl(fd, AMAUDIO_IOC_SET_RIGHT_MONO, 0);
        break;

    case HW_STEREO_MODE:
        ioctl(fd, AMAUDIO_IOC_SET_STEREO, 0);
        break;

    default:
        adec_print("Unknow Command %d!", cmd);
        break;

    };

    close(fd);

    return 0;

}

/**
 * \brief start audio dec when receive START command.
 * \param audec pointer to audec
 */
static void start_adec(aml_audio_dec_t *audec)
{
    int ret;
    audio_out_operations_t *aout_ops = &audec->aout_ops;
    dsp_operations_t *dsp_ops = &audec->adsp_ops;

    if (audec->state == INITTED) {
        audec->state = ACTIVE;

        while ((!audiodsp_get_first_pts_flag(dsp_ops)) && (!audec->need_stop)) {
            adec_print("wait first pts checkin complete !");
            usleep(100000);
        }

        /*start  the  the pts scr,...*/
        ret = adec_pts_start(audec);

        //adec_pts_droppcm(audec);
		
        if (audec->auto_mute) {
            avsync_en(0);
            audiodsp_automute_on(dsp_ops);
            adec_pts_pause();

            while ((!audec->need_stop) && track_switch_pts(audec)) {
                usleep(1000);
            }

            audiodsp_automute_off(dsp_ops);
            avsync_en(1);
            adec_pts_resume();

            audec->auto_mute = 0;
        }

        aout_ops->start(audec);

    }
}

/**
 * \brief pause audio dec when receive PAUSE command.
 * \param audec pointer to audec
 */
static void pause_adec(aml_audio_dec_t *audec)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (audec->state == ACTIVE) {
        audec->state = PAUSED;
        adec_pts_pause();
        aout_ops->pause(audec);
    }
}

/**
 * \brief resume audio dec when receive RESUME command.
 * \param audec pointer to audec
 */
static void resume_adec(aml_audio_dec_t *audec)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (audec->state == PAUSED) {
        audec->state = ACTIVE;
        aout_ops->resume(audec);
        adec_pts_resume();
    }
}

/**
 * \brief stop audio dec when receive STOP command.
 * \param audec pointer to audec
 */
static void stop_adec(aml_audio_dec_t *audec)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (audec->state > INITING) {
        audec->state = STOPPED;
	aout_ops->mute(audec, 1); //mute output, some repeat sound in audioflinger after stop
        aout_ops->stop(audec);
        feeder_release(audec);
    }
}

/**
 * \brief release audio dec when receive RELEASE command.
 * \param audec pointer to audec
 */
static void release_adec(aml_audio_dec_t *audec)
{
    audec->state = TERMINATED;
}

/**
 * \brief mute audio dec when receive MUTE command.
 * \param audec pointer to audec
 * \param en 1 = mute, 0 = unmute
 */
static void mute_adec(aml_audio_dec_t *audec, int en)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (aout_ops->mute) {
        adec_print("%s the output !\n", (en ? "mute" : "unmute"));
        aout_ops->mute(audec, en);
        audec->muted = en;
    }
}

/**
 * \brief set volume to audio dec when receive SET_VOL command.
 * \param audec pointer to audec
 * \param vol volume value
 */
static void adec_set_volume(aml_audio_dec_t *audec, float vol)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (aout_ops->set_volume) {
        adec_print("set audio volume! vol = %f\n", vol);
        aout_ops->set_volume(audec, vol);
    }
}

/**
 * \brief set volume to audio dec when receive SET_LRVOL command.
 * \param audec pointer to audec
 * \param lvol left channel volume value
 * \param rvol right channel volume value
 */
static void adec_set_lrvolume(aml_audio_dec_t *audec, float lvol,float rvol)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (aout_ops->set_lrvolume) {
        adec_print("set audio volume! left vol = %f,right vol:%f\n", lvol,rvol);
        aout_ops->set_lrvolume(audec, lvol,rvol);
    }
}
static void adec_flag_check(aml_audio_dec_t *audec)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (audec->auto_mute && (audec->state > INITTED)) {
        aout_ops->pause(audec);
        while ((!audec->need_stop) && track_switch_pts(audec)) {
            usleep(1000);
        }
        aout_ops->resume(audec);
        audec->auto_mute = 0;
    }
}

/**
 * \brief adec main thread
 * \param args pointer to thread private data
 * \return NULL
 */
 #if 0 
static int decode_audio(AVCodecContext *ctxCodec, char *outbuf, int *outlen, char *inbuf, int inlen){
	AVPacket avpkt;
	int ret;

	av_init_packet(&avpkt);
	avpkt.data = inbuf;
	avpkt.size = inlen;
	int i;

	switch (ctxCodec->codec_id) {
	case CODEC_ID_AAC:
		ret = avcodec_decode_audio3(ctxCodec, (int16_t *)outbuf, outlen, &avpkt);
		//adec_print("ape samplerate=%d,channels=%d,framesize=%d,ret=%d,outlen=%d,inlen=%d,-------------------------------------------\n",
	    //		ctxCodec->sample_rate,ctxCodec->channels,ctxCodec->frame_size,ret,*outlen,inlen);
		break;
	default: 
		break;
	}

	return ret;
}
#endif
static int write_buffer(char *outbuf, int outlen){
	return 0;
}


static void *adec_message_loop(void *args)
{
    int ret;
    aml_audio_dec_t *audec;
    audio_out_operations_t *aout_ops;
    adec_cmd_t *msg = NULL;

    audec = (aml_audio_dec_t *)args;
    aout_ops = &audec->aout_ops;

    while (!audec->need_stop) {
        audec->state = INITING;
        ret = feeder_init(audec);
        if (ret == 0) {
            ret = aout_ops->init(audec);
            if (ret) {
                adec_print("Audio out device init failed!");
                feeder_release(audec);
                continue;
            }
            audec->state = INITTED;
            adec_print("Audio out device init ok!");
            start_adec(audec);
            break;
        }

	if(!audec->need_stop){
            usleep(100000);
	    }
       }
            
    do {
        //if(message_pool_empty(audec))
        //{
        //adec_print("there is no message !\n");
        //  usleep(100000);
        //  continue;
        //}
        adec_flag_check(audec);

        msg = adec_get_message(audec);
        if (!msg) {
            usleep(100000);
            continue;
        }

        switch (msg->ctrl_cmd) {
        case CMD_START:

            adec_print("Receive START Command!\n");
            start_adec(audec);
            break;

        case CMD_PAUSE:

            adec_print("Receive PAUSE Command!");
            pause_adec(audec);
            break;

        case CMD_RESUME:

            adec_print("Receive RESUME Command!");
            resume_adec(audec);
            break;

        case CMD_STOP:

            adec_print("Receive STOP Command!");
            stop_adec(audec);
            break;

        case CMD_MUTE:

            adec_print("Receive Mute Command!");
            if (msg->has_arg) {
                mute_adec(audec, msg->value.en);
            }
            break;

        case CMD_SET_VOL:

            adec_print("Receive Set Vol Command!");
            if (msg->has_arg) {
                adec_set_volume(audec, msg->value.volume);
            }
            break;
	 case CMD_SET_LRVOL:

            adec_print("Receive Set LRVol Command!");
            if (msg->has_arg) {
                adec_set_lrvolume(audec, msg->value.volume,msg->value_ext.volume);
            }
            break;	 	
		
        case CMD_CHANL_SWAP:

            adec_print("Receive Channels Swap Command!");
            audio_hardware_ctrl(HW_CHANNELS_SWAP);
            break;

        case CMD_LEFT_MONO:

            adec_print("Receive Left Mono Command!");
            audio_hardware_ctrl(HW_LEFT_CHANNEL_MONO);
            break;

        case CMD_RIGHT_MONO:

            adec_print("Receive Right Mono Command!");
            audio_hardware_ctrl(HW_RIGHT_CHANNEL_MONO);
            break;

        case CMD_STEREO:

            adec_print("Receive Stereo Command!");
            audio_hardware_ctrl(HW_STEREO_MODE);
            break;

        case CMD_RELEASE:

            adec_print("Receive RELEASE Command!");
            release_adec(audec);
            break;

        default:
            adec_print("Unknow Command!");
            break;

        }

        if (msg) {
            adec_message_free(msg);
            msg = NULL;
        }
    } while (audec->state != TERMINATED);

    adec_print("Exit Message Loop Thread!");
    pthread_exit(NULL);
    return NULL;
}

#define READ_ABUFFER_SIZE 10*1024
#define APACKET_END_SIZE 15*1024
#if 0
static void *adec_armdec_loop(void *args)
{
    int ret;
	int rlen = 0;
	int inlen = 0;
	int dlen = 0;
	int declen = 0;
    aml_audio_dec_t *audec;
    audio_out_operations_t *aout_ops;
    adec_cmd_t *msg = NULL;
	AVPacket apkt;
	char *inbuf = NULL;
	char apkt_end[APACKET_END_SIZE];
	char outbuf[AVCODEC_MAX_AUDIO_FRAME_SIZE];
	int outlen = 0;
	AVCodecContext *ctxCodec = NULL;
	AVCodec *acodec = NULL;
	int in_ape_fp = -1;
	int out_ape_fp = -1;
	int audio_handle = -1;
	
	adec_print("adec_armdec_loop start!\n");

	audec = (aml_audio_dec_t *)args;
    aout_ops = &audec->aout_ops;
	av_init_packet(&apkt); 
	//memset(inbuf, 0, READ_ABUFFER_SIZE);
	memset(outbuf, 0, AVCODEC_MAX_AUDIO_FRAME_SIZE);

	//buffer_stream_t init and set adsp_ops param
	audec->bs=malloc(sizeof(buffer_stream_t));
	int ret_value=init_buff(audec->bs);
	if(ret_value==1)
		adec_print("=====pcm buffer init ok buf_size:%d buf_data:0x%x  end:0x%x !\n",audec->bs->buf_length,audec->bs->data,audec->bs->data+1024*1024);
	audec->adsp_ops.dsp_on=1;
	aout_ops->init(audec);
	aout_ops->start(audec);

	ctxCodec = avcodec_alloc_context();
	if(!ctxCodec) {
		adec_print("APE AVCodecContext allocate error!\n");
		ctxCodec = NULL;
	}
	adec_print("ctxCodec!\n");

	
	adec_print("adec_armdec_loop   audec->pcodec = %d, audec->pcodec->ctxCodec = %d!\n", audec->pcodec, audec->pcodec->ctxCodec);
		
	ctxCodec = audec->pcodec->ctxCodec;
	ctxCodec->codec_type = CODEC_TYPE_AUDIO;
		
	adec_print("open codec_id = %d--\n",ctxCodec->codec_id);
	acodec = avcodec_find_decoder(ctxCodec->codec_id);
	if (!acodec) {
		adec_print("acodec not found\n");
	}
	adec_print("open codec_id = %d----------------------------------\n",ctxCodec->codec_id);
		
	if (avcodec_open(ctxCodec, acodec) < 0) {
		adec_print("Could not open acodec = %d\n", acodec);
	}

	out_ape_fp = open("./dump/123.dat", O_CREAT | O_RDWR);
	if (out_ape_fp < 0) {
        adec_print("Create input file failed! fd=%d------------------------------\n", out_ape_fp);
    }
    adec_print("out_ape_fp = %d!", out_ape_fp);
	in_ape_fp = open("./dump/in.dat", O_CREAT | O_RDWR);
	if (in_ape_fp < 0) {
        adec_print("Create input file failed! fd=%d------------------------------\n", out_ape_fp);
    }
    adec_print("in_ape_fp = %d!", in_ape_fp);

	ret = uio_init();
	if (ret < 0){
		adec_print("uio init error! \n");
		goto error;
	}
		
    while (1){

		if (inlen > 0) {
			if (inbuf) {
				free(inbuf);
				inbuf = NULL;
			}
			inbuf = malloc(READ_ABUFFER_SIZE + inlen);
			memcpy(inbuf, apkt_end, inlen);
			rlen = read_buffer(inbuf+inlen, READ_ABUFFER_SIZE);
			rlen += inlen;
		}
		else {
			if (inbuf) {
				free(inbuf);
				inbuf = NULL;
			}
			inbuf = malloc(READ_ABUFFER_SIZE);
			rlen = read_buffer(inbuf+inlen, READ_ABUFFER_SIZE);
		}
		if (out_ape_fp >= 0) {
			write(in_ape_fp, inbuf, rlen);
			adec_print("write ape data in rlen = %d bytes\n",rlen);
		}
		declen = 0;
		if (rlen > 0){
			inlen = rlen;
			while (declen<rlen) {	
				outlen = AVCODEC_MAX_AUDIO_FRAME_SIZE;
				dlen = decode_audio(ctxCodec, outbuf, &outlen, inbuf+declen, inlen);
				if (dlen <= 0){
					adec_print("dlen = %d error----\n",dlen);
					if (inlen > 0) {
						adec_print("packet end %d bytes----\n",inlen);
						memcpy(apkt_end, (uint8_t *)(inbuf+declen), inlen);
					}
					break;
				}
				declen += dlen;
				inlen -= dlen;
				write_pcm_buffer(outbuf, audec->bs,outlen);
				//write_buffer(outbuf, outlen);
				if (outlen > 0) {
					if (out_ape_fp >= 0) {
						write(out_ape_fp, outbuf, outlen);
					}
				}
			} 
		
#if 0			
			outsize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
			apkt.data = inbuffer;
			apkt.size = rlen;
			while (apkt.size > 0){
				dlen = avcodec_decode_audio3(ctxCodec, (int16_t *)outbuffer, &outsize, &apkt);
				if (dlen <= 0){
					adec_print("dlen = %d error----\n",dlen);
				}
				if (outsize > 0) {
					if (out_ape_fp >= 0) {
						write(out_ape_fp, outbuffer, outsize);
						adec_print("write ape data%d bytes\n UUUUUUUU----\n",outsize);
					}
				}				
				declen += dlen;
				apkt.size -= dlen;					
				if (apkt.size > 0){
					apkt.data += dlen;
				}
				else if (apkt.size < 0){
					adec_print("wrong aptk.size = %d, declen = %d, dlen = %d!", apkt.size, declen, dlen);
				}					
			}
#endif			
		}
		else {			
			adec_print("rlen = %d", rlen);
			break;
		}
	}
    close(out_ape_fp);
    close(in_ape_fp);
	
    adec_print("Exit adec_armdec_loop Thread!");

error:	
    pthread_exit(NULL);
    return NULL;
}
#endif
/**
 * \brief start audio dec
 * \param audec pointer to audec
 * \return 0 on success otherwise -1 if an error occurred
 */
 #if 0
static int get_dectype(int id)
{
	switch (id) {
		case  CODEC_ID_MP3:
			return ACODEC_FMT_MPEG;
	
		case  CODEC_ID_AAC_LATM:
			return ACODEC_FMT_AAC_LATM;
	
		case  CODEC_ID_AAC:
			return ACODEC_FMT_AAC;
	
		case  CODEC_ID_AC3:
			return ACODEC_FMT_AC3;
	
		case  CODEC_ID_DTS:
			return ACODEC_FMT_DTS;
	
		case  CODEC_ID_FLAC:
			return ACODEC_FMT_FLAC;
	
		case  CODEC_ID_COOK:
			return ACODEC_FMT_COOK;
	
		case  CODEC_ID_AMR_NB:
		case  CODEC_ID_AMR_WB:
			return ACODEC_FMT_AMR;
	
		case  CODEC_ID_RA_144:
		case  CODEC_ID_RA_288:
			return ACODEC_FMT_RAAC;
	
		case  CODEC_ID_ADPCM_IMA_QT:
		case  CODEC_ID_ADPCM_IMA_WAV:
		case  CODEC_ID_ADPCM_IMA_DK3:
		case  CODEC_ID_ADPCM_IMA_DK4:
		case  CODEC_ID_ADPCM_IMA_WS:
		case  CODEC_ID_ADPCM_IMA_SMJPEG:
		case  CODEC_ID_ADPCM_MS:
		case  CODEC_ID_ADPCM_4XM:
		case  CODEC_ID_ADPCM_XA:
		case  CODEC_ID_ADPCM_ADX:
		case  CODEC_ID_ADPCM_EA:
		case  CODEC_ID_ADPCM_G726:
		case  CODEC_ID_ADPCM_CT:
		case  CODEC_ID_ADPCM_SWF:
		case  CODEC_ID_ADPCM_YAMAHA:
		case  CODEC_ID_ADPCM_SBPRO_4:
		case  CODEC_ID_ADPCM_SBPRO_3:
		case  CODEC_ID_ADPCM_SBPRO_2:
		case  CODEC_ID_ADPCM_THP:
		case  CODEC_ID_ADPCM_IMA_AMV:
		case  CODEC_ID_ADPCM_EA_R1:
		case  CODEC_ID_ADPCM_EA_R3:
		case  CODEC_ID_ADPCM_EA_R2:
		case  CODEC_ID_ADPCM_IMA_EA_SEAD:
		case  CODEC_ID_ADPCM_IMA_EA_EACS:
		case  CODEC_ID_ADPCM_EA_XAS:
		case  CODEC_ID_ADPCM_EA_MAXIS_XA:
		case  CODEC_ID_ADPCM_IMA_ISS:
		case  CODEC_ID_ADPCM_G722:
			return ACODEC_FMT_ADPCM;

		case CODEC_ID_PCM_S16LE:
		case CODEC_ID_PCM_S16BE:
		case CODEC_ID_PCM_U16LE:
		case CODEC_ID_PCM_U16BE:
		case CODEC_ID_PCM_S8:
		case CODEC_ID_PCM_U8:
		case CODEC_ID_PCM_MULAW:
		case CODEC_ID_PCM_ALAW:
		case CODEC_ID_PCM_S32LE:
		case CODEC_ID_PCM_S32BE:
		case CODEC_ID_PCM_U32LE:
		case CODEC_ID_PCM_U32BE:
		case CODEC_ID_PCM_S24LE:
		case CODEC_ID_PCM_S24BE:
		case CODEC_ID_PCM_U24LE:
		case CODEC_ID_PCM_U24BE:
		case CODEC_ID_PCM_S24DAUD:
		case CODEC_ID_PCM_ZORK:
		case CODEC_ID_PCM_S16LE_PLANAR:
		case CODEC_ID_PCM_DVD:
		case CODEC_ID_PCM_F32BE:
		case CODEC_ID_PCM_F32LE:
		case CODEC_ID_PCM_F64BE:
		case CODEC_ID_PCM_F64LE:
		case CODEC_ID_PCM_BLURAY:
		case CODEC_ID_PCM_LXF:
		case CODEC_ID_S302M:
			return ACODEC_FMT_PCM_U8;
	
		case CODEC_ID_WMAV1:
    	case CODEC_ID_WMAV2:
			return ACODEC_FMT_WMA;
	
		case CODEC_ID_WMAPRO:
			return ACODEC_FMT_WMAPRO;

		case CODEC_ID_ALAC:
			return ACODEC_FMT_ALAC;
			
		case CODEC_ID_VORBIS:
			return ACODEC_FMT_VORBIS;
			
		case CODEC_ID_APE:
			return ACODEC_FMT_APE;
			
		default:
			return ACODEC_FMT_NULL;
	}
}
#endif
int match_types(const char *filetypestr,const char *typesetting)
{
	const char * psets=typesetting;
	const char *psetend;
	int psetlen=0;
	char typestr[64]="";
	if(filetypestr==NULL || typesetting==NULL)
		return 0;

	while(psets && psets[0]!='\0'){
		psetlen=0;
		psetend=strchr(psets,',');
		if(psetend!=NULL && psetend>psets && psetend-psets<64){
			psetlen=psetend-psets;
			memcpy(typestr,psets,psetlen);
			typestr[psetlen]='\0';
			psets=&psetend[1];//skip ";"
		}else{
			strcpy(typestr,psets);
			psets=NULL;
		}
		if(strlen(typestr)>0&&(strlen(typestr)==strlen(filetypestr))){
			if(strstr(filetypestr,typestr)!=NULL)
				return 1;
		}
	}
	return 0;
}
#if 0
static int set_audio_decoder(codec_para_t *pcodec)
{
	int audio_id;
	int i;	
    int num;
	int ret;
    audio_type_t *t;
	char value[PROPERTY_VALUE_MAX];
	

	audio_id = get_dectype(pcodec->ctxCodec->codec_id);

    num = ARRAY_SIZE(audio_type);
    for (i = 0; i < num; i++) {
        t = &audio_type[i];
        if (t->audio_id == audio_id) {
            break;
        }
    }
	
	ret = property_get("media.arm.audio.decoder",value,NULL);
	adec_print("media.amplayer.audiocodec = %s, t->type = %s\n", value, t->type);
	if (ret>0 && match_types(t->type,value))
	{	
		audio_decoder = AUDIO_ARM_DECODER;
		return 0;
	} 
	
	ret = property_get("media.arc.audio.decoder",value,NULL);
	adec_print("media.amplayer.audiocodec = %s, t->type = %s\n", value, t->type);
	if (ret>0 && match_types(t->type,value))
	{	
		audio_decoder = AUDIO_ARC_DECODER;
		return 0;
	} 
	
	ret = property_get("media.ffmpeg.audio.decoder",value,NULL);
	adec_print("media.amplayer.audiocodec = %s, t->type = %s\n", value, t->type);
	if (ret>0 && match_types(t->type,value))
	{	
		audio_decoder = AUDIO_FFMPEG_DECODER;
		return 0;
	} 
	
	audio_decoder = AUDIO_ARC_DECODER; //set arc decoder as default
	return 0;
}
#endif
static int set_audio_decoder(int codec_id)
{
	int audio_id;
#ifdef ANDROID
	int i;	
    int num;
	int ret;
    audio_type_t *t;
	char value[PROPERTY_VALUE_MAX];
	

	audio_id = codec_id;

    num = ARRAY_SIZE(audio_type);
    for (i = 0; i < num; i++) {
        t = &audio_type[i];
        if (t->audio_id == audio_id) {
            break;
        }
    }
	
	ret = property_get("media.arm.audio.decoder",value,NULL);
	adec_print("media.amplayer.audiocodec = %s, t->type = %s\n", value, t->type);
	if (ret>0 && match_types(t->type,value))
	{	
		audio_decoder = AUDIO_ARM_DECODER;
		return 0;
	} 
	
	ret = property_get("media.arc.audio.decoder",value,NULL);
	adec_print("media.amplayer.audiocodec = %s, t->type = %s\n", value, t->type);
	if (ret>0 && match_types(t->type,value))
	{	
		audio_decoder = AUDIO_ARC_DECODER;
		return 0;
	} 
	
	ret = property_get("media.ffmpeg.audio.decoder",value,NULL);
	adec_print("media.amplayer.audiocodec = %s, t->type = %s\n", value, t->type);
	if (ret>0 && match_types(t->type,value))
	{	
		audio_decoder = AUDIO_FFMPEG_DECODER;
		return 0;
	} 
#endif	
	audio_decoder = AUDIO_ARC_DECODER; //set arc decoder as default
	return 0;
}

int get_audio_decoder(void)
{
	//adec_print("audio_decoder = %d\n", audio_decoder);

	return audio_decoder;
#if 0
	char value[PROPERTY_VALUE_MAX];
	if(property_get("media.amplayer.audiocodec",value,NULL)>0)
	{	
		adec_print("media.amplayer.audiocodec = %s\n", value);
		if (strcmp(value, "arc_decoder")==0){
			return AUDIO_ARC_DECODER;
		}
		else if (strcmp(value, "arm_decoder")==0){
			return AUDIO_ARM_DECODER;
		}
		else if(strcmp(value, "ffmpeg_decoder")==0){
			return AUDIO_FFMPEG_DECODER;
		}
		
		return AUDIO_ARC_DECODER;
	}
	return AUDIO_ARC_DECODER;
#endif	
}

int audiodec_init(aml_audio_dec_t *audec)
{
    int ret = 0;
    pthread_t    tid;
    adec_print("audiodec_init!");
    adec_message_pool_init(audec);
    get_output_func(audec);
    int nCodecType=audec->format;
    set_audio_decoder(nCodecType);

    if (get_audio_decoder() == AUDIO_ARC_DECODER) {
    		audec->adsp_ops.dsp_file_fd = -1;
		ret = pthread_create(&tid, NULL, (void *)adec_message_loop, (void *)audec);
    }
    else 
    {
		int codec_type=get_audio_decoder();
		RegisterDecode(audec,codec_type);
		ret = pthread_create(&tid, NULL, (void *)adec_armdec_loop, (void *)audec);
    }
    if (ret != 0) {
        adec_print("Create adec main thread failed!\n");
        return ret;
    }
    adec_print("Create adec main thread success! tid = %d\n", tid);
    audec->thread_pid = tid;
    return ret;
}
