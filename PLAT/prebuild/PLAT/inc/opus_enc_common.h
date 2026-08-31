#pragma once
#ifndef _OPUS_DEC_COMMON_H_
#define _OPUS_DEC_COMMON_H_

#ifndef WIN32
#include "sctdef.h"
#define printf(...)
#else
#include "FxpDefine.h"
#include "SigProc_FIX.h"
#include "pitch.h"
#include "define.h"
#endif

#define Vpu_int32       int
#define Vpu_int16       short
#define Vpu_int8        signed char
#define Vpu_uint32      unsigned int
#define Vpu_uint16      unsigned short
#define Vpu_uint8       unsigned char
#define Vpu_int64       long long


#ifndef WIN32
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


#define MAX_LPC_ORDER_VPU                           16
#ifndef LTP_ORDER
#define LTP_ORDER                               5
#endif
#ifndef MAX_NB_SUBFR
#define MAX_NB_SUBFR                            4
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
void OpusDecVpuInit(void);
void OpusDecVpuClock_On(void);
void OpusDecVpuClock_Off(void);
#endif


#if 0


#define SILK_DEC_BIQUAD_VPU_ENABLE//ec_deviation
#ifdef SILK_DEC_BIQUAD_VPU_ENABLE
void silk_biquad_alt_vpu(
	const Vpu_int16      *in,        /* I:    input signal               */
	const Vpu_int32      *B_Q28,         /* I:    MA coefficients  [3]   */
	const Vpu_int32      *A_Q28,         /* I:    AR coefficients [2]   */
	Vpu_int32            *S,         /* I/O:  state vector [2]           */
	Vpu_int16            *out,       /* O:    output signal              */
	const Vpu_int32      len,         /* I:    signal length              */
	int	stride
	);
/*
* use IIR instruction to implement this function
* Verified on VPU simulator&chip by pxu
* Algorithm analyze:
*  y[i] = B[0] * x[i] +B[1] * x[i - 1] + B[2] * x[i - 2]+A[0]*y[i - 1]+A[1]*y[i - 2];
*/
void OPUS_Silk_biquad_vpu(
	const Vpu_int16      *in,        /* I:    input signal               */
	const Vpu_int16      *B,         /* I:    MA coefficients, Q13 [3]   */
	const Vpu_int16      *A,         /* I:    AR coefficients, Q13 [2]   */
	Vpu_int32            *S,         /* I/O:  state vector [2]           */
	Vpu_int16            *out,       /* O:    output signal              */
	const Vpu_int32      len         /* I:    signal length              */
	);
#endif



#define SILK_ANA_FILT_BANK_1_VPU_ENABLE//ec_deviation
#ifdef SILK_ANA_FILT_BANK_1_VPU_ENABLE
void silk_ana_filt_bank_1_vpu(
	const Vpu_int16            *in,                /* I    Input signal [N]                                            */
	Vpu_int32                  *S,                 /* I/O  State vector [2]                                            */
	Vpu_int16                  *outL,              /* O    Low band [N/2]                                              */
	Vpu_int16                  *outH,              /* O    High band [N/2]                                             */
	const Vpu_int32            N                   /* I    Number of input samples                                     */
	);
#endif



//#define SILK_APPLY_SINE_WINDOW_VPU_ENABLE//ec_costmoretime
#ifdef SILK_APPLY_SINE_WINDOW_VPU_ENABLE
void silk_apply_sine_window_vpu(
	Vpu_int16                  px_win[],           /* O    Pointer to windowed signal                                  */
	const Vpu_int16            px[],               /* I    Pointer to input signal                                     */
	const Vpu_int16              win_type,           /* I    Selects a window type                                       */
	const Vpu_int16              length              /* I    Window length, multiple of 4                                */
	);
#endif



