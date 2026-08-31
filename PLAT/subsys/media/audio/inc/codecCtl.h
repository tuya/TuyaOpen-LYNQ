/******************************************************************************

*(C) Copyright 2018 EIGENCOMM International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: codecCtl.h
*
*  Description:
*
*  History: Rev1.0   2024-07-25
*
*  Notes: codec control
*
******************************************************************************/

#ifndef __CODEC_CTL_H__
#define __CODEC_CTL_H__


#include <stdint.h>


void codecCtlInit(void);
void codecCtlWrite(uint8_t output);


#endif
