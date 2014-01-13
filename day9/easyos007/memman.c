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
        // 禁用 cache
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    i = memtest_sub(start, end);
    if (flg486 != 0) {
        // 启用缓存
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
    unsigned int i, mid = 0;
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
