;INITDATA.INC - Copyright 1991,1992 Knight Software
;
; History: 
;   22 Nov 1992 - 1st Created during conversion to protected mode 
;
;----------------------------------------------------------------------
;
; Initialization Data tables for BGI driver 
;  For Version 3.00 driver, this stuff *MUST* appear first in 
;    the data segment (ie offset zero).
;  For Version 2.00 driver it can appear anyplace.
;
; NOTE: For version 3.00 driver operation -
;       The data in this file resides in the data selector
;       when operating in protected mode. The data segment is 
;       shared with the code segment in real mode.
;


;=====================================================================
;This table is mainly for protected mode operation in ver 3+
;however, some routines do peek at the table to determine
;if we are running in protected mode even when compiled for ver 2.

InitTable:

NRealSegs    DB 3          ;number of items in segment table
RealSegOfs   DW SegTable   ;offset (in DS) to segment lookup table
NIntVects    DB 1          ;number of real mode interrupt vectors
IntVecOfs    DW VectTable  ;offset (in DS) to interrupt vect table
NRealBlocks  DB 0          ;number of real mode block descriptors
RealBlockOfs DW 0 ;BlockTable ;offset (in DS) to block table
DSinRealMem  DB 1	   ;* indicates where to put the data segment 
RealModeDS   DW 0	   ;** Actual real mode segment value of DS
CurOpMode    DB 0          ;Z=real mode, NZ=protected mode

;*  if DSinRealMem is zero, the data segment will reside in protected
;    memory someplace. If DSinRealMem is 1 (NZ), the data segment
;    will be allocated in the real mode memory area (below 1M).
;     (For my purposes, I need to have the data seg in real mem area.)
;
;** RealModeDS is patched by loader at load time this is the real mode
;    "segment" address of the DS, not the protected mode selector. 


;----------------------------------------------------------------------
;The segment/selector table is used to access real mode memory
;directly. In real mode, the values will be left as is. In protected
;mode, the loader will read the values in the table and convert them
;to selectors. ie SEGA000 will contain 0A000h in real mode or the
;allocated selector in protected mode.

SegTable:
SEG0040	     DW 00040h	   ;segment/selector for seg 40    (bios)
SEGA000      DW 0A000h	   ;segment/selector for seg A000  (video)
SEGF000      DW 0F000h     ;segment/selector for seg F000  (rom)

;----------------------------------------------------------------------
;the indicated interrupt vector in this table is read from real memory
;and either stuffed in the indicate pointer in real mode, or the segment
;part is converted to a selector in protected mode. With the selector
;being given a size of 64K. The offset remains the real mode offset value.

VectTable:
ChrGen:      DD 0000001Fh  ;this gets replaced with the sel:ofs
			   ;in protected mode, or seg:ofs in real mode
			   ;as obtained from int vect 1Fh

;----------------------------------------------------------------------
;The loader allocates the memory block at load time and inits 
;RBlockSeg and RBlockSel to appropriate values. In real mode, RBlockSeg
;will point to the real segment of the allocated memory. In protected 
;mode RBlockSeg and RBlockSel will point to the real mode segment and 
;protected mode selector of the allocated memory. The memory block 
;will always be located in real mode memory (under 1M).
;Note in this case I am creating my own real mode memory block
;to maintain reverse compatiblity to the V2.0 drivers.

;BlockTable:
;RBlockMode   DW 32	   ;how much real mem to grab (512 bytes)
;RBlockSeg    DW 0FFFFh     ;segment of temp work real memory  (NZ=none)
;RBlockSel    DW 0	   ;selector of temp work real memory


;======================================================================
;The following are constant variables. They are set at compile time.
;======================================================================
;Interface table - these pointers are used to access the routines
DDOVEC	DW _install	  ; Driver initialization and installation
	DW _init   	  ; Initialize device for output
	DW _clear	  ; Clear graphics device 
	DW _post	  ; Make picture visible. Enable screen.
	DW _move	  ; Set Current Drawing Pointer to value.
	DW _draw	  ; Draw a line from CP to X,Y coordinates.
	DW _vect	  ; Draw a line between given coordinates.
	DW EMULATE	  ; _polygon (Draw a polygon.)
	DW EMULATE	  ;_bar (Draw a 3D filled bar.)
	DW _patbar	  ; Draw a patterned bar.
	DW EMULATE	  ;_arc (Draw an elliptical arc.)
	DW EMULATE	  ;_pieslice (Draw an elliptical sector.)
	DW EMULATE	  ;_filled_ellipse (Draw a filled ellipse.) 
	DW _palette	  ; Set a palette entry to a given color.
	DW _allpalette	  ; Load the palette with a table of colors.
	DW _color	  ; Set Drawing and Fill colors.
	DW _fillstyle	  ; Set current fill pattern. 
	DW _linestyle	  ; Set current line style. 
	DW _textstyle	  ; Set the text font attributes.
	DW _drawtext	  ; Draw a string in the current font.
	DW _textsize	  ; Calculate string size (in pixels).
	DW RESERVED	  ; RESERVED
	DW _floodfill	  ; Do a floodfill.
	DW _getpixel	  ; Read a pixel from the given coord.
	DW _setpixel	  ; Write a pixel to the given coord.
	DW _bitmaputil	  ; Return a pointer to misc utilities. 
	DW _savebitmap	  ; Save a block of screen to CPU memory.
	DW _restorebitmap ; Restore block of screen from CPU memory.
	DW _setclip	  ; Set the clipping window.
	DW _color_query	  ; Return device color information
	DW 35 DUP (NONE)  ; Reserved Entry Points


;----------------------------------------------------------------------
;this is the table that the _bitmaputil function points at

BitMapTable:	
	DW	bmi_gotographic	  ; enter pixel drawing mode
	DW	bmi_exitgraphic   ; leave pixel drawing mode
	DW	bmi_putpixel	  ; put a pixel on screen
	DW	bmi_getpixel	  ; read a pixel from screen
	DW	bmi_bitsperpixel  ; display memory bits used per pixel
	DW	bmi_setpage	  ; draw specified page
	DW	bmi_setvisual	  ; select visual display page
	DW	bmi_setwritemode  ; set write mode 


;======================================================================
