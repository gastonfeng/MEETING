
/************************************************************************
* Project : junior.dll
* Module  : junscol.c
* Object  : handle scrolling
* Author  : Kung-Puu Tang
* Date	  : 821110
************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
//#include <io.h>
#include <dos.h>           //enable(),disable(),getvect(),setvect()
//#include <time.h>          //time()
//#include <sys\types.h>     // "    ,time_t
#include "juniora.h"
#include "externa.h"


/************************************************************************
* JUN_ScrollBitmap
*
*   Scroll input page continue.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
int FAR PASCAL JUN_ScrollBitmap(HDC hDC,         HPALETTE hPAL,
                                UINT xVramLeft,  UINT yVramTop,
                                UINT xDataWidth, UINT yDataHeight,
                                UINT xDataLeft,  UINT yDataTop,
                                UINT xPageWidth, UINT yPageHeight,
			        UINT wType,      UINT wSpeed)
{
   HBITMAP      hBitmap, hOldBitmap;
   BITMAP	Bitmap;
   HANDLE       hMem;
   LPSTR	lpBits;
   DWORD	dwDataSize, dwNumBytes, dwSegBoundary;
   int		wNumColors, reg_temp_start;
   int          wVramStart, temp_vram_start;
   UINT         i, j, wRegGaps, padded;
   BYTE huge    *hpBits;       // local huge pointer to image bits
   ULONG        lSize;
   UINT         xVmemLeft, yVmemTop, wPieceCount, xChangeLeft, yChangeTop;
   BOOL         fScroll;
   UINT         vert_scan_start, horz_scan_start;

   if ( wType > 4 )  return(ERR_SCROLL_TYPE_NOT_SUPPORT);

   if ( wSpeed < 1 || wSpeed > 4 )  return(ERR_SCROLL_SPEED_NOT_ALLOW);

//   wsprintf(debugstr, "hDC=%4d, PageWidth=%4d, PageHeight=%4d", hDC, xPageWidth, yPageHeight);
//   MessageBox(NULL, "Create Bitmap", debugstr, MB_OK | MB_ICONEXCLAMATION);
   if ( hDC ) {
      hBitmap = CreateCompatibleBitmap(hDC, xDataWidth, yDataHeight);
      if ( !hBitmap ) {
         wsprintf(debugstr, "hBitmap=%4d", hBitmap);
         MessageBox(NULL, "Create Bitmap Error!", debugstr, MB_OK | MB_ICONEXCLAMATION);
         outportb(0x80, 0x04);
         return(FALSE);
      }

//   MessageBox(NULL, "Select Bitmap ", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      hOldBitmap = SelectObject(hDC, hBitmap);
      if ( !hOldBitmap ) {
         MessageBox(NULL, "Select Bitmap Error!", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
         outportb(0x80, 0x05);
         DeleteObject(hBitmap);
         return(FALSE);
      }

//      MessageBox(NULL, "Get Object", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      if ( !GetObject(hOldBitmap, sizeof(BITMAP), (LPSTR)&Bitmap) ) {
         MessageBox(NULL, "Get Object Error!", "JUNIOR",
                    MB_OK | MB_ICONEXCLAMATION);
         outportb(0x80, 0x06);
         SelectObject(hDC, hOldBitmap);
         DeleteObject(hBitmap);
         return(FALSE);
      }
   }
   else {
      Bitmap.bmType = 0;
      Bitmap.bmWidth  = VGA_PAGE_WIDTH;
      Bitmap.bmHeight = VGA_PAGE_HEIGHT;
      Bitmap.bmWidthBytes = VGA_PAGE_WIDTH;
      Bitmap.bmPlanes =
      Bitmap.bmBitsPixel = 0;
      Bitmap.bmBits = 0L;
   }


//   wsprintf(debugstr, "width=%4d, height=%4d", Bitmap.bmWidth, Bitmap.bmHeight);
//   MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);

   if ( padded=(Bitmap.bmWidth%2) )
     padded = 2 - padded;

   /* insure to make the width is a multiple of 2 */
   dwDataSize = (DWORD)(Bitmap.bmWidth+padded) * Bitmap.bmHeight;
   if ( ! CompactGlobalHeap(dwDataSize) ) {
      outportb(0x80, 0x87);
      gwErrorCode = ERR_INSUFFICIENT_MEMORY;
      return(gwErrorCode);
   }
   hMem = GlobalAlloc(GHND, dwDataSize);
   if ( ! hMem ) {
      outportb(0x80, 0x77);
      wsprintf(debugstr, "Request Memory %l", dwDataSize);
      MessageBox(NULL, "Alloc Memory Error!", debugstr, MB_OK | MB_ICONEXCLAMATION);
      SelectObject(hDC, hOldBitmap);
      DeleteObject(hBitmap);
      return(FALSE);
   }
   lpBits = GlobalLock(hMem);

   outportb(0x80, 0x55);
