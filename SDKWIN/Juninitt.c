
/************************************************************************
* Project : junior.dll
* Module  : juninit.c
* Object  : DLL and card initialization and selector, address allocation
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
#include <dos.h>	//enable(),disable(),getvect(),setvect()
#include <time.h>	//time()
#include "juniora.h"
#include "externa.h"

/*
//extern WORD _MyD000h;
//#define MySelector (&_MyD000h)
//extern WORD _D000h;
//#define MySelector (&_D000h)

int FAR AllocSelect(void)
{
   glpVideoMemory = (LPSTR)MAKELONG(0, MySelector);
   return(TRUE);
}
*/

// 11/29/93 Tang added
extern WORD _near _D000H;   //global variables exported by the KERNEL module
WORD __D000H = (WORD) (&_D000H);
//unsigned char far *fpD000 = MK_FP(__D000H, 0);


/************************************************************************
* LibMain(hInstance,wDataSeg,wHeapSize,lpszCmdLine)
*
*   This routine is called by Windows when the DLL is first called.
* It allocates the heap, opens a log file and loads the real DLL.
*
* Entry:	DS = DLLs data segment
*		DI = Handle to module instance
*		CX = Heap size (whatever is in our .DEF file)
*		ES = Segment of command line string
*		SI = Offset of command line string
*
* Exported:	Yes
* Return:	AX = 1 (TRUE)
*************************************************************************/
int FAR PASCAL LibMain(HANDLE hInstance,
		       WORD   wDataSeg,
		       WORD   wHeapSize,
		       LPSTR  lpszCmdLine)
{
   return(TRUE);
}

/***********************************************************************
* WEP(msg)
*
*  Called by Windows when the DLL is discarded.
*
* Exported:	Yes
* Entry:	nMesg(bSystemExit)
* Return:	AX = 1 (TRUE)
***********************************************************************/
int FAR PASCAL WEP(int bSystemExit)
{
   return(TRUE);
}


/************************************************************************
* AllocSel() & FreeSel()
*
*   Allocate 1 selector and set the selector's base address and limit
* by Windows 3.1(only) API.
*
* Steps: 1. AllocSelector(DataSeg)
*	 2. SetSelectorBase(selector,baseaddr)
*	 3. SetSelectorLimit(selector,limit)
* Usage: far pointer return by MK_FP(selector,0)
*
* Exported:	No
* Entry:	None
* Return:	AX = 1 (TRUE)
************************************************************************/
/*
int NEAR AllocSel(void)
{
   unsigned long  base, limit;
   unsigned short  sel;
   _asm mov sel,ds   //must be!, sel can't be 0
   // Allocate a new selector
   if ( (gwFrameBufSel=AllocSelector(sel)) == 0 )
      return(FALSE);
   base = ( ((DWORD)gConfig.wFrameAddr) << 4 ) + 0;
   limit = 0xFFFF;
   SetSelectorBase(gwFrameBufSel, base);
   SetSelectorLimit(gwFrameBufSel, limit);
//   glpVideoMemory = (WORD FAR *)((DWORD)gwFrameBufSel << 16);
   return(TRUE);
}
*/
//unsigned char far *fPointer;
int NEAR AllocSel(WORD wOffset, WORD wSize)
{
   DWORD   base, limit;
   WORD    wDataSelector;

   wDataSelector = HIWORD ((DWORD) (WORD FAR *) &wDataSelector);
   /* Allocate a new selector */
   if ( (gwFrameBufSel=AllocSelector(wDataSelector)) == 0 )
      return(FALSE);
   // 3/16/94 Tang added
   GlobalLock(gwFrameBufSel);
   // 3/16/94 Tang added
   base = ( ((DWORD)gConfig.wFrameAddr) << 4 ) + wOffset;
   limit = wSize;
   SetSelectorBase(gwFrameBufSel, base);
   SetSelectorLimit(gwFrameBufSel, limit);
   /* get the video frame buffer address */
   glpVideoMemory = (BYTE FAR *)((DWORD)gwFrameBufSel << 16);
//   glpVideoMemory = (BYTE FAR *)((DWORD)__D000H << 16);
//   glpVideoMemory = MK_FP(__D000H, 0);
   return(TRUE);
}


int NEAR FreeSel(void)
{
   // 3/16/94 Tang added
   GlobalUnlock(gwFrameBufSel);
   // 3/16/94 Tang added
   if ( FreeSelector(gwFrameBufSel) == 0 )
      return(TRUE);
   else return(FALSE);
}


/**************************************************************************
* GetVideoAddress
*
*   From the mapping base address in the selector return by DPMI server,
* we can get the start video address at coordinate (X,Y).
*
* Exported:	No
* Entry:	(xStart,yStart)-Coordinate of video memory(screen)
* Return:	far pointer(linear address) to that location
***************************************************************************/
BYTE FAR * GetVideoAddress(UINT xStart, UINT yStart)
{
//   unsigned int  wOffset;
//   unsigned long lFarAddr;
//   lFarAddr = (DWORD)gwFrameBufSel;
//   wOffset = (yStart<<10) + xStart;
//   lFarAddr = (DWORD)(lFarAddr << 16) + wOffset;
//11/30   lFarAddr = (DWORD)glpVideoMemory + wOffset;
//   return( (BYTE far *)lFarAddr );
     //12/01/93 Tang updated
     return( (BYTE far *)(glpVideoMemory + (DWORD)((yStart<<10) + xStart)) );
//   return MK_FP(gwFrameBufSel,wOffset);

}

