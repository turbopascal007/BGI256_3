;SBASE.INC - Copyright 1992 Knight Software
; History:
;  30 Nov 1992 - Adapted for protected mode operation
;                 split off of main to simplify configuration 
;  22 Dec 1992 - Changed SetWriteMode to add floodfill options.

;**********************************************************************
;
;--- INSTALL ----------------------------------------------------------
;Driver initialization and installation
;The Install function is used to prepare the driver for use. 
;
;The calls to this function allow the kernal to inquire the 
;mode information, and allow the kernal to install the mode 
;infomation.                                                
;Assume:  DS = data segment
;Entry:   AL = 00	CL = Mode number
;Return:  ES:BX = Pointer to Device Status Table
;
;Entry:	  AL = 01	CL = N/A
;Return:  CX = Number of modes supported (last mode+1)
;
;Entry:	  AL = 02	CL = Mode number
;Return:  ES:BX = Pointer to Mode name (Pascal type String)
;Destory: None

_install PROC	NEAR
	PUSH	SI
	PUSH	DI
	PUSH	AX
	PUSH	DX
	CMP	AL,0
	JNZ	@Install1 	;Install device command 
	CMP	CL,MaxModes	;if greater than Max # modes
	JC	@installa	;then force default mode
	MOV	CL,DefaultMode	;else use selected value
@installa:
	MOV	WORD PTR DS:[ScanLineLength],0 ;clear scan line length
	MOV	DS:[ModeSelect],CL	;save requested mode
	PUSH	DS
	POP	ES		;Point ES:BX to status block
	LEA	BX,StatBlock
	CALL	DetectCard	;determine what card we have
	CALL	MiscInit	;initalize misc variables
	JMP	@InstallDone

@Install1:	; Mode Query Command
	CMP	AL,1
	JNZ	@Install2
	CMP	BYTE PTR DS:[ModeXflag],0
	JNZ	@Installm
	MOV	CX,DS:[NumberModes]  ;return number of modes available
	JMP	@InstallDone
@Installm:
	MOV	CX,DS:[RetValue]     ;return alternate selected value
	INC	CX		     ;pre-distort the result
	JMP	@InstallDone

@Install2:	; Mode Name Command
	CMP	AL,2
	MOV	AH,grError	 ;Bad cmd
	JNZ	@Installx
	CALL	GetModeName
	MOV	AH,grInvalidMode ;Bad mode number
	JNC	@Installx
	JMP	@InstallDone

@Installx:	; Bad Install Exit
	MOV	DS:[StatBlock.stat],AH	;save error
@Installdone:
	POP	DX
	POP	AX
	POP	DI
	POP	SI
	RET
_install ENDP


;--- INIT -------------------------------------------------------------
;Initialize device for output 
;Assume:  DS = data segment
;Entry:   ES:BX points to Device Information Table 
;                        (not Device Status Table)
;Return:  N/A
;Destory: None

_init	PROC	NEAR
	PUSH	AX
	PUSH	CX
	MOV	CH,ES:[BX]	;get InitColor
	MOV	CL,ES:[BX+1]	;Get Init flag
	CMP	CL,65H		;if 65H don't init
	JZ	@InitExit
	MOV	DS:[InitColor],CH	  ;save init color to use
	CALL	word ptr DS:[InitDisplayProc] ;call the selected init code
	JZ	@InitExit
	MOV	AH,grInvalidMode  ;bad mode
	MOV	DS:[StatBlock.stat],AH
@InitExit:
	POP	CX
	POP	AX
	RET
_init	ENDP


;--- CLEAR ------------------------------------------------------------
;Clear graphics device (screen) and ready it for new output. 
;Assume:  DS = data segment
;Entry:   N/A
;Return:  N/A
;Destroy: None

_clear	PROC	NEAR
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	CX,0		      ;Start clear at 0,0
	MOV	DX,0
	MOV	AX,DS:[StatBlock.BytesPerScanLine] ;Clear full width of screen
	MOV	BX,DS:[StatBlock.TotalScanLines]   ;Clear full height of screen
	LEA	SI,FillPattern	      ;DS:SI = start of cpu memory 
	LEA	DI,WriteClear         ;point DI at proc to use
	CALL	DoBitMap	      ;Copy cpu memory to screen
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	RET
	RET
_clear	ENDP


;--- POST -------------------------------------------------------------
;Post screen. This is required for devices that need to be posted.
;Since the IBM displays are always visible, there is no need to
;do anything here.
;Assume:  DS = data segment
;Entry:   N/A
;Return:  N/A
;Destory: None

_post	PROC	NEAR
	RET
_post	ENDP


;--- MOVE -------------------------------------------------------------
;Set the master Current Pointer to coordinates passed in AX, BX.
;Assume:  DS = data segment
;Entry:   AX = X  
;         BX = Y 
;Return:  N/A
;Destory: None

