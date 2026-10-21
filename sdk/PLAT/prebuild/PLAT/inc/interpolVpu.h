#ifndef _INTERPOL_VPU_H_
#define _INTERPOL_VPU_H_

void InterpolVpuInit(void);

void InterpolVpuDeInit(void);

void FirFilterVpu(short *inPtr, short *outPtr, short inLen, const short *coeffs, short filterLen, short L);

void InterpolVpu(short *inPtr, short *outPtr, short inLen, const short *coeffs, short filterLen, short L);

#endif