char  szCfgFileSpec[256];
char  szFontFileSpec[256];
char  szBuffer[120];
char  szData[20], *endptr;
/************************************************************************
* JUN_LoadConfiguration
*
* Reads in the OVERLAY JUNIOR configuration file junior.ini.
*
* Exit: return code
*	= 1	Success
*	=-1	File does not exist.
************************************************************************/
int FAR PASCAL JUN_LoadConfiguration(void)
{
   int	fd, i;

#ifdef DOSLIB
   getcwd((LPSTR)szCfgFileSpec, 144);	/* Get current working directory */
   i = strlen(szCfgFileSpec) - 1;
   if ( szCfgFileSpec[i] == '\\' )
      szCfgFileSpec[i] = 0;
   strcat((LPSTR)szCfgFileSpec, CFG_FILE_NAMED);	 /* [ADDA] */
#else  //Windows
   GetWindowsDirectory((LPSTR)szCfgFileSpec, 144);
   wsprintf((LPSTR)szCfgFileSpec, CFG_FILE_NAMEW, (LPSTR)szCfgFileSpec);
#endif

   if ((fd = fileopen((LPSTR)szCfgFileSpec, F_READ)) == -1) {
      outportb(0x80, 0x01);
#ifdef TEST_VER
      wsprintf(debugstr, "JUNIOR.INI file open error!");
      MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
#endif
      return(FALSE);	/* File does not exist. */
   }
   if ( fileclose(fd) != 0 ) {
#ifdef TEST_VER
      wsprintf(debugstr, "JUNIOR.INI file close error!");
      MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
#endif
      return(FALSE);
   }

#ifdef DOSLIB
   JUN_GetProfileString("JUNIOR", "FrameAddr", szData, 10);
   gConfig.wFrameAddr = (WORD)strtoul(szData, &endptr, 0);
//   gConfig.wFrameAddr = (WORD)atol(szData);

   JUN_GetProfileString("JUNIOR", "PortBaseAddr", szData, 10);
   gConfig.wPortBaseAddr = (WORD)strtoul(szData, &endptr, 0);
//   gConfig.wPortBaseAddr = (WORD)atol(szData);

   JUN_GetProfileString("JUNIOR", "CardID", szData, 10);
   gConfig.bCardID = (BYTE)strtoul(szData, &endptr, 0);
//   gConfig.bCardID = (BYTE)atol(szData);

   JUN_GetProfileString("JUNIOR", "IRQUsed", szData, 10);
   gConfig.bIRQUsed = (BYTE)strtoul(szData, &endptr, 0);
//   gConfig.bIRQUsed = (BYTE)atol(szData);

   JUN_GetProfileString("JUNIOR", "ColorKey", szData, 10);
   gConfig.bColorKey = (BYTE)strtoul(szData, &endptr, 0);
//   gConfig.bColorKey = (BYTE)atol(szData);

   JUN_GetProfileString("JUNIOR", "DisplayMode", szData, 10);
   if ( stricmp(szData, "EXTERNAL_VIDEO_ONLY") == 0 )
      gConfig.bDisplayMode = EXTERNAL_VIDEO_ONLY; // EXTERNAL_VIDEO_ONLY mode //
   else
      gConfig.bDisplayMode = OVERLAY;  // OVERLAY mode //

   JUN_GetProfileString("JUNIOR", "ScanMode", szData, 10);
   if ( stricmp(szData, "OVERSCAN") == 0 )
      gConfig.bScanMode = OVERSCAN; // OVERSCAN mode //
   else
      gConfig.bScanMode = UNDERSCAN;  // UNDERSCAN mode //

   JUN_GetProfileString("JUNIOR", "VertScanStart", szData, 10);
   gConfig.wVertScanStart = (WORD)strtoul(szData, &endptr, 0);
//   gConfig.wVertScanStart = (WORD)atol(szData);

   JUN_GetProfileString("JUNIOR", "HorzScanStart", szData, 10);
   gConfig.wHorzScanStart = (WORD)strtoul(szData, &endptr, 0);
//   gConfig.wHorzScanStart = (WORD)atol(szData);
#endif

   GetPrivateProfileString("JUNIOR", "FrameAddr", "0xD000", szData, 10, szCfgFileSpec);
   gConfig.wFrameAddr = (WORD)strtoul(szData, &endptr, 0);
//   gConfig.wFrameAddr = (WORD)atol(szData);

   GetPrivateProfileString("JUNIOR", "PortBaseAddr", "0x280", szData, 10, szCfgFileSpec);
   gConfig.wPortBaseAddr = (WORD)strtoul(szData, &endptr, 0);
//   gConfig.wPortBaseAddr = (WORD)atol(szData);

   GetPrivateProfileString("JUNIOR", "CardID", "0x0E", szData, 10, szCfgFileSpec);
   gConfig.bCardID = (BYTE)strtoul(szData, &endptr, 0);
//   gConfig.bCardID = (BYTE)atol(szData);

   GetPrivateProfileString("JUNIOR", "IRQUsed", "0x05", szData, 10, szCfgFileSpec);
   gConfig.bIRQUsed = (BYTE)strtoul(szData, &endptr, 0);
//   gConfig.bIRQUsed = (BYTE)atol(szData);

   GetPrivateProfileString("JUNIOR", "ColorKey", "0xFC", szData, 10, szCfgFileSpec);
   gConfig.bColorKey = (BYTE)strtoul(szData, &endptr, 0);
//   gConfig.bColorKey = (BYTE)atol(szData);

   GetPrivateProfileString("JUNIOR", "DisplayMode", "OVERLAY", szData, 10, szCfgFileSpec);
   if ( stricmp(szData, "EXTERNAL_VIDEO_ONLY") == 0 )
      gConfig.bDisplayMode = EXTERNAL_VIDEO_ONLY; // EXTERNAL_VIDEO_ONLY mode //
   else
      gConfig.bDisplayMode = OVERLAY;  // OVERLAY mode //

   GetPrivateProfileString("JUNIOR", "ScanMode", "OVERSCAN", szData, 10, szCfgFileSpec);
   if ( stricmp(szData, "OVERSCAN") == 0 )
      gConfig.bScanMode = OVERSCAN; // OVERSCAN mode //
   else
      gConfig.bScanMode = UNDERSCAN;  // UNDERSCAN mode //

   GetPrivateProfileString("JUNIOR", "VertScanStart", "0x00", szData, 10, szCfgFileSpec);
   gConfig.wVertScanStart = (WORD)strtoul(szData, &endptr, 0);
//   gConfig.wVertScanStart = (WORD)atol(szData);

   GetPrivateProfileString("JUNIOR", "HorzScanStart", "0x00", szData, 10, szCfgFileSpec);
   gConfig.wHorzScanStart = (WORD)strtoul(szData, &endptr, 0);
//   gConfig.wHorzScanStart = (WORD)atol(szData);

   return(TRUE);
}


/************************************************************************
* JUN_SaveConfiguration
*
* Saves the system configuration to junior.ini.
*
* Exit: return code
*	= 1	Success
*	= 0	File does not exist.
*
* Note:   The configuration structure is given in JUNIOR.H .
************************************************************************/
int FAR PASCAL JUN_SaveConfiguration(void)
{
   int	fd, i;

   remove(szCfgFileSpec);
   if ((fd = filecreat((LPSTR)&szCfgFileSpec, F_MODE)) == -1)
       return(FALSE);	/* Could not create file */

   /* Save JUNIOR info */
   stringf(szBuffer,"[JUNIOR]\r\n");
   if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
      return(FALSE);

   stringf(szBuffer,"FrameAddr=0x%X\r\n", gConfig.wFrameAddr);
   if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
      return(FALSE);

   stringf(szBuffer,"PortBaseAddr=0x%X\r\n", gConfig.wPortBaseAddr);
   if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
      return(FALSE);

   stringf(szBuffer,"CardID=0x%X\r\n", gConfig.bCardID);
   if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
      return(FALSE);

   stringf(szBuffer,"IRQUsed=0x%X\r\n", gConfig.bIRQUsed);
   if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
      return(FALSE);

   stringf(szBuffer,"ColorKey=0x%X\r\n", gConfig.bColorKey);
   if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
      return(FALSE);

    if ( gConfig.bDisplayMode == EXTERNAL_VIDEO_ONLY ) /* EXTERNAL_VIDEO_ONLY mode */
       stringf(szBuffer, "DisplayMode=EXTERNAL_VIDEO_ONLY\r\n");
    else			    /* PAL format */
       stringf(szBuffer, "DisplayMode=OVERLAY\r\n");
    if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
       return(FALSE);

    if ( gConfig.bScanMode == OVERSCAN ) /* OVERSCAN mode */
       stringf(szBuffer, "ScanMode=OVERSCAN\r\n");
    else			    /* PAL format */
       stringf(szBuffer, "ScanMode=UNDERSCAN\r\n");
    if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
       return(FALSE);

    stringf(szBuffer,"VertScanStart=0x%X\r\n", gConfig.wVertScanStart);
    if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
       return(FALSE);

    stringf(szBuffer,"HorzScanStart=0x%X\r\n", gConfig.wHorzScanStart);
    if (filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer)) != strlen(szBuffer))
       return(FALSE);

    fileclose(fd);		/* Close ini file */
    return(TRUE);
}


