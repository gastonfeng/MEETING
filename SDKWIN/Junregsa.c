// **************************************************************************
// Project : junior.prj
// File    : junregsa.c (Version 2.0)
// Object  : handle JUNIOR hardware's registers
// Author  : Ricky Hsu
// Date    : 820810
// Updator : Kung-Puu Tang
// Date    : 821206
// **************************************************************************

#ifdef VGA_DISPLAY
  #include  <graphics.h>
#endif

#include  <windows.h>
#include  <stdio.h>
#include  <string.h>
#include  <dos.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <math.h>
#include  "juniora.h"
#include  "globala.h"

// ---------------------------------------------------------------------------
// 2X0(WRITE)		    control register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// memory_seg_select() : select a memory segment(64K) from Video memory whose
//			 size is 8 segments(512K)
//    input
//	   seg_no : segment number ranging from
//			     MEMORY_SEGMENT0(0)
//				     to
//			     MEMORY_SEGMENT7(7)
// ***************************************************************************
int FAR memory_seg_select(BYTE	seg_no)
{
  // step 1. control_reg_val & 0xf8(X111 1000) = XXXX X000
  // step 2.	XXXX X000
  //	     OR 0000 0ABC (seg_no)
  //	     =	XXXX XABC
  control_reg_val = (control_reg_val & 0xf8) | seg_no;
  outportb(control_reg, control_reg_val);

  return(TRUE);
} // end function memory_seg_select()

// ***************************************************************************
// memory_read_enable() : enable Video memory to read
//   input
//	  flag : TRUE  ==> enable
//		 FALSE ==> disable
// ***************************************************************************
int FAR memory_read_enable(BYTE  flag)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfC ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  // step 1. control_reg_val & 0xf7(1111 0111) = XXXX 0XXX
  // step 2.	XXXX 0XXX
  //	     OR 0000 A000
  //	     =	XXXX AXXX
  if (flag == TRUE)  {// enable		         0000 0000 = 00
    control_reg_val = (control_reg_val & 0xf7) | MEMORY_READ_ENABLE;
    //12/03/93 Tang                                0000 0000 = 00
    crt_data_reg_val = (crt_data_reg_val & 0xfe) | MEMORY_READ_ENABLE_BT;
  }
  else { // disable			         0000 1000 = 08
    control_reg_val = (control_reg_val & 0xf7) | MEMORY_READ_DISABLE;
    //12/03/93 Tang                                0000 0001 = 01
    crt_data_reg_val = (crt_data_reg_val & 0xfe) | MEMORY_READ_DISABLE_BT;
  }
  outportb(control_reg, control_reg_val);
  //11/03/93 Tang
  myoutportb(crt_data_reg, crt_data_reg_val);

  return(TRUE);
} // end function memory_read_enable()

// ***************************************************************************
// memory_write_enable() : enable Video memory to write
//   input
//	  flag : TRUE  ==> enable
//		 FALSE ==> disable
// ***************************************************************************
int FAR memory_write_enable(BYTE  flag)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfC ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  // step 1. control_reg_val & 0xef(1110 1111) = XXX0 XXXX
  // step 2.	XXX0 XXXX
  //	     OR 000A 0000
  //	     =	XXXA XXXX
  if (flag == TRUE) {// enable		         0000 0000 = 00
    control_reg_val = (control_reg_val & 0xef) | MEMORY_WRITE_ENABLE;
    //12/03/93 Tang                                0000 0000 = 00
    crt_data_reg_val = (crt_data_reg_val & 0xfe) | MEMORY_WRITE_ENABLE_BT;
  }
  else {// disable			         0001 0000 = 10
    control_reg_val = (control_reg_val & 0xef) | MEMORY_WRITE_DISABLE;
    //12/03/93 Tang                                0000 0010 = 02
    crt_data_reg_val = (crt_data_reg_val & 0xfe) | MEMORY_WRITE_DISABLE_BT;
  }
  outportb(control_reg, control_reg_val);
  //11/03/93 Tang
  myoutportb(crt_data_reg, crt_data_reg_val);

  return(TRUE);
} // end function memory_write_enable()

// ***************************************************************************
// JUN_set_display_mode() : set one of 2 display modes
//   input
//	  display_mode : EXTERNAL_VIDEO_ONLY  (0x00 : 0000 0000)
//			 OVERLAY       (0x20 : 0010 0000)
// ***************************************************************************
int FAR PASCAL JUN_set_display_mode(BYTE  display_mode)
{
  // step 1. control_reg_val & 0xdf(1101 1111) = 0X0X XXXX
  // step 2.	XX0X XXXX
  //	     OR 00A0 0000 (video_mode)
  //	     =	XXAX XXXX
  control_reg_val = (control_reg_val & 0xdf) | display_mode;
  outportb(control_reg, control_reg_val);
  // 10/27 Tang added
  gConfig.bDisplayMode = display_mode;

  return(TRUE);
} // end function JUN_set_display_mode()

// ***************************************************************************
// JUN_enable_vert_sync_int() : enable for vertical sync interrupt
//   input
//	  flag : TRUE  ==> enable
//		 FALSE ==> disable
// ***************************************************************************
int FAR PASCAL JUN_enable_vert_sync_int(BYTE  flag)
{
  // step 1. control_reg_val & 0xbf(1011 1111) = X0XX XXXX
  // step 2.	X0XX XXXX
  //	     OR 0A00 0000
  //	     =	XAXX XXXX
  if (flag == TRUE) // enable		    0100 0000 = 40
    control_reg_val = (control_reg_val & 0xbf) | VERT_SYNC_INT_ENABLE;
  else // disable			    0000 0000 = 00
    control_reg_val = (control_reg_val & 0xbf) | VERT_SYNC_INT_DISABLE;
  outportb(control_reg, control_reg_val);

  return(TRUE);
} // end function JUN_enable_vert_sync_int()


