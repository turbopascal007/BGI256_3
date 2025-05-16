;TRIDENT.INC - Writen by Erik Olbrys
;    History:
;        July 14, 1991 - first release
;        22 Nov 1992   - Adapted by Knight Software for protected mode
;
;----------------------------------------------------------------------
;Determine what sort of card is out there
;Assumed: DS=data segment
;Entry:   N/A
;Return:  Correct procedure pointers installed
;Destroy: AX

DetectCard PROC NEAR
	PUSH    ES                      ;check if this is a Trident
	PUSH    BX

	MOV	BX,DS:[SEGA000]		;init video segment/selector
	MOV	DS:[VideoSegment],BX

noncr:  mov     dx,3c4h                 ;Test for Trident
	mov     al,0bh
	out     dx,al           ;out(3c4,0b)
	inc     dl
	in      al,dx           ;al:=inp(3c5)
	cmp     al,06h
	ja      notri           ;if al>6
	cmp     al,2
	jb      notri           ;if al<2

	JAE     @IsTrident              ;yup, so go init it

notri:  MOV     AX,1                    ;bummer, it ain't so error out
	MOV     DS:[NumberModes],AX        ;set number modes available
	LEA     BX,VGABankSelect        ;set bank select proc pointer
	MOV     DS:[BankSelectProc],BX

	LEA     BX,VGA320x200Init       ;we do all this so as to
	MOV     DS:[InitDisplayProc],BX    ;keep things in some 
	LEA     BX,DummyCard            ;form of order to prevent
	MOV     DS:[CardNamePtr],BX        ;nasty things from happening
	MOV     BX,64                   ;we are just telling the code
	MOV     DS:[WinGran],BX            ;that we have a simple VGA 
	MOV     AH,grNotDetected        ;or MCGA card set to mode 13H
	MOV     DS:[StatBlock.stat],AH     ;then set no detect error
	MOV	BYTE PTR DS:[ModeSelect],0 ;force mode to 0
	OR      AX,255
	JMP     @DetectExit

@IsTrident:
	MOV     AX,MaxModes             ;set number modes available
	MOV     DS:[NumberModes],AX
	LEA     BX,TRBankSelect         ;set bank select proc pointer
	MOV     DS:[BankSelectProc],BX

	MOV     AL,DS:[ModeSelect]
	AND     AX,7
	LEA     BX,TRModeProcTable      ;set display init proc pointer
	ADD     BX,AX
	ADD     BX,AX
	MOV     BX,CS:[BX]
	MOV     DS:[InitDisplayProc],BX
	LEA     BX,TRname
	MOV     DS:[CardNamePtr],BX        ;set up pointer to card name
	MOV     BX,64
	MOV     DS:[WinGran],BX            ;set window granularity
	XOR     AX,AX
@DetectExit:
	POP     BX
	POP     ES
	RET
DetectCard ENDP

MaxModes EQU 5  ;max number modes available for Trident

TRModeProcTable:
	DW      VGA320x200Init          ;table of init proc ptrs
	DW      TRvga640x400Init        ;for the various modes
	DW      TRvga640x480Init 
	DW      TRvga800x600Init 
	DW      TRvga1024x768Init       ;Requires 1 meg ram

	DW      VGA320x200Init          ;dummy excess mode selects
	DW      VGA320x200Init          ;to fill out to 8 positions
	DW      VGA320x200Init 

TRname:
	DB 8            ;name length
	DB ' Trident',0 ;Card name preceeded with space
			;and taged with a zero

;----------------------------------------------------------------------
;Select TRvga640x400 mode operation
;Assumed: DS=data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX,BX

TRvga640x400Init PROC NEAR
	mov     ax,5ch
	INT     10H                     ;call BIOS to set the mode
	XOR     AX,AX
	RET
TRvga640x400Init ENDP

;----------------------------------------------------------------------
;Select TRvga640x480 mode operation
;Assumed: DS=data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX,BX

TRvga640x480Init PROC NEAR
	mov     ax,5dh
	INT     10H                     ;call BIOS to set the mode
	XOR     AX,AX
	RET
TRvga640x480Init ENDP

;----------------------------------------------------------------------
;Select TRvga800x600 mode operation
;Assumed: DS=data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX,BX

TRvga800x600Init PROC NEAR
	mov     ax, 5eh
	INT     10H                     ;call BIOS to set the mode
	XOR     AX,AX
	RET
TRvga800x600Init ENDP

;----------------------------------------------------------------------
;Select TRvga1024x768 mode operation
;Assumed: DS=data segment
;Entry:   N/A
;Return:  Correct display mode selected
;Destroy: AX,BX

TRvga1024x768Init PROC NEAR
	mov     ax,62h
	INT     10H                     ;call BIOS to set the mode
	XOR     AX,AX
	RET
TRvga1024x768Init ENDP

;----------------------------------------------------------------------
;Trident Bank select routines - We assume that full memory is
;available if you try to access it.
;Assumed: DS=data segment
;Entry:   DX = Bank to select
;Return:  Display Bank is selected
;Destroy: nothing

TRBankSelect PROC NEAR
	PUSH	DX
	PUSH	AX
	CLI
	mov     DS:[curbank],dx
	mov     dx,3ceh                 ;set pagesize to 64k
	mov     al,6
	out     dx,al           ;out(03ce,6)
	inc     dl
	in      al,dx           ;inp
	dec     dl
	or      al,4
	mov     ah,al
	mov     al,6
	out     dx,ax           ;out(03ce,6)

	mov     dl,0c4h                 ;switch to BPS mode
	mov     al,0bh
	out     dx,al           ;out(03ce,0b)
	inc     dl
	in      al,dx           ;inp
	dec     dl

	mov     ah,byte ptr DS:[curbank]
	xor     ah,2
	mov     dx,3c4h
	mov     al,0eh
	out     dx,ax           ;out(03c4,0e)
	sti
	POP	AX
	POP	DX
	ret
TRBankSelect ENDP

