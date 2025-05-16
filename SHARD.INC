;SHARD.INC - Copyright 1991,1992 Knight Software
; History:
;  17 May 1991 - first release
;  14 Jul 1991 - Corrected default line draw from MoveByte to ForeByte
;  20 Aug 1992 - fixed scan line length init problem 
;  22 Nov 1992 - Adapted for protected mode operation
;
;**********************************************************************
;Misc init stuff this is common VGA initialization code
;It sets up various variables and tables in the system
;Assume:  DS = data segment
;Entry:   Nothing
;Return:  (Stuff is initialized)
;Destroys Nothing

MiscInit PROC NEAR
	PUSH	ES
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	PUSHF

	MOV	BX,0FFH			;Init bank select reg
	MOV	DS:[PixelSegment],BX

	MOV	AL,0
	MOV	DS:[ModeXflag],0	;clr modex flag
	MOV	AL,DS:[ModeSelect]
	AND	AX,7
	LEA	BX,StatusPtrTable	;copy mode status table
	ADD	BX,AX
	ADD	BX,AX
	MOV	SI,DS:[BX]
	MOV	CX,StatBlockSize
	LEA	DI,StatBlock
	PUSH	DS
	POP	ES
	REP	MOVSB

	MOV	BX,DS:[ScanLineLength]
	OR	BX,BX			;if scan line length not
	JNZ	@MiscInitB		;initialized, fix it up
	MOV	BX,DS:[StatBlock.BytesPerScanLine]
	MOV	DS:[ScanLineLength],BX
@MiscInitB:
	LEA	BX,MoveByte
	MOV	DS:[FillPixelProc],BX	;preset proc ptrs
	MOV	DS:[BitMapPixelProc],BX
	LEA	BX,ForeByte
	MOV	DS:[LinePixelProc],BX
	MOV	DS:[TextPixelProc],BX
	MOV	AL,0
	MOV	DS:[FillPixelWritemode],AL	;preset mode selects
	MOV	DS:[BitMapPixelWritemode],AL
	MOV	AL,8
	MOV	DS:[LinePixelWritemode],AL
	MOV	DS:[TextPixelWritemode],AL

	MOV	AL,DS:[StatBlock.ctblf]	;init colors
	MOV	DS:[DrawForeColor],AL
	MOV	DS:[FillForeColor],AL
	MOV	AL,0
	MOV	DS:[DrawBackColor],AL
	MOV	DS:[PixelBackColor],AL
	MOV	DS:[FillBackColor],AL
	MOV	DS:[TextBackColor],AL
	MOV	DS:[BitMapBackColor],AL

	CALL	GetTextInfo	;Load text info from BIOS

	MOV	AX,1012H
	MOV	BX,0		;Set default palette table
	MOV	CX,256
	LEA	DX,PaletteTable
	PUSH	DS
	POP	ES  	   	;point ES:DI at pal table
	CALL	SetPal

	POPF
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	ES
	RET
MiscInit ENDP

;**********************************************************************
;Create mode string from mode name and card name
;Assume: DS    = data segment
;Entry:  CL    = Mode number for name
;Return: ES:BX = pointer to mode string
;        NC = valid mode; C= bad mode number
GetModeName PROC NEAR
	PUSH	SI
	PUSH	DI
	PUSH	CX
	PUSH	DX
	LEA	DI,ModeNameBuffer
	INC	DI		 ;Point DI to target buffer
	MOV	CH,0
	MOV	DX,0	         ;init count to zero

	PUSH	DS	;return a pointer to a pascal string ES:BX
	POP	ES	;of the mode name for the mode passed in CX
	CMP	BYTE PTR DS:[ModeErrorFlag],0
	JZ	@GetModeName1
	PUSH	CX
	LEA	SI,ModeNameBad
	MOV	CL,DS:[SI]	    ;get length
	ADD	DL,CL
	INC	SI
	REP	MOVSB
	POP	CX