_move   PROC  NEAR
	MOV	DS:[CPX],AX
	MOV	DS:[CPY],BX
	RET
_move	ENDP


;--- DRAW -------------------------------------------------------------
;Draw a line from the master Current Pointer to the 
;coordinates passed in AX,BX.
;Assume:   DS = data segment
;Entry:    AX = X
;          BX = Y 
;Return:   N/A
;Destroys: None

_draw	PROC	NEAR
	PUSH	CX
	PUSH	DX
	MOV	CX,DS:[CPX]
	MOV	DX,DS:[CPY]
	CALL	PlotLine
	POP	DX
	POP	CX
	RET
_draw	ENDP


;--- VECT -------------------------------------------------------------
;Draw a line between the coordinates passed in AX, BX, CX, DX.
;Assume:   DS = data segment
;Entry:    AX = X Start
;          BX = Y Start
;          CX = X End
;          DX = Y End
;Return:   N/A
;Destroys: None

_vect	PROC	NEAR
	CALL	PlotLine
	RET
_vect	ENDP


; @EM --- BAR ---------------------------------------------------------
;Draw a filled rectangle with the CP as the Lower Left corner, 
;and the coordinate in AX, BX as the upper right.
;Assume:   DS = data segment
;Entry:    AX = X upper corner  
;          BX = Y upper corner
;          CX = Depth for 3D bars
;          DX = Draw Top Flag (DX<>0 = Draw Top)
;Return:   N/A
;Destroys: None

;_bar  PROC  NEAR                   	 ; @@@@@@                    
;	RET                         	 ; this function is emulated 
;_bar	ENDP                        	 ; @@@@@@                    


;--- PATBAR -----------------------------------------------------------
;Draw a patterned box at the passed coordinates in AX, BX, CX, DX. 
;The pattern is provided by the information previously passed to 
;the set fill pattern function.
;Assume:   DS = data segment
;Entry:    AX = X1 Corner  
;          BX = Y1 Corner
;          CX = X2 Corner  
;          DX = Y2 Corner
;Return:   N/A
;Destroys: None

_patbar	PROC	NEAR
	PUSH	SI
	PUSH	DI
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
	CMP	AX,CX		;insure that start X is in CX
	JNC	@PatBar1
	XCHG	AX,CX
@PatBar1:
	CMP	BX,DX		;insure that start Y is in DX
	JNC	@PatBar2
	XCHG	BX,DX
@PatBar2:
	SUB	AX,CX		;put X size in AX
	INC	AX
	SUB	BX,DX		;put Y size in BX
	INC	BX
	LEA	SI,FillPattern	 ;DS:SI = start of cpu memory 
	LEA	DI,WriteFillLine ;point DI at proc to use
	CALL	DoBitMap	 ;Copy cpu memory to screen
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DI
	POP	SI
	RET
_patbar	ENDP


; @EM --- ARC ---------------------------------------------------------
;Draw an elliptical arc from the start angle in AX, 
;to the end angle in BX, using the the CP as the Center Point, 
;and the X and Y Radii in CX, DX. Angles are 0-360 degrees.
;Assume:  DS = data segment
;Entry:   AX = Start Angle  BX = End Angle
;         CX = X radius of arc
;         DX = Y radius of arc
;Return:  N/A
;Destory: None

;_arc  PROC  NEAR                        ; @@@@@@                    
;	RET                              ; this function is emulated 
;_arc	ENDP                             ; @@@@@@                    


; @EM --- PIESLICE ----------------------------------------------------
;Draw an elliptical sector from the start angle in AX,
;to the end angle in BX, using the CP as the Center Point, 
;and the X and Y Radii in CX, DX. Angles are 0-360 degrees.
;Assume:  DS = data segment
;Entry:   AX = Start Angle  BX = End Angle
;         CX = X radius of arc
;         DX = Y radius of arc
;Return:  N/A
;Destory: None

;_pieslice  PROC  NEAR                   ; @@@@@@                    
;	RET                              ; this function is emulated 
;_pieslice ENDP                          ; @@@@@@                    


; @EM --- FILLED ELLIPSE ----------------------------------------------
;Draw an ellipse using the CP as the Center Point, 
;and the X and Y radii passed in AX, BX.
;Assume:  DS = data segment
;Entry:   AX = X Radius of ellipse
;         BX = Y Radius of ellipse
;Return:  N/A
;Destory: None

;_filled_ellipse  PROC  NEAR             ; @@@@@@                    
;	RET                              ; this function is emulated 
;_filled_ellipse ENDP                    ; @@@@@@                    


;--- PALETTE ---------------------------------------------------------- 
;Set a specific palette entry to a given color.
;Assume:  DS = data segment
;Entry:
; Top two bits of AX contain command code.
; 00 : set EGA color - Index in AL, Color in BL
; 01 : not used 
; 10 : set specific VGA palette - BX=red, CX=green, DX=blue, AX=index
; 11 : set EGA background color - BL=Color
;Return:  N/A
;Destory: None

