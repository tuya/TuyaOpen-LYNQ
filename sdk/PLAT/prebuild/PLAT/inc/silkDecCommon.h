#ifndef _SILK_DEC_COMMON_H_
#define _SILK_DEC_COMMON_H_

#ifndef WIN32
#include "sctdef.h"
#define printf(...)
#define fprintf(...)
#else
#include "FxpDefine.h"
#endif

#define Vpu_int32       int
#define Vpu_int16       short
#define Vpu_int8        signed char

#define Vpu_uint32      unsigned int
#define Vpu_uint16      unsigned short
#define Vpu_uint8       unsigned char

#ifndef WIN32
//#define SILK_DECODER_DECAPI_LOG
//#define SILK_DECODER_DECFRAME_LOG
//#define SILK_DECODER_DECCORE_LOG
//#define SILK_DECODER_PLCUPDATE_LOG
//#define SILK_DECODER_DECPARAM_LOG
//#define SILK_DECODER_DECRANGE_LOG
//#define SILK_DECODER_NLSF_LOG
//#define SILK_DECODER_NLSF2A_LOG
//#define SILK_DECODER_NLSF_MSVQ_LOG
//#define SILK_DECODER_DECPULSE_LOG
//#define SILK_DECODER_MAPREDICT_LOG
//#define SILK_RUNTIME_CONVERT(x) (x*1000/3072)
//#define SILK_RUNTIME_CONVERT2(x) (x/3072)

#define SILK_DECODER_RAMCODE				PLAT_FM_RAMCODE
#define SILK_DECODER_RAMCODE_ASMB			PLAT_FA_RAMCODE
#define SILK_DECODER_PSRAM_RAMDATA			PLAT_FPSRAM_DATA
#define SILK_DECODER_PSRAM_ZIDATA			PLAT_FPSRAM_ZI
#define SILK_DECODER_FORECEINLINE_CODE 		__FORCEINLINE
#else
#define SILK_DECODER_RAMCODE				
#define SILK_DECODER_RAMCODE_ASMB		
#define SILK_DECODER_PSRAM_RAMDATA		
#define SILK_DECODER_PSRAM_ZIDATA		
#define SILK_DECODER_FORECEINLINE_CODE
#endif


#define SILK_MAX_LPC_ORDER	16
#define SILK_MAX_NSTAGES	16

/*use this macro modifier instead of const to change the table position in ram or flash*/
#ifndef WIN32
#define SILK_CONST_TABLE_LOCATE __attribute__((aligned(4)))
#else
#define SILK_CONST_TABLE_LOCATE const
#endif

/*enable for MA prediction error filter, enable for C-level acceleration method */
#define SILK_DECODER_EC_CPUCORE

#ifndef WIN32
void SilkDecVpuInit(void);

void SilkDecVpuClock_On(void);

void SilkDecVpuClock_Off(void);
#endif
/*
* use IIR instruction to implement this function
* Verified on VPU simulator&chip by pxu
* Algorithm analyze:
*  y[i] = B[0] * x[i] +B[1] * x[i - 1] + B[2] * x[i - 2]+A[0]*y[i - 1]+A[1]*y[i - 2];
*/
#define SILK_DEC_BIQUAD_VPU_ENABLE
#ifdef SILK_DEC_BIQUAD_VPU_ENABLE
void SKP_Silk_biquad_vpu(
	const Vpu_int16      *in,        /* I:    input signal               */
	const Vpu_int16      *B,         /* I:    MA coefficients, Q13 [3]   */
	const Vpu_int16      *A,         /* I:    AR coefficients, Q13 [2]   */
	Vpu_int32            *S,         /* I/O:  state vector [2]           */
	Vpu_int16            *out,       /* O:    output signal              */
	const Vpu_int32      len         /* I:    signal length              */
	);
#endif

/*enable for LTPQ16 SMULWB, use vpu mult instruction*/
#define SILK_DEC_LTPQ16CAL_SMULWB_VPU_ENABLE
#ifdef SILK_DEC_LTPQ16CAL_SMULWB_VPU_ENABLE
void SKP_Silk_LTPQ16Cal_SMULWB_vpu(Vpu_int32* sLTP_Q16, Vpu_int16 *sLTP, Vpu_int32 inv_gain_Q32, Vpu_int32 len);
#endif

/*enable for LTPQ16 SMULWW, use vpu mult instruction*/
#define SILK_DEC_LTPQ16CAL_SMULWW_VPU_ENABLE
#ifdef SILK_DEC_LTPQ16CAL_SMULWW_VPU_ENABLE
void SKP_Silk_LTPQ16Cal_SMULWW_vpu(Vpu_int32* sLTP_Q16, Vpu_int32 gain_adj_Q16, Vpu_int32 cal_len);
#endif

#define SILK_DEC_LONGTERM_PREDICT_VPU_ENABLE
#ifdef SILK_DEC_LONGTERM_PREDICT_VPU_ENABLE
/*use FIR instruction to implement this function
  Verified on simulator&chip by pxu*/
void SKP_Silk_LongTermPredict_vpu(Vpu_int32 *sLTP_Q16,  /*output y*/
	Vpu_int32 *pres_Q10,
	Vpu_int32 *pexc_Q10,
	Vpu_int32 *pred_lag_ptr, /*input x ptr*/
	Vpu_int16 *CoeffA,		/*coeff x:a*/
	Vpu_int32 cal_len,
	Vpu_int32 seg_Len);
#endif

/*enable for pxq SMULWW, use vpu mult instruction*/
#define SILK_DEC_SCALEWITHGAIN_VPU_ENABLE
#ifdef SILK_DEC_SCALEWITHGAIN_VPU_ENABLE
void SKP_Silk_ScaleWithGain_vpu(Vpu_int16* pxq, Vpu_int32* vec_Q10, Vpu_int32 Gain_Q16, Vpu_int32 cal_len);
#endif

/*
* use IIR instruction to implement this function
* Verified on VPU simulator&chip by pxu
* Algorithm analyze:
*  y[i] = a[0] * x[i] +b[0] * y[i - 1] + b[1] * y[i - 2]+...+b[15] * y[i - 16];
*/
#define SILK_DEC_SHORTTERM_PREDICT_VPU_ENABLE
#ifdef SILK_DEC_SHORTTERM_PREDICT_VPU_ENABLE
void SKP_Silk_decode_short_term_prediction_vpu(
	Vpu_int32	*vec_Q10,
	Vpu_int32	*pres_Q10,
	Vpu_int32	*sLPC_Q14,
	Vpu_int16	*A_Q12_tmp,
	Vpu_int32		LPC_order,
	Vpu_int32		subfr_length
	);
#endif

/*enable for NLSF ADD, use vpu ADD instruction*/
#define SILK_DEC_NLSF_MSVQ_VPU_ENABLE
#ifdef SILK_DEC_NLSF_MSVQ_VPU_ENABLE
void SKP_Silk_NLSF_MSVQ_decode_half_vpu(
	Vpu_int32                         *pNLSF_Q15,     /* O    Pointer to decoded output vector [LPC_ORDER x 1]    */
	Vpu_int32						  volatile *vldAddr,
	Vpu_int16						  nStages,
	const Vpu_int32                   LPC_order       /* I    LPC order used                                      */
	);
#endif


#endif