//   MessageBox(NULL, "Get Bitmap", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   if ( hDC )
      dwNumBytes = GetBitmapBits(hOldBitmap, dwDataSize, lpBits);
   else {
      hpBits = lpBits;
      dwNumBytes = 0;
      for ( i=0; i<Bitmap.bmHeight; i++ ) {
         /* byte left in 64K segment */
	 dwSegBoundary = (0xFFFF ^ LOWORD(hpBits)) + 1;
	 if (dwSegBoundary >= (DWORD)Bitmap.bmWidth)
            for ( j=0; j<Bitmap.bmWidth; j++ ) {
               *hpBits = gConfig.bColorKey;
               hpBits += 1;
               dwNumBytes++;
            }
         /* take care of scan line on 64K segment */
	 else {
            for ( j=0; j<(WORD)dwSegBoundary; j++ ) {
               *hpBits = gConfig.bColorKey;
               hpBits += 1;
               dwNumBytes++;
            }
            for ( j=0; j<Bitmap.bmWidth-(WORD)dwSegBoundary; j++ ) {
               *hpBits = gConfig.bColorKey;
               hpBits += 1;
               dwNumBytes++;
            }
	 }
      }
   } /* end of else */

   if ( ! dwNumBytes ) {
      outportb(0x80, 0x66);
      MessageBox(NULL, "Get Bitmap Bits Error!", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      SelectObject(hDC, hOldBitmap);
      DeleteObject(hBitmap);
      GlobalUnlock(hMem);
      GlobalFree(hMem);
      return(FALSE);
   }

   outportb(0x80, 0x56);

   if ( ! hPAL ) {
//     wsprintf(debugstr," palette handle %d", hPAL);
//     MessageBox(NULL,"Palette Handle",
//                debugstr, MB_OK | MB_ICONEXCLAMATION);
     // 11/16/93 Tang added
      wNumColors = GetSystemPaletteEntries(hDC, 0,
     		            GetDeviceCaps(hDC, SIZEPALETTE), pe);
   }
   else {
//     wsprintf(debugstr," palette handle %d", hPAL);
//     MessageBox(NULL,"Palette Handle",
//	      	 debugstr, MB_OK | MB_ICONEXCLAMATION);
      GetSystemPaletteEntries(hDC, 0,
	              GetDeviceCaps(hDC, SIZEPALETTE), pe);
      wNumColors = GetPaletteEntries(hPAL, 10, MAXCOLORENTRY-20, pe);
//     wsprintf(debugstr," entries %d", wNumColors);
//     MessageBox(NULL,"Get Palette",
//	    	     debugstr, MB_OK | MB_ICONEXCLAMATION);

   }

//   MessageBox(NULL, "Set RAMDAC", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   // 11/16/93 Tang added
//   JUN_init_ramdac_entry(pe);
//   ReversePalette(pe);

//   MessageBox(NULL, "Write to video memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   /* delete bitmap object */
   if ( hDC ) {
      SelectObject(hDC, hOldBitmap);
      DeleteObject(hBitmap);
   }
   else {
      xVramLeft = 0;
      yVramTop  = 0;
      xDataWidth  = Bitmap.bmWidth;
      yDataHeight = Bitmap.bmHeight;
      xDataLeft = 0;
      yDataTop  = 0;
      xPageWidth  = Bitmap.bmWidth;
      yPageHeight = Bitmap.bmHeight;
   }

   wRegGaps = wSpeed;	 //Increment of register value
   wPieceCount = 0;   //Count for Image height

   /* get the start address for the scrolling */
   hpBits = lpBits;
   /* prepare scrolling important variable */
   xVmemLeft = xVramLeft;
   yVmemTop  = yVramTop;
   switch ( wType ) {
      case SCROLL_UP :
           /* start x & y position in video meory */
//           xVmemLeft = xVramLeft;         //all  x pos. in Vmem
           /* 11/25/93 Tang, y start must relative to yVramTop */
           wVramStart = yVramTop + gwVideoHeight;  //each y pos. in Vmem
           yChangeTop = 0;
           /* start register value */
           reg_temp_start = gConfig.wVertScanStart;
           break;

      case SCROLL_DOWN :
           /* start x & y position in video meory */
//           xVmemLeft = xVramLeft;      //all  x pos. in Vmem
           yChangeTop = Bitmap.bmHeight - 1;
           wVramStart = (int)(VIDEO_MEMORY_HEIGHT - 1);  //each y pos. in Vmem
           /* start register value */
           reg_temp_start = gConfig.wVertScanStart;
           break;

      case SCROLL_LEFT :
           /* start x & y position in video meory */
           /* 11/25/93 Tang, x start must relative to xVramLeft */
           wVramStart = xVramLeft + gwVideoWidth; //each y pos. in Vmem
//           wVramStart = Bitmap.bmWidth; //each y pos. in Vmem
//           yVmemTop = yVramTop;         //all  y pos. in Vmem
           xChangeLeft = 0;
           /* start register value */
           reg_temp_start = gConfig.wHorzScanStart;
           break;

      case SCROLL_RIGHT :
           /* start x & y position in video meory */
           wVramStart = (int)(VIDEO_MEMORY_WIDTH - 1);  //each y pos. in Vmem
//           yVmemTop = yVramTop;        //all  y pos. in Vmem
           xChangeLeft = Bitmap.bmWidth - 1;
           /* start register value */
           reg_temp_start = gConfig.wHorzScanStart;
           break;

      case SCROLL_NONE :
           xVmemLeft = xVramLeft;     //all  x pos. in Vmem
           wVramStart = 0;            //each y pos. in Vmem
           yChangeTop = 0;
           break;
   } /* end of switch */

   fScroll = TRUE;

   /* really perform scrolling */
   switch ( wType ) {
      case SCROLL_UP:   // Scroll the bitmap from bottom to top
           while ( fScroll ) {
	      /* Step 1. Scrolling bitmap into video memory */
	      /* write proper lines of bitmap image into video memory */
	      if ( wPieceCount < (gwVideoHeight + yVmemTop) ) {
	         for ( i=0; i<wRegGaps*2; i++ ) {
	            if ( wVramStart >= VIDEO_MEMORY_HEIGHT )
		       wVramStart = 0;	//Get Unused area
                    if ( wPieceCount < yVmemTop )
                      //->usually 30 (0-29)
		       /* write color-key into video memory */
		       JUN_ClearImageRect(xVmemLeft, (UINT)wVramStart,
                               xDataWidth, 1,
                               gConfig.bColorKey);
                    else {
	               if ( (wPieceCount - yVmemTop) < Bitmap.bmHeight ) {
                          outportb(0x80,(BYTE)gwPieceCount);
			  HandleImageRect1(xVmemLeft, (UINT)wVramStart,
                               xDataWidth, 1,
                               xDataLeft, yChangeTop,
                               xPageWidth, yPageHeight,
                               (LPSTR)hpBits, H_WRITE);
		       }
	               else {
		          /* write color-key into video memory */
		          JUN_ClearImageRect(xVmemLeft, (UINT)wVramStart,
                                xDataWidth, 1, gConfig.bColorKey);
	               }
                       yChangeTop++;
                    }
	            wVramStart++;
	            wPieceCount++;
	         }  /* end of for */
              }

              /* Step 2. write to scroll register */
	      reg_temp_start += wRegGaps;
	      if ( reg_temp_start > 0xff )
	         reg_temp_start &= 0x00ff;
              /* set vert scan start reg. */
	      vert_scan_start = (BYTE)reg_temp_start;
	      wait_vert_retrace_start();
	      set_vert_scroll_reg(vert_scan_start);

              /* End of scrolling ? Yes, set flag */
	      if ( wPieceCount >= (gwVideoHeight + yVmemTop) ) {
	         fScroll = FALSE;
              }

	   } /* end of while (fScroll)*/

           /* restore to original status */
           gConfig.wVertScanStart = (WORD)(vert_scan_start);
           break;

      case SCROLL_DOWN :   // Scroll the bitmap from top to bottom
           while ( fScroll ) {
	      /* Step 1. Scrolling bitmap into video memory */
	      /* write proper lines of bitmap image into video memory */
	      if ( wPieceCount < gwVideoHeight ) {
	         for ( i=0; i<wRegGaps*2; i++ ) {
	            if ( wVramStart < 0 )
		       wVramStart = VIDEO_MEMORY_HEIGHT - 1;  //Get Unused area
	            if ( wPieceCount < (gwVideoHeight - yPageHeight) ) {
		       /* write color-key into video memory */
		       JUN_WriteImageRect(xVmemLeft, wVramStart,
                                xDataWidth, 1, (LPSTR)hpBits);
	            }
	            else {
		       if ( wPieceCount < gwVideoHeight ) { //65-479=415
                          outportb(0x80,(BYTE)wPieceCount);
			  HandleImageRect1(xVmemLeft, (UINT)wVramStart,
                               xDataWidth, 1,
                               xDataLeft,  yChangeTop,
                               xPageWidth, yPageHeight,
                               (LPSTR)hpBits, H_WRITE);
                       }
		       else {
			  /* write color-key into video memory */
			  JUN_ClearImageRect(xVmemLeft, (UINT)wVramStart,
                               xDataWidth, 1, gConfig.bColorKey);
		       }
                       yChangeTop--;
	            }
	            wVramStart--;
	            wPieceCount++;
	         } /* end of for */
              }
              else {  // 11/25/93 Tang added for compensate
                 if ( (wPieceCount-gwVideoHeight) <= yVmemTop ) 
		    for ( i=0; i<wRegGaps*2; i++ ) {
                       /* reset proper start line to put image data into */
		       if ( wVramStart< 0 )
			  wVramStart= (int)(VIDEO_MEMORY_HEIGHT - 1);
		       /* write color-key into video memory */
		       JUN_ClearImageRect(xVmemLeft, wVramStart,
                                xDataWidth, 1, gConfig.bColorKey);
		       wVramStart--;
		       wPieceCount++;
		    } /* end of for */
              }  // 11/25/93 Tang added for compensate

              /* Step 2. write to scroll register */
	      reg_temp_start -= wRegGaps;
	      if ( reg_temp_start < 0 )
	         reg_temp_start &= 0x00ff;
	      vert_scan_start = (BYTE)reg_temp_start;
	      wait_vert_retrace_start();
	      set_vert_scroll_reg(vert_scan_start);

              /* End of scrolling ? Yes, set flag */
	      if ( wPieceCount >= (gwVideoHeight + yVmemTop) ) 
	         fScroll = FALSE;
	   } /* end of while (fScroll) */

           /* restore to original status */
           gConfig.wVertScanStart = ((WORD)(vert_scan_start) & 0x00ff);
           break;

      case SCROLL_LEFT :
           while ( fScroll ) {
	      /* Step 1. Shifting bitmap into video memory */
	      /* write proper lines of bitmap image into video memory */
	      if ( wPieceCount < (gwVideoWidth + xVmemLeft) ) {
	         for ( i=0; i<wRegGaps*2; i++ ) {
	            if ( wVramStart >= VIDEO_MEMORY_WIDTH )
		       wVramStart = 0;	//Get Unused area
                    //11/30/93 Tang, write yVramTop lines color key first
                    if ( wPieceCount < xVmemLeft )
                       //->usually 30 (0-29)
		       /* write color-key into video memory */
		       JUN_ClearImageRect((UINT)wVramStart, yVmemTop,
                                1, yDataHeight, gConfig.bColorKey);
                    else {
	               if ( (wPieceCount - xVmemLeft) < Bitmap.bmWidth ) {
                          outportb(0x80,(BYTE)wPieceCount);
		          HandleImageRect1(wVramStart, yVmemTop,
                                   1, yDataHeight,
                                   xChangeLeft, yDataTop,
                                   xPageWidth, yPageHeight,
                                   (LPSTR)hpBits, H_WRITE);
	               }
	               else {
		          /* write color-key into video memory */
		          JUN_ClearImageRect(wVramStart, yVmemTop,
                                1, yDataHeight, gConfig.bColorKey);
	               }
                       xChangeLeft++;
                    }
	            wVramStart++;
	            wPieceCount++;
	         } /* end of for */
              }

              /* Step 2. write to shift register */
	      reg_temp_start += wRegGaps;
              /* 11/26/93 Tang updated */
              reg_temp_start %= 0x200;
	      wait_vert_retrace_start();
//	      set_horz_shift_reg(reg_temp_start);
	      set_horz_shift_reg1(reg_temp_start);
/*
              if ( reg_temp_start > 0x1ff ) {
                 // overflow 9 bits' value, reset to 0
                 set_horz_shift_reg_msb(0);
	         reg_temp_start = 0;
              }
	      else if ( reg_temp_start > 0xff ) {
                      // 9th bit is 0, set LSB
                      set_horz_shift_reg_msb(1);
                   }
	      horz_scan_start = (BYTE)reg_temp_start;
	      wait_vert_retrace_start();
	      set_horz_shift_reg(horz_scan_start);
*/
              /* 11/26/93 Tang updated */

              /* End of scrolling ? Yes, set flag */
	      if ( wPieceCount >= (gwVideoWidth + xVmemLeft) )
	         fScroll = FALSE;
	   } /* end of while (fScroll) */

           /* restore to original status */
           gConfig.wHorzScanStart = reg_temp_start;
           break;

      case SCROLL_RIGHT :
           while ( fScroll ) {
	      /* Step 1. Shifting bitmap into video memory */
	      /* write proper lines of bitmap image into video memory */
	      if ( wPieceCount < gwVideoWidth ) {
	         for ( i=0; i<wRegGaps*2; i++ ) {
	            if ( wVramStart < 0 )
		       wVramStart = VIDEO_MEMORY_WIDTH - 1;  //Get Unused area
	            if ( wPieceCount < (gwVideoWidth - xPageWidth) ) 
		       /* write color-key into video memory */
  		       JUN_ClearImageRect((UINT)wVramStart, yVmemTop,
                               1, yDataHeight, gConfig.bColorKey);
                    else { // 11/25/93 Tang added
                       /* write one line of image data into video memory */
		       if ( wPieceCount < gwVideoWidth ) {
                          outportb(0x80,(BYTE)gwPieceCount);
		          HandleImageRect1((UINT)wVramStart, yVmemTop,
                                1, yDataHeight,
                                xChangeLeft, yDataTop,
                                xPageWidth, yPageHeight,
                                (LPSTR)hpBits, H_WRITE);
	               }
	               else {
		          /* write color-key into video memory */
		          JUN_ClearImageRect(wVramStart, yVmemTop,
                                1, yDataHeight, gConfig.bColorKey);
	               }
                       xChangeLeft--;
                    }
	            wVramStart--;
	            wPieceCount++;
	         } /* end of for */
              }
              else {  // 11/25/93 Tang added for compensate
                 if ( (wPieceCount - gwVideoWidth) <= xVmemLeft ) 
                    /* reset proper start line to put image data into */
		    if ( wVramStart< 0 )
		       wVramStart= (int)(VIDEO_MEMORY_WIDTH - 1);
		    for ( i=0; i<wRegGaps*2; i++ ) {
                       /* reset proper start line to put image data into */
		       if ( wVramStart < 0 )
			  wVramStart = (int)(VIDEO_MEMORY_WIDTH - 1);
		       /* write color-key into video memory */
		       JUN_ClearImageRect((UINT)wVramStart, yVmemTop,
                                   1, yDataHeight, gConfig.bColorKey);
		       wVramStart--;
		       wPieceCount++;
		    } /* end of for */
              }  // 11/25/93 Tang added for compensate


              /* Step 2. write to shift register */
	      reg_temp_start -= wRegGaps;
              /* 11/26/93 Tang updated */
              reg_temp_start &= 0x1ff;
	      wait_vert_retrace_start();
//	      set_horz_shift_reg(reg_temp_start);
	      set_horz_shift_reg1(reg_temp_start);
/*
              if ( reg_temp_start < 0 ) {
                 set_horz_shift_reg_msb(1);
	         reg_temp_start = (int)(0x01ff & reg_temp_start);
              }
	      else if ( reg_temp_start <= 0xff ) {
                      // 9th bit is 0, set LSB
                      set_horz_shift_reg_msb(0);
                   }
	      horz_scan_start = (BYTE)reg_temp_start;
	      wait_vert_retrace_start();
	      set_horz_shift_reg(horz_scan_start);
*/

              /* End of scrolling ? Yes, set flag */
	      if ( wPieceCount >= (gwVideoWidth + xVmemLeft) )
	         fScroll = FALSE;
	   } /* end of while (fScroll) */

           /* restore to original status */
           gConfig.wHorzScanStart = reg_temp_start;
           break;

      case SCROLL_NONE :
           while ( fScroll ) {
              /* Step 1. Scrolling bitmap into video memory */
	      /* write proper lines of bitmap image into video memory */
	      if ( wPieceCount < gwVideoHeight ) {
                 /* write wRegGaps*2 lines per times */
	         for ( i=0; i<wRegGaps*2; i++ ) {
                    /* reset proper start line to put image data into */
		    if ( wVramStart >= VIDEO_MEMORY_HEIGHT )
		       wVramStart = 0;
                    /* write one line of image data into video memory */
		    if ( wPieceCount < yPageHeight ) {
                       outportb(0x80,(BYTE)wPieceCount);
		       HandleImageRect1(xVmemLeft, (UINT)wVramStart,
                             xDataWidth, 1,
                             xDataLeft,  yChangeTop,
                             xPageWidth, yPageHeight,
                             (LPSTR)hpBits, H_WRITE);
		    }
		    else {
		       /* write color-key into video memory */
		       JUN_ClearImageRect(xVmemLeft, wVramStart,
                              xDataWidth, 1, gConfig.bColorKey);
		    }
		    wVramStart++;
		    wPieceCount++;
		 } /* end of for */
	      }
              else
                 fScroll = FALSE;
           } /* end of while (fScroll) */
           break;

   } /* end of switch */

   /* release resources to Windows */
   SelectObject(hDC, hOldBitmap);
   DeleteObject(hBitmap);
   GlobalUnlock(hMem);
   GlobalFree(hMem);

   return(TRUE);
}


