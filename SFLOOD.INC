
;SFLOOD.INC - Copyright 1991,1992 Knight Software
;   History:
;       25 May 1991 - first release
;       22 Nov 1992 - adapted for protected mode operation
;	19 Dec 1992 - fixed floodfill so it works now
;                      also added lots of options (see description)
;       03 Jan 1993 - corrected lockup with inverted floodfills
;
;===============================================================
;Floodfill can handle seed fill, or border fill methods.
;This is selected by the SetWriteMode command.
;Normally, the floodfill will select the best fill mode to use. 
;For simple move type fills, a fast fill is selected. For complex fills, 
;it would be the complex (slow) mode. The complex mode uses a border 
;tracer and saves all information on the internal XY stack. The stack 
;is not released until the fill is done. This allows us to know where 
;we've been so that multiple overfills don't happen. This is required 
;for fills that do read/modify/write cycles such as XOR, AND, OR, and NOT.  
;The simplex (fast) fill allows overwrites to happen. It does this by 
;popping unused scan segments from the XY stack. This makes XY stack usage 
;more efficient. Also, the simplex fill method doesn't search the stack 
;for used scan segments (which is why overwrites can happen). 
;A border fill fills everything that is not the border color. 
;A seed fill fills everything that is not the seed color. 
;In compressed mode, the stack storage is reduced by 33%. It does this by 
;not saving the right side of the scan segment. Instead, the right side is 
;recreated at the time the segment is drawn. This slows things down, 
;but it reduces stack usage thus allowing for more complex fills. 
;
; flood fill type bits
;bit
; 0: 0=border fill, 1=seed fill
; 1: 0 <reserved>
; 2: 0=auto fill, 1=complex fill
; 3: 0=normal stack, 1=compressed stack
; 4: 0=draw during search, 1=draw after search (if no error)
; 5: 0=no tracer during scan, 1=show tracer during scan
; 6: 0 (not used)
; 7: 0=fast, 1=slow  (computed by floodfill routine)

XYStackBeg EQU 2*4  ;start pos of XY stack on the main stack
		    ;also marks the end of the prev loc check array

FALSE	EQU 0
TRUE	EQU 0FFH
DEBUG	EQU FALSE

     MAKEFILE EQU FALSE
     SHOWLOC  EQU FALSE
     IF DEBUG
	INCLUDE DEBFILE.INC
     ENDIF

;-------------------------------------------------------------
;Do a floodfill in the current color using the specified X,Y address 
;in AX, BX as the seed point. CL has the border color.
;if FloodFillMode is zero, border fill is done (fill up to border).
;if FloodFillMode is non-zero, seed fill is done (fill only seed color area).
;Assume:  DS = data segment
;Entry:   AX = Seed X
;         BX = Seed Y
;         CL = Border color
;Return:  AL = 0FFH on error, 00H if went ok
;Destroy: AH

DoFloodFill PROC NEAR
	PUSH	ES
	PUSH	BP
	PUSH	DI
	PUSH	SI
	PUSH	DX
	PUSH	CX
	PUSH	BX

	MOV	DS:[PixelX],AX		;save seed addr
	MOV	DS:[PixelY],BX
	MOV	DS:[BorderColor],CL	;save border color
	CALL	GetPixelAddress
	CALL	ReadPixel		;get seed color
	MOV	DS:[SeedColor],AL	;save it for later
	MOV	byte ptr DS:[StatBlock.stat],0 ;init error flag
	MOV	word ptr DS:[XYStackPeak],0    ;init XY stack peak cnt
	MOV	word ptr DS:[XYStackFree],0    ;init XY stack free

	MOV	AH,DS:[FloodFillType]	;get current control flags
	OR	AH,SlowFill		;default to slow fill
	TEST	AH,ComplexFill		;if forced complex fill,
	JNZ	@DoFloodFill1		;ignore this
	MOV	AL,DS:[FillPixelWriteMode] ;check fill write mode
	AND	AL,17H			;if read/modify/write
	JNZ	@DoFloodFill1		;we have to do it in slow mode
	LEA	SI,[FillPattern]
	MOV	CX,8