// ***************************************************************************
// JUN_flash_video_memory(BYTE	color_entry) : clear video memory
//   input
//	  flag : TRUE  ==> enable
//		 FALSE ==> disable
// ***************************************************************************
int FAR PASCAL JUN_flash_video_memory(BYTE  color_entry)
{
  int	i, j, temp_ss;

  // step 1. write color_entry to border register
  set_screen_border_reg(color_entry);
  for ( j=0; j<4; j++ ) {
    // step 2. set control register bit 7
    //	     control_reg_val & 0x7f(0111 1111) = 0XXX XXXX
    //		0XXX XXXX
    //	     OR A000 0000
    //	     =	AXXX XXXX
    control_reg_val = (control_reg_val & 0x7f) | FLASH_VIDEO_MEMORY;
    outportb(control_reg, control_reg_val);

    // step 3. wait for status register bit 7 go down (1->0)
    status_reg_val = myinportb(status_reg);
    i = 0;
    while ( (status_reg_val & 0x80) && (i < 32767) ) {    //add time out
       status_reg_val = myinportb(status_reg);
       i++;
    }
    /* step 4. clear control register bit 7 immediately */
    control_reg_val = (control_reg_val & 0x7f);
    outportb(control_reg, control_reg_val);

    /* flash another 3 areas to clear all video memory */
    wait_vert_retrace_start();
    switch ( j ) {
      case 0 :
        set_vert_scroll_reg( (BYTE)(gConfig.wVertScanStart+(gwVideoHeight/4)) );
        break;
      case 1 :
        temp_ss = ( gConfig.wHorzScanStart+(gwVideoWidth/4) );
        /* 11/26/93 Tang added */
        temp_ss %= 0x200;
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
  } /* end of FOR loop */

  //01/04/94 Tang, Reset coordinate to original
  gConfig.wVertScanStart = 0;
  gConfig.wHorzScanStart = 0;
  // 3/9/94 Tang added
  gConfig.wVertScanStart += gwVertRegOffset;
  set_vert_scroll_reg( (BYTE)gConfig.wVertScanStart );
  set_horz_shift_reg (       gConfig.wHorzScanStart );
  gbPreMemSeg = 9;             // previous selected memory segment
  gbPreBnkNo  = 9;             // previous selected bank no.
  //01/04/94 Tang, Reset coordinate to original

  return(TRUE);
} // end function JUN_enable_vert_sync_int()


// ---------------------------------------------------------------------------
// 2X0(READ)		    status register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// JUN_external_video_exist() : check if external video source exists
//   return
//	   TRUE  ==> exist
//	   FALSE ==> not exist
// ***************************************************************************
int FAR PASCAL JUN_external_video_exist(void)
{
  /* 10/28/93 Tang added to set OVERLAY mode */
  control_reg_val = (control_reg_val | 0x20);	//FORCEPC bit, OVERLAY-0010 0000
  outportb(control_reg, control_reg_val);
  /* 10/28/93 Tang added to set OVERLAY mode */
  status_reg_val = myinportb(status_reg);
//  status_reg_val = inportb(status_reg);
  if ( status_reg_val & EXTERNAL_VIDEO_EXIST ) // 0000 0001 = 01
    return(TRUE);
  else
    return(FALSE);
} // end function external_video_exist(void)

// ***************************************************************************
// JUN_get_TVsystem_type() : get the TV system type of Junior
//   return
//	   NTSC ==> NTSC TV system  0000 0000 = 00
//	   PAL	==> PAL TV system   0000 0010 = 02
// ***************************************************************************
int FAR PASCAL JUN_get_TVsystem_type(void)
{
  status_reg_val = inportb(status_reg);
  if ( status_reg_val & PAL )  //PAL=0000 0010
    return(PAL);
  else
    return(NTSC);
} // end function JUN_get_TVsystem_type(void)

// ***************************************************************************
// JUN_vert_sync_int_occur() : check if vertical sync interrupt flag is set
//   return
//	   TRUE  ==> interrupt occurs
//	   FALSE ==> no interrupt
// ***************************************************************************
int FAR PASCAL JUN_vert_sync_int_occur(void)
{
  /* 11/15/93 Tang added */
  disable();
  status_reg_val = inportb(status_reg);
  /* 11/15/93 Tang added */
  enable();
  if ( status_reg_val & VERT_SYNC_INT_OCCUR ) // 0000 0100 = 04
    return(TRUE);
  else
    return(FALSE);
} // end function JUN_vert_sync_int_occur(void)

// ***************************************************************************
// JUN_junior_exist() : check if Junior card exists
//   return
//	   TRUE  ==> Junior exists
//	   FALSE ==> not exist
// ***************************************************************************
int FAR PASCAL JUN_junior_exist(void)
{
  BYTE	old_status;

  old_status = inportb(status_reg);
  // enable memory read and check if "READ BACK" bit is 0
  memory_read_enable(TRUE);
  status_reg_val = inportb(status_reg);

//  if ( ~(status_reg_val & READ_BACK1) ) // 0000 1000 = 00
  if ( (status_reg_val & READ_BACK1) == 0x00 ) // 0000 1000 = 08
    {
      // disable memory read and check if "READ BACK" bit is 1
      memory_read_enable(FALSE);
	status_reg_val = inportb(status_reg);
      if ( status_reg_val & READ_BACK1) // 0000 1000 = 08
       {
	/* restore old status */
	if ( (old_status & READ_BACK1) == 0x00 )
	  memory_read_enable(TRUE);
	return(TRUE);
      }
      else
	return(FALSE);
    }
  else
    return(FALSE);
} // end function JUN_junior_exist(void)

// ***************************************************************************
// JUN_during_vert_sync_period() : check if Junior card is during vertical sync
//			       period
//   return
//	   TRUE  ==> Junior during vertical sync period
//	   FALSE ==> not
// ***************************************************************************
int FAR PASCAL JUN_during_vert_sync_period(void)
{
  status_reg_val = inportb(status_reg);
  if ( status_reg_val & VERT_SYNC_PERIOD ) // 0001 0000 = 10
    return(TRUE);
  else
    return(FALSE);
} // end function JUN_during_vert_sync_period(void)


// ***************************************************************************
// JUN_get_junior_status() : get the status of junior card
//
//   return
//	   high_byte ==> 0
//	   low_byte  ==> status value
// ***************************************************************************
int FAR PASCAL JUN_get_junior_status(void)
{
  status_reg_val = inportb(status_reg);
  return((int)status_reg_val & 0x00ff);
} // end function JUN_get_junior_status(void)


/************************************************************************
* wait_vert_retrace_start(void)
*
*   Wait until the start of video vertical retrace(blanking).
*
*		    1				     2
* Vsync : _____|~~~~|________________________________|~~~~|___________
*		 ^		   ^
*		high(180Usec)	  low
*						~~~~~|___________|~~~~
*						     ^ Blanking time(1200Usec)
* Process:	Step 1. wait until out of vertical sync.
*		Step 2. wait for start of vertical sync.
*
* Exported:	No
* Entry:	None
* Modifies:	AX,CX,DX
************************************************************************/
int FAR  wait_vert_retrace_start(void)
{
   long i;

   /* Step 1. wait until out of vertical sync. */
   for (i=0; i<0xFFFF; i++) {
      // 11/09/93 Tang updated
      status_reg_val = inportb(status_reg);
//	status_reg_val = myinportb(status_reg);
      /* status_reg_val   XXXX XXXX
	 VERT_SYNC_PERIOD 0001 0000 */
      if ( !(status_reg_val & VERT_SYNC_PERIOD) ) //if bit 4=0, drop out
//	if ( (status_reg_val & VERT_SYNC_PERIOD) == 0x00 ) //if 0, drop out
	 break;
   }
   /* Step 2. wait for start of vertical sync. */
   for (i=0; i<0xFFFF; i++) {
      // 11/09/93 Tang updated
      status_reg_val = inportb(status_reg);
//	status_reg_val = myinportb(status_reg);
      if ( (status_reg_val & VERT_SYNC_PERIOD) )  //if bit 4=1, drop out
	 break;
   }
   return(TRUE);
}// end function wait_vert_retrace_start(void)

// ---------------------------------------------------------------------------
// 2X1(WRITE)		      corlor key register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// JUN_set_color_key() : set color key value
//   input
//	  color_key_ramdac_entry : RAMDAC entry of color key
// ***************************************************************************
int FAR PASCAL JUN_set_color_key(BYTE  color_key_ramdac_entry)
{
  color_key_reg_val = color_key_ramdac_entry;
  outportb(color_key_reg, color_key_reg_val);
  // 10/27 Tang added
  gConfig.bColorKey = color_key_reg_val;

  return(TRUE);
} // end function JUN_set_color_key()


// ---------------------------------------------------------------------------
// 2X2(WRITE)		       card ID register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// JUN_set_card_id() : set card ID value
// ***************************************************************************
int FAR PASCAL JUN_set_card_id(BYTE  id)
{
  card_id_reg_val = id & 0x0f;
  outportb(card_id_reg, card_id_reg_val);

  return(TRUE);
} // end function JUN_set_card_id(BYTE	card_id)


// ***************************************************************************
// disable_card() : disable junior card
// ***************************************************************************
int FAR  disable_card(void)
{
  card_id_reg_val = 0x0f;
  outportb(card_id_reg, card_id_reg_val);  //write 15(0x0f) to card_id_register

  return(TRUE);
} // end function disable_card(void)


// ---------------------------------------------------------------------------
// 2X3(WRITE)		      CRT select register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// select_crt_register() : set one of 4 CRT registers
//   input
//	  crt_no : VERT_SCROLL_REG_SEL	   0x00  select vertical scroll register
//		   HORZ_SHIFT_REG_SEL	   0x03  select horizontal shift register
// ***************************************************************************
int FAR select_crt_register(BYTE  crt_no)
{
  // step 1. crt_sel_reg_val & 0xfc(1111 1100) = XXXX XX00
  // step 2.	XXXX XX00
  //	     OR XXXX XXAB
  //	     =	XXXX XXAB
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | crt_no;
  outportb(crt_sel_reg, crt_sel_reg_val);

  return(TRUE);
} // end function select_crt_register(BYTE  crt_no)

// ***************************************************************************
// select_bank_segment() : select 1(8k or 16k) of a 64K segment
//   input
//	  bank_no : the desired bank no(from 0 to 7 for 8k, 8*8K=64K)
//                                           0    3     16k,4*16K=64K
// ***************************************************************************
int FAR select_bank_segment(BYTE type, BYTE bank_no)
{
  // step 1. crt_sel_reg_val & 0xe3(1110 0011) = XXX0 00XX
  // step 2.	XXX0 00XX
  //	     OR 000A BC00
  //	     =	XXXA BC00
  switch ( type ) {
     case BANK_SIZE_64K :
          return(TRUE);
     case BANK_SIZE_16K :
          crt_sel_reg_val = ( crt_sel_reg_val & 0xe3 ) | ( bank_no << 3 );
          break;
     case BANK_SIZE_8K :
          crt_sel_reg_val = ( crt_sel_reg_val & 0xe3 ) | ( bank_no << 2 );
          break;
  }
  outportb(crt_sel_reg, crt_sel_reg_val);
  return(TRUE);
} // end function select_bank_segment(BYTE type, BYTE  bank_no)


// ***************************************************************************
// select_bank_address() : select address of a bank
//   input
//	  bank_no : the desired bank no(from 0 to 7, 8*8K=64K)
//                                           0    3, 4*16K=64K
// ***************************************************************************
int FAR select_bank_address(BYTE type, BYTE bank_no)
{
  // step 1. crt_sel_reg_val & 0x1f(0001 1111) = 000X XXXX
  // step 2.	000X XXXX
  //	     OR ABC0 0000
  //	     =	ABCX XXXX
  switch ( type ) {
     case BANK_SIZE_64K :
          return(TRUE);
     case BANK_SIZE_16K :
          crt_sel_reg_val = ( crt_sel_reg_val & 0x1f ) | ( bank_no << 6 );
          break;
     case BANK_SIZE_8K :
          crt_sel_reg_val = ( crt_sel_reg_val & 0x1f ) | ( bank_no << 5 );
          break;
  }
  outportb(crt_sel_reg, crt_sel_reg_val);
  return(TRUE);
} // end function select_bank_address(BYTE type, BYTE  bank_no)


// ***************************************************************************
// JUN_set_whole_display_color() : set the whole display to a certain color
//   input
//	  color_value : the desired color to be displayed
// ***************************************************************************
/*
int FAR PASCAL JUN_set_whole_display_color(BYTE red_color,
					   BYTE green_color,
					   BYTE blue_color)
*/
int FAR PASCAL JUN_set_whole_display_color(BYTE  color_entry)
{
  // step 1. set the color desired to be displayed : RAMDAC entry 0
//  set_ramdac_address_write_reg(0);
//  set_ramdac_data_reg(red_color, green_color, blue_color);
  set_screen_border_reg(color_entry);

  // step 2. crt_sel_reg_val & 0xfb(1111 1011) = XXXX X0XX , clear bit 2
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  crt_data_reg_val &= SET_WHOLE_DISPLAY_COLOR; // 0xFB : set whole display to a color
  outportb(crt_data_reg, crt_data_reg_val);

  return(TRUE);
} // end function JUN_set_whole_display_color(BYTE	color_value)

// ***************************************************************************
// JUN_reset_whole_display_color() : not set the whole display to a certain color
// ***************************************************************************
int FAR PASCAL JUN_reset_whole_display_color(void)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  // step     000XX0XX
  //	   OR 00000100 (0x04)
  //	   =  0000X1XX
  crt_data_reg_val |= RESET_WHOLE_DISPLAY_COLOR; // 0x04 : reset whole display to a color
  outportb(crt_data_reg, crt_data_reg_val);

  return(TRUE);
} // end function JUN_reset_whole_display_color(void)

