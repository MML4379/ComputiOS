#include <libk/types.hpp>

alignas(16) static uint8 kernel_stack[16 * 1024];
alignas(16) static uint8 ist_df_stack[16 * 1024];

extern "C" uint8 kernel_stack_top = kernel_stack[sizeof(kernel_stack)];
extern "C" uint8 ist_df_stack_top = ist_df_stack[sizeof(ist_df_stack)];