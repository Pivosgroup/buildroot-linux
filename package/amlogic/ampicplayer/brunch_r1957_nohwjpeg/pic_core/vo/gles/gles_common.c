/* vim: set sts=4 ts=4 noexpandtab: */
/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifdef WIN32
#include "win32_runtime.h"
#else /* WIN32 */
#include "linux_runtime.h"
#endif /* WIN32 */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <config.h>
#include <math.h>

#include "gles_common.h"
#include "egl_runtime.h"
#include "gl_runtime.h"
#include "statics.h"
#include "shaders.h"
#include "textures.h"
#include "3d.h"

static char* texture_shader_vert= " \
attribute vec4 a_v4Position;\n \
uniform mat4 mvp; \
attribute vec2 a_v2TexCoord;\n \
varying vec2 v_v2TexCoord;\n \
void main()\n \
{\n \
	v_v2TexCoord = a_v2TexCoord;\n \
	gl_Position = mvp*a_v4Position;\n \
} \
";

static char* texture_shader_frag= " \
precision mediump float;\n \
uniform sampler2D u_s2dTexture;\n \
varying vec2 v_v2TexCoord;\n \
void main()\n  \
{\n \
	gl_FragColor = texture2D(u_s2dTexture, v_v2TexCoord);\n \
} \
";

void* local_init(void* arg) {
	gles_canvas_t* my_canvas=(gles_canvas_t*)malloc(sizeof(gles_canvas_t));
	if(!my_canvas) return NULL;
	
	/* Declare variables. */
	int bEnd = 0;

	/* Texture variables. */
	my_canvas->iTexName = 0;
	my_canvas->pTexData = NULL;

	/* Shader variables. */
	my_canvas->iVertName = 0;
	my_canvas->iFragName = 0;
	my_canvas->iLocPosition = -1;
	my_canvas->iLocTexCoord = -1;
	my_canvas->iLocSampler = -1;

	/* Initialize the single global data pointer. */
	/* The pStatics pointer will point to a structure containing all global data. */
	initializeStatics();

	/* Initialize windowing system. */
	createWindow(WINDOW_W, WINDOW_H);

	/* Initialize EGL. */
	initializeEGL();

	/* Initialize OpenGL-ES. */
	/* Check which formats are supported. */
	//reportTextureFormats();

	/* Initialize OpenGL-ES. */
	GL_CHECK(glEnable(GL_CULL_FACE));
	GL_CHECK(glCullFace(GL_BACK));
	GL_CHECK(glEnable(GL_DEPTH_TEST));
	GL_CHECK(glEnable(GL_BLEND));	
	
	/* Should do src * (src alpha) + dest * (1-src alpha). */
	GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	
	/* Load shaders. */
	processBIShader(&my_canvas->iVertName, texture_shader_vert, GL_VERTEX_SHADER);
	processBIShader(&my_canvas->iFragName, texture_shader_frag, GL_FRAGMENT_SHADER);

	/* Set up shaders. */
	my_canvas->iProgName = GL_CHECK(glCreateProgram());
	GL_CHECK(glAttachShader(my_canvas->iProgName, my_canvas->iVertName));
	GL_CHECK(glAttachShader(my_canvas->iProgName, my_canvas->iFragName));
	GL_CHECK(glLinkProgram(my_canvas->iProgName));
	GL_CHECK(glUseProgram(my_canvas->iProgName));

	/* Vertex positions. */
	my_canvas->iLocPosition = GL_CHECK(glGetAttribLocation(my_canvas->iProgName, "a_v4Position"));
	if(my_canvas->iLocPosition == -1)
	{
#		ifdef DEBUG
		fprintf(stderr, "Error: Attribute not found at line %i\n", __LINE__);
#		endif
		free(my_canvas);
		return NULL;
	}
	GL_CHECK(glEnableVertexAttribArray(my_canvas->iLocPosition));

	/* Get uniform locations */
    my_canvas->iLocMVP = GL_CHECK(glGetUniformLocation(my_canvas->iProgName, "mvp"));
	if(my_canvas->iLocMVP==-1) {
#ifdef DEBUG
		printf("iLocMVP      = %i\n", my_canvas->iLocMVP);
#endif
		free(my_canvas);
		return NULL;
	}
	/* Texture coordinates. */
	my_canvas->iLocTexCoord = GL_CHECK(glGetAttribLocation(my_canvas->iProgName, "a_v2TexCoord"));
	if(my_canvas->iLocTexCoord == -1)
	{
#		ifdef DEBUG
		fprintf(stderr, "Error: Attribute not found at line %i\n", __LINE__);
#		endif
		free(my_canvas);
		return NULL;
	}
	GL_CHECK(glEnableVertexAttribArray(my_canvas->iLocTexCoord));

	/* Set the sampler to point at the 0th texture unit. */
	my_canvas->iLocSampler = GL_CHECK(glGetUniformLocation(my_canvas->iProgName, "u_s2dTexture"));
	if(my_canvas->iLocSampler == -1)
	{
#		ifdef DEBUG
		fprintf(stderr, "Error: Uniform not found at line %i\n", __LINE__);
#		endif
		free(my_canvas);
		return NULL;
	}
	GL_CHECK(glUniform1i(my_canvas->iLocSampler, 0));

	/* Set clear screen color. */
	GL_CHECK(glClearColor(1.0f, 1.0f, 0.0f, 1.0));
    return (void*)my_canvas;
}

