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

#ifndef __matrix_h
#define __matrix_h
#include <math.h>
typedef struct
{
	/* NB: Items are stored in column major order as GL expects them. */
    float aElem[16];
} Matrix;

extern const Matrix g_sMatIdentity;

Matrix matrixIdentity(void);
void matrixTranspose(Matrix *pM);
Matrix matrixMultiply(Matrix *pL, Matrix *pR);
Matrix matrixRotateX(int iAngle);
Matrix matrixRotateY(int iAngle);
Matrix matrixRotateZ(int iAngle);
Matrix matrixTranslate(float fX, float fY, float fZ);
void matrixPrint(Matrix *pMatrix);
Matrix matrixPerspective(float fFOV, float fRatio, float fNear, float fFar);
Matrix matrixOrthographic(float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar);


void rotate_matrix(double angle, double x, double y, double z, float *R);
void perspective_matrix(double fovy, double aspect, double znear, double zfar, float *P);
void multiply_matrix(float *A, float *B, float *C);


#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif /* M_PI */

#endif
