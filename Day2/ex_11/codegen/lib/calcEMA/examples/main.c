/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: main.c
 *
 * MATLAB Coder version            : 24.2
 * C/C++ source code generated on  : 13-Nov-2024 10:50:46
 */

/*************************************************************************/
/* This automatically generated example C main file shows how to call    */
/* entry-point functions that MATLAB Coder generated. You must customize */
/* this file for your application. Do not modify this file directly.     */
/* Instead, make a copy of this file, modify it, and integrate it into   */
/* your development environment.                                         */
/*                                                                       */
/* This file initializes entry-point function arguments to a default     */
/* size and value before calling the entry-point functions. It does      */
/* not store or use any values returned from the entry-point functions.  */
/* If necessary, it does pre-allocate memory for returned values.        */
/* You can use this file as a starting point for a main function that    */
/* you can deploy in your application.                                   */
/*                                                                       */
/* After you copy the file, and before you deploy it, you must make the  */
/* following changes:                                                    */
/* * For variable-size function arguments, change the example sizes to   */
/* the sizes that your application requires.                             */
/* * Change the example values of function arguments to the values that  */
/* your application requires.                                            */
/* * If the entry-point functions return values, store these values or   */
/* otherwise use them as required by your application.                   */
/*                                                                       */
/*************************************************************************/

/* Include Files */
#include "main.h"
#include "calcEMA.h"
#include "calcEMA_terminate.h"

/* Function Declarations */
static void argInit_1x100_real_T(double result[100]);

static double argInit_real_T(void);

static unsigned int argInit_uint32_T(void);

/* Function Definitions */
/*
 * Arguments    : double result[100]
 * Return Type  : void
 */
static void argInit_1x100_real_T(double result[100])
{
  int idx1;
  /* Loop over the array to initialize each element. */
  for (idx1 = 0; idx1 < 100; idx1++) {
    /* Set the value of the array element.
Change this value to the value that the application requires. */
    result[idx1] = argInit_real_T();
  }
}

/*
 * Arguments    : void
 * Return Type  : double
 */
static double argInit_real_T(void)
{
  return 0.0;
}

/*
 * Arguments    : void
 * Return Type  : unsigned int
 */
static unsigned int argInit_uint32_T(void)
{
  return 0U;
}

/*
 * Arguments    : int argc
 *                char **argv
 * Return Type  : int
 */
int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  /* The initialize function is being called automatically from your entry-point
   * function. So, a call to initialize is not included here. */
  /* Invoke the entry-point functions.
You can call entry-point functions multiple times. */
  main_calcEMA();
  /* Terminate the application.
You do not need to do this more than one time. */
  calcEMA_terminate();
  return 0;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void main_calcEMA(void)
{
  double dv[100];
  double ema[100];
  /* Initialize function 'calcEMA' input arguments. */
  /* Initialize function input argument 'data'. */
  /* Call the entry-point 'calcEMA'. */
  argInit_1x100_real_T(dv);
  calcEMA(dv, argInit_uint32_T(), ema);
}

/*
 * File trailer for main.c
 *
 * [EOF]
 */
