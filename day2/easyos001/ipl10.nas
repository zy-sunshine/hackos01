; hello-os
; TAB=4
CYLS EQU 10
ORG 0x7c00

JMP entry
DB 0x90
DB "HELLOIPL"
DW 512
DB 1
DW 1
DB 2
DW 224
DW 2880
DB 0xf0
DW 9
DW 18
DW 2
DD 0
DD 2880
DB 0,0,0x29
DD 0xffffffff
DB "HELLO-OS   "
DB "FAT12   "
RESB 18

entry:
	MOV AX,0
	MOV SS,AX
	MOV SP,0x7c00
	MOV DS,AX
	
	
	MOV AX, 0x0820
	MOV ES, AX	; copy data to [ES:BX],[ES:BX] stands for ESx16+BX, so we use 0x8200-0x83ff address range
	MOV CH, 0;	Cylinder      read order(C-H-S)
	MOV	DH,0;	Header
	MOV CL, 2;	Sector  index start from 1

readloop:
	MOV SI, 0;	counter
retry:
	MOV AH, 0X02; read from floppy
	MOV AL, 1;	deal with 1 sector
	MOV BX, 0;	copy data to [ES:BX]
	MOV DL, 0X00; floppy driver A
	INT 0X13;	call floppy BIOS function
	JNC next;	jump to next label, if not carry (carry stands for error occur)
	ADD SI, 1;	if we get an error, try it 5 times
	CMP SI, 5
	JAE error;	error five times, so jump error label
	MOV AH, 0X00
	MOV DL, 0X00;	driver A
	INT 0X13; call BIOS reset driver A
	JMP retry;	retry read

next:
	MOV AX, ES
	ADD AX, 0X0020; Add ES+20 equal to add address 200 (because [ES:BX] = ESx16+BX)
	MOV ES, AX
	ADD CL, 1;	sector increase
	CMP CL, 18
	JBE readloop
	MOV CL, 1;  from begin of next Sector 1
	ADD DH, 1;	from next Header +1
	CMP DH, 2
	JB  readloop
	MOV DH, 0;	from begin of next Header 0
	ADD CH, 1;	from next Cylinder +1
	CMP CH, CYLS
	JB  readloop
; Now read 10(CYLS) * 2 (Header) * 18 (Sector) * 512 Byte = 184320 byte = 180KB
; from 0x8200 + (184320-512(read by hardware BIOS)) = 0x35000  (0x8200-0x34fff)
; We load many bytes(from 0+0x200 bytes) to memory(from 0x8200), so disk position 
; is map to 0x8000+disk_offset, so the 0x4200 is the disk's file content of floopy
; we should jump to 0x8000 + 0x4200 = 0xc200 to execute the first program on floopy
MOV		[0x0ff0],CH; Save bot Cylinder number into boot info section
JMP 0xc200

error:
	mov SI, msg
putloop:
	mov AL, [SI]
	ADD SI, 1
	CMP AL, 0
	JE  fin
	MOV AH, 0x0e
	mov BX, 15
	int 0x10; Show character on screen
	jmp putloop
fin:
	HLT
	JMP fin
	
msg:
	db 0x0a, 0x0a
	db "load error"
	db 0
	
RESB	0x7dfe-$
DB		0x55, 0xaa