int local_release(void* arg) {
	gles_canvas_t* my_canvas=(gles_canvas_t*)arg;
	
	/* Shut down OpenGL-ES. */
	free(my_canvas->pTexData);
	my_canvas->pTexData = NULL;
	GL_CHECK(glDeleteTextures(1, &my_canvas->iTexName));

	GL_CHECK(glUseProgram(0));
    GL_CHECK(glDeleteShader(my_canvas->iVertName));
    GL_CHECK(glDeleteShader(my_canvas->iFragName));
    GL_CHECK(glDeleteProgram(my_canvas->iProgName));
	
	terminateEGL();
	destroyWindow();
	terminateStatics();
	
	free(my_canvas);
    return 0;
}

int local_display(const char *arg,unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs) {
	Statics *pStatics = NULL;
	pStatics = getStatics();
	int bEnd=0;
	float aRotate[16], aModelView[16], aPerspective[16], aMVP[16];
	gles_canvas_t* my_canvas=(gles_canvas_t*)arg;
	
	createTexture(&my_canvas->pTexData,2048,1024,rgbbuff,x_size,y_size,x_pan,y_pan,x_offs,y_offs);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,2048,1024,0,GL_RGBA,GL_UNSIGNED_BYTE,my_canvas->pTexData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GL_CHECK(glEnableVertexAttribArray(my_canvas->iLocPosition));
	GL_CHECK(glUniform1i(my_canvas->iLocSampler, 0));

	/* Set clear screen color. */
	GL_CHECK(glClearColor(1.0f, 1.0f, 0.0f, 1.0));

	/* Test drawing. */
	while(!bEnd)
	{
#ifdef WIN32
		bEnd = checkWindow();
#endif /* WIN32 */

		/* 
		 * Do some rotation with Euler angles. It is not a fixed axis as
         * quaterions would be, but the effect is cool. 
		 */
        rotate_matrix(my_canvas->iXangle, 1.0, 0.0, 0.0, aModelView);
        rotate_matrix(my_canvas->iYangle, 0.0, 1.0, 0.0, aRotate);

        multiply_matrix(aRotate, aModelView, aModelView);

        rotate_matrix(my_canvas->iZangle, 0.0, 1.0, 0.0, aRotate);

        multiply_matrix(aRotate, aModelView, aModelView);

		/* Pull the camera back from the cube */
        aModelView[14] -= 2.5;

        perspective_matrix(45.0, (double)WINDOW_W/(double)WINDOW_H, 0.01, 100.0, aPerspective);
        multiply_matrix(aPerspective, aModelView, aMVP);

        GL_CHECK(glUniformMatrix4fv(my_canvas->iLocMVP, 1, GL_FALSE, aMVP));

		my_canvas->iXangle += 3;
        my_canvas->iYangle += 2;
        my_canvas->iZangle += 1;

        if(my_canvas->iXangle >= 360) my_canvas->iXangle -= 360;
        if(my_canvas->iXangle < 0) my_canvas->iXangle += 360;
        if(my_canvas->iYangle >= 360) my_canvas->iYangle -= 360;
        if(my_canvas->iYangle < 0) my_canvas->iYangle += 360;
        if(my_canvas->iZangle >= 360) my_canvas->iZangle -= 360;
        if(my_canvas->iZangle < 0) my_canvas->iZangle += 360;

        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

		
		GL_CHECK(glVertexAttribPointer(my_canvas->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, aVertices));
		GL_CHECK(glVertexAttribPointer(my_canvas->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aTexCoords));
		GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, sizeof(aIndices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, aIndices));

		eglSwapBuffers(pStatics->sEGLInfo.sEGLDisplay, pStatics->sEGLInfo.sEGLSurface);
	}
	
}

int local_getCurrentRes(void* arg,int* width,int* height) {
	*width=WINDOW_W;
	*height=WINDOW_H;
	return 0;
}