_palette PROC	NEAR
	AND	AH,0C0H
	JZ	@Pal0	;set color value
	CMP	AH,80H
	JZ	@Pal2	;set dac color register
	CMP	AH,40H	;this function not used
	JZ	@PalX
	MOV	AL,0	;set background color
@Pal0:	CALL	SetEGAPal ;Go set the EGA palette
	RET

@Pal2:	CALL	SetVGAPal ;Get set the VGA palette
@PalX:	RET

_palette ENDP


;--- ALL PALETTE ------------------------------------------------------
;Load an EGA palette with a table of colors.
;Assume:  DS    = data segment
;Entry:   ES:BX = EGA Palette Table to load
;Return:  N/A
;Destory: None

_allpalette PROC  NEAR
	CALL	SetFullEGAPal
	RET
_allpalette ENDP


;--- COLOR ------------------------------------------------------------
;Set the palette indexs to the specified drawing color in AL
;and the specified fill color in AH.
;Assume:  DS = data segment
;Entry:   AL = Drawing Color
;         AH = Fill color
;Return:  N/A
;Destroy: None

_color  PROC  NEAR
	MOV	DS:[DrawForeColor],AL
	MOV	DS:[FillForeColor],AH
	RET
_color	ENDP


;--- FILL STYLE -------------------------------------------------------
;Set the fill pattern to the specified pattern number passed in AL.
;If the pattern number is 0FFH, the pattern is define in
;a user supplied array pointed at by ES:BX.
;Assume:  DS    = data segment
;Entry:   AL    = Pattern # (0FFH for user def)
;         ES:BX = User pattern data if needed
;Return:  N/A
;Destroy: None

_fillstyle PROC	NEAR
	PUSH	ES
	PUSH	DI
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	CX,4		 ;Copy four words (eight bytes)
	LEA	DI,FillPattern   ;copy pattern to Fill Pattern array
	CMP	AL,0FFH
	JZ	@UserFill
	PUSH	DS
	POP	ES
	MOV	AH,0	;Adjust for table index
	SHL	AX,1	;Pattern number times eight
	SHL	AX,1
	SHL	AX,1
	LEA	BX,FillTable
	ADD	BX,AX	;ES:BX points into table
@UserFill:
	MOV	AX,ES:[BX] ;Copy pattern to drawing array
	MOV	DS:[DI],AX
	INC	BX
	INC	DI
	INC	BX
	INC	DI
	LOOP	@UserFill
	POP	AX
	POP	BX
	POP	CX
	POP	DI
	POP	ES
	RET
_fillstyle ENDP


;--- LINE STYLE -------------------------------------------------------
;Set the line style to the specified style number passed in AL.
;If the style number is > 3  then the style is define in the user 
;supplied pattern in BX. The line width to use is passed in CX. 
;Note: The calling Graph unit actually does the triple width
;      by drawing the line three times, so don't do it in here. 
;Assume:  DS = data segment
;Entry:   AL = Style # ( >3 for user def)
;         BX = User Line Pattern
;         CX = Line Width (1 or 3)
;Return:  N/A
;Destory: None

_linestyle PROC	NEAR
	PUSH	DI
	PUSH	BX
	PUSH	AX
	MOV	DS:[LineWidth],CL
	CMP	AL,4
	JNC	@LineStyleX
	AND	AX,03H
	ADD	AX,AX
	LEA	DI,LinePatternTable
	ADD	DI,AX
	MOV	BX,DS:[DI]
@LineStyleX:
	MOV	DS:[LinePattern],BX
	POP	AX
	POP	BX
	POP	DI
	RET
_linestyle ENDP


;--- TEXT STYLE -------------------------------------------------------
;Set the text attributes for font rendering
;Assume:  DS = data segment
;Entry:   AL = Font Number
;         AH = Font Path and Direction
;         BX = Desired X Character Size
;         CX = Desired Y Character Size
;Return:  BX = Actual X Character Size
;         CX = Actual Y Character Size
;Destroy: None

_textstyle PROC NEAR
	PUSH	CX
	PUSH	BX
	PUSH	AX
	MOV	DS:[FontNumber],AL
	MOV	DS:[FontDir],AH
	SHR	CL,1
	SHR	CL,1
	SHR	CL,1
	JNZ	@TextStyle1
	INC	CL
@TextStyle1:
	CMP	CL,10
	JL	@TextStyle2
	MOV	CL,10
@TextStyle2:
	MOV	DS:[FontMultX],CL

	SHR	BL,1
	SHR	BL,1
	SHR	BL,1
	JNZ	@TextStyle3
	INC	BL
@TextStyle3:
	CMP	BL,10
	JL	@TextStyle4
	MOV	BL,10