// ***************************************************************************
// set_horz_shift_reg_msb() : set MSB of horizontal shift register
//   input
//	  lsb_value : value(0 or 1) of MSB of horizontal shift register
// ***************************************************************************
int FAR set_horz_shift_reg_msb(BYTE  flag)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  // step 1. crt_sel_reg_val & 0xf7(1111 0111) = XXXX 0XXX
  // step 2.	XXXX 0XXX
  //	     OR 0000 A000 (lsb_value)
  //	     =	XXXX AXXX
  crt_data_reg_val &= 0xf7;  // clear bit 3
  if ( flag == TRUE )  // set the bit to 1
    crt_data_reg_val |= SET_HORZ_SCAN_START_MSB; // 0x08 : set MSB of horizontal shift reg.

  outportb(crt_data_reg, crt_data_reg_val);

  return(TRUE);
} // end function set_horz_shift_reg_msb(BYTE  flag)

// ***************************************************************************
// select_horizontal_width512() : select width of horizontal display
// ***************************************************************************
int FAR select_horizontal_width512(void)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  /* first clear bit 4,5, i.e., 1100 1111 = 0xcf then
			     OR 0000 0000 = 0x00 = HORIZONTAL_WIDTH512  */
  crt_data_reg_val = ( crt_data_reg_val & 0xcf ) | HORIZONTAL_WIDTH512;
  outportb(crt_data_reg, crt_data_reg_val);
  return(TRUE);
} // end function select_horizontal_width512(void)

// ***************************************************************************
// select_horizontal_width640() : select width of horizontal display
// ***************************************************************************
int FAR select_horizontal_width640(void)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  /* first clear bit 4,5, i.e., 1100 1111 = 0xcf then
			     OR 0001 0000 = 0x10 = HORIZONTAL_WIDTH640  */
  crt_data_reg_val = ( crt_data_reg_val & 0xcf ) | HORIZONTAL_WIDTH640;
  outportb(crt_data_reg, crt_data_reg_val);
  return(TRUE);
} // end function select_horizontal_width640(void)

// ***************************************************************************
// select_horizontal_width720() : select width of horizontal display
// ***************************************************************************
int FAR select_horizontal_width720(void)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  /* first clear bit 4,5, i.e., 1100 1111 = 0xcf then
			     OR 0010 0000 = 0x20 = HORIZONTAL_WIDTH720  */
  crt_data_reg_val = ( crt_data_reg_val & 0xcf ) | HORIZONTAL_WIDTH720;
  outportb(crt_data_reg, crt_data_reg_val);
  return(TRUE);
} // end function select_horizontal_width720(void)

int FAR select_horizontal_width756(void)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  /* first clear bit 4,5, i.e., 1100 1111 = 0xcf then
			     OR 0011 0000 = 0x30 = HORIZONTAL_WIDTH756  */
  crt_data_reg_val = ( crt_data_reg_val & 0xcf ) | HORIZONTAL_WIDTH756;
  outportb(crt_data_reg, crt_data_reg_val);
  return(TRUE);
} // end function select_horizontal_width756(void)

// ***************************************************************************
// select_Vertical_Height400() : select width of horizontal display
// ***************************************************************************
int FAR select_vertical_height400(void)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  /* first clear bit 6,7, i.e., 0011 1111 = 0x3f then
			     OR 0000 0000 = 0x00 = VERTICAL_HEIGHT_400  */
  crt_data_reg_val = ( crt_data_reg_val & 0x3f ) | VERTICAL_HEIGHT400;
  outportb(crt_data_reg, crt_data_reg_val);
  return(TRUE);
} // end function select_vertical_height400(void)

// ***************************************************************************
// select_Vertical_Height480() : select width of horizontal display
// ***************************************************************************
int FAR select_vertical_height480(void)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  /* first clear bit 6,7, i.e., 0011 1111 = 0x3f then
			     OR 0100 0000 = 0x40 = VERTICAL_HEIGHT_400  */
  crt_data_reg_val = ( crt_data_reg_val & 0x3f ) | VERTICAL_HEIGHT480;
  outportb(crt_data_reg, crt_data_reg_val);
  return(TRUE);
} // end function select_vertical_height480(void)

// ***************************************************************************
// select_Vertical_Height510() : select width of horizontal display
// ***************************************************************************
int FAR select_vertical_height510(void)
{
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | BLANK_TIMING_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  /* first clear bit 6,7, i.e., 0011 1111 = 0x3f then
			     OR 1000 0000 = 0x80 = VERTICAL_HEIGHT_400  */
  crt_data_reg_val = ( crt_data_reg_val & 0x3f ) | VERTICAL_HEIGHT510;
  outportb(crt_data_reg, crt_data_reg_val);
  return(TRUE);
} // end function select_vertical_height510(void)


// ---------------------------------------------------------------------------
// 2X8(WRITE)		       CRT data register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// set_vert_scroll_reg() : set vertical scroll register (vertical scan start
//			   point)
//   input
//	  scan_start : vertical scan start point
// ***************************************************************************
int FAR set_vert_scroll_reg(BYTE  scan_start)
{
  /* first clear bit 0,1, i.e., 1111 1100 = 0xfc then
			     OR 0100 0000 = 0x00 = VERTICAL_SCROLL_REG_SEL  */
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | VERT_SCROLL_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select vertical scroll register
  outportb(crt_data_reg, scan_start);
  return(TRUE);
} // end function set_vert_scroll_reg(BYTE  scan_start)


// ***************************************************************************
// set_horz_shift_reg() : set horizontal shift register
//   input
//	  shift_pixel : #pixel of horizontal shift(9 bits)
// ***************************************************************************
int FAR set_horz_shift_reg(UINT  shift_pixel)
{
    BYTE  low_byte, msb_byte;

  /* 1. Set horz scan start's msb */
  msb_byte  = (BYTE) ((shift_pixel>>8) & 0x01);
  set_horz_shift_reg_msb(msb_byte); // set MSB value into CRT select register
  /* first clear bit 0,1, i.e., 1111 1100 = 0xfc then
			     OR 0100 0011 = 0x03 = HORZ_SHIFT_REG_SEL  */
  crt_sel_reg_val = ( crt_sel_reg_val & 0xfc ) | HORZ_SHIFT_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  low_byte = (BYTE) shift_pixel;
  outportb(crt_data_reg, low_byte);

  return(TRUE);
} // end function set_horz_shift_reg(UINT  shift_pixel)

// *********221******************************************************************
// set_screen_border_reg() : set screnn border register
//   input
//	  color_entry : color_entry
// ***************************************************************************
int FAR  set_screen_border_reg(BYTE  color_entry)
{
  /* first clear bit 0,1, i.e., 1111 1100 = 0xfc then
			     OR 0100 0001 = 0x01 = SCREEN_BORDER_REG_SEL  */
  crt_sel_reg_val = (crt_sel_reg_val & 0xfc) | SCREEN_BORDER_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select vertical scroll register
  outportb(crt_data_reg, color_entry);
  return(TRUE);
} /* end of set_screen_border_reg(BYTE	color_entry) */

// ---------------------------------------------------------------------------
// 2X4(WRITE)		  RAMDAC address write register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// set_ramdac_address_write_reg() : set RAMDAC address write register value
//   input
//	  ramdac_address : the address to be written
// ***************************************************************************
int FAR set_ramdac_address_write_reg(BYTE  ramdac_address)
{
  ramdac_addr_write_reg_val = ramdac_address;
  outportb(ramdac_addr_write_reg, ramdac_addr_write_reg_val);

  return(TRUE);
} // end function set_ramdac_address_write_reg(BYTE  ramdac_address)


// ---------------------------------------------------------------------------
// 2X7(WRITE)		  RAMDAC address read register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// set_ramdac_address_read_reg() : set RAMDAC address read register value
//   input
//	  ramdac_address : the address to be read
// ***************************************************************************
int FAR set_ramdac_address_read_reg(BYTE  ramdac_address)
{
  ramdac_addr_read_reg_val = ramdac_address;
  outportb(ramdac_addr_read_reg, ramdac_addr_read_reg_val);

  return(TRUE);
} // end function set_ramdac_address_read_reg(BYTE  ramdac_address)