#define CELT_PITCH_XCORR_VPU_ENABLE//ec_ok
#ifdef CELT_PITCH_XCORR_VPU_ENABLE
Vpu_int32 celt_pitch_xcorr_vpu(const Vpu_int16 *_x, const Vpu_int16 *_y,
	Vpu_int32 *xcorr, int len, int max_pitch, int arch);
#endif



#define CELT_PITCH_XCORR_1_VPU_ENABLE//ec_ok
#ifdef CELT_PITCH_XCORR_1_VPU_ENABLE
Vpu_int32 celt_pitch_xcorr_1_vpu(const Vpu_int16 *_x, int i,int n, Vpu_int32 ac0);
#endif



#define CELT_PITCH_XCORRPSHR32_2_VPU_ENABLE//ec_ok
#ifdef CELT_PITCH_XCORRPSHR32_2_VPU_ENABLE
void celt_pitch_xcorrpshr32_2_vpu(const Vpu_int16 *xptr, Vpu_int16 *xx, int n, int shift);
#endif



#define CELT_PITCH_XCORRSH32_2_VPU_ENABLE//ec_ok
#ifdef CELT_PITCH_XCORRSH32_2_VPU_ENABLE
void celt_pitch_xcorrsh32_2_vpu(Vpu_int32 *ac, int lag, int shift2, Vpu_int16 dir);
#endif



#define SILK_LPC_ANALYSIS_FILTER_VPU_ENABLE//ec_deviation
#ifdef SILK_LPC_ANALYSIS_FILTER_VPU_ENABLE
void silk_LPC_analysis_filter_vpu(
	Vpu_int16                  *out,               /* O    Output signal                                               */
	const Vpu_int16            *in,                /* I    Input signal                                                */
	const Vpu_int16            *B,                 /* I    MA prediction coefficients, Q12 [order]                     */
	const Vpu_int32            len,                /* I    Signal length                                               */
	const Vpu_int32            d,                  /* I    Filter order                                                */
	int                         arch                /* I    Run-time architecture                                       */
	);
#endif




#define SILK_SUM_SQR_SHIFT_VPU_ENABLE//ec_costmoretime_deviation//must enable
#ifdef SILK_SUM_SQR_SHIFT_VPU_ENABLE
void silk_sum_sqr_shift_vpu(
	Vpu_int32                  *energy,            /* O   Energy of x, after shifting to the right                     */
	int                    *shift,             /* O   Number of bits right shift applied to energy                 */
	const Vpu_int16            *x,                 /* I   Input vector                                                 */
	int                    len                 /* I   Length of input vector                                       */
	);
#endif





#define SILK_RESAMPLER_DOWN2_VPU_ENABLE//ec_error_160
#ifdef SILK_RESAMPLER_DOWN2_VPU_ENABLE
void silk_resampler_down2_vpu(
	Vpu_int32                  *S,                 /* I/O  State vector [ 2 ]                                          */
	Vpu_int16                  *out,               /* O    Output signal [ floor(len/2) ]                              */
	const Vpu_int16            *in,                /* I    Input signal [ len ]                                        */
	Vpu_int32                  inLen               /* I    Number of input samples                                     */
	);
#endif




//#define SILK_CORRMATRIX_FIX_VPU_ENABLE//ec_ok // silk_sum_sqr_shift_vpu_assert
#ifdef SILK_CORRMATRIX_FIX_VPU_ENABLE
void silk_corrMatrix_FIX_vpu(
	const Vpu_int16                *x,                                     /* I    x vector [L + order - 1] used to form data matrix X                         */
	const int                  L,                                      /* I    Length of vectors                                                           */
	const int                  order,                                  /* I    Max lag for correlation                                                     */
	Vpu_int32                      *XX,                                    /* O    Pointer to X'*X correlation matrix [ order x order ]                        */
	Vpu_int32                      *nrg,                                    /* O    Energy of x vector                                                            */
	int                        *rshifts,                               /* O    Right shifts of correlations and energy                                     */
	int                             arch                                    /* I    Run-time architecture                                                       */
	);
