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

#ifndef __textures_h
#define __textures_h

typedef struct {
	char	aName[6];
	unsigned short iBlank;
	/* NB: Beware endianess issues here. */
	unsigned char iPaddedWidthMSB;
	unsigned char iPaddedWidthLSB;
	unsigned char iPaddedHeightMSB;
	unsigned char iPaddedHeightLSB;
	unsigned char iWidthMSB;
	unsigned char iWidthLSB;
	unsigned char iHeightMSB;
	unsigned char iHeightLSB;
} ETCHeader;

unsigned short getWidth(ETCHeader *pHeader);
unsigned short getHeight(ETCHeader *pHeader);
unsigned short getPaddedWidth(ETCHeader *pHeader);
unsigned short getPaddedHeight(ETCHeader *pHeader);

int reportTextureFormats(void);
int createTexture(unsigned char **ppTexData, int iWidth, int iHeight,unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);
int copy2Texture(unsigned char *pTexData, int iWidth, int iHeight,unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);


#endif /* __textures_h */