@DoFloodFill0:
	CMP	byte ptr DS:[SI],0FFH	;simplex fill requires 
	JNZ	@DoFloodFill1		;forground fill color only
	INC	SI			;no pattern or background
	LOOP	@DoFloodFill0		;filling allowed

	;if we made it this far, it is a simplex fill, so allow for it
	AND	AH,0FFH-(DelayedDraw+SlowFill) ;clr draw and slow fill

@DoFloodFill1:
	MOV	AL,DS:[SeedColor]
	MOV	DS:[FloodFillFlags],AH  ;save operating control flags
	TEST	AH,SeedFill
	JNZ	@DoFloodFill2
	CMP	AL,DS:[BorderColor]	;if border fill
	JNZ	@DoFloodFIll3		;we are done if 
	JMP	@FloodFillExit		;seed is border color
@DoFloodFill2:
	TEST	AH,SlowFIll		;if slow fill, we accept
	JNZ	@DoFloodFill3		;any kind of seed fill
	CMP	AL,DS:[FillForeColor]	;if fast seed fill
	JNZ	@DoFloodFill3		;we are done if 
	JMP	@FloodFillExit		;seed is fill color

@DoFloodFill3:
	MOV	word ptr DS:[FillFlipX1],-1 ;init tracer addr
	MOV	word ptr DS:[FillFlipX2],-1
	MOV	word ptr DS:[FillFlipY],-1
	MOV	word ptr DS:[XYStackAdj],6  ;init stack control var
	TEST	byte ptr DS:[FloodFillFlags],StackCompress
	JZ	@DoFloodFill5
	MOV	word ptr DS:[XYStackAdj],4
@DoFloodFill5:

     IF MAKEFILE
	CALL OPENFILE
     ENDIF

	MOV	AX,DS:[StatBlock.BytesPerScanLine] ;alloc whole scan line
	ADD	AX,1000		;allow 1000 extra sp bytes for interrupts
	CMP	SP,AX		;Hows the stack doing?
	JNC	@FloodStackOK	;lots of room yet!

	MOV	byte ptr DS:[StatBlock.stat],grNoFloodMem ;ack! no room! 
	JMP	@FloodFillExit	      		   ;mark error and exit

@FloodStackOK:
	SUB	SP,DS:[StatBlock.BytesPerScanLine] ;alloc whole scan line
	MOV	SI,SS		;ES:SI points to scan line buffer
	MOV	ES,SI
	MOV	SI,SP

	MOV	BP,XYStackBeg          ;init XY stack pointer
	MOV	DS:[XYStackIndex],BP   ;init XY stack R/W pointer
	MOV	DI,0		;init the prev stack with -1
	MOV	AX,-1		;in each of the prev stack records
	MOV	CX,XYStackBeg	;(never visited location)
	REP	STOSW		;assumes ES:DI points at stack
	MOV	word ptr DS:[PrevStackIndex],0  ;init prev stack index

	MOV	DX,DS:[PixelY]	;start at seed XY point
	MOV	BX,DS:[PixelX]
	MOV	CX,BX
	MOV	DI,0C000H	;scan in both directions on start
	CALL	FloodScan	;scan it for stuff to do
	CALL	FloodProc	;just do it
	JC	@FloodFillDone	;don't draw if error

	TEST	byte ptr DS:[FloodFillFlags],DelayedDraw
	JZ	@FloodFillDone	;if delayed draw enabled, 
	CALL	DrawXYStack	;it is now time to draw the XY stack

@FloodFillDone:
	ADD	SP,DS:[StatBlock.BytesPerScanLine] ;deallocate XY stack

@FloodFillExit:
	XOR	AL,AL	;default to no error
	CMP	byte ptr DS:[StatBlock.stat],0
	JZ	@FloodExitOK
	MOV	AL,0FFH	;return error flag
@FloodExitOK:

     IF MAKEFILE
	CALL CLOSEFILE
     ENDIF

	POP	BX
	POP	CX
	POP	DX
	POP	SI
	POP	DI
	POP	BP
	POP	ES
	RET
DoFloodFill ENDP


