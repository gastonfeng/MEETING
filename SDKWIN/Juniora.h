

// ****************************************************************************
// Project : junior.prj
// File    : juniora.h	(Version 2.0)
// Object  : header file of constant definition and function prototypes
// Author  : Ricky Hsu
// Date    : 820810
// Updator : Kung-Puu Tang
// Date    : 821206
// ****************************************************************************
//#define  VGA_DISPLAY

/* type definition for program used */
typedef unsigned char		BYTE;
typedef unsigned int		UINT;
typedef unsigned long		ULONG;
typedef int			BOOL;

#define  TRUE			1
#define  FALSE			0
#define  OPT_NUM		12    // # of system options

#define VIDEO_MEMORY_WIDTH	1024
#define VIDEO_MEMORY_HEIGHT	512
#define MAXCOLORENTRY		256
#define VGA_PAGE_WIDTH		640
#define VGA_PAGE_HEIGHT 	480
#define VGA_SHOW_HEIGHT 	400

#define  MEMORY_SIZE_16K	0x4000
#define  MEMORY_SIZE_8K 	0x2000

#define  BANK_SIZE_64K		0
#define  BANK_SIZE_16K		1
#define  BANK_SIZE_8K		2


#define LINES_PER_SEGMENT	64

#define NTSC_XRES_OVERSCAN	720
#define NTSC_YRES_OVERSCAN	480
#define NTSC_XRES_UNDERSCAN	640
#define NTSC_YRES_UNDERSCAN	400

#define PAL_XRES_OVERSCAN	924
#define PAL_YRES_OVERSCAN	510
#define PAL_XRES_UNDERSCAN	800
#define PAL_YRES_UNDERSCAN	480

/* define CONST for Scrolling & Shifting */
#define SCROLL_UP		0
#define SCROLL_DOWN		1
#define SCROLL_LEFT		2
#define SCROLL_RIGHT		3
#define SCROLL_NONE		4

#define HORZ_SCAN_START_LSB	0x0100	 // the bit 8(9th) of horizontal start

#define  RED_PART		0    // red color index of palette table
#define  GREEN_PART		1    // green color index of palette table
#define  BLUE_PART		2    // blue color index of palette table

/* define CONST for scan mode */
#define  UNDERSCAN		0
#define  OVERSCAN		1

/* define CONST for HandleImageRect */
#define  H_READ 		0
#define  H_WRITE		1
#define  H_CLEAR		2

/* define CONST for BitBlt's nRop */
#define  JUN_BLACKNESS		0
#define  JUN_WHITENESS		1
#define  JUN_DSTINVERT		2
#define  JUN_MERGECOPY		3
#define  JUN_SRCAND		4
#define  JUN_SRCCOPY		5
#define  JUN_SRCINVERT		6

/* define CONST for error code */
#define ERR_INITIALIZED_BEFORE		-1000
#define ERR_JUNIOR_INI_NOT_FOUND	-1001
#define ERR_JUNIOR_CARD_NOT_EXIST	-1002
#define ERR_CANNOT_ALLOCATE_SELECTOR	-1003
#define ERR_VRAM_ACCESS_ERROR		-1004
#define ERR_WRITE_JUNIOR_INI_ERROR	-1005
#define ERR_ACCESS_VRAM_OUT_OF_RANGE	-1006
#define ERR_INSUFFICIENT_MEMORY 	-1007
#define ERR_MEMORY_ALLOCATE_ERROR	-1008
#define ERR_CREATE_BITMAP_ERROR 	-1009
#define ERR_SELECT_OBJECT_ERROR 	-1010
#define ERR_GET_OBJECT_ERROR		-1011
#define ERR_SCROLL_TYPE_NOT_SUPPORT	-1012
#define ERR_SCROLL_SPEED_NOT_ALLOW	-1013
#define ERR_IO_PORT_SETTING_BAD 	-1014
#define ERR_CARD_ID_SETTING_BAD 	-1015
#define ERR_FRAME_ADDR_SETTING_BAD	-1016
#define ERR_IRQ_SETTING_BAD		-1017
#define ERR_BANK_ACCESS_CHECK_ERROR	-1018
#define ERR_RAMDAC_ACCESS_ERROR 	-1019
#define ERR_VGA_MODE_VIOLATION		-1020
#define ERR_SPECIAL_EFFECT_NOT_SUPPORT  -1021