/************************************************************************
* JUN_GetProfileInt
*
* Returns the value of an interger key from the junior.ini file.
*
* Exit: return code
*	= Key value	Success
*	=-1		Key does not exist.
*
************************************************************************/
WORD FAR PASCAL JUN_GetProfileInt(LPSTR lpSectionName, LPSTR lpKeyName)
{

   if ( JUN_GetProfileString(lpSectionName, lpKeyName, szData, 10) != 0 )
      return( (WORD)strtoul(szData, &endptr, 0) );    /* Return the value */
   else
      return(-1);

}

char   szSectionName[20];	/* Input string buffer */
LPSTR  szPtr;
/************************************************************************
* JUN_GetProfileString
*
* Write the character string to the junior.ini file.
*
************************************************************************/
int FAR PASCAL JUN_GetProfileString(LPSTR lpSectionName, LPSTR lpKeyName,
				    LPSTR lpReturnedString, int nSize)
{
   FILE  *fd;
   int	 stringlength, i;

//   ShowFreeSpace("before open");
   if ( (fd = fopen(szCfgFileSpec, "r")) == NULL ) {
      return(FALSE);		/* File does not exist */
   }

//   ShowFreeSpace("after open");
   stringf(szSectionName,"[%s]", (LPSTR)lpSectionName);
   stringlength = strlen(szSectionName)+1;

   while ( fgets(szBuffer, stringlength, fd) != NULL ) {
      if ( stringcmp(szSectionName, szBuffer) == 0 ) { /* Section matched */
	 szPtr = lpSectionName; 	      /* Initialize ptr */
	 while ( stringcmp((LPSTR)szPtr, lpKeyName) != 0 ) {	/* Key matched */
	    if ( (fgets(szBuffer, 80, fd) == NULL) ||
		 (strrchr(szBuffer, '[') != NULL) ) {
				/* End of the Section or end of file */
	       fclose(fd);
	       return(FALSE);
	    }
	    szPtr = strtok(szBuffer,"=");           /* Get the key */
	 }
	 i = fclose(fd);
	 szPtr=strtok(NULL,"\n");                   /* Get the string */
	 stringcpy(lpReturnedString, szPtr);
	 return(stringlen(lpReturnedString));
      }
   } /* end of while */

   i = fclose(fd);
   return(0);
}


/************************************************************************
* JUN_Diagnosis
*
*   This routine is responsible for diagnosing that the special setting
* value is correct. We attempt to detect the hardware and test setting.
* This routine should be written in such a way that it can be called at
* any time to diagnosis the hardware is prepared to "work".
*
* Exported:  Yes
* Entry:     None
* Return:    TRUE,	if success,
*	     ErrorCode, else failure.
************************************************************************/
int FAR PASCAL JUN_Diagnosis(BOOL flag)
{
   int	wReturnCode, i;

   wReturnCode = TRUE;

   /* Initialize I/O registers & their valuse */
   if ( ! gbInitial || flag ) {
      //1/27/93 Tang
      InitialGlobals();
      /* 1. Load JUNIOR.INI file to get configuration */
      if ( ! JUN_LoadConfiguration() ) {
//	 wsprintf(debugstr, "JUNIOR.INI file not found!");
//	 MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
	 outportb(0x80, 0x81);
	 return(ERR_JUNIOR_INI_NOT_FOUND);
      }
      /* 2. Initial io registers' value */
      init_io_reg_val();
      /* 3. set JUNIOR configuration */
      SetConfiguration();

      /* check the JUNIOR card is exist */
/* 1/27/94 Tang
      if ( ! JUN_junior_exist() ) {
//	 wsprintf(debugstr, "JUNIOR card not exist!");
//	 MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
	 outportb(0x80, 0x82);
	 return(ERR_JUNIOR_CARD_NOT_EXIST);
      }
*/
   }

   /* 1. Verify I/O Port & Card ID */
   wReturnCode = VerifyPortAndID(gConfig.wPortBaseAddr, gConfig.bCardID);
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   /* 2. Verify Frame Address */
   wReturnCode = VerifyFrameAddr(gConfig.wFrameAddr);
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   /* 3. Vefify IRQ */
   wReturnCode = VerifyIRQ(gConfig.bIRQUsed);
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   /* 4. Verify video memory */
   wReturnCode = alloc_and_check_bank_access();
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   /* 5. Verify Ramdac */
   for ( i=0; i<64; i++ )
      if ( ! VerifyRamdac() )
	 return(ERR_RAMDAC_ACCESS_ERROR);

   /* 12/13/93 Tang added */
   JUN_flash_video_memory(gConfig.bColorKey);

//1/27/94 Tang masked	gbDiagnose++;
//1/27/94 Tang added
   /* Free selector */
   if ( ! gbInitial )	// if no initialized, free selector !
      if ( ! FreeSel() ) {
#ifdef TEST_VER
	 wsprintf(debugstr, "Free Selector Error!");
	 MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
#endif
      }

   return(TRUE);
}



/************************************************************************
* VerifyConfiguration
*
*   This routine is responsible for verifying that the configuration
* information is correct. We attempt to detect the hardware and gaurantee
* that the configuration information is valid. Note! This is "verify"
* setting, not "test" hardware.
*
* Exported:  Yes
* Entry:     None
* Return:    TRUE,	if success,
*	     ErrorCode, else failure.
************************************************************************/
int NEAR VerifyConfiguration(void)
{
   int	wReturnCode;

   wReturnCode = TRUE;

   /* 1. Verify I/O Port & Card ID */
   wReturnCode = VerifyPortAndID(gConfig.wPortBaseAddr, gConfig.bCardID);
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   /* 2. Verify Frame Address */
   wReturnCode = VerifyFrameAddr(gConfig.wFrameAddr);
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   /* 3. Vefify IRQ */
   wReturnCode = VerifyIRQ(gConfig.bIRQUsed);
   if ( wReturnCode != TRUE )
      return(wReturnCode);

   if ( (gConfig.bDisplayMode != EXTERNAL_VIDEO_ONLY) &&
	(gConfig.bDisplayMode != OVERLAY) )
      gConfig.bDisplayMode = OVERLAY;

   if ( (gConfig.bScanMode != UNDERSCAN) &&
	(gConfig.bScanMode != OVERSCAN) )
      gConfig.bScanMode = OVERSCAN;

   if ( gConfig.wVertScanStart > 0xff )
      gConfig.wVertScanStart &= 0xff;

   if ( gConfig.wHorzScanStart > 0x1ff )
      gConfig.wHorzScanStart &= 0x1ff;

   return(TRUE);
}


int NEAR VerifyPortAndID(WORD wPort, BYTE bCardID)
{
   int	 i, wCurrStatus;

   /* I/O port must be 2x0, or we know it is bad. */
   if ( ! ( (wPort >= 0x200 && wPort <= 0x2ff) && !(wPort & 0x000f) ) )
      return(ERR_IO_PORT_SETTING_BAD);

   /* Card ID must be from 0 to 14, or we know it is bad. */
   if ( ! ( bCardID <= 14 ) )
      return(ERR_CARD_ID_SETTING_BAD);

   /* Check card ID */
   JUN_set_card_id(bCardID);
   wCurrStatus = JUN_get_junior_status();
   if ( (BYTE)(wCurrStatus) == 0xff )
      return(ERR_JUNIOR_CARD_NOT_EXIST);

   /* Check I/O port */
   randomize();  //Initialize the random number generator
   for ( i=0; i<3; i++ )
      if ( ! VerifyRamdac() )
	 return(ERR_JUNIOR_CARD_NOT_EXIST);

   return(TRUE);
}


