
/************************************************************************
* Project : junior.dll
* Module  : junisr.c
* Object  : ISR routine for handleing scrolling
* Author  : Kung-Puu Tang
* Date	  : 821118
************************************************************************/

#include <windows.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <conio.h>
//#include <io.h>
#include <dos.h>           //enable(),disable(),getvect(),setvect(),outportb
#include <time.h>
#include "juniora.h"
#include "externa.h"


int FAR PASCAL JUN_StartScroll(void)
{
   int          i;
   UINT         padded;
   DWORD	dwDataSize;
   LPSTR        lpEmpty;

   outportb(0x80, 0x77);

   /* 1. Initialize global variables used in ISR scrolling */
   gfSwitch_PRG = 0;            // flag to switch buffer in commander
   gfSwitch_ISR = 0;            // flag to switch buffer in ISR
   gwPieceCount = 0;	        // count for Image piece(row/column)
   gwPageCount = 1;             // loaded page's count
   for (i=0; i<2; i++) {
      gfBufferFull[i]  = FALSE; // flag to indicate buffer is full or not
      gbScrollType[i]  = 0;     // 4 types of scrolling
      gwRegTempStart[i]= 0;     // scoll register temp value
      gwVramStart[i]   = 0;     // start location in Vram where image write
      gwRegGaps[i]     = 0;     // increments of register value
      gwLineGaps[i]    = 0;     // increments of processed line
      gwxShowLeft[i]   = 0;     // start x pos. of desired area in a page
      gwyShowTop[i]    = 0;     // start y pos. of desired area in a page
      if ( gbTVsystemType ==  NTSC ) {
         gwxShowWidth[i]  = 640;   // width of desired area
         gwyShowHeight[i] = 400;   // height of desired area
         gwxPageWidth[i]  = 640;   // width of a page
         gwyPageHeight[i] = 400;   // height of a page
      }
      else {
         gwxShowWidth[i]  = 800;   // width of desired area
         gwyShowHeight[i] = 480;   // height of desired area
         gwxPageWidth[i]  = 800;   // width of a page
         gwyPageHeight[i] = 480;   // height of a page
      }
      gwxChangeLeft[i] = 0;     // processed x-loc of image to put into Vram
      gwyChangeTop[i]  = 0;     // processed y-loc of image to put into Vram
   }
   gbVertScanStart = 0;         // vert. scan start value of reg.
   gbHorzScanStart = 0;         // horz. scan start value of reg.
   if ( gbTVsystemType ==  NTSC ) {
      gwxVmemLeft = 48;            // start x pos. in Video memory
      gwyVmemTop  = 38;            // start y pos. in Video memory
   }
   else {
      gwxVmemLeft = 54;            // start x pos. in Video memory
      gwyVmemTop  = 12;            // start y pos. in Video memory
   }
   gbPreMemSeg = 9;             // previous selected memory segment
   gbPreBnkNo  = 9;             // previous selected bank no.
   /* flag to control scrolling */
   gfHandleScroll = TRUE;       // flag to begin scrolling process
   gfScrollStop   = FALSE;      // flag to stop scrolling process
   gfPriIsrFinish = TRUE;       // flag to indicate prior ISR process is finished

   /* 2. Allocate 8K memory filled with color key for scrolling empty */
   ghMemory = GlobalAlloc(GHND, ((DWORD)VIDEO_MEMORY_WIDTH << 3));
   if ( !ghMemory )  return(ERR_MEMORY_ALLOCATE_ERROR);
   glpEmptyArray = lpEmpty = GlobalLock(ghMemory);
   for ( i=0; i<(VIDEO_MEMORY_WIDTH << 3); i++ )
      *lpEmpty++ = gConfig.bColorKey;

   /* 3. Allocate 2 buffers used for ISR to scroll pages */
   // allocate at Initialization time
   InitialPageBuffer();

   /* 4. setup interrupt vector and mask interrupt no. */
   init_interrupt(gConfig.bIRQUsed);

   return(TRUE);
}

int FAR PASCAL JUN_QuitScroll(void)
{
   /* If not scrolling, then return FALSE */
   if ( ! gfHandleScroll )  return(FALSE);

   /* only 1 page to scroll, trigle ISR to do scrolling */
   if ( gfHandleScroll && (gwPageCount <= 2) )
      JUN_enable_vert_sync_int(TRUE);

   /* remain 2 pages in ISR for scrolling */
   if ( gfSwitch_PRG == gfSwitch_ISR ) {
      /* wait for the prior page to complete */
      while ( gfBufferFull[gfSwitch_PRG] ) ;
   }  //scroll complete
   /* remain last page to do scrolling */
   /* Semaphore ISR that scrolling process is STOP */
   gfScrollStop = TRUE;    // command to ISR for stop scrolling
   /* wait for ISR scroll over last page */
   while ( gfHandleScroll ) ;

   /* free last allocated memory */
   // free at Exit time

   GlobalUnlock( ghMemory );
   GlobalFree( ghMemory );

   /* restore original interrupt vector and unmask interrupt no. */
   restore_interrupt(gConfig.bIRQUsed);

   return(TRUE);
}


/************************************************************************
* JUN_ScrollPages
*
*   Scrolling continue pages(these pages must have consistency.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
int FAR PASCAL JUN_ScrollPages(HDC  hDC,    HPALETTE hPAL,
                               UINT xVramLeft,  UINT yVramTop,
                               UINT xDataWidth, UINT yDataHeight,
                               UINT xDataLeft,  UINT yDataTop,
                               UINT xPageWidth, UINT yPageHeight,
			       UINT wType,      UINT wSpeed)
{
   DWORD	dwNumBytes, dwSegBoundary;
   UINT         i, j;
   BYTE huge    *hpBits;       // local huge pointer to image bits
   int          wReturnCode;

   if ( wType > 4 )  return(ERR_SCROLL_TYPE_NOT_SUPPORT);

   if ( wSpeed < 1 || wSpeed > 3 )  return(ERR_SCROLL_SPEED_NOT_ALLOW);

   /* Bitmap load too fast! We must wait until ISR scrolling over a page
      and release empty buffer, so we can fill data in the buffer. */
   while ( gfBufferFull[gfSwitch_PRG] && (gwPageCount >= 3) ) ;

