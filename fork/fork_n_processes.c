#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define N 100 // Number of processes

int main()
{
    FILE *file;
    pid_t pid;
    int i;

    // Open a file for writing
    file = fopen("output.txt", "w");
    if (file == NULL)
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
            if (i == 0)
            {
                // Only one child (the first one) writes to the file
                fprintf(file, "Child %d writing to the file.\n", i);
                fflush(file); // Ensure the output is written immediately
            }
            else
            {
                // Dummy work for other child processes
                printf("Child %d doing some other work.\n", i);
            }
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
    if (fclose(file) == EOF)
    {
        perror("Failed to close file");
        exit(EXIT_FAILURE);
    }

    printf("All processes completed and file closed.\n");

    return 0;
}