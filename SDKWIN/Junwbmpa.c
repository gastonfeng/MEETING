/************************************************************************
* Project : junior.dll
* Module  : junwrbmp.c
* Object  : load bitmap from VB's hDC and write into video memory
* Author  : Kung-Puu Tang
* Date	  : 820920
************************************************************************/

#ifdef DOSLIB
  #include <fcntl.h>
  #include <sys\types.h>
  #include <sys\stat.h>
#else
  #include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>	//MK_FP
#include <string.h>
#include "juniora.h"
#include "externa.h"

HANDLE	ghdcMemory;  //Handle of Device Context of memory
//HANDLE	ghBitmap;
//LPSTR	glpPageBuf;
int	gwLoadCount=0;

/************************************************************************
* JUN_ReadImageRect(xLeft, yTop, xWidth, yHeight, lpImageBuf)
*
*   This routine is called to read a specified rectangle in the video
* memory to a output image buffer for CPU.
*
* Exported:  Yes
* Entry:     A specified rect.- (x,y,width,height) & output buffer
* Destroys:
* Exit:      TRUE for success, FALSE for failure
************************************************************************/
int FAR PASCAL JUN_ReadImageRect(UINT xLeft,  UINT yTop,
				 UINT xWidth, UINT yHeight,
				 LPSTR lpImageBuf)
{
   if ( ! HandleImageRect(xLeft, yTop, xWidth, yHeight, lpImageBuf, H_READ) ) {
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }
   return(TRUE);
}


/************************************************************************
* JUN_WriteImageRect(xLeft, yTop, xWidth, yHeight, lpImageBuf)
*
*   This routine is called to write a specified rectangle from the
* input image buffer to the video memory for CPU.
*
* Exported:  Yes
* Entry:     A specified rect.- (x,y,width,height) & input buffer
* Destroys:
* Exit:      TRUE for success, FALSE for failure
************************************************************************/
int FAR PASCAL JUN_WriteImageRect(UINT xLeft,  UINT yTop,
				  UINT xWidth, UINT yHeight,
				  LPSTR lpImageBuf)
{
   if ( ! HandleImageRect(xLeft, yTop, xWidth, yHeight, lpImageBuf, H_WRITE) ) {
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }
   return(TRUE);
}

/************************************************************************
* JUN_ClearImageRect(xLeft, yTop, xWidth, yHeight, bColorEntry)
*
*   This routine is called to clear a specified rectangle in the video
* memory with a specified color entry.
*
* Exported:  Yes
* Entry:     A specified rect.- (x,y,width,height), color-RAMDAC entry
* Destroys:
* Exit:      TRUE for success, FALSE for failure
************************************************************************/
int FAR PASCAL JUN_ClearImageRect(UINT xLeft,  UINT yTop,
				  UINT xWidth,	UINT yHeight,
				  BYTE bColorEntry )
{
   BYTE   memory_segment, bank_no;
   static BYTE	prev_memory_segment=10, prev_bank_no=9;
   UINT   x, y, start_line, rel_x, rel_y;

//   if ( (xLeft+xWidth) > VIDEO_MEMORY_WIDTH || (yTop+yHeight) > VIDEO_MEMORY_HEIGHT ) {
   if ( xWidth > VIDEO_MEMORY_WIDTH || yHeight > VIDEO_MEMORY_HEIGHT ) {
//	MessageBox(NULL, "Out of range !", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }

   /* 11/20/93 Tang added */
   if ( (xWidth == 0)  || (yHeight == 0) )
      return(TRUE);
   /* 11/20/93 Tang added */

   if ( xLeft==0 && yTop==0 && xWidth==gwVideoWidth && yHeight==gwVideoHeight )
      return( JUN_flash_video_memory(bColorEntry) );

   /* get relative x location */
   rel_x = fnRelX(xLeft);

   for ( y=yTop; y<yTop+yHeight; y++) {     //Process one line per times
      /* get relative y location */
      rel_y = fnRelY(y);
      /* (xLeft, y) locate at which segment */
      memory_segment = (BYTE)(rel_y >> 6);  // div 64 - segment no.
      /* start position in a segment */
      start_line = (UINT)rel_y % LINES_PER_SEGMENT;

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

      if ( ( rel_x + xWidth ) > VIDEO_MEMORY_WIDTH ) {
	 for ( x=rel_x; x<VIDEO_MEMORY_WIDTH; x++ )
	    glpVideoMemory[(start_line<<10) + x] = bColorEntry;
	 for ( x=0; x<xWidth-(VIDEO_MEMORY_WIDTH - rel_x); x++ )
	    glpVideoMemory[(start_line<<10) + x] = bColorEntry;
      }
      else
	 for ( x=rel_x; x<rel_x+xWidth; x++ )
	    glpVideoMemory[(start_line<<10) + x] = bColorEntry;
   }
   return(TRUE);

}

//UINT	xLength[2]={0};   //two section if beyond 1024 boundary
/****************************************************************************
* HandleImageRect(xLeft, yTop, xWidth, yHeight, lpImageBuf, nType)
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
int FAR  HandleImageRect(UINT xLeft,  UINT yTop,      //(left,top)
			 UINT xWidth, UINT yHeight,   //(width,height)
			 LPSTR lpImageBuf,	      //r/w buffer
			 UINT nType)		      //operation type
{
   BYTE   memory_segment, bank_no;
   static BYTE	prev_memory_segment=10, prev_bank_no=9;
   UINT   x, y, padded, start_line, rel_x, rel_y;
   DWORD  dwSegBoundary;
   BYTE huge * hpImageBuf;
   LPSTR  lpVideo;
   UINT   i, j;

//   if ( (xLeft+xWidth) > VIDEO_MEMORY_WIDTH || (yTop+yHeight) > VIDEO_MEMORY_HEIGHT ) {
   if ( xWidth > VIDEO_MEMORY_WIDTH || yHeight > VIDEO_MEMORY_HEIGHT ) {
//	MessageBox(hWnd, "Out of range !", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
      return(gwErrorCode);
   }

   /* 11/20/93 Tang added */
   if ( (xWidth == 0)  || (yHeight == 0) )
      return(TRUE);
   /* 11/20/93 Tang added */

//   if ( nType == H_READ ) {
//	gbMemReadMode = 0;    //Enable
//	memory_read_enable(gbMemReadMode);
//   }
//   else {		   //H_WRITE or H_CLEAR
//	gbMemWriteMode = 0;   //Enable
//	memory_write_enable(gbMemWriteMode);
//   }

   /* get relative x location */
   rel_x = fnRelX(xLeft);

   /* convert to huge pointer */
   hpImageBuf = lpImageBuf;

   for ( y=yTop; y<yTop+yHeight; y++) {     //Process one line per times
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
      xLength[i++] = xWidth;
      xLength[i--] = 0;
      if ( ( rel_x + xWidth ) > VIDEO_MEMORY_WIDTH ) { // 3/16 add =
	 xLength[i++] = VIDEO_MEMORY_WIDTH - rel_x;
	 xLength[i] = xWidth - xLength[i-1];
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
		    FarMemoryCopy((DWORD)hpImageBuf, (DWORD)lpVideo, (WORD)dwSegBoundary);
		    hpImageBuf += dwSegBoundary;
		    FarMemoryCopy((DWORD)lpVideo+dwSegBoundary, (DWORD)hpImageBuf,
				  xLength[j]-(WORD)dwSegBoundary);
		    hpImageBuf += (xLength[j] - (WORD)dwSegBoundary);
		 }
	      }
	      /* padded 0 to satisfy 2-byte boundary,
		 not only for video memory, but also Bitmap file itself */
//		if ( (padded=(xWidth%2)) ) {
//		   for ( x=0; x<(2-padded); x++)   //no. of zero to padded
//		      glpVideoMemory[(start_line<<10) + (xLeft+xWidth+x)] = 0;
//		   hpImageBuf += (2-padded);
///		 }
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
	      /* padded 0 to satisfy 2-byte boundary,
		 not only for video memory, but also Bitmap file itself */
//		if ( (padded=(xWidth%4)) ) {
//		   for ( x=0; x<(2-padded); x++)   //no. of zero to padded
//		      glpVideoMemory[(start_line<<10) + (xLeft+xWidth+x)] = gConfig.bColorKey;
//		   hpImageBuf += (2-padded);
//		}
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
//		lpVideo = GetVideoAddress(rel_x, start_line);
//		for ( x=xLeft; x<xLeft+xWidth; x++ ) {
//		   *lpVideo = gConfig.bColorKey;
//		   lpVideo += 1;
//		}
//		for ( x=xLeft; x<xLeft+xWidth; x++ )
//		   glpVideoMemory[(start_line<<10) + x] = gConfig.bColorKey;
	      break;


      } /* end of switch */
   } /* end of for */
   return(TRUE);
} /* end of HandleImageRect() */