int NEAR VerifyIRQ(BYTE bIRQ)
{
   time_t  first, second;
   UINT    count, int_count;
   BYTE    status_reg;

   /* Interrupt must be from 3 to 7, or we know it is bad. */
   if ( ! ( bIRQ >= 3 && bIRQ <= 7 ) )
      return(ERR_IRQ_SETTING_BAD);

   /* Set interrupt vector & mask interrupt no */
   init_interrupt_test(bIRQ);

   /* Step 1. Test Vsync period by polling status reg. bit 4 */
   count = 0;
   first = time(NULL);	/* Gets system time */
   do {
      /* polling Vsync to ask Vsync is generated */
      wait_vert_retrace_start();
      count++;
      second = time(NULL); /* Gets system time again */
   /* calculates the elapsed time in seconds, return as a double */
   } while ( difftime(second, first) < 1 ); //wait for 1 second

   /* Step 2. Test Vsync interrupt with ISR(junaux.c) */
   gulEnterIntCount = 0;
   // enable interrupt
   JUN_enable_vert_sync_int(TRUE);

   first = time(NULL);	/* Gets system time */
   do {
      second = time(NULL); /* Gets system time again */
      /* calculates the elapsed time in seconds, return as a double */
   } while ( difftime(second, first) < 1 ); //wait for 1 second

   // disable interrupt
   disable();
   JUN_enable_vert_sync_int(FALSE);
   enable();

   /* Restore interrupt vector & unmask interrupt no */
   restore_interrupt(bIRQ);

//   wsprintf(debugstr, "IntCount=%d, VsyncCount=%d", gulEnterIntCount, count);
//   MessageBox(NULL, debugstr, "Verify IRQ", MB_OK | MB_ICONEXCLAMATION);

   /* Fv = 60 Hz, so 60 times at 1 second period, error allow to 5 */
   if ( JUN_get_TVsystem_type() == NTSC )
      int_count = 60;
   else
      int_count = 50;

   if ( (gulEnterIntCount>=(int_count+10)) || (gulEnterIntCount<=(int_count-10)) ) {
//	wsprintf(debugstr, "Int count after 1 sec = %d", gulEnterIntCount);
//	MessageBox(NULL, debugstr, "Verify IRQ", MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, gulEnterIntCount);
      return(ERR_IRQ_SETTING_BAD);
   }

   return(TRUE);
}

int NEAR VerifyFrameAddr(WORD wFrameAddr)
{
   /* Frame address must be 0xD000 or 0xE000, or we know it is bad. */
   if ( ((BYTE)(wFrameAddr>>12) >= 0x0B && (BYTE)(wFrameAddr>>12) <= 0x0E) ) {
      if ( wFrameAddr & 0x0FFF )
	 return(ERR_FRAME_ADDR_SETTING_BAD);
   }
   else
      return(ERR_FRAME_ADDR_SETTING_BAD);
   return(TRUE);
}


int NEAR  VerifyRamdac(void)
{
   BYTE  bEntryNo, bRed, bGreen, bBlue;
   ULONG lOldRGB, lNewRGB;

   randomize();  //Initialize the random number generator
//   for ( i=0; i<64; i++) {
      bEntryNo = random(MAXCOLORENTRY);
      // save original value
      set_ramdac_address_read_reg(bEntryNo);
      lOldRGB = get_ramdac_data_reg_Long();
      bRed   = random(64);   //0x3f
      bGreen = random(64);
      bBlue  = random(64);
      // write new value
      set_ramdac_address_write_reg(bEntryNo);
      set_ramdac_data_reg(bRed, bGreen, bBlue);
      // read new value
      set_ramdac_address_read_reg(bEntryNo);
      lNewRGB = get_ramdac_data_reg_Long();

      if ( !( ((BYTE)(lNewRGB) >> 2)	 == bRed   &&
	      ((BYTE)(lNewRGB>>8) >> 2)  == bGreen &&
	      ((BYTE)(lNewRGB>>16) >> 2) == bBlue ) )
	 return(FALSE);
      // restore old value
      set_ramdac_address_write_reg(bEntryNo);
      set_ramdac_data_reg( ((BYTE)(lOldRGB) >> 2),
			   ((BYTE)(lOldRGB>>8) >> 2),
			   ((BYTE)(lOldRGB>>16) >> 2) );
//  }

  return(TRUE);

} // end function VrerifyRamdac(void)



int NEAR alloc_and_check_bank_access(void)
{
   int	i, j, nLines, bank_pos;
   BYTE display_mode;
   //12/13/93 Tang added
   LPSTR  lpData;

   /* set video mode to video only, so write to memory cannot show at screen */
   display_mode = gConfig.bDisplayMode;
   JUN_set_display_mode(EXTERNAL_VIDEO_ONLY);

   gbBankType = BANK_SIZE_16K;	//default bank size
   bank_pos = 3;		//default bank base

   while (TRUE) {
      switch ( gbBankType ) {
	 case BANK_SIZE_16K :
	   if ( ! (gwFrameBufSel || gbDiagnose) )
	     if ( ! AllocSel((WORD)bank_pos*MEMORY_SIZE_16K, (WORD)MEMORY_SIZE_16K) )
	       return(ERR_CANNOT_ALLOCATE_SELECTOR);
	   nLines = 16;
	   break;
	 case BANK_SIZE_8K :
	   if ( ! (gwFrameBufSel || gbDiagnose) )
	     if ( ! AllocSel((WORD)bank_pos*MEMORY_SIZE_8K, (WORD)MEMORY_SIZE_8K) )
	       return(ERR_CANNOT_ALLOCATE_SELECTOR);
	   nLines = 8;
	   break;
      }

      /* special process for test version */
      /* restore to original mode */
#ifdef TEST_VER
      gConfig.bDisplayMode = display_mode;
      JUN_set_display_mode(gConfig.bDisplayMode);
      return (TRUE);
#endif

      /* set corresponding bank address */
      select_bank_address(gbBankType, bank_pos);

      /* select a video memory segment and a bank segment to test */
      memory_seg_select(bank_pos);
      select_bank_segment(gbBankType, bank_pos);

      // fill this segment with pattern 0x55
      for ( i=0; i<nLines; i++ )
	 for ( j=0; j<1024; j++ )
	  glpVideoMemory[(i<<10) + j] = 0x55;

      // check if this segment if full of pattern 0x55
      for ( i=0; i<nLines; i++ )
	 for ( j=0; j<1024; j++ )
#ifndef TEST_VER
	    if ( glpVideoMemory[(i<<10) + j] != 0x55 )
	       goto change_bank;
#endif

      // fill this segment with pattern 0xaa
      for ( i=0; i<nLines; i++ )
	 for ( j=0; j<1024; j++ )
	    glpVideoMemory[(i<<10) + j] = 0xaa;

      // check if this segment if full of pattern 0xaa
      for ( i=0; i<nLines; i++ )
	 for ( j=0; j<1024; j++ )
#ifndef TEST_VER
	    if ( glpVideoMemory[(i<<10) + j] != 0xaa )
	       goto change_bank;
#endif

      /* restore to original mode */
      gConfig.bDisplayMode = display_mode;
      JUN_set_display_mode(gConfig.bDisplayMode);

      return(TRUE);

change_bank:
      FreeSel();
      gwFrameBufSel = 0;
      gwNSelectors = 1;
      if ( (--bank_pos) < 0 ) {
//12/27 Tang updated ! no need !
//12/27 	if ( gbBankType == BANK_SIZE_8K ) {
//12/27 Tang updated ! no need !
//	      wsprintf(debugstr, "Frame address setting at 0x%X can't be accessed", gConfig.wFrameAddr);
//	      MessageBox(NULL, debugstr, "Check Bank", MB_OK | MB_ICONEXCLAMATION);
	    outportb(0x80, (BYTE)gbBankType);
	    return(ERR_BANK_ACCESS_CHECK_ERROR);
//12/27 	}
//12/27 Tang updated ! no need !
//12/27 	else {
//12/27 	   gbBankType = BANK_SIZE_8K;
//12/27 	   bank_pos = 7;
//12/27 	}
      }

   }  /* end of while */

}