/****************************************************************************
* JUN_ScrollLines(int wDir, int wSpeed, int wAmount)
*
*   This routine scroll the data in the video memory, and wrap around
* from end to end if user is request.
*
* Entry: wDir	- dirction(0:up or 1:down, 2:left, 3:right) for scrolling
*	 wSpeed - speed for scrolling
*		  val  RegsInc(Gaps)  LinesInc	LoopTimes
*		   1 -	  1		 2	  256
*		   2 -	  2		 4	  128
*		   3 -	  3		 6	   -
*		   4 -	  4		 8	   64
****************************************************************************/
int FAR PASCAL JUN_ScrollLines(UINT wDir, UINT wSpeed, UINT wAmount)
{
   UINT  i, horz_scan_start;
   BYTE  vert_scan_start;

   if ( wDir > 4 )  return(ERR_SCROLL_TYPE_NOT_SUPPORT);

   if ( wSpeed < 1 || wSpeed > 4 )  return(ERR_SCROLL_SPEED_NOT_ALLOW);

   switch ( wDir ) {
      case SCROLL_UP :
           vert_scan_start = (BYTE)gConfig.wVertScanStart;
           for ( i=0; i<wAmount; i++ ) {
              vert_scan_start += wSpeed;
              vert_scan_start &= 0xff; 
	      wait_vert_retrace_start();
	      set_vert_scroll_reg(vert_scan_start);
           }
           /* set vert scrlll register to new value */
           gConfig.wVertScanStart = vert_scan_start;
           break;

      case SCROLL_DOWN :
           vert_scan_start = (BYTE)gConfig.wVertScanStart;
           for ( i=0; i<wAmount; i++ ) {
              vert_scan_start -= wSpeed;
              vert_scan_start &= 0xff; 
	      wait_vert_retrace_start();
	      set_vert_scroll_reg(vert_scan_start);
           }
           /* set vert scrlll register to new value */
           gConfig.wVertScanStart = vert_scan_start;
           break;

      case SCROLL_LEFT :
           horz_scan_start = gConfig.wHorzScanStart;
           for ( i=0; i<wAmount; i++ ) {
              horz_scan_start += wSpeed;
              horz_scan_start &= 0x01ff; 
	      wait_vert_retrace_start();
//	      set_horz_shift_reg1(horz_scan_start);
	      set_horz_shift_reg(horz_scan_start);
           }
           /* set horz shift register to new value */
           gConfig.wHorzScanStart = horz_scan_start;
           break;

      case SCROLL_RIGHT :
           horz_scan_start = gConfig.wHorzScanStart;
           for ( i=0; i<wAmount; i++ ) {
              horz_scan_start -= wSpeed;
              horz_scan_start &= 0x01ff; 
	      wait_vert_retrace_start();
//	      set_horz_shift_reg1(horz_scan_start);
	      set_horz_shift_reg(horz_scan_start);
           }
           /* set horz shift register to new value */
           gConfig.wHorzScanStart = horz_scan_start;
           break;
   } // end of switch //

   return(TRUE);
}