;-------------------------------------------------------------
;Main flood fill procedure
;Assume:  DS = data segment
;Enter:   ES:SI = ScanLine - points to scan line buffer
;Return:  if error, returns 0F9H in [StatBlock.stat] 
;         flags = C - carray set on error
;		 NC - carry clear if all ok 
;Destroy: AX,BX,CX,DX,DI,BP

FloodProc PROC NEAR
	CALL	GetFlood	;get a flood segment from the stack
	JC	@FloodProcDone	;Nothing left to do

	TEST	byte ptr DS:[FloodFillFlags],TracerOn
	JZ	@FloodProc3	;if tracer is on,
	CALL	UnFlipFillScan	;remove trace from screen
@FloodProc3:
	TEST	byte ptr DS:[FloodFillFlags],StackCompress
	JZ	@FloodProc4	;if stack is compressed, we have to
	CALL	FindRightEdge	;go find the right scan edge first
@FloodProc4:
	TEST	byte ptr DS:[FloodFillFlags],DelayedDraw
	JNZ	@FloodProc5	;if immediate drawing, 
	CALL	DrawFloodLine	;draw the flood segment
@FloodProc5:
	TEST	byte ptr DS:[FloodFillFlags],TracerOn ;if tracer is on,
	JZ	@FloodProc6			      ;show where we are
	CALL	FlipFillScan	;BX=LeftX, CX=RightX, DX=LineY, DI=Dir

@FloodProc6:
     IF SHOWLOC
	CALL	WHEREXY
     ENDIF

	TEST	DI,4000H	;seek upward moving scan?
	JZ	@FloodProc8	;no, try for downward scan
	DEC	DX
	CMP	DX,DS:[ClipY1]	;out of bounds?
	JL	@FloodProc7	;yes, do do this
	PUSH	DI
	MOV	DI,4000H	;mark as upward scan direction
	CALL	FloodScan	;scan it for stuff to do
	POP	DI
@FloodProc7:
	INC	DX

@FloodProc8:
	TEST	DI,8000H	;seek downward moving scan?
	JZ	@FloodProc9	;nope, so we're done
	INC	DX
	CMP	DX,DS:[ClipY2]	;out of bounds?
	JG	@FloodProc9	;yes, don't do this
	PUSH	DI
	MOV	DI,8000H	;mark as upward scan direction
	CALL	FloodScan	;scan it for stuff to do
	POP	DI
@FloodProc9:
	JMP	FloodProc	;keep on truckin'

@FloodProcDone:
	TEST	byte ptr DS:[FloodFillFlags],TracerOn
	JZ	@FloodProcNoTrace ;if tracer is on,
	CALL	UnFlipFillScan	  ;unflip the last pixel
	MOV	word ptr DS:[FillFlipX1],-1
	MOV	word ptr DS:[FillFlipX2],-1 ;init tracer addr
	MOV	word ptr DS:[FillFlipY],-1
@FloodProcNoTrace:
	CMP	byte ptr DS:[StatBlock.stat],0
	JZ	@FloodProcExit
	STC			;return error flag
@FloodProcExit:
	RET

FloodProc ENDP


;---------------------------------------------------------
;Scan the next flood line for stuff to do
;Assume:  DS = data segment
;Enter:   BX = LeftX       - current left edge of flood scan 
;         CX = RightX      - current right edge of flood scan 
;         DX = LineY       - current scan line
;         DI = Dir         - current Y search direction 
;      ES:SI = ScanLinePtr - pointer to scan line buffer
;
;Return:  New floodfill segments to draw on stack
;Destroy: nothing

FloodScan PROC NEAR
	PUSH	BX
	PUSH	CX
	MOV	DS:[LeftX],BX	;save leftX pos
	MOV	DS:[RightX],CX	;save rightX pos
	CALL	ReadScanStart	;get the inital scan read
	MOV	CX,BX		;save current pos in CX
	CALL	Fillable	;if fillable, go find left edge 
	JZ	@FloodScan2	;to the left otherwise try right
@FloodScan1:
	INC	BX
	CMP	BX,DS:[RightX]	;if over boundry, nothing here
	JG	@FloodScanExit
	CALL	Fillable	;keep trying until something happens
	JNZ	@FloodScan1
	MOV	CX,BX
	JMP	@FloodScanRight	;CX and BX now has left edge of scan

