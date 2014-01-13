#include "bootpack.h"

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

unsigned int memtest_sub(unsigned int start, unsigned int end);

unsigned int memtest(unsigned int start, unsigned int end)
{
    char flg486 = 0;
    unsigned int eflg, cr0, i;
    
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if((eflg & EFLAGS_AC_BIT) != 0) {
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    
    if(flg486 != 0){
        // disable cache
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    i = memtest_sub(start, end);
    if (flg486 != 0) {
        // enable cache
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
    unsigned int mid = 0;
    // // check start
    if (!check_mem_valid0(start)){
        // !!
        return start;
    }
    // // check end
    if (check_mem_valid0(end)){
        // !!
        return end + 4;
    }
    // check each middle
    for(;;){
        mid = (end + start) / 2;
        if(start == mid){
            break;
        }
        if (check_mem_valid0(mid)) {
            start = mid;
        }else{
            end = mid;
        }
    }
    return start + 4; // 因为start地址是可用的，start地址后面的4个字节已经检查可用，需要加上这4个字节长度

}

void memman_init(MEMMAN_t *man)
{
    man->frees = 0;
    man->maxfrees = 0;
    man->lostsize = 0;
    man->losts = 0;
    return;
}

unsigned int memman_total(MEMMAN_t *man)
{
    unsigned int i, t = 0;
    for (i = 0; i < man->frees; i++) {
        t += man->free_[i].size;
    }
    return t;
}

unsigned int memman_alloc(MEMMAN_t *man, unsigned int size)
{
    unsigned int i, a;
    for (i = 0; i < man->frees; ++i)
    {
        if (man->free_[i].size >= size){
            a = man->free_[i].addr;
            man->free_[i].addr += size;
            man->free_[i].size -= size;
            if (man->free_[i].size == 0) {
                man->frees--;
                for (; i < man->frees; ++i)
                {
                    man->free_[i] = man->free_[i+1];    // 移位 结构体赋值
                }
            }
            return a;
        }
    }
}

int memman_free(MEMMAN_t *man, unsigned int addr, unsigned int size)
{
    int i, j;
    for (i = 0; i < man->frees; ++i)
    {
        if (man->free_[i].addr > addr){
            break;
        }
    }
    /* free_[i - 1].addr < addr < free_[i].addr */
    if (i > 0){
        // have mem block before
        if (man->free_[i - 1].addr + man->free_[i - 1].size == addr) {
            // integrate mem with previous block
            man->free_[i - 1].size += size;
            if(i < man->frees) {
                if (addr + size == man->free_[i].addr) {
                    // integrate mem with next block
                    man->free_[i - 1].size += man->free_[i].size;
                    man->frees--;
                    for (; i < man->frees; ++i) {
                        man->free_[i] = man->free_[i + 1];
                    }
                }
            }
            return 0;
        }
    }
    if (i < man->frees) {
        // have mem block after

        if (addr + size == man->free_[i].addr) {
            // integrate mem with next block
            man->free_[i].addr = addr;
            man->free_[i].size += size;
            return 0;
        }
    }
    if (man->frees < MEMMAN_FREES) {
        // can not integrate with previous and next mem block
        for (j = man->frees; j > i; j--) {
            man->free_[j] = man->free_[j - 1];
        }
        man->frees++;
        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees;
        }
        man->free_[i].addr = addr;
        man->free_[i].size = size;
        return 0;
    }
    // can not integrate and move another free space.
    man->losts++;
    man->lostsize += size;
    return -1;
}

unsigned int memman_alloc_4k(MEMMAN_t *man, unsigned int size)
{
    unsigned int a;
    size = (size + 0xfff) & 0xfffff000;
    a = memman_alloc(man, size);
    return a;
}

int memman_free_4k(MEMMAN_t *man, unsigned int addr, unsigned int size)
{
    int i;
    size = (size + 0xfff) & 0xfffff000;
    i = memman_free(man, addr, size);
    return i;
}