int FAR PASCAL JUN_ScrollBitmap1(LPSTR lpImageBuf, UINT xWidth, UINT yHeight,
	       int nDir, int nSpeed)
{
   UINT  nRegGaps, yMemStart, yImgCount, padded;
   int	 i, temp, count;
   unsigned long  lSize;
   BYTE  vert_scan_start, fScroll;
   LPSTR  lpBuf;
   BYTE huge *hpBits;

   if ( nSpeed <1 && nSpeed >4 )  return(FALSE);

   /* padded 0 to satisfy 2-byte boundary */
   if ( (padded=(xWidth % 2)) )
      padded = 2 - padded;

   hpBits = lpImageBuf;
   /* set pointer to the last line(first scan line) */
   lSize = (unsigned long)(xWidth+padded)*(yHeight-1);
   hpBits += lSize;

   nRegGaps = 1 << (nSpeed-1);	 //Increment of register value
   yImgCount = 0;   //Count for Image height
   fScroll = TRUE;

   if ( nDir == SCROLL_UP ) {
      temp = 0;
      yMemStart = 0;
      while ( fScroll ) {
	 if ( temp > 0xff )
	    temp = 0;
	 vert_scan_start = (BYTE)temp;
	 wait_vert_retrace_start();
	 set_vert_scroll_reg(vert_scan_start);
	 temp += nRegGaps;
	 /* Step 1. Scrolling bitmap into vide memory */
	 /* write proper lines of bitmap image into video memory */
	 if ( yImgCount < yHeight ) {
	    for ( i=0; i<nRegGaps*2; i++ ) {
	       if ( yMemStart >= VIDEO_MEMORY_HEIGHT )
		  yMemStart = 0;	//Get Unused area
	       if ( yImgCount < yHeight ) {
		  HandleImageRect(0, yMemStart, xWidth, 1, (LPSTR)hpBits, H_WRITE);
		  hpBits -= (xWidth+padded);
	       }
	       else {
		  /* write color-key into video memory */
		  HandleImageRect(0, yMemStart, xWidth, 1, NULL, H_CLEAR);
	       }
	       yMemStart++;
	       yImgCount++;
	    }
	    count = 0;
	 }
	 /* Step 2. Scrolling bitmap out from video memory */
	 else {
	    if ( count < VIDEO_MEMORY_HEIGHT/nRegGaps ) {
	       for ( i=0; i<nRegGaps*2; i++ ) {   //lines to processed
		  if ( yMemStart >= VIDEO_MEMORY_HEIGHT )
		     yMemStart = 0;	//Get Unused area
		  /* write color-key into video memory */
		  HandleImageRect(0, yMemStart, xWidth, 1, NULL, H_CLEAR);
		  yMemStart++;
	       }
	       count += nRegGaps;
	    }
	    else
	       fScroll = FALSE;
	 }

      } /* end of while (fScroll) */
      /* restore to original status */
      set_vert_scroll_reg(0);
   }

   return(TRUE);
}


