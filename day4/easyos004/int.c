#include <stdio.h>
#include "bootpack.h"

void init_pic(void)
{
    io_out8(PIC0_IMR, 0xff); // 禁止所有中断
    io_out8(PIC1_IMR, 0xff); // 禁止所有中断
    
    io_out8(PIC0_ICW1, 0x11);   // 边沿触发模式 (edge trigger mode)
    io_out8(PIC0_ICW2, 0x20);   // IRQ0-7 由 INT20-27 接收
    io_out8(PIC0_ICW3, 1 << 2); // PIC1 由 IRQ2 连接
    io_out8(PIC0_ICW4, 0x01);   // 无缓冲区模式
    
    io_out8(PIC1_ICW1, 0x11);   // 边沿触发模式 (edge trigger mode)
    io_out8(PIC1_ICW2, 0x28);   // IRQ8-15 由 INT28-2f 接收
    io_out8(PIC1_ICW3, 2);      // PIC1 由 IRQ2 连接
    io_out8(PIC1_ICW4, 0x01);   // 无缓冲区模式
    
    io_out8(PIC0_IMR, 0xfb);    // 11111011 PIC1 接收中断外，其他都屏蔽
    io_out8(PIC1_IMR, 0xff);    // 11111111 屏蔽所有中断
    
    return;
}
// 鼠标 IRQ12 键盘 IRQ1
#define PORT_KEYDAT 0x0060 // 中断时, 键盘信息的读取地址 P127
void inthandler21(int *esp)
{
    unsigned char data, s[4];
    BOOTINFO_t *binfo = (BOOTINFO_t *) BOOTINFO_ENTRY;
    
    io_out8(PIC0_OCW2, 0x61);   // 通知 IRQ-1 已经受理完毕
    data = io_in8(PORT_KEYDAT);
    sprintf(s, "%02X", data);
    
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
}

void inthandler2c(int *esp)
{
    unsigned char data;
    BOOTINFO_t *binfo = (BOOTINFO_t *) BOOTINFO_ENTRY;

    io_out8(PIC1_OCW2, 0x64);   // 通知 PIC1 IRQ-12 已受理完毕
    io_out8(PIC0_OCW2, 0x62);   // 通知 PIC0 IRQ-02 已受理完毕
    data = io_in8(PORT_KEYDAT);

    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2c (IRQ-c) : mouse");
    return;
}
// P119 需要执行 STI
void inthandler27(int *esp)
{
    BOOTINFO_t *binfo = (BOOTINFO_t *) BOOTINFO_ENTRY;
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 27 (IRQ-7) : None");
    io_out8(PIC0_OCW2, 0x67);
}

#define PORT_KEYDAT             0x0060
#define PORT_KEYSTA             0x0064
#define PORT_KEYCMD             0x0064
#define KEYSTA_SEND_NOTREADY    0x02
#define KEYCMD_WRITE_MODE       0x60
#define KBC_MODE                0x47

void wait_KBC_sendready(void)
{
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
    return;
}

void init_keyboard(void)
{
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

#define KEYCMD_SENDTO_MOUSE     0xd4
#define MOUSECMD_ENABLE         0xf4
void enable_mouse(void)
{
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    return;
}
