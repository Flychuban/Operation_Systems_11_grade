#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>  // For open() and write()
#include <string.h> // For strlen()

#define N 5 // Number of processes

int main()
{
    int file;
    pid_t pid;
    int i;

    // Open a file for writing using low-level system calls (open and write)
    file = open("output_race_lowlevel.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Create N child processes using fork()
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
            // Write using low-level write() system call in chunks
            char buffer1[20];
            char buffer2[30];
            char buffer3[10];

            // Preparing text fragments
            snprintf(buffer1, sizeof(buffer1), "Child %d ", i);
            snprintf(buffer2, sizeof(buffer2), "writing to the ");
            snprintf(buffer3, sizeof(buffer3), "file.\n");

            // Perform the writes using strlen() to ensure only valid parts are written
            write(file, buffer1, strlen(buffer1)); // Write the first part
            write(file, buffer2, strlen(buffer2)); // Write the second part
            write(file, buffer3, strlen(buffer3)); // Write the final part

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
    if (close(file) == -1)
    {
        perror("Failed to close file");
        exit(EXIT_FAILURE);
    }

    printf("All processes completed and file closed.\n");

    return 0;
}
