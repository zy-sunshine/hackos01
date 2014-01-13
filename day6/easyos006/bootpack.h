#include <stdio.h>

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

#define BOOTINFO_ENTRY 0x0ff0
#define BOOTINFO_SCRX 0x0ff4
#define BOOTINFO_SCRY 0x0ff6
#define BOOTINFO_VRAM 0x0ff8

typedef struct BOOTINFO_s {
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
} BOOTINFO_t;

/* fifo.c */
struct FIFO8 {
	unsigned char *buf;
	int p, q, size, free, flags;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void asm_inthandler21();
void asm_inthandler2c();
void asm_inthandler27();

/* dsctbl.c */
typedef struct SEGMENT_DESCRIPTOR_s {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
} SEGMENT_DESCRIPTOR_t;

typedef struct GATE_DESCRIPTOR_s {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
} GATE_DESCRIPTOR_t;


void init_gdtidt(void);
void set_segmdesc(SEGMENT_DESCRIPTOR_t *sd, unsigned int limit, int base, int ar);
void set_gatedesc(GATE_DESCRIPTOR_t *gd, int offset, int selector, int ar);


#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092 // 数据段可读写
#define AR_CODE32_ER	0x409a // 代码段和执行(Excute) 可写
#define AR_INTGATE32	0x008e

/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(char *vram, int xsize, int ysize);

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void putfont8(char *vram, int xsize, int x, int y, char c, unsigned char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);

void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
    int px0, int py0, char *buf, int bxsize);
    
/* int.c */
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

typedef struct MOUSE_DEC_s {
    unsigned char buf[3], phase;
    int x, y, btn;
} MOUSE_DEC_t;

void init_pic(void);
void inthandler21(int *esp);
void inthandler2c(int *esp);
void inthandler27(int *esp);
void init_keyboard(void);

void enable_mouse(MOUSE_DEC_t *mdec);
int mouse_decode(MOUSE_DEC_t *mdec, unsigned char dat);