@TextStyle4:
	MOV	DS:[FontMultY],BL

	MOV	AL,DS:[CharSizeX]
	MUL	BL
	MOV	BL,AL
	MOV	BH,0
	MOV	DS:[FontSizeX],BL

	MOV	AL,DS:[CharSizeY]
	MUL	CL
	MOV	CL,AL
	MOV	CH,0
	MOV	DS:[FontSizeY],CL
	POP	AX
	POP	BX
	POP	CX
	RET
_textstyle ENDP


;--- TEXT -------------------------------------------------------------
;Draw a string in the current font with the
;justification point at the CP.
;Assume:  DS    = data segment
;Entry:   ES:BX = Pointer to string
;         CX    = String Length
;Return:  N/A
;Destroy: None

_drawtext	PROC	NEAR
	CALL	DrawText
	RET
_drawtext	ENDP


;--- TEXT SIZE --------------------------------------------------------
;Calculate the dimensions (in pixels) of an input text string.
;Assume:  DS    = data segment
;Entry:   ES:BX = Pointer to string
;         CX    = String Length
;Return:  BX    = Width of string
;         CX    = Height of string
;Destroy: None

_textsize PROC	NEAR
	PUSH	AX
	MOV	AH,0
	MOV	AL,DS:[FontSizeX]
	MUL	CX
	MOV	BX,AX
	MOV	CH,0
	MOV	CL,DS:[FontSizeY]
	POP	AX
	RET
_textsize ENDP