/* define CONST for the type of show */
#define SEF_WRITE_BITMAP_DOWN           0
#define SEF_WRITE_BITMAP_UP             1
#define SEF_WRITE_BITMAP_RIGHT          2
#define SEF_WRITE_BITMAP_LEFT           3
#define SEF_VERT_CROSS			4
#define SEF_HORZ_CROSS			5
#define SEF_JALOUSIE			6
#define SEF_STREAM_WATER		7
#define SEF_VERT_CRUSH			8
#define SEF_VERT_REVEAL 		9
#define SEF_HORZ_CRUSH			10
#define SEF_HORZ_REVEAL 		11
#define SEF_PULL_UP		        12
#define SEF_PULL_DOWN			13
#define SEF_PULL_RIGHT			14
#define SEF_PULL_LEFT			15
#define SEF_SHRINK			16
#define SEF_EXPLOAD			17
#define SEF_SCAN			18
#define SEF_SPIRAL			19


#define CFG_FILE_NAME		"JUNIOR.INI"
#define CFG_FILE_NAMED		"\\junior.ini"          //DOS
#define CFG_FILE_NAMEW		"%s\\junior.ini"        //Windows
#define FONT_FILE_NAME		"JUNFNT.FNT"
#define FONT_FILE_NAMED 	"\\junfnt.fnt"          //DOS
#define FONT_FILE_NAMEW 	"%s\\junfnt.fnt"        //Windows

#define min(a,b)	 (((a) < (b)) ? (a) : (b))
#define max(a,b)	 (((a) > (b)) ? (a) : (b))

// Following rules are defined for DOS using --------------------------------
#ifdef DOSLIB

  typedef unsigned int		WORD;
  typedef unsigned long 	DWORD;
  typedef char far		*LPSTR;
  typedef unsigned int		HANDLE;
  typedef HANDLE		HWND;
  typedef struct tagRGBQUAD {
	BYTE	rgbBlue;
	BYTE	rgbGreen;
	BYTE	rgbRed;
	BYTE	rgbReserved;
  } RGBQUAD;

  #define PASCAL		pascal
  #define FAR			far
  #define NEAR			near
  #define ADDRESS		WORD

  #define MAKELONG(a, b)	(((LONG)(((DWORD)((WORD)(a))) << 16)|(WORD)(b)))
  #define LOWORD(l)		((WORD)(l))
  #define HIWORD(l)		((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
  #define LOBYTE(w)		((BYTE)(w))
  #define HIBYTE(w)		((BYTE)(((WORD)(w) >> 8) & 0xFF))

  /* Flags specifying the access mode for files. */
  #define P_MODE		S_IREAD | S_IWRITE
  #define F_MODE		O_RDWR | O_CREAT | O_BINARY
  #define F_READ		O_RDONLY | O_BINARY
  #define F_WRITE		O_WRONLY | O_BINARY

  #define filecreat(a, b)	open(a, b, P_MODE)
  #define fileread(a, b, c)	read(a, b, c)
  #define filewrite(a, b, c)	write(a, b, c)
  #define fileopen(a, b)	open(a, b)
  #define fileclose(a)		close(a)
  #define fileseek(a, b, c)	lseek(a, b, c)
  #define stringf		sprintf
  #define stringcmp		stricmp
  #define stringcpy		strcpy
  #define stringlen		strlen

// Following rules are defined for WINDOWS using ----------------------------
#else		/* Windows */

  #define ADDRESS		DWORD
  typedef LPSTR 		FPSTR;

  /* Global memory flags */
  #define GMEM_FIXED		0x0000
  #define GMEM_MOVEABLE 	0x0002
  #define GMEM_NOCOMPACT	0x0010
  #define GMEM_NODISCARD	0x0020
  #define GMEM_ZEROINIT 	0x0040
  #define GMEM_MODIFY		0x0080
  #define GMEM_DISCARDABLE	0x0100
  #define GMEM_NOT_BANKED	0x1000
  #define GMEM_SHARE		0x2000
  #define GMEM_DDESHARE 	0x2000
  #define GMEM_NOTIFY		0x4000
  #define GMEM_LOWER		GMEM_NOT_BANKED

  /* OpenFile() Flags */
  #define OF_READ		0x0000
  #define OF_WRITE		0x0001
  #define OF_READWRITE		0x0002
  #define OF_SHARE_COMPAT	0x0000
  #define OF_SHARE_EXCLUSIVE	0x0010
  #define OF_SHARE_DENY_WRITE	0x0020
  #define OF_SHARE_DENY_READ	0x0030
  #define OF_SHARE_DENY_NONE	0x0040
  #define OF_PARSE		0x0100
  #define OF_DELETE		0x0200
  #define OF_VERIFY		0x0400
  #define OF_CANCEL		0x0800
  #define OF_CREATE		0x1000
  #define OF_PROMPT		0x2000
  #define OF_EXIST		0x4000
  #define OF_REOPEN		0x8000

  /* Flags specifying the access mode for files. */
  #define F_MODE		0
  #define F_READ		OF_READ
  #define F_WRITE		OF_WRITE

  #define filecreat(a, b)	_lcreat(a, b)
  #define fileread(a, b, c)	_lread(a, b, c)
  #define filewrite(a, b, c)	_lwrite(a, b, c)
  #define fileopen(a, b)	_lopen(a, b)
  #define fileclose(a)		_lclose(a)
  #define fileseek(a, b, c)	_llseek(a, b, c)
  #define stringf		wsprintf
  #define stringcmp		lstrcmpi
  #define stringcpy		lstrcpy
  #define stringlen		lstrlen
