unit junor2;

interface

implementation

Function FreeSel;
begin
   GlobalUnlock(gwFrameBufSel);
   if  FreeSelector(gwFrameBufSel) = 0 then
      result:=TRUE
   else result:=FALSE;
end;

{************************************************************************
* SetConfiguration
*
*   This routine is responsible for setting OVERLAY JUNIOR hardware
* (registers) from the configuration information we got by ini filw.
*
* Exported:  Yes
* Entry:     None
* Return:    TRUE,	if success,
*	     ErrorCode, else failure.
************************************************************************}
Function SetConfiguration:Boolean;
begin
   // 2x2 card-id register */
   JUN_set_card_id(gConfig.bCardID);
   //* 2x1 color-key register */
   JUN_set_color_key(gConfig.bColorKey);
   // 2x0 control register */
   JUN_set_display_mode(gConfig.bDisplayMode);
   // 2x3 crt-select(02:blank-timing) register */
   JUN_reset_whole_display_color();    // the whole TV not set to a color
   JUN_set_scan_mode(gConfig.bScanMode);

   // Get status of JUNIOR card from status register */
   gfExtrnVideoExist := JUN_external_video_exist();
   gbTVsystemType := JUN_get_TVsystem_type();

   // 3/8/94 Tang added for PAL
   PortWriteByte($80, gbTVsystemType);
   if gbTVsystemType = PAL then
      gwVertRegOffset := $F0
   else
      gwVertRegOffset := 0;
   // Vertical Scroll Reg. is set to F0 for PAL ver., but the origin of
   // the video memory is (0,0) as usual !
   gConfig.wVertScanStart := gConfig.wVertScanStart+gwVertRegOffset;
   // 3/8/94 Tang added for PAL

   // 2x3 crt-select(00:vertical-scroll) register */
   set_vert_scroll_reg(gConfig.wVertScanStart);
   // 2x3 crt-select(03:horizontal-shift) register */
   set_horz_shift_reg (gConfig.wHorzScanStart);
   // 2x6 ramdac-input-mask register */
   set_ramdac_input_mask_reg(ramdac_input_mask_reg_val);

   // 2x4,2x5 set RAMDAC register */
   if JUN_GetSystemPalette(system_palette)=false then
   begin
      Portwritebyte($80, $99);
      // use default palette to set RAMDAC */
      JUN_init_ramdac_entry(palette_table);
   end
   else
      // set RAMDAC to Windows system palette */
      JUN_init_ramdac_entry(system_palette);

   result:=TRUE;
end;

Function alloc_and_check_bank_access:integer;
var
   i, j, nLines, bank_pos:integer;
   display_mode:BYTE;
   lpData:LPSTR;
label change_bank;
begin
   // set video mode to video only, so write to memory cannot show at screen */
   display_mode := gConfig.bDisplayMode;
   JUN_set_display_mode(EXTERNAL_VIDEO_ONLY);

   gbBankType:= BANK_SIZE_16K;	//default bank size
   bank_pos := 3;		//default bank base

   while (TRUE) do
     case ( gbBankType ) of
       1:
       begin
	   if gwFrameBufSel or gbDiagnose =0 then
	     if AllocSel(bank_pos*MEMORY_SIZE_16K, MEMORY_SIZE_16K)=0 then
       	       result:=ERR_CANNOT_ALLOCATE_SELECTOR;
	   nLines := 16;
	   break;
       end;
       2://BANK_SIZE_8K :
       begin
       	   if gwFrameBufSel or gbDiagnose =0 then
	     if AllocSel(bank_pos*MEMORY_SIZE_8K, MEMORY_SIZE_8K) =0 then
	       result:=ERR_CANNOT_ALLOCATE_SELECTOR;
	   nLines := 8;
	   break;
       end;
      end;

      // special process for test version */
      // restore to original mode */
      // set corresponding bank address */
      select_bank_address(gbBankType, bank_pos);

      // select a video memory segment and a bank segment to test */
      memory_seg_select(bank_pos);
      select_bank_segment(gbBankType, bank_pos);

      // fill this segment with pattern 0x55
      for i:=0 to nLines do
	 for  j:=0 to 1024 do
//f	  glpVideoMemory[(i shl 10) + j] = 0x55;
        ;
      // check if this segment if full of pattern 0x55
      for i:=0 to nLines do
	 for j:=0 to 1024 do
//f	    if ( glpVideoMemory[(i shl 10) + j] <> 0x55 )
	       goto change_bank;

      // fill this segment with pattern 0xaa
      for i:=0 to nLines do
	 for  j:=0 to 1024 do
