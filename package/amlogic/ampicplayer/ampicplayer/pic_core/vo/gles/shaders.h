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

#ifndef __shaders_h
#define __shaders_h

#include <GLES2/gl2.h>

void processShader(GLuint *pShader, char *sFilename, GLint iShaderType);
void processBIShader(GLuint *pShader, char *shader, GLint iShaderType);
char *loadShader(char *sFilename);

#endif /* __shaders_h */
