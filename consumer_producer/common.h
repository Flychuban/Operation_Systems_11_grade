#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

// Constants
#define BUFFER_SIZE 1000
#define SHM_NAME "/task_queue"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"
#define SEM_MUTEX "/sem_mutex"

// Task structure
typedef struct
{
    int type; // 0 for simple, 1 for complex
    union
    {
        int simple_data;
        char complex_data[256]; // For complex tasks (strings/JSON)
    } data;
} Task;

// Shared memory structure
typedef struct
{
    Task buffer[BUFFER_SIZE];
    int in;              // Producer position
    int out;             // Consumer position
    int producer_active; // Flag to indicate if producer is running
    long total_tasks;    // Total tasks produced
    time_t start_time;   // Start time for performance measurement
} SharedMemory;

// Function declarations
void cleanup_resources(void);
void handle_error(const char *msg);

#endif // COMMON_H