;let's try to find the left edge of the scan segment
@FloodScan2:
	DEC	BX
	CMP	BX,DS:[ClipX1]	;if at left edge,
	JL	@FloodScan3	;go find right edge
	CALL	ReadScanLeft	;make sure we have it in memory
	CALL	Fillable	;scan until we find edge
	JZ	@FloodScan2	;CX still has last good right pos
@FloodScan3:
	INC	BX		;restore BX back to valid pixel

;ok, we got a valid left pixel, now find the right pixel of the scan
@FloodScanRight:
	XCHG	BX,CX		;Swap CX/BX and search for right edge
@FloodScan4:
	INC	BX
	CMP	BX,DS:[RightX]	;if we bump into master edge
	JG	@FloodScan6	;go handle seperately
	CALL	Fillable
	JZ	@FloodScan4	;keep going until full segment found
	DEC	BX		;make BX valid
	XCHG	BX,CX		;swap the regs back to normal
	CALL	FloodPush	;save it on the stack
	INC	CX
	MOV	BX,CX		;start new search at right edge +1
	JMP	@FloodScan1	;go find the next left edge

@FloodScan6:
	CMP	BX,DS:[ClipX2]	;are we at the right limit?
	JG	@FloodScan8	;yup, so go save it and we're done
	CALL	ReadScanRight	;make sure we have it in memory
	CALL	Fillable	;if not fillable
	JNZ	@FloodScan8	;go save it and we're done
	INC	BX
	JMP	@FloodScan6	;keep a tryin'

@FloodScan8:
	DEC	BX		;make BX valid
	XCHG	CX,BX		;swap the regs back to normal
	CALL	FloodPush	;save the scan segment
@FloodScanExit:
	POP	CX
	POP	BX
	RET
FloodScan ENDP


;---------------------------------------------------------
;Assume:  DS    = data segment
;Enter:   ES:SI = points to scan line buffer
;         BX    = buffer index (X) 
;	  DX    = scan line number (Y)
;Return:  flags = Z - ok to write,  NZ - not ok to write
;Destory: Nothing

Fillable PROC NEAR
	PUSH	AX
	TEST	byte ptr DS:[FloodFillFlags],SeedFill
	JNZ	@SeedFillCheck	      ;seed file type?

	MOV	AL,ES:[SI+BX]	      ;is this a border color?
	CMP	AL,DS:[BorderColor]
	JZ	@NotFillable
	TEST	byte ptr DS:[FloodFillFlags],SlowFill
	JNZ	@IsFillable
	CMP	AL,DS:[FillForeColor] ;how about the fill color?
	JZ	@NotFillable
@IsFillable:
	XOR	AL,AL		      ;nope, so ok to fill it
	POP	AX
	RET

@NotFillable:
	OR	AL,0FFH		      ;its not a fillable pixel
	POP	AX
	RET

@SeedFillCheck:
	MOV	AL,ES:[SI+BX]
	CMP	AL,DS:[SeedColor]      ;is this a seed color?
	POP	AX
	RET
Fillable ENDP



;-----------------------------------------------------------
;Push a flood line segment on the XY stack
;Assume:  DS = data segment
;Enter:   BP = XY stack - index into XY flood stack
;         BX = LeftX    - left edge of scan line flood segment
;         CX = RightX   - right edge of scan line flood segment
;	  DX = LineY    - scan line number
;	  DI = Dir      - direction of Y movement : 4000H=up, 8000H=down
;Return:  BP = XY stack - new XY flood stack index 
;      flags = NC - values pushed on stack ok
;              C  - nothing pushed (stack overflow)
;Destroy: nothing

FloodPush PROC NEAR
	ADD	BP,1000		;check on stack condition
	CMP	SP,BP		;allow 1000 bytes for interrupts
	JNC	@FloodPushStart	;it's ok, so go push the data
	MOV	byte ptr DS:[StatBlock.stat],grNoFloodMem ;ack! no room! 
	SUB	BP,1000		;restore XY stack pointer
	STC			;return carry set on error
	RET

