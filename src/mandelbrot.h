
#ifndef _MANDELBROT_H
#define _MANDELBROT_H

#include <sgl.h>
#include "debug.h"

/**
 * @file mandelbrot.h
 * @brief Mandelbrot rendering helpers and work-sliced render routines.
 */

/** Render surface width in pixels. */
static const Sint16 width = X_RESOLUTION;
/** Render surface height in pixels. */
static const Sint16 height = Y_RESOLUTION;
/** Minimum real-axis bound in the complex plane. */
static double minReal = -2.0;
/** Maximum real-axis bound in the complex plane. */
static double maxReal = 1.0;
/** Minimum imaginary-axis bound in the complex plane. */
static double minImag = -1.0;
/** Maximum imaginary-axis bound in the complex plane. */
static double maxImag = 1.0;
/** Maximum Mandelbrot iterations per pixel. */
static const Sint16 maxIterations = 100;

/** Current scanline index for work-sliced rendering. */
static Uint16 y = 0;
/** Current column index for work-sliced rendering. */
static Uint16 x = 0;
/** External stop flag for the renderer. */
static Uint8 done = 0;

/** Indicates whether the slave processor task has finished. */
static volatile Uint8 slavedone = 1;
/** Optional slave call counter (debug/telemetry). */
static Uint16 slaveCpt = 0;


typedef struct parameter_slow{
  /** Real-axis coordinate of the complex point. */
  double real;
  /** Imaginary-axis coordinate of the complex point. */
  double imag;
  /** Screen-space x coordinate. */
  Uint16 x;
  /** Screen-space y coordinate. */
  Uint16 y;
} parameter_slow;

typedef struct parameter_fixed{
  /** Real-axis coordinate in FIXED format. */
  FIXED real;
  /** Imaginary-axis coordinate in FIXED format. */
  FIXED imag;
  /** Screen-space x coordinate. */
  Uint16 x;
  /** Screen-space y coordinate. */
  Uint16 y;
} parameter_fixed;


/** Shared parameter block for floating-point slave jobs. */
static volatile parameter_slow slave_param;
/** Shared parameter block for fixed-point slave jobs. */
static volatile parameter_fixed slave_param_fixed;

/**
 * @brief Multiplies two FIXED values while preserving scale.
 * @param a Left operand.
 * @param b Right operand.
 * @return Scaled FIXED product.
 */
static inline FIXED fixed_mul(FIXED a, FIXED b)
{
  return (FIXED)(((long long)a * (long long)b) / toFIXED(1.0));
}

/**
 * @brief Divides two FIXED values while preserving scale.
 * @param num Numerator.
 * @param den Denominator.
 * @return Scaled FIXED quotient.
 */
static inline FIXED fixed_div(FIXED num, FIXED den)
{
  return (FIXED)(((long long)num * (long long)toFIXED(1.0)) / den);
}


/**
 * @brief Computes Mandelbrot escape iteration using floating-point arithmetic.
 * @param param Complex coordinate and pixel metadata.
 * @return Iteration count when escaping, or maxIterations if bounded.
 */
static int isInMandelbrotSlow(parameter_slow * param) {
  Sint16 iteration = 0;
  double zReal = param->real;
  double zImag = param->imag;

  while (iteration < maxIterations) {
    double zRealTemp = zReal * zReal - zImag * zImag + param->real;
    zImag = 2 * zReal * zImag + param->imag;
    zReal = zRealTemp;

    if (zReal * zReal + zImag * zImag > 4.0) {
      return iteration;
    }

    ++iteration;
  }

  return maxIterations;
}

/**
 * @brief Computes Mandelbrot escape iteration using FIXED arithmetic.
 * @param param Complex coordinate and pixel metadata.
 * @return Iteration count when escaping, or maxIterations if bounded.
 */
static int isInMandelbrot(parameter_fixed * param) {
  Sint16 iteration = 0;
  FIXED zReal = param->real;
  FIXED zImag = param->imag;
  const FIXED two = toFIXED(2.0);
  const FIXED four = toFIXED(4.0);

  while (iteration < maxIterations) {
    FIXED zRealTemp = fixed_mul(zReal, zReal) - fixed_mul(zImag, zImag) + param->real;
    zImag = fixed_mul(two, fixed_mul(zReal, zImag)) + param->imag;
    zReal = zRealTemp;

    if (fixed_mul(zReal, zReal) + fixed_mul(zImag, zImag) > four) {
      return iteration;
    }

    ++iteration;
  }

  return maxIterations;
}

/**
 * @brief Slave callback for one floating-point Mandelbrot pixel.
 * @param data Pointer to parameter_slow.
 */