char szMemErrFileSpec[256];
#define MEM_ERR_FILE_NAMEW	    "%s\\junmem.err"        //Windows
/************************************************************************
* check_frame_buffer
*
*   Check the selected frame buffer whether can be accessd or not.
*
* Return: TRUE : Yes
*	  FALSE : there is section of memory that can't be access
************************************************************************/
int NEAR check_frame_buffer(void)
{
   BYTE  img_data;
   int	 fd, i, j, errflag;

   errflag = TRUE;

   GetWindowsDirectory((LPSTR)szMemErrFileSpec, 144);
   wsprintf((LPSTR)szMemErrFileSpec, MEM_ERR_FILE_NAMEW, (LPSTR)szMemErrFileSpec);
   if ((fd = filecreat((LPSTR)&szMemErrFileSpec, F_MODE)) == -1)
       return(FALSE);	/* Could not create file */

   /* set video to external_video_only mode to disable screen display */
   JUN_set_display_mode(EXTERNAL_VIDEO_ONLY);

   // select a video memory segment
   memory_seg_select(0);

   // fill this segment with pattern 0x55
   for (i=0; i<64; i++)
      for (j=0; j<1024; j++)
	  glpVideoMemory[i<<10 + j] = 0x55;

   // read back and check if this segment full of pattern
   for (i=0; i<64; i++)
      for (j=0; j<1024; j++) {
	  img_data = glpVideoMemory[i<<10 + j];
	  if ( img_data != 0x55 ) {
	     stringf(szBuffer,"(%4d,%2d) W:0x55, R:0x%X\r\n",
		     j, i, img_data);
	     filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer));
	     errflag = FALSE;
	  }
      }

   // fill this segment with pattern 0xAA
   for (i=0; i<64; i++)
      for (j=0; j<1024; j++)
	  glpVideoMemory[i<<10 + j] = 0xAA;

   // read back and check if this segment full of pattern
   for (i=0; i<64; i++)
      for (j=0; j<1024; j++) {
	  img_data = glpVideoMemory[i<<10 + j];
	  if ( img_data != 0xAA ) {
	     stringf(szBuffer,"(%4d,%2d) W:0xAA, R:0x%X\r\n",
		     j, i, img_data);
	     filewrite(fd, (LPSTR)szBuffer, strlen(szBuffer));
	     errflag = FALSE;
	  }
      }

   fileclose(fd);		/* Close file */

   return(errflag);

} // end function check_frame_buffer(void)



/****************************************************************************
* init_io_reg_val
*
*   Set JUNIOR register name and its values.
*
****************************************************************************/
int NEAR init_io_reg_val(void)
{
  // initialize addresses of I/O registers
  control_reg		= gConfig.wPortBaseAddr + 0x00; // control reg.
  status_reg		= gConfig.wPortBaseAddr + 0x00; // status reg.
  color_key_reg 	= gConfig.wPortBaseAddr + 0x01; // color key reg.
  card_id_reg		= gConfig.wPortBaseAddr + 0x02; // card ID reg.
  crt_sel_reg		= gConfig.wPortBaseAddr + 0x03; // CRT select reg.
  crt_data_reg		= gConfig.wPortBaseAddr + 0x08; // CRT data reg.
  ramdac_addr_write_reg = gConfig.wPortBaseAddr + 0x04; // RAMDAC address write reg.
  ramdac_addr_read_reg	= gConfig.wPortBaseAddr + 0x07; // RAMDAC address read reg.
  ramdac_data_reg	= gConfig.wPortBaseAddr + 0x05; // RAMDAC data reg.
  ramdac_input_mask_reg = gConfig.wPortBaseAddr + 0x06; // RAMDAC input mask reg.

  // initialize values of I/O registers
  control_reg_val	    = 0x20;  // 0010 0000 control reg.
  status_reg_val	    = 0x61;  // 0000 0000 status reg.
  color_key_reg_val	    = 0xfc;  // 1111 1100 color key reg.
  card_id_reg_val	    = 0x0e;  // 0000 1110 card ID reg.
  crt_sel_reg_val	    = 0x64;  // 1100 0000 CRT select reg.
  crt_data_reg_val	    = 0x00;  // 0110 0100 CRT data reg.
  ramdac_addr_write_reg_val = 0x00;  // 0000 0000 RAMDAC address write reg.
  ramdac_addr_read_reg_val  = 0x00;  // 0000 0000 RAMDAC address read reg.
  ramdac_input_mask_reg_val = 0xff;  // 0000 0000 RAMDAC input mask reg.
  ramdac_data_reg_val	    = 0L;    // 0000 0000 RAMDAC data reg.

  return(TRUE);
} // end function init_io_register()


/************************************************************************
* SetConfiguration
*
*   This routine is responsible for setting OVERLAY JUNIOR hardware
* (registers) from the configuration information we got by ini filw.
*
* Exported:  Yes
* Entry:     None
* Return:    TRUE,	if success,
*	     ErrorCode, else failure.
************************************************************************/
int NEAR SetConfiguration(void)
{
   /* 2x2 card-id register */
   JUN_set_card_id(gConfig.bCardID);
   /* 2x1 color-key register */
   JUN_set_color_key(gConfig.bColorKey);
   /* 2x0 control register */
   JUN_set_display_mode(gConfig.bDisplayMode);
   /* 2x3 crt-select(02:blank-timing) register */
   JUN_reset_whole_display_color();    // the whole TV not set to a color
   JUN_set_scan_mode(gConfig.bScanMode);

   /* Get status of JUNIOR card from status register */
   gfExtrnVideoExist = (BYTE)JUN_external_video_exist();
   gbTVsystemType = (BYTE)JUN_get_TVsystem_type();

   // 3/8/94 Tang added for PAL
   outportb(0x80, gbTVsystemType);
   if ( gbTVsystemType == PAL )
      gwVertRegOffset = 0xF0;
   else
      gwVertRegOffset = 0x00;
   // Vertical Scroll Reg. is set to F0 for PAL ver., but the origin of
   // the video memory is (0,0) as usual !
   gConfig.wVertScanStart += gwVertRegOffset;
   // 3/8/94 Tang added for PAL

   /* 2x3 crt-select(00:vertical-scroll) register */
   set_vert_scroll_reg((BYTE)gConfig.wVertScanStart);
   /* 2x3 crt-select(03:horizontal-shift) register */
   set_horz_shift_reg (gConfig.wHorzScanStart);
   /* 2x6 ramdac-input-mask register */
   set_ramdac_input_mask_reg(ramdac_input_mask_reg_val);

//   memory_read_enable(TRUE);	    //default
//   memory_write_enable(TRUE);
//   JUN_flash_video_memory(gConfig.bColorKey);

   /* 2x4,2x5 set RAMDAC register */
   if ( ! JUN_GetSystemPalette(system_palette) ) {
      outportb(0x80, 0x99);
      /* use default palette to set RAMDAC */
      JUN_init_ramdac_entry(palette_table);
   }
   else
      /* set RAMDAC to Windows system palette */
      JUN_init_ramdac_entry(system_palette);

   return(TRUE);
}

