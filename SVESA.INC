;SVESA.INC - Copyright 1991,1992 Knight Software
;    History:
;       17 May 1991 - first release
;       19 Aug 1992 - fixed bug with seperate read/write windows. 
;                     Fixed push/pop bug in VesaBank select
;       22 Nov 1992 - adapted for protected mode operation
;
;----------------------------------------------------------
;This file provides VESA support

;----------------------------------------------------------
;DetectCard detects whether the VESA support is out there.
;If VESA support is found, then the highest supported 
;256 color mode is determined. If no VESA support is found, 
;we default to standard VGA mode. 
;Assumed: DS=data segment 
;Enter: Nothing
;Return: NZ=error, Z=valid display/mode  and card is setup
DetectCard PROC NEAR
	PUSH	ES
	PUSH	DI
	PUSH	SI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX

	MOV	BX,DS:[SEGA000]	   ;init video segment/selector
	MOV	DS:[VideoSegment],BX

	MOV	BX,0
	MOV	DS:[VidMode],BX       ;init VESA mode to zero
	MOV	DS:[NumberModes],BX   ;and max mode
	MOV	DS:[ModeErrorFlag],BL ;preclear error flag
	MOV	AX,4F00H	;get VESA display info
	LEA	DI,TempWork
	CALL	VESAInt
	JC	@NoVESA		;dpmi call failed - sigh
	CMP	WORD PTR ES:[DI],"EV"
	JNZ	@NoVESA		;error out if not VESA
	CMP	WORD PTR ES:[DI+2],"AS"
	JZ	@IsVESA
@NoVESA:
	JMP	@NotVESA

@IsVESA:
	MOV	SI,ES:[DI+0EH]	;BX:SI = ptr to table of avail modes
	MOV	CX,ES:[DI+10H]

	CMP DS:[CurOpMode],0  	;if CurOpMode is zero,
	JZ	@IsVESAchk	;no need to convert it

	PUSH	SI
	MOV	AX,0002H	;is prot mode, so gotta convert
	MOV	BX,CX		;real segment to a selector
	INT	31H
	POP	SI
	JC	@NoVESA		;ack, something went wrong
	MOV	CX,AX		;stick selector value in CX

@IsVESAchk:
	MOV	ES,CX		;ES:SI now points to mode table
	SUB	SI,2		;preadj pointer
@NextMode:
	ADD	SI,2
	MOV	AX,ES:[SI]	;get item from vesa list
	CMP	AX,-1		;last item?
	JZ	@LastMode
	MOV	CX,VESAmodeSize	;CX=our table length
	LEA	DI,VESAmodes	;DS:DI = modes we use
	MOV	BX,0		;BX=test mode number
@ModeLoop:
	CMP	AX,CS:[DI]	;Is the item in our list?
	JZ	@GoodMode
	ADD	DI,2		;try everything in our list
	INC	BX
	LOOP	@ModeLoop
	JMP	SHORT @NextMode	 ;go try next vesa mode
@GoodMode:
	MOV	BH,0
	CMP	BL,DS:[ModeSelect]
	JNZ	@NotOurMode
	MOV	DS:[VidMode],AX ;Save VESA mode number
@NotOurMode:
	CMP	BX,DS:[NumberModes] ;is mode higher than last one? 
	JL	@NextMode
	MOV	DS:[NumberModes],BX ;update max mode if higher
	JMP	SHORT @NextMode  ;check on next vesa mode

@LastMode:
	MOV	CX,DS:[VidMode]
	OR	CX,CX		 ;if Zero is not a valid
	JNZ	@ModeOK		 ;VESA mode 
	CMP	BYTE PTR DS:[ModeSelect],0 ;if mode zero
	JZ	@NotVESA	 ;is OK not to be VESA mode
	MOV	AH,grInvalidMode ;set error flag
	MOV	DS:[StatBlock.stat],AH
	MOV	BYTE PTR DS:[ModeErrorFlag],0FFH ;mark as mode error
	JMP	@NotVESA	 ;and go to default mode

@ModeOK:
	MOV	AX,4F01H	;get mode info
	LEA	DI,TempWork	;(mode is in CX)
	CALL	VESAInt		;buffer of data is 
	JC	@NotVESA	; returned in ES:DI
	MOV	CX,ES:[DI+4]	;Get window granularity
	OR	CX,CX		;if zero, not a valid value
	JZ	@NotVESA
	MOV	AX,64		;convert to 64K multiplier
	XOR	DX,DX		;for bank selects
	DIV	CX
	MOV	DS:[WinGran],AX	;Save it for later
	OR	AX,AX
	JZ	@NotVESA	;bad window granularity
	MOV	BX,ES:[DI+16]	;get real scan line length
	MOV	DS:[ScanLineLength],BX

	LEA	DI,VESAbank	;everything looks good
	MOV	DS:[BankSelectProc],DI ;so init pointers
	LEA	DI,VESAinit
	MOV	DS:[InitDisplayProc],DI
	INC	WORD PTR [NumberModes] ;adj number modes
	MOV	BX,8		;Analog color display type 
	MOV	DS:[DisplayType],BX
	LEA	DI,VESAname
	MOV	DS:[CardNamePtr],DI
	XOR	AX,AX
	JMP	SHORT @DetectExit

