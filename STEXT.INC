;STEXT.INC - Copyright 1991,1992 Knight Software
;    History:
;       17 May 1991 - first release
;       08 Feb 1992 - fixed text position off by one
;       22 Nov 1992 - adapted for protected mode operation
;
;------------------------------------------------------
;Update internal text registers with system 8x8 font info
;Assume: DS = data segment
;Enter:  Nothing
;Return: Initializes text information 
GetTextInfo PROC NEAR
	PUSH	ES
	PUSH	BP
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	DS:[FontSizeX],8
	MOV	DS:[FontSizeY],8
	MOV	DS:[CharSizeX],8
	MOV	DS:[CharSizeY],8
	MOV	DS:[FontMultX],1
	MOV	DS:[FontMultY],1
	POP	AX
	POP	BX
	POP	CX
	POP	BP
	POP	ES
	RET
GetTextInfo ENDP

;-----------------------------------------------------------
;Read a character from font table
;Assume: DS = data segment
;Enter:  AL = Char to read
;Return: Character map is in TempChar

ReadChar PROC NEAR
	PUSH	ES
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX

	CALL	GetFontChar   ;point ES:SI at font bitmap for char
	MOV	AX,ES:[SI]
	MOV	BX,ES:[SI+2]  ;get character font from bios rom
	MOV	CX,ES:[SI+4]
	MOV	DX,ES:[SI+6]

	LEA	DI,TempChar	;point to temp char array
	CMP	DS:[FontDir],1
	JZ	@Rotate1
	CMP	DS:[FontDir],2
	JZ	@Rotate2
	CMP	DS:[FontDir],3
	JZ	@Rotate3
	MOV	DS:[DI],AX
	MOV	DS:[DI+2],BX	;copy character to temp vars 
	MOV	DS:[DI+4],CX	;as is since no rotate needed
	MOV	DS:[DI+6],DX
	JMP	@RotateDone

@Rotate1:
	PUSH	BP	;rotate the character to the
	MOV	BP,8	;right 90 degrees
@Cloop1:
	SHL	DH,1	;if rotate required then rotate it
	RCL	SI,1	;as we stuff it to the local stack
	SHL	DL,1
	RCL	SI,1
	SHL	CH,1
	RCL	SI,1
	SHL	CL,1
	RCL	SI,1
	SHL	BH,1
	RCL	SI,1
	SHL	BL,1
	RCL	SI,1
	SHL	AH,1
	RCL	SI,1
	SHL	AL,1
	RCL	SI,1
	MOV	DS:[DI],SI	;copy char to temp vars
	INC	DI
	DEC	BP
	JNZ	@Cloop1
	POP	BP
	JMP	@RotateDone

@Rotate2:
	PUSH	BP	;rotate the character to the 
	MOV	BP,8	;left 90 degrees
@Cloop2:
	SHR 	AL,1	;if rotate required then rotate it
	RCL 	SI,1    ;as we stuff it to the local stack
	SHR 	AH,1
	RCL 	SI,1
	SHR 	BL,1
	RCL 	SI,1
	SHR 	BH,1
	RCL 	SI,1
	SHR 	CL,1
	RCL 	SI,1	;rotate in the other direction
	SHR 	CH,1	;@@@ this probably does not work,  @@@
	RCL 	SI,1	;@@@ I haven't checked it yet      @@@
	SHR 	DL,1
	RCL 	SI,1
	SHR 	DH,1
	RCL 	SI,1
	MOV 	DS:[DI],SI	;copy char to local stack
	INC 	DI
	DEC 	BP
	JNZ 	@Cloop2
	POP 	BP
	JMP	@RotateDone

@Rotate3:
	XCHG	DH,AL	;flip the character upside down
	XCHG	DL,AH
	XCHG	CH,BL
	XCHG	CL,BH
	MOV	DS:[DI],AX
	MOV	DS:[DI+2],BX	;copy character 
	MOV	DS:[DI+4],CX	;to local stack array
	MOV	DS:[DI+6],DX
	JMP	@RotateDone

@RotateDone:
        POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	ES
	RET
ReadChar ENDP

;----------------------------------------------------------
;Draw a character on the screen at given location
;Assume: DS = data segment
;Entry:  AL = Character to draw
;        BX = X location
;        DX = Y location
;Return: N/A

DrawChar PROC NEAR
	PUSH	ES
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	DS:[PixelX1],BX	;store temp x location
	MOV	DS:[PixelY1],DX	;store temp y location
	CALL	ReadChar	;Read the character map 
	MOV	ES,DS:[VideoSegment] ;point ES at video segment
	MOV	DH,0
	LEA	SI,TempChar
@CharLp:
	MOV	DL,DS:[FontMultY]
@YmultLp:
	MOV	AX,DS:[PixelX1]
	MOV	DS:[PixelX],AX
	MOV	AL,DH
	MUL	BYTE PTR DS:[FontMultY]
	ADD	AL,DL
	ADC	AH,0
	DEC	AX
	ADD	AX,DS:[PixelY1]
	MOV	DS:[PixelY],AX
	CALL	GetPixelAddress
	MOV	DI,DS:[PixelAddress]
	MOV	CH,DS:[CharSizeX]
	MOV	BH,DS:[SI]
	MOV	AL,DS:[DrawForeColor] ;text fore uses drawcolor
	MOV	AH,DS:[TextBackColor] ;backcolor is its own color
