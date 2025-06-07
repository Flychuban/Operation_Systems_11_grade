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
        shared_mem->producer_active = 0; // Signal consumers to stop
    munmap(shared_mem, sizeof(SharedMemory));
    if (shm_fd != -1)
        close(shm_fd);
    shm_unlink(SHM_NAME);
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

    // Initialize shared memory
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
        handle_error("shm_open failed");

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1)
        handle_error("ftruncate failed");

    shared_mem = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED)
        handle_error("mmap failed");

    // Initialize shared memory structure
    memset(shared_mem, 0, sizeof(SharedMemory));
    shared_mem->producer_active = 1;
    shared_mem->start_time = time(NULL);

    // Initialize semaphores
    sem_empty = sem_open(SEM_EMPTY, O_CREAT, 0666, BUFFER_SIZE);
    sem_full = sem_open(SEM_FULL, O_CREAT, 0666, 0);
    sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);

    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED || sem_mutex == SEM_FAILED)
        handle_error("sem_open failed");

    printf("Producer started. Press Ctrl+C to stop.\n");

    // Main producer loop
    while (running)
    {
        // Create a new task
        Task task;
        task.type = rand() % 2; // Randomly choose task type

        if (task.type == 0)
        {
            task.data.simple_data = rand(); // Simple task: random integer
        }
        else
        {
            // Complex task: random string
            snprintf(task.data.complex_data, sizeof(task.data.complex_data),
                     "Task-%ld", shared_mem->total_tasks);
        }

        // Wait for empty slot
        sem_wait(sem_empty);
        sem_wait(sem_mutex);

        // Add task to buffer
        shared_mem->buffer[shared_mem->in] = task;
        shared_mem->in = (shared_mem->in + 1) % BUFFER_SIZE;
        shared_mem->total_tasks++;

        // Print performance metrics every 10,000 tasks
        if (shared_mem->total_tasks % 10000 == 0)
        {
            time_t current_time = time(NULL);
            double elapsed = difftime(current_time, shared_mem->start_time);
            double tasks_per_sec = shared_mem->total_tasks / elapsed;
            double ms_per_task = (elapsed * 1000) / shared_mem->total_tasks;
            printf("%.0f tasks/sec, %.4fms/task\n", tasks_per_sec, ms_per_task);
        }

        sem_post(sem_mutex);
        sem_post(sem_full);
    }

    printf("\nShutting down producer...\n");
    cleanup_resources();
    return 0;
}