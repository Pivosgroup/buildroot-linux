#ifndef MP_TYPES_H
#define MP_TYPES_H


//===============about common================//

#define INTERNAL_SUBTITLE_MAX 10
#define INTERNAL_AUDIOTRACK_MAX 10

typedef enum{
    MP_TONE_STEREO,
    MP_TONE_LEFTMONO,
    MP_TONE_RIGHTMONO,
    MP_TONE_SWAP,
}MP_Tone;

typedef enum{
    MP_PLAY_ALL = 0,
    MP_PLAY_NO_AUDIO,
    MP_PLAY_NO_VIDEO,    
}MP_MediaPlayMode;

/*
Define the service type.
*/
typedef enum
{
    MP_SERV_SELF = 0,//default player-self
    MP_SERV_HTTP = 100,
    MP_SERV_XUNLEI,
    MP_SERV_PPS,
    MP_SERV_GOUMANG,
    MP_SERV_FILE,
    MP_SERV_DIGITTV,
    MP_NUMOF_TYPE,
}MP_ServiceType;

typedef enum{
    MP_SUBTITLE_UNKOWN,
    MP_SUBTITLE_CH,
    MP_SUBTITLE_EN,
    MP_SUBTITLE_JP,
    MP_SUBTITLE_KOR,
    MP_SUBTITLE_RUS,
    MP_SUBTITLE_FR,
}MP_SubTitleLanguage;//enum subtitle language

typedef struct __SubTitleItem{
    int sub_pid;    
    unsigned short width;
    unsigned short height;    
    int internal_external; //0:internal_sub 1:external_sub       
    long long subtitle_size;  
    int sub_lang;  
    int resolution;
    //MP_SubTitleLanguage lang;    
}MP_SubTitleItem;

typedef enum{
    MP_AFORMAT_MPEG   = 0,
    MP_AFORMAT_PCM_S16LE = 1,
    MP_AFORMAT_AAC   = 2,
    MP_AFORMAT_AC3   =3,
    MP_AFORMAT_ALAW = 4,
    MP_AFORMAT_MULAW = 5,
    MP_AFORMAT_DTS = 6,
    MP_AFORMAT_PCM_S16BE = 7,
    MP_AFORMAT_FLAC = 8,
    MP_AFORMAT_COOK = 9,
    MP_AFORMAT_PCM_U8 = 10,
    MP_AFORMAT_MAX,
}MP_AudioTrackType;

typedef enum{
    MP_VFORMAT_UNKNOW =0,
    MP_VFORMAT_MPEG4_3,
    MP_VFORMAT_MPEG4_4,
    MP_VFORMAT_MPEG4_5,
    MP_VFORMAT_H264,
    MP_VFORMAT_MJPEG,
    MP_VFORMAT_MP4,
    MP_VFORMAT_H263,
    MP_VFORMAT_REAL_8,
    MP_VFORMAT_REAL_9,
    MP_VFORMAT_WMV3,
    MP_VFORMAT_WVC1,   
    MP_VFORMAT_MAX,
}MP_VideoTrackType;

typedef enum{
    MP_UNKNONWN_FILE = 0,
    MP_AVI_FILE,
    MP_MPEG_FILE,
    MP_WAV_FILE,
    MP_MP3_FILE,
    MP_AAC_FILE,
    MP_AC3_FILE,
    MP_RM_FILE,
    MP_DTS_FILE,        
    MP_MKV_FILE,    
    MP_MOV_FILE,
    MP_MP4_FILE,		
    MP_FLAC_FILE,
    MP_H264_FILE	,
    MP_M2V_FILE,
    MP_FLV_FILE,
    MP_P2P_FILE,
    MP_ASF_FILE,
    MP_FILE_MAX,  
}MP_AVMediaFileType;

typedef enum{
    MP_VASPECT_1X1,
    MP_VASPECT_4X3,
    MP_VASPECT_16X9,
    MP_VASPECT_16X10,
    MP_VASPECT_221X1,
    MP_VASPECT_5X4,  
    
}MP_VAspect_Ratio;


typedef struct __AudioTrackItem{
    int track_pid;
    MP_AudioTrackType type;   
}MP_AudioTrackItem;