#endif



#define SILK_CORRVECTOR_FIX_VPU_ENABLE//ec_cannot_test
#ifdef SILK_CORRVECTOR_FIX_VPU_ENABLE
void silk_corrVector_FIX_vpu(
	const Vpu_int16                *x,                                     /* I    x vector [L + order - 1] used to form data matrix X                         */
	const Vpu_int16                *t,                                     /* I    Target vector [L]                                                           */
	const int                  L,                                      /* I    Length of vectors                                                           */
	const int                  order,                                  /* I    Max lag for correlation                                                     */
	Vpu_int32                      *Xt,                                    /* O    Pointer to X'*t correlation vector [order]                                  */
	const int                  rshifts,                                /* I    Right shifts of correlations                                                */
	int                             arch                                    /* I    Run-time architecture                                                       */
	);
#endif

#define SILK_SCALE_COPY_VECTPR16_VPU_ENABLE//ec_ok
#ifdef SILK_SCALE_COPY_VECTPR16_VPU_ENABLE
void silk_scale_copy_vector16_vpu(
	Vpu_int16                  *data_out,
	const Vpu_int16            *data_in,
	Vpu_int32                  gain_Q16,           /* I    Gain in Q16                                                 */
	const int              dataSize            /* I    Length                                                      */
	);
#endif




#define SILK_SMULWW_VPU_ENBALE//ec_ok
#ifdef SILK_SMULWW_VPU_ENBALE
void silk_SMULWW_vpu(const Vpu_int16 *x16, Vpu_int32 *x_sc_Q10, Vpu_int32 inv_gain_Q26, int subfr_length);
#endif




#define CELT_INNER_PROD_C_VPU_ENABLE//ec_ok
#ifdef CELT_INNER_PROD_C_VPU_ENABLE
Vpu_int32 celt_inner_prod_c_vpu(const Vpu_int16 *x,
	const Vpu_int16 *y, int N);
#endif





#define OPUS_DEC_LONGTERM_PREDICT_VPU_ENABLE//ec_deviation
#ifdef OPUS_DEC_LONGTERM_PREDICT_VPU_ENABLE
/*use FIR instruction to implement this function
Verified on simulator&chip by pxu*/
void OPUS_Silk_LongTermPredict_vpu(Vpu_int32 *sLTP_Q16,  /*output y*/
	Vpu_int32 *pres_Q10,
	Vpu_int32 *pexc_Q10,
	Vpu_int32 *pred_lag_ptr, /*input x ptr*/
	Vpu_int16 *CoeffA,		/*coeff x:a*/
	Vpu_int32 cal_len,
	Vpu_int32 seg_Len);
#endif




#define OPUS_DEC_SHORTTERM_PREDICT_VPU_ENABLE//ec_deviation
#ifdef OPUS_DEC_SHORTTERM_PREDICT_VPU_ENABLE
void OPUS_Silk_decode_short_term_prediction_vpu(
	Vpu_int16	*pxq,
	Vpu_int32	*pres_Q14,
	Vpu_int32	*sLPC_Q14,
	Vpu_int16	*A_Q12_tmp,
	Vpu_int32		LPC_order,
	Vpu_int32		subfr_length,
	Vpu_int32		Gain_Q10
	);
#endif




#define SILK_INTERPOLATE_VPU_ENABLE//ec_ok
#ifdef SILK_INTERPOLATE_VPU_ENABLE
void silk_interpolate_vpu(
	Vpu_int16                  xi[MAX_LPC_ORDER_VPU],            /* O    interpolated vector                         */
	const Vpu_int16            x0[MAX_LPC_ORDER_VPU],            /* I    first vector                                */
	const Vpu_int16            x1[MAX_LPC_ORDER_VPU],            /* I    second vector                               */
	const int              ifact_Q2,                       /* I    interp. factor, weight on 2nd vector        */
	const int              d                               /* I    number of parameters                        */
	);