/************************************************************************
* JUN_Initialize
*
*   This routine is called to initialized Overlay Junior board.
* It loads the configuration file, and initializes all the Junior
* registers.
*
* Exported:  Yes
* Entry:     None
* Destroys:
* Exit:      TRUE for success, FALSE for failure
************************************************************************/
int FAR PASCAL JUN_Initialize(HANDLE hWnd)
{
   int	i;

//   hWnd = hwnd;
#ifdef TEST_VER
//   ShowFreeSpace("before init");
#endif
   /* Step 1. Check the times of initialization */
   if ( gbInitial ) {
//	wsprintf(debugstr, "JUNIOR has been initialized before!");
//	MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x80);
      gwErrorCode = ERR_INITIALIZED_BEFORE;
      return(gwErrorCode);
   }

   //12/31/93 Tang
   InitialGlobals();

   /* Step 2. Load JUNIOR.INI file to get configuration */
   if ( ! JUN_LoadConfiguration() ) {
//	wsprintf(debugstr, "JUNIOR.INI file not found!");
//	MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x81);
      gwErrorCode = ERR_JUNIOR_INI_NOT_FOUND;
      return(gwErrorCode);
   }

   /* Initialize I/O registers & their valuse */
   init_io_reg_val();

   /* Step 3. Verify configuration is correct */
   gwErrorCode = TRUE;
   gwErrorCode = VerifyConfiguration();
   if ( gwErrorCode != TRUE ) {
//	wsprintf(debugstr, "Configuration Error %d", gwErrorCode);
//	MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x81);
      return(gwErrorCode);
   }

   /* Step 4. Set JUNIOR configuration */
   SetConfiguration();

   /* check the JUNIOR card is exist */
   if ( ! JUN_junior_exist() ) {
//	wsprintf(debugstr, "JUNIOR card not exist!");
//	MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
      outportb(0x80, 0x82);
      gwErrorCode = ERR_JUNIOR_CARD_NOT_EXIST;
      return(gwErrorCode);
   }

   //12/23/93 Tang addeed
//   if ( ! CheckVGAMode() ) {
//	outportb(0x80, 0x83);
//	gwErrorCode = ERR_VGA_MODE_VIOLATION;
//	return(gwErrorCode);
//   }


   /* Step 5. Allocate a selector which is 64K size to access video memory.
	      The selector's base address is set to frame buffer address. */

#ifdef DOSLIB
   gwFrameBufSel = gConfig.wFrameAddr;
#else
   gwNSelectors = 1;
   gwErrorCode = alloc_and_check_bank_access();
   if ( gwErrorCode != TRUE )
      return(gwErrorCode);
//DPMI.ASM   if ( !AllocSelectors() ) { //Allocate N selectors from LDT by DPMI
// 11/10/93 Tang masked for demo
//12/06   if ( !AllocSel() ) { //Allocate N selectors from LDT by Windows API
//12/06      wsprintf(debugstr, "Selector allocate error!");
//12/06      MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
//12/06      outportb(0x80, 0x83);
//12/06      gwErrorCode = ERR_CANNOT_ALLOCATE_SELECTOR;
//12/06      return(gwErrorCode);
//12/06   }
// 11/10/93 Tang masked for demo
//   if ( ! AllocSelect() ) {
//	outportb(0x80, 0x83);
//	return(FALSE);
//   }
#endif

   /* Step 5. Check the selected frame buffer for access video memory later */
// 11/10/93 Tang masked for demo
//   if ( ! check_frame_buffer() ) {
//	outportb(0x80, 0x84);
//	gwErrorCode = ERR_VRAM_ACCESS_ERROR;
//	return(gwErrorCode);
//   }
// 11/10/93 Tang masked for demo

   JUN_flash_video_memory(gConfig.bColorKey);

   //12/01/93 Tang added
   if ( AllocPageBuffer() != TRUE ) {
      FreeSel();
      return(ERR_INSUFFICIENT_MEMORY);
   }

   gbInitial++;     //Increment times of Initialization

   return(TRUE);
}


/************************************************************************
* JUN_Exit
*
*  Called by Windows when the DLL is discarded.  Disable video, free
* selectors to the video buffer, and save the current configuration.
*   Routine to save the configuration file junior.ini.
*
* Exported:  Yes
* Entry:     None
************************************************************************/
int FAR PASCAL JUN_Exit(void)
{
   /* not initialize before, do nothing and return TRUE */
   if ( gbInitial == 0 )  return(TRUE);
//12/21   if ( gbInitial != 1 )  return(FALSE);

   /* Step 1. Free selector */
//   FreeSelectors();
   if ( ! FreeSel() ) {
#ifdef TEST_VER
      wsprintf(debugstr, "Free Selector Error!");
      MessageBox(NULL, debugstr, "JUNIOR", MB_OK | MB_ICONEXCLAMATION);
#endif
   }

   /* Step 2. Restore Windows palette */
   JUN_SetSystemPalette(system_palette);

   /* Step 3. Disable all JUNIOR card */
   disable_card();

   /* Step 4. Save configuration to JUNIOR.INI file */
//   if ( ! JUN_SaveConfiguration() ) {
//	gwErrorCode = ERR_WRITE_JUNIOR_INI_ERROR;
//	return(gwErrorCode);
//   }

   //12/01/93 Tang added
   FreePageBuffer();

   gbInitial = 0;
   gbDiagnose = 0;
   gwFrameBufSel = 0;
#ifdef TEST_VER
//   ShowFreeSpace("before exit");
#endif
   return(TRUE);
}


int FAR PASCAL JUN_SetSystemPalette(PALETTEENTRY palette[])
{
   LPLOGPALETTE  lpPal;
   HANDLE	 hLogPal;
   HPALETTE	 hPal, hOldPal = NULL;
   HDC		 hDC;
   int		 i, nEntries;

   hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) +
			 sizeof(PALETTEENTRY) * MAXCOLORENTRY);

   if (!hLogPal) {
      outport(0x80, 0x88);
      gwErrorCode = ERR_MEMORY_ALLOCATE_ERROR;
      return(gwErrorCode);
   }

   lpPal = (LPLOGPALETTE) GlobalLock(hLogPal);

   //construct a logical palette
   lpPal->palVersion = PALVERSION;
   lpPal->palNumEntries = MAXCOLORENTRY;

   for ( i=0; i<MAXCOLORENTRY; i++ ) {
      lpPal->palPalEntry[i].peRed   = palette[i].peRed;
      lpPal->palPalEntry[i].peGreen = palette[i].peGreen;
      lpPal->palPalEntry[i].peBlue  = palette[i].peBlue;
      lpPal->palPalEntry[i].peFlags = 0;
   }

   hPal = CreatePalette(lpPal);
   if ( ! hPal )  {
      GlobalUnlock(hLogPal);
      GlobalFree(hLogPal);
      return(FALSE);
   }

   hDC = GetDC(NULL);

   if ( hPal ) {
      /* select a specified logical palette into device context */
      hOldPal = SelectPalette(hDC, hPal, FALSE);
      /* map palette entries from current logical palette to system palette */
      nEntries = RealizePalette(hDC);
   }

   GlobalUnlock(hLogPal);
   GlobalFree(hLogPal);

   if ( hOldPal )
      SelectPalette(hDC, hOldPal, FALSE);

   DeleteObject(hPal);
   ReleaseDC(NULL, hDC);

   return(TRUE);
}


