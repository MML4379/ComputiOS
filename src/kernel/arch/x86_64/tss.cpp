#include "tss.hpp"
#include <libk/types.hpp>

extern "C" uint8 kernel_stack_top;
extern "C" uint8 ist_df_stack_top;

TSS tss;

void tss_init() {
    // zero the TSS
    for (uint64 i = 0; i < sizeof(TSS); i++) {
        ((uint8*)&tss)[i] = 0;
    }

    tss.rsp0 = (uint64)&kernel_stack_top;
    tss.ist1 = (uint64)&ist_df_stack_top;

    // disable IO bitmap
    tss.io_map = sizeof(TSS);
}