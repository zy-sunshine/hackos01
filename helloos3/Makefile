ipl.bin: ipl.nas Makefile
	../z_tools/nask.exe ipl.nas ipl.bin ipl.lst
helloos.img: ipl.bin Makefile
	../z_tools/edimg.exe imgin:../z_tools/fdimg0at.tek wbinimg src:ipl.bin len:512 from:0 to:0 imgout:helloos.img

run: helloos.img Makefile
	cp helloos.img ../z_tools/qemu/fdimage0.bin 
	../z_tools/make.exe -C ../z_tools/qemu
