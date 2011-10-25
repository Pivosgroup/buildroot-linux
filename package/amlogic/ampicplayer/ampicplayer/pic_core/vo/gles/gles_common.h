/* vim:set sts=4 ts=4 noexpandtab: */
/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef gles_common_h
#define gles_common_h

#define WINDOW_W	1280
#define WINDOW_H	720

#ifndef M_PI
#define M_PI 3.14159f
#endif /* M_PI */

#define VERTEX_SHADER_FILE		"res/texture.vert"
#define FRAGMENT_SHADER_FILE	"res/texture.frag"
#include <GLES2/gl2.h>
#include "matrix.h"

#define MAX_BUFFER_WIDTH 1920
#define MAX_BUFFER_HIGHT 720

typedef struct s_gles_canvas_t {
	/* for texture. 0 is for front, 1 is for backup. */
	GLuint iTexName[2];
	GLuint iTexID[2];
	unsigned char *pTexData[2];

	/* Shader variables. */
	GLuint iVertName;
	GLuint iFragName;
	GLuint iProgName;
	GLint iLocPosition;
	GLint iLocFillColor;
	GLint iLocProjection;
	GLint iLocModelview;	
	GLint iLocTexCoord;
	GLint iLocSampler;
	GLint iLocMVP;
	
	/* transport. variables. */
	int iXangle;
	int iYangle;
	int iZangle;
	
} gles_canvas_t;

#endif  /* __main_h */

