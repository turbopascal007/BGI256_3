;SDATA.INC - Copyright 1991,1992 Knight Software
;
; History: 
;   17 May 1991 - first release
;   14 Jul 1991 - Corrected Line draw default from MoveByte to ForeByte
;   19 Aug 1992 - Added mode error flag
;   22 Nov 1992 - Addapted for protected mode operation
;
;----------------------------------------------------------------------
;
; Data variables and constants for BGI driver
;
; NOTE: The data in this file resides in the data selector
;       when operating in protected mode. The data segment is 
;       shared with the code segment in real mode.
;
;----------------------------------------------------------------------

;register structure used for processing real mode interrupts 
;from protected mode operation
SimIntStruc struc
  RealDI	DD 0
  RealSI	DD 0
  RealBP	DD 0
  RealXX	DD 0
  RealBX	DD 0
  RealDX	DD 0
  RealCX	DD 0
  RealAX	DD 0
  RealFlags	DW 0
  RealES	DW 0
  RealDS	DW 0
  RealFS	DW 0
  RealGS	DW 0
  RealIP	DW 0
  RealCS	DW 0
  RealSP	DW 0
  RealSS	DW 0
SimIntStruc ends

SimInt:	SimIntStruc <>


;======================================================================
;The following are declared constants. They are set at compile time.
;======================================================================
grOk		=   0	;Normal Operation, No errors
grNoInitGraph	=  -1
grNotDetected	=  -2
grFileNotFound	=  -3
grInvalidDriver	=  -4
grNoLoadMem	=  -5
grNoScanMem	=  -6
grNoFloodMem	=  -7
grFontNotFound	=  -8
grNoFontMem	=  -9
grInvalidMode	= -10
grError		= -11	;Generic Driver Error
grIOerror	= -12
grInvalidFont	= -13
grInvalidFontNum   = -14
grInvalidDeviceNum = -15


;======================================================================
;The Status Block is used to detect and control the operation
;of the driver. The Status Block is set by the INSTALL procedure
;by copying the appropriate mode table into the common access table.
;======================================================================

StatBlockSize	EQU	26	;Stat block size in bytes
status struc
  stat		DB	0	; 0 = no errors
  devtyp 	DB	0	; device type 0=screen 1=printer 2=plotter
  xres		DW	639 ;799 ;639	; full resolution
  yres		DW	479 ;599 ;479
  xefres 	DW	639 ;799 ;639	; effective resolution
  yefres 	DW	479 ;599 ;479
  xinch		DW	9000	; device size in inches*1000
  yinch		DW	7000
  aspec		DW	10000	; aspect ratio
  chsizx 	DB	8
  chsizy	DB	8	; standard char size x,y (points)
  ctblf 	DB	255	;foreground colors
  ctblb		DB	255	;background colors
  BytesPerScanLine DW	640 ;# bytes in a scan line
  TotalScanLines   DW	480 ;# scan lines on display
  MaxColor	   DW	256 ;max colors available
  PixelBits	   DB	8   ;Number of bits per pixel
status ends

StatBlock:	status <>

;----------------------------------------------------------------------
;Table of Pointers to the available mode status blocks 

	EVEN		;make sure this starts on word boundry

StatusPtrTable:
	DW	Mode0Statblock ;320x200
	DW	Mode1Statblock ;640x400
	DW	Mode2Statblock ;640x480
	DW	Mode3Statblock ;800x600
	DW	Mode4Statblock ;1024x768
	DW	Mode5Statblock ;2048x1024
	DW	Mode0Statblock ;320x200 ;dummy fill table to 8 pos
	DW	Mode0Statblock ;320x200

Mode0StatBlock:	; 320 x 200 x 256
	DB	0	; stat	  0 = no errors
	DB	0	; devtyp  device: 0=screen 1=printer 2=plotter
	DW	319	; xres	  full resolution
	DW	199	; yres	
	DW	319	; xefres  effective resolution
	DW	199	; yefres 
	DW	9000	; xinch	  device size in inches*1000
	DW	7000	; yinch	
	DW	8333	; aspec	  aspect ratio
	DB	8	; chsizx 	
	DB	8	; chsizy  standard char size x,y (points)
	DB	255	; ctblf   foreground colors
	DB	255	; ctblb	  background colors
	DW	320 	; BytesPerScanLine  # bytes in a scan line
	DW	200 	; TotalScanLines    # scan lines on display
	DW	256 	; MaxColor	    max colors available
	DB	8	; PixelBits         Number of bits per pixel