@GetModeName1:
	LEA	SI,ModeNameError    ;Beyond mode number allowed?
	CMP	CX,DS:[NumberModes]
	JNC	@GetModeName2
	LEA	SI,ModeName
	MOV	AL,ModeNameSize     ;get table entry size
	MUL	CL
	ADD	SI,AX		    ;index into table for name

@GetModeName2:
	MOV	CL,DS:[SI]	    ;get mode name length
	INC	SI
	ADD	DL,CL		    ;save count
	REP	MOVSB		    ;copy mode name to buffer

	MOV	SI,DS:[CardNamePtr] ;get card name buffer address
	MOV	CL,CS:[SI]	    ;get length of card name
	INC	SI		    ; (card name is in code segment)
	ADD	DL,CL		    ;add to overall name length
	CMP	DL,ModeNameMax      ;make sure mode name doesn't
	JC	@GetModeName3	    ;overflow buffer
	MOV	DL,ModeNameMax
@GetModeName3:
	MOV	AL,CS:[SI]
	INC	SI
	MOV	ES:[DI],AL	    ;add card name to mode name
	INC	DI
	LOOP	@GetModeName3
	MOV	BYTE PTR ES:[DI],0  ;tag with a null
	LEA	BX,ModeNameBuffer   ;return pointer to name string 
	MOV	ES:[BX],DL	    ;set length

	POP	DX
	POP	CX
	POP	DI
	POP	SI
	RET
GetModeName ENDP


;---------------------------------------------------------------
;Default dummy card name (code segment placement)

DummyCard	DB 12
		DB 'Unknown card',0


;**********************************************************************
;Take the pixel position specified by PixelX, PixelY and the
;scan line length specified by BytesPerScanLine and compute
;the long address into the display. Selects proper segment bank
;on the display and stuffs bank select into PixelSegment and 
;the Display Offset address into PixelAddress. Returns how many 
;bytes are left in the display segment from the specified address
;in PixelSegmentLength.
;Assume:  DS       = data segment
;Entry:   [PixelX] = scan line offset (see description above)
;         [PixelY] = scan line number 
;Return:  [PixelAddress] = (see description abive)
;Destroy: None

GetPixelAddress PROC NEAR
	PUSH	AX
	PUSH	DX
	PUSH	SI
	MOV	AX,DS:[ScanLineLength]   ;bytes per scan line
	MOV	DX,DS:[PixelY]           ;PixelY
	MUL	DX                    ;dx:ax=beginning of scan line
	MOV	SI,AX
	ADD	SI,DS:[PixelX]           ;si=address
	ADC	DX,0                  ;dx=bank
	CMP	DS:[PixelSegment],DX     ;set the bank select
	JE	@DoneBankSelect
	MOV	DS:[PixelSegment],DX
	CALL	DS:[BankSelectProc]   ;call approprate bank sel code
@DoneBankSelect:
	MOV	DS:[PixelAddress],SI     ;save the pixel address
	NOT	SI
	INC	SI
	JNZ	@BankSelectOK
	DEC	SI		;limit length to 65535
@BankSelectOK:
	MOV	DS:[PixelSegmentLength],SI  ;save the segment length
	POP	SI
	POP	DX
	POP	AX
	RET
GetPixelAddress ENDP


;**********************************************************************
;Standard VGA only has 320x200 which fully fits in 64K bank.
;Thus no selection needs to be done here.
;Assume:  DS = Nothing
;Entry:   N/A
;Return:  N/A
;Destroy: None
VGABankSelect PROC NEAR
	RET
VGABankSelect ENDP


;----------------------------------------------------------------------
;Select standard VGA320x200 mode operation
;Assume:  DS = data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX

VGA320x200Init PROC NEAR
	MOV	AX,0013H	;select standard mode
	INT	10H		;call BIOS to set the mode
	RET
VGA320x200Init ENDP


;----------------------------------------------------------------------
;Set EGA Pal color register in AL
;Assume:  DS = data segment
;Entry:   AL = color index
;         BL = color number
;Return:  palette is updated
;Destroy: AX,BX