//f	    glpVideoMemory[(i shl 10) + j] = 0xaa;
             ;
      // check if this segment if full of pattern 0xaa
      for i:=0 to nLines do
	 for  j:=0 to 1024 do
//	    if ( glpVideoMemory[(i<<10) + j] <> 0xaa )
	       goto change_bank;

      // restore to original mode */
      gConfig.bDisplayMode := display_mode;
      JUN_set_display_mode(gConfig.bDisplayMode);

      result:=1;

change_bank:
      FreeSel();
      gwFrameBufSel := 0;
      gwNSelectors := 1;
      if ( (--bank_pos) < 0 ) then
      begin
	    PortwriteByte($80, gbBankType);
	    result:=ERR_BANK_ACCESS_CHECK_ERROR;
      end;
end;

{***********************************************************************
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
************************************************************************}
Function VerifyConfiguration:integer;
var
   wReturnCode:integer;
begin
   // 1. Verify I/O Port AND Card ID */
   wReturnCode := VerifyPortAndID(gConfig.wPortBaseAddr, gConfig.bCardID);
   if ( wReturnCode <> 1 ) then
      result:=wReturnCode;

   // 2. Verify Frame Address */
   wReturnCode := VerifyFrameAddr(gConfig.wFrameAddr);
   if ( wReturnCode <> 1 )   then
      result:=wReturnCode;

   // 3. Vefify IRQ */
   wReturnCode := VerifyIRQ(gConfig.bIRQUsed);
   if ( wReturnCode <> 1 )  then
      result:=wReturnCode;

   if ( (gConfig.bDisplayMode <> EXTERNAL_VIDEO_ONLY) and
	(gConfig.bDisplayMode <> OVERLAY) ) then
      gConfig.bDisplayMode := OVERLAY;

   if ( (gConfig.bScanMode <> UNDERSCAN) and
	(gConfig.bScanMode <> OVERSCAN) )  then
      gConfig.bScanMode := OVERSCAN;

   if ( gConfig.wVertScanStart > $ff ) then
      gConfig.wVertScanStart :=gConfig.wVertScanStart and $ff;

   if ( gConfig.wHorzScanStart > $1ff )  then
      gConfig.wHorzScanStart := gConfig.wHorzScanStart and $1ff;

   result:=1;
end;

Function init_io_reg_val:integer;
begin
  // initialize addresses of I/O registers
  control_reg		:= gConfig.wPortBaseAddr + $00; // control reg.
  status_reg		:= gConfig.wPortBaseAddr + $00; // status reg.
  color_key_reg 	:= gConfig.wPortBaseAddr + $01; // color key reg.
  card_id_reg		:= gConfig.wPortBaseAddr + $02; // card ID reg.
  crt_sel_reg		:= gConfig.wPortBaseAddr + $03; // CRT select reg.
  crt_data_reg		:= gConfig.wPortBaseAddr + $08; // CRT data reg.
  ramdac_addr_write_reg := gConfig.wPortBaseAddr + $04; // RAMDAC address write reg.
  ramdac_addr_read_reg	:= gConfig.wPortBaseAddr + $07; // RAMDAC address read reg.
  ramdac_data_reg	:= gConfig.wPortBaseAddr + $05; // RAMDAC data reg.
  ramdac_input_mask_reg := gConfig.wPortBaseAddr + $06; // RAMDAC input mask reg.

  // initialize values of I/O registers
  control_reg_val	    := $20;  // 0010 0000 control reg.
  status_reg_val	    := $61;  // 0000 0000 status reg.
  color_key_reg_val	    := $fc;  // 1111 1100 color key reg.
  card_id_reg_val	    := $0e;  // 0000 1110 card ID reg.
  crt_sel_reg_val	    := $64;  // 1100 0000 CRT select reg.
  crt_data_reg_val	    := $00;  // 0110 0100 CRT data reg.
  ramdac_addr_write_reg_val := $00;  // 0000 0000 RAMDAC address write reg.
  ramdac_addr_read_reg_val  := $00;  // 0000 0000 RAMDAC address read reg.
  ramdac_input_mask_reg_val := $ff;  // 0000 0000 RAMDAC input mask reg.
  ramdac_data_reg_val	    := 0;    // 0000 0000 RAMDAC data reg.

  result:=1;
end; // end Function init_io_register()

