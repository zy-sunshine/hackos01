#include "bootpack.h"

void HariMain(void)
{
    BOOTINFO_t *binfo;
    char mcursor[16*16];
    int mx, my;
    extern char hankaku[4096];
    
    binfo = (BOOTINFO_t *)BOOTINFO_ENTRY;

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
    
    enable_mouse(); // 通知开启鼠标中断信息产生
fin:
    io_hlt();
    goto fin;    
}
