/**
 * \file adec-types.h
 * \brief  Definitiond Of Audio Dec Types
 * \version 1.0.0
 * \date 2011-03-08
 */
/* Copyright (C) 2007-2011, Amlogic Inc.
 * All right reserved
 *
 */
#ifndef ADEC_TYPES_H
#define ADEC_TYPES_H

typedef enum {
    AUDIO_FORMAT_UNKNOWN = 0,
    AUDIO_FORMAT_MPEG,
    AUDIO_FORMAT_PCM_S16LE,
    AUDIO_FORMAT_AAC,
    AUDIO_FORMAT_AC3,
    AUDIO_FORMAT_ALAW,
    AUDIO_FORMAT_MULAW,
    AUDIO_FORMAT_DTS,
    AUDIO_FORMAT_PCM_S16BE,
    AUDIO_FORMAT_FLAC,
    AUDIO_FORMAT_COOK,
    AUDIO_FORMAT_PCM_U8,
    AUDIO_FORMAT_ADPCM,
    AUDIO_FORMAT_AMR,
    AUDIO_FORMAT_RAAC,
    AUDIO_FORMAT_WMA,
    AUDIO_FORMAT_WMAPRO,
    AUDIO_AFORMAT_PCM_BLURAY,
    AUDIO_AFORMAT_ALAC,
    AUDIO_AFORMAT_VORBIS,
    AUDIO_FORMAT_AAC_LATM,
    AUDIO_FORMAT_APE,
    AUDIO_FORMAT_MAX,
} audio_format_t;

#define VALID_FMT(f)    ((f>AUDIO_FORMAT_UNKNOWN)&& (f<AUDIO_FORMAT_MAX))

typedef enum {
    IDLE,
    TERMINATED,
    STOPPED,
    INITING,
    INITTED,
    ACTIVE,
    PAUSED,
} adec_state_t;

#endif
