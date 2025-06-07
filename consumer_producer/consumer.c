#include "common.h"

// Global variables for cleanup
sem_t *sem_empty, *sem_full, *sem_mutex;
int shm_fd;
SharedMemory *shared_mem;
volatile sig_atomic_t running = 1;

void signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
    {
        running = 0;
    }
}

void cleanup_resources(void)
{
    if (sem_empty)
        sem_close(sem_empty);
    if (sem_full)
        sem_close(sem_full);
    if (sem_mutex)
        sem_close(sem_mutex);
    if (shared_mem)
        munmap(shared_mem, sizeof(SharedMemory));
    if (shm_fd != -1)
        close(shm_fd);
}

void handle_error(const char *msg)
{
    perror(msg);
    cleanup_resources();
    exit(EXIT_FAILURE);
}

int main(void)
{
    // Set up signal handling
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1)
    {
        handle_error("sigaction failed");
    }

    // Open shared memory
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
        handle_error("shm_open failed");

    shared_mem = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED)
        handle_error("mmap failed");

    // Open semaphores
    sem_empty = sem_open(SEM_EMPTY, 0);
    sem_full = sem_open(SEM_FULL, 0);
    sem_mutex = sem_open(SEM_MUTEX, 0);

    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED || sem_mutex == SEM_FAILED)
        handle_error("sem_open failed");

    printf("Consumer started. Press Ctrl+C to stop.\n");

    // Main consumer loop
    while (running)
    {
        // Check if producer is still active
        if (!shared_mem->producer_active)
        {
            printf("Producer is no longer active. Consumer shutting down...\n");
            break;
        }

        // Wait for a task
        sem_wait(sem_full);
        sem_wait(sem_mutex);

        // Get task from buffer
        Task task = shared_mem->buffer[shared_mem->out];
        shared_mem->out = (shared_mem->out + 1) % BUFFER_SIZE;

        sem_post(sem_mutex);
        sem_post(sem_empty);

        // Process the task
        if (task.type == 0)
        {
            // Simple task
            printf("Processed simple task: %d\n", task.data.simple_data);
        }
        else
        {
            // Complex task
            printf("Processed complex task: %s\n", task.data.complex_data);
        }
    }

    printf("Consumer shutting down...\n");
    cleanup_resources();
    return 0;
}