int FAR PASCAL JUN_GetSystemPalette(PALETTEENTRY palette[])
{
//   LPLOGPALETTE  lpPal;
//   HANDLE	   hLogPal;
//   HPALETTE	   hPal = NULL;
   HDC		 hDC;
   int		 wNumColors;
   int		 tt;

   hDC = GetDC(NULL);

   tt = GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE;
   if ( !tt ) {
      ReleaseDC(NULL, hDC);
      return(FALSE);
   }

   wNumColors = GetDeviceCaps(hDC, SIZEPALETTE);

   GetSystemPaletteEntries(hDC, 0, wNumColors,
			   palette);

   ReleaseDC(NULL, hDC);

   return(TRUE);
}


/****************************************************************************
*  JUN_RetrieveColorKey(long rgbColor)
*
*   Get the entry of palette to set the color key index for RAMDAC.
*
* Return: color_key_entry
****************************************************************************/
int FAR PASCAL JUN_RetrieveColorKey(long lRGBcolor)
{
   int		 i;
   HDC		 hDC;
//   PALETTEENTRY  pe[MAXCOLORENTRY];
   int		 wNumColors;

   hDC = GetDC(NULL);

   if ( !(GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE) ) {
      ReleaseDC(NULL, hDC);
      return(FALSE);
   }

   wNumColors = GetSystemPaletteEntries(hDC, 0, GetDeviceCaps(hDC, SIZEPALETTE),
			   pe);

   if ( !wNumColors )  return(FALSE);

   i = 0;
   while ( pe[i].peRed	 != (BYTE)lRGBcolor ||
	   pe[i].peGreen != ((BYTE)lRGBcolor>>8) ||
	   pe[i].peBlue  != ((BYTE)lRGBcolor>>16) ) {
       i++;
       if ( i == 256 ) break;
       if ( i == 10 )  i = 246;
   }

   ReleaseDC(NULL, hDC);

   return(i);
}


/****************************************************************************
* JUN_ColorentryToRGB(BYTE color_entry)
*
*   Get the RGB value of RAMDAC entry .
*
* Return: Color_RGB(long integer 0x00bbggrr)
****************************************************************************/
long FAR PASCAL  JUN_ColorentryToRGB(BYTE color_entry)
{
   BYTE *red_color, *green_color, *blue_color;
//   UINT	*red_color, *green_color, *blue_color;
   long temp_r=0L, temp_g=0L, temp_b=0L;
   ULONG lRGBcolor;

   // set the first RAMDAC entry to read
   set_ramdac_address_read_reg((BYTE)color_entry);

   // get RAMDAC data 3 times to get R, G, B color
//   get_ramdac_data_reg(red_color, green_color, blue_color);
   lRGBcolor = get_ramdac_data_reg_Long();

//   wsprintf(debugstr, "RGBlong==%ld", lRGBcolor);
//   MessageBox(NULL, "Color Value!", debugstr, MB_OK | MB_ICONEXCLAMATION);

//   wsprintf(debugstr, "R=%d, G=%d, B=%d", *red_color, *green_color, *blue_color);
//   MessageBox(NULL, "Color Value!", debugstr, MB_OK | MB_ICONEXCLAMATION);

   // convert RGB value to a long intrger
//   temp_r  = (long)(((*red_color   & 0x3f) << 2) | 0x03);
//   temp_g  = (long)(((*green_color & 0x3f) << 2) | 0x03);
//   temp_b  = (long)(((*blue_color  & 0x3f) << 2) | 0x03);

//   return( (long) ((temp_b << 16) + (temp_g << 8) + temp_r) );
   return(lRGBcolor);

}


int  section[3];
/****************************************************************************
* JUN_RGBToColorentry(long rgbColor)
*
*   Get the RAMDAC entry which correspond to RGB value.
*
* Return: Color entry(0-255)
****************************************************************************/
int FAR PASCAL	JUN_RGBToColorentry(long lRGBcolor)
{
   BYTE  red_color, green_color, blue_color;
//   BYTE *temp_r, *temp_g, *temp_b;
   BYTE  temp_r, temp_g, temp_b;
   int	 i, j, idx;
//   int   section[3];
   ULONG temp_lRGB;

   red_color   = (BYTE) (lRGBcolor & 0x000000ff);
   green_color = (BYTE) ((lRGBcolor>>8) & 0x000000ff);
   blue_color  = (BYTE) ((lRGBcolor>>16) & 0x000000ff);

//   wsprintf(debugstr, "R=%d, G=%d, B=%d", red_color, green_color, blue_color);
//   MessageBox(NULL, "Input Color Value!", debugstr, MB_OK | MB_ICONEXCLAMATION);

   section[0] = section[1] = 10;
   section[2] = 236;

   idx = 0;    //index of ramdac entry

   /* divided into 3 sections 0-9, 246-255,10-245, to find the color entry */
   for ( j=0; j<3; j++ ) {
      /* set the first RAMDAC entry to read */
      set_ramdac_address_read_reg(idx);
      /* check the obtained R,G,B value */
      for ( i=0; i<section[j]; i++ ) {	  //see a ghost!
//	   get_ramdac_data_reg(temp_r, temp_g, temp_b);
//	   wsprintf(debugstr, "E=%d, R=%d, G=%d, B=%d", i, *temp_r, *temp_g, *temp_b);
//	   MessageBox(NULL, "Color Value!", debugstr, MB_OK | MB_ICONEXCLAMATION);
	 temp_lRGB = get_ramdac_data_reg_Long();
	 temp_r = (BYTE) (temp_lRGB	  & 0x000000ff);
	 temp_g = (BYTE) ((temp_lRGB>>8)  & 0x000000ff);
	 temp_b = (BYTE) ((temp_lRGB>>16) & 0x000000ff);

//	   wsprintf(debugstr, "I=%d, R=%d, G=%d, B=%d", i, (temp_r>>2), (temp_g>>2), (temp_b>>2);
//	   MessageBox(NULL, "Color Value!", debugstr, MB_OK | MB_ICONEXCLAMATION);

//	   if ( (*temp_r == (red_color>>2)) &&
//		(*temp_g == (green_color>>2)) &&
//		(*temp_b == (blue_color>>2))  )
	 if ( ((temp_r>>2) == (red_color>>2))	&&
	      ((temp_g>>2) == (green_color>>2)) &&
	      ((temp_b>>2) == (blue_color>>2)) )
	    return(idx);
	 idx ++;     //index of ramdac entry
      } /* end of for (i) */
      if ( j == 0 )
	 idx = 246;
      else
	 idx = 10;
   } /* end of for (j) */

   return(MAXCOLORENTRY);

}

/****************************************************************************
* JUN_get_color_key(void)
*
*   Get the current entry of RAMDAC which the color key set.
*
* Return: color_key_entry
****************************************************************************/
int FAR PASCAL	JUN_get_color_key(void)
{
   return( (int)gConfig.bColorKey );
}


/****************************************************************************
* JUN_get_display_mode(void)
*
*   Get the current display mode.
*
* Return: display_mode
****************************************************************************/
int FAR PASCAL	JUN_get_display_mode(void)
{
   return( (int)gConfig.bDisplayMode );
}


/****************************************************************************
* JUN_get_scan_mode(void)
*
*   Get the current scan mode.
*
* Return: scan_mode
****************************************************************************/
int FAR PASCAL	JUN_get_scan_mode(void)
{
   return( (int)gConfig.bScanMode );
}



