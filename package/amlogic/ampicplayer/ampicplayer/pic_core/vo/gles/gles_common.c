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

void free_canvas(gles_canvas_t** my_canvas) {
	if(!my_canvas||!(*my_canvas)) return;
	gles_canvas_t* pmy_canvas=*my_canvas;
//	if(pmy_canvas->pTexData[0])	free(pmy_canvas->pTexData[0]);
//	if(pmy_canvas->pTexData[1])	free(pmy_canvas->pTexData[1]);
	free(pmy_canvas);
	*my_canvas=NULL;
}
void swap_canvas(gles_canvas_t* my_canvas) {
	if(my_canvas) {
		int index = my_canvas->iTexName[0];
		int id =  my_canvas->iTexID[0]; 
		my_canvas->iTexName[0] = my_canvas->iTexName[1] ;
		my_canvas->iTexName[1] = index ;
		my_canvas->iTexID[0] = my_canvas->iTexID[1]; 
		my_canvas->iTexID[1] = id; 		
		
		GL_CHECK(glDeleteTextures(1, &my_canvas->iTexName));  /*release current*/
		glGenTextures(1,my_canvas->iTexName);				  /*re-gen current*/
	}
}

void* local_init(void* arg) {
	gles_canvas_t* my_canvas=(gles_canvas_t*)malloc(sizeof(gles_canvas_t));
	if(!my_canvas) return NULL;
	
	/* Declare variables. */
	int bEnd = 0;

	/* Texture variables. */
	my_canvas->iTexName[0] = -1;
	my_canvas->pTexData[0] = NULL;
	my_canvas->iTexName[1] = -1;
	my_canvas->pTexData[1] = NULL;
	my_canvas->iTexID[0] = GL_TEXTURE0; 
	my_canvas->iTexID[1] = GL_TEXTURE1; 
	
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
	//GL_CHECK(glEnable(GL_CULL_FACE));
	//GL_CHECK(glCullFace(GL_BACK));
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
		free_canvas(&my_canvas);
		return NULL;
	}
	GL_CHECK(glEnableVertexAttribArray(my_canvas->iLocPosition));

	/* Get uniform locations */
    my_canvas->iLocMVP = GL_CHECK(glGetUniformLocation(my_canvas->iProgName, "mvp"));
	if(my_canvas->iLocMVP==-1) {
#ifdef DEBUG
		printf("iLocMVP      = %i\n", my_canvas->iLocMVP);
#endif
		free_canvas(&my_canvas);
		return NULL;
	}
	/* Texture coordinates. */
	my_canvas->iLocTexCoord = GL_CHECK(glGetAttribLocation(my_canvas->iProgName, "a_v2TexCoord"));
	if(my_canvas->iLocTexCoord == -1)
	{
#		ifdef DEBUG
		fprintf(stderr, "Error: Attribute not found at line %i\n", __LINE__);
#		endif
		free_canvas(&my_canvas);
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
		free_canvas(&my_canvas);
		return NULL;
	}
	GL_CHECK(glUniform1i(my_canvas->iLocSampler, 0));

	/* Set clear screen color. */
	GL_CHECK(glClearColor(1.0f, 1.0f, 0.0f, 1.0));
	glGenTextures(2,my_canvas->iTexName);
//	glEnable(GL_TEXTURE_2D);
#if 0	
	/* init texture. */	
	my_canvas->pTexData[0] = (unsigned char *)calloc(MAX_BUFFER_WIDTH* MAX_BUFFER_HIGHT* 4, sizeof(unsigned char));
	my_canvas->pTexData[1] = (unsigned char *)calloc(MAX_BUFFER_WIDTH* MAX_BUFFER_HIGHT* 4, sizeof(unsigned char));

	if(!my_canvas->pTexData[0]||!my_canvas->pTexData[1]) {
		free_canvas(&my_canvas);
	}