Function JUN_Initialize():integer;
begin
   if gbInitial=true then
   begin
     PortWritebyte($80, $80);
     gwErrorCode := ERR_INITIALIZED_BEFORE;
     result:=gwErrorCode;
   end;
   InitialGlobals();
   if JUN_LoadConfiguration()=false then
   begin
     PortWriteByte($80, $81);
     gwErrorCode := ERR_JUNIOR_INI_NOT_FOUND;
     result:=gwErrorCode;
   end;
   init_io_reg_val();
   gwErrorCode := 1;
   gwErrorCode := VerifyConfiguration();
   if gwErrorCode =0 then
   begin
      PortWriteByte($80, $81);
      result:=gwErrorCode;
   end;
   SetConfiguration();
   if JUN_junior_exist()=false then
   begin
      PortWriteByte($80, $82);
      gwErrorCode := ERR_JUNIOR_CARD_NOT_EXIST;
      result:=gwErrorCode;
   end;
   gwNSelectors := 1;
   gwErrorCode := alloc_and_check_bank_access();
   if gwErrorCode =0 then
      result:=gwErrorCode;
//f   JUN_flash_video_memory(gConfig.bColorKey);
   if AllocPageBuffer() <> TRUE then
   begin
     FreeSel();
     result:=ERR_INSUFFICIENT_MEMORY;
   end;

   inc(gbInitial);     //Increment times of Initialization

   result:=1;
end;

procedure InitialGlobals();
var
   i:integer;
begin
   gwVideoWidth:=720;	 // width of video memory at current scan mode
   gwVideoHeight:=480;  // height of video memory at current scan mode

   // Used in libinit.asm */
   gwFrameBufSel:=0;	 // frame buffer selector(given by Windows)
   gwNSelectors:=1;	 // no of selectors to allocate

   gbInitial:=false;	 // times of initialization
   gbTVsystemType:=0;	 // type of TV system(0:NTSC, 2:PAL)
   gbMemReadMode :=0;	 // read mode of video memory(ENABLE/DISABLE)
   gbMemWriteMode:=0;	 // write mode of video memory(ENABLE/DISABLE)
   gfExtrnVideoExist:=false;// flag of external video exist

   gulEnterIntCount:=0;	 // times of interrupt occur

   // junisr.c for scrolling */
   gfHandleScroll:=FALSE;     // flag to initinate ISR to start
   gfScrollStop:=TRUE;        // flag indicate last page
   for i:=0 to 2 do
   begin
      gwPadded[i]:=0;	       // padded no. to make a multiple of 2
      ghMem[i]:=0;	       // global handle of memory
      gfBufferFull[i]:=FALSE; // flag to indicate buffer is full or not
   end;

// 11/18/93 Tang added
//   gfStartScroll:=FALSE;    // semaphore to isr to start scrolling

// 12/06/93 Tang added */
   gbBankType:=1;
   gbDiagnose:=0;

// 12/22/93 Tang added */
   gbVideoMode:=0;
   gwImgWidthPad:=0;

   gbPreMemSeg:=9;	     // previous selected memory segment
   gbPreBnkNo:=9;	     // previous selected bank no.

   // 3/9/94 Tang added //
   gwVertRegOffset:=0;

end;

Function JUN_LoadConfiguration:Boolean;
var
   fd:integer;
begin
   GetWindowsDirectory(szCfgFileSpec, 144);
   wsprintf(szCfgFileSpec, CFG_FILE_NAMEW);
   fd := fileopen(szCfgFileSpec, fmOpenRead);
   if fd = -1 then
   begin
      portwritebyte($80, $01);
      result:=FALSE;	// File does not exist. */
   end;
//f   if  fileclose(fd) <> 0  then
//f      result:=FALSE);
    fileclose(fd) ;
   GetPrivateProfileString('JUNIOR', 'FrameAddr', '$D000', szData, 10, szCfgFileSpec);
   gConfig.wFrameAddr := strtoint(szData{, @endptr, 0});
//   gConfig.wFrameAddr:=(WORD)atol(szData);

   GetPrivateProfileString('JUNIOR', 'PortBaseAddr', '$280', szData, 10, szCfgFileSpec);
   gConfig.wPortBaseAddr:=strtoint(szData{, ANDendptr, 0});
//   gConfig.wPortBaseAddr:=(WORD)atol(szData);

   GetPrivateProfileString('JUNIOR', 'CardID', '$0E', szData, 10, szCfgFileSpec);
   gConfig.bCardID:=strtoint(szData{, ANDendptr, 0});