#endif		 /* Windows ---------------------------------------*/

typedef struct CFGSTRUCT {
	WORD   wFrameAddr;
	WORD   wPortBaseAddr;
	BYTE   bCardID;
	BYTE   bIRQUsed;
	BYTE   bColorKey;
	BYTE   bDisplayMode;
	BYTE   bScanMode;
	WORD   wVertScanStart;
	WORD   wHorzScanStart;
} CFGSTRUCT;

#define PALVERSION		0x300  //windows logical palette version no.

// ---------------------------------------------------------------------------
//			      Index of alphabet table
// ---------------------------------------------------------------------------
#define  CHAR_NUM	  95
#define  CHAR_WIDTH	  16
#define  CHAR_HEIGHT	  20
#define  LINE_MAX_CHAR	  (640 / CHAR_WIDTH)
#define  PAGE_MAX_LINE	  (480 / CHAR_HEIGHT)
#define  CH_A		  0
#define  CH_B		  1
#define  CH_C		  2
#define  CH_D		  3
#define  CH_E		  4
#define  CH_F		  5
#define  CH_G		  6
#define  CH_H		  7
#define  CH_I		  8
#define  CH_J		  9
#define  CH_K		 10
#define  CH_L		 11
#define  CH_M		 12
#define  CH_N		 13
#define  CH_O		 14
#define  CH_P		 15
#define  CH_Q		 16
#define  CH_R		 17
#define  CH_S		 18
#define  CH_T		 19
#define  CH_U		 20
#define  CH_V		 21
#define  CH_W		 22
#define  CH_X		 23
#define  CH_Y		 24
#define  CH_Z		 25

#define  CH_LA		( 0+26 )
#define  CH_LB		( 1+26 )
#define  CH_LC		( 2+26 )
#define  CH_LD		( 3+26 )
#define  CH_LE		( 4+26 )
#define  CH_LF		( 5+26 )
#define  CH_LG		( 6+26 )
#define  CH_LH		( 7+26 )
#define  CH_LI		( 8+26 )
#define  CH_LJ		( 9+26 )
#define  CH_LK		(10+26 )
#define  CH_LL		(11+26 )
#define  CH_LM		(12+26 )
#define  CH_LN		(13+26 )
#define  CH_LO		(14+26 )
#define  CH_LP		(15+26 )
#define  CH_LQ		(16+26 )
#define  CH_LR		(17+26 )
#define  CH_LS		(18+26 )
#define  CH_LT		(19+26 )
#define  CH_LU		(20+26 )
#define  CH_LV		(21+26 )
#define  CH_LW		(22+26 )
#define  CH_LX		(23+26 )
#define  CH_LY		(24+26 )
#define  CH_LZ		(25+26 )

#define  CH_BLANK	(26+26 )
#define  CH_SLASH	(41+26 )

#define  CH_0		(42+26 )
#define  CH_1		(43+26 )
#define  CH_2		(44+26 )
#define  CH_3		(45+26 )
#define  CH_4		(46+26 )
#define  CH_5		(47+26 )
#define  CH_6		(48+26 )
#define  CH_7		(49+26 )
#define  CH_8		(50+26 )
#define  CH_9		(51+26 )

