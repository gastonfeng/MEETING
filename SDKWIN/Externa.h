
// **************************************************************************
// Project : junior.prj
// File    : global.h
// Object  : declaration of global variables for reference by all modules
// Author  : Ricky Hsu
// Date    : 820810
// **************************************************************************


// ---------------------------------------------------------------------------
//			 system related parameters
// ---------------------------------------------------------------------------

//extern BYTE
extern PALETTEENTRY
       palette_table[256];		   // color values of RAMDAC

extern UINT
       mask[16],
       *char_set[]; // character set table

// ---------------------------------------------------------------------------
//		  addresses of I/O registers of Overlay Junior
// ---------------------------------------------------------------------------
extern UINT
       control_reg,	      // 2X0 : control register
       status_reg,	      // 2X0 : status register
       color_key_reg,	      // 2X1 : color key register
       card_id_reg,	      // 2X2 : card ID register
       crt_sel_reg,	      // 2X3 : CRT select register
       crt_data_reg,	      // 2X4 : CRT data register
       ramdac_addr_write_reg, // 2X5 : RAMDAC address write register
       ramdac_addr_read_reg,  // 2X6 : RAMDAC address read register
       ramdac_data_reg,       // 2X7 : RAMDAC data register
       ramdac_input_mask_reg; // 2X8 : RAMDAC input mask register

// ---------------------------------------------------------------------------
//		   values of I/O registers of Overlay Junior
// ---------------------------------------------------------------------------
extern BYTE
       control_reg_val, 	  // value of control register
       status_reg_val,		  // value of status register
       color_key_reg_val,	  // value of color key register
       card_id_reg_val, 	  // value of card ID register
       crt_sel_reg_val, 	  // value of CRT select register
       crt_data_reg_val,	  // value of CRT data register
       ramdac_addr_write_reg_val, // value of RAMDAC address write register
       ramdac_addr_read_reg_val,  // value of RAMDAC address read register
       ramdac_input_mask_reg_val; // value of RAMDAC input mask register

extern ULONG
       ramdac_data_reg_val;	  // value of RAMDAC data register

extern CFGSTRUCT  gConfig;      // structure of JUNIOR.INI file

extern WORD
       gwVideoWidth,            // width of video memory at current scan mode
       gwVideoHeight,           // height of video memory at current scan mode
       /* Used in libinit.asm */
       gwFrameBufSel,           // frame buffer selector(given by Windows)
       gwNSelectors;            // no of selectors to allocate

// 10/18 Tang added
extern BYTE FAR
       *glpVideoMemory;         // far pointer to address of video memory

extern BYTE
       gbInitial,               // times of initialization
       gbTVsystemType,          // type of TV system(0:NTSC, 2:PAL)
       gbMemReadMode,           // read mode of video memory(ENABLE/DISABLE)
       gbMemWriteMode,          // write mode of video memory(ENABLE/DISABLE)
       gfExtrnVideoExist;       // flag of external video exist

extern int
       gwErrorCode;             // return error code of function


extern	HANDLE  ghMemory; 	/* Handle to memory allocated by Windows */
extern	HANDLE	hWnd, hInst, hdcInfo;

extern  PALETTEENTRY  system_palette[256];  // color values of system palette

extern unsigned long
       gulEnterIntCount;

extern int  ynouse, yheight, sstart, gfScroll;

/* array variable in function */
extern char          debugstr[100];      //string for MessageBox
extern PALETTEENTRY  pe[MAXCOLORENTRY];  //256 palette entries
extern UINT          xLength[2];

// global variable for handling scrolling
extern BOOL  gfHandleScroll;     // flag to initinate ISR to start
extern BOOL  gfScrollStop;       // flag indicate last page
extern BOOL  gfSwitch_PRG;
extern BYTE  gbScrollType[2];    // 4 types of scrolling
extern int   gwRegTempStart[2];  // scoll register temp value
extern int   gwVramStart[2];     // line to put image into
extern WORD  gwPageCount;        // count to load pages
extern WORD  gwRegGaps[2];       // increments of register value
extern WORD  gwxShowWidth[2];    // width of desired area
extern WORD  gwyShowHeight[2];   // height of desired area
extern WORD  gwxPageWidth[2];    // width of a page
extern WORD  gwyPageHeight[2];   // height of a page
extern WORD  gwPadded[2];        // padded no. to make a multiple of 2
extern WORD  gwPieceCount;       // lines to processed
extern BYTE huge  *ghpBits[2];   // huge pointer to image bits

extern BYTE  gbVertScanStart;    // vert. scan start value of reg.
extern BYTE  gbHorzScanStart;    // horz. scan start value of reg.
extern BOOL  gfSwitch_ISR;       // switch
extern BOOL  gfPriIsrFinish;     // flag of prior ISR execution
extern HANDLE  ghMem[2];         // handle of memory

// 11/18/93 Tang added
//0104/94 extern BOOL  gfStartScroll;      // semaphore to isr to start scrolling

extern UINT  gwxVmemLeft;        // start x pos. in Video memory
extern UINT  gwyVmemTop;         // start y pos. in Video memory
extern UINT  gwxChangeLeft[2];
extern UINT  gwyChangeTop[2];
extern UINT  gwxShowLeft[2];
extern UINT  gwyShowTop[2];

extern BOOL  gfBufferFull[2];
extern UINT  gwLineGaps[2];
extern LPSTR glpEmptyArray;            // pointer to an empty array
extern BYTE huge  *ghpBitsStart[2];    // huge pointer to image bits

/* 12/06/93 Tang added */
extern BYTE  gbBankType, gbDiagnose;

/* 12/22/93 Tang added */
extern BYTE  gbVideoMode;
extern WORD  gwImgWidthPad;
extern LPSTR glpPageBuf;

//12/30
extern BYTE  gbPreMemSeg,gbPreBnkNo;

// 3/8/94 Tang added for PAL
extern WORD gwVertRegOffset;
