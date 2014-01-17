#include "bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;
#define MEMMAN_ADDR 0x003c0000  // used by memman to record free mem block information
void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "O$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@"
    };
    int x, y;
    char c;
    boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
    boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
    boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
    boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
    boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
    boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
    putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
    for (y = 0; y < 14; y++) {
        for (x = 0; x < 16; x++) {
            c = closebtn[y][x];
            if (c == '@') {
                c = COL8_000000;
            } else if (c == '$') {
                c = COL8_848484;
            } else if (c == 'Q') {
                c = COL8_C6C6C6;
            } else {
                c = COL8_FFFFFF;
            }
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
    return;
}
void HariMain(void)
{
    BOOTINFO_t *binfo;
    int mx, my, i;
    unsigned int memsize, count = 0;
    char s[40], keybuf[32], mousebuf[128];

    MOUSE_DEC_t mdec;
    MEMMAN_t * memman = (MEMMAN_t*) MEMMAN_ADDR;    // os is nubility...

    SHTCTL_t *shtctl;
    SHEET_t *sht_back, *sht_mouse, *sht_win;
    unsigned char *buf_back, buf_mouse[256], *buf_win;

    binfo = (BOOTINFO_t *)BOOTINFO_ENTRY;
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
    
    // init programmable interrupt controller
    init_gdtidt();
    init_pic();
    io_sti();   // 开启中断
    // init fifo buffer
    // ......
    io_out8(PIC0_IMR, 0xf9); /* PIC1‚ÆƒL[ƒ{[ƒh‚ð‹–‰Â(11111001) 设置PIC0 中断mask */
	io_out8(PIC1_IMR, 0xef); /* ƒ}ƒEƒX‚ð‹–‰Â(11101111) 设置PIC1 中断mask */
    
    init_keyboard(); // 初始化键盘控制芯片中的鼠标控制器
    init_palette();
    enable_mouse(&mdec); // 通知开启鼠标中断信息产生

    // init memory manager
    memsize = memtest(0x00000000, 0xc0000000);
    memman_init(memman);
    //memman_free(memman, 0x00001000, 0x0009e000); // 0x00000ff0 - 0x00000ffb 是 BOOTINFO_ENTRY
    memman_free(memman, 0x00001000, 0x0009f000); // 书上是用的 0x00001000-0x0009efff, 但是 0x00100000(1MB) 之前就没有被使用的区域了
    memman_free(memman, 0x00400000, memsize - 0x00400000);

    // init sheet controller
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    sht_win = sheet_alloc(shtctl);
    buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // -1 no invisable color
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 99 is the invisable color
    sheet_setbuf(sht_win, buf_win, 160, 52, -1);

    init_screen8(buf_back, binfo->scrnx, binfo->scrny);
    
    mx = my = 100;
    init_mouse_cursor8(buf_mouse, 99);

    make_window8(buf_win, 160, 68, "counter");

    sheet_slide(sht_back, 0, 0);
    sheet_slide(sht_win, 80, 72);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 16) / 2;
    sheet_slide(sht_mouse, mx, my);
    sheet_updown(sht_back, 0);
    sheet_updown(sht_win, 1);
    sheet_updown(sht_mouse, 2);

    sprintf(s, "(%3d, %3d)", mx, my);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

    //putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    // static char font_A[16] = {
		// 0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
		// 0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	// };
    // putfont8(binfo->vram, binfo->scrnx, 8, 8, COL8_0000FF, hankaku+'A'*16);
    // putfonts8_asc(binfo->vram, binfo->scrnx, 16, 8, COL8_0000FF, "BC 123");
    
    // putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "EasyOS.");
    // putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "EasyOS.");
    
    //memsize = memtest(0x00400000, 0xbfffffff) / (1024*1024);

    sprintf(s, "memory %uMB free %uKB %dx%d", memsize/(1024*1024), memman_total(memman)/1024, binfo->scrnx, binfo->scrny);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
    sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);
    
    for (;;) {
        count++;
        sprintf(s, "%010d", count);
        boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
        putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);

        sheet_refresh(sht_win, 40, 28, 120, 44);

		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_sti();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(buf_back, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
                sheet_refresh(sht_back, 0, 16, 16, 32);
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
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                    sheet_refresh(sht_back, 32, 16, 32 + 15 * 8, 32);

                    //boxfill8(buf_mouse, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); // hide mouse
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) {
                        mx = 0;
                    }
                    if (my < 0) {
                        my = 0;
                    }
                    if (mx > binfo->scrnx - 1) {
                        mx = binfo->scrnx - 1;
                    }
                    if (my > binfo->scrny - 1) {
                        my = binfo->scrny - 1;
                    }
                    sprintf(s, "(%3d, %3d)", mx, my);
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                    sheet_refresh(sht_back, 0, 0, 80, 16);
                    //putblock8_8(buf_mouse, binfo->scrnx, 16, 16, mx, my, mcursor, 16); // show new position of mouse
                    sheet_slide(sht_mouse, mx, my);

                }
			}
		}
	}
    
fin:
    io_hlt();
    goto fin;    
}