//   outportb(0x80, 0x33);

//   wsprintf(debugstr, "hDC=%4d, PageWidth=%4d, PageHeight=%4d", hDC, xPageWidth, yPageHeight);
   if ( hDC ) {
      wReturnCode = GetImageData(hDC, xPageWidth, yPageHeight, ghpBitsStart[gfSwitch_PRG]);
      if ( wReturnCode != TRUE )
         return(wReturnCode);
      // 3/10/94 Tang added //
      GetAndSetPalette(hDC, hPAL);
   }
   else {
      if ( gbTVsystemType ==  NTSC ) {
         gwxVmemLeft = 48;            
         gwyVmemTop  = 38;            
      }
      else {
         gwxVmemLeft = 54;            
         gwyVmemTop  = 12;            
      }
      xDataWidth  = VGA_PAGE_WIDTH;
      yDataHeight = VGA_PAGE_HEIGHT;
      xDataLeft   = 0;
      yDataTop    = 0;
      xPageWidth  = VGA_PAGE_WIDTH;
      yPageHeight = VGA_PAGE_HEIGHT;

      hpBits = ghpBitsStart[gfSwitch_PRG];
      dwNumBytes = 0;
      for ( i=0; i<VGA_PAGE_HEIGHT; i++ ) {
         // byte left in 64K segment //
	 dwSegBoundary = (0xFFFF ^ LOWORD(hpBits)) + 1;
	 if (dwSegBoundary >= (DWORD)VGA_PAGE_WIDTH)
            for ( j=0; j<VGA_PAGE_WIDTH+gwImgWidthPad; j++ ) {
               *hpBits = gConfig.bColorKey;
               hpBits += 1;
               dwNumBytes++;
            }
         // take care of scan line on 64K segment //
	 else {
            for ( j=0; j<(WORD)dwSegBoundary; j++ ) {
               *hpBits = gConfig.bColorKey;
               hpBits += 1;
               dwNumBytes++;
            }
            for ( j=0; j<(WORD)(VGA_PAGE_WIDTH+gwImgWidthPad)-dwSegBoundary; j++ ) {
               *hpBits = gConfig.bColorKey;
               hpBits += 1;
               dwNumBytes++;
            }
	 }
      }
   }

//   wsprintf(debugstr, "width=%4d, height=%4d", Bitmap.bmWidth, Bitmap.bmHeight);
//   MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);

   gwPadded[gfSwitch_PRG] = gwImgWidthPad;

   gwxVmemLeft                 = xVramLeft;
   gwyVmemTop                  = yVramTop;
   gwxShowLeft[gfSwitch_PRG]   = xDataLeft;
   gwyShowTop[gfSwitch_PRG]    = yDataTop;
   gwxShowWidth[gfSwitch_PRG]  = xPageWidth;
   gwyShowHeight[gfSwitch_PRG] = yPageHeight;
   gwxPageWidth[gfSwitch_PRG]  = xPageWidth;
   gwyPageHeight[gfSwitch_PRG] = yPageHeight;

   gbScrollType[gfSwitch_PRG] = wType;  //scroll type
   gwRegGaps[gfSwitch_PRG] = wSpeed;    //Increment of register value
   gwLineGaps[gfSwitch_PRG] = gwRegGaps[gfSwitch_PRG] << 1;

   /* get the start address for the scrolling */
   ghpBits[gfSwitch_PRG] = ghpBitsStart[gfSwitch_PRG];
