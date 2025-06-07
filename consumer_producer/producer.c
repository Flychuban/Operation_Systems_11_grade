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
    {
        sem_close(sem_empty);
        sem_unlink(SEM_EMPTY);
    }
    if (sem_full)
    {
        sem_close(sem_full);
        sem_unlink(SEM_FULL);
    }
    if (sem_mutex)
    {
        sem_close(sem_mutex);
        sem_unlink(SEM_MUTEX);
    }
    if (shared_mem)
        atomic_store(&shared_mem->producer_active, 0); // Signal consumers to stop
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
    atomic_store(&shared_mem->producer_active, 1);
    atomic_store(&shared_mem->start_time, (long)time(NULL));

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
        // Create a batch of tasks
        Task tasks[BATCH_SIZE];
        for (int i = 0; i < BATCH_SIZE && running; i++)
        {
            tasks[i].type = rand() % 2;
            if (tasks[i].type == 0)
            {
                atomic_store(&tasks[i].data.simple_data, rand());
            }
            else
            {
                snprintf(tasks[i].data.complex_data, sizeof(tasks[i].data.complex_data),
                         "Task-%ld", atomic_load(&shared_mem->total_tasks) + i);
            }
        }

        // Wait for empty slots for the entire batch
        for (int i = 0; i < BATCH_SIZE && running; i++)
        {
            safe_sem_wait(sem_empty, "sem_wait empty failed");
        }

        // Acquire mutex once for the entire batch
        safe_sem_wait(sem_mutex, "sem_wait mutex failed");

        // Add tasks to buffer
        int current_in = atomic_load(&shared_mem->in);
        for (int i = 0; i < BATCH_SIZE && running; i++)
        {
            shared_mem->buffer[current_in] = tasks[i];
            current_in = (current_in + 1) % BUFFER_SIZE;
            atomic_fetch_add(&shared_mem->total_tasks, 1);
        }
        atomic_store(&shared_mem->in, current_in);

        // Print performance metrics every 10,000 tasks
        long total = atomic_load(&shared_mem->total_tasks);
        if (total % 10000 == 0)
        {
            time_t current_time = time(NULL);
            double elapsed = difftime(current_time, (time_t)atomic_load(&shared_mem->start_time));
            double tasks_per_sec = total / elapsed;
            double ms_per_task = (elapsed * 1000) / total;
            printf("%.0f tasks/sec, %.4fms/task\n", tasks_per_sec, ms_per_task);
        }

        safe_sem_post(sem_mutex, "sem_post mutex failed");

        // Signal full slots for the entire batch
        for (int i = 0; i < BATCH_SIZE && running; i++)
        {
            safe_sem_post(sem_full, "sem_post full failed");
        }
    }

    printf("\nShutting down producer...\n");
    cleanup_resources();
    return 0;
}