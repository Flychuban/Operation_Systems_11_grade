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
#include <stdatomic.h>

// Constants
#define BUFFER_SIZE 10000 // Increased buffer size for better throughput
#define SHM_NAME "/task_queue"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"
#define SEM_MUTEX "/sem_mutex"
#define BATCH_SIZE 64 // Process tasks in batches

// Task structure - aligned for better cache performance
typedef struct __attribute__((aligned(64)))
{
    atomic_int type; // 0 for simple, 1 for complex
    union
    {
        atomic_int simple_data;
        char complex_data[256]; // For complex tasks (strings/JSON)
    } data;
} Task;

// Shared memory structure - aligned for better cache performance
typedef struct __attribute__((aligned(64)))
{
    Task buffer[BUFFER_SIZE];
    atomic_int in;              // Producer position
    atomic_int out;             // Consumer position
    atomic_int producer_active; // Flag to indicate if producer is running
    atomic_long total_tasks;    // Total tasks produced
    atomic_long start_time;     // Start time for performance measurement
} SharedMemory;

// Function declarations
void cleanup_resources(void);
void handle_error(const char *msg);

#endif // COMMON_H