/*12/30*/  if ( gwPageCount < 3 ) {
   switch ( wType ) {
      case SCROLL_UP :
           /* in video memory, fix x, variable y */
           gwyChangeTop[gfSwitch_PRG] = 0;
           /* 11/25/93 Tang, y start must relative to yVramTop */
           gwVramStart[gfSwitch_PRG] = gwVideoHeight;
           /* set scroll/shift scan start register value */
//12/30    if ( gwPageCount < 3 )
           gwRegTempStart[gfSwitch_PRG] = gConfig.wVertScanStart;
           break;

      case SCROLL_DOWN :
           /* in video memory, fix x, variable y */
//3/11/94           gwyChangeTop[gfSwitch_PRG] = gwVideoHeight - 2;
           gwyChangeTop[gfSwitch_PRG] = gwVideoHeight - 1;
//3/11/94
//           gwyChangeTop[gfSwitch_PRG] = gwyPageHeight[gfSwitch_PRG] - 1;
           /* 11/25/93 Tang, y start must relative to yVramTop, but
              this can be shown at the screen while writing color key */
//           gwVramStart[gfSwitch_PRG] = yVramTop - 1;
//3/11/94           gwVramStart[gfSwitch_PRG] = (int)(VIDEO_MEMORY_HEIGHT - 2);
           // if PAL then h=510, we must start from last line:511
           if ( gbTVsystemType == PAL && gConfig.bScanMode == OVERSCAN )
              gwVramStart[gfSwitch_PRG] = (int)VIDEO_MEMORY_HEIGHT - 1;
           else  // NTSC or other
              gwVramStart[gfSwitch_PRG] = (int)(VIDEO_MEMORY_HEIGHT - gwLineGaps[gfSwitch_PRG]);
//3/11/94
//           gwVramStart[gfSwitch_PRG] = (int)(VIDEO_MEMORY_HEIGHT - 1);
           /* set scroll/shift scan start register value */
//12/30    if ( gwPageCount < 3 )
           gwRegTempStart[gfSwitch_PRG] = gConfig.wVertScanStart;
           break;

      case SCROLL_LEFT :
           /* 11/25/93 Tang, x start must relative to xVramLeft */
           /* in video memory, fix y, variable x */
           gwVramStart[gfSwitch_PRG] = gwVideoWidth;
           gwxChangeLeft[gfSwitch_PRG] = 0;
           /* set scroll/shift scan start register value */
//12/30    if ( gwPageCount < 3 )
           gwRegTempStart[gfSwitch_PRG] = gConfig.wHorzScanStart;
           break;

      case SCROLL_RIGHT :
           /* in video memory, fix y, variable x */
           /* 11/25/93 Tang, x start must relative to xVramLeft, but
              this can be shown at the screen while writing color key */
//           gwVramStart[gfSwitch_PRG] = xVramLeft - 1;
//3/11/94           gwVramStart[gfSwitch_PRG] = (int)(VIDEO_MEMORY_WIDTH - 2);
           if ( gbTVsystemType == PAL && gConfig.bScanMode == OVERSCAN )
              gwVramStart[gfSwitch_PRG] = (int)VIDEO_MEMORY_WIDTH - 1;
           else  // NTSC or other
              gwVramStart[gfSwitch_PRG] = (int)(VIDEO_MEMORY_WIDTH - gwLineGaps[gfSwitch_PRG]);
//3/11/94
//3/11/94           gwxChangeLeft[gfSwitch_PRG] = gwVideoWidth - 2;
           gwyChangeTop[gfSwitch_PRG] = gwVideoWidth - 1;
//3/11/94
//           gwVramStart[gfSwitch_PRG] = (int)(VIDEO_MEMORY_WIDTH - 1);
//           gwxChangeLeft[gfSwitch_PRG] = gwxPageWidth[gfSwitch_PRG] - 1;
           /* start register value */
           /* set scroll/shift scan start register value */
//12/30    if ( gwPageCount < 3 )
           gwRegTempStart[gfSwitch_PRG] = gConfig.wHorzScanStart;
           break;

      case SCROLL_NONE :
//           ghpBits[gfSwitch_PRG] += xDataLeft;
           gwyChangeTop[gfSwitch_PRG] = 0;
           gwVramStart[gfSwitch_PRG]  = 0; //each y pos. in Vmem
           break;

   } /* end of switch */
   } /*12/30*/

   gwPageCount++;

   /* set flag of buffer_full to TRUE at ISR is done */
//   while ( ! gfPriIsrFinish ) ;
   disable();
   gfBufferFull[gfSwitch_PRG] = TRUE;
   enable();

   /* After load 2 pages, it is time to start ISR to do scrolling */
   if ( gwPageCount == 3 ) {
      JUN_enable_vert_sync_int(TRUE);
      outportb(0x80, 0x99);
   }

   /* IF no semaphore to STOP scrolling(not call JUN_QuitScroll yet),
      then we return back to upper_layer to load next page */
   if ( gfHandleScroll )    // continue to load next page
      gfSwitch_PRG ^= 1;

   return(TRUE);
}


// ===========================================================================
//	       vertical sync interrupt service routines(ISR)
// 1. void	      init_interrupt(void)
// 2. void	      restore_interrupt(void)
// 3. void interrupt  new_interrupt(void)
// 4. static int      music(void)
// ===========================================================================
static BYTE	       old_mask; // store original 8259 IMR
static void interrupt  (*old_interrupt)(void); // old interrupt function pointer

// ***************************************************************************
// music() : issue a beep, used to debugging
// ***************************************************************************
static int  music(void)
{
    UINT  i;

  outportb(0x43, 0xb6);
  outportb(0x42, 0x53);
  outportb(0x42, 0x02);
  outportb(0x61, inportb(0x61) | 0x03);
  for(i=10000; i>0; i--);
  outportb(0x61, inportb(0x61) & 0xfc);
  return(0);
} // end function static int music(void)


// ***************************************************************************
// init_interrupt() : set new interrupt service routine
// ***************************************************************************
void FAR init_interrupt(BYTE bIRQ)
{
  disable();

  // save old interrupt vector
  old_interrupt = getvect(bIRQ + 8);
  // install new interrupt vector
  setvect(bIRQ + 8, JUN_isr_scrolling);
//  setvect(gConfig.bIRQUsed + 8, new_interrupt);

//  old_interrupt = SetInterruptVector(gConfig.bIRQUsed+8, new_interrupt);

  old_mask = SetInterruptMask(bIRQ);

  enable();
} // end function void interrupt init_interrupt(void)

void FAR init_interrupt_test(BYTE bIRQ)
{
  disable();

  // save old interrupt vector
  old_interrupt = getvect(bIRQ + 8);
  // install new interrupt vector
  setvect(bIRQ + 8, new_interrupt);

//  old_interrupt = SetInterruptVector(gConfig.bIRQUsed+8, new_interrupt);

  old_mask = SetInterruptMask(bIRQ);

  enable();
} // end function void interrupt init_interrupt(void)

// ***************************************************************************
// restore_interrupt() : restore original interrupt service routine
// ***************************************************************************
void FAR restore_interrupt(BYTE bIRQ)
{
  disable();

  // restore to original interrupt routine
  setvect(bIRQ + 8, old_interrupt);

//  SetInterruptVector(gConfig.bIRQUsed+8, old_interrupt);

  // program 8259 IMR register, use OCW1 to restore original IMR
  outportb(0x21, old_mask);
//  myoutportb(0x21, old_mask);

  enable();
} // end function void restore_interrupt(void)


