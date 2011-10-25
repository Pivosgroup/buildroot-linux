#ifndef MP_API_SYNC_H
#define MP_API__SYNC_H

#include"mp_types.h"


#ifdef __cplusplus
extern "C" {
#endif


//======================================================================
//synchronouce API,just for android adapter wrapper.

/*
Description: Get volume synchronoucely.
        @ media_id: refer to play media returns.
        @ vol: OUT value.   
        @ if 0 ok,others failed
*/
int MP_GetVolumeSync(int media_id,int* vol);
/*
Description: Get play position synchronoucely
        @ media_id: refer to play media returns.
        @ pos: OUT value.   
        @ if 0 ok,others failed
*/
int MP_GetPositionSync(int media_id,int* pos);
/*
Description: Get play status synchronoucely
        @ media_id: refer to play media returns.
        @ status: OUT value.   
        @isMuteon: OUT value,0 or 1(mute)
        @isRepeat: OUT value,0 or 1(repeat)
        @ if 0 ok,others failed
Comments: 
        20100618: isMuteon,isRepeat not ready.
*/
int MP_GetStatusSync(int media_id,int* status,int* isMuteon,int* isRepeat);

/*
Description: Get play duration synchronoucely
        @ media_id: refer to play media returns.
        @ dur: OUT value.   
        @ if 0 ok,others failed
*/
int MP_GetDurationSync(int media_id,int* dur);


//======================================================================
//some Miscs for testing,just can be as helpers.. 

int MP_GetValidMediaIDSync(MP_ValidMediaIDInfo *mediaidpool);

#ifdef __cplusplus
}
#endif

#endif // MP_API_H

