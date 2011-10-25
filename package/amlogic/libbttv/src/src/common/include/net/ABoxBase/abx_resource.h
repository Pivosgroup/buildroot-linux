/*******************************************************************

 * 

 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.

 *

 *  Description: the interface of MSP parser

 *

 *  Author: Sympeer Software

 *

 *******************************************************************/

#ifndef		ABX_RESOURCE_H
#define		ABX_RESOURCE_H

#include "abx_common.h"

#ifndef WIN32
    #include "AW_Resource.h"
#endif

typedef int ABX_HRESFILE;
#define ABX_INVALID_HRESFILE	(-1)

#define	ABX_ResInit	    AWResourceInit
#define	ABX_ResReg		AWResourceRegister
#define	ABX_ResUnreg	AWResourceUnregister
#define	ABX_ResGet		AWGetResource

typedef struct {
	char* name;
	INT32U resid;
	char * resfile;
	const unsigned char * res_mem;
} abx_res_item_t;

#ifndef WIN32
    ABX_HRESFILE ABX_ResOpenAsFile(const char* fname);
    //insure the return datas used before ABX_ResCloseAsFile() called
    void *ABX_ResGetAsFile(ABX_HRESFILE hf, INT16U* pSourceLength) ;
    void ABX_ResCloseAsFile(ABX_HRESFILE hf);
    void ABX_SetResource_info(u32 resource_head_addr);
#endif

#endif //ABX_RESOURCE_H