#endif



//#define SILK_NLSF2A_1_VPU_ENABLE//ec_ok_costmoretime
#ifdef SILK_NLSF2A_1_VPU_ENABLE
	void silk_NLSF2A_1_vpu(Vpu_int32 *P, Vpu_int32 *Q, Vpu_int32 *a32_QA1, int length, int d);
#endif


//#define SILK_LPC_FIT_VPU_ENBALE//ec_ok_costmoretime
#ifdef SILK_LPC_FIT_VPU_ENBALE
	void silk_LPC_fit_vpu(
		Vpu_int16                  *a_QOUT,            /* O    Output signal                                               */
		Vpu_int32                    *a_QIN,             /* I/O  Input signal                                                */
		const int              QOUT,               /* I    Input Q domain                                              */
		const int              QIN,                /* I    Input Q domain                                              */
		const int              d                   /* I    Filter order                                                */
		);
#endif


#define SILK_NLSF2_RSHIFT_ROUND_VPU_ENABLE//ec_ok
#ifdef SILK_NLSF2_RSHIFT_ROUND_VPU_ENABLE
	void silk_NLSF2_RSHIFT_ROUND_vpu(Vpu_int32 *a32_QA1, Vpu_int16 *a_Q12, int shift, int length);
#endif



#define SILK_PROCESS_NLSFS_1_VPU_ENABLE//ec_ok
#ifdef SILK_PROCESS_NLSFS_1_VPU_ENABLE
	void silk_process_NLSFs_1_vpu(Vpu_int16 *pNLSFW_QW, Vpu_int16 *pNLSFW0_temp_QW, int mul, int length);
#endif


#define SILK_NSFL_STABILIZE_1_VPU_ENABLE//ec_deviation
#ifdef SILK_NSFL_STABILIZE_1_VPU_ENABLE
	void silk_NLSF_stabilize_1_vpu(Vpu_int16 *NLSF_Q15, const Vpu_int16 *NDeltaMin_Q15, int length, int* i, int* diff_Q15);
#endif


#define OPUS_DEC_LTPQ16CAL_SMULWB_VPU_ENABLE//ec_ok
#ifdef OPUS_DEC_LTPQ16CAL_SMULWB_VPU_ENABLE
	void opus_Silk_LTPQ16Cal_SMULWB_vpu(Vpu_int32* sLTP_Q16, Vpu_int16 *sLTP, Vpu_int32 inv_gain_Q32, Vpu_int32 len);
#endif



#define OPUS_DEC_LTPQ16CAL_SMULWW_VPU_ENABLE//ec_ok
#ifdef OPUS_DEC_LTPQ16CAL_SMULWW_VPU_ENABLE
	void opus_Silk_LTPQ16Cal_SMULWW_vpu(Vpu_int32* sLTP_Q16, Vpu_int32 gain_adj_Q16, Vpu_int32 cal_len);
#endif



#define OPUS_DEC_SCALEWITHGAIN_VPU_ENABLE//ec_ok
#ifdef OPUS_DEC_SCALEWITHGAIN_VPU_ENABLE
	void opus_Silk_ScaleWithGain_vpu(Vpu_int16* pxq, Vpu_int32* vec_Q10, Vpu_int32 Gain_Q16, Vpu_int32 cal_len);
#endif


//#define SILK_LPC_INVERSE_PRED_GAIN_FOR_VPU_ENABLE//ec_ok_costmoretime
#ifdef SILK_LPC_INVERSE_PRED_GAIN_FOR_VPU_ENABLE
	void silk_LPC_inverse_pred_gain_for_vpu(const Vpu_int16* Input, Vpu_int32* Output, Vpu_int32 sum, Vpu_int32 shift, Vpu_int32 len);
#endif	



