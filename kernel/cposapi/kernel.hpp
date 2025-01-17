#include "../../apis/libc/libc.h"

typedef struct Process {
    int pid;
    int ppid;
    char name[32];
    int state;
    int priority;
    int queue_level;
    unsigned long time_in_current_level;
    unsigned long quantum;
    unsigned long last_exec_time;
    unsigned long cpu_time;
    unsigned long arrival_time;
    unsigned long burst_time;
    unsigned long wait_time;
    int demotion_count;
    unsigned long memory_base;
    unsigned long memory_limit;
    int *page_table;
    int open_files[1024];
    void *registers;
    unsigned long program_counter;
    int signals;
    int exit_code;
    int user_id;
    int group_id;
    int cpu_affinity;
    char file_path[256];
} Process;

typedef struct Queue {
    Process *processes[100];
    int front;
    int rear;
} Queue;