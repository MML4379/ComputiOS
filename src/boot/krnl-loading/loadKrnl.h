#pragma once

#ifndef LOAD_KRNL_H
#define LOAD_KRNL_H

void loadKrnl();
void loadDrivers();
void unloadDrivers();
void showBootMenu();
void enableKeyboard();
void errorHalt(int errCode, const char* errText);
void checkForKernel();

#endif // LOAD_KRNL_H