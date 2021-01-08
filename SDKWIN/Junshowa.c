// **************************************************************************
// Project : junior.prj
// File    : junshowa.c
// Object  : special effect show to user
// Author  : Kung-Pu Tang
// Date    : 821216
// **************************************************************************

#ifdef DOSLIB
  #include <fcntl.h>
  #include <sys\types.h>
  #include <sys\stat.h>
#else
  #include <windows.h>
#endif

#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <conio.h>
#include  "juniora.h"
#include  "externa.h"


PALETTEENTRY temp_palette[256];

void FAR PASCAL JUN_SEF_FadeInPal(int wSpeed)
{
   int  i, j, wTotal;
   BYTE bRedKey, bGreenKey, bBlueKey, fFinish=FALSE;

   /* save current palette to system_palette */
   JUN_get_ramdac_entry_6b(system_palette);
   /* set temp_palette to color_key */
   set_ramdac_address_read_reg(gConfig.bColorKey);
   get_ramdac_data_reg(&bRedKey, &bGreenKey, &bBlueKey);
   for ( i=0; i<MAXCOLORENTRY; i++ ) {
      temp_palette[i].peRed   = bRedKey;
      temp_palette[i].peGreen = bGreenKey;
      temp_palette[i].peBlue  = bBlueKey;
   }
   /* set ramdac to all black - 0 */
   JUN_init_ramdac_entry_6b(temp_palette);

   /* Fade palette in */
   while ( ! fFinish ) {
      wTotal = 0; 
      /* get current ramdac into temp_palette */
//      JUN_get_ramdac_entry_6b(temp_palette);
      for ( i=0; i<256; i++ ) {
         // Red color //
         if ( temp_palette[i].peRed < system_palette[i].peRed ) {
            temp_palette[i].peRed ++;
         }
         else {
            if ( temp_palette[i].peRed > system_palette[i].peRed ) 
               temp_palette[i].peRed --;
         }
         if ( temp_palette[i].peRed == system_palette[i].peRed )
                 wTotal++;
         // Green color //
         if ( temp_palette[i].peGreen < system_palette[i].peGreen ) {
            temp_palette[i].peGreen ++;
         }
         else {
            if ( temp_palette[i].peGreen > system_palette[i].peGreen ) 
               temp_palette[i].peGreen --;
         }
         if ( temp_palette[i].peGreen == system_palette[i].peGreen )
            wTotal++;
         // Blue color //
         if ( temp_palette[i].peBlue < system_palette[i].peBlue ) {
            temp_palette[i].peBlue ++;
         }
         else {
            if ( temp_palette[i].peBlue > system_palette[i].peBlue ) 
               temp_palette[i].peBlue --;
         }
         if ( temp_palette[i].peBlue == system_palette[i].peBlue )
            wTotal++;
      }
      delay(wSpeed*50);

      /* set new ramdac vaule */
      JUN_init_ramdac_entry_6b(temp_palette);
      if ( wTotal == 768 )
         fFinish = TRUE;
   } /* end of while */
} // end of JUN_SEF_FadeInPal

      
void FAR PASCAL JUN_SEF_FadeOutPal(int wSpeed)
{
   int  i, j, wTotal;
   BYTE bRedKey, bGreenKey, bBlueKey, fFinish=FALSE;

   /* save current palette to system_palette */
//   JUN_get_ramdac_entry_6b(system_palette);
   /* get current ramdac into temp_palette */
   JUN_get_ramdac_entry_6b(temp_palette);
   while ( ! fFinish ) {
      wTotal = 0; 
      /* get current ramdac into temp_palette */
//      JUN_get_ramdac_entry_6b(temp_palette);
      for ( i=0; i<256; i++ ) {
         // Red color //
         if ( temp_palette[i].peRed > 0 ) 
            temp_palette[i].peRed --;
         if ( temp_palette[i].peRed == 0 )
                 wTotal++;
         // Green color //
         if ( temp_palette[i].peGreen > 0 ) 
            temp_palette[i].peGreen --;
         if ( temp_palette[i].peGreen == 0 )
            wTotal++;
         // Blue color //
         if ( temp_palette[i].peBlue > 0 ) 
            temp_palette[i].peBlue --;
         if ( temp_palette[i].peBlue == 0 )
            wTotal++;
      }
      delay((WORD)wSpeed*50);

      /* set new ramdac value */
      JUN_init_ramdac_entry_6b(temp_palette);
      if ( wTotal == 768 )
         fFinish = TRUE;
   } /* end of while */

   /* flash to color key */
   JUN_flash_video_memory(gConfig.bColorKey);

} // end of JUN_SEF_FadeOutPal


