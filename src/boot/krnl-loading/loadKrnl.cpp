/**
    ComputiOS Loader
    Copyright (C) 2024 MML Tech LLC
    Licensed under the MIT license.
*/

#include "loadKrnl.h"
#include "longMode.h"
#include "../boot-drivers/vga/video.h"

void checkForKernel() {
    
}

void loadKrnl() {
    initializeVideoDriver(); // Initialize the basic VGA driver
    switchToLongMode(); // Switch to long mode before initializing the kernel
}

void loadDrivers() {

}

void unloadDrivers() {

}

void showBootMenu() {

}

void enableKeyboard() {

}

void errorHalt(int errCode, const char* errText) {
    // Clear the screen
    clearScreen();
    
    // Display error code
    writeString("Error code: ");
    writeInteger(errCode);
    writeString(" - Reason: ");
    writeString(errText);
}