//   gConfig.bCardID:=(BYTE)atol(szData);

   GetPrivateProfileString('JUNIOR', 'IRQUsed', '$05', szData, 10, szCfgFileSpec);
   gConfig.bIRQUsed:=strtoint(szData{, AND{endptr, 0});
//   gConfig.bIRQUsed:=(BYTE)atol(szData);

   GetPrivateProfileString('JUNIOR', 'ColorKey', '$FC', szData, 10, szCfgFileSpec);
   gConfig.bColorKey:=strtoint(szData{, ANDendptr, 0});
//   gConfig.bColorKey:=(BYTE)atol(szData);

   GetPrivateProfileString('JUNIOR', 'DisplayMode', 'OVERLAY', szData, 10, szCfgFileSpec);
   if ( stricomp(szData, 'EXTERNAL_VIDEO_ONLY') = 0 )    then
      gConfig.bDisplayMode:=EXTERNAL_VIDEO_ONLY // EXTERNAL_VIDEO_ONLY mode //
   else
      gConfig.bDisplayMode:=OVERLAY;  // OVERLAY mode //

   GetPrivateProfileString('JUNIOR', 'ScanMode', 'OVERSCAN', szData, 10, szCfgFileSpec);
   if ( stricomp(szData, 'OVERSCAN') = 0 )    then
      gConfig.bScanMode:=OVERSCAN // OVERSCAN mode //
   else
      gConfig.bScanMode:=UNDERSCAN;  // UNDERSCAN mode //

   GetPrivateProfileString('JUNIOR', 'VertScanStart', '$00', szData, 10, szCfgFileSpec);
   gConfig.wVertScanStart:=strtoint(szData{, ANDendptr, 0});
//   gConfig.wVertScanStart:=(WORD)atol(szData);

   GetPrivateProfileString('JUNIOR', 'HorzScanStart', '$00', szData, 10, szCfgFileSpec);
   gConfig.wHorzScanStart:=strtoint(szData{, ANDendptr, 0});
//   gConfig.wHorzScanStart:=(WORD)atol(szData);

   result:=TRUE;
end;


           // ***************************************************************************
// JUN_flash_video_memory(BYTE	color_entry) : clear video memory
//   input
//	  flag : TRUE  => enable
//		 FALSE => disable
// ***************************************************************************
Function JUN_flash_video_memory(  color_entry:BYTE):integer;
var
  i, j, temp_ss:integer;
begin
  // step 1. write color_entry to border register
  set_screen_border_reg(color_entry);
  for j:=0 to 4 do
  begin
    // step 2. set control register bit 7
    //	     control_reg_val AND $7f(0111 1111):=$XX XXXX
    //		$XX XXXX
    //	     OR A000 0000
    //	     =	AXXX XXXX
    control_reg_val:=(control_reg_val AND $7f) OR FLASH_VIDEO_MEMORY;
    PortWriteByte(control_reg, control_reg_val);

    // step 3. wait for status register bit 7 go down (1->0)
    status_reg_val:=PortReadByte(status_reg);
    i:=0;
    while ( (status_reg_val AND $80) and (i < 32767) ) <>0 do    //add time out
    begin
       status_reg_val:=myinportb(status_reg);
       i++;
    end;
    // step 4. clear control register bit 7 immediately */
    control_reg_val:=(control_reg_val AND $7f);
    PortWriteByte(control_reg, control_reg_val);

    // flash another 3 areas to clear all video memory */
    wait_vert_retrace_start();
    switch ( j ) {
      case 0 :
        set_vert_scroll_reg( (BYTE)(gConfig.wVertScanStart+(gwVideoHeight/4)) );
        break;
      case 1 :
        temp_ss:=( gConfig.wHorzScanStart+(gwVideoWidth/4) );
        // 11/26/93 Tang added */
        temp_ss %= $200;
        set_horz_shift_reg(temp_ss);
//        set_horz_shift_reg1(temp_ss);
        break;
      case 2 :
        set_vert_scroll_reg( (BYTE)gConfig.wVertScanStart );
        break;
//01/04/94      case 3 :
//01/04/94        set_horz_shift_reg( gConfig.wHorzScanStart );
//        set_horz_shift_reg1( gConfig.wHorzScanStart );
//01/04/94        break;
    }
  } // end of FOR loop */

  //01/04/94 Tang, Reset coordinate to original
  gConfig.wVertScanStart:=0;
  gConfig.wHorzScanStart:=0;
  // 3/9/94 Tang added
  gConfig.wVertScanStart += gwVertRegOffset;
  set_vert_scroll_reg( (BYTE)gConfig.wVertScanStart );
  set_horz_shift_reg (       gConfig.wHorzScanStart );
  gbPreMemSeg:=9;             // previous selected memory segment
  gbPreBnkNo :=9;             // previous selected bank no.
  //01/04/94 Tang, Reset coordinate to original

  result:=TRUE);
} // end Function JUN_enable_vert_sync_int()