#define  CH_COLON	(52+26 ) // :
#define  CH_QMARK	(57+26 ) // ?
#define  CH_SMALL_MOUSE (58+26 ) // @
#define  CH_BRACKET	(59+26 ) // [
#define  CH_UP_COMMA	(64+26 ) // {
#define  CH_LEFT_BRACE	(65+26 ) // {
#define  CH_FLY 	(68+26 ) // ~

// ---------------------------------------------------------------------------
// 2X0(WRITE)		      control register
// ---------------------------------------------------------------------------
#define  MEMORY_SEGMENT0	      0  // Video memory segment 0
#define  MEMORY_SEGMENT1	      1  // Video memory segment 1
#define  MEMORY_SEGMENT2	      2  // Video memory segment 2
#define  MEMORY_SEGMENT3	      3  // Video memory segment 3
#define  MEMORY_SEGMENT4	      4  // Video memory segment 4
#define  MEMORY_SEGMENT5	      5  // Video memory segment 5
#define  MEMORY_SEGMENT6	      6  // Video memory segment 6
#define  MEMORY_SEGMENT7	      7  // Video memory segment 7
#define  MEMORY_READ_ENABLE	   0x00  // Video memory read enable
#define  MEMORY_READ_DISABLE	   0x08  // Video memory read disable
#define  MEMORY_WRITE_ENABLE	   0x00  // Video memory write enable
#define  MEMORY_WRITE_DISABLE	   0x10  // Video memory write disable
#define  EXTERNAL_VIDEO_ONLY	   0x00  // Video mode : external vide only
#define  OVERLAY		   0x20  // Video mode : overlay
#define  VERT_SYNC_INT_ENABLE	   0x40  // vertical sync interrupt enable
#define  VERT_SYNC_INT_DISABLE	   0x00  // vertical sync interrupt disable
#define  FLASH_VIDEO_MEMORY	   0x80  // Flash video memory by Hardware

// ---------------------------------------------------------------------------
// 2X0(READ)		       status register
// ---------------------------------------------------------------------------
#define  EXTERNAL_VIDEO_EXIST	   0x01  // external video source exist
#define  NTSC			   0x00  // TV system type of Junior
#define  PAL			   0x02  // TV system type of Junior
#define  VERT_SYNC_INT_OCCUR	   0x04  // vertical sync interrupt occurs
#define  READ_BACK0		   0x00  // read back bit = 0
#define  READ_BACK1		   0x08  // read back bit = 1
#define  VERT_SYNC_PERIOD	   0x10  // raster scan is during vertical sync period

// ---------------------------------------------------------------------------
// 2X1(WRITE)		     corlor key register
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 2X2(WRITE)		      card ID register
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 2X3(WRITE)		      CRT select register
// ---------------------------------------------------------------------------
#define  VERT_SCROLL_REG_SEL	   0x00  // select vertical scroll register
#define  SCREEN_BORDER_REG_SEL	   0x01  // select screen border register
#define  BLANK_TIMING_REG_SEL	   0x02  // select blank timeing register
#define  HORZ_SHIFT_REG_SEL	   0x03  // select horizontal shift register

// ---------------------------------------------------------------------------
// Blank timing register(CRT_select_reg=02)
// ---------------------------------------------------------------------------
#define  MEMORY_READ_ENABLE_BT	   0x00  // Video memory read enable
#define  MEMORY_READ_DISABLE_BT    0x01  // Video memory read disable
#define  MEMORY_WRITE_ENABLE_BT    0x00  // Video memory write enable
#define  MEMORY_WRITE_DISABLE_BT   0x02  // Video memory write disable
#define  SET_WHOLE_DISPLAY_COLOR   0xFB  // set whole display to a color
#define  RESET_WHOLE_DISPLAY_COLOR 0x04  // reset whole display to a color]
#define  SET_HORZ_SCAN_START_MSB   0x08  // set MSB of horizontal shift reg.
#define  HORIZONTAL_WIDTH512	   0x00  // select horizontal width 512 pixel
#define  HORIZONTAL_WIDTH640	   0x10  // select horizontal width 640 pixels
#define  HORIZONTAL_WIDTH720	   0x20  // select horizontal width 720(800 PAL) pixels
#define  HORIZONTAL_WIDTH756	   0x30  // select horizontal width 756(924 PAL) pixels
#define  VERTICAL_HEIGHT400	   0x00  // select vertical height 400 pixels
#define  VERTICAL_HEIGHT480	   0x40  // select vertical height 400 pixels
#define  VERTICAL_HEIGHT510	   0x80  // select vertical height 400 pixels

