;SBLT.INC   Copyright 1991,1992 Knight Software
;  History:
;      17 May 1991 - first release
;      22 Nov 1992 - adapted for protected mode operation
;
;-------------------------------------------------------
;Assume: Nothing
;Enter:  AL = color to draw if NZ flag passed (foreground)
;        AH = color to draw if Z flag is passed (background)
;        ES:DI points to screen address to write
;Return: N/A

MoveByte PROC NEAR	;#0  Overwrite mode
	JZ	@NoMove
	MOV	ES:[DI],AL
	RET
@NoMove:
	MOV	ES:[DI],AH
	RET
MoveByte ENDP

;-----------------------------------
XorByte PROC NEAR	;#1 XOR write mode
	JZ	@NoXor
	XOR	ES:[DI],AL
	RET
@NoXor:
	XOR	ES:[DI],AH
	RET
XorByte ENDP

;-----------------------------------
OrByte PROC NEAR	;#2 OR write mode
	JZ	@NoOr
	OR	ES:[DI],AL
	RET
@NoOr:
	OR	ES:[DI],AH
	RET
OrByte ENDP

;-----------------------------------
AndByte PROC NEAR	;#3 AND write mode
	JZ	@NoAnd
	AND	ES:[DI],AL
	RET
@NoAnd:
	AND	ES:[DI],AH
	RET
AndByte ENDP

;-----------------------------------
NotByte PROC NEAR	;#4 Complement write mode
	JZ	@NoXor
	NOT	AL
	MOV	ES:[DI],AL
	NOT	AL
	RET
@NoNot:
	NOT	AH
	MOV	ES:[DI],AH
	NOT	AH
	RET
NotByte ENDP

;-----------------------------------
NxorByte PROC NEAR	;#5 NXOR write mode
	JZ	@NoNxor
	NOT	AL
	XOR	ES:[DI],AL
	NOT	AL
	RET
@NoNxor:
	NOT	AH
	XOR	ES:[DI],AH
	NOT	AH
	RET
NxorByte ENDP

;-----------------------------------
NorByte PROC NEAR	;#6 NOR write mode
	JZ	@NoNor
	NOT	AL
	OR	ES:[DI],AL
	NOT	AL
	RET
@NoNor:
	NOT	AH
	OR	ES:[DI],AH
	NOT	AH
	RET
NorByte ENDP

;-----------------------------------
NandByte PROC NEAR	;#7 NAND write mode
	JZ	@NoNand
	NOT	AL
	AND	ES:[DI],AL
	NOT	AL
	RET
@NoNand:
	NOT	AH
	AND	ES:[DI],AH
	NOT	AH
	RET
NandByte ENDP

;----------------------------------
ForeByte PROC NEAR	;#8 Foreground write mode
	JZ	@NoFore
	MOV	ES:[DI],AL
	RET
@NoFore:
	RET
ForeByte ENDP

;----------------------------------
ForeXorByte PROC NEAR	;#9 Foreground XOR write mode
	JZ	@NoForeXor
	XOR	ES:[DI],AL
	RET
@NoForeXor:
	RET
ForeXorByte ENDP

;----------------------------------
ForeOrByte PROC NEAR	;#10 Foreground OR write mode
	JZ	@NoForeOr
	OR	ES:[DI],AL
	RET
@NoForeOr:
	RET
ForeOrByte ENDP

;----------------------------------
ForeAndByte PROC NEAR	;#11 Foreground AND write mode
	JZ	@NoForeAnd
	AND	ES:[DI],AL
	RET
@NoForeAnd:
	RET
ForeAndByte ENDP

;----------------------------------
ForeNotByte PROC NEAR	;#12 Foreground NOT write mode
	JZ	@NoForeNot
	NOT	AL
	MOV	ES:[DI],AL
	NOT	AL
	RET
@NoForeNot:
	RET
ForeNotByte ENDP

;----------------------------------
ForeNxorByte PROC NEAR	;#13 Foreground NXOR write mode
	JZ	@NoForeNxor
	NOT	AL
	XOR	ES:[DI],AL
	NOT	AL
	RET
@NoForeNxor:
	RET
ForeNxorByte ENDP

;----------------------------------
ForeNorByte PROC NEAR	;#14 Foreground NOR write mode
	JZ	@NoForeNor
	NOT	AL
	OR	ES:[DI],AL
	NOT	AL
	RET
@NoForeNor:
	RET
ForeNorByte ENDP

;----------------------------------
ForeNandByte PROC NEAR	;#15 Foreground NAND write mode
	JZ	@NoForeNand
	NOT	AL
	AND	ES:[DI],AL
	NOT	AL
	RET
@NoForeNand:
	RET
ForeNandByte ENDP

;----------------------------------
BackByte PROC NEAR	;#16 Background write mode
	JZ	@NoBack
	RET
@NoBack:
	MOV	ES:[DI],AH
	RET
BackByte ENDP

;----------------------------------
BackXorByte PROC NEAR	;#17 Background XOR write mode
	JZ	@NoBackXor
	RET
@NoBackXor:
	XOR	ES:[DI],AH
	RET
BackXorByte ENDP

;----------------------------------
BackOrByte PROC NEAR	;#18 Background OR write mode
	JZ	@NoBackOr
	RET
@NoBackOr:
	MOV	ES:[DI],AH
	RET
BackOrByte ENDP

;----------------------------------
BackAndByte PROC NEAR	;#19 Background AND write mode
	JZ	@NoBackAnd
	RET
@NoBackAnd:
	MOV	ES:[DI],AH
	RET
BackAndByte ENDP

;----------------------------------
BackNotByte PROC NEAR	;#20 Background NOT write mode
	JZ	@NoBackNot
	RET
@NoBackNot:
	NOT	AH
	MOV	ES:[DI],AH
	NOT	AH
	RET
BackNotByte ENDP

;----------------------------------
BackNxorByte PROC NEAR	;#21 Background NXOR write mode
	JZ	@NoBackNor
	RET
@NoBackNxor:
	NOT	AH
	XOR	ES:[DI],AH
	NOT	AH
	RET
BackNxorByte ENDP

;----------------------------------
BackNorByte PROC NEAR	;#22 Background NOR write mode
	JZ	@NoBackNor
	RET
@NoBackNor:
	NOT	AH
	MOV	ES:[DI],AH
	NOT	AH
	RET
BackNorByte ENDP

;----------------------------------
BackNandByte PROC NEAR	;#23 Background NAND write mode
	JZ	@NoBackNand
	RET
@NoBackNand:
	NOT	AH
	MOV	ES:[DI],AH
	NOT	AH
	RET
BackNandByte ENDP

