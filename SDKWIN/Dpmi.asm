;****************************************************************************
; Project : junior.dll
; Module  : dpmi.asm
; Description: 1. DPMI service for mapping physical memory.
; Author  : Kung-Puu Tang
; Date	  : 820917
;****************************************************************************
.286p
include junior.inc

.MODEL medium

.DATA
_ProtectedModeAPI dw	0		;No protected mode API
wHorzRes          dw    0
wVertRes          dw    0
wMode             dw    0
fHicolor          dw    0
wHorzTotal        dw    0
ENDS

extrn  _gConfig:word
extrn  _gwFrameBufSel:WORD
extrn  _gwNSelectors:WORD
extrn  __D000H:word
extrn  _gbVideoMode:BYTE

public	_AllocSelectors
public	_FreeSelectors
public	_FarMemoryCopy
public	_AccessMapMemory
public	_SetInterruptVector
public  _GetVideoMode_asm

;****************************************************************************
; pause
;   macro to insure that an instruction fetch occurs between IN and/or
;   OUT instructions on the PC-AT machine ALSO defeat the prefetch cache!
;
; Registers Destroyed:
;   None
;****************************************************************************
pause   MACRO               
        jmp     $+2         
        ENDM                

.CODE

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

;****************************************************************************
; AllocSelectors
;
;   Allocate 1 selector to access 64KB of the Overlay Junior buffer located
; at the specified address(Upper Memory Block) which must be on 64K boundary.
;
; Following steps are implemented:
;
;	1. Find out whether the Windows Protected Mode API (INT 31H) services
;	   are available?  If not, return an error and exit.
;	2. Allocate 1 selector and save the selector.
;	3. Map physical address to a logical address.
;	 3a. Use the logical address and set the selector limit.
;	 3b. Set the selector limit to 64K.
;
; Exported:	No
; Entry:	None
; Destroys:	AX,BX,CX,DX,SI,DI,FLAGS
;****************************************************************************
_AllocSelectors PROC	far
	push	bp
	mov	bp, sp
	push	si
	push	di

	xor	ax,ax
	mov	_ProtectedModeAPI, ax	;Assume no INT31 services are available.
	mov	_gwFrameBufSel, ax	  ;Assume no valid selector.

	mov	ax, 1686H		;Function number for INT 31H
;;	mov	ax, 1687H		;Function number for INT 31H
	int	2FH			;Get DPMI service
	test	ax, ax
	jnz	ais_Int31_is_not_available

	mov	ax, 0400h		;Get DPMI version
	int	31h

	xor	ax, ax			;Func 0 to allocate selector from LDT
					; TTAO	32 selectors to address 2MB
	mov	cx, _gwNSelectors	;Needs N selectors to address 64K

	int	31H			;Protected mode API services
	jc	ais_sel_aloc_err	;Could not get selector
	mov	_gwFrameBufSel, ax	;Save first selector in the array of selectors.

ais_set_sel_lp:
	mov	ax, 0800h		;Map physical address (Needed for 386)
	mov	bx, _gConfig.wFrameAddr ;Frame buffer address at UMB
	xchg	bh, bl
	shr	bx, 4			;shr bx,24
	mov	cx, 0000h		;16 LSBs, bx:cx - physical linear addr.
	mov	si, 0000h		;Size of the region (16 MSBs), si:di
	mov	di, 1000h		;16 LSBs(64K)
	int	31H			;return: bx:cx=linear addr. that can be
	jc	ais_phy_map_err 	;used to access physical addr.

;;	mov	ax, 0600h
;;	mov	si, 0010h
;;	mov	di, 0000h
;;	jc	ais_phy_map_lock_err

; 9/30 Tang added
	mov	dx, 0
	mov	cx, _gConfig.wFrameAddr
	xchg	ch, cl
	shr	cx, 4
	mov	dx, cx			;DX = 16 LSBs of the mapped address
	mov	cx, bx			;CX = 16 MSBs of the mapped address
	mov	bx, _gwFrameBufSel	;Current selector = FrameBufSel
	mov	ax, 0007		;Set selector's base address
	int	31H			;Call Protected Mode API services.
	jc	ais_set_addr_err
					;BX = selector
	mov	ax, 0008H		;Set selector limit to 64K(CX:DX)
	mov	cx, 0000H		;16 MSBs of linear size
	mov	dx, 0FFFFH		;16 LSBs of linear size
	int	31H			;Call Protected Mode API services.
	jc	ais_set_lim_err

;;	mov	ax, 0009H		;Set descriptor access right
;;					;BX = Current selector
;;	mov	cx, 00F3H		;CX = Access rights/type byte
;;	int	31H			;Call Protected Mode API services.
;;	jc	ais_set_access_right_err

	mov	ax, 1
	mov	_ProtectedModeAPI, ax	;Indicate Protected Mode API available.
;	xor	ax, ax
	jmp	short AllocNSelRet

ais_Int31_is_not_available:		;No INT 31H Services are available.
	mov	ah, 2			;Set border color to 1.
	jmp	short ais_err_exit

