#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h> // For file locking (fcntl)
#include <string.h>

#define N 5 // Number of processes

void lock_file(int fd)
{
    struct flock lock;
    lock.l_type = F_WRLCK;    // Write lock
    lock.l_whence = SEEK_SET; // Lock the entire file
    lock.l_start = 0;
    lock.l_len = 0; // 0 means to lock the entire file

    // Apply the lock
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("Failed to acquire file lock");
        exit(EXIT_FAILURE);
    }
}

void unlock_file(int fd)
{
    struct flock lock;
    lock.l_type = F_UNLCK; // Unlock the file
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    // Release the lock
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("Failed to release file lock");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int fd;
    pid_t pid;
    int i;

    // Open a file for writing using low-level open() system call (required for fcntl locks)
    fd = open("output_race_locked.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Create N child processes using fork(), allowing them to work concurrently but with file locks
    for (i = 0; i < N; i++)
    {
        pid = fork();

        if (pid < 0)
        { // Check for error in fork()
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        { // Child process
            // Acquire the lock before writing to the file
            lock_file(fd);

            // Write to the file after locking it
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "Child %d writing to the file.\n", i);
            write(fd, buffer, strlen(buffer));

            // Flush the output to ensure it is written immediately
            fsync(fd);

            // Release the lock after writing
            unlock_file(fd);

            // Inform that the child wrote to the file
            printf("Child %d wrote to the file.\n", i);
            exit(EXIT_SUCCESS); // Exit the child process after work is done
        }
    }

    // Parent process waits for all child processes to finish
    for (i = 0; i < N; i++)
    {
        pid_t wpid = wait(NULL);
        if (wpid == -1)
        {
            perror("Wait failed");
            exit(EXIT_FAILURE);
        }
    }

    // Close the file after all work is done
    if (close(fd) == -1)
    {
        perror("Failed to close file");
        exit(EXIT_FAILURE);
    }

    printf("All processes completed and file closed.\n");

    return 0;
}