@FloodPushStart:
	SUB	BP,1000		;restore XY stack ptr to normal
	PUSH	DI		;presume default dir search
	CMP	BX,DS:[LeftX]	;over left edge?
	JL	@FloodPush1	;if so, search both dir
	CMP	CX,DS:[RightX]	;over right edge?
	JLE	@FloodPush2	;if so, search both dir
@FloodPush1:
	MOV	DI,0C000H	;0C000H = both dir search
@FloodPush2:
	CALL	PushCheck	;Have we been here before?
	JZ	@FloodPushExit	;yes, so don't do this
	PUSH	DX
	OR	DX,DI		;mix dir with LineY to save stack space
	MOV	SS:[BP+0],DX	;save XL,XR,Y,Dir on the stack
	MOV	SS:[BP+2],BX	;BX=LeftX    CX=RightX
	TEST	byte ptr DS:[FloodFillFlags],StackCompress
	JNZ	@FloodPush4	;DX=LineY    DI=Dir
	MOV	SS:[BP+4],CX	;don't push CX if compressed
@FloodPush4:
     IF MAKEFILE
	CALL WRITEFILE		;copy stack info to file for debugging
     ENDIF
	POP	DX
	ADD	BP,DS:[XYStackAdj] ;adjust stack up
	MOV	DS:[XYStackIndex],BP
	CMP	BP,DS:[XYStackPeak]
	JC	@FloodPushExit
	MOV	DS:[XYStackPeak],BP ;save peak stack level
	MOV	DI,SP
	SUB	DI,1000
	SUB	DI,BP
	MOV	DS:[XYStackFree],DI ;save stack free space
@FloodPushExit:
	POP	DI
	OR	DI,DI		;make sure carry flag is clear
	RET			;for good return
FloodPush ENDP


;------------------------------------------------------------
;check if we've been to this scan before. For fast fill, we
;use a short previous stack to prevent lockups. For slow fill
;we have to track everything, so we watch the XY stack instead.
;Enter:  BX = LeftX - left edge of scan line flood segment
;	 DX = LineY - scan line number
;        DI = Dir   - direction of Y movement : 4000H=up, 8000H=down
;Return: flags:  Z= we've been there
;		NZ= never seen that one before
;Destroy: nothing

PushCheck PROC NEAR
	TEST	byte ptr DS:[FloodFillFlags],SlowFill
	JNZ	StackCheck	 ;check XY stack if slow fill
	OR	DI,DI		 ;non-direction scan, so don't push it
	RET			 ;rets NZ if directional, Z if not

StackCheck:
	PUSH	BP		   ;slow stack check 
	PUSH	AX

@StackCheckNext:
	SUB	BP,DS:[XYStackAdj] ;check if anything 
	CMP	BP,XYStackBeg	   ;is on the XY stack
	JC	@StackCheckDone	   ;nope, so we're done
@StackCheckLoop:
	CMP	SS:[BP+2],BX	   ;ScanX - have we been here?
	JNZ	@StackCheckNext	   ;nope, so try next pos
	MOV	AX,SS:[BP+0]	   ;LineY - have we been here?
	AND	AX,03FFFH	   ;if so, we're done
	CMP	AX,DX		   ;else try the next pos
	JNZ	@StackCheckNext	   ;nope, so try next pos

@StackCheckExit:
	XOR	AX,AX		   ;we found something so ret Z
	POP	AX
	POP	BP
	RET

@StackCheckDone:
	OR	AX,1		   ;nothing found so ret NZ
	POP	AX
	POP	BP
	RET

PushCheck ENDP


;-----------------------------------------------------------
;Get a flood line segment from the XY stack
;Assume:  DS = data segment
;Enter:   BP = XY stack  - index into XY flood stack
;	  [XYStackIndex] - read index into XY stack
;Return:  BX = LeftX     - left edge of flood scan line 
;	  CX = RightX    - right edge of flood scan line (uncompressed) 
;	  DX = LineY     - flood scan line 
;	  DI = Dir       - direction of Y movement to search
;	  BP = XY stack  - new XY flood stack index 
;	  [XYStackIndex] - new XY stack read index 
;      flags = NC - valid value returned
;              C  - nothing returned (stack is empty)
;Destroy: nothing (in compressed CX is invalid)

