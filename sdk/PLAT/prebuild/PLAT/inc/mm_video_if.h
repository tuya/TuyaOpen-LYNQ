#ifndef MM_VIDEO_IF_H
#define MM_VIDEO_IF_H


/*
*brief      set memory buffer for video module allocation
*
*param_in   pBuf, buffer address
*           uLen, buffer size
*           uDataLen, jpeg data length
*
*return     0, success
*           <0, error
*/
int Video_MemSetBuf(unsigned char *pBuf,unsigned int uLen);


/*
*brief      scale image, rgb565 format
*
*param_in   pInBuf, input image buffer
*           uInWidth, input image width
*           uInHeight, input image height
*param_out pOutBuf, output image buffer
*           uOutWidth, output image width
*           uOutHeight, output image height
*
*return     none
*/
void Video_ScaleImageRgb565(unsigned short *pInBuf,unsigned int uInWidth,unsigned int uInHeight,unsigned short *pOutBuf,unsigned int uOutWidth,unsigned int uOutHeight);


/*
*brief      transfrom rgb565 image to yuyv image
*
*param_in   pRgb, rgb image buffer
*           uWidth, image width
*           uHeight, image height
*param_out pYuyv, yuyv image buffer
*
*return     none
*/
void Video_TransRgb565ToYuyv(unsigned short *pRgb,unsigned char *pYuyv,unsigned int uWidth,unsigned int uHeight);





#endif