// ***************************************************************************
// new_interrupt() : new interrupt service routine
//		     this interrupt is triggerd by "rising edge of vertical
//		     sync"
// ***************************************************************************
//void interrupt	new_interrupt(void)
static void interrupt	new_interrupt(void)
{
  int  nColor, imr_value;

//  gulEnterIntCount++;
  /* if the interrupt is generated by our JUNIOR IRQ ? */
  if ( JUN_vert_sync_int_occur() ) {
//  if ( JUN_vert_sync_int_occur() && gConfig.bIRQUsed ) {
#ifdef TEST_VER
     /* play sound to attrative */
     music();
#endif
     /* got an interrupt */
     gulEnterIntCount++;
     /* disable interrupt */
     JUN_enable_vert_sync_int(FALSE);
     /* enable interrupt immediately */
//     JUN_enable_vert_sync_int(TRUE);

     /* need to do scrolling ? */
     if ( gfScroll ) {
	// 10/15 Tang added
	if ( sstart > 480/2 )  gfScroll=FALSE;
	else {
	   // 10/15 Tang added
	   //	  randomize();
	   if ( ynouse >= 512 )  ynouse = 0;
	   //	  nColor = random(255)+1;
	   //	  jun_draw_rect(  0,  ynouse, 1024, yheight, nColor, TRUE);
	   JUN_draw_rect(  0,  ynouse, 1024, yheight, sstart%15, TRUE);
	   ynouse += yheight;
	   wait_vert_retrace_start();
	   set_vert_scroll_reg((BYTE)sstart);
	   sstart += yheight/2;
	}
     }

    // 11/06/93 Tang add to enable interrupt
    JUN_enable_vert_sync_int(TRUE);

    outportb(0x80,(BYTE)gulEnterIntCount);
    outportb(0x20, 0x20); // send EOI to 8259]
//    myoutportb(0x80,(BYTE)gulEnterIntCount);
//    myoutportb(0x20, 0x20); // send EOI to 8259]
  }
  else {
      /* case 1. run old ISR */
     (*old_interrupt)();	// call old interrupt function
     /* 11/11/93 Tang Note ! After old_interrupt has return, almost all
	IRQs's ISR has the IMR register changed  to original state, only when
	use IRQ 4 and load mouse driver is good. */

     /* case 2. escape old ISR, send EOI to 8259 directly */
//     outportb(0x20, 0x20); // send EOI to 8259

     imr_value = inportb(0x21);

     // 11/11/93 Tang added
     SetInterruptMask(gConfig.bIRQUsed);
  }

  return;
} // end function void interrupt  new_interrupt(void)


/* 11/10/93 Tang added. It is because affer ISR is done, the mask of IMR
   is restore to original state, so it must be set before every run */
BYTE NEAR  SetInterruptMask(BYTE bIRQ)
{
  BYTE	new_mask, current_mask;

  new_mask = (1 << bIRQ);           // bit n to mask (PIC mask)
  new_mask = ~new_mask;             // inverse of mask
  disable();		            // enter critical region
  // program 8259 IMR register, use OCW1 to unmask interrupt level
  current_mask = inportb(0x21);     // get original IMR
  new_mask &= current_mask;         // unmask interrupt level
  if ( ! (new_mask == current_mask) )   //don't set the same state again
     outportb(0x21, new_mask);
//  myoutportb(0x21, new_mask);
  enable();		    // leave critical region
  return(current_mask);
}

//
// ===========================================================================


int FAR PASCAL test_vert_sync_int(void)
{
   time_t  first, second;
   int count, errflag;

   errflag = FALSE;

   init_interrupt_test(gConfig.bIRQUsed);
   /* Step 1. Test Vsync period by polling status reg. bit 4 */
   count = 0;
//   wait_vert_retrace_start();
   first = time(NULL);	/* Gets system time */
   do {
      /* polling Vsync to ask Vsync is generated */
//	while ( ! during_vert_sync_period() ) ;
      wait_vert_retrace_start();
      count++;
      second = time(NULL); /* Gets system time again */
   /* calculates the elapsed time in seconds, return as a double */
  } while ( difftime(second,first) < 2 ); //wait for 1 second


  /* Step 2. Test Vsync interrupt with ISR(junaux.c) */

//  set_int_mask(gConfig.bIRQUsed);	 // set interrupt mask

  gulEnterIntCount = 0;
  count = 0;

  // enable interrupt
  // 11/05/93 Tang masked
  JUN_enable_vert_sync_int(TRUE);
  // 11/05/93 Tang masked

  /* wait vertical sync pulse start */
//  wait_vert_retrace_start();
  first = time(NULL);  /* Gets system time */
  do {
//     if ( ! JUN_vert_sync_int_occur() )  { //INTEN is cleared
	// wait_vert_retrace_start();  // if int occur, break & wait too long
	// delay(1);
//	JUN_enable_vert_sync_int(TRUE);
	count++ ;
//     }
     second = time(NULL); /* Gets system time again */
     /* calculates the elapsed time in seconds, return as a double */
  } while ( difftime(second,first) < 2 ); //wait for 1 second

  // disable interrupt
  disable();
  // 11/05/93 Tang masked
  JUN_enable_vert_sync_int(FALSE);
  // 11/05/93 Tang masked
  enable();

  /* Fv = 60 Hz, so 60 times at 1 second period, error allow to 1 */
  if ( (gulEnterIntCount>=125) || (gulEnterIntCount<=115) ) {
     errflag = TRUE;;
  }

  restore_interrupt(gConfig.bIRQUsed);

  return( (errflag^=1) );     //excursive-or XOR
}



