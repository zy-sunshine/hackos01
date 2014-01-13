[INSTRSET "i486p"]
ORG 0xc200	; ipl 加载文件到 0x8000 ,磁盘镜像中 文件0入口在 0x4200
		; 所以相对位置为 0x8000 + 0x4200 = 0xC200

BOTPAK	EQU		0x00280000		; 与 bootpack.h 中声明的 ADR_BOTPAK 匹配
DSKCAC	EQU		0x00100000		; 磁盘从0-10柱面共180KB大小数据之前被加载到 0x7c00(512) 0x8200(剩下的都在这里)
DSKCAC0	EQU		0x00008000		; 磁盘的10柱面都拷贝到0x8000之后(0x8000-0x8200 的数据其实不在这里，这个里面是MBR，是由BIOS初始加载到0x7c00处)

CYLS EQU 0x0ff0
LEDS EQU 0x0ff1
VMODE EQU 0x0ff2
SCRNX EQU 0x0ff4
SCRNY EQU 0x0ff6
VRAM EQU 0x0ff8

VBEMODE EQU 0x103 ; 800x600x8bit
;	0x100 :  640 x  400 x 8bit
;	0x101 :  640 x  480 x 8bit
;	0x103 :  800 x  600 x 8bit
;	0x105 : 1024 x  768 x 8bit
;	0x107 : 1280 x 1024 x 8bit

start:
	; enable grahpic mode
	; check whether have VBE mode
	mov AX, 0x9000
	mov ES, AX
	mov DI, 0
	mov AX, 0x4f00
	int 0x10
	cmp AX, 0x004f
	jne scrn320
	
	; Check VBE mode version
	mov AX, [ES:DI+4]
	cmp AX, 0x0200
	jb  scrn320		; below 2.0 version can not use VBE mode

	; check whether can use this mode
	mov CX, VBEMODE
	mov AX, 0x4f01
	int 0x10
	cmp AX, 0x004f
	jne scrn320

	; check vbe mode attribute, please refer P270
	cmp BYTE [ES:DI+0x19], 8	; 8bit
	jne scrn320
	cmp BYTE [ES:DI+0x1b], 4 	; whether can use palette
	jne scrn320
	mov AX, [ES:DI+0x00]
	and AX, 0x0080 				; & 0x0000 0000 1000 0000
	jz  scrn320

	; ok, we can use VBE mode and VBEMODE there.
	mov BX, VBEMODE + 0x4000
	mov AX, 0x4f02	; switch VBE mode
	int 0x10
	mov BYTE [VMODE], 8
	mov AX, [ES:DI+0x12]
	mov WORD [SCRNX], AX
	mov AX, [ES:DI+0x14]
	mov WORD [SCRNY], AX
	mov EAX, [ES:DI+0x28]
	mov DWORD [VRAM], EAX		; 实模式寻址不同，但是可以使用32位寄存器
	jmp keystatus

scrn320:	; 非 VBE 模式
	mov AL, 0x13
	mov AH, 0x00
	int 0x10
	
	MOV BYTE [VMODE], 8
	MOV WORD [SCRNX], 320
	MOV WORD [SCRNY], 200
	MOV DWORD [VRAM], 0x000a0000

keystatus:
	MOV AH, 0x02
	INT 0x16
	MOV [LEDS], AL
	
	jmp entry
	
showmsg:
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
	db "hello debug"
	db 0
	
	
entry:
; PIC 关闭一切中断
;	根据 AT 兼容机的规格，如果要初始化 PIC 必须在 CLI 之前进行，否则有时会挂起
;	随后进行 PIC 的初始化
		MOV		AL,0xff
		OUT		0x21,AL  	; <=> io_out(PIC0_IMR, 0xff);
		NOP
		OUT		0xa1,AL  	; <=> io_out(PIC1_IMR, 0xff);

		CLI ; 禁止所有中断

; 开启1mb以上寻址空间 与开启键盘和鼠标函数类似 init_keyboard enable_mouse，都是向键盘控制器发送信号完成
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20GATE
		OUT		0x60,AL
		CALL	waitkbdout

