;--------------------------------------------------------
;this is used for debugging to save the stack info 
;to an external file

OPENFILE PROC NEAR
	PUSHF
	PUSH AX
	PUSH BX
	PUSH CX
	PUSH DX

	MOV AH,3CH
	LEA DX,DS:[FLOODNAME]
	MOV CX,0
	INT 21H
	MOV DS:[FILEHANDLE],AX

	POP DX
	POP CX
	POP BX
	POP AX
	POPF
	RET
OPENFILE ENDP


WRITEFILE PROC NEAR
	PUSHF
	PUSH AX
	PUSH BX
	PUSH CX
	PUSH DX
	PUSH DI
	PUSH SI
	PUSH ES
	PUSH DS

	MOV AH,40H
	MOV BX,DS:[FILEHANDLE]
	MOV DX,BP
	MOV CX,6
	PUSH SS
	POP DS
	INT 21H

	POP DS
	POP ES
	POP SI
	POP DI
	POP DX
	POP CX
	POP BX
	POP AX
	POPF
	RET
WRITEFILE ENDP


CLOSEFILE PROC NEAR
	PUSHF
	PUSH AX
	PUSH BX
	MOV AH,3EH
	MOV BX,DS:[FILEHANDLE]
	INT 21H
	POP BX
	POP AX
	POPF
	RET
CLOSEFILE ENDP


;--------------------------------------------------------------
;this is used to show whats going on inside.
;DX=Y, BX=X, SP=SP
WHEREXY PROC NEAR
	PUSH DX
	PUSH CX
	PUSH BX
	PUSH AX

	PUSH CX
	PUSH BX
	PUSH DX

	MOV BX,0
	MOV DX,DS:[statblock.totalscanlines]
	SUB DX,10
	MOV AL,'S'
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,':'
	CALL DRAWCHAR
	ADD BX,8
	MOV AX,SP
	CALL DRAWHEXW

	MOV AL,' '
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,'P'
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,':'
	CALL DRAWCHAR
	ADD BX,8
	MOV AX,BP
	CALL DRAWHEXW

	MOV AL,' '
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,'Y'
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,':'
	CALL DRAWCHAR
	ADD BX,8
	POP AX
	CALL DRAWHEXW

	MOV AL,' '
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,'L'
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,':'
	CALL DRAWCHAR
	ADD BX,8
	POP AX
	CALL DRAWHEXW

	MOV AL,' '
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,'R'
	CALL DRAWCHAR
	ADD BX,8
	MOV AL,':'
	CALL DRAWCHAR
	ADD BX,8
	POP AX
	CALL DRAWHEXW

	POP AX
	POP BX
	POP CX
	POP DX
	RET
WHEREXY ENDP


DRAWHEXW PROC NEAR
	PUSH AX
	MOV AL,AH
	SHR AL,1
	SHR AL,1
	SHR AL,1
	SHR AL,1
	CALL DONIBBLE

	POP AX
	PUSH AX
	MOV AL,AH
	AND AL,0FH
	CALL DONIBBLE

	POP AX
	PUSH AX
	SHR AL,1
	SHR AL,1
	SHR AL,1
	SHR AL,1
	CALL DONIBBLE

	POP AX
	PUSH AX
	AND AL,0FH
	CALL DONIBBLE

	POP AX
	RET
DRAWHEXW ENDP


DONIBBLE PROC NEAR
	ADD AL,30H
	CMP AL,'9'
	JLE DONIB2
	ADD AL,7
DONIB2:
	CALL DRAWCHAR
	ADD BX,8
	RET
DONIBBLE ENDP


;-----------------------------------------------------------