/************************************************************************
* JUN_isr_scrolling
*
*   ISR for scrolling pages.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
static void interrupt  JUN_isr_scrolling(void)
{
   int  i, imr_value;

//  gulEnterIntCount++;
  /* if the interrupt is generated by our JUNIOR IRQ ? */
  if ( JUN_vert_sync_int_occur() && gConfig.bIRQUsed ) {
#ifdef TEST_VER
     /* play sound to attrative */
     music();
#endif
     /* got an interrupt */
     gulEnterIntCount++;
     /* disable interrupt */
     JUN_enable_vert_sync_int(FALSE);
     /* enable interrupt immediately */
//     JUN_enable_vert_sync_int(TRUE);

     /* need to do scrolling ? */
     if ( gfHandleScroll && gfBufferFull[gfSwitch_ISR] )
     {
        /* the execution of prior ISR finished ? Yes, do new job.
           No, return back to continue. */
        if ( gfPriIsrFinish ) {
           gfPriIsrFinish = FALSE;
	   switch ( gbScrollType[gfSwitch_ISR] )
	   {
	     case SCROLL_UP :
		/* Step 1. Scrolling bitmap into video memory */
		/* write proper lines of bitmap image into video memory */
		if ( gwPieceCount < gwVideoHeight )
		{
                   /* write gwRegGaps*2 lines per times */
		   for ( i=0; i<gwLineGaps[gfSwitch_ISR]; i++ )
		   {
                      /* reset proper start line to put image data into */
		      if ( gwVramStart[gfSwitch_ISR] >= VIDEO_MEMORY_HEIGHT )
			 gwVramStart[gfSwitch_ISR] = 0;
                      /* write data into video memory */
                      outportb(0x80,(BYTE)gwVramStart[gfSwitch_ISR]);
                      VertWrite(gwxVmemLeft, gwyVmemTop,
                                gwxPageWidth[gfSwitch_ISR], gwyPageHeight[gfSwitch_ISR],
                                gwyChangeTop[gfSwitch_ISR], 1,
                                (UINT)gwVramStart[gfSwitch_ISR],
                                (LPSTR)ghpBits[gfSwitch_ISR] );
                      gwyChangeTop[gfSwitch_ISR]++;
		      gwVramStart[gfSwitch_ISR]++;
		      gwPieceCount++;
		   } /* end of for */
		}
                /* Step 2. Set Scan-Start register */
                /* update vert scan start value */
		gwRegTempStart[gfSwitch_ISR] += gwRegGaps[gfSwitch_ISR];
                /* need to reset vert scan start reg. ? */
		if ( gwRegTempStart[gfSwitch_ISR] > 0xff )
		   gwRegTempStart[gfSwitch_ISR] &= 0x00ff;
                /* set vert scroll reg. */
		gbVertScanStart = (BYTE)gwRegTempStart[gfSwitch_ISR];
		wait_vert_retrace_start();
		set_vert_scroll_reg(gbVertScanStart);

                /* End of scrolling ? Yes, set flag */
		if ( gwPieceCount >= gwVideoHeight )
//		if ( gwPieceCount >= gwVideoHeight )
                {
                   gwPieceCount = 0;
                   /* set new vert scan start value for later use */
                   gConfig.wVertScanStart = (WORD)gbVertScanStart & 0x00ff;
                   gfBufferFull[gfSwitch_ISR] = FALSE;
		   if ( gfScrollStop ) {    //last page
                      /* scrolling is done, return back */
		      gfHandleScroll = FALSE;
                      JUN_enable_vert_sync_int(FALSE);
                   }
                   else {
                      /* set another page */
                      gfSwitch_ISR ^= 1;    //exclusive OR with 1
                   }
                }

		break;
          
	     case SCROLL_DOWN :
		/* Step 1. Scrolling bitmap into video memory */
		/* write proper lines of bitmap image into video memory */
		if ( gwPieceCount < gwVideoHeight )
		{
                   /* write gwRegGaps*2 lines per times */
		   for ( i=0; i<gwLineGaps[gfSwitch_ISR]; i++ )
		   {
                      /* reset proper start line to put image data into */
		      if ( gwVramStart[gfSwitch_ISR] < 0 )
			 gwVramStart[gfSwitch_ISR] = (int)(VIDEO_MEMORY_HEIGHT - 1);
                      outportb(0x80,(BYTE)gwVramStart[gfSwitch_ISR]);
                      /* write data into video memory */
                      VertWrite(gwxVmemLeft, gwyVmemTop,
                                gwxPageWidth[gfSwitch_ISR], gwyPageHeight[gfSwitch_ISR],
                                gwyChangeTop[gfSwitch_ISR], 1,
                                (UINT)gwVramStart[gfSwitch_ISR],
                                (LPSTR)ghpBits[gfSwitch_ISR] );
                      gwyChangeTop[gfSwitch_ISR]--;
		      gwVramStart[gfSwitch_ISR]--;
		      gwPieceCount++;
		   } /* end of for */
		}

                /* Step 2. Set Scan-Start register */
                /* update vert scan start value */
		gwRegTempStart[gfSwitch_ISR] -= gwRegGaps[gfSwitch_ISR];
                /* need to reset vert scan start reg. ? */
		if ( gwRegTempStart[gfSwitch_ISR] < 0 )
		   gwRegTempStart[gfSwitch_ISR] &= 0x00ff;
                /* set vert scroll reg. */
		gbVertScanStart = (BYTE)gwRegTempStart[gfSwitch_ISR];
		wait_vert_retrace_start();
		set_vert_scroll_reg(gbVertScanStart);

                /* End of scrolling ? Yes, set flag */
		if ( gwPieceCount >= gwVideoHeight )
                {
                   gwPieceCount = 0;
                   /* set new vert scan start value for later use */
                   gConfig.wVertScanStart = (WORD)gbVertScanStart & 0x00ff;
                   gfBufferFull[gfSwitch_ISR] = FALSE;
		   if ( gfScrollStop ) {    //last page
                      /* scrolling is done, return back */
		      gfHandleScroll = FALSE;
                      JUN_enable_vert_sync_int(FALSE);
                   }
                   else {
                      /* set another page */
                      gfSwitch_ISR ^= 1;    //exclusive OR with 1
                   }
                }

                break;

	     case SCROLL_LEFT :
		/* Step 1. Scrolling bitmap into video memory */
		/* write proper lines of bitmap image into video memory */
		if ( gwPieceCount < gwVideoWidth )
		{
                   /* write gwRegGaps*2 lines per times */
//		   for ( i=0; i<gwRegGaps[gfSwitch_ISR]*2; i++ )
//		   {
                      /* reset proper start line to put image data into */
		      if ( gwVramStart[gfSwitch_ISR] >= VIDEO_MEMORY_WIDTH )
			 gwVramStart[gfSwitch_ISR] = 0;
                       //11/30/93 Tang, write yVramTop lines color key first
                      outportb(0x80,(BYTE)gwVramStart[gfSwitch_ISR]);
                      /* write data into video memory */
                      HorzWrite(gwxVmemLeft, gwyVmemTop,
                                gwxPageWidth[gfSwitch_ISR], gwyPageHeight[gfSwitch_ISR],
                                gwxChangeLeft[gfSwitch_ISR], gwLineGaps[gfSwitch_ISR],
                                (UINT)gwVramStart[gfSwitch_ISR],
                                (LPSTR)ghpBits[gfSwitch_ISR] );
                      gwxChangeLeft[gfSwitch_ISR] += gwLineGaps[gfSwitch_ISR];
		      gwVramStart[gfSwitch_ISR] += gwLineGaps[gfSwitch_ISR];
		      gwPieceCount += gwLineGaps[gfSwitch_ISR];
//		   } /* end of for */
		}
                /* Step 2. Set Scan-Start register */
                /* update vert scan start value */
		gwRegTempStart[gfSwitch_ISR] += gwRegGaps[gfSwitch_ISR];
                /* 11/26/93 Tang updated */
                gwRegTempStart[gfSwitch_ISR] %= 0x200;
	        wait_vert_retrace_start();
	        set_horz_shift_reg(gwRegTempStart[gfSwitch_ISR]);
//	        set_horz_shift_reg1(gwRegTempStart[gfSwitch_ISR]);

                /* End of scrolling ? Yes, set flag */
		if ( gwPieceCount >= gwVideoWidth )
                {
                   gwPieceCount = 0;
                   /* set new horz scan start value for later use */
                   gConfig.wHorzScanStart = gwRegTempStart[gfSwitch_ISR] & 0x01ff;
//                   gConfig.wHorzScanStart = (WORD)gbHorzScanStart;
                   gfBufferFull[gfSwitch_ISR] = FALSE;
		   if ( gfScrollStop ) {    //last page
                      /* scrolling is done, return back */
		      gfHandleScroll = FALSE;
                      JUN_enable_vert_sync_int(FALSE);
                   }
                   else {
                      /* set another page */
                      gfSwitch_ISR ^= 1;    //exclusive OR with 1
                   }
                }

		break;

	     case SCROLL_RIGHT :
		/* Step 1. Scrolling bitmap into video memory */
		/* write proper lines of bitmap image into video memory */
		if ( gwPieceCount < gwVideoWidth )
		{
                   /* write gwRegGaps*2 lines per times */
//		   for ( i=0; i<gwRegGaps[gfSwitch_ISR]*2; i++ )
//		   {
                      /* reset proper start line to put image data into */
		      if ( gwVramStart[gfSwitch_ISR] < 0 )
			 gwVramStart[gfSwitch_ISR] = (int)(VIDEO_MEMORY_WIDTH - 1);
                      outportb(0x80,(BYTE)gwVramStart[gfSwitch_ISR]);
                      /* write data into video memory */
                      HorzWrite(gwxVmemLeft, gwyVmemTop,
                                gwxPageWidth[gfSwitch_ISR], gwyPageHeight[gfSwitch_ISR],
                                gwxChangeLeft[gfSwitch_ISR], gwLineGaps[gfSwitch_ISR],
                                (UINT)gwVramStart[gfSwitch_ISR],
                                (LPSTR)ghpBits[gfSwitch_ISR] );
                      gwxChangeLeft[gfSwitch_ISR] -= gwLineGaps[gfSwitch_ISR];
		      gwVramStart[gfSwitch_ISR] -= gwLineGaps[gfSwitch_ISR];
		      gwPieceCount += gwLineGaps[gfSwitch_ISR];
//		   } /* end of for */
		}

                /* Step 2. Set Scan-Start register */
                /* update vert scan start value */
		gwRegTempStart[gfSwitch_ISR] -= gwRegGaps[gfSwitch_ISR];
                /* need to reset vert scan start reg. ? */
                /* 11/26/93 Tang updated */
                gwRegTempStart[gfSwitch_ISR] &= 0x01ff;
	        wait_vert_retrace_start();
	        set_horz_shift_reg(gwRegTempStart[gfSwitch_ISR] );
//	        set_horz_shift_reg1(gwRegTempStart[gfSwitch_ISR] );

                /* End of scrolling ? Yes, set flag */
		if ( gwPieceCount >= gwVideoWidth )
                {
                   gwPieceCount = 0;
                   /* set new horz scan start value for later use */
                   gConfig.wHorzScanStart = gwRegTempStart[gfSwitch_ISR] & 0x01ff;
//                   gConfig.wHorzScanStart = (WORD)gbHorzScanStart;
                   gfBufferFull[gfSwitch_ISR] = FALSE;
		   if ( gfScrollStop ) {    //last page
                      /* scrolling is done, return back */
		      gfHandleScroll = FALSE;
                      JUN_enable_vert_sync_int(FALSE);
                   }
                   else {
                      /* set another page */
                      gfSwitch_ISR ^= 1;    //exclusive OR with 1
                   }
                }

                break;

	     case SCROLL_NONE :
		/* Step 1. Scrolling bitmap into video memory */
		/* write proper lines of bitmap image into video memory */
		if ( gwPieceCount < gwVideoHeight )
		{
                   /* write gwRegGaps*2 lines per times */
		   for ( i=0; i<gwLineGaps[gfSwitch_ISR]; i++ )
		   {
                      /* reset proper start line to put image data into */
		      if ( gwVramStart[gfSwitch_ISR] >= VIDEO_MEMORY_HEIGHT )
			 gwVramStart[gfSwitch_ISR] = 0;
                      /* write one line of image data into video memory */
		      if ( gwPieceCount < gwyPageHeight[gfSwitch_ISR] )
		      {
//12/29                         outportb(0x80,(BYTE)gwPieceCount);
			 HandleImageRect1(gwxVmemLeft, (UINT)gwVramStart[gfSwitch_ISR],
                               gwxShowWidth[gfSwitch_ISR], 1,
                               gwxShowLeft[gfSwitch_ISR], gwyChangeTop[gfSwitch_ISR],
                               gwxPageWidth[gfSwitch_ISR], gwyPageHeight[gfSwitch_ISR],
                               (LPSTR)ghpBits[gfSwitch_ISR], H_WRITE);
		      }
		      else
		      {
			 /* write color-key into video memory */
			 JUN_ClearImageRect(gwxVmemLeft, gwVramStart[gfSwitch_ISR],
                                  gwxShowWidth[gfSwitch_ISR], 1,
                                  gConfig.bColorKey);
		      }
		      gwVramStart[gfSwitch_ISR]++;
		      gwPieceCount++;
                      gwyChangeTop[gfSwitch_ISR]++;
		   } /* end of for */
		}
                /* End of scrolling ? Yes, set flag */
		if ( gwPieceCount >= gwVideoHeight )
                {
                   gwPieceCount = 0;
                   gfBufferFull[gfSwitch_ISR] = FALSE;
		   if ( gfScrollStop ) {    //last page
                      /* scrolling is done, return back */
		      gfHandleScroll = FALSE;
                      JUN_enable_vert_sync_int(FALSE);
                   }
                   else {
                      /* set another page */
                      gfSwitch_ISR ^= 1;    //exclusive OR with 1
                   }
                }
                break;

	   } /* end of switch */

           /* At this time, JUN_ScrollPages begins to load page, while ISR is
              ready for prcossing next page */
           /* IF a page is scrolling done, set scroll/shift scan start register */
	   if ( (gwPieceCount == 0) && !(gfBufferFull[gfSwitch_ISR ^ 1]) ) {
              switch ( gbScrollType[gfSwitch_ISR] ) {
                 case SCROLL_UP :
                      /* in video memory, fix x, variable y */
                      gwyChangeTop[gfSwitch_ISR] = 0;
                      /* 11/25/93 Tang, y start must relative to yVramTop */
                      gwVramStart[gfSwitch_ISR] = gwVideoHeight;
                      /* set scroll/shift scan start register value */
                      gwRegTempStart[gfSwitch_ISR] = gConfig.wVertScanStart;
                      break;

                 case SCROLL_DOWN :
                      /* in video memory, fix x, variable y */
                      gwyChangeTop[gfSwitch_ISR] = gwVideoHeight - 2;
//                      gwyChangeTop[gfSwitch_ISR] = gwyPageHeight[gfSwitch_ISR] - 1;
                      /* 11/25/93 Tang, y start must relative to yVramTop, but
                         this can be shown at the screen while writing color key */
                      // gwVramStart[gfSwitch_ISR] = yVramTop - 1;
                      gwVramStart[gfSwitch_ISR] = (int)(VIDEO_MEMORY_HEIGHT - 2);
//                      gwVramStart[gfSwitch_ISR] = (int)(VIDEO_MEMORY_HEIGHT - 1);
                      /* set scroll/shift scan start register value */
                      gwRegTempStart[gfSwitch_ISR] = gConfig.wVertScanStart;
                      break;

                 case SCROLL_LEFT :
                      /* 11/25/93 Tang, x start must relative to xVramLeft */
                      /* in video memory, fix y, variable x */
                      gwVramStart[gfSwitch_ISR] = gwVideoWidth;
                      gwxChangeLeft[gfSwitch_ISR] = 0;
                      /* set scroll/shift scan start register value */
                      gwRegTempStart[gfSwitch_ISR] = gConfig.wHorzScanStart;
                      break;

                 case SCROLL_RIGHT :
                      /* in video memory, fix y, variable x */
                      /* 11/25/93 Tang, x start must relative to xVramLeft, but
                         this can be shown at the screen while writing color key */
                      // gwVramStart[gfSwitch_ISR] = xVramLeft - 1;
                      gwVramStart[gfSwitch_ISR] = (int)(VIDEO_MEMORY_WIDTH - 2);
//                      gwVramStart[gfSwitch_ISR] = (int)(VIDEO_MEMORY_WIDTH - 1);
                      gwxChangeLeft[gfSwitch_ISR] = gwVideoWidth - 2;
//                      gwxChangeLeft[gfSwitch_ISR] = gwxPageWidth[gfSwitch_ISR] - 1;
                       /* set scroll/shift scan start register value */
                       gwRegTempStart[gfSwitch_ISR] = gConfig.wHorzScanStart;
                       break;

                 case SCROLL_NONE :
                      gwyChangeTop[gfSwitch_ISR] = 0;
                      gwVramStart[gfSwitch_ISR]  = 0; //each y pos. in Vmem
                      break;

               } /* end of switch */
           }

           gfPriIsrFinish = TRUE;

        } /* end of if (gfPriIsrFinish) */
     } /* end of if (gfScroll) */

     if ( gfHandleScroll )
        /* enable interrupt immediately */
        JUN_enable_vert_sync_int(TRUE);

//     outportb(0x80,(BYTE)gulEnterIntCount);
     outportb(0x20, 0x20); // send EOI to 8259]
//     myoutportb(0x20, 0x20); // send EOI to 8259]
  } /* end of vert_sync_occur */
  else {
     outportb(0x80, 0xFF);
      /* case 1. run old ISR */
     (*old_interrupt)();	// call old interrupt function
     /* 11/11/93 Tang Note ! After old_interrupt has return, almost all
	IRQs's ISR has the IMR register changed  to original state, only when
	use IRQ 4 and load mouse driver is good. */

     /* case 2. escape old ISR, send EOI to 8259 directly */
//     outportb(0x20, 0x20); // send EOI to 8259

     imr_value = inportb(0x21);

     // 11/11/93 Tang added
     SetInterruptMask(gConfig.bIRQUsed);
  }

  return;
} // end function void interrupt  new_interrupt(void)