// ---------------------------------------------------------------------------
// 2X8(WRITE)		     CRT data register
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 2X4(WRITE)		RAMDAC address write register
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 2X7(WRITE)		 RAMDAC address read register
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 2X5(READ / WRITE)	     RAMDAC data register
// ---------------------------------------------------------------------------
// ***************************************************************************
//	       Define some usually used color's RAMDAC entries
// ***************************************************************************
#ifndef VGA_DISPLAY
  #define  JUN_BLACK			   16
  #define  JUN_WHITE			   31
  #define  JUN_BLUE			   32
  #define  JUN_MAGENTA			   36 // default RAMDAC entry of color key
  #define  JUN_RED			   40
  #define  JUN_YELLOW			   44
  #define  JUN_GREEN			   48
  #define  JUN_CYAN			   52
  #define  JUN_BROWN			   05
  #define  JUN_UNKNOWN			   0x6c
  #define  DEFAULT_COLOR		   JUN_WHITE
#else
  #define  JUN_BLUE			   LIGHTBLUE
  #define  JUN_MAGENTA			   LIGHTMAGENTA
  #define  JUN_RED			   LIGHTRED
  #define  JUN_GREEN			   LIGHTGREEN
  #define  JUN_CYAN			   LIGHTCYAN
  #define  JUN_BROWN			   LIGHTBROWN
  #define  DEFAULT_COLOR	       WHITE
#endif

// ---------------------------------------------------------------------------
// 2X6(WRITE)		   RAMDAC input mask register
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//		      function prototype declaractions
// ---------------------------------------------------------------------------
// ***************************************************************************
//    junregsa.c
// ***************************************************************************
// ===========================================================================
// 2X0(WRITE)		      control register functions
// ===========================================================================
int FAR 	memory_seg_select(BYTE seg_no);
int FAR 	memory_read_enable(BYTE flag);
int FAR 	memory_write_enable(BYTE flag);

int FAR PASCAL	JUN_set_display_mode(BYTE display_mode),
		JUN_enable_vert_sync_int(BYTE flag),
		JUN_flash_video_memory(BYTE color_entry);

// ===========================================================================
// 2X0(READ)		      status register functions
// ===========================================================================
int FAR PASCAL	JUN_external_video_exist(void),
		JUN_get_TVsystem_type(void),
		JUN_vert_sync_int_occur(void),
		JUN_junior_exist(void),
		JUN_during_vert_sync_period(void),
		// 10/27 Tang added
		JUN_get_junior_status(void);
int FAR 	wait_vert_retrace_start(void);

// ===========================================================================
// 2X1(WRITE)		      corlor key register functions
// ===========================================================================
int FAR PASCAL	JUN_set_color_key(BYTE color_key_ramdac_entry);

// ===========================================================================
// 2X2(WRITE)		       card ID register functions
// ===========================================================================
int FAR PASCAL	JUN_set_card_id(BYTE card_id);
int FAR 	disable_card(void);

// ===========================================================================
// 2X3(WRITE)		      CRT select register functions
// ===========================================================================
int FAR 	select_crt_register(BYTE crt_no);
int FAR 	select_bank_segment(BYTE type, BYTE bank_no);
int FAR 	select_bank_address(BYTE type, BYTE bank_no);

// ===========================================================================
// 2X8(WRITE)		       CRT data register functions
// ===========================================================================
int FAR 	set_vert_scroll_reg(BYTE scan_start);
int FAR 	set_horz_shift_reg(UINT shift_pixel);
		// 11/05/93 Tang added
int FAR 	set_screen_border_reg(BYTE color_entry);

/* 12/06/93 Tang added */
// BLANK_TIMING_REGISTER(0x02 selected by crt_sel_register)
//int FAR PASCAL JUN_set_whole_display_color(BYTE red_color, BYTE  green_color, BYTE  blue_color);
int FAR PASCAL	JUN_set_whole_display_color(BYTE color_entry),
		JUN_reset_whole_display_color(void);
int FAR 	set_horz_shift_reg_lsb(BYTE lsb_value);
int FAR 	select_horizontal_width512(void);
int FAR 	select_horizontal_width640(void);
int FAR 	select_horizontal_width720(void);
int FAR 	select_horizontal_width756(void);
int FAR 	select_vertical_height400(void);
int FAR 	select_vertical_height480(void);
int FAR 	select_vertical_height510(void);