ais_sel_aloc_err:			;Could not get selector.
	mov	ah, 3			;Set border color to 2.
	jmp	short ais_err_exit

ais_phy_map_lock_err:			;Could not map physical memory.
ais_phy_map_err:			;Could not map physical memory.
	mov	ah, 4			;Set border color to 3.
	db	3dH			;Swallow next 2 bytes by CMP AX,

ais_set_addr_err:			;Could not set the address.
	mov	ah, 5			;Set border color to 4.
	db	3dH			;Swallow next 2 bytes by CMP AX,

ais_set_access_right_err:
ais_set_lim_err:			;Could not set selector limit.
	mov	ah, 6			;Set border color to 5.
	push	ax
	call	far ptr _FreeSelectors
	pop	ax			;Get proper border color.

ais_err_exit:
AllocNSelRet:
	pop	di
	pop	si
	mov	sp, bp
	pop	bp
	ret
_AllocSelectors ENDP

;****************************************************************************
; FarMemoryCopy
;
;   Copy block of memory to a buffer(does not handle segment crossings).
;
; Exported:	No
; Entry:	1. lpSource - long pointer to source buffer
;		2. lpDestin - long pointer to destination buffer
;		3. wCount   - number of bytes to copy
; Return:	DX:AX - Source Address
; Modifies:	BX,CX,SI,DI,DS,ES
;****************************************************************************
_FarMemoryCopy	PROC	far
   ARG	lpSource:DWORD, lpDestin:DWORD, wCount:WORD

	push	bp
	mov	bp, sp
	push	ds
	push	es
	push	si
	push	di
	push	cx

	lds	si, lpSource		;Get source pointer
	les	di, lpDestin		;Get destin pointer
;;	  mov	  di, WORD PTR lpDestin[0]
;;	  mov	  ax, WORD PTR lpDestin[2]
;;	  mov	  es, ax
;;	  mov	  si, WORD PTR lpSource[0]
;;	  mov	  ax, WORD PTR lpSource[2]
;;	  mov	  ds, ax
	mov	cx,wCount
;; 11/15/93 Tang updated
;	rep	movsb			;Copy the memory
;; 11/15/93 Tang updated
	shr	cx, 1
	rep	movsw			;unit:word
	adc	cl, cl			;If odd, then CF=1
	rep	movsb			;Copy last one byte

;return value
	mov	dx,ds			;Return (dx,ax) = (ds,si)
	mov	ax,si

	pop	cx
	pop	di
	pop	si
	pop	es
	pop	ds
	mov	sp, bp
	pop	bp

	ret
_FarMemoryCopy	ENDP


; Return:	DX:AX - start address to mapping
GetStartAddr	PROC	near
	push	es
	push	dx
	push	si
	push	di
	mov	dx, __D000H
	mov	es, dx
	and	ax, 03Fh
	xchg	ah, al
	shl	ax, 2			;yPos*1024
	add	ax, bx
	pop	dx
	pop	di
	pop	si
	pop	es
	ret
GetStartAddr	ENDP


;**************************************************************************
; AccessMapMemory
;**************************************************************************
_AccessMapMemory	PROC	far
  ARG	xPos:WORD, yPos:WORD, wWidth:WORD, fpImageBuf:DWORD, bFlag:BYTE
	push	bp
	mov	bp, sp
	push	ds
	push	es
	push	si
	push	di
	mov	ax, yPos
	mov	bx, xPos
	call	GetStartAddr
h_read:
	cmp	bFlag, 0
	jne	h_write
	mov	ds, dx
	mov	si, ax
	mov	ax, WORD PTR fpImageBuf[2]
	mov	es, ax
	mov	di, WORD PTR fpImageBuf[0]
	mov	cx, wWidth
	rep	movsb
	jmp	short map_ret
h_write:
	cmp	bFlag, 1
	jne	h_clear
	mov	es, dx
	mov	di, ax
	mov	ax, WORD PTR fpImageBuf[0]
	mov	ds, ax
	mov	si, WORD PTR fpImageBuf[2]
	mov	cx, wWidth
	rep	movsb
h_clear:
	cmp	bFlag, 1
	jne	h_clear
	mov	es, dx
	mov	di, ax
	mov	al, 0
	mov	cx, wWidth
	rep	movsb
map_ret:
	pop	di
	pop	si
	pop	es
	pop	ds
	mov	sp, bp
	pop	bp
	ret
_AccessMapMemory	ENDP


