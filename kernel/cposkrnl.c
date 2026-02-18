/**
 *    ComputiOS Kernel - By Michael Peace
 *    https://github.com/MML4379/ComputiOS
 *    https://cpos.mmltech.net/kernel
 * 
 */

#include "include/types.h"
#include "include/bootinfo.h"

// Forward declaration for main loop
static void main_loop(void);

// Kernel entry point
void cposkrnl(BootInfo *boot_info) {
    // validate boot info
}