UINT FAR  fnRelX(UINT xPos)
{
   UINT origin_X, rel_x;

   /* get the original point of video memory, generally is (0,0) */
   origin_X = gConfig.wHorzScanStart << 1;   //*2
   rel_x = xPos + origin_X;
   return( (rel_x >= VIDEO_MEMORY_WIDTH) ?
      (UINT)(rel_x % VIDEO_MEMORY_WIDTH) : rel_x );
}


UINT FAR  fnRelY(UINT yPos)
{
   UINT origin_Y, rel_y;

   /* get the original point of video memory, generally is (0,0) */
//3/9/94   origin_Y = gConfig.wVertScanStart << 1;    //*2
   origin_Y = (gConfig.wVertScanStart - gwVertRegOffset) << 1;  //PAL
//   origin_Y = ((gwVertRegOffset - gConfig.wVertScanStart) & 0x01ff) << 1;  //PAL
   rel_y = yPos + origin_Y;
   return( (rel_y >= VIDEO_MEMORY_HEIGHT) ?
      (UINT)(rel_y % VIDEO_MEMORY_HEIGHT) : rel_y );
}


/************************************************************************
* CompactGlobalHeap(dwReqAllocSize)
*
*   Compact global heap to allocate a block of memory.
*
* Exit: TRUE  : Actually request size
*	FALSE : Insufficient memory
************************************************************************/
BOOL CompactGlobalHeap(DWORD dwReqAllocSize)
{
   DWORD dwActAllocSize;

   dwActAllocSize = GlobalCompact(dwReqAllocSize);
   if ( dwActAllocSize < dwReqAllocSize ) {
//      wsprintf(debugstr,"  [Actual %8ld<Required %8ld]", dwActAllocSize, dwReqAllocSize);
//      MessageBox(NULL,"Insufficient Memory !",
//		 debugstr, MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x87);
      return (FALSE);
   }
   return (TRUE);
}


/************************************************************************
* AllocateMemory(wSizeBytes)
*
* Allocates a block of memory of the specified size and returns a far
* pointer.
*
* Exit: Far pointer to memory block
*	= 0	Could not allocate memory.
************************************************************************/
char far * AllocateMemory(unsigned long wSizeBytes)
{
   char far *p;

#ifdef DOSLIB
//  #ifdef BorlandC
//    return( (char far *)calloc(wSizeBytes, sizeof(char)) );
//  #else
    if ( !(p = (char far *)halloc(wSizeBytes, sizeof(char))) )
       return(NULL);
    else return( (char far *)p );
//  #endif
#else
   if (!(ghMemory = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, wSizeBytes)))
      return(NULL);
   else return(GlobalLock(ghMemory));
#endif
}

/************************************************************************
* FreeMemory(char far * fpMemory, HANDLE hMem)
*
* Frees the specified memory block.
*
* Exit: None
************************************************************************/
void NEAR FreeMemory(char far *fpMemory, HANDLE hMem)
{
#ifdef DOSLIB
    hfree( ( (void huge *)fpMemory ) );
#else
    GlobalUnlock(hMem);
    GlobalFree(hMem);
#endif
}

/************************************************************************
* JUN_WriteBitmap6
*
*   Write the bitmap to video memory.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
int FAR PASCAL JUN_WriteBitmap6(HDC hDC, HPALETTE hPAL,
			       UINT xLeft, UINT yTop,
			       UINT xWidth, UINT yHeight)
{
   BYTE huge	*hpBits;
   int		i, wReturnCode;
   WORD         padded;

   wReturnCode = GetImageData(hDC, xWidth, yHeight, glpPageBuf);
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   GetAndSetPalette(hDC, hPAL);

   padded = xWidth % 2;

//   MessageBox(NULL, "Write to video memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   hpBits = (LPSTR)glpPageBuf;
//12/23   hpBits = (LPSTR)lpBits;
/*
      for ( i=0; i<yHeight; i++ ) {
	 outportb(0x80, i);
	 HandleImageRect(xLeft, yTop+i,
			 xWidth, 1, hpBits, H_WRITE);
	 hpBits += xWidth;
      }
*/
      for ( i=0; i<yHeight; i++ ) {
	 outportb(0x80, i);
	 if ( ! HandleImageRect(xLeft, yTop+i,
		    xWidth, 1, (LPSTR)hpBits, H_WRITE) )
	 {
	    return(ERR_ACCESS_VRAM_OUT_OF_RANGE);
	 }
	 // 11/04/93 Tang updated to make a multiple of 2-byte
	 hpBits += (DWORD) (xWidth + padded);
      }

   return(TRUE);
}


