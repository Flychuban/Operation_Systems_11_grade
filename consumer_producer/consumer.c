#include "common.h"

// Global variables for cleanup
sem_t *sem_empty, *sem_full, *sem_mutex;
int shm_fd;
SharedMemory *shared_mem;
volatile sig_atomic_t running = 1;
long processed_tasks = 0; // Counter for processed tasks

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

// Helper function to safely wait on semaphore
void safe_sem_wait(sem_t *sem, const char *msg)
{
    while (sem_wait(sem) == -1)
    {
        if (errno != EINTR)
        {
            handle_error(msg);
        }
    }
}

// Helper function to safely post to semaphore
void safe_sem_post(sem_t *sem, const char *msg)
{
    if (sem_post(sem) == -1)
    {
        handle_error(msg);
    }
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
        if (!atomic_load(&shared_mem->producer_active))
        {
            printf("Producer is no longer active. Consumer shutting down...\n");
            break;
        }

        // Wait for a batch of tasks
        for (int i = 0; i < BATCH_SIZE && running; i++)
        {
            safe_sem_wait(sem_full, "sem_wait full failed");
        }

        // Acquire mutex once for the entire batch
        safe_sem_wait(sem_mutex, "sem_wait mutex failed");

        // Process a batch of tasks
        int current_out = atomic_load(&shared_mem->out);
        for (int i = 0; i < BATCH_SIZE && running; i++)
        {
            Task task = shared_mem->buffer[current_out];
            current_out = (current_out + 1) % BUFFER_SIZE;

            // Process the task (minimal processing as per requirements)
            if (task.type == 0)
            {
                // Simple task - just consume it
                (void)atomic_load(&task.data.simple_data);
            }
            else
            {
                // Complex task - just consume it
                (void)task.data.complex_data;
            }

            processed_tasks++;

            // Log every 10,000 tasks
            if (processed_tasks % 10000 == 0)
            {
                printf("Processed %ld tasks\n", processed_tasks);
            }
        }
        atomic_store(&shared_mem->out, current_out);

        safe_sem_post(sem_mutex, "sem_post mutex failed");

        // Signal empty slots for the entire batch
        for (int i = 0; i < BATCH_SIZE && running; i++)
        {
            safe_sem_post(sem_empty, "sem_post empty failed");
        }
    }

    printf("Consumer shutting down after processing %ld tasks\n", processed_tasks);
    cleanup_resources();
    return 0;
}