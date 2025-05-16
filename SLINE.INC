;SLINE.INC - Copyright 1991,1992 Knight Software
;    History:
;        17 May 1991 - first release
;        22 Nov 1992 - adapted for protected mode operation
;
;--------------------------------------------------
;put a pixel on the screen in the indicated color for line type 
;drawing. Assumes that the address and segment have been preset
;Assume: DS = data segment
;Entry:  N/A
;Return: N/A
DrawLinePixel PROC NEAR
	PUSH	ES
	PUSH	DI
	PUSH	BX
	PUSH	CX
	PUSH	AX
	MOV	DI,DS:[PixelAddress] ;get the address to write
	MOV	ES,DS:[VideoSegment] ;video is at segment 0a000h
	MOV	CL,DS:[LinePixelCount]
	INC	BYTE PTR DS:[LinePixelCount]
	AND	CL,0FH
	MOV	BX,DS:[LinePattern]
	SHR	BX,CL
	MOV	AL,DS:[DrawForeColor] ;Get foreground color to plot
	MOV	AH,DS:[DrawBackColor] ;Get background color to plot
	AND	BL,01H		   ;set zero flag for call
	CALL	WORD PTR DS:[LinePixelProc] ;write to video memory
	POP	AX
	POP	CX
	POP	BX
	POP	SI
	POP	ES
	RET
DrawLinePixel ENDP

;-----------------------------------------------------
;line drawing procedure - draw a line from x1,y1 to x2,y2
;Assume: DS = data segment
;Entry:  AX = X1  start coordinate
; 	 BX = Y1  start coordinate
;	 CX = X2  end coordinate
;	 DX = Y2  end coordinate
;Return: N/A

PlotLine PROC NEAR
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	SI
	PUSH	DI
	PUSH	ES
	MOV	DS:[PixelX2],CX	; save x2
	MOV	DS:[PixelY2],DX	; save y2
	MOV	DS:[PixelX],AX	; save	x
	MOV	DS:[PixelY],BX	; save	y
	MOV	BYTE PTR DS:[LinePixelCount],0 ;clr pixel cnt
	CALL	GetPixelAddress ; plot first point
	CALL	DrawLinePixel	; plot x,y,color

	MOV	DI,-1		; if x1 = x2 then
	CMP	AX,CX		;   xstep = 0
	JZ	Draw1		; else
	JG	Draw2		;   if x1 > x2 then
	INC	DI		;	xstep = -1
Draw1:	INC	DI		;    else
Draw2:	MOV	DS:[PlotStepX],DI	;	xstep = 1

	MOV	SI,-1		; if y1 = y2 then
	CMP	BX,DX		;   ystep = 0
	JZ	Draw3		; else
	JG 	Draw4		;   if y1 > y2 then
	INC	SI		;   	ystep = -1
Draw3:	INC	SI		;   else
Draw4:	MOV	DS:[PlotStepY],SI	;	ystep = 1

; check for special case x1=x2 and y1=y2 => xstep = ystep = 0
	OR	SI,DI		; si or di <> 0
	JZ	Plexit		; and exit

Draw5:	SUB	DX,BX		; deltay = abs(y2-y1)
	JNS	Draw6
	NEG	DX		; dx = deltay

Draw6:	SUB	CX,AX		; deltax = abs(x2-x1)
	JNS	Draw7
	NEG	CX		; cx = deltax

Draw7:
;	mov	di,-1		; di = direction
;	or	cx,cx		; if deltax = 0
;	jz	draw8		; then direction = -1
;	xor	di,di		; else direction = 0

	XOR	DI,DI		; assume vert slope to start
	MOV	SI,DX
	SUB	SI,CX		; compute the angle
	JNS	Draw8		; positive = vertical
	DEC	DI		; negative = horizontal

Draw8:	MOV	DS:[PlotSlope],SI	; save slope of line
	MOV	DS:[PlotDeltaX],CX	; save delx
	MOV	DS:[PlotDeltaY],DX	; and dely

Drawlp:	MOV	SI,DS:[PlotStyle]	; get plot style
	AND 	SI,03H		; strip off garbage
	OR	DI,DI		; if direction = 0
	JNS	Drawld
	ADD	BX,DS:[PlotStepY]	; then y = y + ystep
	ADD	DI,DS:[PlotDeltaX]	;	dir := dir + deltax
	JNS	Drawlx		; if dir >= 0
	CMP	SI,1		;   or lines > 0
	JGE	Drawlx		; then plot x,y
	JMP	Drawll

Drawld:	ADD	AX,DS:[PlotStepX]	; else x = x + xstep
	SUB	DI,DS:[PlotDeltaY]	; 	dir = dir - deltay
	JNS	Drawlx		; if dir >=0
	CMP	SI,2		;	or lines > 1
	JGE	Drawlx		; then plot x,y
	JMP	Drawll

Drawlx:	PUSH	DI		; save	dir
	MOV	DS:[PixelX],AX	; save	x
	MOV	DS:[PixelY],BX	; save	y
	CALL	GetPixelAddress
	CALL	DrawLinePixel	; plot x,y,color
	POP	DI		; restore dir

Drawll:	CMP	BX,DS:[PixelY2]	; if y <> y2 then goto drawlp
	JNZ	Drawlp
	CMP	AX,DS:[PixelX2]	; if x <> x2 then goto drawlp
	JNZ	Drawlp
Plexit:	POP	ES
	POP	DI
	POP	SI
	POP	DX
	POP	CX
	POP	BX
	POP	AX
	RET

PlotLine ENDP