//BYTE wColor1[3], wColor2[3];
void FAR PASCAL JUN_SEF_RotatePal(int wStartEntry, int wEndEntry,
                                  int wTimes, int wSpeed)
{
   int  i, j, k, wNumEntry;
   BYTE bColor1[3], bColor2[3];

   wNumEntry = wEndEntry - wStartEntry;
   for ( i=0; i<wTimes; i++ ) {
      /* 1. get the start entry RGB color and save to wColor1 */
      set_ramdac_address_read_reg(wStartEntry);
      get_ramdac_data_reg(&bColor1[0], &bColor1[1], &bColor1[2]);
      /* replace color with next entry */
      for ( j=0; j<wNumEntry; j++ ) {
         /* get the next entry RGB color and save to wColor2 */
         set_ramdac_address_read_reg(wStartEntry+j+1);
         get_ramdac_data_reg(&bColor2[0], &bColor2[1], &bColor2[2]);
         delay(wSpeed*50);

         /* set wColor2 to the current entry RGB color */
         set_ramdac_address_write_reg(wStartEntry+j);
         set_ramdac_data_reg(bColor2[0], bColor2[1], bColor2[2]);
      }
//      delay(wSpeed);
      set_ramdac_address_write_reg(wEndEntry);
      set_ramdac_data_reg(bColor1[0], bColor1[1], bColor1[2]);
      delay(wSpeed*50);

   }
}

WORD FAR SpeedToTime(WORD wSpeed)
{
   int  gap_table[8]={7,6,5,4,3,2,1,0};
   return(gap_table[wSpeed]);
}

/* delay CPU time to pause, uint:milisecond */
void FAR delay1(WORD wMiliSec)
{
   DWORD  dwTick1, dwTick2;

   if ( wMiliSec == 0 )  return;
   dwTick1 = GetTickCount();
   do {
      dwTick2 = GetTickCount();
   } while ( (WORD)(dwTick2-dwTick1) < wMiliSec );
}
void delay(int time_unit)   /* delay is in 1/10 seconds */
{
   int i, k;

/*
   k = time_unit;
   if ( time_unit > 0) 
      for (i = 0; i < k; i++) {
         while (!(inp(0x3da) & 0x08));
         while ((inp(0x3da) & 0x08));
      }
*/
   if ( time_unit == 0 ) return;
   for ( i=0; i<time_unit; i++ )
      for ( k=0; k<16384; k++ ) ;
} /* Wait */