//===============about comand================//
typedef enum{
    MP_CMD_UNKOWN = 0,
    MP_CMD_ADDSOURCE,    
    MP_CMD_PLAYFILE,
    MP_CMD_PLAYLIST,
    MP_CMD_PAUSE,
    MP_CMD_RESUME,
    MP_CMD_STOP,
    MP_CMD_FASTFORWARD,
    MP_CMD_FASTREWIND,
    MP_CMD_SEEK,
    MP_CMD_MUTE,
    MP_CMD_REPEAT,
    MP_CMD_BLACKOUT,
    MP_CMD_START,
    MP_CMD_SETVOLUME = 20,
    MP_CMD_SETTONE,
    MP_CMD_SETOPTION,
    MP_CMD_SETAUDIOTRACK,
    MP_CMD_SETSUBTITLE,
    MP_CMD_SETSONGSPECTRUM = 25,
    MP_CMD_GETVOLUME = 30,
    MP_CMD_QUERY,
    MP_CMD_GETAUDIOTRICK,
    MP_CMD_GETSUBTITLE,
    MP_CMD_GETPOSITION,
    MP_CMD_GETDURATION,
    MP_CMD_GETMEDIAINFO,
    MP_CMD_GETSTATUS,
    MP_CMD_GETALLMEDIAID,
    MP_CMD_QUITPLAYER = 100,
}MP_CommandType;

typedef struct{
    int client_id;
    int req_seq_no;
    MP_CommandType type;    
}MP_CommandHeader;

typedef struct{
    MP_CommandHeader head;
    int auxDatLen;
    void* auxDat;
}MP_CommandMsg;


typedef struct{    
    int urlLength;
    char *url;
    int isStart;
    int isRepeat;
    int isBlackout;
    int stPosition;
    int playDuration;
    MP_MediaPlayMode plmode;
}MP_PlayFileMsgBody;

typedef struct{  
    int mediaID;
    int speed;
}MP_TrickPlayMsgBody;//for ff/fr msg.


typedef struct{
    int mediaID;
    int isStart;
    int searchPos;
}MP_SeekMsgBody;

typedef struct{   
    int mediaID;
    int isMute;
}MP_MuteMsgBody;

typedef struct{    
    int mediaID;
    int isRepeat;
}MP_RepeatMsgBody;

typedef struct{   
    int mediaID;
    int isBlackout;
}MP_BlackoutMsgBody;

typedef struct{    
    int mediaID;
    int vol;//0~255
}MP_SetVolumeMsgBody;

typedef struct{    
    int mediaID;
    MP_Tone tone;
}MP_SetToneMsgBody;

typedef struct{
    int mediaID;
    int track_pid;    
}MP_SetAudioTrackMsgBody;

typedef struct{
    int mediaID;
    int isStartPop;
    int interval;
}MP_SetSpectrumMsgBody;

typedef struct{
    int mediaID;
    int sub_pid;   
}MP_SetSubtitleMsgBody;


typedef struct{
    int mediaID;
    char* url;    
}MP_GetInfoMsgBody;

typedef struct{
    int mediaID;
    MP_ServiceType optClass;//class
    int optSubClass;//subclass
    int auxDataLen;
    void* auxData;
}MP_SetOptionMsgBody;



typedef struct{    
    int mediaID;
    MP_ServiceType queryClass;//
    int querySubClass;//just self-defined
    int auxDataLen;
    void* auxData;//
}MP_QueryMsgBody;


//===============about response==================//
typedef enum{
    MP_RESP_COMMAND = 0,
    MP_RESP_STATE_CHANGED,
    MP_RESP_TIME_CHANGED,
    MP_RESP_DURATION,
    MP_RESP_VOLUME,
    MP_RESP_QUERY,
    MP_RESP_SET_OPTION,
    MP_RESP_BUFFERING_PORGRESS,
    MP_RESP_AUDIOTRCK,
    MP_RESP_SUBTITLE,
    MP_RESP_MEDIAINFO,
    MP_RESP_VALIDMEDIAID,
    MP_RESP_EXTLIBMSG,
    MP_RESP_TOTAL,
}MP_ResponseType;
typedef struct{
    int client_id;
    int req_seq_no;
    int resp_seq_no;
    MP_ResponseType type;
}MP_ResponseMsgHeader;