/****************************************************************************
* JUN_VerticalScroll(int nDir, int nSpeed)
*
*   This routine scroll up the data in the video memory, and wrap around
* from end to end if user is request.
*
* Entry: nDir	- dirction(0:up or 1:down) for scrolling
*	 nSpeed - speed for scrolling
*		  val  RegsInc(Gaps)  LinesInc	LoopTimes
*		   1 -	  1		 2	  256
*		   2 -	  2		 4	  128
*		   3 -	  4		 8	   64
*		   4 -	  8		16	   32
****************************************************************************/
int FAR PASCAL JUN_VerticalScroll(int nDir, int nSpeed)
{
   UINT  i;
   BYTE  vert_scan_start;

   if ( nSpeed <1 && nSpeed >4 )
      return(FALSE);

   if ( nDir == SCROLL_UP ) {
      vert_scan_start = 0;
      for (i=0; i<VIDEO_MEMORY_HEIGHT/(1<<nSpeed); i++) {
	 wait_vert_retrace_start();
	 set_vert_scroll_reg(vert_scan_start);
	 vert_scan_start += (1 << (nSpeed-1));
      }
      /* restore to original status */
//      set_vert_scroll_reg(0);
   }

   if ( nDir == SCROLL_DOWN ) {
      vert_scan_start = (VIDEO_MEMORY_HEIGHT/2) - (1 << (nSpeed-1));
     for (i=0; i<VIDEO_MEMORY_HEIGHT/(1<<nSpeed); i++) {
	 wait_vert_retrace_start();
	 set_vert_scroll_reg(vert_scan_start);
	 vert_scan_start -= (1 << (nSpeed-1));
      }
      /* restore to original status */
//      set_vert_scroll_reg(0);
   }
   gConfig.wVertScanStart = vert_scan_start;

/* ISR scrolling
  vert_scan_start = 0;
  fScroll = TRUE;
  sstart = 0;
  yheight = 4;
  ynouse = vert_scan_start*yheight + 480;
  JUN_enable_vert_sync_int(TRUE);
  while ( fScroll ) {
    JUN_enable_vert_sync_int(TRUE);
  }

  fScroll = FALSE;
  JUN_enable_vert_sync_int(FALSE);
*/

   return(TRUE);
}