#define MAXREAD  32768		       /* Number of bytes to be read during */

/**************** PRIVATE ROUTINE TO READ MORE THAN 64K *********************/
/****************************************************************************
 *									    *
 *  FUNCTION   : lread(int fh, VOID FAR *pv, DWORD ul)			    *
 *									    *
 *  PURPOSE    : Reads data in steps of 32k till all the data has been read.*
 *									    *
 *  RETURNS    : 0 - If read did not proceed correctly. 		    *
 *		 number of bytes read otherwise.			    *
 *									    *
 ****************************************************************************/
DWORD PASCAL lread (int fh, VOID far *pv, DWORD ul)
{
    DWORD     ulT = ul;
    BYTE huge *hp = pv;

    while (ul > (DWORD)MAXREAD) {
	if (_lread(fh, (LPSTR)hp, (WORD)MAXREAD) != MAXREAD)
		return 0;
	ul -= MAXREAD;
	hp += MAXREAD;
    }
    if (_lread(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul)
	return 0;
    return ulT;
}
/**************** PRIVATE ROUTINE TO READ MORE THAN 64K *********************/



    char achFileName[128]={0};
    char str[255]={0};
    WORD offBits;			/* offset to the bits */
    HANDLE hDIBInfo = NULL;		/* the DIB header */



int FAR PASCAL JUN_WriteBitmap4(UINT xLeft, UINT yTop,LPSTR filenamestr)
{
    HFILE              fh;
    LPBITMAPINFOHEADER lpbi;
    OFSTRUCT	       of;
    BITMAPFILEHEADER   bf;
    WORD	       nNumColors;
    int 	       i, j, k, nOffsetX, nOffsetY;
    BYTE huge	       *hpBits;
   HBITMAP	      hBitmap, hbitmap, hOldBitmap;
   BITMAP	      Bitmap;
    int 	       xWidth, yHeight, wReturnCode;
    HDC 	    hMem,hMDC, hdcMemory,hDC;
   LPSTR	lpBits;
   long 	dwNumBytes;
   DWORD	dimen;
   BITMAPINFO FAR *lpbmi;
   DWORD	 dwDataSize;


    /*************************************************************
    *  Read DIB
    ***************************/

    achFileName[0] = 0; 	/* pass in NULL */
//fjt    MessageBox(NULL,filenamestr,"FILENAME",MB_ICONSTOP | MB_OK);
    strcat(achFileName,filenamestr);
    fh = OpenFile (achFileName, &of, OF_READ);
    if ( fh == HFILE_ERROR ) {
//12/03/93    if (fh == -1) {
	wsprintf(str,"文件未能打开: '%s'", (LPSTR)achFileName);
	MessageBox(NULL, str, "打开文件错误!", MB_ICONSTOP | MB_OK);
	return (FALSE);
    }

    hDIBInfo = GlobalAlloc(GMEM_MOVEABLE,
	 (DWORD)(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)));

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIBInfo);

    /* read the BITMAPFILEHEADER */
    if (sizeof (bf) != _lread (fh, (LPSTR)&bf, sizeof (bf)))
	goto ErrExit;

    if (bf.bfType != 0x4d42)	/* 'BM' */
	goto ErrExit;

    if (sizeof(BITMAPINFOHEADER) != _lread (fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)))
	goto ErrExit;

    /* !!!!! for now, don't even deal with CORE headers */
    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
	goto ErrExit;

    if (!(nNumColors = (WORD)lpbi->biClrUsed))
    {
	/* no color table for 24-bit, default size otherwise */
	if (lpbi->biBitCount != 24)
	    nNumColors = 1 << lpbi->biBitCount; /* standard size table */
    }

    /*	fill in some default values if they are zero */
    if (lpbi->biClrUsed == 0)
	lpbi->biClrUsed = (DWORD)nNumColors;

    if (lpbi->biSizeImage == 0)
    {
	lpbi->biSizeImage = ((((lpbi->biWidth * (DWORD)lpbi->biBitCount) + 31) & ~31) >> 3)
			 * lpbi->biHeight;
    }

    /* get a proper-sized buffer for header, color table and bits */
    GlobalUnlock(hDIBInfo);
    hDIBInfo = GlobalReAlloc(hDIBInfo, lpbi->biSize +
					nNumColors * sizeof(RGBQUAD) +
					lpbi->biSizeImage, 0);
    if (!hDIBInfo)	/* can't resize buffer for loading */
	goto ErrExit2;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIBInfo);

    /* read the color table */
    _lread (fh, (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBQUAD));

    /* offset to the bits from start of DIB header */
    offBits = (WORD)lpbi->biSize + nNumColors * sizeof(RGBQUAD);

    if (bf.bfOffBits != 0L)
    {
	_llseek(fh,bf.bfOffBits,SEEK_SET);
    }
    if (lpbi->biSizeImage == lread(fh, (LPSTR)lpbi + offBits, lpbi->biSizeImage))
	goto doit;

ErrExit:
    _lclose(fh);
    GlobalUnlock(hDIBInfo);
    GlobalFree(hDIBInfo);
ErrExit2:
    return(FALSE);


doit:
    i = GetDeviceCaps(hdcInfo, RASTERCAPS) ;

      /* Write palette to color RAMDAC */
//	ReversePalette((LPSTR)(lpbi) + lpbi->biSize);
//	JUN_init_ramdac_entry((LPSTR)(lpbi) + lpbi->biSize);

//	HandleImageRect(xLeft, yTop, lpbi->biWidth, lpbi->biHeight,
//		(LPSTR)lpbi + offBits, H_WRITE);
      hpBits = (LPSTR)lpbi + offBits;
      for ( i=0; i<lpbi->biHeight; i++ ) {
	HandleImageRect(xLeft, yTop+lpbi->biHeight-(i+1),
			lpbi->biWidth, 1, (LPSTR)hpBits, H_WRITE);   
//	  HandleImageRect(xLeft, yTop+lpbi->biHeight-(i+1),
//			  lpbi->biWidth, 1, (LPSTR)hpBits, H_WRITE);
	 hpBits += lpbi->biWidth;
      }

      /* Write palette to color RAMDAC */
      ReversePalette((LPSTR)(lpbi) + lpbi->biSize);
      JUN_init_ramdac_entry((LPSTR)(lpbi) + lpbi->biSize);

      /* 10/21/93 Tang added to test scrolling */
