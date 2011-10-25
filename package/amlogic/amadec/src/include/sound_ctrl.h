#ifndef _SOUND_CTRL_H_
#define _SOUND_CTRL_H_


int sound_mute_set(char * id_string , long num, int rw, long * value);



typedef struct _spectrumPoint{
    unsigned short int left;// u16 left,
    unsigned short int right;// u16 right;
}MP_SpectrumPoint;
 
typedef struct _audioSpectrum{
    int flag;//MP_DATA_SPECTRUM  just refer to 1001
    MP_SpectrumPoint sample_point[32];
    int lpeak_value;//left peak value
    int rpeak_value;//right peak value
}MP_AudioSpectrum;


//data just refer to spectrum data. if return 0,others need drop it.
// data need need allocate memory by caller.
int get_audioSpectrum(MP_AudioSpectrum *data);


#endif