/**************************************************************************
* JUN_HorizontalShift(int nDir, int nSpeed)
*
*   This routine shift right the data in the video memory, and wrap around
* from end to end if user is request.
*
* Entry: nDir	- dirction(up or down) for scrolling
*	 nSpeed - speed for scrolling
*		  val  RegsInc(Gaps)  LinesInc	LoopTimes
*		   1 -	  1		 2	  512
*		   2 -	  2		 4	  256
*		   3 -	  4		 8	  128
*		   4 -	  8		16	   64
****************************************************************************/
int FAR PASCAL JUN_HorizontalShift(int nDir, int nSpeed)
{
   UINT i, temp;
   UINT horz_scan_start;

   if ( nSpeed <1 && nSpeed >4 )
      return(FALSE);

   if ( nDir == SCROLL_RIGHT ) {
      temp = 0;
      for (i=0; i<VIDEO_MEMORY_WIDTH/(1<<nSpeed); i++) {
	 wait_vert_retrace_start();
	 horz_scan_start = (BYTE)temp;
	 set_horz_shift_reg(horz_scan_start);
	 temp += (1 << (nSpeed-1));
      }
      /* restore to original status */
//      set_horz_shift_reg_msb(0);
//      set_horz_shift_reg(0);
   }

   if ( nDir == SCROLL_LEFT ) {
     temp = (VIDEO_MEMORY_WIDTH/2) - (1 << (nSpeed-1));
     set_horz_shift_reg_msb(1);
      for (i=0; i<VIDEO_MEMORY_WIDTH/(1<<nSpeed); i++) {
	 wait_vert_retrace_start();
	 horz_scan_start = temp;
	 set_horz_shift_reg(horz_scan_start);
	 temp -= (1 << (nSpeed-1));
      }
      /* restore to original status */
//      set_horz_shift_reg_msb(0);
//      set_horz_shift_reg(0);
   }
   gConfig.wHorzScanStart = (WORD)temp;

   return(TRUE);
}