GetFlood PROC NEAR
	TEST	byte ptr DS:[FloodFillFlags],SlowFill
	JZ	FloodPop

;Read the stack, but leave it intact (slow fill mode)
XYStackRead:
	PUSH	SI
	MOV	SI,DS:[XYStackIndex]
@XYStackLoop1:
	SUB	SI,DS:[XYStackAdj] ;check on condition of XY stack
	CMP	SI,XYStackBeg
	JC	@XYStackReadExit ;its empty Jim
	MOV	DX,SS:[SI+0]	 ;DX=LineY     DI=Dir
	MOV	DI,DX		 ;DI = Dir - direction of Y movement
	AND	DI,0C000H	 ;8000H = downward, 0C000H = both dir
	JZ	@XYStackLoop1	 ;4000H = seek upward movement, 0=neither
	MOV	DS:[XYStackIndex],SI ;update stack read index
	AND	DX,3FFFH
	MOV	SS:[SI+0],DX	 ;update dir flags on stack
	MOV	BX,SS:[SI+2] 	 ;pop XL,XR,Y,Dir off the stack
	MOV	CX,SS:[SI+4]	 ;BX=LeftX     CX=RightX
@XYStackReadExit:
	POP	SI
	RET			 ;returns carry set on empty stack

;pop a level off the stack - adjs stack ptr (fast fill mode)
FloodPop:
	SUB	BP,DS:[XYStackAdj] ;check on condition of XY stack
	CMP	BP,XYStackBeg
	JC	@FloodPopExit	;its empty Jim
	MOV	CX,SS:[BP+4] 	;pop XL,XR,Y,Dir off the stack
	MOV	BX,SS:[BP+2]	;BX=LeftX     CX=RightX
	MOV	DX,SS:[BP+0]	;DX=LineY     DI=Dir
	MOV	DI,DX		;DI = Dir - direction of Y movement
	AND	DX,3FFFH	;4000H = seek upward movement
	AND	DI,0C000H	;8000H = downward, 0C000H = both dir
@FloodPopExit:
	RET			;returns carry set on empty stack

GetFlood ENDP

;-----------------------------------------------------------
;find the right edge of the flood scan line
;Assume:  DS = data segment
;Enter:   BX = LeftX  - left edge of flood scan line 
;	  DX = LineY  - scan line to read 
;Return:  BX = LeftX  - left edge of flood scan line 
;	  CX = RightX - right edge of flood scan line
;      flags = NC - no carry if data found 
;               C - carry set on error
;Destroy: nothing

FindRightEdge PROC NEAR
	MOV	CX,BX
	CALL	ReadScanStart    ;init right scan pos
	CALL	Fillable
	JZ	@FindRightEdge1
	STC
	RET

@FindRightEdge1:
	INC	BX		 ;we presume at least one pixel here
	CMP	BX,DS:[ClipX2]	 ;if at left edge we're done
	JG	@FindRightEdge2
	CALL	ReadScanRight	 ;make sure data is in buffer
	CALL	Fillable	 ;if fillable, go find right edge 
	JZ	@FindRightEdge1  ;else we are done
@FindRightEdge2:
	DEC	BX		 ;restore BX
	XCHG	BX,CX		 ;swap regs to proper places
	RET
FindRightEdge ENDP


;---------------------------------------------------------
;Read an initial scan line segment from the display 
;Assume:  DS = data segment
;Enter:	  BX = LextX  - left edge of scan segment
;	  CX = RightX - right edge of scan segment
;	  DX = LineY  - scan line to read 
;Return:  Scan line buffer is updated with new scan line data
;Destroy: nothing

ReadScanStart PROC NEAR
	CALL	ReadScan	      ;read the indicated segment
	MOV	DS:[ScanLeftX],BX     ;mark which segment we have read
	MOV	DS:[ScanRightX],CX
	RET
ReadScanStart ENDP


;---------------------------------------------------------
;Read a scan line segment from the display to the left of normal
;Assume:  DS = data segment
;Enter:	  BX = LextX - left edge of scan segment
;	  DX = LineY - scan line to read 
;Return:  Scan line buffer is updated with new scan line data
;Destroy: nothing

