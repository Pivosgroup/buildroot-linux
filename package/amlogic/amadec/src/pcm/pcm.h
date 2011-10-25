#ifndef AMADECD_PCM_H
#define AMADECD_PCM_H

#ifdef __GNUC__
#    define AV_GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > x || __GNUC__ == x && __GNUC_MINOR__ >= y)
#else
#    define AV_GCC_VERSION_AT_LEAST(x,y) 0
#endif

#ifndef av_always_inline
#if AV_GCC_VERSION_AT_LEAST(3,1)
#    define av_always_inline __attribute__((always_inline)) inline
#else
#    define av_always_inline inline
#endif
#endif

typedef enum codec_type{
  CODEC_ID_NONE,
  CODEC_ID_PCM_S16LE,
  CODEC_ID_PCM_S16BE,
  CODEC_ID_PCM_U8,
  CODEC_ID_PCM_MULAW,
  CODEC_ID_PCM_ALAW,
}codec_type_t;

#endif