int FAR  VertWrite(UINT xLeft,  UINT yTop,      //(left,top)
                   UINT xWidth, UINT yHeight,
                   UINT yWrite, UINT yLines,
                   UINT VramPos,
		   LPSTR lpImageBuf)            //r/w buffer
{
   BYTE	  memory_segment, bank_no;
   UINT	  y, start_line, rel_x, rel_y;
   DWORD  dwSegBoundary;
   BYTE huge * hpImageBuf;
   LPSTR  lpVideo;
   UINT   i, j;

   if ( xWidth > VIDEO_MEMORY_WIDTH || yHeight > VIDEO_MEMORY_HEIGHT ) {
//      MessageBox(hWnd, "Out of range !", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }

   /* 11/20/93 Tang added */
   if ( (xWidth == 0)  || (yHeight == 0) )
      return(TRUE);
   /* 11/20/93 Tang added */

   /* convert to huge pointer */
   if ( yWrite >= yTop && yWrite < yTop+yHeight ) {
      hpImageBuf = lpImageBuf;
      hpImageBuf += (DWORD)(yWrite-yTop)*xWidth;
   }

   if ( yWrite < yTop )
      JUN_ClearImageRect(0, VramPos, gwVideoWidth, yLines, gConfig.bColorKey);
   else if ( yWrite >= yTop && yWrite < yTop+yHeight ) {
   for ( y=VramPos; y<VramPos+yLines; y++) {     //Process one line per times
      /* write xTVshift color key */
      JUN_ClearImageRect(0, y, xLeft, 1, gConfig.bColorKey);
      /* get relative x location */
      rel_x = fnRelX(xLeft);

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
      xLength[i++] = xWidth;
      xLength[i--] = 0;
      if ( ( rel_x + xWidth ) > VIDEO_MEMORY_WIDTH ) { //3/16 added =
         xLength[i++] = VIDEO_MEMORY_WIDTH - rel_x;
         xLength[i] = xWidth - xLength[i-1];
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
//01/04/94 no need      hpImageBuf += (DWORD)xWidth;

      JUN_ClearImageRect(xLeft+xWidth, y, gwVideoWidth-xLeft-xWidth, 1, gConfig.bColorKey);

   } /* end of for */
   }
   else if ( yWrite >= yTop+yHeight )
      JUN_ClearImageRect(0, VramPos, gwVideoWidth, yLines, gConfig.bColorKey);


   return(TRUE);
} /* end of HandleImageRect() */



int FAR  HorzWrite(UINT xLeft,  UINT yTop,      //(left,top)
                   UINT xWidth, UINT yHeight,
                   UINT xWrite, UINT xLines,
                   UINT VramPos,
		   LPSTR lpImageBuf)            //r/w buffer
{
   BYTE	  memory_segment, bank_no;
   UINT	  y, start_line, rel_x, rel_y;
   DWORD  dwSegBoundary;
   BYTE huge * hpImageBuf;
   LPSTR  lpVideo;
   UINT   i, j;

   if ( xWidth > VIDEO_MEMORY_WIDTH || yHeight > VIDEO_MEMORY_HEIGHT ) {
//      MessageBox(hWnd, "Out of range !", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }

   /* 11/20/93 Tang added */
   if ( (xWidth == 0) || (yHeight == 0) )
      return(TRUE);
   /* 11/20/93 Tang added */


   /* convert to huge pointer */
   if ( xWrite >= xLeft && xWrite < xLeft+xWidth ) {
      hpImageBuf = lpImageBuf;
      hpImageBuf += (DWORD)(xWrite-xLeft);
   }

   if ( xWrite < xLeft )
      JUN_ClearImageRect(VramPos, 0, xLines, gwVideoHeight, gConfig.bColorKey);
   else if ( xWrite >= xLeft && xWrite < xLeft+xWidth ) {
   for ( y=0; y<gwVideoHeight; y++) {     //Process one line per times
      if ( y < yTop ) {
         JUN_ClearImageRect(VramPos, y, xLines, 1, gConfig.bColorKey);
      }
      else if ( y >= yTop && y < yTop + yHeight ) {
      /* get relative x location */
      rel_x = fnRelX(VramPos);

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
      xLength[i++] = xLines;
      xLength[i--] = 0;
      if ( ( rel_x + xLines ) > VIDEO_MEMORY_WIDTH ) { //3/16 added =
         xLength[i++] = VIDEO_MEMORY_WIDTH - rel_x;
         xLength[i] = xLines - xLength[i-1];
      }

      for ( j=0; j<=i; j++) {
         // length which write to selector must be smaller than 16K //
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
      hpImageBuf += (DWORD)xWidth - xLines;
      }
      else {
         JUN_ClearImageRect(VramPos, y, xLines, 1, gConfig.bColorKey);
      }

   } /* end of for */
   }
   else if ( xWrite >= xLeft+xWidth )
      JUN_ClearImageRect(VramPos, 0, xLines, gwVideoHeight, gConfig.bColorKey);


   return(TRUE);
} /* end of HandleImageRect() */




// end of junisr.c //