// ---------------------------------------------------------------------------
// 2X5(READ / WRITE)	      RAMDAC data register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// set_ramdac_data_reg() : set RAMDAC data value(i.e. color) at the address
//			   of previous write register
//   input
//	  red_color   : the red component(6 bits) of this color(18 bits)
//	  green_color : the green component(6 bits) of this color(18 bits)
//	  blue_color  : the blue component(6 bits) of this color(18 bits)
// ***************************************************************************
int FAR set_ramdac_data_reg(BYTE  red_color, BYTE  green_color, BYTE  blue_color)
{
    ULONG  temp_color1=(ULONG) (red_color & 0x3f),
	   temp_color2=(ULONG) (green_color & 0x3f),
	   temp_color3=(ULONG) (blue_color & 0x3f);

  // 10/24 Tang masked ! use for what?
  ramdac_data_reg_val = (temp_color1 << 16) + (temp_color2 << 8) + temp_color3;
  myoutportb(ramdac_data_reg, red_color);
  myoutportb(ramdac_data_reg, green_color);
  myoutportb(ramdac_data_reg, blue_color);
//  outportb(ramdac_data_reg, red_color);
//  outportb(ramdac_data_reg, green_color);
//  outportb(ramdac_data_reg, blue_color);

  return(TRUE);
} // end function set_ramdac_data_reg(BYTE  ramdac_address)

// ***************************************************************************
// get_ramdac_data_reg() : get RAMDAC data value(i.e. color) at the address
//			   of previous read register
//   input
//	  red_color   : address of the red component(6 bits)
//	  green_color : address of the green component(6 bits)
//	  blue_color  : address of the blue component(6 bits)
// ***************************************************************************
int FAR get_ramdac_data_reg(BYTE far *red_color,
			    BYTE far *green_color,
			    BYTE far *blue_color)
{
    ULONG  temp_color1,
	   temp_color2,
	   temp_color3;
     
  *red_color   = myinportb(ramdac_data_reg);
  *green_color = myinportb(ramdac_data_reg);
  *blue_color  = myinportb(ramdac_data_reg);
//  *red_color	 = inportb(ramdac_data_reg);
//  *green_color = inportb(ramdac_data_reg);
//  *blue_color  = inportb(ramdac_data_reg);
  temp_color1  = (ULONG)(*red_color & 0x3f);
  temp_color2  = (ULONG)(*green_color & 0x3f);
  temp_color3  = (ULONG)(*blue_color & 0x3f);
  // 10/24 Tang masked ! use for what?
  ramdac_data_reg_val = (temp_color1 << 16) + (temp_color2 << 8) + temp_color3;

  return(TRUE);
} // end function get_ramdac_data_reg(BYTE  ramdac_address)


ULONG FAR get_ramdac_data_reg_Long(void)
{
  ULONG  temp_r, temp_g, temp_b;

  /* in ramdac 3 times and convert RGB value to a long intrger */
  temp_r = (ULONG)( ( (myinportb(ramdac_data_reg) & 0x3f) << 2) | 0x03);
  temp_g = (ULONG)( ( (myinportb(ramdac_data_reg) & 0x3f) << 2) | 0x03);
  temp_b = (ULONG)( ( (myinportb(ramdac_data_reg) & 0x3f) << 2) | 0x03);
  // 10/24 Tang masked ! use for what?
  ramdac_data_reg_val = (temp_r << 16) + (temp_g << 8) + temp_b;

  return((ULONG)(temp_b << 16) + (temp_g << 8) + temp_r);
} // end function get_ramdac_data_reg_Long(BYTE  ramdac_address)


// ---------------------------------------------------------------------------
// 2X6(WRITE)		   RAMDAC input mask register functions
// ---------------------------------------------------------------------------
// ***************************************************************************
// set_ramdac_input_mask_reg() : set RAMDAC input mask register value
//				 to prevent some entries to dsiplay
//   input
//	  mask_value : the mask value to be used
// ***************************************************************************
int FAR set_ramdac_input_mask_reg(BYTE	mask_value)
{
  ramdac_input_mask_reg_val = mask_value;
  outportb(ramdac_input_mask_reg, ramdac_input_mask_reg_val);

  return(TRUE);
} // end function set_ramdac_input_mask_reg(BYTE  mask_value)


// ---------------------------------------------------------------------------
//			   miscellaneous routines
// ---------------------------------------------------------------------------
// ***************************************************************************
// JUN_init_ramdac_entry() : initialize RAMDAC entries values
//
// Note: each R.G.B color is 6-bit long, but the RGB color of input palette
//	 is rang from 0 to FF(8-bit), so we must round to 6-bit.
// ***************************************************************************
int FAR PASCAL JUN_init_ramdac_entry(PALETTEENTRY far *palette)
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
} // end function JUN_init_ramdac_entry(void)

// ***************************************************************************
// JUN_set_scan_mode : set TV in underscan/overscan mode
// ***************************************************************************
int FAR PASCAL JUN_set_scan_mode(BYTE time_interval)
{
  if ( time_interval == UNDERSCAN ) {
     if ( JUN_get_TVsystem_type() == NTSC ) {
	select_horizontal_width640();
	select_vertical_height400();
	gwVideoWidth  = NTSC_XRES_UNDERSCAN;
	gwVideoHeight = NTSC_YRES_UNDERSCAN;
     }
     else {    // PAL
	select_horizontal_width720();          //800 for PAL
	select_vertical_height480();           //480 for PAL
	gwVideoWidth  = PAL_XRES_UNDERSCAN;
	gwVideoHeight = PAL_XRES_UNDERSCAN;
     }
  }
  else {  // Overscan
     if ( JUN_get_TVsystem_type() == NTSC ) {
	select_horizontal_width720();
	select_vertical_height480();
	gwVideoWidth  = NTSC_XRES_OVERSCAN;
	gwVideoHeight = NTSC_YRES_OVERSCAN;
     }
     else {    // PAL
	select_horizontal_width756();          //924 for PAL
	select_vertical_height510();           //510 for PAL
	gwVideoWidth  = PAL_XRES_OVERSCAN;
	gwVideoHeight = PAL_YRES_OVERSCAN;
     }
  }
  // 10/27 Tang added
  gConfig.bScanMode = time_interval;
  return(TRUE);
} // end function JUN_set_scan_mode


// ***************************************************************************
// JUN_clear_TVscreen() : clear the TV screen to black color
// ***************************************************************************
int FAR PASCAL JUN_clear_TVscreen(void)
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
} // end function JUN_clear_TVscreen()

// ***************************************************************************
// JUN_set_TVscreen_color() : set TV screen to a specific color
//    input
//	   color_entry : color's RAMDAC entry
// ***************************************************************************
int FAR PASCAL JUN_set_TVscreen_color(BYTE  color_entry)
{
  /*
  JUN_set_whole_display_color(palette_table[color_entry].peRed,
			      palette_table[color_entry].peGreen,
			      palette_table[color_entry].peBlue);
  */
  JUN_set_whole_display_color(color_entry);
  return(TRUE);
} // end function JUN_set_TVscreen_color


// ***************************************************************************
// JUN_draw_point() : draw a point in video memory
//   input
//	  x	      : X coordinate where  0 <= x <= 639
//	  y	      : Y coordinate where  0 <= y <= 479
//	  color_entry : RAMDAC entry
// ***************************************************************************

#ifdef VGA_DISPLAY
int  JUN_draw_point(UINT  x, UINT  y, BYTE  color_entry)
{
  if ( x >= 640 || y >= 480 )
    {
      printf("coordinate out of range : 0 <= x <= 639, 0 <= y <= 478\n");
      return(FALSE);
    }

  putpixel(x, y, color_entry);
  return(TRUE);
} // end function JUN_draw_point(UINT  x, UINT	y, BYTE  color_entry)

#else // #ifdef VGA_DISPLAY

int FAR PASCAL JUN_draw_point(UINT  x, UINT  y, BYTE  color_entry)
{
    static BYTE  prev_memory_segment=10, prev_bank_no=9;
    BYTE	 memory_segment, bank_no;
    UINT	 start_line, rel_x, rel_y;
//    BYTE far	 *lpVideoAddr;

// 11/22/93 Tang masked
//  if ( x >= VIDEO_MEMORY_WIDTH || y >= VIDEO_MEMORY_HEIGHT ) {
//     outportb(0x80, 8);
//     gwErrorCode = 8;
//     return(gwErrorCode);
//  }

   /* get relative x, y location */
   rel_x = fnRelX(x);
   rel_y = fnRelY(y);

  memory_segment = (BYTE)(rel_y >> 6); // y / 64;
  start_line	 = (UINT) (rel_y) % 64;

  // select the desired memory segment
  /*12/30
  if ( memory_segment != prev_memory_segment ) // not previous memory segment
    {
      memory_seg_select(memory_segment);
      prev_memory_segment = memory_segment; // preserve this value
    }
  12/30*/
  if ( memory_segment != gbPreMemSeg ) // not previous memory segment
    {
      memory_seg_select(memory_segment);
      gbPreMemSeg = memory_segment; // preserve this value
    }
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

  // set the desired color to this point
//  lpVideoAddr = (BYTE far *) GetVideoAddress(x, start_line);
//  *lpVideoAddr = color_entry;
//  glpVideoMemory[(start_line<<10) + x] = color_entry;
  glpVideoMemory[(start_line<<10) + rel_x] = color_entry;
  return(TRUE);
}

#endif // #ifdef VGA_DISPLAY


