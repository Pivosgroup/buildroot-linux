#ifndef MP_UTILS_H
#define MP_UTILS_H

#include"mp_types.h"
#ifdef __cplusplus
extern "C" {
#endif

/*
Description:  pack command message.
*@inMsg:input message,just refer to command body.
*@outMsg:output message after packed,can be posted.need allocate memory by users
*@outLen:out message length
*@return: 0:ok,others:failed.
*/

int MP_pack_command_msg(MP_CommandMsg* inMsg,unsigned char* outMsg,int* outLen);

/*
Description:  pack command message.
*@inMsg:input message before unpacking.
*@msgLen:input message length
*@outMsg:output message after unpacked,can be used.need allocate memory by users
*         but some internal structure need allocate momory by api,but need free it by users.
*@type: command type,a enum value,see mp_types.h
*@return: 0:ok,others:failed.
*/
int MP_unpack_command_msg(unsigned char* inMsg,unsigned int msgLen,MP_CommandMsg* outMsg,MP_CommandType* type);

int MP_pack_response_msg(MP_ResponseMsg* inMsg,unsigned char* outMsg,int* outLen);

int MP_unpack_response_msg(unsigned char* inMsg,unsigned int msgLen,MP_ResponseMsg* outMsg,MP_ResponseType* type);

//some helper functions.
int MP_free_command_msg(MP_CommandMsg* inMsg);
int MP_free_response_msg(MP_ResponseMsg* inMsg);

int MP_free_taginfo(MP_AudioTagInfo* tag);
int  MP_get_timespec(int milli_sec , struct timespec * ts);



#ifdef __cplusplus
}
#endif


#endif // MP_UTILS_H