int FAR PASCAL JUN_ShowImage(HDC hDC, HPALETTE hPAL,
			     UINT xVramLeft,  UINT yVramTop,
			     UINT xDataWidth, UINT yDataHeight,
			     UINT xDataLeft,  UINT yDataTop,
			     UINT xPageWidth, UINT yPageHeight,
                             UINT wEffectType,UINT wShowSpeed)
{
   int	       i, j, k, wReturnCode;
   BYTE huge   *hpBits;
   UINT        xExt, yExt, nCols;

   if ( ( xDataLeft+xDataWidth > VIDEO_MEMORY_WIDTH ) ||
        ( yDataTop+yDataHeight > VIDEO_MEMORY_HEIGHT ) ) {
      return(ERR_ACCESS_VRAM_OUT_OF_RANGE);
   }
   if ( wEffectType > 20)
      return(ERR_SPECIAL_EFFECT_NOT_SUPPORT);

   if ( (wReturnCode=GetImageData(hDC, xPageWidth, yPageHeight, glpPageBuf)) != TRUE )
      return(wReturnCode);
//   GetAndSetPalette(hDC, hPAL);

   if ( !( wShowSpeed <=7 ) )
      wShowSpeed = 0;

   hpBits = (LPSTR)glpPageBuf;  //global variable pointer to image buffer
//   hpBits += ( ((DWORD)yDataTop*((DWORD)xPageWidth+gwImgWidthPad)) + xDataLeft);

//   wsprintf(debugstr, "xVram=%d, yVram=%d, xLeft=%d, yTop=%d, xWidth=%d, yHeight=%d",
///		    xVramLeft, yVramTop, xDataLeft, yDataTop, xDataWidth, yDataHeight);
//   MessageBox(hWnd, "Write to Video memory!", debugstr, MB_OK | MB_ICONEXCLAMATION);

   switch ( wEffectType ) {

      case SEF_WRITE_BITMAP_DOWN:   //0 
           for ( i=0; i<yDataHeight; i++ ) {
              delay(SpeedToTime(wShowSpeed));
              outportb(0x80, i);
              CopyBuf2TV(xDataLeft, yDataTop+i,
                         xVramLeft, yVramTop+i,
                         xPageWidth, yPageHeight,
	                 xDataWidth);
           }
           break;

      case SEF_WRITE_BITMAP_UP:     //1
           for ( i=yDataHeight-1; i>=0; i-- ) {
              delay(SpeedToTime(wShowSpeed));
              outportb(0x80, i);
              CopyBuf2TV(xDataLeft, yDataTop+i,
                         xVramLeft, yVramTop+i,
                         xPageWidth, yPageHeight,
	                 xDataWidth);
           }
           break;

      case SEF_WRITE_BITMAP_RIGHT:   //2
           for ( i=0; i<xDataWidth; i++ ) {
              delay(SpeedToTime(wShowSpeed));
              outportb(0x80, i);
              for ( j=0; j<yDataHeight; j++ ) {
                 CopyBuf2TV(xDataLeft+i, yDataTop+j,
                            xVramLeft+i, yVramTop+j,
                            xPageWidth, yPageHeight,
	                    1);
              }
           }
           break;

      case SEF_WRITE_BITMAP_LEFT:   //3
           for ( i=xDataWidth-1; i>=0; i-- ) {
              delay(SpeedToTime(wShowSpeed));
              outportb(0x80, i);
              for ( j=0; j<yDataHeight; j++ ) {
                 CopyBuf2TV(xDataLeft+i, yDataTop+j,
                            xVramLeft+i, yVramTop+j,
                            xPageWidth, yPageHeight,
	                    1);
              }
           }
           break;

      case SEF_VERT_CROSS:          //4
           // even line get from bottom of Image and show at TV from top to bottom,
           // odd  line    "     top              "                  bottom to top.
           for ( i=0; i<yDataHeight; i+=2 ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=i; j>=0; j-=2 ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft, yDataTop+(int)(yDataHeight/2)*2-1-i+j,
                            xVramLeft, yVramTop+j,
                            xPageWidth, yPageHeight,
                            xDataWidth);
                 CopyBuf2TV(xDataLeft, yDataTop+(i-j+1),
                            xVramLeft, yVramTop+(int)(yDataHeight/2)*2-1-j,
                            xPageWidth, yPageHeight,
                            xDataWidth);
              }
           }
           break;

      case SEF_HORZ_CROSS:          //5
           for ( i=0; i<xDataWidth; i+=2 ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<yDataHeight; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft+xDataWidth-1-i, yDataTop+j,
                            xVramLeft+i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
                 CopyBuf2TV(xDataLeft+i, yDataTop+j,
                            xVramLeft+xDataWidth-1-i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
           }
           break;

      case SEF_JALOUSIE:            //6
           for ( i=0; i<4; i++ ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<yDataHeight/4; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft, yDataTop+(int)(yDataHeight/4)*i+j,
                            xVramLeft, yVramTop+(int)(yDataHeight/4)*i+j,
                            xPageWidth, yPageHeight,
                            xDataWidth);
              }
           }
           break;

      case SEF_STREAM_WATER:        //7
           for ( i=yDataHeight-1; i>=0; i-- ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<=i; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft, yDataTop+i,
                            xVramLeft, yVramTop+j,
                            xPageWidth, yPageHeight,
                            xDataWidth);
              }
           }
           break;

      case SEF_VERT_CRUSH:          //8
           for ( i=0; i<yDataHeight/2; i++ ) {
              delay(SpeedToTime(wShowSpeed));
              outportb(0x80, i);
              CopyBuf2TV(xDataLeft, yDataTop+i,
                         xVramLeft, yVramTop+i,
                         xPageWidth, yPageHeight,
                         xDataWidth);
              CopyBuf2TV(xDataLeft, yDataTop+yDataHeight-1-i,
                         xVramLeft, yVramTop+yDataHeight-1-i,
                         xPageWidth, yPageHeight,
                         xDataWidth);
           }
           break;

      case SEF_VERT_REVEAL:         //9
           for ( i=0; i<yDataHeight/2; i++ ) {
              delay(SpeedToTime(wShowSpeed));
              outportb(0x80, i);
              CopyBuf2TV(xDataLeft, yDataTop+(yDataHeight/2)-1+i,
                         xVramLeft, yVramTop+(yDataHeight/2)-1+i,
                         xPageWidth, yPageHeight,
                         xDataWidth);
              CopyBuf2TV(xDataLeft, yDataTop+(yDataHeight/2)-1-i,
                         xVramLeft, yVramTop+(yDataHeight/2)-1-i,
                         xPageWidth, yPageHeight,
                         xDataWidth);
           }
           break;

      case SEF_HORZ_CRUSH:          //10
           for ( i=0; i<xDataWidth/2; i++ ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<yDataHeight; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft+i, yDataTop+j,
                            xVramLeft+i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
                 CopyBuf2TV(xDataLeft+xDataWidth-1+i, yDataTop+j,
                            xVramLeft+xDataWidth-1+i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
           }
           break;

      case SEF_HORZ_REVEAL:         //11
           for ( i=0; i<xDataWidth/2; i++ ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<yDataHeight; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft+(xDataWidth/2)-1-i, yDataTop+j,
                            xVramLeft+(xDataWidth/2)-1-i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
                 CopyBuf2TV(xDataLeft+(xDataWidth/2)-1+i, yDataTop+j,
                            xVramLeft+(xDataWidth/2)-1+i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
           }
           break;

      case SEF_PULL_UP:             //12
           for ( i=yDataHeight-1; i>=0; i-- ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<=yDataHeight-1-i; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft, yDataTop+j,
                            xVramLeft, yVramTop+i+j,
                            xPageWidth, yPageHeight,
                            xDataWidth);
              }
           }
           break;

      case SEF_PULL_DOWN:           //13
           for ( i=yDataHeight-1; i>=0; i-- ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<=yDataHeight-1-i; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft, yDataTop+i+j,
                            xVramLeft, yVramTop+j,
                            xPageWidth, yPageHeight,
                            xDataWidth);
              }
           }
           break;

      case SEF_PULL_RIGHT:          //14
           nCols = 4;    // 4 cloumns/times
           for ( i=0; i<(xDataWidth/nCols); i+=nCols ) {
//              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<=i; j++ ) {
                 outportb(0x80, i);
                 //write 1 column
                 for ( k=0; k<yDataHeight; k++ )
                    CopyBuf2TV(xDataLeft+j*nCols, yDataTop+k,
                               xVramLeft+j*nCols, yVramTop+k,
                               xPageWidth,        yPageHeight,
                               nCols);
              }
           }
           break;

      case SEF_PULL_LEFT:           //15
           nCols = 4;    // 4 cloumns/times
           for ( i=0; i<(xDataWidth/nCols); i++ ) {
//              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<=xDataWidth-1-i; j++ ) {
                 outportb(0x80, i);
                 //write 1 column
                 for ( k=0; k<yDataHeight; k++ )
                    CopyBuf2TV(xDataLeft+i+j, yDataTop+k,
                               xVramLeft+j, yVramTop+k,
                               xPageWidth, yPageHeight,
                               nCols);
              }
           }
           break;

      case SEF_SHRINK:              //16
           for ( k=0; k<min(xDataWidth/2,yDataHeight/2); k++ ) {
              delay(SpeedToTime(wShowSpeed));
              //write top_most & bottom_most lines
              for ( i=k; i<max(xDataWidth,yDataHeight)-k; i++ ) {
                 CopyBuf2TV(xDataLeft+i, yDataTop+k,
                            xVramLeft+i, yVramTop+k,
                            xPageWidth, yPageHeight,
                            1);
                 CopyBuf2TV(xDataLeft+i, yDataTop+yDataHeight-1-k,
                            xVramLeft+i, yVramTop+yDataHeight-1-k,
                            xPageWidth, yPageHeight,
                            1);
              }
              //write left_most & right_most lines
              for ( j=k+1; j<min(xDataWidth,yDataHeight)-k; j++ ) {
                 CopyBuf2TV(xDataLeft+k, yDataTop+j,
                            xVramLeft+k, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
                 CopyBuf2TV(xDataLeft+xDataWidth-1-k, yDataTop+j,
                            xVramLeft+xDataWidth-1-k, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
           }
           break;

      case SEF_EXPLOAD:             //17
           for ( k=min(xDataWidth/2,yDataHeight/2)-1; k>=0; k-- ) {
              delay(SpeedToTime(wShowSpeed));
              //write top_most & bottom_most lines
              for ( i=max(xDataWidth,yDataHeight)-1-k; i>=k; i-- ) {
                 CopyBuf2TV(xDataLeft+i, yDataTop+k,
                            xVramLeft+i, yVramTop+k,
                            xPageWidth, yPageHeight,
                            1);
                 CopyBuf2TV(xDataLeft+i, yDataTop+yDataHeight-1-k,
                            xVramLeft+i, yVramTop+yDataHeight-1-k,
                            xPageWidth, yPageHeight,
                            1);
              }
              //write left_most & right_most lines
              for ( j=min(xDataWidth,yDataHeight)-1-k; j>=k; j-- ) {
                 CopyBuf2TV(xDataLeft+k, yDataTop+j,
                            xVramLeft+k, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
                 CopyBuf2TV(xDataLeft+xDataWidth-1-k, yDataTop+j,
                            xVramLeft+xDataWidth-1-k, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
           }
           break;

      case SEF_SCAN:                //18
           // even column show from left to right
           for ( i=0; i<xDataWidth; i+=2 ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<yDataHeight; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft+i, yDataTop+j,
                            xVramLeft+i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
           }
           // odd column show from right to left
           for ( i=(xDataWidth/2)*2-1; i>0; i-=2 ) {
              delay(SpeedToTime(wShowSpeed));
              for ( j=0; j<yDataHeight; j++ ) {
                 outportb(0x80, i);
                 CopyBuf2TV(xDataLeft+i, yDataTop+j,
                            xVramLeft+i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
           }
           break;

      case SEF_SPIRAL:              //19
           xExt = xDataWidth;
           yExt = yDataHeight;
           for ( k=0; k<min(xDataWidth/2,yDataHeight/2); k++ ) {
//              delay(SpeedToTime(wShowSpeed));
              //write 1 line
              for ( i=k; i<xExt; i++ ) {
                 CopyBuf2TV(xDataLeft+i, yDataTop+k,
                            xVramLeft+i, yVramTop+k,
                            xPageWidth, yPageHeight,
                            1);
              }
              //write 1 column
              for ( j=k+1; j<yExt; j++ ) {
                 CopyBuf2TV(xDataLeft+xDataWidth-1-k, yDataTop+j,
                            xVramLeft+xDataWidth-1-k, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
              //write 1 line
              for ( i=xExt; i>k; i-- ) {
                 CopyBuf2TV(xDataLeft+i, yDataTop+j-1,
                            xVramLeft+i, yVramTop+j-1,
                            xPageWidth, yPageHeight,
                            1);
              }
              //write 1 column
              for ( j=yExt-1; j>k; j-- ) {
                 CopyBuf2TV(xDataLeft+i, yDataTop+j,
                            xVramLeft+i, yVramTop+j,
                            xPageWidth, yPageHeight,
                            1);
              }
              xExt -= 1;
              yExt -= 1;
           }
           break;
           
   } // end of switch //
   return(TRUE);
}


/****************************************************************************
* CopyBuf2TV(xBufLeft, yBufTop, xTVLeft, yTVTop, wSize)
*
* Exported:  No
* Entry:     A specified rect.- (x,y,width,height), buffer, and operation type
* Destroys:
* Exit:      TRUE for success, FALSE for failure
****************************************************************************/
int FAR  CopyBuf2TV(UINT xDataLeft,  UINT yDataTop,  //(left,top) of Image
		    UINT xTVLeft,    UINT yTVTop,   //(left,top) of TV
                    UINT xPageWidth, UINT yPageHeight,
		    UINT wSize)		          //n bytes
{
   BYTE   memory_segment, bank_no;
   UINT   start_line, rel_x, rel_y;
   DWORD  dwSegBoundary;
   BYTE huge * hpImageBuf;
   LPSTR  lpVideo;
   UINT   i, j;

   if ( wSize > VIDEO_MEMORY_WIDTH || wSize > VIDEO_MEMORY_HEIGHT ) {
//	MessageBox(hWnd, "Out of range !", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }

   /* 11/20/93 Tang added */
   if ( wSize == 0 )
      return(TRUE);
   /* 11/20/93 Tang added */

   /* get relative x location */
   rel_x = fnRelX(xTVLeft);

   /* convert to huge pointer */
   hpImageBuf = glpPageBuf +
                ( ((DWORD)yDataTop*((DWORD)xPageWidth+gwImgWidthPad)) + xDataLeft);

//   for ( y=yTVTop; y<yTVTop+wSize; y++) {     //Process one line per times
      /* get relative y location */
      rel_y = fnRelY(yTVTop);
      /* (xLeft, y) locate at which segment */
      memory_segment = (BYTE)(rel_y >> 6);  // div 64 - segment no.
      /* start position in a segment */
      start_line = (UINT)(rel_y) % LINES_PER_SEGMENT;

      // select the desired memory segment
      /*12/30
      if ( memory_segment != prev_memory_segment ) { // not previous memory segment
	 memory_seg_select(memory_segment);
	 prev_memory_segment = memory_segment; // preserve this value
      }
      12/30*/
      if ( memory_segment != gbPreMemSeg) { // not previous memory segment
	 memory_seg_select(memory_segment);
	 gbPreMemSeg = memory_segment; // preserve this value
      }
      /* 12/06/93 Tang added to support 8k/16k bank process */
      if ( gbBankType != BANK_SIZE_64K ) {
         bank_no = (start_line >> ( 5 - gbBankType)); //16K(1:4),8K(2:3)
         /*12/30
         if ( bank_no != prev_bank_no ) {
            select_bank_segment(gbBankType, bank_no);
            prev_bank_no = bank_no;
         }
         12/30*/
         if ( bank_no != gbPreBnkNo ) {
            select_bank_segment(gbBankType, bank_no);
            gbPreBnkNo = bank_no;
         }
         start_line = (UINT) (start_line % (16 >> (gbBankType - 1)) );
      }
      /* 12/06/93 Tang added to support 8k/16k bank process */

      i = 0;

//	lpVideo = (LPSTR)glpVideoMemory + (start_line<<10 + xLeft);
      lpVideo = GetVideoAddress(rel_x, start_line);
      xLength[i++] = wSize;
      xLength[i--] = 0;
      if ( ( rel_x + wSize ) > VIDEO_MEMORY_WIDTH ) {
	 xLength[i++] = VIDEO_MEMORY_WIDTH - rel_x;
	 xLength[i] = wSize - xLength[i-1];
      }

      for ( j=0; j<=i; j++) {
         if ( j > 0 )
	    lpVideo = GetVideoAddress(0, start_line);
	 /* byte left in 64K segment */
	 dwSegBoundary = (0xFFFF ^ LOWORD(hpImageBuf)) + 1;
	 if (dwSegBoundary >= (DWORD)xLength[j]) {
	    FarMemoryCopy((DWORD)hpImageBuf, (DWORD)lpVideo, xLength[j]);
	    hpImageBuf += xLength[j];
	 }
	 /* take care of scan line on 64K segment */
	 else {
	    FarMemoryCopy((DWORD)hpImageBuf, (DWORD)lpVideo, (WORD)dwSegBoundary);
	    hpImageBuf += dwSegBoundary;
	    FarMemoryCopy((DWORD)hpImageBuf, (DWORD)lpVideo+dwSegBoundary,
			   xLength[j]-(WORD)dwSegBoundary);
	    hpImageBuf += (xLength[j] - (WORD)dwSegBoundary);
	 }
      }
//   } /* end of for */
   return(TRUE);
} /* end of CopyBuf2TV() */


// end of junshowa.c //

