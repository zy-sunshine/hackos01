TOP=../..
DEL=rm

TOOLPATH = $(TOP)/z_tools/
INCPATH  = $(TOP)/z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com

MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe

default: easyos.img

ipl10.bin:
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin: asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

%.gas: %.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas: %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj: %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

naskfunc.obj: naskfunc.nas Makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

BOOTPACK_SRC_OBJ = bootpack.obj dsctbl.obj graphic.obj \
				   naskfunc.obj hankaku.obj int.obj fifo.obj \
				   memman.obj
	
bootpack.bim: Makefile $(BOOTPACK_SRC_OBJ)
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		 $(BOOTPACK_SRC_OBJ)

bootpack.hrb: bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

easyos.sys: asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > easyos.sys

easyos.img: ipl10.bin easyos.sys Makefile
	$(EDIMG)   imgin:$(TOP)/z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:easyos.sys to:@: \
		imgout:easyos.img

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

run: easyos.img
	cp easyos.img ${TOP}/z_tools/qemu/fdimage0.bin 
	${TOP}/z_tools/make.exe -C ${TOP}/z_tools/qemu

clean:
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) easyos.sys
	-$(DEL) easyos.img