// ===========================================================================
// 2X4(WRITE)		  RAMDAC address write register functions
// ===========================================================================
int FAR 	set_ramdac_address_write_reg(BYTE ramdac_address);

// ===========================================================================
// 2X7(WRITE)		  RAMDAC address read register functions
// ===========================================================================
int FAR 	set_ramdac_address_read_reg(BYTE ramdac_address);

// ===========================================================================
// 2X5(READ / WRITE)	      RAMDAC data register functions
// ===========================================================================
int FAR 	set_ramdac_data_reg(BYTE red_color, BYTE green_color, BYTE blue_color);
int FAR 	get_ramdac_data_reg(BYTE far *red_color,
				    BYTE far *green_color,
				    BYTE far *blue_color);
// 11/19/93 Tang  add
ULONG FAR	get_ramdac_data_reg_Long(void);

// ===========================================================================
// 2X6(WRITE)		   RAMDAC input mask register functions
// ===========================================================================
int FAR 	set_ramdac_input_mask_reg(BYTE mask_value);

// ---------------------------------------------------------------------------
//			   miscellaneous routines
// ---------------------------------------------------------------------------
int FAR PASCAL	JUN_init_ramdac_entry(PALETTEENTRY far *palette);
//int FAR PASCAL  JUN_init_ramdac_entry(RGBQUAD palette[]);
int FAR PASCAL	JUN_set_scan_mode(BYTE interval_mode);

int FAR PASCAL	JUN_clear_TVscreen(void);
int FAR PASCAL	JUN_set_TVscreen_color(BYTE  color_entry);

int FAR PASCAL
     JUN_draw_point(UINT  x, UINT  y, BYTE  color_entry),
     JUN_draw_line(UINT  x1, UINT  y1, UINT  x2, UINT  y2, BYTE  color_entry),
     JUN_draw_rect(UINT  x, UINT  y, UINT  width, UINT	height,
		   BYTE  color_entry, UINT  fill_flag),
     JUN_draw_color_bar(int  pitch, int  color_no, BYTE  color_bar[]),
     JUN_draw_slant_bar(int  pitch, int  color_no, BYTE  color_bar[], int dirt),
     JUN_draw_circle(UINT  x, UINT  y, UINT  radius, BYTE  color_entry, UINT  fill_flag),
     JUN_draw_ellipse(UINT  x, UINT  y, UINT  xradius, UINT  yradius, BYTE  color_entry, UINT  fill_flag),
     JUN_putchar(UINT  x, UINT	y, BYTE  ch, BYTE  color_entry),
     JUN_putstring(UINT  x, UINT  y, LPSTR  str, BYTE  color_entry);

/* 11/05/93 Tang added for Hardware update */
void NEAR	    myoutportb(unsigned portid, unsigned char value);
unsigned char NEAR  myinportb(unsigned portid);
/* 11/05/93 Tang added for Hardware update */

/* 11/26/93 Tang added for update horizontal shift register */
int FAR 	set_horz_shift_reg1(UINT  shift_pixel);
/* 11/26/93 Tang added for update horizontal shift register */

int FAR 	select_bank(BYTE type, UINT yPos);

//12/20/93 Tang added for special effect
int FAR PASCAL	JUN_get_ramdac_entry_6b(PALETTEENTRY far *palette);
int FAR PASCAL	JUN_init_ramdac_entry_6b(PALETTEENTRY far *palette);
//12/20/93 Tang added for special effect

// ***************************************************************************
//    juninita.c
// ***************************************************************************
int NEAR	init_io_reg_val(void);
int FAR PASCAL	JUN_Initialize(HANDLE hWND),
		JUN_Exit(void),
		JUN_LoadConfiguration(void),
		JUN_SaveConfiguration(void),
		JUN_GetProfileString(LPSTR lpSectionName, LPSTR lpKeyName,
                                     LPSTR lpReturnString, int nSize);
WORD FAR PASCAL JUN_GetProfileInt(LPSTR lpSectionName, LPSTR lpKeyName);

int FAR PASCAL	JUN_SetSystemPalette(PALETTEENTRY palette[]),
		JUN_GetSystemPalette(PALETTEENTRY palette[]);
/* 11/04/93 Tang added to support retrieve info. */
int FAR PASCAL	JUN_RetrieveColorKey(long rgbColor);
long FAR PASCAL JUN_ColorentryToRGB(BYTE color_entry);
int FAR PASCAL	JUN_RGBToColorentry(long rgbColor),
		JUN_get_color_key(void),
		JUN_get_display_mode(void),
		JUN_get_scan_mode(void);

