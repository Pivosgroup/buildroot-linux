
#ifndef _AO_CONFIG_H 
#define _AO_CONFIG_H
//#define WORDS_BIGENDIAN
#define PATH_DEV_DSP   "/dev/dsp" 
#define PATH_DEV_MIXER "/dev/mixer"
//#define __linux__
#define MAX_OUTBURST	65536
#define OUTBURST 512

//typedef unsigned char       uint8_t;
//typedef signed char         int8_t;
typedef unsigned int        uint32_t;
//typedef signed int          int32_t;

#ifdef __GNUC__
  #define DECLARE_ALIGNED(n,t,v)       t v __attribute__ ((aligned (n)))
#else
  #define DECLARE_ALIGNED(n,t,v)      __declspec(align(n)) t v
#endif

#define fast_memcpy(a,b,c) memcpy(a,b,c)
#endif//_AO_CONFIG_H

