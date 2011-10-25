// #define NEWBUFFERING
//#define DEBUG_RESYNC




#include        <stdlib.h>
#include        <stdio.h>
#include        <string.h>
#include        <signal.h>
#include        <math.h>

#define real float


#define LOCAL inline

#include "mpg123.h"
#include "huffman.h"
#include "mp3.h"

#include "adec.h"
#include "log.h"

adec_feeder_t *g_feeder=NULL;



static int fsizeold=0,ssize=0;
static int bitindex;
static unsigned char *wordpointer;
static int bitsleft;


#define MAXFRAMESIZE 1280
#define MAXFRAMESIZE2 (512+MAXFRAMESIZE)


static unsigned char bsspace[2][MAXFRAMESIZE2]; /* !!!!! */
static unsigned char *bsbufold=bsspace[0]+512;
static unsigned char *bsbuf=bsspace[1]+512;
static int bsnum=0;


int MP3_frames=0;
int MP3_eof=0;
int MP3_pause=0;
int MP3_filesize=0;
int MP3_fpos=0;      // current file position
int MP3_framesize=0; // current framesize
int MP3_bitrate=0;   // current bitrate
int MP3_samplerate=0;  // current samplerate
int MP3_resync=0;
int MP3_channels=0;
int MP3_bps=2;

static long outscale = 32768;
#include "tabinit.c"


LOCAL int  read_byte(void)
{
	unsigned long tmp;
	g_feeder->get_bits(&tmp,8);
	return tmp;
}
LOCAL int  read_2byte(void)
{
	unsigned long tmp;
	g_feeder->get_bits(&tmp,16);
	return tmp;
}

LOCAL int  reset_bits(void)
{
	unsigned long tmp,bit;
	bit=g_feeder->bits_left()&(7);
	if(bit>0)
		g_feeder->get_bits(&tmp,bit);
}
LOCAL int mp3_read(char *buf,int size){
  int i;
  reset_bits();
  for(i=0;i<size;i++)
  	{
	buf[i]=read_byte();
	//tmp=read_2byte();
	//buf[2*i]=(tmp&0xff00)>>8;
	//buf[2*i+1]=tmp&0xff;

  	}
  return size;
}

//#define getbitoffset
#define getbitoffset() ((-bitindex)&0x7)
#define getbyte()      (*wordpointer++)

LOCAL  void backbits(int n)
{
	bitindex -= n; 
	wordpointer += (bitindex>>3); 
	bitindex &= 0x7;
	return;
}
LOCAL unsigned int getbits(short number_of_bits)
{
  unsigned rval;
  if((bitsleft-=number_of_bits)<0) return 0;
  if(!number_of_bits) return 0;
         rval =   wordpointer[0];
         rval <<= 8;
         rval |= wordpointer[1];
         rval <<= 8;
         rval |= wordpointer[2];
         rval <<= bitindex;
         rval &= 0xffffff;
         bitindex += number_of_bits;
         rval >>= (24-number_of_bits);
         wordpointer += (bitindex>>3);
         bitindex &= 7;
  return rval;
}


LOCAL unsigned int getbits_fast(short number_of_bits)
{
	  unsigned rval;
	  if((bitsleft-=number_of_bits)<0) return 0;
	  if(!number_of_bits) return 0;
	  rval = wordpointer[0] << 8 | wordpointer[1];
         rval <<= bitindex;
         rval &= 0xffff;
         bitindex += number_of_bits;
         rval >>= (16-number_of_bits);
         wordpointer += (bitindex>>3);
         bitindex &= 7;
  return rval;
}

LOCAL unsigned int get1bit(void)
{
	  unsigned char rval;
	  if((--bitsleft)<0) return 0;
	  rval = *wordpointer << bitindex;
	  bitindex++;
	  wordpointer += (bitindex>>3);
	  bitindex &= 7;
	  return ((rval>>7)&1);
}

LOCAL void set_pointer(int backstep)
{
	  wordpointer = bsbuf + ssize - backstep;
	  if (backstep) fast_memcpy(wordpointer,bsbufold+fsizeold-backstep,backstep);
	  bitindex = 0;
	  bitsleft+=8*backstep;
}


LOCAL int stream_head_read(unsigned char *hbuf,uint32_t *newhead){
  	*newhead =	read_byte()<<24|
  				read_byte()<<16|
  				read_byte()<<8|
  				read_byte();

  	return TRUE;
}
LOCAL int stream_head_shift(unsigned char *hbuf,uint32_t *head){
	  *head <<= 8;
	  *head&=0xffffff00;
	  *head |=read_byte();
	  return 1;
}