; 切换到保护模式(protected virtual address mode)
; 实模式(real mode, 是real address mode的省略形式)
; 实模式是使用 ES 等寄存器的段x16 加段内偏移来寻址，保护模式要使用GDT来寻址，而GDT在用户空间是不能被修改的，因此成为保护模式
; 有带保护的16位模式和带保护的32位模式，这里我们切换的是32位模式
; [INSTRSET "i486p"]				; 之后的指令都为 486 格式, 已经在开头设置

		LGDT	[GDTR0]			; 设置临时全局段号描述表  Global(segment) Description Table
		MOV		EAX,CR0         ; 很重要的一个寄存器 CR0 control register 0
		AND		EAX,0x7fffffff	; bit31 设为 0 ; 禁止分页
		OR		EAX,0x00000001	; bit0 设为 1 ; 切换保护模式
		MOV		CR0,EAX
		JMP		pipelineflush	; CPU 的规范，切换为保护模式后需要马上执行 JMP 操作 (CPU 的 流水线pipeline 需要刷新清空)
; 切换为
pipelineflush:
		MOV		AX,1*8			; gdt+1 可读写的段 0x00000000-0xffffffff 32bit，初始化除CS之外的所有段寄存器，因为CS会之后处理，现在处理会造成执行混乱
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack 的转送

		MOV		ESI,bootpack	; 源地址
		MOV		EDI,BOTPAK		; 目的地址，将我们自己的 bootpack 程序 加载到 0x00280000 与 gdt+2(代码段) 对应
		MOV		ECX,512*1024/4  ; 转送块数量 4Byte为单位(1024Byte会大于bootpack的长度，这个如果之后bootpack过大需要调节)
		CALL	memcpy

; 将磁盘数据转送到它本来的位置去

; 首先从启动扇区开始

		MOV		ESI,0x7c00		; 源地址
		MOV		EDI,DSKCAC		; 目的地址， 将 MBR 所在位置0x7c00读取到 0x00100000(1M地址之后)
		MOV		ECX,512/4 		; 读取 512 个字节 0x00100000 - 0x00100200
		CALL	memcpy

; 转送所有剩下的

		MOV		ESI,DSKCAC0+512	; 0x00008200 复制到
		MOV		EDI,DSKCAC+512	; 0x00100200
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]	; 10 柱面
		IMUL	ECX,512*18*2/4	; 10 柱面(C) x 2(H) x 18 (S) x 512(Byte)
		SUB		ECX,512/4		; 去掉那个 MBR (其中包含IPL)
		CALL	memcpy

; 由 asmhead 来完成的工作，到此完毕
; 以后就交由 bootpack 来完成

; bootpack 的启动

	; 拷贝 "程序" 到 0x00310000
		; 校验程序头
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有要转送的东西时跳转到skip (??? TODO: 这个地方的版本检查和拷贝"程序"数据的问题要在之后确认一下)
		MOV		ESI,[EBX+20]	; 拷贝 (EBX+20 位置数据是 0x10c8 书上P157有具体描述)
		ADD		ESI,EBX			; 拷贝地址为bootpack头偏移 0x10c8 位置
		MOV		EDI,[EBX+12]	; 至 (EBX+12 位置数据是 0x00310000)
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 初始栈的值为程序运行的起始地址，其增长方向是由高到低
		
		; CS 代码段地址在这里改变，这里的 2*8 会load到CS中，0x0000001b 是相对于CS的偏移，那绝对地址就是 [gdt+2]+0x0000001b
		JMP		DWORD 2*8:0x0000001b ; 2*8 是代码段(gdt+2)偏移 到 bootpack 的0x1b地址执行，这个地址正是去掉 bootpack 文件头的代码位置

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; <=> wait_KBC_sendready
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 如果转送块计数器不到0就继续转送
		RET

		ALIGNB	16            	; 地址16位对齐 (将之前的地址对齐，以便下面的数据table声明)
GDT0:
		RESB	8				; 空出8个字节
		; 下面这段等价于  set_segment(gdt+1, 0xffffffff, 0x00000000, AR_DATA32_RW)
		DW		0xffff,0x0000,0x9200,0x00cf		; 可以读写的段 (segment) 32bit
		;                 set_segment(gdt+2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER)
		DW		0xffff,0x0000,0x9a28,0x0047		; 可以执行的段 (segment) 32bit bootpack 用

		DW		0
GDTR0:
		; 在汇编中 word 为 两个字节长，DB, DW, DD(4Byte)
		DW		8*3-1 	; double word 两个字节 0x0017 (GDT 的limit长度)
		DD		GDT0  	; GDT 的地址 4Byte长度
		;GDTR0 指针中的数值一共48bit，这样传给 LGDT 就会将 GDT 指向 GDT0 来配置

		ALIGNB	16
bootpack:
