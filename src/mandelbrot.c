#include <sgl.h>
#include <sega_tim.h>

#include "video.h"
#include "pal.h"
#include "debug.h"
#include "mandelbrot.h"

#define NBG1_CEL_ADR ( VDP2_VRAM_A0 )
#define NBG1_MAP_ADR ( VDP2_VRAM_B0 )
#define NBG1_PAL_ADR ( VDP2_COLRAM )

#define LINE_COLOR_ADR          VDP2_VRAM_A1

#define	BACK_COL_ADR		( VDP2_VRAM_A1 + 0x1fffe )

#define	S2D_COLTYPE	COL_TYPE_256
#define	S2D_CHRSIZE	CHAR_SIZE_1x1
#define	S2D_PNBSIZE	PNB_1WORD

#define UNIT toFIXED( 1.0 );



// Function to check if a point is in the Mandelbrot set
int isInMandelbrot(double real, double imag, int maxIterations);
void mandelbrot();

void setPalette() {
  for (int i=0; i < 1024; ++i) {
    palette[i] =C_RGB(i, i * 2 % 1024, i * 4 % 1024);
  }
}


int main(void)
{

  FIXED  posX, posY;

  posX = toFIXED( 0.0 );
  posY = toFIXED( 0.0 );

  Sint16 i,j;

  i = -(X_RESOLUTION>>1);
  j = -(Y_RESOLUTION>>1);

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
  (void *)NBG1_MAP_ADR
);

if ( slInitBitMap( bmNBG1, BM_512x256, ( void * )NBG1_MAP_ADR ) == FALSE ) {
  slPrint( "FALSE", slLocate( 10, 3 ) );
  exit( 0 );
}

setPalette();

slDMACopy( palette, ( void * )NBG1_PAL_ADR, sizeof( palette ) );

slScrPosNbg1(posX, posY);
slScrTransparent(NBG1ON);
slScrAutoDisp(NBG0ON|NBG1ON);

slTVOn();

while( 1 ) {

  Uint16 pad = Smpc_Peripheral[ 0 ].data;

  if ( pad & PER_DGT_KU ) {
    posY -= UNIT;
  }
  if ( pad & PER_DGT_KD ) {
    posY += UNIT;
  }
  if ( pad & PER_DGT_KL ) {
    posX -= UNIT;
  }
  if ( pad & PER_DGT_KR ) {
    posX += UNIT;
  }


  mandelbrot();

  slSynch();
}
return 0;
}