int stream_read_frame_body(int size)
{
	  /* flip/init buffer for Layer 3 */
	  bsbufold = bsbuf;
	  bsbuf = bsspace[bsnum]+512;
	  bsnum = (bsnum + 1) & 1;
	  if( mp3_read(bsbuf,size) != size) return 0; // broken frame
	  bitindex = 0;
	  wordpointer = (unsigned char *) bsbuf;
	  bitsleft=8*size;
	  return 1;
}




static unsigned char *pcm_sample;   /* outbuffer address */
static int pcm_point = 0;           /* outbuffer offset */

static struct frame fr;

static int tabsel_123[2][3][16] = {
   { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
     {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
     {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} },

   { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,} }
};

static int freqs[9] = { 44100, 48000, 32000, 22050, 24000, 16000 , 11025 , 12000 , 8000 };



/*
 * decode a header and write the information
 * into the frame structure
 */
LOCAL int decode_header(struct frame *fr,uint32_t newhead){

    // head_check:
    if( (newhead & 0xffe00000) != 0xffe00000 ||  
        (newhead & 0x0000fc00) == 0x0000fc00) return FALSE;

    fr->lay = 4-((newhead>>17)&3);
//    if(fr->lay!=3) return FALSE;

    if( newhead & (1<<20) ) {
      fr->lsf = (newhead & (1<<19)) ? 0x0 : 0x1;
      fr->mpeg25 = 0;
    } else {
      fr->lsf = 1;
      fr->mpeg25 = 1;
    }

    if(fr->mpeg25)
      fr->sampling_frequency = 6 + ((newhead>>10)&0x3);
    else
      fr->sampling_frequency = ((newhead>>10)&0x3) + (fr->lsf*3);

    if(fr->sampling_frequency>8) return FALSE;  // valid: 0..8

    fr->error_protection = ((newhead>>16)&0x1)^0x1;
    fr->bitrate_index = ((newhead>>12)&0xf);
    fr->padding   = ((newhead>>9)&0x1);
    fr->extension = ((newhead>>8)&0x1);
    fr->mode      = ((newhead>>6)&0x3);
    fr->mode_ext  = ((newhead>>4)&0x3);
    fr->copyright = ((newhead>>3)&0x1);
    fr->original  = ((newhead>>2)&0x1);
    fr->emphasis  = newhead & 0x3;

    MP3_channels = fr->stereo    = (fr->mode == MPG_MD_MONO) ? 1 : 2;

    if(!fr->bitrate_index){
      return FALSE;
    }

switch(fr->lay){
  case 2:
    MP3_bitrate=tabsel_123[fr->lsf][1][fr->bitrate_index];
    MP3_samplerate=freqs[fr->sampling_frequency];
    fr->framesize = MP3_bitrate * 144000;
    fr->framesize /= MP3_samplerate;
    MP3_framesize=fr->framesize;
    fr->framesize += fr->padding - 4;
    break;
	
  case 3:
    if(fr->lsf)
      ssize = (fr->stereo == 1) ? 9 : 17;
    else
      ssize = (fr->stereo == 1) ? 17 : 32;
    if(fr->error_protection) ssize += 2;

    MP3_bitrate=tabsel_123[fr->lsf][2][fr->bitrate_index];
    MP3_samplerate=freqs[fr->sampling_frequency];
    fr->framesize  = MP3_bitrate * 144000;
    fr->framesize /= MP3_samplerate<<(fr->lsf);
    MP3_framesize=fr->framesize;
    fr->framesize += fr->padding - 4;
    break;
  case 1:
//    fr->jsbound = (fr->mode == MPG_MD_JOINT_STEREO) ? (fr->mode_ext<<2)+4 : 32;
    MP3_bitrate=tabsel_123[fr->lsf][0][fr->bitrate_index];
    MP3_samplerate=freqs[fr->sampling_frequency];
    fr->framesize  = MP3_bitrate * 12000;
    fr->framesize /= MP3_samplerate;
    MP3_framesize  = ((fr->framesize+fr->padding)<<2);
    fr->framesize  = MP3_framesize-4;
    break;
  default:
    MP3_framesize=fr->framesize=0;
    printf("Sorry, unsupported layer type.\n");
    return 0;
}
    if(fr->framesize<=0 || fr->framesize>MAXFRAMESIZE) return FALSE;

    return 1;
}


/*****************************************************************
 * read next frame     return number of frames read.
 */