#endif	

	/*check invalid uniforms*/
	GLint maxUniformLen;
	GLint numUniforms;
	char *uniformName;
	GLint index;
	glGetProgramiv(my_canvas->iProgName, GL_ACTIVE_UNIFORMS, &numUniforms);
	glGetProgramiv(my_canvas->iProgName, GL_ACTIVE_UNIFORM_MAX_LENGTH,
	&maxUniformLen);
	uniformName = malloc(sizeof(char) * maxUniformLen);		
	
	for(index = 0; index < numUniforms; index++){
		GLint size;
		GLenum type;
		GLint location;
		glGetActiveUniform(my_canvas->iProgName, index, maxUniformLen, NULL,
		&size, &type, uniformName);
		// Get the uniform location
		location = glGetUniformLocation(my_canvas->iProgName, uniformName);		
	}
	
	GLint maxVertexAttribs; // n will be >= 8
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	printf("maxVertexAttribs is %d \n" ,maxVertexAttribs);
    return (void*)my_canvas;
}

int local_release(void* arg) {
	gles_canvas_t* my_canvas=(gles_canvas_t*)arg;
	
	/* Shut down OpenGL-ES. */
	GL_CHECK(glDeleteTextures(2, &my_canvas->iTexName));
	glGenTextures(2,my_canvas->iTexName);
	GL_CHECK(glUseProgram(0));
    GL_CHECK(glDeleteShader(my_canvas->iVertName));
    GL_CHECK(glDeleteShader(my_canvas->iFragName));
    GL_CHECK(glDeleteProgram(my_canvas->iProgName));
	
	terminateEGL();
	destroyWindow();
	terminateStatics();
	free_canvas(&my_canvas);
    return 0;
}

GLfloat aVertices1[] =
{
	/* 256 x 128 */
	-1.000000f,  1.0000f, 0.0f, /* 0 */
	-1.000000f,  -1.0000f, 0.0f, /* 1 */
	1.00000f,  1.00000f, 0.000f, /* 2 */
	1.00000f,  -1.00000f, 0.000f, /* 3 */
};

GLfloat aVertices2[] =
{
	/* 256 x 128 */
	-1.000000f,  1.0000f, 0.0f, /* 0 */
	-1.000000f,  -1.0000f, 0.0f, /* 1 */
	-1.000000f,  1.0000f, 0.0f, /* 0 */
	-1.000000f,  -1.0000f, 0.0f, /* 1 */
};
GLfloat aVertices3[] =
{
	/* 256 x 128 */
	0.000000f,  0.0000f, 0.0f, /* 0 */
	0.000000f,  -1.0000f, 0.0f, /* 1 */
	1.000000f,  0.00000f, 0.000f, /* 2 */
	1.000000f,  -1.00000f, 0.000f, /* 3 */
};

void change_vertice(GLfloat* ver , int n, float step, int flag)
{
	if(!flag){
		ver[0] += step;
		ver[1*n] += step;
	}else{
		ver[2*n] += step;
		ver[3*n] += step;	
	}
}