typedef enum{
    MP_STATE_UNKNOWN = 0,
    MP_STATE_INITING,
    MP_STATE_NORMALERROR,
    MP_STATE_FATALERROR,
    MP_STATE_PARSERED,
    MP_STATE_STARTED,  
    MP_STATE_PLAYING ,
    MP_STATE_PAUSED,
    MP_STATE_CONNECTING,
    MP_STATE_CONNECTDONE,
    MP_STATE_BUFFERING,
    MP_STATE_BUFFERINGDONE,
    MP_STATE_SEARCHING,
    MP_STATE_TRICKPLAY,     
    MP_STATE_MESSAGE_EOD,//    
    MP_STATE_MESSAGE_BOD,//the beginning of the data
    MP_STATE_TRICKTOPLAY,//from trick to normal play  
    MP_STATE_FINISHED,   
    MP_STATE_STOPED,
}MP_PlayBackState;


typedef struct{
    MP_ServiceType type;    
    int code;//app need translate it to local language,according type.
    int org_code;
}MP_ExtMsgCode;

typedef struct{
    MP_ResponseMsgHeader head;
    int mediaID;
    int auxDatLen;
    void* auxDat;
}MP_ResponseMsg;

typedef struct{
    int subcount;
    MP_SubTitleItem item[INTERNAL_SUBTITLE_MAX];
}MP_SubTitleRespBody;

typedef struct{
    int trackcount;
    MP_AudioTrackItem item[INTERNAL_AUDIOTRACK_MAX];
}MP_AudioTrackRespBody;

typedef struct{
    MP_PlayBackState state; 
    int isMuteOn;
    int isBlackout;
    int isRepeat;
    void* stateEx;
}MP_StateChangedRespBody;

//==============about media info======================
// 
#define _AUDIO_TRACK_MAX 8
#define _VIDEO_TRACK_MAX 8


#define TITLE_LEN_MAX   512
#define ALBUM_LEN_MAX   512
#define AUTHOR_LEN_MAX 512
#define COMMENT_LEN_MAX 512
#define YEAR_LEN_MAX 5
#define GENRE_LEN_MAX 32
#define COPYRIGHT_LEN_MAX 512

typedef struct{
    int title_len;
    char* title;
    int album_len;
    char* album;
    int author_len;
    char* author;
    int comment_len;
    char* comment;
    int year_len;
    char* year;
    int genre_len;
    char*genre;
    int copyright_len;
    char*copyright;
    int track;
    
    
}MP_AudioTagInfo;

typedef struct{
    MP_AudioTrackType audio_format;
    int audio_channel;   
    unsigned bit_rate;
    unsigned audio_samplerate; 
    int track_pid;
}MP_AudioMediaInfo;

typedef struct{
     MP_VideoTrackType video_format;
     int video_w;
     int video_h;
     int bit_rate;   
     MP_VAspect_Ratio ratio;
     int track_pid;
}MP_VideoMediaInfo;

typedef struct{
    MP_AVMediaFileType   filetype;
    long long size;
    int has_internal_sub; //if 1,yes.
    int sub_counts;
    int has_atrack;//if 1,yes.
    int has_vtrack;//if 1,yes.
    int atrack_counts;
    int vtrack_counts;
    int has_tag; // if 1,yes.
    int duration; //in seconds,,,
    MP_VideoMediaInfo vinfo[_VIDEO_TRACK_MAX];
    MP_AudioMediaInfo ainfo[_AUDIO_TRACK_MAX];  
    MP_SubTitleItem item[INTERNAL_SUBTITLE_MAX];
    int taginfo_len;
    MP_AudioTagInfo *taginfo;
    
}MP_AVMediaFileInfo;

//============================================

#define _MEDIA_ID_NUM_MAX 32

typedef struct _MP_ValidMediaID
{
    int num;
    int media_id[_MEDIA_ID_NUM_MAX];
}MP_ValidMediaIDInfo;

#endif // MPTYPES_H