Mode1StatBlock:	; 640 x 400 x 256
	DB	0	; stat	  0 = no errors
	DB	0	; devtyp  device: 0=screen 1=printer 2=plotter
	DW	639	; xres	  full resolution
	DW	399	; yres	
	DW	639	; xefres  effective resolution
	DW	399	; yefres 
	DW	9000	; xinch	  device size in inches*1000
	DW	7000	; yinch	
	DW	8333	; aspec	  aspect ratio
	DB	8	; chsizx 
	DB	8	; chsizy  standard char size x,y (points)
	DB	255	; ctblf   foreground colors
	DB	255	; ctblb	  background colors
	DW	640 	; BytesPerScanLine  # bytes in a scan line
	DW	400 	; TotalScanLines    # scan lines on display
	DW	256 	; MaxColor	    max colors available
	DB	8	; PixelBits         Number of bits per pixel

Mode2StatBlock:	; 640 x 480 x 256
	DB	0	; stat	  0 = no errors
	DB	0	; devtyp  device: 0=screen 1=printer 2=plotter
	DW	639	; xres	  full resolution
	DW	479	; yres	
	DW	639	; xefres  effective resolution
	DW	479	; yefres 
	DW	9000	; xinch	  device size in inches*1000
	DW	7000	; yinch	
	DW	10000	; aspec	  aspect ratio
	DB	8	; chsizx 
	DB	8	; chsizy  standard char size x,y (points)
	DB	255	; ctblf   foreground colors
	DB	255	; ctblb	  background colors
	DW	640 	; BytesPerScanLine  # bytes in a scan line
	DW	480 	; TotalScanLines    # scan lines on display
	DW	256 	; MaxColor	    max colors available
	DB	8	; PixelBits         Number of bits per pixel

Mode3StatBlock:	; 800 x 600 x 256
	DB	0	; stat	  0 = no errors
	DB	0	; devtyp  device: 0=screen 1=printer 2=plotter
	DW	799	; xres	  full resolution
	DW	599	; yres	
	DW	799	; xefres  effective resolution
	DW	599	; yefres 
	DW	9000	; xinch	  device size in inches*1000
	DW	7000	; yinch	
	DW	10000	; aspec	  aspect ratio
	DB	8	; chsizx 
	DB	8	; chsizy  standard char size x,y (points)
	DB	255	; ctblf   foreground colors
	DB	255	; ctblb	  background colors
	DW	800 	; BytesPerScanLine  # bytes in a scan line
	DW	600 	; TotalScanLines    # scan lines on display
	DW	256 	; MaxColor	    max colors available
	DB	8	; PixelBits         Number of bits per pixel

Mode4StatBlock:	; 1024 x 768 x 256
	DB	0	; stat	  0 = no errors
	DB	0	; devtyp  device: 0=screen 1=printer 2=plotter
	DW	1023	; xres	  full resolution
	DW	767	; yres	
	DW	1023	; xefres  effective resolution
	DW	767	; yefres 
	DW	9000	; xinch	  device size in inches*1000
	DW	7000	; yinch	
	DW	10000	; aspec	  aspect ratio
	DB	8	; chsizx 
	DB	8	; chsizy  standard char size x,y (points)
	DB	255	; ctblf   foreground colors
	DB	255	; ctblb	  background colors
	DW	1024 	; BytesPerScanLine  # bytes in a scan line
	DW	768 	; TotalScanLines    # scan lines on display
	DW	256 	; MaxColor	    max colors available
	DB	8	; PixelBits         Number of bits per pixel

Mode5StatBlock:	; 2048 x 1024 x 256
	DB	0	; stat	  0 = no errors
	DB	0	; devtyp  device: 0=screen 1=printer 2=plotter
	DW	2047	; xres	  full resolution
	DW	1023	; yres	
	DW	2047	; xefres  effective resolution
	DW	1023	; yefres 
	DW	9000	; xinch	  device size in inches*1000
	DW	7000	; yinch	
	DW	10000	; aspec	  aspect ratio
	DB	8	; chsizx 
	DB	8	; chsizy  standard char size x,y (points)
	DB	255	; ctblf   foreground colors
	DB	255	; ctblb	  background colors
	DW	2048 	; BytesPerScanLine  # bytes in a scan line
	DW	1024 	; TotalScanLines    # scan lines on display
	DW	256 	; MaxColor	    max colors available
	DB	8	; PixelBits         Number of bits per pixel

