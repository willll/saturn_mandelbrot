
#ifndef _MANDELBROT_H
#define _MANDELBROT_H

#include <sgl.h>

const Sint16 width = X_RESOLUTION;
const Sint16 height = Y_RESOLUTION;
static double minReal = -2.0;
static double maxReal = 1.0;
static double minImag = -1.0;
static double maxImag = 1.0;
const Sint16 maxIterations = 100;

static Uint16 y = 0;
static Uint16 x = 0;
static Uint8 done = 0;

static volatile Uint8 slavedone = 1;
static Uint16 slaveCpt = 0;


typedef struct parameter_slow{
  double real;
  double imag;
  Uint16 x;
  Uint16 y;
} parameter_slow;

typedef struct parameter_fixed{
  FIXED real;
  FIXED imag;
  Uint16 x;
  Uint16 y;
} parameter_fixed;


static volatile parameter_slow slave_param;
static volatile parameter_fixed slave_param_fixed;


// Function to check if a point is in the Mandelbrot set
int isInMandelbrotSlow(parameter_slow * param) {
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

// Function to check if a point is in the Mandelbrot set
int isInMandelbrot(parameter_fixed * param) {
  Sint16 iteration = 0;
  FIXED zReal = param->real;
  FIXED zImag = param->imag;

  while (iteration < maxIterations) {
    FIXED zRealTemp = zReal * zReal - zImag * zImag + param->real;
    zImag = 2 * zReal * zImag + param->imag;
    zReal = zRealTemp;

    if (zReal * zReal + zImag * zImag > 4.0) {
      return iteration;
    }

    ++iteration;
  }

  return maxIterations;
}

void SlaveTask_Slow(void * data)
{
  //char tmp[6];
  parameter_slow * param = (parameter_slow *)data;
  int iteration = isInMandelbrotSlow(param);
  slBMPset( param->x-(X_RESOLUTION>>1), param->y-(Y_RESOLUTION>>1), palette[iteration % 256] );
  //sprintf(tmp, "%d", slaveCpt++);
  //slPrint( tmp, slLocate( 12, 3 ) );
  slavedone = 1;
}

void SlaveTask(void * data)
{
  //char tmp[6];
  parameter_fixed * param = (parameter_fixed *)data;
  int iteration = isInMandelbrot(param);
  slBMPset( param->x-(X_RESOLUTION>>1), param->y-(Y_RESOLUTION>>1), palette[iteration % 256] );
  //sprintf(tmp, "%d", slaveCpt++);
  //slPrint( tmp, slLocate( 12, 3 ) );
  slavedone = 1;
}

void mandelbrot_fixed() {
  slavedone = 1;
  Uint32 timemax = TIM_FRT_MCR_TO_CNT(100000);
  TIM_FRT_SET_16(0);

  for (; y < height; y++) {
    for (; x < width; x++) {

      if(TIM_FRT_CNT_TO_MCR(TIM_FRT_GET_16()) > timemax) {
        return;
      }

      if(done) {
        return;
      }

      if (slavedone) {
        slavedone = 0;
        slave_param_fixed.real = minReal + x * (maxReal - minReal) / (width - 1);
        slave_param_fixed.imag = minImag + y * (maxImag - minImag) / (height - 1);
        slave_param.x = x;
        slave_param.y = y;
        slSlaveFunc(SlaveTask, (void *)(&slave_param_fixed));

      } else {
        parameter_fixed param;
        param.real = minReal + x * (maxReal - minReal) / (width - 1);
        param.imag = minImag + y * (maxImag - minImag) / (height - 1);
        int iteration = isInMandelbrot(&param);
        slBMPset( x-(X_RESOLUTION>>1), y-(Y_RESOLUTION>>1), palette[iteration % 256] );
      }
    }
    x= 0;
  }

  done=0;
}

void mandelbrot_slow() {
  slavedone = 1;
  Uint32 timemax = TIM_FRT_MCR_TO_CNT(100000);
  TIM_FRT_SET_16(0);

  for (; y < height; y++) {
    for (; x < width; x++) {

      if(TIM_FRT_CNT_TO_MCR(TIM_FRT_GET_16()) > timemax) {
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
}

#endif
