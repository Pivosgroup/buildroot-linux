/***************************************************
 * name	    : player_error.c
 * function : player error message
 * date		: 2010.3.23
 ***************************************************/

#include <player_error.h>

int system_error_to_player_error(int error)
{
	return error;
}
char * player_error_msg(int error)
{
	switch(error)
	{        
    	case PLAYER_SUCCESS:		return "player no errors";
    	case PLAYER_RD_FAILED:		return "error:player read file error";
    	case PLAYER_RD_EMPTYP:	 	return "error:invalid pointer when reading";
        case PLAYER_RD_TIMEOUT:	 	return "error:no data for reading,time out";
    	case PLAYER_WR_FAILED:		return "error:player write data error";
    	case PLAYER_WR_EMPTYP:		return "error:invalid pointer when writing";
    	case PLAYER_WR_FINISH:		return "error:player write finish";
    	case PLAYER_PTS_ERROR:		return "error:player pts error";
    	case PLAYER_NO_DECODER:		return "error:can't find valid decoder";
    	case DECODER_RESET_FAILED:	return "error:decoder reset failed";
        case DECODER_INIT_FAILED:   return "error:decoder init failed";
        case PLAYER_UNSUPPORT:      return "error:player unsupport file type";   	
    	case FFMPEG_OPEN_FAILED:	return "error:can't open input file";
        case FFMPEG_PARSE_FAILED:	return "error:parse file failed";
        case FFMPEG_EMP_POINTER:    return "error:check invalid pointer";
        case FFMPEG_NO_FILE:        return "error:not assigned a file to play";
        case PLAYER_SEEK_OVERSPILL: return "error:seek time point overspill";
        case PLAYER_CHECK_CODEC_ERROR: return "error:check codec status error";
        case PLAYER_INVALID_CMD:    return "error:invalid command under current status";
    	default:
    			return "error:invalid operate";
	}
}