;Couldn't find VESA, so fall back on standard VGA
@NotVESA:
	LEA	DI,VGABankSelect ;allow select mode 13H only
	MOV	DS:[BankSelectProc],DI
	LEA	DI,VGA320x200Init
	MOV	DS:[InitDisplayProc],DI
	MOV	BX,64		 ;default window granularity = 64K
	MOV	DS:[WinGran],BX
	MOV	BX,0
	MOV	DS:[ModeSelect],BL  ;def select mode
	INC	WORD PTR DS:[NumberModes] ;adj number modes

	MOV	AX,1A00H
	INT	10H		;check if VGA type display
	CMP	AL,1AH
	JNZ	@NotVGA
	MOV	DS:[DisplayType],BX
	CMP	BL,6		;allow MCGA or VGA support
	JC	@NotVGA
	CMP	BL,0AH		;but not digital MCGA
	JZ	@NotVGA

	LEA	DI,VGAcard
	MOV	DS:[CardNamePtr],DI
	XOR	AX,AX
	JMP	SHORT @DetectExit

;Couldn't find VGA/MCGA, so just error out
@NotVGA:
	MOV	BYTE PTR DS:[ModeErrorFlag],0FFH ;mark as mode error
	MOV	AH,grNoInitGraph ;set error flag
	MOV	DS:[StatBlock.stat],AH
	LEA	DI,DummyCard
	MOV	DS:[CardNamePtr],DI
	MOV	BYTE PTR DS:[ModeSelect],0 ;force mode to 0
	OR	AL,255
@DetectExit:
	OR	AX,AX
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	SI
	POP	DI
	POP	ES
	RET
DetectCard ENDP


;--------------------------------------------------
;code segment placment variables and stuff

VESAmodeSize EQU 6	;number of modes in table
VESAmodes:
	 DW 13H		;0: 320X200X256
	 DW 100H	;1: 640x400x256
	 DW 101H	;2: 640x480x256
	 DW 103H	;3: 800x600x256
	 DW 105H	;4: 1024x768x256
	 DW 107H	;5: 1280x1024x256

MaxModes EQU 6	;maximum available modes we can use
;VESAmode DW 0	;VESA mode we will be using

VESAname DB 7
	 DB 'VESA256',0		;VESA driver name

VGAcard	 DB 8
	 DB 'VGA/MCGA',0	;default VGA id


;----------------------------------------------------------------------
;Set screen mode to desired value
;Assume:  DS = data segment
;Enter:   mode to select in VidMode
;Return:  selected mode; Rets Z if selected, NZ if bad
;Destory: AX,BX

VESAinit PROC NEAR
	PUSH	AX
	PUSH	BX
	MOV	AX,04F02H
	MOV	BX,DS:[VidMode]
	INT	10H
	CMP	AX,004FH	;ret Z if good
	POP	BX
	POP	AX
	RET
VESAinit ENDP

;----------------------------------------------------------------------
;select new display memory bank via Vesa calls
;Assume:  DS = data segment
;Enter:   DX = 64K bank to select
;Return:  Bank is selected 
;Destory: None

VESAbank PROC NEAR
	PUSH	AX
	PUSH	BX
	PUSH	DX
	MOV	AX,DS:[WinGran]
	MUL	DX
	MOV	DX,AX
	PUSH	DX
	MOV	AX,04F05H
	MOV	BX,0
	INT	10H
	POP	DX
	MOV	AX,04F05H
	MOV	BX,1
	INT	10H
	POP	DX
	POP	BX
	POP	AX
	RET
VESAbank ENDP


;----------------------------------------------------------------------
;This processes the Vesa interupt $10 call to manage the protected
;verses real mode TSR calling problem with selectors via the DPMI
;Assume:  DS = data segment
;Enter:   registers as needed
;Return:  registers as needed    C=failed, NC=ok
;Destory: None

VESAInt:
	CMP DS:[CurOpMode],0  	;if CurOpMode is zero
	JNZ @VesaProt	   	;just manage the INT 10 call 
	PUSH DS
	POP ES  	   	;point ES:DI at temp buffer
	INT 10H		   	;normally
	CMP AX,004FH
	JZ @VESAintOK
	STC			;ret carry set if function fails
@VESAintOK:
	RET

;process VESA int $10 call via DPMI protected mode operation
@VesaProt:
	PUSH DS
	POP ES
	PUSH DI
	PUSH ES
	MOV word ptr DS:[SimInt.RealAX],AX ;pass command number 
	MOV word ptr DS:[SimInt.RealCX],CX ;vid mode where used
	MOV word ptr DS:[SimInt.RealDI],DI ;buf adr in DI
	MOV AX,DS:[RealModeDS]
	MOV DS:[SimInt.RealES],AX	;pass the real mode segment
	MOV DS:[SimInt.RealDS],AX
	MOV word ptr DS:[SimInt.RealSS],0	;let DPMI make it's own stack
	MOV word ptr DS:[SimInt.RealSP],0
	MOV word ptr DS:[SimInt.RealXX],0
	MOV word ptr DS:[SimInt.RealXX+2],0
	LEA DI,SimInt
	MOV AX,0300H
	MOV BL,10H
	MOV BH,0
	MOV CX,0
	INT 31H
	POP ES
	POP DI		;returns ES:DI pointing at temp work 
	JC @VesaProtRet ;eek! dpmi call failed
	MOV AX,word ptr DS:[SimInt.RealAX]
	CMP AX,004FH    ;check on vesa call status
	JZ @VesaProtRet ;good, it made it
	STC		;urg. ret carry set on failure
@VesaProtRet:
	RET


;=====================================================================