//	JUN_ScrollBitmap1((LPSTR)lpbi+offBits, lpbi->biWidth, lpbi->biHeight,
//			  SCROLL_UP, 2);
      /* 10/21/93 Tang added to test scrolling */
    /* BitBlt 10/26 */
//    nOffsetX = (VIDEO_MEMORY_WIDTH-lpbi->biWidth)/2;
//    nOffsetY = VIDEO_MEMORY_WIDTH/5*4;
//	 for ( i=0; i<lpbi->biHeight/2+1; i+=2 ) {
//	    for ( k=0; k<lpbi->biHeight; k++ ) {
//	       HandleImageRect(nOffsetX, nOffsetY-i,
//			  lpbi->biWidth, 1, (LPSTR)hpBits, H_WRITE);
//	       hpBits += lpbi->biWidth;
//	    }
//	 }
//    for ( j=0; j<VIDEO_MEMORY_WIDTH/lpbi->biWidth+1; j++)
//	 for ( i=0; i<VIDEO_MEMORY_HEIGHT/lpbi->biHeight+1; i++) {
//	    hpBits = (LPSTR)lpbi+offBits;
//	    for ( k=0; k<lpbi->biHeight; k++ ) {
//	       HandleImageRect(j*lpbi->biWidth, i*lpbi->biHeight+lpbi->biHeight-(k+1),
//			  lpbi->biWidth, 1, (LPSTR)hpBits, H_WRITE);
//	       hpBits += lpbi->biWidth;
//	    }
//	 }
/*    */

/*-------------------------------------------------------------------*/

   hpBits = (LPSTR)lpbi + offBits;
   hDC = GetDC(NULL);
   hMDC = CreateCompatibleDC(hDC);

   hBitmap = CreateDIBitmap(hDC,
			    (LPBITMAPINFOHEADER)lpbi,
			    CBM_INIT,
			    (LPSTR)hpBits,
			    (LPBITMAPINFO)lpbi,
			    DIB_RGB_COLORS);
   //paint image
   hOldBitmap=SelectObject(hMDC, hBitmap);
   xWidth = lpbi->biWidth;
   yHeight = lpbi->biHeight;

/*   if ( !BitBlt(hDC,
	  0,0,
	  xWidth,
	  yHeight,
	  hMDC,
	  0,0,
	  SRCCOPY) ) {
      ReleaseDC(NULL, hDC);
      DeleteDC(hMDC);
      return(FALSE);
   }              fjt-99-08-06*/
/* 12/10
//12/09 Tang, no need!   hdcMemory = CreateCompatibleDC(hDC);
   hbitmap = CreateCompatibleBitmap(hDC, xWidth, yHeight);

   if ( !hbitmap ) {
      outportb(0x80, 0x04);
      return(FALSE);
   }


   // 12.09.93 Tang note! hDC must be memory DC, so is hMDC 
//   hOldBitmap = SelectObject(hDC, hbitmap);
   hOldBitmap = SelectObject(hMDC, hbitmap);
//   hOldBitmap = SelectObject(hdcMemory, hbitmap);

   dimen = GetBitmapDimension(hbitmap);

12/10 Tang */

//   hOldBitmap = SelectObject(hdcMemory, hbitmap);

/*12/27
   if ( !GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap) ) {
//   if ( !GetObject(hOldBitmap, sizeof(BITMAP), (LPSTR)&Bitmap) ) {
	outportb(0x80, 0x05);
	return(FALSE);
   }

//   if (!(GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)) {
//	 ReleaseDC(NULL, hDC);
//	 return(FALSE);
//   }

//   MessageBox(NULL, "Alloc Memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);


   dwDataSize = (DWORD)Bitmap.bmWidth*Bitmap.bmHeight;
12/27*/
/*
   if ( ! CompactGlobalHeap(dwDataSize) )  return(FALSE);
   hMem = GlobalAlloc(GHND, dwDataSize);
//   hMem = GlobalAlloc(GHND, 400*300);
   if ( !hMem ) {
      outportb(0x80, 0x77);
      MessageBox(NULL, "Can't alloc memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      return(FALSE);
   }
   lpBits = GlobalLock(hMem);
*/
/*12/27
   outportb(0x80, 0x55);
//   MessageBox(NULL, "Get Bitmap", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   dwNumBytes = GetBitmapBits(hBitmap, (DWORD)Bitmap.bmWidth*Bitmap.bmHeight, glpPageBuf);
//12/23   dwNumBytes = GetBitmapBits(hBitmap, (DWORD)Bitmap.bmWidth*Bitmap.bmHeight, lpBits);
//12/10   dwNumBytes = GetBitmapBits(hOldBitmap, (DWORD)xWidth*yHeight, lpBits) ;

   if ( ! dwNumBytes ) {
      outportb(0x80, 0x66);
//12/23      GlobalUnlock(hMem);
//12/23      GlobalFree(hMem);
      return(FALSE);
   }
12/27 */

//1/6   if ( (wReturnCode=GetImageData(hMDC, xWidth, yHeight, glpPageBuf)) != TRUE )
//1/6      return(wReturnCode);
//1/6   GetAndSetPalette(hMDC, 0);

   //12/27/93 Tang for special effective
   i = 0;
   JUN_ShowImage(hMDC, 0,
		 48, 38,
		 xWidth, yHeight,
		 0, 0,
		 xWidth, yHeight,
		 i, 3);   
//   GetSystemPaletteEntries(hDC, 0, GetDeviceCaps(hDC, SIZEPALETTE), pe);

//   HandleImageRect(xLeft, yTop, bmp_test.bmWidth, bmp_test.bmHeight,
//		     lpBits, H_WRITE);

//   JUN_init_ramdac_entry(pe);

//12/23   GlobalUnlock(hMem);
//12/23   GlobalFree(hMem);
   SelectObject(hMDC,hOldBitmap);
   DeleteObject(hBitmap);
   ReleaseDC(NULL, hDC);
   DeleteDC(hMDC);
   DeleteDC(hdcMemory);


/*-------------------------------------------------------------------*/
    _lclose(fh);
    GlobalUnlock(hDIBInfo);
    GlobalFree(hDIBInfo);

   return(TRUE);
}

/****************************************************************************
* ReversePalette(PALETTEENTRY *ramdac_palette, PALETTEENTRY far *image_palette)
*
*   Reverse RGBQUAD(B.G.R.0) of DIBimap to PALETTEENTRY(R.G.B.0) format,
* in order write to RAMDAC. i.e., (BGR)->(RGB)
*
****************************************************************************/
void ReversePalette(PALETTEENTRY far *palette)
{
   BYTE  tempR, tempG, tempB;
   UINT  i;

   for (i = 0;	i < MAXCOLORENTRY;  i++) {
      tempR = palette[i].peBlue;
      tempG = palette[i].peGreen;
      tempB = palette[i].peRed;
      palette[i].peRed	 = tempR;
      palette[i].peGreen = tempG;
      palette[i].peBlue  = tempB;
   }

   return;
}