ReadScanLeft PROC NEAR
	CMP	BX,DS:[ScanLeftX]	;is it in memory already?
	JGE	@ReadScanLeftExit	;yep, so no need to get it
	PUSH	CX
	PUSH	BX
	MOV	CX,DS:[ScanLeftX]	;gotta go get the data
	SUB	BX,8			;grab it in eight pixel chunks
	CMP	BX,DS:[ClipX1]		;or to the left limit
	JGE	@ReadScanLeft2		;whichever is first
	MOV	BX,DS:[ClipX1]
@ReadScanLeft2:
	CALL	ReadScan		;go read the additional data
	MOV	DS:[ScanLeftX],BX	;save new location
	POP	BX
	POP	CX
@ReadScanLeftExit:
	RET
ReadScanLeft ENDP


;---------------------------------------------------------
;Read a scan line segment from the display to the right of normal
;Assume:  DS = data segment
;Enter:	  BX = RightX - right edge of scan segment
;	  DX = LineY  - scan line to read 
;Return:  Scan line buffer is updated with new scan line data
;Destroy: nothing

ReadScanRight PROC NEAR
	CMP	BX,DS:[ScanRightX]	;is it in memory already?
	JLE	@ReadScanRightExit	;yep, so no need to get it
	PUSH	CX
	PUSH	BX
	MOV	CX,BX
	MOV	BX,DS:[ScanRightX]	;gotta go get the data
	ADD	CX,8			;grab it in eight pixel chunks
	CMP	CX,DS:[ClipX2]		;or to the left limit
	JLE	@ReadScanRight2		;whichever is first
	MOV	CX,DS:[ClipX2]
@ReadScanRight2:
	CALL	ReadScan		;go read the additional data
	MOV	DS:[ScanRightX],CX	;save new location
	POP	BX
	POP	CX
@ReadScanRightExit:
	RET
ReadScanRight ENDP


;---------------------------------------------------------
;Read a scan line segment from the display 
;Assume:  DS = data segment
;Enter:	  BX = LextX  - left edge of scan segment
;	  CX = RightX - right edge of scan segment
;	  DX = LineY  - scan line to read 
;Return:  Scan line buffer is updated with new scan line data
;Destroy: nothing

ReadScan PROC NEAR
	PUSH	SI
	PUSH	DI
	PUSH	CX
	PUSH	BX
	PUSH	AX
	ADD	SI,BX		     ;index into the scan buffer
	SUB	CX,BX		     ;determine how many pixels to read
	INC	CX
	MOV	AX,CX	             ;set the length
	MOV	CX,BX		     ;start at LeftX position
	MOV	BX,1		     ;read only one scan line
	LEA	DI,ReadBitMap	     ;on scan line Y (reg DX)
	CALL	DoBitMap 	     ;returns scan line in ES:SI
	POP	AX
	POP	BX
	POP	CX
	POP	DI
	POP	SI
	RET
ReadScan ENDP


;---------------------------------------------------------
;Draw flood fill line on screen
;Assume:  DS = data segment
;Enter:	  BX = LeftX     - left edge of scan line to be drawn
;	  CX = RightX    - right edge of scan line to be drawn
;	  DX = LineY     - current scan line to draw 
;Return:  nothing
;Destroy: nothing

DrawFloodLine PROC NEAR
	PUSH	DI
	PUSH	BX
	PUSH	CX
	SUB	CX,BX		 ;write between BX and CX
	INC	CX
	MOV	AX,CX		 ;AX = width 
	MOV	CX,BX		 ;start at left edge mark
	MOV	BX,1		 ;write only one scan line
	LEA	DI,WriteFillLine ;on scan line Y (reg DX)
	CALL	DoBitMap
	POP	CX
	POP	BX
	POP	DI
	RET
DrawFloodLine ENDP


;---------------------------------------------------------
;Draw flood fill segments sitting on the XY stack
;Assume:  DS = data segment
;Enter:	  flood fill segment list is on the XY stack
;Return:  nothing
;Destroy: nothing

DrawXYStack PROC NEAR
	CALL	FloodPop	 ;get a scan segment from stack 
	JC	@DrawXYStackDone ;stack is empty
	TEST	byte ptr DS:[FloodFillFlags],StackCompress
	JZ	@DrawXYStack1	;if stack is compressed, we have to
	CALL	FindRightEdge	;go find the right scan edge first
