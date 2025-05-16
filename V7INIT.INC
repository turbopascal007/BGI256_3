;V7INIT.INC - Copyright 1991,1992  Knight Software
;    History:
;         17 May 1991 - first release
;        
;----------------------------------------------------------------------
;Determine what sort of card is out there
;Assume:  DS = data segment
;Entry:   N/A
;Return:  Correct procedure pointers installed
;Destroy: AX

DetectCard PROC NEAR
	PUSH	ES
	PUSH	BX		;check if this is a Video Seven

	MOV	BX,DS:[SEGA000]
	MOV	DS:[VideoSegment],BX  ;init Video Segment/selector info

	MOV	BX,0
	MOV	AX,6F00H
	INT	10H
	CMP	BX,'V7'
	JZ	@IsVideoSeven	;yup, so go init it

	MOV	AX,1		 ;bummer, it ain't so error out
	MOV	DS:[NumberModes],AX ;set number modes available
	LEA	BX,VGABankSelect ;set bank select proc pointer
	MOV	DS:[BankSelectProc],BX

	LEA	BX,VGA320x200Init	;we do all this so as to
	MOV	DS:[InitDisplayProc],BX	;keep things in some 
	LEA	BX,DummyCard		;form of order to prevent
	MOV	DS:[CardNamePtr],BX	;nasty things from happening
	MOV	BX,64			;we are just telling the code
	MOV	DS:[WinGran],BX		;that we have a simple VGA 
	MOV	AH,grNotDetected	;or MCGA card set to mode 13H
	MOV	DS:[StatBlock.stat],AH	;then set no detect error
	MOV	BYTE PTR DS:[ModeSelect],0 ;force mode to 0
	OR	AX,255
	JMP	@DetectExit

@IsVideoSeven:
	MOV	AX,MaxModes	;set number modes available
	MOV	DS:[NumberModes],AX
	LEA	BX,V7BankSelect ;set bank select proc pointer
	MOV	DS:[BankSelectProc],BX

	MOV	AL,DS:[ModeSelect]
	AND	AX,7
	LEA	BX,V7ModeProcTable ;set display init proc pointer
	ADD	BX,AX
	ADD	BX,AX
	MOV	BX,CS:[BX]
	MOV	DS:[InitDisplayProc],BX
	LEA	BX,V7name
	MOV	DS:[CardNamePtr],BX   ;set up pointer to card name
	MOV	BX,64
	MOV	DS:[WinGran],BX	   ;set window granularity
	XOR	AX,AX
@DetectExit:
	POP	BX
	POP	ES
	RET
DetectCard ENDP

MaxModes EQU 5	;max number modes available for Video Seven

V7ModeProcTable:
	DW	VGA320x200Init 	  ;table of init proc ptrs
	DW	V7vga640x400Init  ;for the various modes
	DW	V7vga640x480Init 
	DW	V7vga800x600Init 
	DW	V7vga1024x768Init ;(V7 VRAM II only)

	DW	VGA320x200Init 	;dummy excess mode selects
	DW	VGA320x200Init  ;to fill out to 8 positions
	DW	VGA320x200Init 

V7name:
	DB 7		;name length
	DB ' Video7',0  ;Card name preceeded with space
			;and taged with a zero

;----------------------------------------------------------------------
;Select V7vga640x400 mode operation
;Assume:  DS = data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX,BX

V7vga640x400Init PROC NEAR
	MOV	AX,6F05H
	MOV	BL,66H	;get graph mode to select
	INT	10H	;call BIOS to set the mode
	XOR	AX,AX
	RET
V7vga640x400Init ENDP

;----------------------------------------------------------------------
;Select V7vga640x480 mode operation
;Assume:  DS = data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX,BX

V7vga640x480Init PROC NEAR
	MOV	AX,6F05H
	MOV	BL,67h	;get graph mode to select
	INT	10H	;call BIOS to set the mode
	XOR	AX,AX
	RET
V7vga640x480Init ENDP

;----------------------------------------------------------------------
;Select V7vga800x600 mode operation
;Assume:  DS = data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX,BX

V7vga800x600Init PROC NEAR
	MOV	AX,6F05H
	MOV	BL,69h	;get graph mode to select
	INT	10H	;call BIOS to set the mode
	XOR	AX,AX
	RET
V7vga800x600Init ENDP

;----------------------------------------------------------------------
;Select V7vga1024x768 mode operation
;Assume:  DS = data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX,BX

V7vga1024x768Init PROC NEAR	;@@@ Is this the right select code? @@@
	MOV	AX,6F05H
	MOV	BL,6Ah	;get graph mode to select
	INT	10H	;call BIOS to set the mode
	XOR	AX,AX
	RET
V7vga1024x768Init ENDP

;----------------------------------------------------------------------
;Video Seven Bank select routines - We assume that full memory is
;available if you try to access it.
;Assume:  DS = data segment
;Entry:   DX = Bank to select
;Return:  Display Bank is selected
;Destroy: nothing

V7BankSelect PROC NEAR
	PUSH	AX
	PUSH	BX
	PUSH	DX
	MOV 	BL,DL
	AND 	BL,1        ;bl=extended page select
	MOV 	AH,DL
	AND 	AH,2
	SHL 	AH,1
	SHL	AH,1
	SHL	AH,1
	SHL	AH,1        ;ah = page select bit
	AND	DL,0CH
	MOV	BH,DL
	SHR	DL,1
	SHR	DL,1
	OR	BH,DL        ;bh=256k bank select
	MOV	DX,3CCH
	IN	AL,DX        ;get misc output reg
	AND	AL,0FFH-20H  ;clear page select bit
	OR	AL,AH        ;set page select bit
	MOV	DX,3C2H      ;write misc output register
	OUT	DX,AL
	MOV	DX,3C4H      ;sequencer
	MOV	AL,0F9H      ;extended page select register
	MOV	AH,BL        ;extended page select value
	OUT	DX,AX
	MOV	AL,0F6H      ;256k bank select
	OUT	DX,AL
	INC	DX           ;point to data
	IN	AL,DX
	AND	AL,0F0H      ;clear out bank select banks
	OR	AL,BH        ;set bank select banks
	OUT	DX,AL
	POP	DX
	POP	BX
	POP	AX
	RET
V7BankSelect ENDP

