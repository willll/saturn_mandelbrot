#include <sgl.h>
#include <sega_tim.h>

#include <stdio.h>

#include "video.h"
#include "pal.h"
#include "debug.h"
#include "mandelbrot.h"

/**
 * @file mandelbrot.c
 * @brief Saturn setup and Mandelbrot render loop entry point.
 */

/** Character pattern table base address for NBG1. */
#define NBG1_CEL_ADR ( VDP2_VRAM_A0 )
/** Bitmap/map data base address for NBG1. */
#define NBG1_MAP_ADR ( VDP2_VRAM_B0 )
/** Color RAM base address used for the active palette. */
#define NBG1_PAL_ADR ( VDP2_COLRAM )

/** VRAM address used by line color effects (reserved). */
#define LINE_COLOR_ADR          VDP2_VRAM_A1

/** Back screen color address. */
#define	BACK_COL_ADR		( VDP2_VRAM_A1 + 0x1fffe )

/** NBG1 color mode (256 colors). */
#define	S2D_COLTYPE	COL_TYPE_256
/** NBG1 character size (1x1 cells). */
#define	S2D_CHRSIZE	CHAR_SIZE_1x1
/** NBG1 pattern name data width (1 word). */
#define	S2D_PNBSIZE	PNB_1WORD

/** Generic unit constant. */
#define UNIT 1


// Function to check if a point is in the Mandelbrot set
//int isInMandelbrot(double real, double imag, int maxIterations);
//void mandelbrot();

/**
 * @brief Initializes the runtime palette gradient.
 */
void setPalette() {
  for (int i=0; i < 256; ++i) {
    palette[i] =C_RGB(i, i * 2 % 256, i * 4 % 256);
  }
  palette[255] = C_RGB(255, 255, 255);
}

/**
 * @brief Program entry point.
 * @return Never returns during normal execution.
 */
int main(void)
{
  FIXED  posX, posY;

  debug_print("trace: main start");

  posX = toFIXED( 0.0 );
  posY = toFIXED( 0.0 );

  slInitSystem( TV_320x224, NULL, 1 ); // TV_704x480

  TIM_FRT_INIT(TIM_CKS_128);

  slTVOff();

  slColRAMMode( CRM16_1024 ); // CRM16_2048
  slBack1ColSet((void *)BACK_COL_ADR, C_RGB(0,0,0));

  slCharNbg1(S2D_COLTYPE, S2D_CHRSIZE);
  slPageNbg1((void *)NBG1_CEL_ADR, 0, S2D_PNBSIZE);
  slPlaneNbg1(PL_SIZE_1x1);

  slMapNbg1(	(void *)NBG1_MAP_ADR,
              (void *)NBG1_MAP_ADR,
              (void *)NBG1_MAP_ADR,
              (void *)NBG1_MAP_ADR );

if ( slInitBitMap( bmNBG1, BM_512x256, ( void * )NBG1_MAP_ADR ) == FALSE ) {
  debug_print("trace: slInitBitMap failed");
  slPrint( "FALSE", slLocate( 10, 3 ) );
  SYS_Exit(0);
}

setPalette();

slDMACopy( palette, ( void * )NBG1_PAL_ADR, sizeof( palette ) );

slScrPosNbg1(posX, posY);
slScrTransparent(NBG1ON);
slScrAutoDisp( NBG0ON| NBG1ON| NBG2OFF| NBG3OFF);

slTVOn();
debug_print("trace: video on, entering render loop");

while( 1 ) {

  mandelbrot_slow();

  slSynch();
}
return 0;
}