LOCAL int read_frame(struct frame *fr){
	  uint32_t newhead;
	  union {
	    unsigned char buf[8];
	    unsigned long dummy; // for alignment
	  } hbuf;
	  int skipped,resyncpos;
	  int frames=0;

	resync:
	  skipped=MP3_fpos;
	  resyncpos=MP3_fpos;

	  set_pointer(512);
	  fsizeold=fr->framesize;       /* for Layer3 */
	  if(!stream_head_read(hbuf.buf,&newhead)) return 0;
	  if(!decode_header(fr,newhead)){
	    // invalid header! try to resync stream!
#ifdef DEBUG_RESYNC
	    printf("ReSync: searching for a valid header...  (pos=%X)\n",MP3_fpos);
#endif
	retry1:
	    while(!decode_header(fr,newhead)){
	      if(!stream_head_shift(hbuf.buf,&newhead)) return 0;
	    }
	    resyncpos=MP3_fpos-4;
	    // found valid header
#ifdef DEBUG_RESYNC
	    printf("ReSync: found valid hdr at %X  fsize=%ld  ",resyncpos,fr->framesize);
#endif
	    if(!stream_read_frame_body(fr->framesize)) return 0; // read body
	    set_pointer(512);
	    fsizeold=fr->framesize;       /* for Layer3 */
	    if(!stream_head_read(hbuf.buf,&newhead)) return 0;
		    if(!decode_header(fr,newhead)){
		      // invalid hdr! go back...
#ifdef DEBUG_RESYNC
		      printf("INVALID\n");
#endif
			//mp3_seek(resyncpos+1);
		      if(!stream_head_read(hbuf.buf,&newhead)) return 0;
		      goto retry1;
		    }
#ifdef DEBUG_RESYNC
	    printf("OK!\n");
	    ++frames;
#endif
	  }

  skipped=resyncpos-skipped;

  /* read main data into memory */
  if(!stream_read_frame_body(fr->framesize)){
    printf("\nBroken frame at 0x%X \n",resyncpos);
    return 0;
  }
  ++frames;

  if(MP3_resync){
    MP3_resync=0;
    if(frames==1) goto resync;
  }

  return frames;
}

static int _has_mmx = 0;  // used by layer2.c, layer3.c to pre-scale coeffs

#include "layer2.c"
#include "layer3.c"
#include "layer1.c"



/******************************************************************************/
/*           PUBLIC FUNCTIONS                  */
/******************************************************************************/

/* It's hidden from gcc in assembler */
extern void dct64_MMX(short *, short *, real *);
extern void dct64_MMX_3dnow(short *, short *, real *);
extern void dct64_MMX_3dnowex(short *, short *, real *);
extern void dct64_sse(short *, short *, real *);
void (*dct64_MMX_func)(short *, short *, real *);



int MP3_Init(adec_feeder_t *feeder){
	g_feeder=feeder;
	if(g_feeder==NULL)
		return -1;

    _has_mmx = 0;
    dct36_func = dct36;

    make_decode_tables(outscale);

    fr.synth=synth_1to1;

    fr.synth_mono=synth_1to1_mono2stereo;
    fr.down_sample=0;
    fr.down_sample_sblimit = SBLIMIT>>(fr.down_sample);

    init_layer2();
    init_layer3(fr.down_sample_sblimit);
    mp_msg(MSGT_DECAUDIO,MSGL_V,"MP3lib: init layer2&3 finished, tables done\n");
    return 0;	
}


// Read & decode a single frame. Called by sound driver.
int MP3_DecodeFrame(unsigned char *hova,short single){
   pcm_sample = hova;
   pcm_point = 0;

   if(!read_frame(&fr))
   	{
   	return(0);

   }
   if(single==-2){ set_pointer(512); return(1); }
   if(fr.error_protection) getbits(16); /* skip crc */
   fr.single=single;

   switch(fr.lay){
     case 2: do_layer2(&fr,single);break;
     case 3: do_layer3(&fr,single);break;
     case 1: do_layer1(&fr,single);break;
     default:
         return 0;	// unsupported
   }
//   ++MP3_frames;
   return(pcm_point?pcm_point:2);
}

int mp3_decode_frame(char *buf,int len,struct frame_fmt *fmt)
{
	int res;
	res=MP3_DecodeFrame(buf,-1);
	if(res>0 && fmt!=NULL)
		{
		fmt->channel_num=MP3_channels;
		fmt->sample_rate=MP3_samplerate;
		fmt->data_width=-1;
		}
	return res;
	}

am_codec_struct mp3_codec=
{
  .name="mp3lib",
  .format=AUDIO_FORMAT_MPEG,
  .used=0,
 .init=MP3_Init,
  .release=NULL,
  .decode_frame=mp3_decode_frame,
};


int register_mp3_codec(void)
{
	return register_audio_codec(&mp3_codec);
}