int NEAR	AllocSel(WORD wOffset, WORD wSize);
//12/06int NEAR AllocSel(void);
int NEAR	FreeSel(void);

BYTE FAR *	GetVideoAddress(UINT, UINT);
#ifdef DOSLIB
  static char far *lpMemoryAddress;
#endif

void		ReversePalette(PALETTEENTRY far *palette);
/* 11/19/93 Tang added */
UINT FAR	fnRelX(UINT xPos);
UINT FAR	fnRelY(UINT yPos);
/* 11/19/93 Tang added */
// 12/02/93 Tang added to verify configuration
int NEAR	VerifyConfiguration(void);
int NEAR	VerifyPortAndID(WORD wPort, BYTE bCardID);
int NEAR	VerifyIRQ(BYTE bIRQ);
int NEAR	VerifyFrameAddr(WORD wFrameAddr);
int NEAR	VerifyRamdac(void);
int NEAR	SetConfiguration(void);
// 12/02/93 Tang added to verify configuration
//12/06
int NEAR	alloc_and_check_bank_access(void);
//12/07/93 Tang added to diagnose hardware
int FAR PASCAL	JUN_Diagnosis(BOOL flag);   //1/27/94
int NEAR 	AllocPageBuffer(void);
int NEAR 	FreePageBuffer(void);
void FAR        InitialPageBuffer(void);

// ***************************************************************************
//    junwbmpa.c
// ***************************************************************************
int FAR PASCAL	JUN_ReadImageRect(UINT xLeft, UINT yTop, UINT xWidth, UINT yHeight, LPSTR lpImageBuf),
		JUN_WriteImageRect(UINT xLeft, UINT yTop, UINT xWidth, UINT yHeight, LPSTR lpImageBuf),
		JUN_ClearImageRect(UINT xLeft, UINT yTop, UINT xWidth, UINT yHeight, BYTE bColorEntry);
/* 10/19/93 Tang added to test bitmap */
int FAR PASCAL	JUN_WriteBitmap(HDC hDC, HPALETTE hPAL,
				UINT xVramLeft, UINT yVramTop, UINT xDataWidth, UINT yDataHeight,
				UINT xDataLeft, UINT xDataTop, UINT xPageWidth, UINT yPageHeight),
		JUN_WriteBitmap2(HANDLE hInst, UINT xLeft, UINT yTop),
//		JUN_WriteBitmap3(HANDLE hInst, UINT xLeft, UINT yTop),
		JUN_WriteBitmap4(UINT xLeft, UINT yTop,LPSTR filenamestr),
		JUN_WriteBitmap5(HBITMAP hDC, HPALETTE hPAL, UINT xLeft, UINT yTop, UINT xWidth, UINT yHeight),
		JUN_WriteBitmap6(HDC hDC, HPALETTE hPAL, UINT xLeft, UINT yTop, UINT xWidth, UINT yHeight),
		JUN_LoadObject(HDC hDC,
			       UINT xLeft,  UINT yTop,
			       UINT xWidth, UINT yHeight),
		JUN_WriteBitmap7(UINT xVramLeft, UINT yVramTop,
				 UINT xDataWidth, UINT yDataHeight,
				 UINT xDataLeft, UINT yDataTop);

BOOL		CompactGlobalHeap(DWORD dwReqAllocSize);
char far *	AllocateMemory(unsigned long);
void NEAR	FreeMemory(char far *, HANDLE);
int  FAR	HandleImageRect(UINT, UINT, UINT, UINT, LPSTR, UINT);
//12/27/93 Tang added
int FAR GetImageData(HDC hDC, UINT xPageWidth, UINT yPageHeight, LPSTR lpImgBuf);
int FAR GetAndSetPalette(HDC hDC, HPALETTE hPAL);


// ***************************************************************************
//    junblta.c
// ***************************************************************************
//int NEAR	  WriteToVideo(UINT xLeft,  UINT yTop, UINT xWidth, UINT yHeight,
//			       LPSTR lpImageBuf, UINT nRop);
//int FAR PASCAL  JUN_BitBlt(HDC hDC, UINT xLeft, UINT yTop,
//			    UINT xWidth, UINT yHeight);

