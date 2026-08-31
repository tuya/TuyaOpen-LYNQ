#pragma once
#ifndef _AAC_VPU_COMMON_H_
#define _AAC_VPU_COMMON_H_

#ifndef WIN32
#include "sctdef.h"
#else
//
#endif
#include <stdlib.h>//for calloc


#define PIVPU 3.1415927

#define Vpu_int32       int
#define Vpu_int16       short
#define Vpu_int8        signed char
#define Vpu_uint32      unsigned int
#define Vpu_uint16      unsigned short
#define Vpu_uint8       unsigned char
#define Vpu_int64       long long
#define aac_SHR(a,shift) ((a) >> (shift))
#define aac_NEG32(x) (-(x))
#define aac_EXTRACT16(x) ((Vpu_int16)(x))
//#define aac_SMULWW(a32, b32)            ((Vpu_int32)(((Vpu_int64)(a32) * (b32)) >> 16))
//#define aac_SMULWW_q0(a32, b32)            ((Vpu_int32)(((Vpu_int64)(a32) * (b32))))
//#define aac_SMULWW_q5(a32, b32)            ((Vpu_int32)(((Vpu_int64)(a32) * (b32)) >> 5))

#define aac_SMULWW(a32, b32)            ((Vpu_int16)(((Vpu_int64)(a32) * (b32)) >> 16))
#define aac_SMULWW_q0(a32, b32)            ((Vpu_int16)(((Vpu_int64)(a32) * (b32))))
#define aac_SMULWW_q0_int32(a32, b32)            ((Vpu_int32)(((Vpu_int64)(a32) * (b32))))
#define aac_SMULWW_q5(a32, b32)            ((Vpu_int16)(((Vpu_int64)(a32) * (b32)) >> 5))
#define aac_SMULWW_q5_int32(a32, b32)            ((Vpu_int32)(((Vpu_int64)(a32) * (b32)) >> 5))

/*#define aac_SMULWW(a32, b32)            (((Vpu_int64)(a32 * b32)) >> 16)
#define aac_SMULWW_q0(a32, b32)            ((Vpu_int64)(a32 * b32))
#define aac_SMULWW_q5(a32, b32)            (((Vpu_int64)(a32 * b32)) >> 5)*/
#define aac_EXTEND32(x) ((int)(x))
#define aac_SHR16(a,shift) ((a) >> (shift))
#define aac_PSHR16(a,shift) (aac_SHR16((a)+((1<<((shift))>>1)),shift))
#define aac_SHR32(a,shift) ((a) >> (shift))
#define aac_SHL32(a,shift) ((a) << (shift))

#define aac_ADD16(a,b) ((Vpu_int16)((Vpu_int16)(a)+(Vpu_int16)(b)))
#define aac_ADD32(a,b) ((int)(a)+(int)(b))
#define aac_SUB16(a,b) ((Vpu_int16)(a)-(Vpu_int16)(b))
#define aac_SUB32(a,b) ((int)(a)-(int)(b))
#define aac_MULT16_16(a,b)     (((int)(Vpu_int16)(a))*((int)(Vpu_int16)(b)))
#define aac_PSHR32(a,shift) (aac_SHR32((a)+((aac_EXTEND32(1)<<((shift))>>1)),shift))
#define aac_DIV32(a,b) (((int)(a))/((int)(b)))
#define aac_MIN16(a,b) ((a) < (b) ? (a) : (b)) 
#define aac_MULT16_16_P15(a,b) (aac_SHR(aac_ADD32(16384,aac_MULT16_16((a),(b))),15))

#ifndef WIN32
void aacEncVpuInit(void);
void aacEncVpuClock_On(void);
void aacEncVpuClock_Off(void);
#endif


void *aac_alloc(int size);
Vpu_int16 _aac_cos_pi_2(Vpu_int16 x);
Vpu_int16 aac_cos_norm(Vpu_int32 x);


#ifdef fixed_point_test

#define AACRFFT_VPU_ENABLE
#ifdef AACRFFT_VPU_ENABLE
void aacrfft_vpu(Vpu_int16* in_real, int N);
#endif

#define AACFFT_VPU_ENABLE
#ifdef AACFFT_VPU_ENABLE
void aacfft_vpu(Vpu_int16* in_real, Vpu_int16* in_imag, int N);
#endif

#endif//202601


#if 1
#define MEMSET0_VPU_ENBALE
#ifdef MEMSET0_VPU_ENBALE
void memset0_vpu(Vpu_int16* in, int len);
#endif

#define MEMCPY_VPU_ENABLE
#ifdef MEMCPY_VPU_ENABLE
void memcpy_vpu(Vpu_int16* in, Vpu_int32* out, int len, int numchan);
#endif

#define MEMCPY32_32_VPU_ENABLE
#ifdef MEMCPY32_32_VPU_ENABLE
void memcpy32_32_vpu(Vpu_int32* in, Vpu_int32* out, int len, int numchan);
#endif

#define MAXAAC_VPU_ENABLE
#ifdef MAXAAC_VPU_ENABLE
int maxaac_vpu(Vpu_int32* in, int start, int end, int maxx_int);
#endif


#define HANN_VPU_ENABLE
#ifdef HANN_VPU_ENABLE
void hann_vpu(Vpu_int16* in, Vpu_int16* win, int len);
#endif


#define FilterBank_WINDOW_VPU_ENABLE
#ifdef FilterBank_WINDOW_VPU_ENABLE
void only_long_window_vpu_enable1(Vpu_int16* in, Vpu_int32* win1, Vpu_int32* out, int len, int Q);
void only_long_window_vpu_enable2(Vpu_int16* in, Vpu_int32* win2, Vpu_int32* out, int len, int Q);
#endif


#define AACFFT_VPU_ENABLE
#ifdef AACFFT_VPU_ENABLE
void aacfft_vpu(Vpu_int16* in_real, Vpu_int16* in_imag, int N);
#endif


#define AACRFFT_VPU_ENABLE
#ifdef AACRFFT_VPU_ENABLE
void aacrfft_vpu(Vpu_int16* in_real, int N);
#endif


#define MUL3232_VPU_ENABLE
#ifdef MUL3232_VPU_ENABLE
long long MulSum3232_vpu(const Vpu_int32* in, long long sum, int len);
#endif


#define SIGN_VPU_ENABLE
#ifdef SIGN_VPU_ENABLE
void sign_vpu(int*sign, int* in1, int* in2, int len);
#endif



#endif//if 0


#endif