;----------------------------------------------------------------------
;Mode names - One entry for each selectable mode
ModeNameSize	EQU 15	;Each name entry in the table is 15 bytes long
ModeName	DB 10
		DB '320 x 200 ',0,0,0,0	;0
		DB 10
		DB '640 x 400 ',0,0,0,0	;1
		DB 10
		DB '640 x 480 ',0,0,0,0	;2
		DB 10
		DB '800 x 600 ',0,0,0,0	;3
		DB 11
		DB '1024 x 768 ',0,0,0	;4
		DB 12
		DB '2048 x 1024 ',0,0	;5
ModeNameError	DB 8
		DB 'Unknown ',0
ModeNameBad	DB 6
		DB 'Error ',0

ModeNameMax	EQU 40		;40 character mode name string max
ModeNameBuffer	DB ModeNameMax DUP(0)

DefaultMode	EQU 0		;start in mode 0 unless otherwise set

VidMode 	DW 0		;Video mode we will be using

CardNamePtr	DW DummyCard	;pointer to card name


;======================================================================
;The pattern tables control how fills and line draws are performed.
;======================================================================

	EVEN		;make sure this starts on word boundry

;fill pattern data table
FillTable:
	DB 000h,000h,000h,000h,000h,000h,000h,000h  ;0  No Fill
	DB 0FFh,0FFh,0FFh,0FFh,0FFh,0FFh,0FFh,0FFh  ;1  Solid Fill
	DB 0FFh,0FFh,000h,000h,0FFh,0FFh,000h,000h  ;2  Line Fill
	DB 001h,002h,004h,008h,010h,020h,040h,080h  ;3  Lt Slash Fill
	DB 0E0h,0C1h,083h,007h,00Eh,01Ch,038h,070h  ;4  Slash Fill
	DB 0F0h,078h,03Ch,01Eh,00Fh,087h,0C3h,0E1h  ;5  Backslash Fill
	DB 0A5h,0D2h,069h,0B4h,05Ah,02Dh,096h,04Bh  ;6  Lt Bkslash Fill
	DB 0FFh,088h,088h,088h,0FFh,088h,088h,088h  ;7  Hatch Fill
	DB 081h,042h,024h,018h,018h,024h,042h,081h  ;8  XHatch Fill
	DB 0CCh,033h,0CCh,033h,0CCh,033h,0CCh,033h  ;9  Interleave Fill
	DB 080h,000h,008h,000h,080h,000h,008h,000h  ;10 Wide Dot Fill
	DB 088h,000h,022h,000h,088h,000h,022h,000h  ;11 Close Dot Fill

;Line pattern used for drawing lines - SetLineStyle
LinePatternTable:
	DW	1111111111111111B ;AL = 0 : Solid Line Style
	DW	1100110011001100B ;AL = 1 : Dotted Line			
	DW	1111110001111000B ;AL = 2 : Center Line			
	DW	1111100011111000B ;AL = 3 : Dashed line			
			     	  ;AL > 3 : User-defined line style

;======================================================================
;The EGA palette is just here for compatibility.
;======================================================================
;This defines the EGA palette that is loaded at init time
EgaPalette	DB	16	;palette size
		DB	0	;palette zero (background)
		DB	1
		DB	2
		DB	3
		DB	4
		DB	5
		DB	6
		DB	7
		DB	8
		DB	9
		DB	10
		DB	11
		DB	12
		DB	13
		DB	14
		DB	15	;palette 15
		DB	0	;overscan register


;======================================================================
;The following are fixed variables. They are set once by INSTALL.
;======================================================================

	EVEN		;make sure this starts on word boundry

VideoSegment	DW	0A000H	;Video Segment location
NumberModes	DW	6	;Number modes available
WinGran		DW	1	;Window granularity multiplier
DisplayType	DW	1	;Display type in use
ScanLineLength	DW	320	;scan line length in bytes

ModeSelect	DB	0	;Selected display mode number
InitColor	DB	0	;initial background color
CharSizeX	DB	8	;Character size
CharSizeY	DB	8
ModeErrorFlag	DB	0	;set NZ when unsupported mode selected

