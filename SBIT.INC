;SBIT.INC - Copyright 1991,1992 Knight Software
;  History:
;     17 May 1991 - First release 
;     22 Nov 1992 - Adapted for protected mode
;     15 Dec 1992 - Fixed bug in WriteLineFill 
;                    non-pattern fill was always doing MoveWrite
;
;-----------------------------------------------------------
;Assume:  DS = data segment
;Entry:   DX = Count - number bytes to write
;Return:  N/A
;Destroy: None

WriteClear PROC  NEAR
	OR	DX,DX		     ;if nothing to write stop this
	JNZ	@WriteClearEnter
	RET

@WriteClearEnter:
	PUSH	ES
	PUSH	DI
	PUSH	CX
	PUSH	AX
	MOV	ES,DS:[VideoSegment] ;video is at segment 0a000h
	MOV	DI,DS:[PixelAddress] ;destination offset address
	MOV	AL,DS:[InitColor]    ;preload init color
	MOV	CX,DX		     ;copy length in bytes to CX
	SHR	CX,1		     ;and convert to words
	JNC	@WriteClearEven      ;if odd count do the odd byte
	STOSB			     ;ES:DI = dest array		  
	JZ	@WriteClearDone      ;nothing else to write if done
@WriteClearEven:
	MOV	AH,AL		     ;extend color for word write
	REP	STOSW		     ;fast write to the screen
@WriteClearDone:
	POP	AX
	POP	CX
	POP	DI
	POP	ES
	RET
WriteClear ENDP

;-----------------------------------------------------------
;Assume:  DS       = data segment
;Entry:   DX       = Count - number bytes to write
;         [PixelX] = offset into scan line to fill (X)
;         [PixelY] = scan line to fill (Y)
;Return:  N/A
;Destroy: None

WriteFillLine PROC  NEAR
	OR	DX,DX		     ;if nothing to write stop this
	JNZ	@WriteFillLineEnter
	RET

@WriteFillLineEnter:
	PUSH	DS
	PUSH	ES
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX

	MOV	ES,DS:[VideoSegment]  ;video is at segment 0a000h
	MOV	DI,DS:[PixelAddress]  ;destination offset address
	LEA	SI,FillPattern        ;point to fill pattern table
	MOV	AX,DS:[PixelY]
	AND	AX,07H
	ADD	SI,AX		      ;can we do it fast?
	MOV	AL,DS:[FillForeColor] ;preload foreground color
	MOV	AH,DS:[FillBackColor] ;preload background color

	MOV	CL,DS:[FillPixelWriteMode] ;preload fill write mode
	TEST	CL,03H		      ;not a fast type fill
	JNZ	@WriteSlowFillLine    ;so we gotta do it the slow way
	MOV	CH,DS:[SI]	      ;get pattern to use
	INC	CH		      ;if pattern = 0FFH, do fast
	JZ	@WriteForeFastFillLine
	DEC	CH		      ;if pattern = 00H, do fast
	JZ	@WriteBackFastFillLine;if not fast pattern, do it slow

@WriteSlowFillLine:
	MOV	CX,DS:[PixelX]	      ;gotta do it slow 
	MOV	CH,8		      ;get PixelX location
	AND	CL,07H		      ;and sync pattern to it
	SUB	CH,CL
	MOV	BL,DS:[SI]
	SHL	BL,CL
	MOV	CL,BL		      ;copy pattern to CL
;	MOV	AL,DS:[FillForeColor] ;preload foreground color
;	MOV	AH,DS:[FillBackColor] ;preload background color
	MOV	BX,DS:[FillPixelProc] ;load pixel calling routine

@WriteAltFillLineLp:
	TEST	CL,80H		      ;set flags for call
	CALL	BX		      ;via table proc pointer
	INC	DI		      ;and loop till done
	DEC	DX
	JZ	@WriteFillLineDone
	SHL	CL,1
	DEC	CH		      ;do next pattern bit
	JNZ	@WriteAltFillLineLp
	MOV	CL,DS:[SI]	      ;reload pattern byte
	MOV	CH,8		      ;and count length
	JMP	SHORT @WriteAltFillLineLp ;and do again

@WriteForeFastFillLine:		   ;AH:AL has color to write
	TEST	CL,04H
	JZ	@WriteForeFastFill2
	NOT	AL		   ;invert color if NOT write
@WriteForeFastFill2:
	TEST	CL,10H		   ;ok to write foreground?
	JZ	@WriteFastFillLine ;yes, go do it
	JMP	@WriteFillLineDone ;nope, so nothing to do

@WriteBackFastFillLine:		   ;AH:AL has color to write
	TEST	CL,04H
	JZ	@WriteBackFastFill2
	NOT	AH		   ;invert color if NOT write
@WriteBackFastFill2:
	MOV	AL,AH		   ;we will be writing back color
	TEST	CL,08H		   ;ok to write background?
	JZ	@WriteFastFillLine ;yes, go do it
	JMP	@WriteFillLineDone ;nope, so do nothing

@WriteFastFillLine:
	MOV	CX,DX		   ;copy length in bytes to CX
	SHR	CX,1		   ;and convert to words
	JNC	@WriteFillLineEven ;if odd count do the odd byte
	STOSB			   ;ES:DI = dest array		  
	JZ	@WriteFillLineDone ;nothing else to write if done
@WriteFillLineEven:
	MOV	AH,AL		   ;extend color for word write
	REP	STOSW		   ;fast fill write to the screen
@WriteFillLineDone:
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	ES
	POP	DS
	RET
WriteFillLine ENDP

