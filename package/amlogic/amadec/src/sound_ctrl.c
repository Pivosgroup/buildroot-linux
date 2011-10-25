#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#include <sys/poll.h>
#include "log.h"
#include "sound_ctrl.h"

#define printf(fmt,args...) lp(0,"[sound ctrl] "fmt,##args)

#define LEVEL_BASIC		(1<<0)
#define LEVEL_INACTIVE		(1<<1)
#define LEVEL_ID		(1<<2)

#define MAX_VALUE(m1,m2) (((m1)>(m2))?(m1):(m2))

char card[16] = "default";

static int get_control(snd_hctl_elem_t *elem, int level, long *value)
{
	int err;
	unsigned int item, idx, count, *tlv;
	snd_ctl_elem_type_t type;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *control;
	snd_aes_iec958_t iec958;
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&control);
	if ((err = snd_hctl_elem_info(elem, info)) < 0) {
		printf("Control %s snd_hctl_elem_info error: %s\n", card, snd_strerror(err));
		return err;
	}
	if (level & LEVEL_ID) {
		snd_hctl_elem_get_id(elem, id);
	}
	count = snd_ctl_elem_info_get_count(info);
	type = snd_ctl_elem_info_get_type(info);
	
	if (level & LEVEL_BASIC) {
		if ((err = snd_hctl_elem_read(elem, control)) < 0) {
			printf("Control %s element read error: %s\n", card, snd_strerror(err));
			return err;
		}

		for (idx = 0; idx < count; idx++) {
			switch (type) {
			case SND_CTL_ELEM_TYPE_BOOLEAN:
				//printf("%s", snd_ctl_elem_value_get_boolean(control, idx) ? "on" : "off");
				break;
			case SND_CTL_ELEM_TYPE_INTEGER:
				 *value = snd_ctl_elem_value_get_integer(control, idx);
				break;
			default:
				printf("?");
				break;
			}
		}
	}

	return 0;
}


int sound_mute_set(char * id_string , long num, int rw, long * value)
{
	int err;
	//char card[16] = "default";
	snd_ctl_t *handle = NULL;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	//char *ptr;
	unsigned int idx = 0, count;
	long tmp, min, max;
	//int dir_flag = 1;
	
	snd_ctl_elem_type_t type;
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);

	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_name(id, id_string);

	if (handle == NULL && (err = snd_ctl_open(&handle, card, 0)) < 0) 
		{
		printf("Control %s open error: %s\n", card, snd_strerror(err));
		return err;
	}

	snd_ctl_elem_info_set_id(info, id);
	if ((err = snd_ctl_elem_info(handle, info)) < 0) 
	{
		printf("Cannot find the given element from control %s\n", card);	
		snd_ctl_close(handle);
		handle = NULL;		
		return err;
	}
	snd_ctl_elem_info_get_id(info, id);	
	type = snd_ctl_elem_info_get_type(info);
	count = snd_ctl_elem_info_get_count(info);
	snd_ctl_elem_value_set_id(control, id);
	if (rw) 
	{
		for (idx = 0; idx < count && idx < 128; idx++) {
			switch (type) {
			case SND_CTL_ELEM_TYPE_BOOLEAN:
				tmp = 0;
				if (num >= 1) {
						tmp = 1;
				}
				snd_ctl_elem_value_set_boolean(control, idx, tmp);
				break;
			case SND_CTL_ELEM_TYPE_INTEGER:
				min = snd_ctl_elem_info_get_min(info);
				max = snd_ctl_elem_info_get_max(info);
				if ((num >= min) && (num <= max))
					tmp = num;
				else if (num < min)
					tmp = min;
				else if (num > max)
					tmp = max;
				snd_ctl_elem_value_set_integer(control, idx, tmp);
				break;
			default:
				break;
			}
		}
		if ((err = snd_ctl_elem_write(handle, control)) < 0) {
			printf("Control %s element write error: %s\n", card, snd_strerror(err));
			snd_ctl_close(handle);
			handle = NULL;
			
			return err;
		}
	}else {
	
		snd_hctl_t *hctl;
		snd_hctl_elem_t *elem;
		if ((err = snd_hctl_open(&hctl, card, 0)) < 0) {
			printf("Control %s open error: %s\n", card, snd_strerror(err));
			return err;
		}
		if ((err = snd_hctl_load(hctl)) < 0) {
			printf("Control %s load error: %s\n", card, snd_strerror(err));
			return err;
		}
		elem = snd_hctl_find_elem(hctl, id);
		if (elem)
			get_control(elem, LEVEL_BASIC | LEVEL_ID, value);
		else
			printf("Could not find the specified element\n");
		snd_hctl_close(hctl);
		}

	snd_ctl_close(handle);
	handle = NULL;
}