@PixLoop:
	MOV	CL,DS:[FontMultX]
@Nxtpix:
	TEST	BH,80H
	CALL	WORD PTR DS:[TextPixelProc]
	INC	WORD PTR DS:[PixelX]
	INC	DI
	JNZ	@AdrOK
	CALL	GetPixelAddress
	MOV	DI,DS:[PixelAddress]
@AdrOK:
	DEC	CL
	JNZ	@Nxtpix
	SHL	BH,1
	DEC	CH
	JNZ	@PixLoop

	DEC	DL
	JNZ	@YmultLp
	INC	SI
	INC	DH
	CMP	DH,DS:[CharSizeY]
	JNZ	@CharLp

	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	ES
	RET
DrawChar ENDP

;-----------------------------------------------------------
;Draw text on the screen
;Assume: DS = data segment
;Entry:  ES:BX = Pointer to string to draw
;           CX = String size in charcters
;Return: N/A 

DrawText PROC NEAR
	PUSH	DI
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	MOV	DI,BX	
	MOV	BX,DS:[CPX]    ;get current pointer
	MOV	DX,DS:[CPY]
@DrawTextLp:
	MOV	AL,ES:[DI]  ;Get a char to draw
	CALL	DrawChar    ;Draw the character
	INC	DI	    ;Point to next char in string

	MOV	AH,0	       ;Preset AH to zero
	CMP	DS:[FontDir],1 ;Adjust CP based on 
	JZ	@DrawText1     ;Text Direction selected
	CMP	DS:[FontDir],2
	JZ	@DrawText2
	CMP	DS:[FontDir],3
	JZ	@DrawText3
	JMP	@DrawText0

@DrawText0:
	MOV	AL,DS:[FontSizeX] ;PixelX+FontSize - 0 degrees
	ADD	BX,AX
	JMP	@DrawTextDone

@DrawText1:
	MOV	AL,DS:[FontSizeY] ;PixelY+FontSize - 90 degrees
	ADD	DX,AX
	JMP	@DrawTextDone

@DrawText2:
	MOV	AL,DS:[FontSizeY] ;PixelY+FontSize - 180 degrees
	ADD	DX,AX
	JMP	@DrawTextDone

@DrawText3:
	MOV	AL,DS:[FontSizeX] ;PixelX+FontSize - 270 degrees
	ADD	BX,AX
	JMP	@DrawTextDone

@DrawTextDone:
        LOOP	@DrawTextLp ;loop til done
	POP	DX
	POP	CX
	POP	BX
	POP	AX
	POP	DI
	RET
DrawText ENDP



;-----------------------------------------------------------
;This will get the font char from the system font tables
;Warning: The font table resides in real mode memory, 
;*not* in protected mode memory, even if you are in protected 
;mode operation. To change the font table, you can load 
;a new font tsr at int $1f ahead of time. Alternately,
;if you are in real mode, you can just point the interrupt 
;at your own font table. If you are in protected mode, you
;must first allocate some real mode memory with GlobalDosAlloc
;then load the new font info to that area, finally, you must 
;call the DMPI service (0201H) to change the real mode int vector. 
;Assume: DS    = data segment
;Entry:  AL    = Character font to get
;Return: ES:SI = points to start of font bitmap 

GetFontChar Proc NEAR
	XOR	AH,AH
	MOV	SI,AX
	SHL	SI,1
	SHL	SI,1
	SHL	SI,1
	MOV	BX,word ptr DS:[ChrGen]
	MOV	CX,word ptr DS:[ChrGen+2]   ;Get Table address
	OR	CX,BX			    ;from int 1f
	JZ	@NoVect			    ;if no vect found
	MOV	ES,word ptr DS:[ChrGen+2]   ;use only system font
	CMP	word ptr ES:[BX+1016],494DH ;check if this is
	JNZ	@NotMyn			    ;our font table
	CMP	word ptr ES:[BX+1018],454BH
	JZ	@GotMyn
@NotMyn:
	CMP	AL,80H			 ;not our font, so use
	JC	@GotLow			 ;system font tables
@GotMyn:
	SUB	SI,0400H		 ;it's our special font
	ADD	SI,BX			 ;table, so address it
	RET				 ;as such

@NoVect:
	CMP	AL,80H			 ;if no ext font show 
	JC	@GotLow			 ;only a space char
	MOV	BX,0100H		 ;else map lower font
@GotLow:
	ADD	SI,0FA6EH		 ;from bios font table
	MOV	ES,DS:[SEGF000]
	RET
GetFontChar ENDP


;Note: Normally, the lower 128 chars are not redefinable.
;This can be a problem. The upper 128 chars are redefinable 
;by loading a new font table pointed at by int $1f. 
;This doesn't affect the lower 128 characters however. 
;Because of this, I adapted the font detection mechanism to 
;look for a special signature in the character position #255.
;if that character has the sequence $4D $69 $6B $65 in the 
;first four bytes, it is assumed that the lower 128 character
;fonts have been added to the end of the regular upper 128 
;character font table. I added it to the end so that it would 
;not confuse other software that is not aware of the extension.
;thus allowing it to use the upper font table even though the
;lower font table exists beyond the end of the upper font table.
;Huh? (You can unglaze your eyes now. <g>)

;-----------------------------------------------------------

