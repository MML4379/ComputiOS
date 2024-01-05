#ifndef KERNEL_H
#define KERNEL_H

void init();
void startProc(const char* location, int& pid);

namespace cposkrnl {};

#endif