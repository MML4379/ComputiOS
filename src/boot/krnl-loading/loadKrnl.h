#pragma once

#ifndef LOAD_KRNL_H
#define LOAD_KRNL_H

#define PROPER_COMPLETION            0x00

#define ERROR_GENERAL_KERNEL         0x10
#define ERROR_KERNEL_INITIALIZATION  0x11
#define ERROR_KERNEL_PROCESS         0x12
#define ERROR_KERNEL_COMMUNICATION   0x13
#define ERROR_KERNEL_SECURITY        0x14
#define ERROR_KERNEL_MEMORY          0x15
#define ERROR_KERNEL_EXECUTION       0x16
#define ERROR_KERNEL_RESOURCE        0x17
#define ERROR_KERNEL_CONFIGURATION   0x18
#define ERROR_KERNEL_INTERRUPT       0x19

#define ERROR_SYSTEM_FILE            0x20
#define ERROR_SYSTEM_FILE_NOT_FOUND  0x21
#define ERROR_SYSTEM_FILE_CORRUPTION 0x22
#define ERROR_SYSTEM_FILE_PERMISSION 0x23
#define ERROR_SYSTEM_FILE_FORMAT     0x24
#define ERROR_SYSTEM_FILE_SIZE       0x25
#define ERROR_SYSTEM_FILE_COMPAT     0x26
#define ERROR_SYSTEM_FILE_READ       0x27
#define ERROR_SYSTEM_FILE_WRITE      0x28
#define ERROR_SYSTEM_FILE_DELETE     0x29

#define ERROR_DISK                   0x30
#define ERROR_DISK_NOT_FOUND         0x31
#define ERROR_DISK_FULL              0x32
#define ERROR_DISK_FORMAT            0x33
#define ERROR_DISK_READ              0x34
#define ERROR_DISK_WRITE             0x35
#define ERROR_DISK_SEEK              0x36
#define ERROR_DISK_NOT_READY         0x37
#define ERROR_DISK_EJECT             0x38
#define ERROR_DISK_INITIALIZATION    0x39

#define ERROR_GPU                    0x40
#define ERROR_GPU_NOT_FOUND          0x41
#define ERROR_GPU_COMPATIBILITY      0x42
#define ERROR_GPU_MEMORY             0x43
#define ERROR_GPU_PROCESSING         0x44
#define ERROR_GPU_OVERHEATING        0x45
#define ERROR_GPU_DRIVER             0x46
#define ERROR_GPU_DRIVER_INIT        0x47
#define ERROR_GPU_RESOURCE           0x48
#define ERROR_GPU_COMMUNICATION      0x49

#define ERROR_MEMORY                 0x50
#define ERROR_MEMORY_ALLOC           0x51
#define ERROR_MEMORY_DEALLOC         0x52
#define ERROR_MEMORY_ACCESS_VIOLATION 0x53
#define ERROR_MEMORY_CORRUPTION      0x54
#define ERROR_MEMORY_FRAGMENTATION   0x55
#define ERROR_MEMORY_SIZE            0x56
#define ERROR_MEMORY_INIT            0x57
#define ERROR_MEMORY_MAPPING         0x58
#define ERROR_MEMORY_HARDWARE        0x59

#define ERROR_DRIVER                 0x60
#define ERROR_DRIVER_NOT_FOUND       0x61
#define ERROR_DRIVER_COMPATIBILITY   0x62
#define ERROR_DRIVER_INSTALL         0x63
#define ERROR_DRIVER_UPDATE          0x64
#define ERROR_DRIVER_REMOVE          0x65
#define ERROR_DRIVER_CONFIG          0x66
#define ERROR_DRIVER_INIT            0x67
#define ERROR_DRIVER_RESOURCE        0x68
#define ERROR_DRIVER_COMMUNICATION   0x69

#define ERROR_DRIVER_INIT            0x70
#define ERROR_DRIVER_NOT_FOUND       0x71
#define ERROR_DRIVER_COMPATIBILITY   0x72
#define ERROR_DRIVER_INSTALL         0x73
#define ERROR_DRIVER_UPDATE          0x74
#define ERROR_DRIVER_REMOVE          0x75
#define ERROR_DRIVER_CONFIG          0x76
#define ERROR_DRIVER_COMMUNICATION   0x77
#define ERROR_DRIVER_RESOURCE        0x78
#define ERROR_DRIVER_EXECUTION       0x79

#define ERROR_NOT_ENOUGH_RESOURCES   0x80
#define ERROR_INSUFFICIENT_MEMORY    0x81
#define ERROR_INSUFFICIENT_STORAGE   0x82
#define ERROR_INSUFFICIENT_POWER     0x83
#define ERROR_INSUFFICIENT_GRAPHICS  0x84
#define ERROR_INSUFFICIENT_NETWORK   0x85
#define ERROR_INSUFFICIENT_SYSTEM    0x86
#define ERROR_INSUFFICIENT_DRIVER    0x87
#define ERROR_INSUFFICIENT_PERIPHERAL 0x88
#define ERROR_INSUFFICIENT_OVERALL   0x89

#define ERROR_CPU                    0x90
#define ERROR_CPU_NOT_FOUND          0x91
#define ERROR_CPU_COMPATIBILITY      0x92
#define ERROR_CPU_EXECUTION          0x93
#define ERROR_CPU_OVERHEATING        0x94
#define ERROR_CPU_CACHE              0x95
#define ERROR_CPU_INSTRUCTION_SET    0x96
#define ERROR_CPU_REGISTER           0x97
#define ERROR_CPU_POWER              0x98
#define ERROR_CPU_COMMUNICATION      0x99

void loadKrnl();
void loadDrivers();
void unloadDrivers();
void showBootMenu();
void enableKeyboard();
void errorHalt(int errCode, const char* errText);
void checkForKernel();

#endif // LOAD_KRNL_H