#define LIMIT_WARPED_COEFS_ABSMAX_VPU_ENABLE//ec_ok
#ifdef LIMIT_WARPED_COEFS_ABSMAX_VPU_ENABLE
	void limit_warped_coefs_absmax_vpu(Vpu_int32* In, Vpu_int32 len, Vpu_int32 max, Vpu_int32 index );
#endif



#define SILK_LTP_ANALYSIS_FILTER_FIX_VPU_ENABLE//ec_deviation
#ifdef SILK_LTP_ANALYSIS_FILTER_FIX_VPU_ENABLE
	void silk_LTP_analysis_filter_FIX_vpu(
		Vpu_int16                      *LTP_res,                               /* O    LTP residual signal of length MAX_NB_SUBFR * ( pre_length + subfr_length )  */
		const Vpu_int16                *x,                                     /* I    Pointer to input signal with at least max( pitchL ) preceding samples       */
		const Vpu_int16                LTPCoef_Q14[LTP_ORDER * MAX_NB_SUBFR],/* I    LTP_ORDER LTP coefficients for each MAX_NB_SUBFR subframe                   */
		const int                  pitchL[MAX_NB_SUBFR],                 /* I    Pitch lag, one for each subframe                                            */
		const Vpu_int32                invGains_Q16[MAX_NB_SUBFR],           /* I    Inverse quantization gains, one for each subframe                           */
		const int                  subfr_length,                           /* I    Length of each subframe                                                     */
		const int                  nb_subfr,                               /* I    Number of subframes                                                         */
		const int                  pre_length                              /* I    Length of the preceding samples starting at &x[0] for each subframe         */
		);
#endif





#define SILK_INNER_PROD16_ALIGNED_64_C_VPU_ENABLE//ec_ok
#ifdef SILK_INNER_PROD16_ALIGNED_64_C_VPU_ENABLE
Vpu_int64 silk_inner_prod16_aligned_64_c_vpu(
	const Vpu_int16            *inVec1,            /*    I input vector 1                                              */
	const Vpu_int16            *inVec2,            /*    I input vector 2                                              */
	const int              len                 /*    I vector lengths                                              */
	);
#endif



#define SILK_INNER_PROD16_ALIGNED_64_FOR2_VPU_ENABLE//ec_cannot_test
#ifdef SILK_INNER_PROD16_ALIGNED_64_FOR2_VPU_ENABLE
Vpu_int32 silk_inner_prod16_aligned_64_for2_vpu(
	const Vpu_int16            *inVec1,            /*    I input vector 1                                              */
	const Vpu_int16            *inVec2,            /*    I input vector 2                                              */
	const int              subfr_len,                 /*    I vector lengths                                              */
	const int              nb_sub,//must be 2
	int              rshifts,
	int				 n
	);
#endif



#define SILK_INNER_PROD16_ALIGNED_64_FOR4_VPU_ENABLE//ec_cannot_test
#ifdef SILK_INNER_PROD16_ALIGNED_64_FOR4_VPU_ENABLE
Vpu_int32 silk_inner_prod16_aligned_64_for4_vpu(
	const Vpu_int16            *inVec1,            /*    I input vector 1                                              */
	const Vpu_int16            *inVec2,            /*    I input vector 2                                              */
	const int              subfr_len,                 /*    I vector lengths                                              */
	const int              nb_sub,//must be 2
	int              rshifts,
	int				 n
	);
#endif





#define SILK_BURG_MODIFIED_C_FOR_VPU_ENBALE//ec_ok
#ifdef SILK_BURG_MODIFIED_C_FOR_VPU_ENBALE
void silk_burg_modified_c_for_vpu(Vpu_int32* In, Vpu_int32* InOut, int shift, int len);
#endif