#ifdef TEST_VER
/************************************************************************
* JUN_WriteBitmap5
*
*   Write the bitmap to video memory.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
int FAR PASCAL JUN_WriteBitmap5(HBITMAP  hDC, HPALETTE hPAL,
				UINT xLeft, UINT yTop,
				UINT xWidth, UINT yHeight)
{
   BITMAP	Bitmap;
   HANDLE	hMem;
//   PALETTEENTRY pe[MAXCOLORENTRY];
   LPSTR	lpBits;
   BYTE huge	*hpBits;
   int		i;
   DWORD	dwDataSize, dwNumBytes;
   int		padded, wNumColors;


   /* hDC is a hBitmap */
// if ( !GetObject(hDC, sizeof(BITMAP), (LPSTR)&Bitmap) ) {
//    MessageBox(NULL, "Get Object Error!", "JUNIOR",
//		 MB_OK | MB_ICONEXCLAMATION);
//    outportb(0x80, 0x06);
//    return(FALSE);
// }

//   wsprintf(debugstr, "width=%4d, height=%4d", Bitmap.bmWidth, Bitmap.bmHeight);
//   MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);

//   dwDataSize = (DWORD)xWidth*yHeight;
// if ( padded=(Bitmap.bmWidth%2) )
//    padded = (2-padded);
   padded = (UINT)(xWidth%2);
   if ( padded ) padded = (2-padded);
//   if ( padded=(UINT)(xWidth%2) )
//      padded = (2-padded);
   /* insure to make the width is a multiple of 2 */
   dwDataSize = (DWORD)(xWidth+padded) * yHeight;
/*12/23
   if ( ! CompactGlobalHeap(dwDataSize) ) {
      outportb(0x80, 0x87);
      gwErrorCode = ERR_INSUFFICIENT_MEMORY;
      return(gwErrorCode);
   }
   hMem = GlobalAlloc(GHND, dwDataSize);
   if ( !hMem ) {
      outportb(0x80, 0x77);
//      wsprintf(debugstr, "Request Memory %ld", dwDataSize);
//      MessageBox(NULL, "Alloc Memory Error!", debugstr, MB_OK | MB_ICONEXCLAMATION);
      return(ERR_MEMORY_ALLOCATE_ERROR);
   }
   lpBits = GlobalLock(hMem);
12/23*/

   outportb(0x80, 0x55);
//   MessageBox(NULL, "Get Bitmap", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   dwNumBytes = GetBitmapBits(hDC, dwDataSize, glpPageBuf);
//12/23   dwNumBytes = GetBitmapBits(hDC, dwDataSize, lpBits);
//   GetBitmapBits(hBitmap, Bitmap.bmWidth*Bitmap.bmHeight, lpBits);
   if ( ! dwNumBytes ) {
      outportb(0x80, 0x66);
//      MessageBox(NULL, "Get Bitmap Bits Error!", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
//12/23      GlobalUnlock(hMem);
//12/23      GlobalFree(hMem);
      return(ERR_CREATE_BITMAP_ERROR);
   }

   outportb(0x80, 0x56);
//   if (!(GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)) {
//	outportb(0x80, 0x05);
//	 return(FALSE);
//   }

   if ( ! hPAL ) {
//     wsprintf(debugstr," palette handle %d", hPAL);
//     MessageBox(NULL,"Palette Handle",
//		 debugstr, MB_OK | MB_ICONEXCLAMATION);
      wNumColors = GetSystemPaletteEntries(hDC, 0,
		   GetDeviceCaps(hDC, SIZEPALETTE), pe);
   }
   else {

//     wsprintf(debugstr," palette handle %d", hPAL);
//     MessageBox(NULL,"Palette Handle",
//		 debugstr, MB_OK | MB_ICONEXCLAMATION);
     GetSystemPaletteEntries(hDC, 0, GetDeviceCaps(hDC, SIZEPALETTE), pe);
     wNumColors = GetPaletteEntries(hPAL, 10, MAXCOLORENTRY-20, pe);
//     wsprintf(debugstr," entries %d", wNumColors);
//     MessageBox(NULL,"Get Palette",
//		 debugstr, MB_OK | MB_ICONEXCLAMATION);

   }


//   MessageBox(NULL, "Set RAMDAC", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   JUN_init_ramdac_entry(pe);
//   ReversePalette(pe);

//   MessageBox(NULL, "Write to video memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   hpBits = (LPSTR)glpPageBuf;
//12/23   hpBits = (LPSTR)lpBits;

   for ( i=0; i<yHeight; i++ ) {
      outportb(0x80, i);
      if ( ! HandleImageRect(xLeft, yTop+i,
		    xWidth, 1, (LPSTR)hpBits, H_WRITE) )
      {
	 GlobalUnlock(hMem);
	 GlobalFree(hMem);
	 return(ERR_ACCESS_VRAM_OUT_OF_RANGE);
      }
      // 11/04/93 Tang updated to make a multiple of 2-byte
      hpBits += (DWORD) (xWidth + padded);
   }


//12/23   GlobalUnlock(hMem);
//12/23   GlobalFree(hMem);

   return(TRUE);
}
#endif


