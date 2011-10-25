/*******************************************************************
 * 
 *  Copyright C 2004 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: Endian definitions
 *
 * Created: Mon Oct 18 14:05:44 2004, Eric Knudstrup
 *
 *******************************************************************/

#ifndef ENDIAN_H
#define ENDIAN_H


#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#else
#error There can only be one definition of LITTLE_ENDIAN
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#else
#error There can only be one definition of BIG_ENDIAN
#endif

/* Define this properly for the processor of the day */
#ifdef _ARC
    #ifdef _ARC_BE
        #define BYTE_ORDER BIG_ENDIAN
    #elif defined (_ARC_LE)
        #define BYTE_ORDER LITTLE_ENDIAN
    #endif
#endif

#define __BYTE_ORDER    BYTE_ORDER
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __BIG_ENDIAN    BIG_ENDIAN

#endif
