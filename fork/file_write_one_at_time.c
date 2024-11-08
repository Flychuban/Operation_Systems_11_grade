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

    // Open a file for writing (done in the parent process)
    file = fopen("output_race.txt", "w");
    if (file == NULL)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Create N child processes using fork(), but wait for each to finish
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
            // Each child process writes to the file
            fprintf(file, "Child %d writing to the file.\n", i);
            fflush(file); // Ensure the output is written immediately
            printf("Child %d wrote to the file.\n", i);
            exit(EXIT_SUCCESS); // Exit the child process after work is done
        }
        else
        {
            // Parent process waits for the child to finish before forking another
            waitpid(pid, NULL, 0); // Wait for the child process to finish
        }
    }

    // Close the file after all work is done in the parent process
    if (fclose(file) == EOF)
    {
        perror("Failed to close file");
        exit(EXIT_FAILURE);
    }

    printf("All processes completed and file closed.\n");

    return 0;
}
