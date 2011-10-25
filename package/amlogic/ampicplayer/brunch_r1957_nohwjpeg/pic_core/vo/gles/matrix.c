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

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <config.h>

#include "gles_common.h"
#include "matrix.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define DEG2RAD(a) (M_PI * a / 180.0f)

#ifdef DMALLOC
#include "dmalloc.h"
#endif /* DMALLOC */

/* Identity matrix. */
static const float m_aIdentity[16] =
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

const Matrix g_sMatIdentity = { { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f } };

Matrix matrixIdentity(void)
{
	Matrix sResult;

	memcpy(sResult.aElem, m_aIdentity, 16 * sizeof(float));

    return sResult;
}

void matrixTranspose(Matrix *pM)
{
	float fTemp;

	fTemp = pM->aElem[1];
	pM->aElem[1] = pM->aElem[4];
	pM->aElem[4] = fTemp;

	fTemp = pM->aElem[2];
	pM->aElem[2] = pM->aElem[8];
	pM->aElem[8] = fTemp;

	fTemp = pM->aElem[3];
	pM->aElem[3] = pM->aElem[12];
	pM->aElem[12] = fTemp;

	fTemp = pM->aElem[6];
	pM->aElem[6] = pM->aElem[9];
	pM->aElem[9] = fTemp;

	fTemp = pM->aElem[7];
	pM->aElem[7] = pM->aElem[13];
	pM->aElem[13] = fTemp;

	fTemp = pM->aElem[11];
	pM->aElem[11] = pM->aElem[14];
	pM->aElem[14] = fTemp;
}

Matrix matrixTranslate(float fX, float fY, float fZ)
{
	Matrix sResult = matrixIdentity();
	
	sResult.aElem[12] = fX;
	sResult.aElem[13] = fY;
	sResult.aElem[14] = fZ;

	return sResult;
}

Matrix matrixPerspective(float fFOV, float fRatio, float fNear, float fFar)
{
	Matrix sResult = matrixIdentity();

	sResult.aElem[ 0] = fFOV / fRatio;
	sResult.aElem[ 5] = fFOV;
	sResult.aElem[10] = -(fFar + fNear) / (fFar - fNear);
	sResult.aElem[14] = (-2.0f * fFar * fNear) / (fFar - fNear);
	sResult.aElem[11] = -1.0f;
	sResult.aElem[15] = 0.0f;

    return sResult;
}

Matrix matrixOrthographic(float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar)
{
	Matrix sResult = matrixIdentity();

	sResult.aElem[ 0] = 2.0f / (fRight - fLeft);
	sResult.aElem[12] = -(fRight + fLeft) / (fRight - fLeft);

	sResult.aElem[ 5] = 2.0f / (fTop - fBottom);
	sResult.aElem[13] = -(fTop + fBottom) / (fTop - fBottom);

	sResult.aElem[10] = 2.0f / (fFar - fNear);
	sResult.aElem[14] = -(fFar + fNear) / (fFar - fNear);

	return sResult;
}

Matrix matrixRotateX(int iAngle)
{
	Matrix sResult = matrixIdentity();

	sResult.aElem[ 5] = cos(DEG2RAD(iAngle));
	sResult.aElem[ 9] = -sin(DEG2RAD(iAngle));
	sResult.aElem[ 6] = sin(DEG2RAD(iAngle));
	sResult.aElem[10] = cos(DEG2RAD(iAngle));

	return sResult;
}

Matrix matrixRotateY(int iAngle)
{
	Matrix sResult = matrixIdentity();

	sResult.aElem[ 0] = cos(DEG2RAD(iAngle));
	sResult.aElem[ 8] = sin(DEG2RAD(iAngle));
	sResult.aElem[ 2] = -sin(DEG2RAD(iAngle));
	sResult.aElem[10] = cos(DEG2RAD(iAngle));

	return sResult;
}

Matrix matrixRotateZ(int iAngle)
{
	Matrix sResult = matrixIdentity();

	sResult.aElem[0] = cos(DEG2RAD(iAngle));
	sResult.aElem[4] = -sin(DEG2RAD(iAngle));
	sResult.aElem[1] = sin(DEG2RAD(iAngle));
	sResult.aElem[5] = cos(DEG2RAD(iAngle));

	return sResult;
}

Matrix matrixMultiply(Matrix *pL, Matrix *pR)
{
	Matrix sResult;
	int iR = 0;
	int iC = 0;

	for(iR = 0; iR < 4; iR ++)
	{
		for(iC = 0; iC < 4; iC ++)
		{
			sResult.aElem[iR * 4 + iC]  = pL->aElem[0 + iR * 4] * pR->aElem[iC + 0 * 4];
			sResult.aElem[iR * 4 + iC] += pL->aElem[1 + iR * 4] * pR->aElem[iC + 1 * 4];
			sResult.aElem[iR * 4 + iC] += pL->aElem[2 + iR * 4] * pR->aElem[iC + 2 * 4];
			sResult.aElem[iR * 4 + iC] += pL->aElem[3 + iR * 4] * pR->aElem[iC + 3 * 4];
		}
	}

	return sResult;
}

void matrixPrint(Matrix *pMatrix)
{
	int iR, iC;

	printf("\n");
	for(iR = 0; iR < 4; iR ++)
	{
		for(iC = 0; iC < 4; iC ++)
		{
			printf("%.1f\t", pMatrix->aElem[iC * 4 + iR]);
		}
		printf("\n");
	}
	printf("\n");
}

/* 
 * Simulates desktop's glRotatef. The matrix is returned in column-major 
 * order. 
 */
void rotate_matrix(double angle, double x, double y, double z, float *R) {
    double radians, c, s, c1, u[3], length;
    int i, j;

    radians = (angle * M_PI) / 180.0;

    c = cos(radians);
    s = sin(radians);

    c1 = 1.0 - cos(radians);

    length = sqrt(x * x + y * y + z * z);

    u[0] = x / length;
    u[1] = y / length;
    u[2] = z / length;

    for (i = 0; i < 16; i++) {
        R[i] = 0.0;
    }

    R[15] = 1.0;

    for (i = 0; i < 3; i++) {
        R[i * 4 + (i + 1) % 3] = u[(i + 2) % 3] * s;
        R[i * 4 + (i + 2) % 3] = -u[(i + 1) % 3] * s;
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            R[i * 4 + j] += c1 * u[i] * u[j] + (i == j ? c : 0.0);
        }
    }
}

/* 
 * Simulates gluPerspectiveMatrix 
 */
void perspective_matrix(double fovy, double aspect, double znear, double zfar, float *P) {
    int i;
    double f;

    f = 1.0/tan(fovy * 0.5);

    for (i = 0; i < 16; i++) {
        P[i] = 0.0;
    }

    P[0] = f / aspect;
    P[5] = f;
    P[10] = (znear + zfar) / (znear - zfar);
    P[11] = -1.0;
    P[14] = (2.0 * znear * zfar) / (znear - zfar);
    P[15] = 0.0;
}

/* 
 * Multiplies A by B and writes out to C. All matrices are 4x4 and column
 * major. In-place multiplication is supported.
 */
void multiply_matrix(float *A, float *B, float *C) {
	int i, j, k;
    float aTmp[16];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            aTmp[j * 4 + i] = 0.0;

            for (k = 0; k < 4; k++) {
                aTmp[j * 4 + i] += A[k * 4 + i] * B[j * 4 + k];
            }
        }
    }

    for (i = 0; i < 16; i++) {
        C[i] = aTmp[i];
    }
}