Function AllocPageBuffer(void)
{
   DWORD	dwDataSize, dwActAllocSize;
   WORD 	padded;

   // Allocate 2 buffers used for WriteBitmap and ISR to scroll pages */
   padded:=gwVideoWidth % 2;
   // insure to make the width is a multiple of 2 */
   dwDataSize:=(DWORD)(gwVideoWidth+padded) * gwVideoHeight;

   //01/06/94 Tang added
   dwActAllocSize:=GlobalCompact(dwDataSize);
   //01/06/94 Tang added
   ghMem[0]:=GlobalAlloc(GHND, dwDataSize);
   if ( !ghMem[0] ) {
      PortWriteByte($80, $08);
#ifdef TEST_VER
      dwActAllocSize:=GlobalCompact(dwDataSize);
      wsprintf(debugstr, '1: Request=%ld, Actual=%ld', dwDataSize, dwActAllocSize);
      MessageBox(NULL, debugstr, 'GlobalAlloc Memory Error!', MB_OK OR MB_ICONEXCLAMATION);
#endif
      gwErrorCode:=ERR_MEMORY_ALLOCATE_ERROR;
      result:=gwErrorCode);
   }
   ghpBitsStart[0]:=glpPageBuf:=GlobalLock(ghMem[0]);

   //01/06/94 Tang added
   dwActAllocSize:=GlobalCompact(dwDataSize);
   //01/06/94 Tang added
   ghMem[1]:=GlobalAlloc(GHND, dwDataSize);
   if ( !ghMem[1] ) {
      PortWriteByte($80, $08);
#ifdef TEST_VER
      wsprintf(debugstr, '2: Request=%ld, Actual=%ld', dwDataSize, dwActAllocSize);
      MessageBox(NULL, debugstr, 'GlobalAlloc Memory Error!', MB_OK OR MB_ICONEXCLAMATION);
#endif
      GlobalUnlock( ghMem[0] );
      GlobalFree( ghMem[0] );
      gwErrorCode:=ERR_MEMORY_ALLOCATE_ERROR;
      result:=gwErrorCode);
   }
   ghpBitsStart[1]:=GlobalLock(ghMem[1]);

   InitialPageBuffer();

   result:=TRUE);
}

;****************************************************************************
; FreeSelectors
;
;   Routine to free all selectors allocated at initialization time.
;
; Exported:	No
; Entry:	None
; Destroys:	AX,BX,CX,FLAGS
;****************************************************************************
_FreeSelectors	PROC	far
	push	bp
	mov	bp, sp
	push	si
	push	di

	mov	bx, _gwFrameBufSel	;First selector.
	mov	ax, 0001		;Free selector.
	int	31H
;	xor	ax, ax
	mov     ax, 1
	jnc	freeOK
	mov	ax, -1

freeOK:
	pop	di
	pop	si
	mov	sp, bp
	pop	bp
	ret
_FreeSelectors	ENDP

// ---------------------------------------------------------------------------
// 2X2(WRITE)		       card ID register Functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// JUN_set_card_id() : set card ID value
// ***************************************************************************
Function  JUN_set_card_id(BYTE  id)
{
  card_id_reg_val:=id AND $0f;
  PortWriteByte(card_id_reg, card_id_reg_val);

  return(TRUE);
}

// ---------------------------------------------------------------------------
// 2X1(WRITE)		      corlor key register Functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// JUN_set_color_key() : set color key value
//   input
//	  color_key_ramdac_entry : RAMDAC entry of color key
// ***************************************************************************
Function PASCAL JUN_set_color_key(BYTE  color_key_ramdac_entry)
{
  color_key_reg_val:=color_key_ramdac_entry;
  PortWriteByte(color_key_reg, color_key_reg_val);
  // 10/27 Tang added
  gConfig.bColorKey:=color_key_reg_val;

  return(TRUE);
}

// ***************************************************************************
// JUN_set_display_mode() : set one of 2 display modes
//   input
//	  display_mode : EXTERNAL_VIDEO_ONLY  ($00 : 0000 0000)
//			 OVERLAY       ($20 : 0010 0000)
// ***************************************************************************
Function PASCAL JUN_set_display_mode(BYTE  display_mode)
{
  // step 1. control_reg_val AND $df(1101 1111):=$$ XXXX
  // step 2.	XX$ XXXX
  //	     OR 00A0 0000 (video_mode)
  //	     =	XXAX XXXX
  control_reg_val:=(control_reg_val AND $df) OR display_mode;
  PortWriteByte(control_reg, control_reg_val);
  // 10/27 Tang added
  gConfig.bDisplayMode:=display_mode;

  return(TRUE);
} // end Function JUN_set_display_mode()

// ***************************************************************************
// JUN_reset_whole_display_color() : not set the whole display to a certain color
// ***************************************************************************
Function PASCAL JUN_reset_whole_display_color(void)
{
  crt_sel_reg_val:=( crt_sel_reg_val AND $fc ) OR BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  // step     00$X$X
  //	   OR 00000100 ($04)
  //	  := 000$1XX
  crt_data_reg_val OR= RESET_WHOLE_DISPLAY_COLOR; // $04 : reset whole display to a color
  PortWriteByte(crt_data_reg, crt_data_reg_val);

  return(TRUE);
} // end Function JUN_reset_whole_display_color(void)