;======================================================================
;The following are reference variables. They are set by various procs.
;======================================================================
;color definition variables
DrawForeColor	DB	255	;line drawing foreground color
DrawBackColor	DB	0	;line drawing background color
FillForeColor	DB	255	;fill foreground color
FillBackColor	DB	0	;fill background color
BitMapBackColor	DB	0	;bitmap background color 
TextBackColor	DB	0	;Text background color
				;(text foreground uses drawcolor)
PixelBackColor	DB	0	;pixel drawing background color
				;(pixel foreground uses passed color)
;Note: forecolor is whatever is specified at the time the object
;is being drawn (setcolor or setfill commands). Drawcolor is set 
;via setcolor command, fillcolor is set via setfill command.
;backcolor is set via the specific setwritemode command.


	EVEN		;make sure this starts on word boundry

;positioning variables
CPX		DW	0	;master cursor position x
CPY		DW	0	;master cursor position y
ClipX1		DW	0
ClipY1		DW	0	;clip boundries
ClipX2		DW	639
ClipY2		DW	479

;line drawing variables
PlotStyle	DW	1	;0=dotperx, 1=x+y, 2=overlap 
LinePattern	DW	1111111111111111B
LineWidth	DB	1

;Fill pattern working array
;do fill from this array - default = Solid Fill
FillPattern:
	DB 0FFh,0FFh,0FFh,0FFh,0FFh,0FFh,0FFh,0FFh  ;(1)  Solid Fill

;Text variables
FontNumber	DB	0	;Font number selected
FontDir		DB	0	;Direction to draw font
FontSizeX	DB	8	;Full size of font
FontSizeY	DB	8	; (CharSize*FontMult)
FontMultX	DB	1	;Character size multiplier
FontMultY	DB	1	;


;**********************************************************************
;The following procedure pointers are used to control program flow
;**********************************************************************

;======================================================================
;The draw mode table is a table of near pointers (set at compile time)
;to the appropriate pixel routine to place the pixel on the screen 
;(32 entries) Currently the only valid seletions are 0-23. 
;Selections 24-31 are invalid, but will map to the first eight to keep 
;the code happy just in case we overflow the table. 
;======================================================================

	EVEN		;make sure this starts on word boundry

DrawModeTable:
	DW	MoveByte	;#0 Overwrite mode
	DW	XorByte		;#1 XOR write mode
	DW	OrByte		;#2 OR write mode
	DW	AndByte		;#3 AND write mode
	DW	NotByte		;#4 Complement write mode
	DW	NxorByte	;#5 NXOR write mode
	DW	NorByte		;#6 NOR write mode
	DW	NandByte	;#7 NAND write mode
	DW	ForeByte	;#8 Foreground Only write mode
	DW	ForeXorByte	;#9 Foreground XOR write mode
	DW	ForeOrByte	;#10 Foreground OR write mode
	DW	ForeAndByte	;#11 Foreground AND write mode
	DW	ForeNotByte	;#12 Foreground NOT write mode
	DW	ForeNxorByte	;#13 Foreground NXOR write mode
	DW	ForeNorByte	;#14 Foreground NOR write mode
	DW	ForeAndByte	;#15 Foreground AND write mode
	DW	BackByte	;#16 Background Only write mode
	DW	BackXorByte	;#17 Background XOR write mode
	DW	BackOrByte	;#18 Background OR write mode
	DW	BackAndByte	;#19 Background AND write mode
	DW	BackNotByte	;#20 Background NOT write mode
	DW	BackNxorByte	;#21 Background NXOR write mode
	DW	BackNorByte	;#22 Background NOR write mode
	DW	BackNandByte	;#23 Background NAND write mode
	;the following are undefined, but we must have something here
	;to keep the code happy. These may change in the future
	DW	MoveByte	;#24 Overwrite mode
	DW	XorByte		;#25 XOR write mode
	DW	OrByte		;#26 OR write mode
	DW	AndByte		;#27 AND write mode
	DW	NotByte		;#28 Complement write mode
	DW	NxorByte	;#29 NXOR write mode
	DW	NorByte		;#30 NOR write mode
	DW	NandByte	;#31 NAND write mode