;--- FLOOD FILL -------------------------------------------------------
;Do a floodfill in the current color using the specified X,Y address 
;in AX, BX as the seed point. CL contains the border color.
;Assume:  DS = data segment
;Entry:   AX = Seed X                  
;         BX = Seed Y            ;warning: this flood fill has problems   
;         CL = Border color      ;with complex floods. I still haven't  
;Return:  N/A                    ;fixed it (I'm not sure how to yet).
;Destroy: None

_floodfill PROC NEAR
	MOV	DS:[CPX],AX	;update CPxy
	MOV	DS:[CPY],BX
	CALL	DoFloodFill	;(Located in SFLOOD.INC)
	RET
_floodfill ENDP


;--- GET PIXEL --------------------------------------------------------
;Read a pixel from the coordinates specified in AX, BX.
;Assume:  DS = data segment
;Entry:   AX = X
;         BX = Y 
;Return:  DL = Pixel value read
;Destroy: None

_getpixel PROC NEAR
	PUSH	AX
	MOV	DS:[PixelX],AX
	MOV	DS:[PixelY],BX
	CALL	GetPixelAddress
	CALL	ReadPixel	;pixel color returned in AL
	MOV	DL,AL
	POP	AX
	RET
_getpixel ENDP


;--- SET PIXEL --------------------------------------------------------
;Write a pixel to the coordinate specified in AX, BX.
;Assume:  DS = data segment
;Entry:   AX = X
;         BX = Y 
;         DL = Pixel value to write
;Return:  N/A
;Destory: None

_setpixel PROC NEAR
	PUSH	AX
	MOV	DS:[PixelX],AX
	MOV	DS:[PixelY],BX
	CALL	GetPixelAddress
	MOV	AL,DL
	CALL	DrawPixel	;fore color is passed in AL
	POP	AX
	RET
_setpixel ENDP


;--- BIT MAP UTILITIES ------------------------------------------------
;Return a pointer to a table of misc bit map driver utilities. 
;Assume:  DS    = data segment
;Entry:   N/A
;Return:  ES:BX = Base of table
;Destory: None
;The table is configured as follows:
;BitMapTable:  	DW GOTOGRAPHIC
;       	DW EXITGRAPHIC
;       	DW PUTPIX
;       	DW GETPIX
;       	DW BITSPERPIXEL
;       	DW SETPAGE
;       	DW SETVISUAL
;       	DW SETWRITEMODE

_bitmaputil PROC NEAR
	PUSH	DS
	POP	ES
	LEA	BX,BitMapTable
	RET
_bitmaputil ENDP


;--- SAVE BIT MAP -----------------------------------------------------
;Save a portion of the screen to CPU memory.
;Assume:  DS        = data segment
;Entry:   ES:BX     = Pointer to CPU memory buffer
;         CX        = Start X coord of block to save
;         DX        = Start Y coord of block to save
;         ES:[BX]   = Width of block to save
;         ES:[BX+2] = Height of block to save
;Return:  Data in CPU memory buffer starting at ES:[BX+4]
;Destory: None

_savebitmap PROC NEAR
	PUSH	SI
	PUSH	DI
	PUSH	BX
	PUSH	AX
	MOV	SI,BX
	MOV	AX,ES:[SI]	;[SI]= X size (width)
	INC	AX		;Save in AX
	MOV	BX,ES:[SI+2]	;[SI+2]= Y size (height)
	INC	BX		;Save in BX
	ADD	SI,4		;ES:SI = start of cpu memory 
	LEA	DI,ReadBitMap   ;point DI at proc to use
	CALL	DoBitMap	;Copy screen to cpu memory
	POP	AX
	POP	BX
	POP	DI
	POP	SI
	RET
_savebitmap ENDP


;--- RESTORE BIT MAP --------------------------------------------------
;Restore a portion of the screen from CPU memory.
;Assume:  DS        = data segment
;Entry:   ES:BX     = Pointer to CPU memory buffer
;         CX        = Start X coordinate of area to restore
;         DX        = Start Y coordinate of area to restore
;         ES:[BX]   = Width of area to restore
;         ES:[BX+2] = Height of area to restore
;         ES:[BX+4] = Data to restore to screen
;         AL        = Write mode to use for restoring the area
;		      (see SetWriteMode procedure for modes allowed)
;Return:  N/A
;Destory: None

_restorebitmap PROC NEAR
	PUSH	SI
	PUSH	DI
	PUSH	BX
	PUSH	AX
	AND	AL,1FH		;strip off any garbage in AL
	MOV	DS:[BitMapPixelWriteMode],AL ;save the mode select
	MOV	AH,0		;index into proc pointer table
	ADD	AX,AX		;to get the proc address
	LEA	SI,DrawModeTable
	ADD	SI,AX
	MOV	DI,DS:[SI]		;get selected proc addr into DI
	LEA	SI,BitMapPixelProc ;save selected draw mode procedure 
	MOV	DS:[SI],DI		;address in the procedure pointer

	MOV	SI,BX
	MOV	AX,ES:[SI]	;[SI]= X size (width)
	INC	AX		;Save in AX
	MOV	BX,ES:[SI+2]	;[SI+2]= Y size (height)
	INC	BX		;Save in BX
	ADD	SI,4		;ES:SI = start of cpu memory 
	LEA	DI,WriteBitMap  ;point DI at proc to use
	CALL	DoBitMap	;Copy cpu memory to screen
	POP	AX
	POP	BX
	POP	DI
	POP	SI
	RET
_restorebitmap	ENDP


;--- SET CLIP ---------------------------------------------------------
;Set the clipping window to the rectangle defined by the 
;the coordinates passed in AX, BX as the upper left corner
;and CX, DX as the lower left corner.
;Note: Clipping per the BGI defintion is actually done in the
;calling graph unit. There's no real need to clip in the BGI driver.
;Assume:  DS = data segment
;Entry:   AX = Upper Left X 
;         BX = Upper Left Y 
;         CX = Lower Right X 
;         DX = Lower Right Y 
;Return:  N/A
;Destory: None

_setclip PROC NEAR
	MOV	DS:[ClipX1],AX
	MOV	DS:[ClipY1],BX
	MOV	DS:[ClipX2],CX
	MOV	DS:[ClipY2],DX
	RET
_setclip ENDP


;--- COLOR QUERY ------------------------------------------------------
;Return the color parameters of the device
;Assume:  DS = data segment
;Entry:   AL = Command for query
;Return: 
;  if AL = 0 : BX = Total colors available
;              CX = Maximum color value (BX minus one)
;  if AL = 1 : ES:BX pointer to default EGA palette 
;Destory: None
 
_color_query PROC NEAR
	CMP	AL,0
	JZ	@Query0	;get max palette
	CMP	AL,1
	JZ	@Query1	;get default palette
	JMP	SHORT @QueryX	;error

@Query0:
	MOV	BH,0
	MOV	BL,DS:[StatBlock.ctblf]
	INC	BX
	MOV	CX,BX
	DEC	CX
	JMP	SHORT @QueryX

@Query1:
	PUSH	DS
	POP	ES
	LEA	BX,EgaPalette
@QueryX:
	RET
_color_query ENDP


;----------------------------------------------------------------------
;35 DUP (NONE)    ; Reserved Entry Points


;**********************************************************************
;WARNING! The data segment (DS) is not correctly setup when BMI code
;is called. Neither is ES. All registers probably should be saved.

;--- BMI ENTER BIT MAP MODE -------------------------------------------
;switch to pixel graphics mode
;Assume:  nothing
;Entry:   N/A
;Return:  N/A
;Destory: None

bmi_gotographic	PROC FAR
	RET		; do nothing
bmi_gotographic	ENDP


;--- BMI EXIT BIT MAP MODE --------------------------------------------
;return to normal graphics mode
;Assume:  nothing
;Entry:   N/A
;Return:  N/A
;Destory: None

bmi_exitgraphic	PROC FAR
	RET		; do nothing
bmi_exitgraphic	ENDP


;--- BMI SET PIXEL ----------------------------------------------------
;Fast version of Set Pixel. 
;Assume:  nothing
;Entry:   AX = X
;         BX = Y 
;         DL = Pixel value to write
;Return:  N/A
;Destory: None

bmi_putpixel	PROC FAR
	PUSH	DS
     IF BGIVERSION LT 3
	PUSH	CS
	POP	DS
     ELSE
	MOV	DS,CS:[ALIAS]
     ENDIF
	CALL	_setpixel   ;See _setpixel for more information
	POP	DS 
	RET		    ;since we only have one setpixel version
bmi_putpixel	ENDP


;--- BMI GET PIXEL ----------------------------------------------------
;Fast version of Get Pixel. 
;Assume:  nothing
;Entry:   AX = X
;         BX = Y 
;Return:  DL = Pixel value read
;Destory: None

bmi_getpixel	PROC FAR
	PUSH	DS
     IF BGIVERSION LT 3
	PUSH	CS
	POP	DS
     ELSE
	MOV	DS,CS:[ALIAS]
     ENDIF
	CALL	_getpixel   ;See _getpixel for more information
	POP	DS 
	RET		    ;since we only have one getpixel version
bmi_getpixel	ENDP


;--- BMI BITS PER PIXEL -----------------------------------------------
;Return how many bits are in a pixel
;Assume:  nothing
;Entry:   N/A
;Return:  AX = number of bits used to define a pixel
;Destory: None

bmi_bitsperpixel PROC FAR
	PUSH	DS
     IF BGIVERSION LT 3
	PUSH	CS
	POP	DS
     ELSE
	MOV	DS,CS:[ALIAS]
     ENDIF
	MOV	AL,DS:[StatBlock.PixelBits]
	MOV	AH,0
	POP	DS
	RET
bmi_bitsperpixel ENDP


;--- BMI SET PAGE -----------------------------------------------------
;Draw the page passed in AL
;Assume:  nothing
;Entry:   AL = Page to draw
;Return:  N/A
;Destory: None

bmi_setpage	PROC FAR
	RET			;no draw page selection being done
bmi_setpage	ENDP


;--- BMI SET VISUAL PAGE ----------------------------------------------
;Select the visual display page passed AL
;Assume:  nothing
;Entry:   AL = Display page to select
;Return:  N/A
;Destory: None

bmi_setvisual	PROC FAR
	RET			;no visual page selection being done
bmi_setvisual	ENDP


;--- BMI SET WRITE MODE -----------------------------------------------
;Set pixel write mode as passed in AL for the procedure group specified
;or select specific function by command.
;Assume: nothing
;Entry:  AL = Bits 5-7 = command
;             Bits 0-4 = mode/type
;
;Bits 5,6 and 7 select the command function to be performed.
;  000 = 0:(00) Line write mode
;  001 = 1:(20) Pixel write mode
;  010 = 2:(40) Fill write mode
;  011 = 3:(60) FloodFill type
;  100 = 4:(80) Text write mode
;  101 = 5:(A0)  <reserved>
;  110 = 6:(C0)  <reserved>
;  111 = 7:(E0) Misc commands 
;
;for commands 00, 20, 40, and 80 one of the following subfunctions
;is added to the command. 
;  0= MOVE write       8= FORE MOVE write      16= BACK MOVE write     
;  1= XOR write        9= FORE XOR write       17= BACK XOR write      
;  2= OR write        10= FORE OR write        18= BACK OR write      
;  3= AND write       11= FORE AND write       19= BACK AND write     
;  4= NOT MOVE write  12= FORE NOT MOVE write  20= BACK NOT MOVE write
;  5= NOT XOR write   13= FORE NOT XOR write   21= BACK NOT XOR write 
;  6= NOT OR write    14= FORE NOT OR write    22= BACK NOT OR write  
;  7= NOT AND write   15= FORE NOT AND write   23= BACK NOT AND write 
;  24 = Set Background color to currently selected foreground color.
;  30 = Return current selected write mode on next GetMaxMode call.
;  31 = Return current background color on next GetMaxMode call.
;  25-29 = (-unused-)
;
;for command 60 (floodfill commands), one of the following subfunctions 
;is added to the command.
;  0= Border fill         8= Auto Fill         12= DelayDraw Off
;  1= Seed fill           9= Complex Fill      13= DelayDraw On
;  2-7   = (-unused-)    10= Compress Off      14= Tracer Off
;  16-30 = (-unused-)    11= Compress On       15= Tracer On
;  31 = Return currently selected floodfill option bits
;
;for command E0 (misc commands), one of the following subfunctions 
;is added to the command.
;   0-23 =   (-unused-)
;     24 = Set PutImage background color to current draw color (SetColor).
;     25 = Return current selected graphics mode in next GetMaxMode call.
;     26 = Return last peak floodfill stack usage in next GetMaxMode call.
;     27 = Return last floodfill stack free space in next GetMaxMode call. 
;     28 =   (-unused-)
;     29 =   (-unused-)
;     30 = Return last used putimage write mode in next GetMaxMode call. 
;     31 = Return current PutImage background color on next GetMaxMode call.
;note: #25 is used for reverse compatiblity to V2.0x of the BGI256 driver.
;Future versions will require the Misc command function to be used 
;(prior version did not require it (ie the top three bits were ignored).
;This version will still respond to #25, but you should use the 0E0H
;misc command tag for future compatibility.
;
;note: The old command C0 (BitMap write mode) will not be supported in 
;the future. it still functions in this version of the driver, but it 
;may change on the next release. The function only was able to set the 
;background color of the PutImage function anyway, so I moved it to 
;the Misc Command function to free up the group number for future use.
;
;-------------
;Return:  N/A
;Destroy: None

bmi_setwritemode PROC FAR
	PUSH	DS
	PUSH	DX
	PUSH	CX
	PUSH	BX
	PUSH	AX
     IF BGIVERSION LT 3
	PUSH	CS
	POP	DS
     ELSE
	MOV	DS,CS:[ALIAS]
     ENDIF
	MOV	CX,AX		;stuff procedure selection in CL
	AND	CL,0E0H		;strip off draw mode select
	AND	AL,1FH		;strip off proc sel info in AL

	CMP	AL,25	;-->	;@(note: this will go away in the future)
	JZ	@Bmiswg	;-->	;@you will have to use misc command instead
	CMP	CL,0C0H	;-->	;@(note: this will go away in the future)
	JZ	@Bmiswa	;-->	;@you will have to use misc command instead

	CMP	CL,060H
	JZ	@Bmiswf		;process fill type command
	CMP	CL,0A0H	
	JZ	@Bmiswx		;currently unused so ignore this
	CMP	CL,0E0H
	JZ	@Bmiswg		;process misc commands

@Bmiswa:
	CMP	AL,24		;0-23 its a write mode command so go do it
	JL	@Bmiswp		;Go set proc mode
	JZ	@Bmiswb		;24=Do Background color selection
	CMP	AL,30
	JL	@Bmiswx		;25-29=unknown function
	JZ	@Bmisww		;30=ret write mode on next GetMaxMode call
	JMP	@Bmiswv		;31=ret backgnd color on next GetMaxMode call

@Bmiswg:
	JMP	@Bmiswm

@Bmiswrb:
	MOV	AH,0		 ;ret byte value
@Bmiswrw:
	MOV	DS:[RetValue],AX ;ret word value
	MOV	byte ptr DS:[ModeXflag],1 ;mark that value is to be returned
@Bmiswx:
	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	DS
	RET
bmi_setwritemode ENDP


;--------------------------------
;write mode command processor
@Bmiswp:
	MOV	CH,AL 		;save the mode select
	MOV	AH,0		;index into proc pointer table
	ADD	AX,AX		;to get the proc address
	LEA	BX,DrawModeTable
	ADD	BX,AX
	MOV	DX,DS:[BX]		;get selected proc addr into DX
	CALL	GetWriteModeAdr
	JNZ	@Bmiswpx     ;invalid mode selection
	MOV	DS:[BX],DX   ;save selected draw mode procedure address
	MOV	BX,AX	     ;in the selected group procedure pointer
	MOV	DS:[BX],CH   ;save mode select number
@Bmiswpx:
	JMP	@Bmiswx

;--------------------------------
;set background write mode 
@Bmiswb:
	CALL	GetBackColorAdr ;function 24, set background
	JNZ	@Bmiswbx	;<-- invalid background selection
	MOV	AH,DS:[DrawForeColor]
	MOV	DS:[BX],AH	;store the new background color
@Bmiswbx:
	JMP	@Bmiswx

;--------------------------------
;return selected write mode on next GetMaxMode call
@Bmisww:
	CALL	GetWriteModeAdr
	JNZ	@Bmiswwx	;<-- invalid background selection
	MOV	BX,AX
	MOV	AL,DS:[BX]	;read current selected mode
	JMP	@Bmiswrb
@Bmiswwx:
	JMP	@Bmiswx

;--------------------------------
;return selected background color on next GetMaxMode call
@Bmiswv:
	CALL	GetBackColorAdr ;function 24, set background
	JNZ	@Bmiswvx	;<-- invalid background selection
	MOV	AL,DS:[BX]	;get the background color
	JMP	@Bmiswrb
@Bmiswvx:
	JMP	@Bmiswx

;--------------------------------
;floodfill type command processor
@Bmiswf:
	CMP	AL,16		;over range?
	JNC	@Bmiswf8
	MOV	CH,DS:[FloodFillType] ;get current type flags
	MOV	BL,AL		;copy new select bit to BL
	AND	BL,01H		;strip off control bits
	MOV	BH,0FEH		;init bit mask
	MOV	CL,AL
	SHR	CL,1		;seed or border fill command?
	JZ	@Bmiswf7	;yes, go set the type bit
	CMP	AL,08H
	JL	@Bmiswfx	;unused command
	CMP	AL,10H		;control command?
	JGE	@Bmiswfx
	AND	CL,03H		;strip garbage
	INC	CL		;adjust bit position
	INC	CL
@Bmiswf7:
	ROL	BH,CL		;move bit mask into position
	ROL	BL,CL		;move bit flag into position
	AND	CH,BH		;strip old bit
	OR	CH,BL		;or in new bit
	MOV	DS:[FloodFillType],CH	;set floodfill type
	JMP	@Bmiswfx

@Bmiswf8:
	CMP	AL,31		;return floodfill option bits?
	JNZ	@Bmiswfx
	MOV	AL,DS:[FloodFillType]
	JMP	@Bmiswrb
@Bmiswfx:
	JMP	@Bmiswx


;--------------------------------
;misc command processor
;24=set putimage background color
;25=ret selected graphics mode
;26=ret xystack peak
;27=ret xystack free
;30=ret putimage background color
;31=ret last used putimage write mode
@Bmiswm:
	CMP	AL,24		;set putimage background color
	JZ	@Bmiswm3
	CMP	AL,25		;ret graphics mode selection
	JZ	@Bmiswm4
	CMP	AL,26		;ret xystack peak
	JZ	@Bmiswm5
	CMP	AL,27		;ret xystack free
	JZ	@Bmiswm6
	CMP	AL,30		;ret last used putimage write mode
	JZ	@Bmiswm7
	CMP	AL,31		;ret putimage background color
	JZ	@Bmiswm8
@Bmiswmx:
	JMP	@Bmiswx		;unknown function request

@Bmiswm3:
	MOV	AH,DS:[DrawForeColor]	;update the putimage backcolor
	MOV	DS:[BitMapBackColor],AH	;to be the current drawing color
	JMP	@Bmiswmx

@Bmiswm4:
	MOV	AL,DS:[ModeSelect]	;25=return current graphics mode
	JMP	@Bmiswrb

@Bmiswm5:
	MOV	AX,DS:[XYStackPeak]	;26=return xystack peak
	JMP	@Bmiswrw

@Bmiswm6:
	MOV	AX,DS:[XYStackFree]	;27=return xystack free
	JMP	@Bmiswrw

@Bmiswm7:
	MOV	AL,DS:[BitMapPixelWriteMode] ;30=ret putimage write mode
	JMP	@Bmiswrb

@Bmiswm8:
	MOV	AL,DS:[BitMapBackColor] ;31=return putimage backcolor
	JMP	@Bmiswrb


;----------------------------------------------------------------
;return the desired write mode and write procedure addresses
;Assume: DS = data segment
;Enter:  CL = mode selector
;Return: AX = write mode variable address
;        BX = write procedure pointer
;destroy: nothing

GetWriteModeAdr PROC NEAR
	LEA	AX,LinePixelWriteMode
	LEA	BX,LinePixelProc
	CMP	CL,0		;set for line procs?
	JZ	@Bmiswdx
	LEA	AX,DrawPixelWriteMode
	LEA	BX,DrawPixelProc
	CMP	CL,20H		;set for pixel procs?
	JZ	@Bmiswdx
	LEA	AX,FillPixelWriteMode
	LEA	BX,FillPixelProc
	CMP	CL,40H		;set for fill procs?
	JZ	@Bmiswdx
	LEA	AX,TextPixelWriteMode
	LEA	BX,TextPixelProc
	CMP	CL,80H		;set for text procs?
	JZ	@Bmiswdx
	LEA	AX,BitMapPixelWriteMode
	LEA	BX,BitMapPixelProc ;(note: 0C0H goes away in future)
	CMP	CL,0C0H
	JZ	@Bmiswdx
@Bmiswdx:
	RET	;unused selections: (060H, 0A0H, 0E0H, 0C0H)
GetWriteModeAdr ENDP


;----------------------------------------------------------------------
;return the desired background color variable address
;Assume: DS = data segment
;Enter:  CL = mode selector
;Return: BX = write mode variable address
;destroy: nothing

GetBackColorAdr PROC NEAR
	LEA	BX,DrawBackColor      ;(line back color)
	CMP	CL,00H
	JZ	@Bmiswcx
	LEA	BX,PixelBackColor
	CMP	CL,20H
	LEA	BX,FillBackColor
	CMP	CL,40H
	JZ	@Bmiswcx
	LEA	BX,TextBackColor
	CMP	CL,80H
	JZ	@Bmiswcx
	LEA	BX,BitMapBackColor ;(note: this bitmap cmd goes away in future)
	CMP	CL,0C0H
	JZ	@Bmiswcx
@Bmiswcx:
	RET	;unused selections: (060H, 0A0H, 0E0H, 0C0H)
GetBackColorAdr ENDP

;-----------------------------------------------------------------------
