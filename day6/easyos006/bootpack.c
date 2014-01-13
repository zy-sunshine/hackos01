#include "bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;


void HariMain(void)
{
    BOOTINFO_t *binfo;
    char mcursor[16*16];
    int mx, my, i;
    extern char hankaku[4096];
    char s[40], keybuf[32], mousebuf[128];
    
    MOUSE_DEC_t mdec;
    
    binfo = (BOOTINFO_t *)BOOTINFO_ENTRY;
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
    
    // init programmable interrupt controller
    init_gdtidt();
    init_pic();
    io_sti();   // 开启中断
    // init fifo buffer
    // ......
    io_out8(PIC0_IMR, 0xf9); /* (11111001) 设置PIC0 中断mask , 打开irq1  */
	io_out8(PIC1_IMR, 0xef); /* (11101111) 设置PIC1 中断mask , 打开irq12 */
    
    init_keyboard(); // 初始化键盘控制芯片中的鼠标控制器
 
    init_palette();
    
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    
    // static char font_A[16] = {
		// 0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
		// 0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	// };
    putfont8(binfo->vram, binfo->scrnx, 8, 8, COL8_0000FF, hankaku+'A'*16);
    putfonts8_asc(binfo->vram, binfo->scrnx, 16, 8, COL8_0000FF, "BC 123");
    
    putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "EasyOS.");
    putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "EasyOS.");
    
    mx = my = 100;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    
    enable_mouse(&mdec); // 通知开启鼠标中断信息产生
    
    for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
                if (mouse_decode(&mdec, i) != 0) {
                    sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0) {
                        s[1] = 'L';
                    }
                    if ((mdec.btn & 0x02) != 0) {
                        s[3] = 'R';
                    }
                    if ((mdec.btn & 0x04) != 0) {
                        s[2] = 'C';
                    }
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                }
				
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
                mx += mdec.x;
                my += mdec.y;
                if (mx < 0) {
                    mx = 0;
                }
                if (my < 0) {
                    my = 0;
                }
                if (mx > binfo->scrnx - 16) {
                    mx = binfo->scrnx - 16;
                }
                if (my > binfo->scrny - 16) {
                    my = binfo->scrny - 16;
                }
                sprintf(s, "(%3d, %3d)", mx, my);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
			}
		}
	}
    
fin:
    io_hlt();
    goto fin;    
}