;****************************************************************************
; SetInterruptVector
;
;   Set interrupt vector and save old interrupt.
;
; Exported:	No
; Entry:	1. bIRQ     - IRQ no.(not interrupt no) to set
;		2. lpNewISR - New ISR's address to set
; Return:	DX:AX - old vector's ISR address
; Modifies:	AX,BX,DX,DS,ES
;****************************************************************************
_SetInterruptVector   PROC    far
  ARG	bIRQ:BYTE, lpNewISR:DWORD
	push	bp
	mov	bp, sp
	push	es

	mov	al, bIRQ
	mov	ah, 08h
	add	al, ah			;AL = interrupt no.
	mov	ah, 35h 		;Get vector
	int	21h			;Get the old interrupt vector in es:bx

	push	es			;Save for a bit
	push	bx

	mov	ah, 25h 		;Set vector
	push	ds
	lds	dx, lpNewISR
	int	21h			;Set the new interrupt vector
	pop	ds

	pop	ax			;Restore old ISR for return value
	pop	dx			;DX:AX is the old handler

	pop	es
	mov	sp, bp
	pop	bp
	ret
_SetInterruptVector  ENDP

;****************************************************************************
; GetVideoMode
;
;   Routine to detect the video mode.
;
; Exported:	No
; Entry:	None
; Destroys:	AX,BX,CX,SI
;****************************************************************************
_GetVideoMode_asm   PROC  far
;        LOCAL	wHorzRes:WORD
;	LOCAL   wVertRes:WORD
;	LOCAL   wMode:WORD
;        LOCAL   fHicolor:WORD
;        LOCAL   wHorzTotal:WORD

	push	bp
	mov	bp, sp

	mov	dx, 3D4h 		;CRTC port address
	mov	al, 1			;Horizontal Display Enable End
	out	dx, al
	inc	dx
	in	al, dx
	xor	ah, ah
	inc	ax
	shl	ax, 3			;Get horizontal resolution.
	mov	wHorzRes, ax

	mov	dx, 3D4h 		;CRTC port address
	mov	al, 0			;Horizontal Total
	out	dx, al
	inc	dx
	in	al, dx
	xor	ah, ah
	add	ax, 5
	shl	ax, 3			;Get horizontal clock.
	mov	wHorzTotal, ax

	mov	dx, 3D4h 		;CRTC port address
	mov	al, 12h			;Vertical Display Enable End
	out	dx, al
	inc	dx
	in	al, dx
	mov	bl, al
	mov	dx, 3D4h 		;CRTC port address
	mov	al, 07h                 ;Overflow
	out	dx, al
	inc	dx
	in	al, dx
	xor	bh, bh
VDEE_bit8:
	test	al, 2                   ;Vert. Disp. Enab. End bit 8
	jz	VDEE_bit9
	or	bh, 1
VDEE_bit9:
	test	al, 40h                 ;Vert. Disp. Enab. End bit 9
	jz	vert_res
	or	bh, 2
vert_res:
	inc	bx
	mov	wVertRes, bx		;Get vertical resolution.

	mov	ah, 0fh			;[ADDA]
	int	10h			;Read Current Video State
        mov     _gbVideoMode, al
	cmp	al, 13h
	jbe	basic_mode
svga:
	mov	dx, 3C6h
	in	al, dx
	in	al, dx
	in	al, dx
	in	al, dx
	in	al, dx
	cmp	al, 0FFh
	je	reset_ramdac
	and	al, 0e0h 		;check RAMDAC mode
	jz	reset_ramdac		;256 color mode
	mov	ah, al
	in	al, dx
	cmp	al, 0ffh
	je	atnt
	test	ah, 80h
	jz	reset_ramdac
	mov	al, 1			;Sierra RAMDAC high byte = 1
hicolor_mode:
	xchg	al, ah
	mov	fHicolor, ax		;low byte contains pixel repack reg
	shr	wHorzRes, 1		;Divide horizontal resolution by 2.
	shr	wHorzTotal, 1		;Divide horizontal resolution by 2.
reset_ramdac:
	mov	dx, 3C8h
	in	al, dx			;Reset RAMDAC internal flag
	cmp	wHorzRes, 800
	ja	basic_mode
	mov	bx, 40h			;for 256 color mode
	jmp	short Exit_GetMode
atnt:
	cmp	ah, 0e0h 		;24 bit true color
	je	reset_ramdac
	cmp	ah, 40h			;24 bit true color
	je	reset_ramdac
	mov	al, 80h			;AT&T RAMDAC high byte = 0
	cmp	ah, 80h
	je	at_t
	or	al, 20h			;rest all double edge
	cmp	ah, 20h
	je	at_t
	cmp	ah, 0a0h
	je	at_t
	or	al, 40h			;rest all 5:6:5 format
at_t:
	mov	ah, al
	xor	al, al			;AT&T RAMDAC high byte = 0
	jmp	hicolor_mode

basic_mode:
	mov	dx, 3DAh
	in	al, dx			;Set attribute flip-flop.
	mov	dx, 3C0h 		;Attribute Controller port address
	mov	al, 30h			;Mode Control Register
	out	dx, al
	inc	dx
	in	al, dx			;Read Mode Control Register.
	mov	bl, al
	xor	bh, bh
	test	bl, 40h			;Packed pixel mode?
	jz	Exit_GetMode
	shr	wHorzRes,1		;Divide horizontal resolution by 2.

Exit_GetMode:
 	mov	sp, bp
	pop	bp
       ret
_GetVideoMode_asm   ENDP

END
