#define COL8_000000 0   // 0黑
#define COL8_FF0000 1   // 1亮红
#define COL8_00FF00 2   // 2亮绿
#define COL8_FFFF00 3   // 3亮黄
#define COL8_0000FF 4   // 4亮蓝
#define COL8_FF00FF 5   // 5亮紫
#define COL8_00FFFF 6   // 6浅亮蓝
#define COL8_FFFFFF 7   // 7白
#define COL8_C6C6C6 8   // 8亮灰
#define COL8_840000 9   // 9暗红
#define COL8_008400 10  // 10暗绿
#define COL8_848400 11  // 11暗黄
#define COL8_000084 12  // 12暗青
#define COL8_840084 13  // 13暗紫
#define COL8_008484 14  // 14浅暗蓝
#define COL8_848484 15  // 15暗灰

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

void HariMain(void)
{
    int i, xsize, ysize;
    char *vram;
    
    init_palette();
    vram = (char *)0xa0000;
    xsize = 320;
    ysize = 200;
    
    // for (i = 0; i <= 0xffff; i++){
        // vram[i] = i & 0x0f;
    // }
    
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize-1, ysize-29);
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize-28, xsize-1, ysize-28);
    boxfill8(vram, xsize, COL8_FFFFFF, 0, ysize-27, xsize-1, ysize-27);
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize-26, xsize-1, ysize-1);
    
    boxfill8(vram, xsize, COL8_FFFFFF, 3, ysize-24, 59, ysize-24);
    boxfill8(vram, xsize, COL8_FFFFFF, 2, ysize-24, 2,  ysize-4);
    boxfill8(vram, xsize, COL8_848484, 3, ysize-4,  59, ysize-4);
    boxfill8(vram, xsize, COL8_848484, 59,ysize-23, 59, ysize-5);
    boxfill8(vram, xsize, COL8_000000, 2, ysize-3,  59, ysize-3);
    boxfill8(vram, xsize, COL8_000000, 60,ysize-24, 60, ysize-3);
    
    boxfill8(vram, xsize, COL8_848484, xsize-47, ysize-24, xsize-4, ysize-24);
    boxfill8(vram, xsize, COL8_848484, xsize-47, ysize-23, xsize-47,ysize-4);
    
    boxfill8(vram, xsize, COL8_FFFFFF, xsize-47, ysize-3,  xsize-4, ysize-3);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize-3,  ysize-24, xsize-3, ysize-3);
fin:
    io_hlt();
    goto fin;    
}

void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00,   // 0黑
        0xff, 0x00, 0x00,   // 1亮红
        0x00, 0xff, 0x00,   // 2亮绿
        0xff, 0xff, 0x00,   // 3亮黄
        0x00, 0x00, 0xff,   // 4亮蓝
        0xff, 0x00, 0xff,   // 5亮紫
        0x00, 0xff, 0xff,   // 6浅亮蓝
        0xff, 0xff, 0xff,   // 7白
        0xc6, 0xc6, 0xc6,   // 8亮灰
        0x84, 0x00, 0x00,   // 9暗红
        0x00, 0x84, 0x00,   // 10暗绿
        0x84, 0x84, 0x00,   // 11暗黄
        0x00, 0x00, 0x84,   // 12暗青
        0x84, 0x00, 0x84,   // 13暗紫
        0x00, 0x84, 0x84,   // 14浅暗蓝
        0x84, 0x84, 0x84,   // 15暗灰
    };
    set_palette(0, 15, table_rgb);
    return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
    int i, eflags;
    eflags = io_load_eflags(); // 记录中断许可标志
    io_cli(); // 将中断许可标志置为0, 禁止中断
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags); // 恢复中断许可标志
    // sti ?
    return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            vram[y * xsize + x] = c;
        }
    }
}