#ifdef TYPE_EC718M
#define SILK_NLSF_VQ_VPU_ENABLE//ec_devaition_ifcostmoretime
#endif
#ifdef SILK_NLSF_VQ_VPU_ENABLE
void silk_NLSF_VQ_vpu(
	Vpu_int32                  err_Q24[],                      /* O    Quantization errors [K]                     */
	const Vpu_int16            in_Q15[],                       /* I    Input vectors to be quantized [LPC_order]   */
	const Vpu_uint8            pCB_Q8[],                       /* I    Codebook vectors [K*LPC_order]              */
	const Vpu_int16            pWght_Q9[],                     /* I    Codebook weights [K*LPC_order]              */
	const int              K,                              /* I    Number of codebook vectors                  */
	const int              LPC_order                       /* I    Number of LPCs                              */
	);
#endif




#define SILK_NLSF_DEL_DEC_QUANT_VPU_ENABLE//ec_ok
#ifdef SILK_NLSF_DEL_DEC_QUANT_VPU_ENABLE
Vpu_int32 silk_NLSF_del_dec_quant_vpu(                             /* O    Returns RD value in Q25                     */
	Vpu_int8                   indices[],                      /* O    Quantization indices [ order ]              */
	const Vpu_int16            x_Q10[],                        /* I    Input [ order ]                             */
	const Vpu_int16            w_Q5[],                         /* I    Weights [ order ]                           */
	const Vpu_uint8            pred_coef_Q8[],                 /* I    Backward predictor coefs [ order ]          */
	const Vpu_int16            ec_ix[],                        /* I    Indices to entropy coding tables [ order ]  */
	const Vpu_uint8            ec_rates_Q5[],                  /* I    Rates []                                    */
	const int              quant_step_size_Q16,            /* I    Quantization step size                      */
	const Vpu_int16            inv_quant_step_size_Q6,         /* I    Inverse quantization step size              */
	const Vpu_int32            mu_Q20,                         /* I    R/D tradeoff                                */
	const Vpu_int16            order                           /* I    Number of input values                      */
	);
#endif



#define SILK_SETUP_FS_VPU_ENABLE//ec_16k_only



//#define SILK_VAD_GETSA_Q8_C_VPU1_ENABLE//ec_deviation_costmoretime
#ifdef SILK_VAD_GETSA_Q8_C_VPU1_ENABLE
void silk_VAD_GetSA_Q8_c_vpu1(Vpu_int16* X, int length);
#endif



//#define SILK_VAD_GETSA_Q8_C_VPU2_ENABLE//ec_Deviation_costmoretime
#ifdef SILK_VAD_GETSA_Q8_C_VPU2_ENABLE
Vpu_int32 silk_VAD_GetSA_Q8_c_vpu2(Vpu_int16* X, int sublen, int startindex, Vpu_int32 sum, Vpu_int32 XnrgOut);
#endif




#define SILK_WARPED_AUTOCORRELATION_FIX_VPU_ENABLE//ec_deviation





#define SILK_RSHIFT_ROUND_SUB_VPU_ENABLE//ec_ok
#ifdef SILK_RSHIFT_ROUND_SUB_VPU_ENABLE
void silk_RSHIFT_ROUND_sub_vpu(Vpu_int32 *Af_QA, Vpu_int32 *A_Q16, int shift, int length);
#endif


//#define SILK_RSHIFT_ROUND_2_VPU_ENABLE//ec_wrong
#ifdef SILK_RSHIFT_ROUND_2_VPU_ENABLE
void silk_RSHIFT_ROUND_2_vpu(Vpu_int32 *Af_QA, Vpu_int32 *CAf, Vpu_int32 *A_Q16, Vpu_int32 nrg, Vpu_int32 tmp1, int shift, int length);
#endif




#define SILK_RSHIFT_ROUND_INDEX_VPU_ENABLE//ec_ok
#ifdef SILK_RSHIFT_ROUND_INDEX_VPU_ENABLE
void silk_rshift_round_index_vpu(Vpu_int32* In, Vpu_int16* Out, int shift, int len, int startindex);
#endif







#endif//if 0




#endif


