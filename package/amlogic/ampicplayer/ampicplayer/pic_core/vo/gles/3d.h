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

#ifndef __3d_h
#define __3d_h

/* These indices describe triangle strips, separated by degenerate triangles where necessary. */
static const GLubyte aIndices[] =
{0,  1,  2,  3};
/* Tri strips, so quads are in this order:
 * 
 * 0 ----- 24 - 68-10 etc.
 * |     / || / |9-11
 * |   /   |5 - 7
 * | /     |
 * 1 ----- 3
 */
static const GLfloat aVertices[] =
{
	/* 256 x 128 */
	-1.000000f,  1.0000f, 0.0f, /* 0 */
	-1.000000f,  -1.0000f, 0.0f, /* 1 */
	1.000000f,  1.00000f, 0.000f, /* 2 */
	1.000000f,  -1.00000f, 0.000f, /* 3 */
};

/* Because textures are loaded flipped, (0, 0) refers to top left corner. */
/* The texture orientation is the same for each quad. */
static const GLfloat aTexCoords[] =
{
	0.0f, 0.0f, /* 0 */
	0.0f, 1.0f, /* 1 */
	1.0f, 0.0f, /* 2 */
	1.0f, 1.0f, /* 3 */
};

#endif /* __3d_h */

