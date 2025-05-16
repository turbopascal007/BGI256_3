;SPIXEL.INC - Copyright 1991,1992 Knight Software
;    History:
;       17 May 1991 - Original release
;       10 Jun 1992 - Added ability to control pixel writing style
;       22 Nov 1992 - Adapted for protected mode operation 
;
;-----------------------------------------------------------------
;read a pixel from the screen. Returns Color in PixelForeColor
;assumes that the address and segment have been pre-set
;Assume: DS = data segment
;Entry:  N/A
;Return: AL = color read

ReadPixel PROC	NEAR
	PUSH	ES
	PUSH	SI
	MOV	SI,DS:[PixelAddress]   ;get the address to read
	MOV	ES,DS:[VideoSegment]   ;video is at segment 0a000h
	MOV	AL,ES:[SI]             ;read from video memory
	POP	SI
	POP	ES
	RET
ReadPixel ENDP


;--------------------------------------------------
;write a pixel on the screen in the indicated color in reg AL
;assumes that the address and segment have been preset
;Assume:  DS = data segment
;Entry:   AL = Color
;Return:  N/A
;Destory: nothing

WritePixel PROC	NEAR
	PUSH	ES
	PUSH	DI
	MOV	DI,DS:[PixelAddress]   ;get the address to write
	MOV	ES,DS:[VideoSegment]   ;video is at segment 0a000h
	MOV	ES:[DI],AL	       ;write to video memory
	POP	DI
	POP	ES
	RET
WritePixel ENDP


;--------------------------------------------------
;put a pixel on the screen in the indicated color
;assumes that the address and segment have been preset
;DrawPixelProc controls how the pixel is drawn
;Assume: DS = data segment
;Entry:  AL = drawing color
;Return: N/A

DrawPixel PROC	NEAR
	PUSH	ES
	PUSH	DI
	PUSH	AX
	MOV	DI,DS:[PixelAddress]   ;get the address to write
	MOV	ES,DS:[VideoSegment]   ;video is at segment 0a000h
	MOV	AH,DS:[PixelBackColor] ;get the background color
	OR	AL,AL		       ;drawing color is in AL
	CALL	WORD PTR DS:[DrawPixelProc] ;write to video memory
	POP	AX
	POP	DI
	POP	ES
	RET
DrawPixel ENDP


