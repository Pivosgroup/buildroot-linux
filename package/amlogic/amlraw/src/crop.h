/*******************************************************************
 * 
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: 
 *
 *  Author: Amlogic Software
 *  Created: 2007-1-24 03:38 PM
 *
 *******************************************************************/
#ifndef PICTURE_CROP_H
#define PICTURE_CROP_H

#if defined(AML_NIKE) || defined(AML_NIKED) || defined(AML_NIKED3) || defined(AML_APOLLO)
#define MAX_MC_SCALE_RATIO 0.5
#else
#define MAX_MC_SCALE_RATIO 1.0
#endif

typedef struct dimensions {
	int top;
	int left;
	int width;
	int height;
} dimension_t;
extern int kFramePixelWidth ;
extern int kFramePixelHeight ;
extern float kFrameAspectRatio ;
extern float kMaxCropAllowed ;
extern int   kTallerLossPolicy;
extern float kPixelAspectRatio ;
extern unsigned PixelAspectHigh ;
extern unsigned PixelAspectLow;
extern int compute_full_image(int imageWidth, int imageHeight, int image_rotation,int frameRotate, dimension_t *image_dimension, dimension_t *frame_dimension,float* ratio_x,float* ratio_y,unsigned char* ratio_method);		

#endif
