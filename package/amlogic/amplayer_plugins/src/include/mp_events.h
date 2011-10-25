#ifndef _MP_EVENTS_H
#define _MP_EVENTS_H


#define MP_ErrnoBase  0x2010
/**
 * An enumeration of error types from AmPlayer
 **/
typedef enum
{
    /**
     AmPlayer sends this error event when the user
     provides an invalid data source for multimedia playback
    **/
    MP_ErrSourceInvalid = MP_ErrnoBase,

    /**
     AmPlayer sends this error event when an error occurs
     during initialization of data source
    **/
    MP_ErrSourceInit,

    /**
     AmPlayer sends this error event for general non-fatal error
     from the data source
    **/
    MP_ErrSource,

    /**
     AmPlayer sends this error event for general fatal error from
     the data source
    **/
    MP_ErrSourceFatal,

     /**
     AmPlayer sends this error event for general non-fatal error from
     decoding
     **/

    MP_ErrMatchValidDecoder,
    MP_ErrDecoderReset,
    MP_ErrDecoderInit,

    /**
     AmPlayer sends this error event when the data source contains
     no media track for playback
    **/
    MP_ErrSourceNoMediaTrack,    

    /**
     AmPlayer sends this error event when no supported media track
     for playback was found
    **/
    MP_ErrNoSupportedTrack,   
    
    MP_ErrUnsupportedAudioCodec,
    MP_ErrUnsupportedVideoCodec,

    /**
     AmPlayer sends this error event when the data source runs out of
     media data
    **/
    MP_ErrSourceMediaDataUnavailable,

    /**
    AMPlayer sends this error event when seek time overspill.
    **/
    MP_ErrSeekOverPlayDuration,

    /**
     AmPlayer sends this error event when the data source encounters a
     general error in the media data
    **/
    MP_PlayerErrSourceMediaData,    

    /**
     AmPlayer sends this error event when an error occurs during shutdown
     of data source
    **/
    MP_ErrSourceShutdown,  
    
    MP_ErrUnknow,
    MP_ErrFatal, 
    /**
     Placeholder for the last AmPlayer error event
     **/
    MP_ErrLast,
} MP_PlayerErrorEventType;



#endif // _MP_EVENTS_H


