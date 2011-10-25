#ifndef PLIST_MNGR_H_
#define PLIST_MNGR_H_


//==========================================

#define PLIST_FILE_MAX 300
typedef enum _AutoPlayMode{
    MP_PLAY_ORDER = 0,
    MP_PLAY_LOOP,//loop play whole list
    MP_PLAY_SHUFFLE,//shuffle play whole list
    MP_PLAY_BOTH, //both shuffle and play whole list        
}MP_AutoPLayMode;


#ifdef __cplusplus
extern "C" {
#endif

int MP_StartAutoPlayTask(MP_AutoPLayMode mode);
int MP_StopAutoPlayTask();

int MP_InactiveMediaID(int mediaID);

int MP_ActiveMediaID(int mediaID);

int MP_AddFileToList(const char* url);

int MP_PlayNextFile();
int MP_DumpPlayList();

//===========================================
//directory play
int MP_AddDirToPlayList(char* path);




#ifdef __cplusplus
}
#endif

#endif // plist_mngr.h