SetEGAPal PROC NEAR
	PUSH	AX
	PUSH	BX
	MOV	BH,BL	;set color palette
	MOV	BL,AL
	MOV	AX,1000H
	INT	10H
	POP	BX
	POP	AX
	RET
SetEGAPal ENDP


;----------------------------------------------------------------------
;Set full EGA color palette
;Note: although the manual says that the overscan register is not
;set with this function, in reality, it is passed in the array
;(it appears to be passed as a zero value). BI's SetAllPalette
;works like this too (ie it passed the overscan value at the end
;of the palette array). Thus there are actually 17 bytes passed.
;Note that while the Get Default Palette function (see Query Color)
;requires a length byte in the start of the array, this function is 
;intended to be feed to the BIOS directly, so there is no length
;byte. It is assumed that the array size is already known by the 
;mode you are currently in. I only support full EGA palette here.
;Assume:  DS    = data segment
;Entry:   ES:BX = points to palette table data
;Return:  palette is updated
;Destroy: AX,BX,DX

SetFullEGAPal PROC NEAR
	PUSH	ES
	PUSH	DI
	PUSH	CX
	PUSH	BX
	PUSH	AX

	LEA	DI,TempWork	;copy the palette stuff
	MOV	CX,17		;to local real mode memory
@EGAPalCopy:
	MOV	AL,ES:[BX]
	MOV	DS:[DI],AL
	INC	DI
	INC	BX
	LOOP	@EGAPalCopy

	PUSH	DS
	POP	ES
	LEA	DX,TempWork
	MOV	AX,1002H
	CALL	SetPal

	POP	AX
	POP	BX
	POP	CX
	POP	DI
	POP	ES
	RET
SetFullEGAPal ENDP


;----------------------------------------------------------------------
;Set VGA Pal color register in AX
;Assume:  DS = data segment
;Entry:   AX = color index
;         BX = red
;         CX = green
;	  DX = blue
;Return:  palette is updated
;Destroy: AX,BX,CX,DX

SetVGAPal PROC NEAR
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	MOV	CH,CL	;set an individual DAC color register
	MOV	CL,DL	;BX=red, CX=green, DX=blue, AX=index
	MOV	DH,BL	;top two bits of AX = 10
	MOV	DL,0
	MOV	BX,AX
	AND	BX,00FFH
	MOV	AX,1010H
	INT	10H
	POP	DX
	POP	CX
	POP	BX
	POP	AX
	RET
SetVGAPal ENDP


;----------------------------------------------------------------------
;process Pal set int $10 call via DPMI protected mode operation
;Assume:  DS = data segment
;Entry:   AX = command
;         BX = pal start index
;         CX = items to change (count)
;Return:  palette is updated
;Destroy: All

SetPal	PROC NEAR
	CMP	DS:[CurOpMode],0  	;if CurOpMode is zero
	JNZ	@PalProt	   	;just do a real INT 10 call 

	INT	10H
	RET

@PalProt:
	MOV word ptr DS:[SimInt.RealAX],AX ;pass command number 
	MOV word ptr DS:[SimInt.RealBX],BX
	MOV word ptr DS:[SimInt.RealCX],CX
	MOV word ptr DS:[SimInt.RealDX],DX
	MOV AX,DS:[RealModeDS]
	MOV DS:[SimInt.RealES],AX	   ;pass the real mode segment
	MOV DS:[SimInt.RealDS],AX
	MOV word ptr DS:[SimInt.RealSS],0  ;let DPMI make it's own stack
	MOV word ptr DS:[SimInt.RealSP],0
	MOV word ptr DS:[SimInt.RealXX],0
	MOV word ptr DS:[SimInt.RealXX+2],0
	MOV AX,DS
	MOV ES,AX
	LEA DI,SimInt	;go simulate the interrupt call
	MOV AX,0300H
	MOV BL,10H
	MOV BH,0
	MOV CX,0
	INT 31H
	RET
SetPal	ENDP


;----------------------------------------------------------------------
