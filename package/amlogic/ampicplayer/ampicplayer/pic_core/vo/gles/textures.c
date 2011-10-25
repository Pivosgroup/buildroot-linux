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
#endif /* WIN32 */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config.h>

#include "textures.h"
#include "gl_runtime.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif /* DMALLOC */

/* Report which compressed texture formats are supported. */
int reportTextureFormats()
{
	int bSupportETC = 0;
	GLint *pTexFormat = NULL;
	GLint cTexFormats = 0;
	int iTexFormat = 0;

	GL_CHECK(glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &cTexFormats));

	pTexFormat = (GLint *)calloc(cTexFormats, sizeof(GLint));
	if(pTexFormat == NULL)
	{
		fprintf(stderr, "Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	GL_CHECK(glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, pTexFormat));
	fprintf(stdout, "Compressed texture formats supported:\n");
	for(iTexFormat = 0; iTexFormat < cTexFormats; iTexFormat ++)
	{
		fprintf(stdout, "0x%.8x\t", pTexFormat[iTexFormat]);
		switch(pTexFormat[iTexFormat])
		{
			case GL_ETC1_RGB8_OES:
				fprintf(stdout, "(%s)\n", "GL_ETC1_RGB8_OES");
				bSupportETC = 1;
				break;
			default:
				fprintf(stdout, "(%s)\n", "UNKNOWN");
				break;
		}
	}

	free(pTexFormat);
	pTexFormat = NULL;

	if(!bSupportETC)
	{
		fprintf(stderr, "Error: Texture compression format GL_ETC1_RGB8_OES not supported\n");
		exit(1);
	}

	return 0;
}



/* Create a texture from random data. */
int createTexture(unsigned char **ppTexData, int iWidth, int iHeight,unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs) {
	int cy = 0,cx=0;
	int depth=4;
	char *p_src,*p_des;
	//printf("mallocing memroy%d X %d \n",iWidth,iHeight);
	unsigned char *pTexData = (unsigned char *)calloc(iWidth * iHeight * depth, sizeof(unsigned char));

	if(pTexData == NULL)
	{
		fprintf(stderr, "Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Initialize texture with random shades. */
	int mx=iWidth>x_size?x_size:iWidth;
	int my=iHeight>y_size?y_size:iHeight;
	for(cy=0;cy<my;cy++) {
		p_src=rgbbuff+cy*3*x_size;
		p_des=pTexData+cy*depth*iWidth;
		for(cx=0;cx<mx;cx++) {
			p_des[4*cx+0]=p_src[3*cx+0];
			p_des[4*cx+1]=p_src[3*cx+1];
			p_des[4*cx+2]=p_src[3*cx+2];
			p_des[4*cx+3]=0xff;
		}
		//memcpy(p_des,p_src,depth*mx);
	}

	*ppTexData = pTexData;

	return 0;
}

/* Create a texture from random data. */
int copy2Texture(unsigned char *pTexData, int iWidth, int iHeight,unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs) {
	int cy = 0,cx=0;
	int depth=4;
	char *p_src,*p_des;
	unsigned char temp;
	/* Initialize texture with random shades. */
	int mx=iWidth>x_size?x_size:iWidth;
	int my=iHeight>y_size?y_size:iHeight;
	for(cy=0;cy<my;cy++) {
		p_src=rgbbuff+cy*4*x_size;
		p_des= p_src;
		for(cx=0;cx<mx;cx++) {
			temp = p_des[4*cx+0];
			p_des[4*cx+0] = p_des[4*cx+2] ;
			p_des[4*cx+2] = temp;
		}
		//memcpy(p_des,p_src,depth*mx);
	}

	return 0;
}