unit global;

interface

uses windows;
type
CFGSTRUCT=Record
	   wFrameAddr:WORD;
	   wPortBaseAddr:WORD;
	   bCardID:BYTE;
	   bIRQUsed:BYTE;
	   bColorKey:BYTE;
	   bDisplayMode:BYTE;
	   bScanMode:BYTE;
	   wVertScanStart:WORD;
	   wHorzScanStart:WORD;
end;


var

  szCfgFileSpec:array [0..255] of char;
  szData:pchar;
  gconfig:CFGSTRUCT;
  status_reg_val:byte=$61;
  crt_sel_reg_val:byte=$c0;
  control_reg_val:byte=$20;
  crt_data_reg_val:byte=$64;
  gbinitial:boolean=false;
  gwErrorCode:integer=0;
  gfExtrnVideoExist:boolean=false;
  gbTVsystemType:byte=0;
  gwVertRegOffset:word;

  ramdac_input_mask_reg_val:byte=0;
  gfHandleScroll:boolean   = FALSE;// flag to initinate ISR to start
  gfScrollStop:boolean     = TRUE; // flag indicate last page

  gwPadded:array[0..1] of word;
  ghmem:array[0..1] of word;
  gfBufferFull:array[0..1] of boolean;
  gbVideoMode:BYTE=0;
  gwImgWidthPad:word=0;
  gbPreMemSeg:BYTE=9;
  gbPreBnkNo:BYTE=9;

  system_palette:PALETTEENTRY;
  palette_table:PALETTEENTRY;    // color values of RAMDAC

  gbBankType:BYTE=1;
  gbDiagnose:BYTE=0;

  gbMemReadMode:BYTE=0;
  gbMemWriteMode:BYTE=0;

  gulEnterIntCount:ulong=0;

  color_key_reg_val:BYTE=$fc;
  card_id_reg_val:BYTE=$0e;
  ramdac_addr_write_reg_val:byte=0;
  ramdac_addr_read_reg_val:byte=0;
  ramdac_data_reg_val:ulong=0;

  gwVideoWidth:word=640;
  gwVideoHeight:word=400;

  glpVideoMemory:^BYTE=NIL;
const

  CFG_FILE_NAMEW:pchar='%s\\junior.ini';
  NTSC:byte=0;
  PAL:byte=2;
  EXTERNAL_VIDEO_ONLY:BYTE=0;
  OVERLAY:BYTE=$20;

  UNDERSCAN:BYTE=0;
  OVERSCAN:BYTE=1;

  BANK_SIZE_64K:BYTE=0;
  BANK_SIZE_16K:BYTE=1;
  BANK_SIZE_8K:BYTE=2;

  MEMORY_SIZE_8K:WORD=$2000;
  MEMORY_SIZE_16K:WORD=$4000;
  FLASH_VIDEO_MEMORY:byte=$80;

  control_reg:word=$280; // 2X0 : control register
  color_key_reg:word=$281; // 2X1 : color key register
  card_id_reg:word          =$282; // 2X2 : card ID register
  crt_sel_reg:word          =$283; // 2X3 : CRT select register
  crt_data_reg:word         =$284; // 2X4 : CRT data register
  ramdac_addr_write_reg:word=$285; // 2X5 : RAMDAC address write register
  ramdac_addr_read_reg:word =$286; // 2X6 : RAMDAC address read register
  ramdac_data_reg :word     =$287; // 2X7 : RAMDAC data register
  ramdac_input_mask_reg:word=$288; // 2X8 : RAMDAC input mask register

  gwNSelectors:word=1;
  gwFrameBufSel:word=0;

  status_reg:word=$280;
  BLANK_TIMING_REG_SEL:byte=2;
  MEMORY_READ_ENABLE:byte=0;
  MEMORY_READ_DISABLE:byte=8;
  MEMORY_READ_ENABLE_BT:byte=0;
  MEMORY_READ_DISABLE_BT:byte=1;
  READ_BACK1:byte=8;

  ERR_INITIALIZED_BEFORE:integer=-1000;
  ERR_JUNIOR_INI_NOT_FOUND:integer=-1001;
  ERR_JUNIOR_CARD_NOT_EXIST:integer=-1002;
  ERR_CANNOT_ALLOCATE_SELECTOR:integer=-1003;
  ERR_INSUFFICIENT_MEMORY:integer=-1007;
  ERR_BANK_ACCESS_CHECK_ERROR:integer=-1018;
implementation

end.