// ***************************************************************************
//    junscola.c
// ***************************************************************************
/* 11/05/93 Tang added to support scroll  */
int FAR PASCAL	JUN_ScrollBitmap(HDC hDC,	  HPALETTE hPAL,
				 UINT xVramLeft,  UINT yVramTop,
				 UINT xDataWidth, UINT yDataHeight,
				 UINT xDataLeft,  UINT yDataTop,
				 UINT xPageWidth, UINT yPageHeight,
				 UINT wType,	  UINT wSpeed),
		JUN_ScrollLines(UINT wDir, UINT wSpeed, UINT wAmount),
		JUN_ScrollBitmap1(LPSTR lpImageBuf, UINT xWidth, UINT yHeight,
				  int nDir, int nSpeed),
		JUN_ScrollPieces(HANDLE hDC, UINT xWidth, UINT yHeight, int nDir),
		JUN_VerticalScroll(int nDir, int nSpeed),
		JUN_HorizontalShift(int nDit, int nSpeed);

int FAR 	HandleImageRect1(UINT, UINT, UINT, UINT, UINT, UINT, UINT, UINT, LPSTR, UINT);

// ***************************************************************************
//    junisra.c
// ***************************************************************************
int FAR PASCAL	JUN_ScrollPages(HDC hDC,	 HPALETTE hPAL,
				UINT xVramLeft,  UINT yVramTop,
				UINT xDataWidth, UINT yDataHeight,
				UINT xDataLeft,  UINT yDataTop,
				UINT xPageWidth, UINT yPageHeight,
				UINT wType,	 UINT wSpeed),
		test_vert_sync_int(void),
		JUN_StartScroll(void),
		JUN_QuitScroll(void);
void interrupt	JUN_isr_scrolling(void);
// 11/18/93 Tang added
void FAR	init_interrupt(BYTE bIRQ);
void FAR	init_interrupt_test(BYTE bIRQ);
void FAR	restore_interrupt(BYTE bIRQ);
void interrupt	new_interrupt(void);
BYTE NEAR	SetInterruptMask(BYTE bIRQ);

// ***************************************************************************
//    dpmi.asm
// ***************************************************************************
int		AllocSelectors(void);
void		FreeSelectors(void);
char far *	FarMemoryCopy(DWORD lpSource, DWORD lpDestin, WORD wCount);
void		AccessMapMemory(WORD xPos, WORD yPos, WORD wWidth, DWORD fpImageBuf, BYTE bFlag);
char far *	SetInterruptVector(BYTE bIRQ, DWORD lpNewISR);
int		GetVideoMode(void);


//12/16/93
void FAR PASCAL JUN_SEF_FadeInPal(int wSpeed);
void FAR PASCAL JUN_SEF_FadeOutPal(int wSpeed);
void FAR PASCAL JUN_SEF_RotatePal(int wStartEntry, int wEndEntry, int wTimes, int wSpeed);
int FAR 	DebugMsg(void);
WORD FAR        SpeedToTime(WORD wSpeed);
void FAR 	delay1(WORD wMiliSec);
int FAR PASCAL	JUN_ShowImage(HDC hDC, HPALETTE hPAL,
			      UINT xVramLeft, UINT yVramTop,
			      UINT xDataWidth, UINT yDataHeight,
			      UINT xDataLeft, UINT yDataTop,
			      UINT xPageWidth, UINT yPageHeight,
			      UINT wEffectType, UINT wShowSpeed);
int FAR 	CopyBuf2TV(UINT xBufLeft,  UINT yBufTop,
			   UINT xTVLeft,   UINT yTVTop,
                           UINT xBufWidth, UINT yBufHeight,
			   UINT wSize);

//12/23/93
int NEAR	CheckVGAMode(void);
int FAR  VertWrite(UINT xLeft,  UINT yTop,      //(left,top)
                   UINT xWidth, UINT yHeight,
                   UINT yWrite, UINT yLines,
                   UINT VramPos,
		   LPSTR lpImageBuf);            //r/w buffer
int FAR  HorzWrite(UINT xLeft,  UINT yTop,      //(left,top)
                   UINT xWidth, UINT yHeight,
                   UINT xWrite, UINT xLines,
                   UINT VramPos,
		   LPSTR lpImageBuf);            //r/w buffer
//12/31/93
void NEAR InitialGlobals(void);
void FAR  ShowFreeSpace(LPSTR lpPromptMsg);
void delay(int time_unin);   /* delay is in 1/10 seconds */