/****************************************************************************
* HandleImageRect1(xLeft, yTop, xWidth, yHeight,
                   xDataLeft, yDataTop, xPageWidth, yPageHeight,
                   lpImageBuf, nType)
*
*   Mapping a bitmap(xLeft, yTop, xWidth, yHeight) of MS-Windows at VGA screen
* to Junior video memory. During the mapping, the 8 segments of the video
* memoery needs to be bank switched in order to be accessed by CPU.
*   The types of processing image rect. in the Video Memory are:
* 1. nType = 1 : Read - Read image rect. from video memory to buffer.
* 2. nType = 2 : Write - Write image rect. from buffer to video memory.
*
* Exported:  No
* Entry:     A specified rect.- (x,y,width,height), buffer, and operation type
* Destroys:
* Exit:      TRUE for success, FALSE for failure
****************************************************************************/
int FAR  HandleImageRect1(UINT xLeft,  UINT yTop,      //(left,top)
			  UINT xDataWidth, UINT yDataHeight,   //(width,height)
                          UINT xDataLeft, UINT yDataTop,
                          UINT xPageWidth, UINT yPageHeight,
			  LPSTR lpImageBuf,            //r/w buffer
			  UINT nType)	              //operation type
{
   BYTE	  memory_segment, bank_no;
   static BYTE  prev_memory_segment=10, prev_bank_no=9;
   UINT	  x, y, padded, start_line, rel_x, rel_y;
   DWORD  dwSegBoundary;
   BYTE huge * hpImageBuf;
   LPSTR  lpVideo;
   UINT   i, j;

   if ( xDataWidth > VIDEO_MEMORY_WIDTH || yDataHeight > VIDEO_MEMORY_HEIGHT ) {
//      MessageBox(hWnd, "Out of range !", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }

   if ( xPageWidth > VIDEO_MEMORY_WIDTH || yPageHeight > VIDEO_MEMORY_HEIGHT ) {
//      MessageBox(hWnd, "Out of range !", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }

   /* 11/20/93 Tang added */
   if ( (xDataWidth == 0)  || (yDataHeight == 0) )
      return(TRUE);
   /* 11/20/93 Tang added */

   if ( padded=(xPageWidth%2) )
     padded = 2 - padded;

   /* get relative x location */
   rel_x = fnRelX(xLeft);

   /* convert to huge pointer */
   hpImageBuf = lpImageBuf;
   hpImageBuf += ( ((DWORD)yDataTop*((DWORD)xPageWidth+padded)) + xDataLeft);

  for ( y=yTop; y<yTop+yDataHeight; y++) {     //Process one line per times
//      if ( gfHandleScroll && (gfScrollType[gfSwitch_ISR] <= 1) ) 
      /* get relative y location */
      rel_y = fnRelY(y);
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
      if ( memory_segment != gbPreMemSeg ) { // not previous memory segment
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
//   outportb(0x80,(BYTE)((memory_segment<<4)+bank_no));
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
//      lpVideo = (LPSTR)glpVideoMemory + (start_line<<10 + xLeft);
      lpVideo = GetVideoAddress(rel_x, start_line);
      xLength[i++] = xDataWidth;
      xLength[i--] = 0;
      if ( ( rel_x + xDataWidth ) > VIDEO_MEMORY_WIDTH ) {
         xLength[i++] = VIDEO_MEMORY_WIDTH - rel_x;
         xLength[i] = xDataWidth - xLength[i-1];
      }

      switch ( nType ) {
	 case H_READ:
              for ( j=0; j<=i; j++) {
                 if ( j > 0 )
                    lpVideo = GetVideoAddress(0, start_line);
                 /* byte left in 64K segment */
	         dwSegBoundary = (0xFFFF ^ LOWORD(hpImageBuf)) + 1;
	         if (dwSegBoundary >= (DWORD)xLength[j]) {
                    FarMemoryCopy((DWORD)lpVideo, (DWORD)hpImageBuf, xLength[j]);
                    hpImageBuf += xLength[j];
	         }
                 /* take care of scan line on 64K segment */
	         else {
                    FarMemoryCopy((DWORD)lpVideo, (DWORD)hpImageBuf, (WORD)dwSegBoundary);
		    hpImageBuf += dwSegBoundary;
                    FarMemoryCopy((DWORD)lpVideo+dwSegBoundary, (DWORD)hpImageBuf,
                                  xLength[j]-(WORD)dwSegBoundary);
		    hpImageBuf += (xLength[j] - (WORD)dwSegBoundary);
	         }
              }
              hpImageBuf += (((DWORD)xPageWidth + padded) - xDataWidth);
              /* padded 0 to satisfy 2-byte boundary,
                 not only for video memory, but also Bitmap file itself */
//              if ( (padded=(xWidth%2)) ) {
//                 for ( x=0; x<(2-padded); x++)   //no. of zero to padded
//                    glpVideoMemory[(start_line<<10) + (xLeft+xWidth+x)] = 0;
//                 hpImageBuf += (2-padded);
///              }
              break;

	 case H_WRITE:
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
              hpImageBuf += (((DWORD)xPageWidth + padded) - xDataWidth);
              /* padded 0 to satisfy 2-byte boundary,
                 not only for video memory, but also Bitmap file itself */
//              if ( (padded=(xWidth%4)) ) {
//                 for ( x=0; x<(2-padded); x++)   //no. of zero to padded
//                    glpVideoMemory[(start_line<<10) + (xLeft+xWidth+x)] = gConfig.bColorKey;
//                 hpImageBuf += (2-padded);
//              }
              break;

         case H_CLEAR:
              for ( j=0; j<=i; j++) {
                 if ( j > 0 )
                    lpVideo = GetVideoAddress(0, start_line);
                 /* byte left in 64K segment */
                 for ( x=0; x<xLength[j]; x++ ) {
                    *lpVideo = gConfig.bColorKey;
                    lpVideo += 1;
                 }
              }
// original style
//              lpVideo = GetVideoAddress(rel_x, start_line);
//              for ( x=xLeft; x<xLeft+xWidth; x++ ) {
//                 *lpVideo = gConfig.bColorKey;
//                 lpVideo += 1;
//              }
//              for ( x=xLeft; x<xLeft+xWidth; x++ )
//                 glpVideoMemory[(start_line<<10) + x] = gConfig.bColorKey;
              break;


      } /* end of switch */

      //12/30 SCROLL_UP && SCROLL_DOWN then do //
/*
      if ( gfHandleScroll && (gfScrollType[gfSwitch_ISR] <= 1) )
         if ( xLeft+xDataWidth < gwVideoWidth ) {
            lpVideo = GetVideoAddress(fnRelX(xLeft+xDataWidth), start_line);
            for ( x=0; x<gwVideoWidth-(xLeft+xDataWidth); x++ ) {
               *lpVideo = gConfig.bColorKey;
               lpVideo += 1;
            }
         }
*/

   } /* end of for */

   //12/30 SCROLL_UP && SCROLL_DOWN then do //
/*
   if ( gfHandleScroll && (gfScrollType[gfSwitch_ISR] >= 2) )
      if ( yTop+yDataHeight < gwVideoHeight ) {
         lpVideo = GetVideoAddress(fnRelX(xLeft+xDataWidth), start_line);
         for ( x=0; x<gwVideoWidth-(xLeft+xDataWidth); x++ ) {
			 JUN_WriteImageRect((UINT)gwVramStart[gfSwitch_ISR], gwyVmemTop,
                                   gwLineGaps[gfSwitch_ISR], gwyShowHeight[gfSwitch_ISR],
                                  (LPSTR)glpEmptyArray);

            *lpVideo = gConfig.bColorKey;
            lpVideo += 1;
         }
      }
*/
   return(TRUE);
} /* end of HandleImageRect() */



// end of junscol.c