;-----------------------------------------------------------
;Assume:   DS    = data segment
;Entry:    DX    = Count - number bytes to read
;	   ES:SI = Pointer to cpu memory 
;Return:   N/A
;Destorys: None

ReadBitMap PROC  NEAR
	OR	DX,DX
	JNZ	@ReadBitMapEnter     ;if nothing to read stop this
	RET

@ReadBitMapEnter:
	PUSH	DS
	PUSH	SI
	PUSH	DI
	PUSH	CX
	MOV	CX,DX
	MOV	DI,SI
	MOV	SI,DS:[PixelAddress] ;DS:SI = source array
	MOV	DS,DS:[VideoSegment] ;video is at segment 0a000h
	SHR	CX,1		     ;ES:DI = dest array
	JNC	@ReadBitMapEven
	MOVSB			     ;if odd count do the odd byte
	JZ	@ReadBitMapDone	     ;nothing else to draw if done
@ReadBitMapEven:
	REP	MOVSW		     ;read it from the screen
@ReadBitMapDone:
	POP	CX
	POP	DI
	POP	SI
	POP	DS
	RET
ReadBitMap ENDP

;-----------------------------------------------------------
;Assume: DS    = data segment
;Entry:  DX    = Count - number bytes to write
;	 ES:SI = Pointer to cpu memory 

WriteBitMap PROC  NEAR
	OR	DX,DX
	JNZ	@WriteBitMapEnter       ;if nothing to write stop this
	RET

@WriteBitMapEnter:
	PUSH	DS
	PUSH	ES
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	CX,DX
	MOV	AL,DS:[BitMapPixelWriteMode]
	MOV	AH,DS:[BitMapBackColor]  ;preload background color
	MOV	BX,DS:[BitMapPixelProc]
	MOV	DI,DS:[PixelAddress]     ;destination offset address
	PUSH	AX
	MOV	AX,ES
	MOV	ES,DS:[VideoSegment]     ;video is at segment 0a000h
	MOV	DS,AX		         ;DS:SI = source array
	POP	AX
	OR	AL,AL
	JZ	@WriteBitMapFast         ;if mode zero, use fast move
@WriteAltBitMapLp:
	MOV	AL,DS:[SI]	  ;get next pixel from buffer
	CMP	AL,AH		  ;is this bitmap background color?
	CALL	BX		  ;if other style write mode (ES:DI^)
	INC	SI		  ;use long call process
	INC	DI		  ;via table proc pointer
	LOOP	@WriteAltBitMapLp ;loop til done
	JMP	SHORT @WriteBitMapDone
@WriteBitMapFast:
	SHR	CX,1		 ;ES:DI = dest array
	JNC	@WriteBitMapEven ;DS:SI = source array
	MOVSB			 ;if odd count do the odd byte
	JZ	@WriteBitMapDone ;if nothing else to write all done
@WriteBitMapEven:
	REP	MOVSW		;fast block write to the screen
@WriteBitMapDone:
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	POP	ES
	POP	DS
	RET
WriteBitMap  ENDP

;-------------------------------------------------------
;Bit map read/write entry code
;BitMapProc ptr = pointer to read/write procedure to call
;To write blockfill, call as: LEA DI,WriteFillLine; CALL DoBitMap
;To write bit map, call as:   LEA DI,WriteBitMap;   CALL DoBitMap
;To read bit map, call as:    LEA DI,ReadBitMap;    CALL DoBitMap
;To clear bit map, call as:   LEA DI,WriteClear;    CALL DoBitMap
;Assume:   DS    = data segment
;Enter:    ES:SI = cpu memory pointer (not needed for WriteFillLine)
;	   DS:DI = operation procedure pointer
;          CX    = start X location
;          DX    = start Y location
;          AX    = image width (x size)
;          BX    = image height (y size)
;Return:   N/A
;Destroys: None

DoBitMap PROC NEAR
	PUSH	ES
	PUSH	DI
	PUSH	SI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX

	MOV	DS:[BitMapProc],DI ;save proc ptr
	MOV	DS:[PixelX2],AX
	MOV	DS:[PixelX1],CX	   ;CX= PixelX1
	MOV	DI,DX		   ;DI= PixelY1
	MOV	CX,BX

@DoBitMapLp:
	MOV	DS:[PixelY],DI		   ;get this scan line #
	MOV	AX,DS:[PixelX1]
	MOV	DS:[PixelX],AX	   	   ;start at left edge 

	MOV	DX,DS:[PixelX2]
	CALL	GetPixelAddress		   ;locate the start adr
	CMP	DX,DS:[PixelSegmentLength] ;if larger than segment
	JC 	@SingleBitScan		   ;split the scan line op
	MOV	DX,DS:[PixelSegmentLength] ;do the first part
	CALL	WORD PTR DS:[BitMapProc]   ;of the scan line
	MOV	AX,DS:[PixelSegmentLength] ;start where we left off
	ADD	DS:[PixelX],AX		   ;on the screen
	MOV	DX,DS:[PixelX2]		   ;reduce count by previous
	SUB	DX,AX			   ;amount processed
	ADD	SI,AX			   ;adj cpu memory pointer
	CALL	GetPixelAddress		   ;get the new start adr
@SingleBitScan:
	CALL	WORD PTR DS:[BitMapProc]   ;process the scan line
	ADD	SI,DX		           ;adj cpu memory pointer
	INC	DI			   ;adj PixelY pointer
	LOOP	@DoBitMapLp		   ;loop til all are done

	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	SI
	POP	DI
	POP	ES
	RET
DoBitMap ENDP