// ***************************************************************************
// JUN_draw_rect() : draw a rectangle in video memory
//   input
//	  x, y		: coordinate of left top coner point
//	  width, height : width and height
//	  color_entry	: RAMDAC entry
//	  fill_flag	: if fill the interior region (TRUE or FALSE)
// ***************************************************************************
int FAR PASCAL JUN_draw_rect(UINT  x,	  UINT	y,
			     UINT  width, UINT	height,
			     BYTE  color_entry,
			     UINT  fill_flag)
{
    UINT  i, j;

#ifdef VGA_DISPLAY
  if ( x + width - 1 >= 640 || y + height - 1 >= 480 )
    {
      printf("coordinate out of range : 0...639, 0...478\n");
      return(FALSE);
    }
#else
//  if ( x + width - 1 >= VIDEO_MEMORY_WIDTH ||
//       y + height - 1 >= VIDEO_MEMORY_HEIGHT ) {
  if ( width >= VIDEO_MEMORY_WIDTH ||
       height >= VIDEO_MEMORY_HEIGHT ) {
     outportb(0x80, 0x86);
     gwErrorCode = ERR_ACCESS_VRAM_OUT_OF_RANGE;
     return(gwErrorCode);
  }
#endif

  if ( fill_flag == TRUE ) // fill interior region
      for(i=0; i<height; i++)
	for(j=0; j<width; j++)
	  JUN_draw_point(x+j, y+i, color_entry);
  else // just draw the border
    {
      // draw top border
      for(i=0; i<width; i++)
	JUN_draw_point(x+i, y, color_entry);

      // draw bottom border
      for(i=0; i<width; i++)
	JUN_draw_point(x+i, y+height-1, color_entry);

      // draw left border
      for(i=0; i<height; i++)
	JUN_draw_point(x, y+i, color_entry);

      // draw right border
      for(i=0; i<height; i++)
	JUN_draw_point(x+width-1, y+i, color_entry);
    }

  return(TRUE);
} // end function JUN_draw_rect()


extern char  szFontFileSpec[256];  //at juniortt.c
// **************************************************************************
// load_font_file() : read a font file and create the character set table
//  return : TRUE  ==>	success
//	     FALSE ==>	error
// **************************************************************************
int FAR load_font_file(void)
{
    int   i, j;
    int   in_file;

#ifdef DOSLIB
    getcwd((LPSTR)szFontFileSpec, 144); /* Get current working directory */
    i = strlen(szFontFileSpec) - 1;
    if (szFontFileSpec[i] == '\\')
       szFontFileSpec[i] = 0;
    strcat((LPSTR)szFontFileSpec, FONT_FILE_NAMED);	 /* [ADDA] */
#else
    GetWindowsDirectory((LPSTR)szFontFileSpec, 144);
    wsprintf((LPSTR)szFontFileSpec, FONT_FILE_NAMEW, (LPSTR)szFontFileSpec);
#endif
    if ( (in_file = fileopen((LPSTR)szFontFileSpec, F_READ)) == -1 )
       return(-1);			/* File does not exist. */
    fileclose(in_file);


  for(i=0; i<CHAR_NUM; i++)
    for(j=0; j<CHAR_HEIGHT; j++)
      fileread(in_file, &char_set[i][j], sizeof(UINT));

  fileclose(in_file);
  return(TRUE);
} // end function load_font_file(void)


// ***************************************************************************
// JUN_putchar() : print a character on TV screen
//   input
//	  x, y	      : screen position
//			0 <= x <= 79, 0 <= y <= 23
//	  ch	      : character to be printed
//	  color_entry : RAMDAC entry
// ***************************************************************************
int FAR PASCAL JUN_putchar(UINT  x, UINT  y, BYTE  ch, BYTE  color_entry)
{
    int   i, j;
    BYTE  upper_ch;
    UINT  *chptr;

  if ( x >= 80 || y >= 24 )
      return(FALSE);


/***********
  x = x * CHAR_WIDTH;  // 16
  y = y * CHAR_HEIGHT; // 20
*************/
//  wsprintf(debugstr,"Current char: '%c'",ch);
//  MessageBox(NULL,debugstr,"Display char",MB_ICONSTOP | MB_OK);
    wsprintf(debugstr,"X:%4d,Y:%4d,CHAR:%c,COLOR:%4d",x,y,ch,color_entry);
    MessageBox(NULL,debugstr,"WRITE CHAR",MB_ICONSTOP |MB_OK);
  x  = x << 4;
  y *= 20;

  // get the character's bitmap starting address in the character set table
  // upper_ch = toupper(ch);
  upper_ch = ch;
  if ( upper_ch >= 'A' && upper_ch <= 'Z' )
    chptr = char_set+(upper_ch - 'A')*CHAR_HEIGHT;   //unit:UINT, no *2
//    chptr = &(char_set[upper_ch - 'A'][0]);
  else if ( upper_ch >= 'a' && upper_ch <= 'z' )
    chptr = char_set+(upper_ch - 'a')*CHAR_HEIGHT;
//    chptr = &(char_set[upper_ch - 'a' + CH_LA][0]);
  else if ( upper_ch >= '0' && upper_ch <= '9' )
    chptr = char_set+(upper_ch - '0' + CH_0)*CHAR_HEIGHT;
//    chptr = &(char_set[upper_ch - '0' + CH_0][0]);
  else if ( upper_ch >= ' ' && upper_ch <= '/' )
    chptr = char_set+(upper_ch - ' ' + CH_BLANK)*CHAR_HEIGHT;
//    chptr = &(char_set[upper_ch - ' ' + CH_BLANK][0]);
  else if ( upper_ch >= ':' && upper_ch <= '@' )
    chptr = char_set+(upper_ch - ':' + CH_COLON)*CHAR_HEIGHT;
//    chptr = &(char_set[upper_ch - ':' + CH_COLON][0]);
  else if ( upper_ch >= '[' && upper_ch <= '`' )
    chptr = char_set+(upper_ch - '[' + CH_BRACKET)*CHAR_HEIGHT;
//    chptr = &(char_set[upper_ch - '[' + CH_BRACKET][0]);
  else if ( upper_ch >= '{' && upper_ch <= '~' )
    chptr = char_set+(upper_ch - '{' + CH_LEFT_BRACE)*CHAR_HEIGHT;
//    chptr = &(char_set[upper_ch - '{' + CH_LEFT_BRACE][0]);
  else
    chptr = char_set+(CH_QMARK*CHAR_HEIGHT);
//    chptr = &(char_set[CH_QMARK][0]);

  // print each point of the derived bitmap
  for(i=0; i<CHAR_HEIGHT; i++)
      for(j=0; j<CHAR_WIDTH; j++)
	  if (/* chptr[i] & */mask[j] ) // this pixel is on
	    JUN_draw_point(x+j, y+i, color_entry);
  wsprintf(debugstr,"char_height:%4d,char_width:%4d",CHAR_HEIGHT,CHAR_WIDTH);
  MessageBox(NULL,debugstr,"Write char complete!",MB_ICONSTOP |MB_OK);

  return(TRUE);
} // end function JUN_putchar()

// ***************************************************************************
// JUN_putstring() : print a string on TV screen
//   input
//	  x, y	      : screen position
//			0 <= x <= 79, 0 <= y <= 23
//	  str	      : string to be printed
//	  color_entry : RAMDAC entry
// ***************************************************************************
int FAR PASCAL JUN_putstring(UINT  x, UINT  y, LPSTR  str, BYTE  color_entry)
{
    int  i, len;
//    char *sss="fengjt";

//  strcpy(str,"FENGJT");
//  str=sss;
  len = strlen(str);
  wsprintf(debugstr,"Str:'%s',length:%4d,X:%d,Y:%d",(LPSTR)str,len,x,y);
  MessageBox(NULL,debugstr,"9",MB_ICONSTOP | MB_OK);
  for(i=0; i<len; i++)
    {
      JUN_putchar(x, y, str[i], color_entry);
      if (++x >= LINE_MAX_CHAR) // exceed line boundary
	{
	  x = 0;
	  if (++y >= PAGE_MAX_LINE) // exceed page boundary
	    y = 0;
	}
    } // for(i=0; i<len; i++)


  return(TRUE);
} // end function JUN_putstring()