/************************************************************************
* JUN_WriteBitmap
*
*   Write the bitmap to video memory.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
int FAR PASCAL JUN_WriteBitmap(HDC hDC, HPALETTE hPAL,
				UINT xVramLeft, UINT yVramTop,
				UINT xDataWidth, UINT yDataHeight,
				UINT xDataLeft, UINT yDataTop,
				UINT xPageWidth, UINT yPageHeight)
{
   LPSTR	lpBits;
   BYTE huge	*hpBits;
   int		i;
   int          wReturnCode;

//12/27
   wReturnCode = GetImageData(hDC, xPageWidth, yPageHeight, glpPageBuf);
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   GetAndSetPalette(hDC, hPAL);

//   wsprintf(debugstr, "xVram=%d, yVram=%d, xLeft=%d, yTop=%d, xWidth=%d, yHeight=%d",
///		    xVramLeft, yVramTop, xDataLeft, yDataTop, xDataWidth, yDataHeight);
//   MessageBox(hWnd, "Write to Video memory!", debugstr, MB_OK | MB_ICONEXCLAMATION);

//   MessageBox(NULL, "Write to video memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   hpBits = (LPSTR)glpPageBuf;
//12/23   hpBits = (LPSTR)lpBits;
   hpBits += ( ((DWORD)yDataTop*((DWORD)xPageWidth+gwImgWidthPad)) + xDataLeft);
//   hpBits += ( (yDataTop*Bitmap.bmWidth) + xDataLeft);

   for ( i=0; i<yDataHeight; i++ ) {
      outportb(0x80, i);
      if ( ! HandleImageRect(xVramLeft, yVramTop+i,
		    xDataWidth, 1, (LPSTR)hpBits, H_WRITE) )
      {
//	 wsprintf(debugstr, "xLeft=%d, yTop=%d, xWidth=%d, yHeight=%d",
//		  xVramLeft, yVramTop, xDataWidth, yDataHeight);
//	 MessageBox(hWnd, "Out of range !", debugstr, MB_OK | MB_ICONEXCLAMATION);
	 return(ERR_ACCESS_VRAM_OUT_OF_RANGE);
      }
	 // 11/04/93 Tang updated to make a multiple of 2-byte
      hpBits += ((DWORD)xPageWidth + gwImgWidthPad);
   }

   return(TRUE);
}


#ifdef TEST_VER
int FAR PASCAL JUN_WriteBitmap2(HANDLE hInst, UINT xLeft, UINT yTop)
{
   static HBITMAP   hbmp_test, hBitmap, hOldBitmap;
   static BITMAP    bmp_test;
   static HANDLE    hMDC, hdcMemory;
   BITMAP	    Bitmap;
   HANDLE	    hMem;
//   PALETTEENTRY     pe[MAXCOLORENTRY];
   PAINTSTRUCT	    ps;
   LPSTR	    lpBits;
   HDC		    hDC;
   long 	    dwNumBytes;
   int		    xWidth, yHeight;
   DWORD	    dwDataSize;


   hbmp_test = LoadBitmap(hInst,"BITMAP_TEST");
   GetObject(hbmp_test,sizeof(BITMAP),(LPSTR)&bmp_test);

   xWidth = bmp_test.bmWidth;
   yHeight = bmp_test.bmHeight;

   dwDataSize = (DWORD)xWidth*yHeight;
   if ( ! CompactGlobalHeap(dwDataSize) )  return(FALSE);
   hMem = GlobalAlloc(GHND, dwDataSize);
   if ( !hMem ) {
      outportb(0x80, 0x77);
      return(FALSE);
   }
   lpBits = GlobalLock(hMem);

   dwNumBytes = GetBitmapBits(hbmp_test, dwDataSize, lpBits);

   hDC = GetDC(NULL);
   hMDC = CreateCompatibleDC(hDC);

   //paint image
   hOldBitmap=SelectObject(hMDC, hbmp_test);
   if ( !BitBlt(hDC,
		0,0,
		xWidth,
		yHeight,
		hMDC,
		0,0,
		SRCCOPY) ) {
      DeleteDC(hMDC);
      return(FALSE);
   }

//   hdcMemory = CreateCompatibleDC(hDC);
//   hBitmap = CreateCompatibleBitmap(hDC, xWidth, yHeight);
   hBitmap = CreateCompatibleBitmap(hMDC, xWidth, yHeight);

   if ( !hBitmap ) {
      outportb(0x80, 0x04);
      return(FALSE);
   }

//   MessageBox(NULL, "Alloc Memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);

   dwDataSize = (DWORD)xWidth*yHeight;
   if ( ! CompactGlobalHeap(dwDataSize) )  return(FALSE);
   hMem = GlobalAlloc(GHND, dwDataSize);
   if ( !hMem ) {
      outportb(0x80, 0x77);
      return(FALSE);
   }
   lpBits = GlobalLock(hMem);

//   if ( !GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap) ) {
//	outportb(0x80, 0x05);
//	return(FALSE);
//   }

//   if (!(GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)) {
//	 ReleaseDC(NULL, hDC);
//	 return(FALSE);
//   }

//   hOldBitmap = SelectObject(hDC, hBitmap);
   hOldBitmap = SelectObject(hMDC, hBitmap);
//   hOldBitmap = SelectObject(hdcMemory, hBitmap);
//   GetObject(hOldBitmap,sizeof(BITMAP),(LPSTR)&bitmap);

   outportb(0x80, 0x55);
//   MessageBox(NULL, "Get Bitmap", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
//   GetBitmapBits(hBitmap, dwDataSize, lpBits);
//   dwNumBytes = GetBitmapBits(hOldBitmap, dwDataSize, lpBits);
   dwNumBytes = GetBitmapBits(hOldBitmap, dwDataSize, lpBits);

   if ( ! dwNumBytes ) {
      outportb(0x80, 0x66);
      GlobalUnlock(hMem);
      GlobalFree(hMem);
      return(FALSE);
   }


//   GetSystemPaletteEntries(hDC, 0, GetDeviceCaps(hDC, SIZEPALETTE), pe);

//   HandleImageRect(xLeft, yTop, bmp_test.bmWidth, bmp_test.bmHeight,
//		     lpBits, H_WRITE);

//   JUN_init_ramdac_entry(pe);

   GlobalUnlock(hMem);
   GlobalFree(hMem);

   SelectObject(hMDC,hOldBitmap);
   DeleteDC(hMDC);
   DeleteDC(hdcMemory);
   ReleaseDC(NULL, hDC);

   return(TRUE);
}


/************************************************************************
* JUN_LoadObject
*
*   Load object into memory buffer.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
int FAR PASCAL JUN_LoadObject(HDC hDC,
			      UINT xLeft,  UINT yTop,
			      UINT xWidth, UINT yHeight)
{
//   HBITMAP	  hBitmap, hOldBitmap;
//   HANDLE	    hdcMem;
   DWORD	  dwNumBytes, wNumColors;
   BITMAP	  Bitmap;
   BYTE huge	  *hpBits;
   int		  i;

   wsprintf(debugstr, "hDC=%d, xLeft=%d, yTop=%d, xWidth=%d, yHeight=%d",
	    hDC, xLeft, yTop, xWidth, yHeight);
   MessageBox(NULL, "Create Bitmap", debugstr, MB_OK | MB_ICONEXCLAMATION);
//   hBitmap = CreateCompatibleBitmap(hDC, xWidth, yHeight);
//   hOldBitmap = SelectObject(hDC, hBitmap);
//   if ( !hOldBitmap ) {
//	MessageBox(NULL, "Select Bitmap Error!", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
//	outportb(0x80, 0x05);
//	DeleteObject(hBitmap);
//	return(FALSE);
//   }
   if ( ! hDC )   return(FALSE);

   if ( ! gwLoadCount )
      ghdcMemory = CreateCompatibleDC(hDC);

   if ( ! BitBlt(ghdcMemory, xLeft, yTop, xWidth, yHeight,  //Destination
		 hDC, 0, 0, SRCCOPY) )			   //Source
   {
      wsprintf(debugstr, "BitBlt Error!");
      MessageBox(NULL, debugstr, "JUNIOR.DLL", MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x99);
      DeleteDC(ghdcMemory);
      return(FALSE);
   }

   gwLoadCount ++;
//   if ( ! GetObject(hdcMem, sizeof(BITMAP), (LPSTR)&Bitmap) ) {
//	MessageBox(NULL, "Get Object Error!", "JUNIOR",
//		 MB_OK | MB_ICONEXCLAMATION);
//	outportb(0x80, 0x06);
//	return(FALSE);
//   }

   /* 12/01/93 Tang added */