// ***************************************************************************
// JUN_set_scan_mode : set TV in underscan/overscan mode
// ***************************************************************************
Function PASCAL JUN_set_scan_mode(BYTE time_interval)
{
  if ( time_interval = UNDERSCAN ) {
     if ( JUN_get_TVsystem_type() = NTSC ) {
	select_horizontal_width640();
	select_vertical_height400();
	gwVideoWidth :=NTSC_XRES_UNDERSCAN;
	gwVideoHeight:=NTSC_YRES_UNDERSCAN;
     }
     else {    // PAL
	select_horizontal_width720();          //800 for PAL
	select_vertical_height480();           //480 for PAL
	gwVideoWidth :=PAL_XRES_UNDERSCAN;
	gwVideoHeight:=PAL_XRES_UNDERSCAN;
     }
  }
  else {  // Overscan
     if ( JUN_get_TVsystem_type() = NTSC ) {
	select_horizontal_width720();
	select_vertical_height480();
	gwVideoWidth :=NTSC_XRES_OVERSCAN;
	gwVideoHeight:=NTSC_YRES_OVERSCAN;
     }
     else {    // PAL
	select_horizontal_width756();          //924 for PAL
	select_vertical_height510();           //510 for PAL
	gwVideoWidth :=PAL_XRES_OVERSCAN;
	gwVideoHeight:=PAL_YRES_OVERSCAN;
     }
  }
  // 10/27 Tang added
  gConfig.bScanMode:=time_interval;
  return(TRUE);
} // end Function JUN_set_scan_mode


// ***************************************************************************
// JUN_clear_TVscreen() : clear the TV screen to black color
// ***************************************************************************
Function PASCAL JUN_clear_TVscreen(void)
{
  // set video mode to OVERLAY mode
  JUN_set_display_mode(OVERLAY);

  /*
  JUN_set_whole_display_color(palette_table[JUN_BLACK].peRed,
			      palette_table[JUN_BLACK].peGreen,
			      palette_table[JUN_BLACK].peBlue);
  */
  JUN_set_whole_display_color(JUN_BLACK);
  return(TRUE);
} // end Function JUN_clear_TVscreen()


// ***************************************************************************
// JUN_external_video_exist() : check if external video source exists
//   return
//	   TRUE  => exist
//	   FALSE => not exist
// ***************************************************************************
Function PASCAL JUN_external_video_exist(void)
{
  /* 10/28/93 Tang added to set OVERLAY mode */
  control_reg_val:=(control_reg_val OR $20);	//FORCEPC bit, OVERLAY-0010 0000
  PortWriteByte(control_reg, control_reg_val);
  /* 10/28/93 Tang added to set OVERLAY mode */
  status_reg_val:=myinportb(status_reg);
//  status_reg_val:=inportb(status_reg);
  if ( status_reg_val AND EXTERNAL_VIDEO_EXIST ) // 0000 0001:=01
    return(TRUE);
  else
    return(FALSE);
} // end Function external_video_exist(void)

// ***************************************************************************
// JUN_get_TVsystem_type() : get the TV system type of Junior
//   return
//	   NTSC => NTSC TV system  0000 0000:=00
//	   PAL	=> PAL TV system   0000 0010:=02
// ***************************************************************************
Function PASCAL JUN_get_TVsystem_type(void)
{
  status_reg_val:=inportb(status_reg);
  if ( status_reg_val AND PAL )  //PAL=0000 0010
    return(PAL);
  else
    return(NTSC);
}
  // 2X8(WRITE)		       CRT data register Functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// set_vert_scroll_reg() : set vertical scroll register (vertical scan start
//			   point)
//   input
//	  scan_start : vertical scan start point
// ***************************************************************************
Function set_vert_scroll_reg(BYTE  scan_start)
{
  /* first clear bit 0,1, i.e., 1111 1100:=$fc then
			     OR 0100 0000:=$00:=VERTICAL_SCROLL_REG_SEL  */
  crt_sel_reg_val:=( crt_sel_reg_val AND $fc ) OR VERT_SCROLL_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select vertical scroll register
  PortWriteByte(crt_data_reg, scan_start);
  return(TRUE);
}

