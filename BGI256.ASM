
;BGI256.ASM
;Copyright 1991,1992,1993 Knight Software
;P.O. Box 22902, Milwaukie, OR 97269
;BGI driver for Super VGA 256 color modes
;    History:
;       17 May 1991 - first release
;       10 Jun 1991 - Added ability to control pixel backgrounds
;       19 Aug 1992 - fixed windowing errors 
;       20 Aug 1992 - fixed VESA selection for Diamond Stealth
;                     which uses non-sequencial memory in 800x600
;       22 Nov 1992 - Adapted for protected mode operation
;	22 Dec 1992 - released working floodfill
;       30 Jan 1993 - corrected version 2 header (wasn't working with TP6)

;======================================================================

BGIVERSION EQU 3   ;select target BGI version 2 or 3

;**********************************************************************
;The following macro builds the required BGI header block.
;This macro is derived from Borland's own BGI toolkit
;from the DEVICE.INC file for compatibilty.
BGI	macro	name		; the following macro builds
	public	name		; the required ddo header block

     IF BGIVERSION LT 3

name	proc	far		; the vector table must be at
				; 'ddovec' 
	push	ds		; (push ds, push cs are signature)
	push	cs		; make ds=cs
	pop	ds
	cld			; just in case
	push	bp		; 6 bytes
	call	[DDOVEC+si]	; call the local function
	pop	bp		
	pop	ds		; restore regs
	ret			; ret far
name	endp
	db	'CB'
	db	0		; spare area
verID	db	BGIVERSION	; version id is at offset 15
;emulate must be at offset 16 
EMULATE:ret			; emulate function (nop until
	dw	0,0		; patched by loader)
RESERVED:
NONE:	ret			; near ret for unused functions
     ELSE

name	proc	far		; the vector table must be at
				; 'ddovec' 
	push	ds		; 
	MOV	DS,CS:[ALIAS]	; 
	cld			; clr dir flag just in case
	push	bp		; 
	call	[DDOVEC+si]	; call the local function
	pop	bp		
	pop	ds		; restore regs
	ret			; ret far
name	endp
verID	db	BGIVERSION		; version id is at offset 15
;alias must be at offset 16 with emulate at offset 18
ALIAS	dw	0
EMULATE:ret			; emulate function (nop until
	dw	0,0		; patched by loader)
RESERVED:
NONE:	ret			; near ret for unused functions

     ENDIF

	endm

;**********************************************************************
;Code segment *must* be first - only code and data segments allowed

_TEXT	SEGMENT PARA PUBLIC 'CODE'
     IF BGIVERSION LT 3
	ASSUME  DS:_TEXT, CS:_TEXT
     ELSE
	ASSUME  DS:_DATA, CS:_TEXT
     ENDIF

	BGI	BGI256		; Define the interface type

	INCLUDE SBASE.INC       ; Include base interface code
	INCLUDE SHARD.INC	; Include init/bank subroutines
	INCLUDE	SPIXEL.INC	; Include pixel subroutines
	INCLUDE SLINE.INC	; Include line subroutines
	INCLUDE SBIT.INC	; Incluce bitmap subroutines
	INCLUDE	SBLT.INC	; Include blt subroutines
	INCLUDE STEXT.INC	; Include text subroutines
	INCLUDE	SFLOOD.INC	; Include floodfill subroutines

     IF BGIVERSION LT 3
	INCLUDE INITDATA.INC	; do this if version 2 driver
	INCLUDE	SDATA.INC	; Include all data / variables
	INCLUDE	SPAL.INC	; Include default palette data
     ENDIF


;------------------------------------------------------------
; <-- If you are creating your own custom driver, comment -->
; <-- out the hardware specific driver here and add your  -->   
; <-- own custom include file for you specific hardware.  -->

;hardware specific code - select one appropriate include file
;	INCLUDE V7INIT.INC	; Include V7 init subroutines
;	INCLUDE TRIDENT.INC	; Include Trident init subroutines
	INCLUDE SVESA.INC	; Include VESA init subroutines

_TEXT	ENDS


;**********************************************************************
;Note: Data segment *must* be after code segment (for protected mode)

     IF BGIVERSION GE 3
_DATA	SEGMENT PARA PUBLIC 'DATA'	;declare the data segment
	ASSUME DS:_DATA, CS:Nothing

	INCLUDE INITDATA.INC	; InitData *MUST* be first in data seg
	INCLUDE	SDATA.INC	; Include all data / variables
	INCLUDE	SPAL.INC	; Include default palette data

	; <-- if you need your own additional custom data space -->
	; <-- add it here as a custom include file.             -->

_DATA	ENDS
     ENDIF

;**********************************************************************

	END