//   wNumColors = GetSystemPaletteEntries(hDC, 0,
//		     GetDeviceCaps(hDC, SIZEPALETTE), pe);
//   if ( ! wNumColors ) {
//	wsprintf(debugstr,"hDC=%d, wNumColor=%d", hDC, wNumColors);
//	MessageBox(NULL, debugstr, "Get Palette error!",
//		   MB_OK | MB_ICONEXCLAMATION);
//   }
//   else
//   MessageBox(NULL, "Set RAMDAC", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
//	JUN_init_ramdac_entry(pe);
//   ReversePalette(pe);


//   MessageBox(NULL, "Write to video memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
//   hpBits = (LPSTR)Bitmap.bmBits;

//   for ( i=0; i<Bitmap.bmHeight; i++ ) {
//	outportb(0x80, i);
//	if ( ! HandleImageRect(30, 40+i,
//		      xWidth, 1, (LPSTR)hpBits, H_WRITE) )
//	{
//	   wsprintf(debugstr, "xLeft=%d, yTop=%d, xWidth=%d, yHeight=%d",
//		    xLeft, yTop, xWidth, yHeight);
//	   MessageBox(hWnd, "Out of range !", debugstr, MB_OK | MB_ICONEXCLAMATION);
//	   return(FALSE);
//	}
	 // 11/04/93 Tang updated to make a multiple of 2-byte
//	hpBits += ((DWORD)Bitmap.bmWidth);
//   }

//   DeleteDC(hdcMem);

   return(TRUE);
}

/************************************************************************
* JUN_WriteBitmap7
*
*   Write the bitmap to video memory.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
int FAR PASCAL JUN_WriteBitmap7(UINT xVramLeft, UINT yVramTop,
				UINT xDataWidth, UINT yDataHeight,
				UINT xDataLeft, UINT yDataTop)
{
   HBITMAP	hBitmap, hOldBitmap;
   BITMAP	Bitmap;
   HDC		hdc;
   UINT 	i, padded;
   WORD 	wNumColors;
   DWORD	dwDataSize, dwNumBytes;
   BYTE huge	*hpBits;

   if ( ghdcMemory ) {
      hBitmap = CreateCompatibleBitmap(ghdcMemory, xDataWidth, yDataHeight);
      if ( !hBitmap ) {
	 wsprintf(debugstr, "hBitmap=%4d", hBitmap);
	 MessageBox(NULL, "Create Bitmap Error!", debugstr, MB_OK | MB_ICONEXCLAMATION);
	 outportb(0x80, 0x04);
	 return(FALSE);
      }

//   MessageBox(NULL, "Select Bitmap ", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      hOldBitmap = SelectObject(ghdcMemory, hBitmap);
      if ( !hOldBitmap ) {
	 MessageBox(NULL, "Select Bitmap Error!", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
	 outportb(0x80, 0x05);
	 DeleteObject(hBitmap);
	 return(FALSE);
      }

//	MessageBox(NULL, "Get Object", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      if ( !GetObject(hOldBitmap, sizeof(BITMAP), (LPSTR)&Bitmap) ) {
	 MessageBox(NULL, "Get Object Error!", "JUNIOR",
		    MB_OK | MB_ICONEXCLAMATION);
	 outportb(0x80, 0x06);
	 SelectObject(ghdcMemory, hOldBitmap);
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

   padded = (UINT)(Bitmap.bmWidth%2);
   if ( padded ) padded = (2-padded);

   /* insure to make the width is a multiple of 2 */
   dwDataSize = (DWORD)(Bitmap.bmWidth+padded) * Bitmap.bmHeight;

   dwNumBytes = GetBitmapBits(hOldBitmap, dwDataSize, glpPageBuf);

//   wsprintf(debugstr, "width=%4d, height=%4d", Bitmap.bmWidth, Bitmap.bmHeight);
//   MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);

   hdc = GetDC(NULL);

   wNumColors = GetSystemPaletteEntries(hdc, 0,
		   GetDeviceCaps(hdc, SIZEPALETTE), pe);
   if ( ! wNumColors ) {
      wsprintf(debugstr,"hdc=%d, wNumColor=%d", hdc, wNumColors);
      MessageBox(NULL, debugstr, "Get Palette error!",
		 MB_OK | MB_ICONEXCLAMATION);
   }
   else
//   MessageBox(NULL, "Set RAMDAC", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      JUN_init_ramdac_entry(pe);
//   ReversePalette(pe);


//   MessageBox(NULL, "Write to video memory", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   hpBits = (LPSTR)glpPageBuf;
   hpBits +=  ( ((DWORD)yDataTop*VGA_PAGE_WIDTH) + xDataLeft);

   for ( i=0; i<yDataHeight; i++ ) {
      outportb(0x80, i);
      if ( ! HandleImageRect(xVramLeft, yVramTop+i,
		    xDataWidth, 1, (LPSTR)hpBits, H_WRITE) )
      {
	 wsprintf(debugstr, "xLeft=%d, yTop=%d, xWidth=%d, yHeight=%d",
		  xVramLeft, yVramTop, xDataWidth, yDataHeight);
	 MessageBox(hWnd, "Out of range !", debugstr, MB_OK | MB_ICONEXCLAMATION);
	 return(FALSE);
      }
	 // 11/04/93 Tang updated to make a multiple of 2-byte
      hpBits += ((DWORD)VGA_PAGE_WIDTH);
   }

   ReleaseDC(NULL, hdc);

   gwLoadCount = 0;
   DeleteDC(ghdcMemory);

   return(TRUE);
}
#endif


//int FAR PASCAL JUN_WriteBitmap3(HANDLE hInst, UINT xLeft, UINT yTop)
//{ return(TRUE);}


