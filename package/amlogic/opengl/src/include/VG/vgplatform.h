/*
* This confidential and proprietary software may be used only as
* authorised by a licensing agreement from ARM Limited
* (C) COPYRIGHT 2008-2010 ARM Limited
* ALL RIGHTS RESERVED
* The entire notice above must be reproduced on all authorised
* copies and copies may only be made to the extent permitted
* by a licensing agreement from ARM Limited.
*/

#ifndef _MALI_VGPLATFORM_H_
#define _MALI_VGPLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VG_API_CALL
#if defined(OPENVG_STATIC_LIBRARY)
#	define VG_API_CALL
#else
#	if defined(_WIN32) || defined(__VC32__)				/* Win32 */
#		if defined (OPENVG_DLL_EXPORTS)
#			define VG_API_CALL __declspec(dllexport)
#		else
#			define VG_API_CALL __declspec(dllimport)
#		endif
#	else
#		define VG_API_CALL extern
#	endif /* defined(_WIN32) ||... */
#endif /* defined OPENVG_STATIC_LIBRARY */
#endif /* ifndef VG_API_CALL */

#ifndef VGU_API_CALL
#if defined(OPENVG_STATIC_LIBRARY)
#	define VGU_API_CALL
#else
#	if defined(_WIN32) || defined(__VC32__)				/* Win32 */
#		if defined (OPENVG_DLL_EXPORTS)
#			define VGU_API_CALL __declspec(dllexport)
#		else
#			define VGU_API_CALL __declspec(dllimport)
#		endif
#	else
#		define VGU_API_CALL extern
#	endif /* defined(_WIN32) ||... */
#endif /* defined OPENVG_STATIC_LIBRARY */
#endif /* ifndef VGU_API_CALL */


#ifndef VG_API_ENTRY
#define VG_API_ENTRY
#endif

#ifndef VG_API_EXIT
#define VG_API_EXIT
#endif

#ifndef VGU_API_ENTRY
#define VGU_API_ENTRY
#endif

#ifndef VGU_API_EXIT
#define VGU_API_EXIT
#endif

typedef float          VGfloat;
typedef signed char    VGbyte;
typedef unsigned char  VGubyte;
typedef signed short   VGshort;
typedef signed int     VGint;
typedef unsigned int   VGuint;
typedef unsigned int   VGbitfield;

#ifndef VG_VGEXT_PROTOTYPES
#define VG_VGEXT_PROTOTYPES
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MALI_VGPLATFORM_H_ */