//---------------------------------------------------------------------------
// jun_draw_line : using Bresenham's line algorithm to draw a line segment
//	       from (x1, y1) to (x2, y2) where x1 < x2
//	       dx = x2 - x1, dy = y2 - y1 and p[1] = 2dy - dx
//	       if p[1] < 0 then next point is (x1 + 1, y1    )
//			   else next point is (x1 + 1, y1 + 1)
//	       for general i
//	       if p[i] < 0 then p[i+1] = p[i] + 2dy
//			   else p[i+1] = p[i] + 2(dy-dx)
//	       and  if p[i+1] < 0 then y[i+2] = y[i+1]
//				  else y[i+2] = y[i+1] + 1
//
//---------------------------------------------------------------------------
int FAR PASCAL JUN_draw_line(UINT  x1, UINT  y1,
		   UINT  x2, UINT  y2,
		   BYTE  color_entry)
{
    int     dx, dy, x, y, x_start, x_end, y_start, y_end,
	    p, c1, c2;
    double  m;

  // compute slope if necessary
  dx = x2 - x1;
  dy = y2 - y1;
  if ( dx == 0	&&  dy == 0 )
    {
      JUN_draw_point(x1, y1, color_entry);
      return(1);
    }

  if ( dx == 0 )  // a vertical line
    {
      if (y1 < y2)
	for(y=y1; y<=y2; y++)
	  JUN_draw_point(x1, y, color_entry);
      else  // y1 > y2
	for(y=y2; y<=y1; y++)
	  JUN_draw_point(x1, y, color_entry);

      return(1);
    }
  else if ( dy == 0 )  // a horizontal line
    {
      if (x1 < x2)
	for(x=x1; x<=x2; x++)
	  JUN_draw_point(x, y1, color_entry);
      else  // x1 > x2
	for(x=x2; x<=x1; x++)
	  JUN_draw_point(x, y1, color_entry);

      return(1);
    }
  else // neither vertical line nor horizontal line
    m = (double) dy / (double) dx;

  // determine if diagonal line
  if ( dx == dy ) // m=1
    {
      if (x1 < x2)
	for(x=x1, y=y1; x<=x2; x++, y++)
	  JUN_draw_point(x, y, color_entry);
      else  // x1 > x2
	for(x=x2,y=y2; x<=x1; x++,y++)
	  JUN_draw_point(x, y, color_entry);

      return(1);
    }
  else if ( dx + dy == 0 )  // m=-1
    {
      if (x1 < x2)
	for(x=x1, y=y1; x<=x2; x++, y--)
	  JUN_draw_point(x, y, color_entry);
      else  // x1 > x2
	for(x=x2,y=y2; x<=x1; x++,y--)
	  JUN_draw_point(x, y, color_entry);

      return(1);
    }

  //-------------------------------------------------------------------------
  //				  normal slant line
  //-------------------------------------------------------------------------
  JUN_draw_point(x1, y1, color_entry);	// draw start point
  JUN_draw_point(x2, y2, color_entry);	// draw end point
  if ( 0 < m && m < 1 )
    {
      dx = abs(dx);
      dy = abs(dy);
      if ( x1 < x2 )
	{
	  x_start = x1 + 1;
	  x_end   = x2 - 1;
	  y_start = y1;
	}
      else  // x1 > x2
	{
	  x_start = x2 + 1;
	  x_end   = x1 - 1;
	  y_start = y2;
	} // if ( x1 < x2 )

      c1 = dy << 1;	   // 2 * dy;
      c2 = (dy - dx) << 1; // 2 * (dy - dx);
      p  = c1 - dx;	   // 2 * dy - dx;
      for(x=x_start, y=y_start; x<=x_end; x++)
	{
	  if ( p < 0 )
	    {
	      p = p + c1;
	    }
	  else
	    {
	      y++;
	      p = p + c2;
	    }  // if ( p < 0 )
	  JUN_draw_point(x, y, color_entry);
	}  // for(x=x_start; x<=x_end; x++)
    }
  else if ( m > 1 )
    {
      dx = abs(dx);
      dy = abs(dy);
      if ( y1 < y2 )
	{
	  y_start = y1 + 1;
	  y_end   = y2 - 1;
	  x_start = x1;
	}
      else  // x1 > x2
	{
	  y_start = y2 + 1;
	  y_end   = y1 - 1;
	  x_start = x2;
	} // if ( x1 < x2 )

      c1 = dx << 1;	   // 2 * dx;
      c2 = (dx - dy) << 1; // 2 * (dx - dy);
      p  = c1 - dy;	   // 2 * dx - dy;
      for(y=y_start, x=x_start; y<=y_end; y++)
	{
	  if ( p < 0 )
	    {
	      p = p + c1;
	    }
	  else
	    {
	      x++;
	      p = p + c2;
	    }  // if ( p < 0 )
	  JUN_draw_point(x, y, color_entry);
	}  // for(x=x_start; x<=x_end; x++)
    }
  else if ( -1 < m && m < 0 )
    {
      dx = abs(dx);
//	dy = abs(dy);
      if ( x1 < x2 )
	{
	  x_start = x1 + 1;
	  x_end   = x2 - 1;
	  y_start = y1;
	}
      else  // x1 > x2
	{
	  dy = y1 - y2;
	  x_start = x2 + 1;
	  x_end   = x1 - 1;
	  y_start = y2;
	} // if ( x1 < x2 )

      c1 = (dx + dy) << 1; // 2 * (dx + dy);
      c2 = dy << 1;	   // 2 * dy;
      p  = c2 + dx;	   // 2 * dy + dx;
      for(x=x_start, y=y_start; x<=x_end; x++)
	{
	  if ( p < 0 )
	    {
	      y--;
	      p = p + c1;
	    }
	  else
	    {
	      p = p + c2;
	    }  // if ( p < 0 )
	  JUN_draw_point(x, y, color_entry);
	}  // for(x=x_start; x<=x_end; x++)
    }
  else	// m < -1
    {
//	dx = abs(dx);
      dy = abs(dy);
      if ( y1 < y2 )
	{
	  y_start = y1 + 1;
	  y_end   = y2 - 1;
	  x_start = x1;
	}
      else  // y1 > y2
	{
	  dx = x1 - x2;
	  y_start = y2 + 1;
	  y_end   = y1 - 1;
	  x_start = x2;
	} // if ( x1 < x2 )

      c1 = (dx + dy) << 1; // 2 * (dx + dy);
      c2 = dx << 1;	   // 2 * dx;
      p  = c2 + dy;	   // 2 * dx + dy;
      for(y=y_start, x=x_start; y<=y_end; y++)
	{
	  if ( p < 0 )
	    {
	      x--;
	      p = p + c1;
	    }
	  else
	    {
	      p = p + c2;
	    }  // if ( p < 0 )
	  JUN_draw_point(x, y, color_entry);
	}  // for(x=x_start; x<=x_end; x++)
    }
  return(1);
} // int  JUN_draw_line(int  x1, int y1, int x2, int y2, int color)


/*
// ***************************************************************************
// JUN_draw_circle() : draw a circle in video memory
//   input
//	  x, y		: center point of this circle
//	  radius	: radius
//	  color_entry	: RAMDAC entry
//	  fill_flag	: if fill the interior region (TRUE or FALSE)
// ***************************************************************************
int FAR PASCAL JUN_draw_circle(UINT  x,     UINT  y,
			       UINT  radius,
			       BYTE  color_entry,
			       UINT  fill_flag)
{
    int     offset_x, offset_y;
    double  angle, step, cycle;

#ifdef VGA_DISPLAY
  if ( x + radius - 1 >= 640 || y + radius - 1 >= 480 )
    {
      printf("coordinate out of range : 0...639, 0...478\n");
      return(FALSE);
    }
#else
  if ( x + radius - 1 >= 1024 || y + radius - 1 >= 512 )
    {
      printf("coordinate out of range : 0...1023, 0...511\n");
      return(FALSE);
    }
#endif

//  cycle = 0.25 * (M_PI+0.2);
//  step  = M_PI/180;
  cycle = 0.8;	// use PI = 3.2 and 1/4 circle = 3.2/4 = 0.8
  step	= 3.2/180;
  if ( fill_flag == TRUE ) // fill interior region
    {
      for(angle=0; angle<=cycle; angle+=step)
	{
	  offset_x = (int) ( radius * cos(angle) + 0.5 );
	  offset_y = (int) ( radius * sin(angle) + 0.5 );
	  JUN_draw_point(x+offset_x, y+offset_y, color_entry); // I
	  JUN_draw_point(x-offset_x, y+offset_y, color_entry); // II
	  JUN_draw_point(x-offset_x, y-offset_y, color_entry); // III
	  JUN_draw_point(x+offset_x, y-offset_y, color_entry); // IV

	  JUN_draw_point(x+offset_y, y+offset_x, color_entry); // I
	  JUN_draw_point(x-offset_y, y+offset_x, color_entry); // II
	  JUN_draw_point(x-offset_y, y-offset_x, color_entry); // III
	  JUN_draw_point(x+offset_y, y-offset_x, color_entry); // IV
	}
    }
  else // just draw the border
    {
      for(angle=0; angle<=cycle; angle+=step)
	{
	  offset_x = (int) ( radius * cos(angle) + 0.5 );
	  offset_y = (int) ( radius * sin(angle) + 0.5 );
	  JUN_draw_point(x+offset_x, y+offset_y, color_entry); // I
	  JUN_draw_point(x-offset_x, y+offset_y, color_entry); // II
	  JUN_draw_point(x-offset_x, y-offset_y, color_entry); // III
	  JUN_draw_point(x+offset_x, y-offset_y, color_entry); // IV

	  JUN_draw_point(x+offset_y, y+offset_x, color_entry); // I
	  JUN_draw_point(x-offset_y, y+offset_x, color_entry); // II
	  JUN_draw_point(x-offset_y, y-offset_x, color_entry); // III
	  JUN_draw_point(x+offset_y, y-offset_x, color_entry); // IV
	}
    }

  return(TRUE);
} // end function JUN_draw_circle()
*/

