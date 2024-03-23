
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

// Function to check if a point is in the Mandelbrot set
int isInMandelbrot(double real, double imag, int maxIterations) {
  Sint16 iteration = 0;
  double zReal = real;
  double zImag = imag;

  while (iteration < maxIterations) {
    double zRealTemp = zReal * zReal - zImag * zImag + real;
    zImag = 2 * zReal * zImag + imag;
    zReal = zRealTemp;

    if (zReal * zReal + zImag * zImag > 4.0) {
      return iteration;
    }

    iteration++;
  }

  return maxIterations;
}

void mandelbrot() {

  Uint32 timemax = TIM_FRT_MCR_TO_CNT(100000);
  TIM_FRT_SET_16(0);


  for (; y < height; y++) {
    for (; x < width; x++) {
      double real = minReal + x * (maxReal - minReal) / (width - 1);
      double imag = minImag + y * (maxImag - minImag) / (height - 1);

      if(TIM_FRT_CNT_TO_MCR(TIM_FRT_GET_16()) > timemax) {
        return;
      }

      if(done) {
        return;
      }

      int iteration = isInMandelbrot(real, imag, maxIterations);

      // Use the iteration count to index into the palette
      //RGBColor color = palette[iteration % 256];

      slBMPset( x-(X_RESOLUTION>>1), y-(Y_RESOLUTION>>1), palette[iteration % 256] );

      // Convert to grayscale for simplicity
      //unsigned char gray = (unsigned char)(0.3 * color.r + 0.59 * color.g + 0.11 * color.b);

      // Print an ASCII character representing the color
      //printf("\033[48;2;%d;%d;%dm ", color.r, color.g, color.b);
    }
    x= 0;
  }

  done=0;
}

#endif