static int get_data_control(snd_hctl_elem_t *elem, int level, char *value)
{
	int err;
	unsigned int idx, count;
	snd_ctl_elem_type_t type;
	//snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *control;
	
	//snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&control);
	
	if ((err = snd_hctl_elem_info(elem, info)) < 0) {
		printf("Control %s snd_hctl_elem_info error: %s\n", card, snd_strerror(err));
		return err;
	}
  //if (level & LEVEL_ID) {
	//	snd_hctl_elem_get_id(elem, id);
	//}
	count = snd_ctl_elem_info_get_count(info);
	type = snd_ctl_elem_info_get_type(info);
	
	if (level & LEVEL_BASIC) {
		if ((err = snd_hctl_elem_read(elem, control)) < 0) {
			printf("Control %s element read error: %s\n", card, snd_strerror(err));
			return err;
		}

		for (idx = 0; idx < count; idx++) {
			switch (type) {
			case SND_CTL_ELEM_TYPE_BYTES:
				value[idx] = snd_ctl_elem_value_get_byte(control, idx);
				break;
			default:
				printf("?");
				break;
			}
		}
	}

	return 0;
}


int sound_data_read(char * id_string, char* value)
{
	int err;
	snd_ctl_elem_id_t *id;
	snd_hctl_t *hctl;
	snd_hctl_elem_t *elem;
	unsigned int idx = 0, count;
	
	snd_ctl_elem_id_alloca(&id);
	
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_name(id, id_string);

		
	if ((err = snd_hctl_open(&hctl, card, 0)) < 0) {
		printf("Control %s open error: %s\n", card, snd_strerror(err));
		return err;
	}
	if ((err = snd_hctl_load(hctl)) < 0) {
		printf("Control %s load error: %s\n", card, snd_strerror(err));
		return err;
	}
	elem = snd_hctl_find_elem(hctl, id);
	if (elem)
		get_data_control(elem, LEVEL_BASIC, value);
	else
		printf("Could not find the specified element\n");
	snd_hctl_close(hctl);

}
#define spec_f_tofixed(x)	((signed int)((x) * (1<<28)+0.5))
  

static inline signed int _SEXW(int x)
{
	signed int res;
#if 0
	__asm__ __volatile__(
	"sexw %0,%1\r\n"
	:"=r"(res)
	:"r"(x)
	);
#else
    res = x;
#endif
	return res;		
}
#define SHIFT4(x) ((x)<<4)
#define SEXW(x)	  (_SEXW(x))


static inline int mul32(int x, int y)
{
    int res;
#if 0 
   __asm__ __volatile__(
    "mpyh %0,%1,%2\r\n"
    :"=r"(res)
    :"r"(x),"r"(y)
    );	
#else
    res = x * y;
#endif
    return res;   	
}
static const int sin_tab[16]=
{
	spec_f_tofixed(0.000000),spec_f_tofixed(0.195090),spec_f_tofixed(0.382683),spec_f_tofixed(0.555570),
	spec_f_tofixed(0.707107),spec_f_tofixed(0.831470),spec_f_tofixed(0.923880),spec_f_tofixed(0.980785),
	spec_f_tofixed(1.000000),spec_f_tofixed(0.980785),spec_f_tofixed(0.923880),spec_f_tofixed(0.831470),
	spec_f_tofixed(0.707107),spec_f_tofixed(0.555570),spec_f_tofixed(0.382683),spec_f_tofixed(0.195090)	
};

static const int cos_tab[16]=
{
    spec_f_tofixed(1.000000),spec_f_tofixed(0.980785),spec_f_tofixed(0.923880),spec_f_tofixed(0.831470),
	spec_f_tofixed(0.707107),spec_f_tofixed(0.555570),spec_f_tofixed(0.382683),spec_f_tofixed(0.195090),
	spec_f_tofixed(0.000000),spec_f_tofixed(-0.195090),spec_f_tofixed(-0.382683),spec_f_tofixed(-0.555570),
	spec_f_tofixed(-0.707107),spec_f_tofixed(-0.831470),spec_f_tofixed(-0.923880),spec_f_tofixed(-0.980785)
};

static int isqrt(int v)
{
	int temp,nHat = 0,b = 0x8000,bshft = 15;
	do
	{
		if(v >= (temp = (((nHat<<1)+b)<<bshft--)))
		{
			nHat +=b;
			v -= temp;
		
		}
	}while(b>>=1);

	return nHat;
}