static void SlaveTask_Slow(void * data)
{
  //char tmp[6];
  parameter_slow * param = (parameter_slow *)data;
  int iteration = isInMandelbrotSlow(param);
  slBMPset( param->x-(X_RESOLUTION>>1), param->y-(Y_RESOLUTION>>1), palette[iteration % 256] );
  //sprintf(tmp, "%d", slaveCpt++);
  //slPrint( tmp, slLocate( 12, 3 ) );
  slavedone = 1;
}

/**
 * @brief Slave callback for one fixed-point Mandelbrot pixel.
 * @param data Pointer to parameter_fixed.
 */
static void SlaveTask(void * data)
{
  //char tmp[6];
  parameter_fixed * param = (parameter_fixed *)data;
  int iteration = isInMandelbrot(param);
  slBMPset( param->x-(X_RESOLUTION>>1), param->y-(Y_RESOLUTION>>1), palette[iteration % 256] );
  //sprintf(tmp, "%d", slaveCpt++);
  //slPrint( tmp, slLocate( 12, 3 ) );
  slavedone = 1;
}

/**
 * @brief Work-sliced fixed-point Mandelbrot renderer.
 *
 * Renders until the timeslice budget is exhausted, then returns so the caller
 * can continue on a later frame.
 */
static void mandelbrot_fixed() {
  static Uint8 started = 0;
  if (!started) {
    debug_print("trace: mandelbrot_fixed start");
    started = 1;
  }

  slavedone = 1;
  Uint32 timemax = TIM_FRT_MCR_TO_CNT(100000);
  TIM_FRT_SET_16(0);

  for (; y < height; y++) {
    for (; x < width; x++) {

      if(TIM_FRT_CNT_TO_MCR(TIM_FRT_GET_16()) > timemax) {
        debug_print("trace: mandelbrot_fixed timeslice return");
        return;
      }

      if(done) {
        return;
      }

      if (slavedone) {
        slavedone = 0;
        slave_param_fixed.real = toFIXED(minReal) + fixed_div(toFIXED(x) * (toFIXED(maxReal) - toFIXED(minReal)), toFIXED(width - 1));
        slave_param_fixed.imag = toFIXED(minImag) + fixed_div(toFIXED(y) * (toFIXED(maxImag) - toFIXED(minImag)), toFIXED(height - 1));
        slave_param_fixed.x = x;
        slave_param_fixed.y = y;
        slSlaveFunc(SlaveTask, (void *)(&slave_param_fixed));

      } else {
        parameter_fixed param;
          param.real = toFIXED(minReal) + fixed_div(toFIXED(x) * (toFIXED(maxReal) - toFIXED(minReal)), toFIXED(width - 1));
          param.imag = toFIXED(minImag) + fixed_div(toFIXED(y) * (toFIXED(maxImag) - toFIXED(minImag)), toFIXED(height - 1));
        int iteration = isInMandelbrot(&param);
        slBMPset( x-(X_RESOLUTION>>1), y-(Y_RESOLUTION>>1), palette[iteration % 256] );
      }
    }
    x= 0;
  }

  done=0;
  debug_print("trace: mandelbrot_fixed complete");
}

/**
 * @brief Work-sliced floating-point Mandelbrot renderer.
 *
 * Renders until the timeslice budget is exhausted, then returns so the caller
 * can continue on a later frame.
 */
static void mandelbrot_slow() {
  static Uint8 started = 0;
  if (!started) {
    debug_print("trace: mandelbrot_slow start");
    started = 1;
  }

  slavedone = 1;
  Uint32 timemax = TIM_FRT_MCR_TO_CNT(100000);
  TIM_FRT_SET_16(0);

  for (; y < height; y++) {
    for (; x < width; x++) {

      if(TIM_FRT_CNT_TO_MCR(TIM_FRT_GET_16()) > timemax) {
        debug_print("trace: mandelbrot_slow timeslice return");
        return;
      }

      if(done) {
        return;
      }

      if (slavedone) {
        slavedone = 0;
        slave_param.real = minReal + x * (maxReal - minReal) / (width - 1);
        slave_param.imag = minImag + y * (maxImag - minImag) / (height - 1);
        slave_param.x = x;
        slave_param.y = y;
        slSlaveFunc(SlaveTask_Slow, (void *)(&slave_param));

      } else {
        parameter_slow param;
        param.real = minReal + x * (maxReal - minReal) / (width - 1);
        param.imag = minImag + y * (maxImag - minImag) / (height - 1);
        int iteration = isInMandelbrotSlow(&param);
        slBMPset( x-(X_RESOLUTION>>1), y-(Y_RESOLUTION>>1), palette[iteration % 256] );
      }
    }
    x= 0;
  }

  done=0;
  debug_print("trace: mandelbrot_slow complete");
}

#endif
