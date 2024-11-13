/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: alsa.c
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

#include "alsa.h"
#include "rtwtypes.h"

/* Block states (default storage) */
DW_alsa_T alsa_DW;

/* External outputs (root outports fed by signals with default storage) */
ExtY_alsa_T alsa_Y;

/* Real-time model */
static RT_MODEL_alsa_T alsa_M_;
RT_MODEL_alsa_T *const alsa_M = &alsa_M_;

/* Model step function */
void alsa_step(void)
{
  int32_T i;
  char_T b[7];
  static const char_T b_0[7] = "hw:1,0";

  /* MATLABSystem: '<Root>/ALSA Audio Capture' */
  for (i = 0; i < 7; i++) {
    b[i] = b_0[i];
  }

  MW_AudioRead(&b[0], MW_AUDIO_16, &alsa_Y.Out1[0]);

  /* End of MATLABSystem: '<Root>/ALSA Audio Capture' */
}

/* Model initialize function */
void alsa_initialize(void)
{
  {
    int32_T i;
    char_T b[7];
    static const char_T b_0[7] = "hw:1,0";

    /* Start for MATLABSystem: '<Root>/ALSA Audio Capture' */
    alsa_DW.obj.matlabCodegenIsDeleted = false;
    alsa_DW.obj.isInitialized = 1;
    for (i = 0; i < 7; i++) {
      b[i] = b_0[i];
    }

    audioCaptureInit(&b[0], 44100.0, 2.0, 0.5, 4410.0, MW_AUDIO_16);
    alsa_DW.obj.isSetupComplete = true;

    /* End of Start for MATLABSystem: '<Root>/ALSA Audio Capture' */
  }
}

/* Model terminate function */
void alsa_terminate(void)
{
  int32_T i;
  char_T b[7];
  static const char_T b_0[7] = "hw:1,0";

  /* Terminate for MATLABSystem: '<Root>/ALSA Audio Capture' */
  if (!alsa_DW.obj.matlabCodegenIsDeleted) {
    alsa_DW.obj.matlabCodegenIsDeleted = true;
    if ((alsa_DW.obj.isInitialized == 1) && alsa_DW.obj.isSetupComplete) {
      for (i = 0; i < 7; i++) {
        b[i] = b_0[i];
      }

      MW_AudioClose(&b[0], MW_AUDIO_IN);
    }
  }

  /* End of Terminate for MATLABSystem: '<Root>/ALSA Audio Capture' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
