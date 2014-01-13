ORG 0xc200	; ipl �����ļ��� 0x8000 ,���̾����� �ļ�0����� 0x4200
		; �������λ��Ϊ 0x8000 + 0x4200 = 0xC200

CYLS EQU 0x0ff0
LEDS EQU 0x0ff1
VMODE EQU 0x0ff2
SCRNX EQU 0x0ff4
SCRNY EQU 0x0ff6
VRAM EQU 0x0ff8


start:
	mov AL, 0x13
	mov AH, 0x00
	int 0x10
	
	MOV BYTE [VMODE], 8
	MOV WORD [SCRNX], 320
	MOV WORD [SCRNY], 200
	MOV DWORD [VRAM], 0x000a0000
	
	MOV AH, 0x02
	INT 0x16
	MOV [LEDS], AL
	
	jmp fin
entry:
	mov SI, msg
putloop:
	mov AL, [SI]
	ADD SI, 1
	CMP AL, 0
	JE  fin
	MOV AH, 0x0e
	mov BX, 15
	int 0x10
	jmp putloop
fin:
	HLT
	JMP fin
	
msg:
	db 0x0a, 0x0a
	db "hello"
	db 0