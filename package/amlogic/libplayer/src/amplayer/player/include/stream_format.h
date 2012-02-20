#ifndef _STREAM_FORMAT_H_
#define _STREAM_FORMAT_H_

#include "amports/vformat.h"
#include "amports/aformat.h"

typedef enum 
{
	NONE = 0,
	ID3V1,
	ID3V2,
	APEV1,
	APEV2,
	WMATAG,
	MPEG4TAG,
}audio_tag_type;

typedef enum 
{
    UNKNOWN_FILE 	= 0,
    AVI_FILE,		
    MPEG_FILE,		
    WAV_FILE,		
    MP3_FILE,		
    AAC_FILE,		
    AC3_FILE,		
    RM_FILE,			
    DTS_FILE,		    
    MKV_FILE,		   
    MOV_MP4_M4A_3GP_3G2_MJ2_FILE,      
    MP4_FILE,			
    FLAC_FILE,		
    H264_FILE,		
    M2V_FILE,        
    FLV_FILE,		
    P2P_FILE,        
    ASF_FILE,       
    STREAM_FILE,    
    APE_FILE,		   
    FILE_MAX		= 255,        
}pfile_type;

#endif