// ***************************************************************************
// set_horz_shift_reg() : set horizontal shift register
//   input
//	  shift_pixel : #pixel of horizontal shift(9 bits)
// ***************************************************************************
Function set_horz_shift_reg(UINT  shift_pixel)
{
    BYTE  low_byte, msb_byte;

  /* 1. Set horz scan start's msb */
  msb_byte :=(BYTE) ((shift_pixel>>8) AND $01);
  set_horz_shift_reg_msb(msb_byte); // set MSB value into CRT select register
  /* first clear bit 0,1, i.e., 1111 1100:=$fc then
			     OR 0100 0011:=$03:=HORZ_SHIFT_REG_SEL  */
  crt_sel_reg_val:=( crt_sel_reg_val AND $fc ) OR HORZ_SHIFT_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  low_byte:=(BYTE) shift_pixel;
  PortWriteByte(crt_data_reg, low_byte);

  return(TRUE);
}

// ***************************************************************************
// JUN_init_ramdac_entry() : initialize RAMDAC entries values
//
// Note: each R.G.B color is 6-bit long, but the RGB color of input palette
//	 is rang from 0 to FF(8-bit), so we must round to 6-bit.
// ***************************************************************************
Function PASCAL JUN_init_ramdac_entry(PALETTEENTRY far *palette)
{
    int  i;

  set_ramdac_address_write_reg(0); // set the first RAMDAC entry to write
  for(i=0; i<256; i++)
    {
//    set_ramdac_address_write_reg(i);
      set_ramdac_data_reg((palette[i].peRed>>2),
			  (palette[i].peGreen>>2),
			  (palette[i].peBlue>>2) );
//	set_ramdac_data_reg(palette[i].rgbRed,
//			  palette[i].rgbGreen,
//			  palette[i].rgbBlue);
    } // for(i=0; i<256; i++)

  return(TRUE);
} // end Function JUN_init_ramdac_entry(void)


// ***************************************************************************
// select_bank_address() : select address of a bank
//   input
//	  bank_no : the desired bank no(from 0 to 7, 8*8K=64K)
//                                           0    3, 4*16K=64K
// ***************************************************************************
Function select_bank_address(BYTE type, BYTE bank_no)
{
  // step 1. crt_sel_reg_val AND $1f(0001 1111):=00$ XXXX
  // step 2.	00$ XXXX
  //	     OR ABC0 0000
  //	     =	ABCX XXXX
  switch ( type ) {
     case BANK_SIZE_64K :
          return(TRUE);
     case BANK_SIZE_16K :
          crt_sel_reg_val:=( crt_sel_reg_val AND $1f ) OR ( bank_no << 6 );
          break;
     case BANK_SIZE_8K :
          crt_sel_reg_val:=( crt_sel_reg_val AND $1f ) OR ( bank_no << 5 );
          break;
  }
  PortWriteByte(crt_sel_reg, crt_sel_reg_val);
  return(TRUE);
} // end Function select_bank_address(BYTE type, BYTE  bank_no)

// ***************************************************************************
// memory_seg_select() : select a memory segment(64K) from Video memory whose
//			 size is 8 segments(512K)
//    input
//	   seg_no : segment number ranging from
//			     MEMORY_SEGMENT0(0)
//				     to
//			     MEMORY_SEGMENT7(7)
// ***************************************************************************
Function memory_seg_select(BYTE	seg_no)
{
  // step 1. control_reg_val AND $f8(X111 1000):=XXXX X000
  // step 2.	XXXX X000
  //	     OR 0000 0ABC (seg_no)
  //	     =	XXXX XABC
  control_reg_val:=(control_reg_val AND $f8) OR seg_no;
  PortWriteByte(control_reg, control_reg_val);

  return(TRUE);
} // end Function memory_seg_select()


// ***************************************************************************
// select_bank_segment() : select 1(8k or 16k) of a 64K segment
//   input
//	  bank_no : the desired bank no(from 0 to 7 for 8k, 8*8K=64K)
//                                           0    3     16k,4*16K=64K
// ***************************************************************************
Function select_bank_segment(BYTE type, BYTE bank_no)
{
  // step 1. crt_sel_reg_val AND $e3(1110 0011):=XXX0 0$X
  // step 2.	XXX0 0$X
  //	     OR 000A BC00
  //	     =	XXXA BC00
  switch ( type ) {
     case BANK_SIZE_64K :
          return(TRUE);
     case BANK_SIZE_16K :
          crt_sel_reg_val:=( crt_sel_reg_val AND $e3 ) OR ( bank_no << 3 );
          break;
     case BANK_SIZE_8K :
          crt_sel_reg_val:=( crt_sel_reg_val AND $e3 ) OR ( bank_no << 2 );
          break;
  }
  PortWriteByte(crt_sel_reg, crt_sel_reg_val);
  return(TRUE);
} // end Function select_bank_segment(BYTE type, BYTE  bank_no)