static void FFT(int dataR[],int dataI[]){
	int x0,x1,x2,x3,x4;
	int L,i,j,k,b,p,invert_pos;
	int TR,TI,temp;
	
/********** following code invert sequence ************/
/* i为原始存放位置，最后得invert_pos为倒位序存放位置 */

	for(i=0;i<32;i++){ 
		x0=x1=x2=x3=x4=0;
		x0=i&0x01; 
		x1=(i>>1)&0x01; 
		x2=(i>>2)&0x01; 
		x3=(i>>3)&0x01;
		x4=(i>>4)&0x01;
		
		invert_pos=x0*16+x1*8+x2*4+x3*2+x4;
		dataI[invert_pos]=dataR[i];
	}
	for(i=0;i<32;i++){ 
		dataR[i]=dataI[i]; 
		dataI[i]=0; 
	}
/************** following code FFT *******************/
	for(L=1;L<=5;L++) { /* for(1) */
		b=1; //旋转因子个数
		i=L-1;
		while(i>0) {
			b<<=1;
			i--;
		} /* b= 2^(L-1) */
		for(j=0;j<=b-1;j++){  /* for (2) */
			p=1; //旋转因子指数
			i=5-L;
			while(i>0) {/* p=pow(2,7-L)*j; */
				p<<=1;
				i--;
			}
			p*=j;
			for(k=j;k<32;k=k+2*b) {/* for (3) */ 
				TR=dataR[k]; 
				TI=dataI[k]; 
				temp=dataR[k+b];

				dataR[k] = (dataR[k] + mul32(SHIFT4(dataR[k+b]),cos_tab[p]) + mul32(SHIFT4(dataI[k+b]),sin_tab[p]))>>1;
				//dataR[k]=dataR[k]+dataR[k+b]*cos_tab[p]+dataI[k+b]*sin_tab[p];
				
				dataI[k] = (dataI[k] - mul32(SHIFT4(dataR[k+b]),sin_tab[p]) + mul32(SHIFT4(dataI[k+b]),cos_tab[p]))>>1;
				//dataI[k]=dataI[k]-dataR[k+b]*sin_tab[p]+dataI[k+b]*cos_tab[p];
								
				dataR[k+b] = (TR - mul32(SHIFT4(dataR[k+b]),cos_tab[p]) - mul32(SHIFT4(dataI[k+b]),sin_tab[p]))>>1;
				//dataR[k+b]=TR-dataR[k+b]*cos_tab[p]-dataI[k+b]*sin_tab[p];
								
				dataI[k+b] = (TI + mul32(SHIFT4(temp),sin_tab[p]) - mul32(SHIFT4(dataI[k+b]),cos_tab[p]))>>1;
				//dataI[k+b]=TI+temp*sin_tab[p]-dataI[k+b]*cos_tab[p];
			} /* END for (3) */
		} /* END for (2) */
	} /* END for (1) */
}

int get_audioSpectrum(MP_AudioSpectrum *data)
{
    int i, j=0;
    char sample[128];
    int data0, data1;
    int dataR[32], dataI[32];
    int   amplitude[32];  
    short *fetch = NULL;
    short *sample_  = NULL;
    MP_AudioSpectrum dt;

     if(data == NULL)
     {
        printf("[%s]: Must allocate memory for data copy\n",__FUNCTION__);
        return -1;
     }
        

    memset(sample, 0, 128);
    sound_data_read("Playback Data Get", sample);
    sample_ = (short*)sample;

    //notes : now is used for 16 bit pcm output.
    fetch = (short*)sample;
        
    for(i=0;i<16;i++)//fetch 16 samples for left channel
    {
        data0 = *fetch++;
        dataR[i] = SEXW(data0);
    }
    fetch += 16;
    for(i=0;i<16;i++)//fetch  another 16 samples
    {
        data0 = *fetch++;
        dataR[i+16] = SEXW(data0);
    }
    FFT(dataR, dataI);//do fft transfrom to the 32 sample
    for(i=0;i<32;i++)
    { 
    	dataR[i]=isqrt(dataR[i]*dataR[i]+dataI[i]*dataI[i]);
    	amplitude[i] = dataR[i]<<1;
    }
    dt.flag = 1001;
    for(i = 0;i<32;i++)
    {
        dt.sample_point[i].left =  amplitude[i] & 0x0000ffff; 
        dt.sample_point[i].right = amplitude[i] & 0x0000ffff;        
    }
    //dt.lpeak_value =  dt.sample_point[0].left ;
    dt.rpeak_value = dt.sample_point[0].right;
    for(i=1;i<32;i++)
    {
        //dt.lpeak_value = MAX_VALUE(dt.lpeak_value, dt.sample_point[i].left);
        dt.rpeak_value = MAX_VALUE(dt.rpeak_value, dt.sample_point[i].right);    	 
    }
    dt.lpeak_value = dt.rpeak_value;
    memcpy((void*)data,(void*)&dt,sizeof(MP_AudioSpectrum));

    return 0;    	

}
