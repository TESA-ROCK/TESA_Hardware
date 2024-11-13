/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: alsa.h
 *
 * Code generated for Simulink model 'alsa'.
 *
 * Model version                  : 1.1
 * Simulink Coder version         : 24.2 (R2024b) 21-Jun-2024
 * C/C++ source code generated on : Wed Nov 13 16:16:05 2024
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-A (64-bit)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef alsa_h_
#define alsa_h_
#ifndef alsa_COMMON_INCLUDES_
#define alsa_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "MW_alsa_audio.h"
#endif                                 /* alsa_COMMON_INCLUDES_ */

#include "alsa_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  codertarget_raspi_internal_Ra_T obj; /* '<Root>/ALSA Audio Capture' */
} DW_alsa_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  int16_T Out1[8820];                  /* '<Root>/Out1' */
} ExtY_alsa_T;

/* Real-time Model Data Structure */
struct tag_RTM_alsa_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_alsa_T alsa_DW;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_alsa_T alsa_Y;

/* Model entry point functions */
extern void alsa_initialize(void);
extern void alsa_step(void);
extern void alsa_terminate(void);

/* Real-time Model object */
extern RT_MODEL_alsa_T *const alsa_M;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'alsa'
 */
#endif                                 /* alsa_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