//---------------------------------------------------------------------------
// JUN_draw_circle : using Bresenham's algorithm to draw a circle
//		 with center (x_center, y_center) and radius
//		 we just draw 1/8 circle and the other 7/8 circle
//		 can be determined by symetric feature of circle :
//		 x axis, y axis, y = x and y = -x
//		 we start with (x1,y1) = (0, r) and p1 = 3 - 2r until x = y
//		 and for each i
//		 if p[i] < 0 then
//		    next point = (x[i]+1, y[i])
//		    p[i+1] = p[i] + 4x[i] + 6
//		 else
//		    next point = (x[i]+1, y[i]-1)
//		    p[i+1] = p[i] + 4(x[i]-y[i]) + 10
//---------------------------------------------------------------------------
int FAR PASCAL JUN_draw_circle(UINT  x_center, UINT  y_center,
			       UINT  radius,
			       BYTE  color,
			       UINT fill_flag)
{
    int  x, y, p;

  x = 0;
  y = radius;
  p = 3 - (radius << 1); // 3 - 2r
  for(; x<=y; x++)
    {
      // symetric to x axis and y axis
      JUN_draw_point(x_center + x, y_center + y, color);
      JUN_draw_point(x_center - x, y_center + y, color);
      JUN_draw_point(x_center - x, y_center - y, color);
      JUN_draw_point(x_center + x, y_center - y, color);

      // symetric to y = x and y = -x
      JUN_draw_point(x_center + y, y_center + x, color);
      JUN_draw_point(x_center - y, y_center + x, color);
      JUN_draw_point(x_center - y, y_center - x, color);
      JUN_draw_point(x_center + y, y_center - x, color);

      if ( p < 0 )
	{
	  p = p + (x << 2) + 6;
	}
      else
	{
	  p = p + ( (x - y) << 2 ) + 10;
	  y--;
	}  // if ( p < 0 )
    } //  for(; x<=y; x++)

  return(1);
} // JUN_draw_circle(int  x_center, int  y_center, int	radius)

/*
// ***************************************************************************
// JUN_draw_ellipse() : draw an ellipse in video memory
//   input
//	  x, y		: center point of this circle
//	  xradius	: radius in x direction
//	  yradius	: radius in y direction
//	  color_entry	: RAMDAC entry
//	  fill_flag	: if fill the interior region (TRUE or FALSE)
// ***************************************************************************
int FAR PASCAL JUN_draw_ellipse(UINT  x,       UINT  y,
				UINT  xradius, UINT  yradius,
				BYTE  color_entry,
				UINT  fill_flag)
{
    int     offset_x, offset_y, offset_x2, offset_y2;
    double  angle, step, cycle,
	    cos1, cos2, sin1, sin2;

#ifdef VGA_DISPLAY
  if ( x + xradius - 1 >= 640 || y + yradius - 1 >= 480 )
    {
      printf("coordinate out of range : 0...639, 0...478\n");
      return(FALSE);
    }
#else
//  if ( x + xradius - 1 >= 1024 || y + yradius - 1 >= 512 )
//    {
//	printf("coordinate out of range : 0...1023, 0...511\n");
//	return(FALSE);
//    }
#endif

//  cycle = 0.25 * (M_PI+0.2);
//  step  = M_PI/180;
  cycle = 0.8;	// use PI = 3.2 and 1/4 circle = 3.2/4 = 0.8
  step	= 3.2/180;
  if ( fill_flag == TRUE ) // fill interior region
    {
      for(angle=0; angle<=cycle; angle+=step)
	{
	  cos1 = cos(angle); sin1 = sin(angle);
	  cos2 = sin1;	     sin2 = cos1;

	  offset_x = (int) ( xradius * cos1 + 0.5 );
	  offset_y = (int) ( yradius * sin1 + 0.5 );
	  offset_x2 = (int) ( xradius * cos2 + 0.5 );
	  offset_y2 = (int) ( yradius * sin2 + 0.5 );

	  JUN_draw_point(x+offset_x, y+offset_y, color_entry); // I
	  JUN_draw_point(x-offset_x, y+offset_y, color_entry); // II
	  JUN_draw_point(x-offset_x, y-offset_y, color_entry); // III
	  JUN_draw_point(x+offset_x, y-offset_y, color_entry); // IV

	  JUN_draw_point(x+offset_x2, y+offset_y2, color_entry); // I
	  JUN_draw_point(x-offset_x2, y+offset_y2, color_entry); // II
	  JUN_draw_point(x-offset_x2, y-offset_y2, color_entry); // III
	  JUN_draw_point(x+offset_x2, y-offset_y2, color_entry); // IV

	}
    }
  else // just draw the border
    {
      for(angle=0; angle<=cycle; angle+=step)
	{
	  cos1 = cos(angle); sin1 = sin(angle);
	  cos2 = sin1;	     sin2 = cos1;

	  offset_x = (int) ( xradius * cos1 + 0.5 );
	  offset_y = (int) ( yradius * sin1 + 0.5 );
	  offset_x2 = (int) ( xradius * cos2 + 0.5 );
	  offset_y2 = (int) ( yradius * sin2 + 0.5 );

	  JUN_draw_point(x+offset_x, y+offset_y, color_entry); // I
	  JUN_draw_point(x-offset_x, y+offset_y, color_entry); // II
	  JUN_draw_point(x-offset_x, y-offset_y, color_entry); // III
	  JUN_draw_point(x+offset_x, y-offset_y, color_entry); // IV

	  JUN_draw_point(x+offset_x2, y+offset_y2, color_entry); // I
	  JUN_draw_point(x-offset_x2, y+offset_y2, color_entry); // II
	  JUN_draw_point(x-offset_x2, y-offset_y2, color_entry); // III
	  JUN_draw_point(x+offset_x2, y-offset_y2, color_entry); // IV
	}
    }

  return(TRUE);
} // end function JUN_draw_ellipse()
*/


int FAR PASCAL jun_draw_ellipse(UINT  xc, UINT	yc,
				UINT  r1, UINT	r2,
				BYTE  color,
				UINT  fill_flag)
{
    int     x, y, tx, ty;
    long    p, c1, c2, c21, c3;
    double  c11, c22;

  if ( r1 == r2 )
    {
      JUN_draw_circle(xc, yc, r1, color, TRUE);
      return(1);
    }

  if ( r1 > r2 )
    {
      x   = 0;
      y   = r2;
      c1  = ((long) r2 * (long) r2) << 1;  // 2*r2^2
      c2  = (long) r1 * (long) r1;	   // r1^2
      c21 = c2 << 2;			   // 4*r1^2
      c3  = (long) ( 1 - ((long)r2<<1) );	 // 1 - 2*r2
      p   = c1 + c2 * c3;		   // 2*r2^2 + r1^2(1-2r2)
      c11 = (double) r1 / (double) r2;
      c22 = (double) r2 / (double) r1;
      for(; ; x++)
	{
	  // symmetric to x axis and y axis
	  JUN_draw_point(xc + x, yc + y, color);
	  JUN_draw_point(xc - x, yc + y, color);
	  JUN_draw_point(xc - x, yc - y, color);
	  JUN_draw_point(xc + x, yc - y, color);

	  // symetric to y = (r2/r1) x and y = -(r2/r1)x
	  tx = (int) ( c11 * (double) y );
	  ty = (int) ( c22 * (double) x );
	  JUN_draw_point(xc + tx, yc + ty, color);
	  JUN_draw_point(xc - tx, yc + ty, color);
	  JUN_draw_point(xc - tx, yc - ty, color);
	  JUN_draw_point(xc + tx, yc - ty, color);

	  if ( p < 0 )
	    {
	      p = p + c1 * (long)( (x<<1) + 3 ); // p[i+1] = p[i] + 2*r2^2(2x[i]+3)
	    }
	  else
	    {
	      p = p + c1 * (long) ( (x<<1) + 3 ) - c21 * (y - 1); // p[i+1] = p[i] + 2k(2x[i]+3) - 4(y[i] - 1)
	      y--;
	    }  // if ( p < 0 )
	  if ( x * r2 >=  y * r1)
	    return(1);
	} //  for(; x<=y; x++)
    }
  else // r1 < r2
    {
      x   = r1;
      y   = 0;
      c1  = ((long) r1 * (long) r1) << 1;  // 2*r1^2
      c2  = (long) r2 * (long) r2;	   // r2^2
      c21 = c2 << 2;			   // 4*r2^2
      c3  = (long) ( 1 - ((long)r1<<1) );	 // 1 - 2*r1
      c11 = (double) r1 / (double) r2;
      c22 = (double) r2 / (double) r1;
      p   = c1 + c2 * c3;    // 2*r1^2 + r2^2(1-2r1)
      for(; ; y++)
	{
	  // symmetric to x axis and y axis
	  JUN_draw_point(xc + x, yc + y, color);
	  JUN_draw_point(xc - x, yc + y, color);
	  JUN_draw_point(xc - x, yc - y, color);
	  JUN_draw_point(xc + x, yc - y, color);

	  // symetric to y = (r2/r1) x and y = -(r2/r1)x
	  tx = (int) ( c11 * (double) y );
	  ty = (int) ( c22 * (double) x );
	  JUN_draw_point(xc + tx, yc + ty, color);
	  JUN_draw_point(xc - tx, yc + ty, color);
	  JUN_draw_point(xc - tx, yc - ty, color);
	  JUN_draw_point(xc + tx, yc - ty, color);

	  if ( p < 0 )
	    {
	      p = p + c1 * (long)( (y<<1) + 3 );  // p[i+1] = p[i] + 2*r1^2(2y[i]+3)
	    }
	  else
	    {
	      p = p + c1 * (long)( (y<<1) + 3 ) - c21 * (x - 1);  // p[i+1] = p[i] + 2*r1^2(2y[i]+3) - 4*r2^2(y[i] - 1)
	      x--;
	    }  // if ( p < 0 )
	  if ( y * r1 >= x * r2 )
	    return(1);
	} //  for(; x<=y; x++)
    }  //  if ( r1 > r2 )

//  return(1);
} // int FAR PASCAL JUN_draw_ellipse()