;----------------------------------------------------------------------
;The initDisplayProc pointer points to the routine to init the display
;to the selected mode. This allows multiple display inits.
InitDisplayProc	DW	VGA320x200Init	;(default init select)

;----------------------------------------------------------------------
;This points to the code that controls bank switching for the given
;pixel address in PixelX/PixelY. It is set by the Install proc.
BankSelectProc DW	VGABankSelect	;(default bank sel proc)

;----------------------------------------------------------------------
;the following four near pointers contain the drawing mode pointer
;for the specified function of the pointer. 
;These procs can be changed through the SetWriteMode function
FillPixelProc	DW	MoveByte	;proc ptr for bar procs
DrawPixelProc	DW	MoveByte	;proc ptr for pixel procs
LinePixelProc	DW	ForeByte	;proc ptr for line procs
TextPixelProc	DW	ForeByte	;proc ptr for text procs
BitMapPixelProc	DW	MoveByte	;proc ptr for bit map procs

;----------------------------------------------------------------------
;This proc pointer is set via the PutImage procedure call
BitMapProc	DW	WriteBitMap	;proc ptr for main proc code

;----------------------------------------------------------------------
;These variables store the selected write mode in use for procs
FillPixelWriteMode	DB	0
DrawPixelWriteMode	DB	0
LinePixelWriteMode	DB	8
TextPixelWriteMode	DB	8
BitMapPixelWriteMode	DB	0

;----------------------------------------------------------------------
;This variable selects the type of flood fill to use. default = zero
;                       ;bit     description
FloodFillType	DB 0	; 0: 0=border fill,  1=seed fill
			; 1: 0 (not used)
			; 2: 0=normal stack, 1=compressed stack
			; 3: 0=auto search,  1=slow search
			; 4: 0=draw on scan, 1=draw after scan (slow)
			; 5: 0=no tracer,    1=tracer if bit4=1
			; 6: 0 (not used)
			; 7: 0=fast fill,    1=slow fill (computed)
;note: bit seven is computed at start of floodfill operation based on
;the search mode selected (bit 1) and the fillstyle (move/xor/etc)

SeedFill	EQU 01H	;fill everything not seed else all but border
ComplexFill	EQU 04H	;force complex fill method (else auto select)
StackCompress	EQU 08H	;use compressed XY stack during fill
DelayedDraw	EQU 10H	;draw after scanning is done (on slowfill only)
TracerOn	EQU 20H ;show tracer during fill scan
SlowFill	EQU 80H	;use slow stack piggy complex fill method
			; (slow fill required for complex fills)

;**********************************************************************
;Misc temp variables. These are only used internally.
;**********************************************************************

	EVEN		;make sure this starts on word boundry

;word variables go first followed by the byte variables 
CurBank		DW	0	;current selected bank

PixelAddress	DW	0	;pixel vars
PixelSegment	DW	0
PixelSegmentLength DW	0
PixelX		DW	0
PixelY		DW	0
PixelX1		DW	0
PixelY1		DW	0
PixelX2		DW	0
PixelY2		DW	0

PlotStepX	DW	0	;line plot vars
PlotStepY	DW	0
PlotDeltaX	DW	0
PlotDeltaY	DW	0
PlotSlope	DW	0

RightX		DW	0	;flood fill vars
LeftX		DW	0
ScanLeftX	DW	0
ScanRightX	DW	0
FillFlipX1	DW	0
FillFlipX2	DW	0
FillFlipY	DW	0
OldFlipColors	DW	0
PrevStackIndex	DW	0
XYStackIndex	DW	0
XYStackAdj	DW	0
XYStackPeak	DW	0
XYStackFree	DW	0

TempChar	DW	0,0,0,0,0,0,0,0  ;temp bitmap text vars
RetValue	DW	0	;used to ret value in GetMaxMode calls

;---------------------------------------
;misc byte oriented variables
SeedColor	DB	0
BorderColor	DB	0
ModeXflag	DB	0
LinePixelCount	DB	0
FloodFillFlags	DB	0
		DB	0	;dummy spare to make even byte count

;---------------------------------------
;other stuff goes here

FILEHANDLE	DW	0
FLOODNAME	DB	'FLOOD.DAT',0

	EVEN		;make sure this starts on word boundry
TempWork	DB 512  DUP(0)	;general temp working scratch buffer

;======================================================================

