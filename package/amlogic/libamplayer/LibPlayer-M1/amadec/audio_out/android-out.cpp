/**
 * \file android-out.cpp
 * \brief  Functions of Auduo output control for Android Platform
 * \version 1.0.0
 * \date 2011-03-08
 */
/* Copyright (C) 2007-2011, Amlogic Inc.
 * All right reserved
 *
 */
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>
#include <media/AudioSystem.h>
#include <media/AudioTrack.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

extern "C" {
#include <audio-dec.h>
#include <adec-pts-mgt.h>
#include <log-print.h>
#include "aml_resample.h"
}

namespace android
{

static Mutex mLock;

/**
 * \brief callback function invoked by android
 * \param event type of event notified
 * \param user pointer to context for use by the callback receiver
 * \param info pointer to optional parameter according to event type
 */
static void dump_pcm_bin(char *path,char *buf,int size)
{
	FILE *fp=fopen(path,"ab+");
	 if(fp!= NULL){
		   fwrite(buf,1,size,fp);
		   fclose(fp);
	}
}


/**
 *  try to read as much data as len from dsp buffer
 */
static int dsp_pcm_read(aml_audio_dec_t*audec,char *data_in,int len)
{
   int pcm_ret=0,pcm_cnt_bytes=0;
   while(pcm_cnt_bytes<len){
      pcm_ret=audec->adsp_ops.dsp_read(&audec->adsp_ops, data_in+pcm_cnt_bytes, len-pcm_cnt_bytes);
	  if(pcm_ret==0)//indicate there is no data in dsp_buf;
	  	break;
	  pcm_cnt_bytes+=pcm_ret;
   }
   return pcm_cnt_bytes/sizeof(short);
}


void audioCallback(int event, void* user, void *info)
{
    int len;
    AudioTrack::Buffer *buffer = static_cast<AudioTrack::Buffer *>(info);
    aml_audio_dec_t *audec = static_cast<aml_audio_dec_t *>(user);
	
    int resample_enable;
	af_resampe_ctl_t *paf_resampe_ctl;
	short data_in[MAX_NUMSAMPS_PERCH*DEFALT_NUMCH], *data_out;
	short outbuftmp16[MAX_NUMSAMPS_PERCH*DEFALT_NUMCH];
    int NumSamp_in,NumSamp_out,NumCh,NumSampRequir=0;
	static int print_flag=0;
	int outbuf_offset=0;
    if (event != AudioTrack::EVENT_MORE_DATA) {
        adec_print("audioCallback: event = %d \n", event);
        return;
    }

    if (buffer == NULL || buffer->size == 0) {
        adec_print("audioCallback: Wrong buffer\n");
        return;
    }

    adec_refresh_pts(audec);

    if (audec->adsp_ops.dsp_on) {
        //------------------------------------------
        NumCh=buffer->channelCount;		
        resample_enable= af_get_reample_enable_flag();
		paf_resampe_ctl=af_resampler_ctx_get();
        data_out=buffer->i16;
		NumSamp_out = buffer->size/sizeof(short);
		NumSampRequir=NumSamp_out;
		//adec_print("resample_enable = %d\n",resample_enable);
		//adec_print("REQURE_NUM-----------------------------%d\n",NumSamp_out);
		//adec_print("PCM_IN_OUTSAMPBUF:%d\n",paf_resampe_ctl->OutSampReserveLen);
        if(resample_enable){    
			   int pcm_cnt=0;
			   if (!paf_resampe_ctl->InitFlag)
                   af_resample_set_SampsNumRatio(paf_resampe_ctl);
			   af_get_pcm_in_resampler(paf_resampe_ctl,data_out+outbuf_offset,&NumSampRequir);
			   
			   //adec_print("RETURN_SIZE_1:%d    OutSampReserve=%d \n",NumSampRequir,paf_resampe_ctl->OutSampReserveLen);

			   outbuf_offset += NumSampRequir;
			   NumSamp_out   -= NumSampRequir;
			   while(NumSamp_out >= DEFALT_NUMSAMPS_PERCH*NumCh)
			   {   
			       int delta_input_sampsnum=af_get_delta_inputsampnum(paf_resampe_ctl,NumCh);
			       NumSamp_in =dsp_pcm_read(audec,(char*)data_in,delta_input_sampsnum*sizeof(short));	
				   af_resample_process_linear_inner(paf_resampe_ctl,data_in, &NumSamp_in,data_out+outbuf_offset,&pcm_cnt,NumCh);

				   //adec_print("RETURN_SIZE_2:%d    OutSampReserve=%d \n",pcm_cnt,paf_resampe_ctl->OutSampReserveLen);

				   if(pcm_cnt==0)
				   	  goto resample_out;
				   outbuf_offset += pcm_cnt;
				   NumSamp_out   -= pcm_cnt;
			   }
			   
               if(NumSamp_out>0)
			   {
			       int delta_input_sampsnum=af_get_delta_inputsampnum(paf_resampe_ctl,NumCh);
			       NumSamp_in =dsp_pcm_read(audec,(char*)data_in,delta_input_sampsnum*sizeof(short));				   
            	   af_resample_process_linear_inner(paf_resampe_ctl,data_in, &NumSamp_in,outbuftmp16,&pcm_cnt,NumCh);
                   if(pcm_cnt==0)
				   	   goto resample_out;

				   //adec_print("RETURN_SIZE_3:%d    OutSampReserve=%d \n",NumSamp_out,pcm_cnt-NumSamp_out);
   
				   memcpy(data_out+outbuf_offset,outbuftmp16,NumSamp_out*sizeof(short));
				   outbuf_offset +=NumSamp_out;
                   memcpy(paf_resampe_ctl->OutSampReserveBuf,outbuftmp16+NumSamp_out,(pcm_cnt-NumSamp_out)*sizeof(short));
                   paf_resampe_ctl->OutSampReserveLen = (pcm_cnt-NumSamp_out);
			   }
			   
		}else{
              if(paf_resampe_ctl->OutSampReserveLen > 0){
			      af_get_pcm_in_resampler(paf_resampe_ctl,data_out+outbuf_offset,&NumSampRequir);
				  //adec_print("RETURN_SIZE_4:%d    OutSampReserve=%d \n",NumSampRequir,paf_resampe_ctl->OutSampReserveLen);
			      outbuf_offset += NumSampRequir;
			      NumSamp_out   -= NumSampRequir;
				  NumSampRequir  =  NumSamp_out;
              } 

			  if(paf_resampe_ctl->ResevedSampsValid > 0){
                  af_get_unpro_inputsampnum(paf_resampe_ctl,data_out+outbuf_offset,&NumSampRequir);
				  //adec_print("RETURN_SIZE_5:%d    OutSampReserve=%d \n",NumSampRequir,paf_resampe_ctl->ResevedSampsValid);
				  outbuf_offset += NumSampRequir;
			      NumSamp_out   -= NumSampRequir;
			  }
			  
              if((paf_resampe_ctl->OutSampReserveLen==0) && (paf_resampe_ctl->ResevedSampsValid==0))
                  af_resample_stop_process(paf_resampe_ctl);

			  if(NumSamp_out > 0){
			  	  len=audec->adsp_ops.dsp_read(&audec->adsp_ops, (char*)(data_out+outbuf_offset),NumSamp_out*sizeof(short));
                  outbuf_offset += len/sizeof(short);
			  }
        }
	resample_out:
		
	    buffer->size=outbuf_offset*sizeof(short);
		//adec_print("RETURN_NUM-----------------------------%d\n\n\n",outbuf_offset);
		//---------------------------------------------
    } else {
        //adec_print("audioCallback: dsp not work!\n");
    }

    return;
}

/**
 * \brief output initialization
 * \param audec pointer to audec
 * \return 0 on success otherwise negative error code
 */
extern "C" int android_init(struct aml_audio_dec* audec)
{
    adec_print("android out init");

    status_t status;
    AudioTrack *track;
    audio_out_operations_t *out_ops = &audec->aout_ops;
    Mutex::Autolock _l(mLock);

    track = new AudioTrack();
    if (track == NULL) {
        adec_print("AudioTrack Create Failed!");
        return -1;
    }

#ifdef _VERSION_ICS
    status = track->set(AUDIO_STREAM_MUSIC,
                        audec->samplerate,
                        AUDIO_FORMAT_PCM_16_BIT,
                        (audec->channels == 1) ? AUDIO_CHANNEL_OUT_MONO : AUDIO_CHANNEL_OUT_STEREO,
                        0,       // frameCount
                        0,       // flags
                        audioCallback,
                        audec,    // user when callback
                        0,       // notificationFrames
                        0,       // shared buffer
                        0);
#else
    status = track->set(AudioSystem::MUSIC,
                        audec->samplerate,
                        AudioSystem::PCM_16_BIT,
                        (audec->channels == 1) ? AudioSystem::CHANNEL_OUT_MONO : AudioSystem::CHANNEL_OUT_STEREO,
                        0,       // frameCount
                        0,       // flags
                        audioCallback,
                        audec,    // user when callback
                        0,       // notificationFrames
                        0,       // shared buffer
                        0);
#endif

    if (status != NO_ERROR) {
        adec_print("track->set returns %d", status);
        adec_print("audio out samplet %d", audec->samplerate);
        adec_print("audio out channels %d", audec->channels);
        delete track;
        track = NULL;
        return -1;

    }
    af_resample_linear_init();
    out_ops->private_data = (void *)track;
    return 0;
}

/**
 * \brief start output
 * \param audec pointer to audec
 * \return 0 on success otherwise negative error code
 *
 * Call android_start(), then the callback will start being called.
 */
extern "C" int android_start(struct aml_audio_dec* audec)
{
    adec_print("android out start");

    status_t status;
    audio_out_operations_t *out_ops = &audec->aout_ops;
    AudioTrack *track = (AudioTrack *)out_ops->private_data;

    Mutex::Autolock _l(mLock);

    if (!track) {
        adec_print("No track instance!\n");
        return -1;
    }

    status = track->initCheck();
    if (status != NO_ERROR) {
        delete track;
        out_ops->private_data = NULL;
        return -1;
    }

    track->start();
    adec_print("AudioTrack initCheck OK and started.");

    return 0;
}

/**
 * \brief pause output
 * \param audec pointer to audec
 * \return 0 on success otherwise negative error code
 */
extern "C" int android_pause(struct aml_audio_dec* audec)
{
    adec_print("android out pause");

    audio_out_operations_t *out_ops = &audec->aout_ops;
    AudioTrack *track = (AudioTrack *)out_ops->private_data;

    Mutex::Autolock _l(mLock);

    if (!track) {
        adec_print("No track instance!\n");
        return -1;
    }

    track->pause();

    return 0;
}

/**
 * \brief resume output
 * \param audec pointer to audec
 * \return 0 on success otherwise negative error code
 */
extern "C" int android_resume(struct aml_audio_dec* audec)
{
    adec_print("android out resume");

    audio_out_operations_t *out_ops = &audec->aout_ops;
    AudioTrack *track = (AudioTrack *)out_ops->private_data;

    Mutex::Autolock _l(mLock);

    if (!track) {
        adec_print("No track instance!\n");
        return -1;
    }

    track->start();

    return 0;
}

/**
 * \brief stop output
 * \param audec pointer to audec
 * \return 0 on success otherwise negative error code
 */
extern "C" int android_stop(struct aml_audio_dec* audec)
{
    adec_print("android out stop");

    audio_out_operations_t *out_ops = &audec->aout_ops;
    AudioTrack *track = (AudioTrack *)out_ops->private_data;

    Mutex::Autolock _l(mLock);

    if (!track) {
        adec_print("No track instance!\n");
        return -1;
    }

    track->stop();

    /* release AudioTrack */
    delete track;
    out_ops->private_data = NULL;

    return 0;
}

/**
 * \brief get output latency in ms
 * \param audec pointer to audec
 * \return output latency
 */
extern "C" unsigned long android_latency(struct aml_audio_dec* audec)
{
    unsigned long latency;
    audio_out_operations_t *out_ops = &audec->aout_ops;
    AudioTrack *track = (AudioTrack *)out_ops->private_data;

    if (track) {
        latency = track->latency();
        return latency;
    }

    return 0;
}

/**
 * \brief mute output
 * \param audec pointer to audec
 * \param en  1 = mute, 0 = unmute
 * \return 0 on success otherwise negative error code
 */
extern "C" int android_mute(struct aml_audio_dec* audec, adec_bool_t en)
{
    adec_print("android out mute");

    audio_out_operations_t *out_ops = &audec->aout_ops;
    AudioTrack *track = (AudioTrack *)out_ops->private_data;

    Mutex::Autolock _l(mLock);

    if (!track) {
        adec_print("No track instance!\n");
        return -1;
    }

    track->mute(en);

    return 0;
}

/**
 * \brief set output volume
 * \param audec pointer to audec
 * \param vol volume value
 * \return 0 on success otherwise negative error code
 */
extern "C" int android_set_volume(struct aml_audio_dec* audec, float vol)
{
    adec_print("android set volume");

    audio_out_operations_t *out_ops = &audec->aout_ops;
    AudioTrack *track = (AudioTrack *)out_ops->private_data;

    Mutex::Autolock _l(mLock);

    if (!track) {
        adec_print("No track instance!\n");
        return -1;
    }

    track->setVolume(vol, vol);

    return 0;
}

/**
 * \brief set left/right output volume
 * \param audec pointer to audec
 * \param lvol refer to left volume value
 * \param rvol refer to right volume value
 * \return 0 on success otherwise negative error code
 */
extern "C" int android_set_lrvolume(struct aml_audio_dec* audec, float lvol,float rvol)
{
    adec_print("android set left and right volume separately");

    audio_out_operations_t *out_ops = &audec->aout_ops;
    AudioTrack *track = (AudioTrack *)out_ops->private_data;

    Mutex::Autolock _l(mLock);

    if (!track) {
        adec_print("No track instance!\n");
        return -1;
    }

    track->setVolume(lvol, rvol);

    return 0;
}

extern "C" void android_basic_init()
{
    adec_print("android basic init!");

    Mutex::Autolock _l(mLock);

    sp<ProcessState> proc(ProcessState::self());
}

/**
 * \brief get output handle
 * \param audec pointer to audec
 */
extern "C" void get_output_func(struct aml_audio_dec* audec)
{
    audio_out_operations_t *out_ops = &audec->aout_ops;

    out_ops->init = android_init;
    out_ops->start = android_start;
    out_ops->pause = android_pause;
    out_ops->resume = android_resume;
    out_ops->stop = android_stop;
    out_ops->latency = android_latency;
    out_ops->mute = android_mute;
    out_ops->set_volume = android_set_volume;
    out_ops->set_lrvolume = android_set_lrvolume;
}

}