//12/27/93 Tang added
/************************************************************************
* GetImageData
*
*   Get the bitmap object from a given hDC(parameter of VB call), and
* return far pointer to image buffer(created at initial time).
* Steps: 1. create compatible bitmap from hDC(get hBitmap),
*        2. select hBitmap into hDC(get hOldBitmap),
*        3. get bitmap bits(get long pointer to image data),
*        4. delete object.
*
* Exported:  No
* Entry:     hDC, bitmap size, returned pointer to image buffer
* Destroys:
* Exit:      TRUE for success, ErrorCode for failure
************************************************************************/
int FAR GetImageData(HDC hDC, UINT xPageWidth, UINT yPageHeight,
                     LPSTR lpImgBuf)
{
   HBITMAP	hBitmap, hOldBitmap;
   BITMAP	Bitmap;
   DWORD	dwDataSize, dwNumBytes;

//   wsprintf(debugstr, "hDC=%4d, PageWidth=%4d, PageHeight=%4d",
//			hDC, xPageWidth, yPageHeight);
//   MessageBox(NULL, debugstr, "Before WriteBitmap", MB_OK | MB_ICONEXCLAMATION);
   hBitmap = CreateCompatibleBitmap(hDC, xPageWidth, yPageHeight);
   if ( !hBitmap ) {
      wsprintf(debugstr, "hBitmap=%4d", hBitmap);
      MessageBox(NULL, "Create Bitmap Error!", debugstr, MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x04);
      return(ERR_CREATE_BITMAP_ERROR);
   }

//   MessageBox(NULL, "Select Bitmap ", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   hOldBitmap = SelectObject(hDC, hBitmap);
   if ( !hOldBitmap ) {
//      MessageBox(NULL, "Select Bitmap Error!", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x05);
      DeleteObject(hBitmap);
      return(ERR_SELECT_OBJECT_ERROR);
   }

//   MessageBox(NULL, "Get Object", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   if ( !GetObject(hOldBitmap, sizeof(BITMAP), (LPSTR)&Bitmap) ) {
//      MessageBox(NULL, "Get Object Error!", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x06);
      SelectObject(hDC, hOldBitmap);
      DeleteObject(hBitmap);
      return(ERR_GET_OBJECT_ERROR);
   }

//   wsprintf(debugstr, "width=%4d, height=%4d", Bitmap.bmWidth, Bitmap.bmHeight);
//   MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);

   // insure to make the width is a multiple of 2 //
   gwImgWidthPad = Bitmap.bmWidth % 2;
//   dwDataSize = (DWORD)xWidth*yHeight;
   dwDataSize = (DWORD)(Bitmap.bmWidth+gwImgWidthPad) * Bitmap.bmHeight;
/*12/23
   if ( ! CompactGlobalHeap(dwDataSize) ) {
      SelectObject(hDC, hOldBitmap);
      DeleteObject(hBitmap);
      outportb(0x80, 0x87);
      gwErrorCode = ERR_INSUFFICIENT_MEMORY;
      return(gwErrorCode);
   }
   hMem = GlobalAlloc(GHND, dwDataSize);
   if ( !hMem ) {
      outportb(0x80, 0x77);
      wsprintf(debugstr, "Request=%ld, Actual=%ld", dwDataSize, dwActAllocSize);
      MessageBox(NULL, debugstr, "GlobalAlloc Memory Error!", MB_OK | MB_ICONEXCLAMATION);
      SelectObject(hDC, hOldBitmap);
      DeleteObject(hBitmap);
      return(ERR_MEMORY_ALLOCATE_ERROR);
   }
   lpBits = GlobalLock(hMem);
12/23*/

   outportb(0x80, 0x55);
   // 3/9/94 Tang added for solving page-fault error. //
   GlobalLock(hOldBitmap);
//   MessageBox(NULL, "Get Bitmap", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
   dwNumBytes = GetBitmapBits(hOldBitmap, dwDataSize, lpImgBuf);
//12/23   dwNumBytes = GetBitmapBits(hOldBitmap, dwDataSize, lpBits);
   if ( ! dwNumBytes ) {
      outportb(0x80, 0x66);
//      MessageBox(NULL, "Get Bitmap Bits Error!", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      GlobalUnlock(hOldBitmap);
      SelectObject(hDC, hOldBitmap);
      DeleteObject(hBitmap);
      return(ERR_CREATE_BITMAP_ERROR);
   }
   // 3/9/94 Tang added for solving page-fault error. //
   GlobalUnlock(hOldBitmap);
   outportb(0x80, 0x56);

   SelectObject(hDC, hOldBitmap);
   DeleteObject(hBitmap);
   return(TRUE);
}


/************************************************************************
* GetAndSetPalette
*
*   Get the palette object from a given hPAL(parameter passing from upper_
* layer, VB call), and set to JUNIOR's RAMDAC. If hPAL is NULL, then
* set RAMDAC the Windows system's palette. 
*
* Exported:  No
* Entry:     hDC, bitmap size, returned pointer to image buffer
* Destroys:
* Exit:      TRUE 
************************************************************************/
int FAR GetAndSetPalette(HDC hDC, HPALETTE hPAL)
{
   int  wNumColors;

   outportb(0x80, 0x56);
//   if (!(GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)) {
//	outportb(0x80, 0x05);
//	 return(FALSE);
//   }

   // if hPAL== 0, then get the system palette and update RAMDAC,
   // to reflect the different palette of varient pages. //
   if ( ! hPAL ) {
//     wsprintf(debugstr," palette handle %d", hPAL);
//     MessageBox(NULL,"Palette Handle",
//		 debugstr, MB_OK | MB_ICONEXCLAMATION);
      wNumColors = GetSystemPaletteEntries(hDC, 0,
		   GetDeviceCaps(hDC, SIZEPALETTE), pe);
      // 3/10/94 Tang updated //
//   MessageBox(NULL, "Set RAMDAC", "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
     JUN_init_ramdac_entry(pe);
//   ReversePalette(pe);
   }
   else {

//     wsprintf(debugstr," palette handle %d", hPAL);
//     MessageBox(NULL,"Palette Handle",
//		 debugstr, MB_OK | MB_ICONEXCLAMATION);
//3/10/94     GetSystemPaletteEntries(hDC, 0, GetDeviceCaps(hDC, SIZEPALETTE), pe);
//3/10/94     wNumColors = GetPaletteEntries(hPAL, 10, MAXCOLORENTRY-20, pe);
//     wsprintf(debugstr," entries %d", wNumColors);
//     MessageBox(NULL,"Get Palette",
//		 debugstr, MB_OK | MB_ICONEXCLAMATION);

   }

   return(TRUE);
}

// end of junwrbmp.c //