int NEAR VerifyPortAndID(WORD wPort, BYTE bCardID)
{
   int	 i, wCurrStatus;

   /* I/O port must be 2x0, or we know it is bad. */
   if ( ! ( (wPort >= $200 and wPort <= $2ff) and !(wPort AND $000f) ) )
      return(ERR_IO_PORT_SETTING_BAD);

   /* Card ID must be from 0 to 14, or we know it is bad. */
   if ( ! ( bCardID <= 14 ) )
      return(ERR_CARD_ID_SETTING_BAD);

   /* Check card ID */
   JUN_set_card_id(bCardID);
   wCurrStatus:=JUN_get_junior_status();
   if ( (BYTE)(wCurrStatus) = $ff )
      return(ERR_JUNIOR_CARD_NOT_EXIST);

   /* Check I/O port */
   randomize();  //Initialize the random number generator
   for ( i=0; i<3; i++ )
      if ( ! VerifyRamdac() )
	 return(ERR_JUNIOR_CARD_NOT_EXIST);

   return(TRUE);
}

int NEAR VerifyFrameAddr(WORD wFrameAddr)
{
   /* Frame address must be $D000 or $E000, or we know it is bad. */
   if ( ((BYTE)(wFrameAddr>>12) >= $0B and (BYTE)(wFrameAddr>>12) <= $0E) ) {
      if ( wFrameAddr AND $0FFF )
	 return(ERR_FRAME_ADDR_SETTING_BAD);
   }
   else
      return(ERR_FRAME_ADDR_SETTING_BAD);
   return(TRUE);
}


int NEAR VerifyIRQ(BYTE bIRQ)
{
   time_t  first, second;
   UINT    count, int_count;
   BYTE    status_reg;

   /* Interrupt must be from 3 to 7, or we know it is bad. */
   if ( ! ( bIRQ >= 3 and bIRQ <= 7 ) )
      return(ERR_IRQ_SETTING_BAD);

   /* Set interrupt vector AND mask interrupt no */
   init_interrupt_test(bIRQ);

   /* Step 1. Test Vsync period by polling status reg. bit 4 */
   count:=0;
   first:=time(NULL);	/* Gets system time */
   do {
      /* polling Vsync to ask Vsync is generated */
      wait_vert_retrace_start();
      count++;
      second:=time(NULL); /* Gets system time again */
   /* calculates the elapsed time in seconds, return as a double */
   } while ( difftime(second, first) < 1 ); //wait for 1 second

   /* Step 2. Test Vsync interrupt with ISR(junaux.c) */
   gulEnterIntCount:=0;
   // enable interrupt
   JUN_enable_vert_sync_int(TRUE);

   first:=time(NULL);	/* Gets system time */
   do {
      second:=time(NULL); /* Gets system time again */
      /* calculates the elapsed time in seconds, return as a double */
   } while ( difftime(second, first) < 1 ); //wait for 1 second

   // disable interrupt
   disable();
   JUN_enable_vert_sync_int(FALSE);
   enable();

   /* Restore interrupt vector AND unmask interrupt no */
   restore_interrupt(bIRQ);

//   wsprintf(debugstr, 'IntCount=%d, VsyncCount=%d', gulEnterIntCount, count);
//   MessageBox(NULL, debugstr, 'Verify IRQ', MB_OK OR MB_ICONEXCLAMATION);

   /* Fv:=60 Hz, so 60 times at 1 second period, error allow to 5 */
   if ( JUN_get_TVsystem_type() = NTSC )
      int_count:=60;
   else
      int_count:=50;

   if ( (gulEnterIntCount>=(int_count+10)) OROR (gulEnterIntCount<=(int_count-10)) ) {
//	wsprintf(debugstr, 'Int count after 1 sec:=%d', gulEnterIntCount);
//	MessageBox(NULL, debugstr, 'Verify IRQ', MB_OK OR MB_ICONEXCLAMATION);
      PortWriteByte($80, gulEnterIntCount);
      return(ERR_IRQ_SETTING_BAD);
   }

   return(TRUE);
}

// *********221******************************************************************
// set_screen_border_reg() : set screnn border register
//   input
//	  color_entry : color_entry
// ***************************************************************************
int FAR  set_screen_border_reg(BYTE  color_entry)
{
  /* first clear bit 0,1, i.e., 1111 1100 = 0xfc then
			     OR 0100 0001 = 0x01 = SCREEN_BORDER_REG_SEL  */
  crt_sel_reg_val = (crt_sel_reg_val AND 0xfc) OR SCREEN_BORDER_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select vertical scroll register
  PortWriteByte(crt_data_reg, color_entry);
  return(TRUE);
} /* end of set_screen_border_reg(BYTE	color_entry) */

end.
 