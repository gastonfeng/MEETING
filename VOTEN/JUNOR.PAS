unit junor;

interface

uses port,global,windows,sysutils;


Function JUN_JUNIOR_EXIST:boolean;
//fprocedure InitialGlobals;
//fFunction JUN_LoadConfiguration:boolean;
//fFunction init_io_reg_val:integer;
//fFunction VerifyConfiguration:integer;
//fFunction SetConfiguration:Boolean;
//fFunction alloc_and_check_bank_access:integer;
//f Function AllocPageBuffer:boolean;
//f Function FreeSel:boolean;
//f Function JUN_set_card_id(id:byte):integer;
//f Function JUN_set_color_key(color_key_ramdac_entry:byte):integer;
//f Function JUN_set_display_mode(display_mode:byte):integer;
//f //f Function JUN_reset_whole_display_color:boolean;
//f Function JUN_set_scan_mode(time_interval:BYTE):boolean;
//f Function JUN_external_video_exist:Boolean;
//f Function JUN_get_TVsystem_type:byte;
//f Function set_vert_scroll_reg(scan_star:BYTE):integer;
//f Function FreeSelector(gwFrameBufSel:word):integer;
//f Function set_horz_shift_reg(shift_pixel:UINT):integer;
//f procedure set_ramdac_input_mask_reg(ramdac_input_mask_reg_val:byte);
//f Function JUN_GetSystemPalette(system_palette:PALETTEENTRY):boolean;
//f Function JUN_init_ramdac_entry(palette:PALETTEENTRY):integer;
//f Function AllocSel(wOffset:WORD;wSize:WORD):integer;
//f Function select_bank_address( bank_type:BYTE;  bank_no:BYTE):integer;
//f Function memory_seg_select(	seg_no:BYTE):integer;
//f Function select_bank_segment( bank_type:BYTE;  bank_no:BYTE):integer;
//f Function VerifyPortAndID( wPort:WORD;  bCardID:BYTE):integer;
//f Function VerifyFrameAddr( wFrameAddr:WORD):integer;
//f Function VerifyIRQ( bIRQ:BYTE):integer;
//f Function set_screen_border_reg(  color_entry:BYTE):boolean;

implementation

// ---------------------------------------------------------------------------
// 2X3(WRITE)		      CRT select register Functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// select_crt_register() : set one of 4 CRT registers
//   input
//	  crt_no : VERT_SCROLL_REG_SEL	   0x00  select vertical scroll register
//		   HORZ_SHIFT_REG_SEL	   0x03  select horizontal shift register
// ***************************************************************************
Function select_crt_register(crt_no:byte):boolean;
begin
  // step 1. crt_sel_reg_val and 0xfc(1111 1100) = XXXX XX00
  // step 2.	XXXX XX00
  //	     OR XXXX XXAB
  //	     =	XXXX XXAB
  crt_sel_reg_val := ( crt_sel_reg_val and $fc ) or crt_no;
  PortWriteByte(crt_sel_reg, crt_sel_reg_val);

  result:=TRUE;
end; // end Function select_crt_register(BYTE  crt_no)


// ***************************************************************************
// memory_read_enable() : enable Video memory to read
//   input
//	  flag : TRUE  => enable
//		 FALSE => disable
// ***************************************************************************
function mem_read_enable(flag:boolean):boolean;
begin
  crt_sel_reg_val := ( crt_sel_reg_val and $fC ) or BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  // step 1. control_reg_val and 0xf7(1111 0111) = XXXX 0XXX
  // step 2.	XXXX 0XXX
  //	     OR 0000 A000
  //	     =	XXXX AXXX
  if (flag = TRUE) then
    begin // enable		         0000 0000 = 00
    control_reg_val := (control_reg_val and $f7) or MEMORY_READ_ENABLE;
    //12/03/93 Tang                                0000 0000 = 00
    crt_data_reg_val := (crt_data_reg_val and $fe) or MEMORY_READ_ENABLE_BT;
  end
  else  // disable			         0000 1000 = 08
  begin
    control_reg_val := (control_reg_val and $f7) or MEMORY_READ_DISABLE;
    //12/03/93 Tang                                0000 0001 = 01
    crt_data_reg_val := (crt_data_reg_val and $fe) or MEMORY_READ_DISABLE_BT;
  end;
  PortWriteByte(control_reg, control_reg_val);
  //11/03/93 Tang
  PortWriteByte(crt_data_reg, crt_data_reg_val);

  result:=TRUE;
end; // end Function memory_read_enable()

Function JUN_JUNIOR_EXIST:boolean;
var OLD_value:byte;
begin
  OLD_value :=PortReadByte(status_reg);
  mem_read_enable(TRUE);
  status_reg_val := PortReadByte(status_reg);
  if ( (status_reg_val and READ_BACK1) = $00 ) then // 0000 1000 = 08
    begin
      // disable memory read and check if "READ BACK" bit is 1
      mem_read_enable(FALSE);
      status_reg_val := PortReadByte(status_reg);
      if ( status_reg_val and READ_BACK1)<>0 then // 0000 1000 = 08
       begin
	if ( (old_value and READ_BACK1) = $00 )then
	  mem_read_enable(TRUE);
	result:=TRUE;
      end
      else
	result:=FALSE;
    end
  else
    result:=FALSE;
end;


end.