int local_display(const char *arg,unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs) {
	Statics *pStatics = NULL;
	pStatics = getStatics();
	int bEnd=0;
	float xxx=1.0;
	float aRotate[16], aModelView[16], aPerspective[16], aMVP[16];
	gles_canvas_t* my_canvas=(gles_canvas_t*)arg;
	
	/* texture setting. */
	swap_canvas(my_canvas);
		
	copy2Texture(my_canvas->pTexData[0],x_size,y_size,rgbbuff,x_size,y_size,x_pan,y_pan,x_offs,y_offs);
    my_canvas->pTexData[0] =  rgbbuff;
	/* locations. */
	GL_CHECK(glEnableVertexAttribArray(my_canvas->iLocPosition));
	
	
	

	glActiveTexture (GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, my_canvas->iTexName[0]);
//	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,MAX_BUFFER_WIDTH,MAX_BUFFER_HIGHT,0,GL_RGBA,GL_UNSIGNED_BYTE,my_canvas->pTexData[0]);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size,y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,my_canvas->pTexData[0]);

	memset(my_canvas->pTexData[0],0 ,x_size*y_size*4);
	/* Set clear screen color. */
	GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0));

	my_canvas->iXangle = 0;
	my_canvas->iYangle = 0;
	my_canvas->iZangle = 0;
	/* Test drawing. */
	GLfloat temp_ver_new[12] , temp_ver_old[12];
	int total_step = 180;
	int step_inc = 0 ;
	int i;
	for( i = 0 ; i < 12 ; i++){
		temp_ver_old[i] = aVertices1[i];
	}	
	
	for( i = 0 ; i < 12 ; i++){
		temp_ver_new[i]= aVertices2[i];
	}		
	while(step_inc < total_step)
	{
#ifdef WIN32
		//bEnd = checkWindow();
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
        perspective_matrix(45.0, (double)WINDOW_W/(double)WINDOW_H, 0.01, 100.0, aPerspective);
		aModelView[14] = -1.0 ;/* Pull the camera back from the cube */
		multiply_matrix(aPerspective, aModelView, aMVP);
		
		GL_CHECK(glUniformMatrix4fv(my_canvas->iLocMVP, 1, GL_FALSE, aMVP));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));		
		

		/* draw new picture. */
#if 0
		glActiveTexture ( GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, my_canvas->iTexName[0]);	
	    GL_CHECK(glVertexAttribPointer(my_canvas->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, aVertices1));
		GL_CHECK(glVertexAttribPointer(my_canvas->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aTexCoords));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
		GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, sizeof(aIndices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, aIndices));
	
#else
		change_vertice(temp_ver_old,3,(float)2/total_step , 0);
		change_vertice(temp_ver_new,3,(float)2/total_step , 1);

		glActiveTexture ( GL_TEXTURE0 );
	//	GL_CHECK(glEnable(GL_TEXTURE_2D));
		glBindTexture(GL_TEXTURE_2D, my_canvas->iTexName[0]);	
		GL_CHECK(glUniform1i(my_canvas->iLocSampler, 0));
	    GL_CHECK(glVertexAttribPointer(my_canvas->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, temp_ver_new));
		GL_CHECK(glVertexAttribPointer(my_canvas->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aTexCoords));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
		GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, sizeof(aIndices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, aIndices));
//		eglSwapBuffers(pStatics->sEGLInfo.sEGLDisplay, pStatics->sEGLInfo.sEGLSurface);		
//		Sleep(1000);
		
		
	
		
			
		glActiveTexture ( GL_TEXTURE1 );
		glBindTexture(GL_TEXTURE_2D, my_canvas->iTexName[1]);
		GL_CHECK(glUniform1i(my_canvas->iLocSampler, 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		GL_CHECK(glVertexAttribPointer(my_canvas->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, temp_ver_old));
		GL_CHECK(glVertexAttribPointer(my_canvas->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aTexCoords));
		GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, sizeof(aIndices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, aIndices));
		
#endif
	
		/* display the backup buffer. */
		eglSwapBuffers(pStatics->sEGLInfo.sEGLDisplay, pStatics->sEGLInfo.sEGLSurface);
//		Sleep(50);
		step_inc++;
//		my_canvas->iXangle += 360/total_step;
//        my_canvas->iYangle += 360/total_step;
//        my_canvas->iZangle += 360/total_step;

		/* change the parameter. */
        if(my_canvas->iXangle >= 360) my_canvas->iXangle -= 360;
        if(my_canvas->iXangle < 0) my_canvas->iXangle += 360;
        if(my_canvas->iYangle >= 360) my_canvas->iYangle -= 360;
        if(my_canvas->iYangle < 0) my_canvas->iYangle += 360;
        if(my_canvas->iZangle >= 360) my_canvas->iZangle -= 360;
        if(my_canvas->iZangle < 0) my_canvas->iZangle += 360;
	}
}

int local_getCurrentRes(void* arg,int* width,int* height) {
	*width=WINDOW_W;
	*height=WINDOW_H;
	return 0;
}