// ***************************************************************************
// JUN_draw_circle2() : draw a circle in video memory
//   input
//	  x, y		: center point of this circle
//	  radius	: radius
//	  color_entry	: RAMDAC entry
//	  fill_flag	: if fill the interior region (TRUE or FALSE)
// ***************************************************************************
int FAR PASCAL JUN_draw_circle2(UINT  x,     UINT  y,
			       UINT  radius,
			       BYTE  color_entry,
			       UINT  fill_flag)
{
    int     offset_x, offset_y;
    double  angle, step, cycle;

#ifdef VGA_DISPLAY
  if ( x + radius - 1 >= 640 || y + radius - 1 >= 480 )
    {
      printf("coordinate out of range : 0...639, 0...478\n");
      return(FALSE);
    }
#else
  if ( x + radius - 1 >= 1024 || y + radius - 1 >= 512 )
    {
      printf("coordinate out of range : 0...1023, 0...511\n");
      return(FALSE);
    }
#endif

  cycle = M_PI + M_PI;
  step	= M_PI/180;
  if ( fill_flag == TRUE ) // fill interior region
    {
      for(angle=0; angle<=cycle; angle+=step)
	{
	  offset_x = (int) ( radius * cos(angle) + 0.5 );
	  offset_y = (int) ( radius * sin(angle) + 0.5 );
	  JUN_draw_point(x+offset_x, y+offset_y, color_entry);
	}
    }
  else // just draw the border
    {
      for(angle=0; angle<=cycle; angle+=step)
	{
	  offset_x = (int) ( radius * cos(angle) + 0.5 );
	  offset_y = (int) ( radius * sin(angle) + 0.5 );
	  JUN_draw_point(x+offset_x, y+offset_y, color_entry);
	}
    }

  return(TRUE);
} // end function JUN_draw_circle2()


// ***************************************************************************
// JUN_draw_color_bar() : fill video memory with color bars
//   input
//	  pitch     : width of one bar
//	  color_no  : available color number
//	  color_bar : color entries of RAMDAC
// ***************************************************************************
int FAR PASCAL JUN_draw_color_bar(int  pitch, int  color_no, BYTE  color_entry[])
{
   int	bar_no, last_pitch, i;

#ifdef VGA_DISPLAY
  bar_no     = 640 / pitch;
  last_pitch = 640 % pitch;
#else
  bar_no     = 1024 / pitch;
  last_pitch = 1024 % pitch;
#endif

  for(i=0; i<bar_no; i++)
#ifdef VGA_DISPLAY
    JUN_draw_rect(i * pitch, 0, pitch, 480, color_entry[i%color_no], TRUE);
#else
    JUN_draw_rect(i * pitch, 0, pitch, 512, color_entry[i%color_no], TRUE);
#endif

  if (last_pitch != 0)
    JUN_draw_rect(bar_no * pitch, 0, last_pitch, 480, color_entry[bar_no%color_no], TRUE);

  return(TRUE);
} // end function JUN_draw_color_bar()


// ***************************************************************************
// JUN_draw_slant_bar() : fill video memory with color slant bars
//   input
//	  pitch     : width of one bar
//	  color_no  : available color number
//	  color_bar : color entries of RAMDAC
// ***************************************************************************
int FAR PASCAL JUN_draw_slant_bar(int pitch, int color_no, BYTE color_entry[], int dirt)
{
   int	i, j, count, bar_no;
   int	xMax, yMax;

xMax = 1024;
yMax = 512;

   count = 0;
   bar_no = 0;

  if ( dirt == 0 )  {	 //Left top first--------------------------------

    for ( i=0; i<yMax; i++,count++ ) {
       if ( count >= pitch )  { bar_no ++; count=0; }
       JUN_draw_line( i, 0, 0, i, color_entry[bar_no%color_no]);
    }
    for ( i=yMax, j=1; i<xMax; i++,j++,count++	) {
       if ( count >= pitch )  { bar_no ++; count=0; }
       JUN_draw_line( i, 0, j, yMax-1, color_entry[bar_no%color_no]);
    }
    for ( i=(xMax-yMax), j=1; j<yMax; i++,j++,count++  ) {
       if ( count >= pitch )  { bar_no ++; count=0; }
       JUN_draw_line( xMax-1, j, i, yMax-1, color_entry[bar_no%color_no]);
    }

  }
  else {   //Right top first --------------------------------------------

    for ( i=xMax-1,j=0;  j<yMax;  i--,j++,count++ ) {
       if ( count >= pitch )  { bar_no ++; count=0; }
       JUN_draw_line( i, 0, xMax-1, j, color_entry[bar_no%color_no]);
    }
    for ( i=(xMax-yMax)-1,j=xMax-1;  i>=0;  i--,j--,count++  ) {
       if ( count >= pitch )  { bar_no ++; count=0; }
       JUN_draw_line( i, 0, j, yMax-1, color_entry[bar_no%color_no]);
    }
    for ( j=0, i=(xMax-yMax)-1; i>=0; i--,j++,count++ ) {
       if ( count >= pitch )  { bar_no ++; count=0; }
       JUN_draw_line( 0, j, i, yMax-1, color_entry[bar_no%color_no]);
    }
  }

  return(TRUE);
} // end function JUN_draw_color_bar()


void NEAR  myoutportb(unsigned portid, unsigned char value)
{
   outportb(portid, value);
}

unsigned char NEAR  myinportb(unsigned portid)
{
   return( inportb(portid) );
}


/* 11/26/93 Tang added to modify horizonatl shift register */
int FAR  set_horz_shift_reg1(UINT  shift_pixel)
{
    BYTE  msb_bit, high_8bits;

  /* first clear bit 0,1, i.e., 1111 1100 = 0xFC then
			     OR 0100 0011 = 0x03 = HORZ_SHIFT_REG_SEL  */
  msb_bit = (BYTE) (shift_pixel & 0x01);
  set_horz_shift_reg_msb(msb_bit); // set LSB value into CRT select register
  crt_sel_reg_val = ( crt_sel_reg_val & 0xFC ) | HORZ_SHIFT_REG_SEL;
  select_crt_register(crt_sel_reg_val); // select horizontal shift register
  high_8bits  = (BYTE) ((shift_pixel>>1) & 0xff);
  myoutportb(crt_data_reg, high_8bits);
//  outportb(crt_data_reg, crt_data_reg_val);

  return(TRUE);
} // end function set_horz_shift_reg(UINT  shift_pixel)
/* 11/26/93 Tang added to modify horizonatl shift register */



int FAR select_bank(BYTE type, UINT yPos)
{
   static BYTE  prev_memory_segment=10, prev_bank_no=9;
   BYTE	        memory_segment, bank_no;
   UINT         y, start_line;

   y = yPos % 512;

   memory_segment = y >> 6; // y / 64;
   start_line	 = (UINT) (y % 64);

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
   if ( type != BANK_SIZE_64K ) {
      bank_no = (start_line >> ( 5 - type)); //16K(1:4),8K(2:3)
      /*12/30
      if ( bank_no != prev_bank_no ) {
         select_bank_segment(type, bank_no);
         prev_bank_no = bank_no;
      }
      12/30*/
      if ( bank_no != gbPreBnkNo ) {
         select_bank_segment(type, bank_no);
         gbPreBnkNo = bank_no;
      }
   }
   return(TRUE);
}


/* 12/20/93 Tang added */
// ***************************************************************************
// JUN_get_ramdac_entry_6b() : get RAMDAC entries values
//
// Note: each R.G.B color return is 6-bit long, but the RGB color of input palette
//	 is rang from 0 to FF(8-bit), so we must round to 6-bit.
// ***************************************************************************
int FAR PASCAL JUN_get_ramdac_entry_6b(PALETTEENTRY far *palette)
{
   int  i;
   ULONG lRGBcolor;

   set_ramdac_address_read_reg(0); // set the first RAMDAC entry to write
   for (i=0; i<256; i++) {
      lRGBcolor = get_ramdac_data_reg_Long();
      palette[i].peRed   = ( (BYTE)( lRGBcolor      & 0x000000ff) ) >> 2;
      palette[i].peGreen = ( (BYTE)((lRGBcolor>>8)  & 0x000000ff) ) >> 2;
      palette[i].peBlue  = ( (BYTE)((lRGBcolor>>16) & 0x000000ff) ) >> 2;
      palette[i].peFlags  = 0;
   }
   return(TRUE);
} // end function JUN_get_ramdac_entry_6b(void)

// ***************************************************************************
// JUN_init_ramdac_entry_6b() : initialize RAMDAC entries values
//
// Note: each R.G.B color is 6-bit long, but the RGB color of input palette
//	 is rang from 0 to FF(8-bit), so we must round to 6-bit.
// ***************************************************************************
int FAR PASCAL JUN_init_ramdac_entry_6b(PALETTEENTRY far *palette)
{
   int  i;

   set_ramdac_address_write_reg(0); // set the first RAMDAC entry to write
   for(i=0; i<256; i++) {
      set_ramdac_data_reg( palette[i].peRed,
			   palette[i].peGreen,
			   palette[i].peBlue );
   }
   return(TRUE);
} // end function JUN_init_ramdac_entry_6b(void)
/* 12/20/93 Tang added */

// end of junregsa.c //