@DrawXYStack1:
	CALL	DrawFloodLine	 ;draw the segment
	JMP	DrawXYStack	 ;do it until it hurts
@DrawXYStackDone:
	RET
DrawXYStack ENDP


;------------------------------------------------------------
;Flip the pixel at the address passed. Used by Tracer
;to show tracing pixel while searching the display.
;Assume:  DS = data segment
;Enter:   BX = LextX  - left edge of scan segment being processed
;	  CX = RightX - right edge of scan segment
;	  DX = LineY  - scan line being processed 
;Return:  (screen pixel in inverted)
;Destroy: nothing

FlipFillScan PROC NEAR
	PUSH	AX
	PUSH	BX
	MOV	DS:[FillFlipY],DX
	MOV	DS:[FillFlipX1],BX
	MOV	DS:[FillFlipX2],CX
	MOV	DS:[PixelY],DX
	MOV	DS:[PixelX],BX
	CALL	GetPixelAddress
	CALL	ReadPixel
	MOV	byte ptr DS:[OldFlipColors],AL
	NOT	AL
	CALL	WritePixel

	CMP	BX,CX
	JZ	@FlipFillScan1
	INC	word ptr DS:[PixelX]
	CALL	GetPixelAddress
	CALL	ReadPixel
	MOV	byte ptr DS:[OldFlipColors+1],AL
	NOT	AL
	CALL	WritePixel
@FlipFillScan1:

	POP	BX
	POP	AX
	RET
FlipFillScan ENDP


;------------------------------------------------------------
;UnFlip the pixel at the address passed. Used by Tracer
;to restore old pixel before writing new one.
;Assume:  DS = data segment
;Enter:   nothing
;Return:  (old pixel is restored)
;Destroy: nothing

UnFlipFillScan PROC NEAR
	PUSH	AX
	PUSH	BX
	MOV	BX,DS:[FillFlipX1]
	CMP	BX,-1		;if never used, don't unflip
	JZ	@UnFlipFillExit
	MOV	DS:[PixelX],BX
	MOV	BX,DS:[FillFlipY]
	MOV	DS:[PixelY],BX
	CALL	GetPixelAddress
	MOV	AL,byte ptr DS:[OldFlipColors]
	CALL	WritePixel

	MOV	BX,DS:[FillFlipX2]
	CMP	BX,DS:[FillFlipX1]
	JZ	@UnFlipFillExit
	INC	word ptr DS:[PixelX]
	CALL	GetPixelAddress
	MOV	AL,byte ptr DS:[OldFlipColors+1]
	CALL	WritePixel
@UnFlipFillExit:
	POP	BX
	POP	AX
	RET
UnFlipFillScan ENDP


;------------------------------------------------------------


;@PushCheck1:
;	PUSH	SI		 ;used by fast fill
;	PUSH	DI		 ;check the prev loc stack 
;	MOV	DI,0		 ;to see if we've been here recently
;@PushCheckLoop:
;	CMP	SS:[DI+0],DX
;	JNZ	@PushCheckNext	 ;lineY the same?
;	CMP	SS:[DI+2],BX
;	JZ	@PushCheckExit	 ;ScanX the same?
;@PushCheckNext:
;	ADD	DI,4		 ;not the same
;	CMP	DI,XYStackBeg	 ;so adj the stack and try again
;	JC	@PushCheckLoop	 ;nothing found, so we're done
;
;	MOV	DI,DS:[PrevStackIndex]
;	ADD	DI,4		 ;adjust prev stack pointer
;	CMP	DI,XYStackBeg
;	JC	@PushCheckStuff	 ;if stack overflowed
;	MOV	DI,0		 ;wrap pointer to zero
;@PushCheckStuff:
;	MOV	DS:[PrevStackIndex],DI
;	MOV	SS:[DI+0],DX	 ;stuff current location 
;	MOV	SS:[DI+2],BX	 ;onto the prev stack
;	INC	DI
;@PushCheckExit:
;	POP	DI
;	POP	SI
;	RET