int NEAR AllocPageBuffer(void)
{
   DWORD	dwDataSize, dwActAllocSize;
   WORD 	padded;

   /* Allocate 2 buffers used for WriteBitmap and ISR to scroll pages */
   padded = gwVideoWidth % 2;
   /* insure to make the width is a multiple of 2 */
   dwDataSize = (DWORD)(gwVideoWidth+padded) * gwVideoHeight;

   //01/06/94 Tang added
   dwActAllocSize = GlobalCompact(dwDataSize);
   //01/06/94 Tang added
   ghMem[0] = GlobalAlloc(GHND, dwDataSize);
   if ( !ghMem[0] ) {
      outportb(0x80, 0x08);
#ifdef TEST_VER
      dwActAllocSize = GlobalCompact(dwDataSize);
      wsprintf(debugstr, "1: Request=%ld, Actual=%ld", dwDataSize, dwActAllocSize);
      MessageBox(NULL, debugstr, "GlobalAlloc Memory Error!", MB_OK | MB_ICONEXCLAMATION);
#endif
      gwErrorCode = ERR_MEMORY_ALLOCATE_ERROR;
      return(gwErrorCode);
   }
   ghpBitsStart[0] = glpPageBuf = GlobalLock(ghMem[0]);

   //01/06/94 Tang added
   dwActAllocSize = GlobalCompact(dwDataSize);
   //01/06/94 Tang added
   ghMem[1] = GlobalAlloc(GHND, dwDataSize);
   if ( !ghMem[1] ) {
      outportb(0x80, 0x08);
#ifdef TEST_VER
      wsprintf(debugstr, "2: Request=%ld, Actual=%ld", dwDataSize, dwActAllocSize);
      MessageBox(NULL, debugstr, "GlobalAlloc Memory Error!", MB_OK | MB_ICONEXCLAMATION);
#endif
      GlobalUnlock( ghMem[0] );
      GlobalFree( ghMem[0] );
      gwErrorCode = ERR_MEMORY_ALLOCATE_ERROR;
      return(gwErrorCode);
   }
   ghpBitsStart[1] = GlobalLock(ghMem[1]);

   InitialPageBuffer();

   return(TRUE);
}

void FAR InitialPageBuffer(void)
{
   DWORD	dwDataSize, dwSegBoundary, dwActAllocSize;
   WORD 	padded;
   int		i, j, k;
   BYTE huge	*hpBits;

   padded = gwVideoWidth % 2;
   /* fill two buffer with color key */
   for ( k=0; k<2; k++ ) {
      hpBits = ghpBitsStart[k];
      for ( i=0; i<gwVideoHeight; i++ ) {
	 // byte left in 64K segment //
	 dwSegBoundary = (0xFFFF ^ LOWORD(hpBits)) + 1;
	 if (dwSegBoundary >= (DWORD)(gwVideoWidth+padded) )
	    for ( j=0; j<gwVideoWidth+padded; j++ ) {
	       *hpBits = gConfig.bColorKey;
	       hpBits  += 1;
	    }
	 // take care of scan line on 64K segment //
	 else {
	    for ( j=0; j<(WORD)dwSegBoundary; j++ ) {
	       *hpBits = gConfig.bColorKey;
	       hpBits  += 1;
	    }
	    for ( j=0; j<(gwVideoWidth+padded)-dwSegBoundary; j++ ) {
	       *hpBits = gConfig.bColorKey;
	       hpBits  += 1;
	    }
	 }
      } // end of for (i) //
   } // end of for (k) //
}


int NEAR FreePageBuffer(void)
{
   /* free allocated memory */
   GlobalUnlock( ghMem[0] );
   GlobalFree( ghMem[0] );
   GlobalUnlock( ghMem[1] );
   GlobalFree( ghMem[1] );
   return(TRUE);
}

int NEAR CheckVGAMode(void)
{
   WORD  gwVGAModeHorzRes, gwVGAModeVertRes, gwVGAModeBitsPixel;
   WORD  wColorPlanes, wNumColors;
   HDC	 hDC;

   hDC = GetDC(NULL);
   gwVGAModeHorzRes   = GetDeviceCaps(hDC, HORZRES);
   gwVGAModeVertRes   = GetDeviceCaps(hDC, VERTRES);
   gwVGAModeBitsPixel = GetDeviceCaps(hDC, BITSPIXEL);
   wColorPlanes       = GetDeviceCaps(hDC, PLANES);
   wNumColors  = (1 << GetDeviceCaps(hDC, BITSPIXEL));
   ReleaseDC(NULL, hDC);
   wsprintf(debugstr, "HorzRes=%d, VertRes=%d, NumColors=%d, Planes=%d",
	    gwVGAModeHorzRes, gwVGAModeVertRes, wNumColors, wColorPlanes);
   MessageBox(NULL, debugstr, "Debug", MB_OK | MB_ICONEXCLAMATION);
   switch ( gbTVsystemType ) {
      case NTSC:
	   if ( !(gwVGAModeHorzRes == 640 && gwVGAModeVertRes == 480 &&
		  gwVGAModeBitsPixel == 8 && wColorPlanes == 1) )
	      return(FALSE);
	   break;
      case PAL:
	   if ( !(gwVGAModeHorzRes == 800 && gwVGAModeVertRes == 600 &&
		  gwVGAModeBitsPixel == 8 && wColorPlanes == 1) )
	      return(FALSE);
	   break;
   }

   return(TRUE);
}


void NEAR InitialGlobals(void)
{
   int	i;

   gwVideoWidth = 720;	 // width of video memory at current scan mode
   gwVideoHeight = 480;  // height of video memory at current scan mode

   /* Used in libinit.asm */
   gwFrameBufSel = 0;	 // frame buffer selector(given by Windows)
   gwNSelectors = 1;	 // no of selectors to allocate

   gbInitial = 0,	 // times of initialization
   gbTVsystemType = 0,	 // type of TV system(0:NTSC, 2:PAL)
   gbMemReadMode  = 0,	 // read mode of video memory(ENABLE/DISABLE)
   gbMemWriteMode = 0,	 // write mode of video memory(ENABLE/DISABLE)
   gfExtrnVideoExist = 0;// flag of external video exist

   gulEnterIntCount=0;	 // times of interrupt occur

   /* junisr.c for scrolling */
   gfHandleScroll = FALSE;     // flag to initinate ISR to start
   gfScrollStop = TRUE;        // flag indicate last page
   for ( i=0; i<2; i++ ) {
      gwPadded[i] = 0;	       // padded no. to make a multiple of 2
      ghMem[i] = 0;	       // global handle of memory
      gfBufferFull[i] = FALSE; // flag to indicate buffer is full or not
   }

// 11/18/93 Tang added
//   gfStartScroll = FALSE;    // semaphore to isr to start scrolling

/* 12/06/93 Tang added */
   gbBankType = 1;
   gbDiagnose = 0;

/* 12/22/93 Tang added */
   gbVideoMode = 0;
   gwImgWidthPad = 0;

   gbPreMemSeg = 9;	     // previous selected memory segment
   gbPreBnkNo = 9;	     // previous selected bank no.

   // 3/9/94 Tang added //
   gwVertRegOffset = 0;

}

//01/05/94
void FAR ShowFreeSpace(LPSTR lpPromptMsg)
{
   DWORD dwFreeSpace;

   dwFreeSpace = GetFreeSpace(0);
   wsprintf(debugstr, "Memory:  %ld KB Free", dwFreeSpace/1024);
   MessageBox(NULL, debugstr, lpPromptMsg, MB_OK | MB_ICONEXCLAMATION);
}

// end of juninit.c //
