/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: _coder_calcEMA_api.h
 *
 * MATLAB Coder version            : 24.2
 * C/C++ source code generated on  : 13-Nov-2024 10:50:46
 */

#ifndef _CODER_CALCEMA_API_H
#define _CODER_CALCEMA_API_H

/* Include Files */
#include "emlrt.h"
#include "mex.h"
#include "tmwtypes.h"
#include <string.h>

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

#ifdef __cplusplus
extern "C" {
#endif

/* Function Declarations */
void calcEMA(real_T data[100], uint32_T N, real_T ema[100]);

void calcEMA_api(const mxArray *const prhs[2], const mxArray **plhs);

void calcEMA_atexit(void);

void calcEMA_initialize(void);

void calcEMA_terminate(void);

void calcEMA_xil_shutdown(void);

void calcEMA_xil_terminate(void);

#ifdef __cplusplus
}
#endif

#endif
/*
 * File trailer for _coder_calcEMA_api.h
 *
 * [EOF]
 */
