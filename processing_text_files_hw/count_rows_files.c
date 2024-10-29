#include <stdio.h>

int main()
{
    FILE *file = fopen("example.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int lineCount = 0;
    char ch;

    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
        {
            lineCount++;
        }
    }

    if (ch != '\n' && lineCount > 0)
    {
        lineCount++;
    }

    printf("The file contains %d lines.\n", lineCount);

    fclose(file);
    return 0;
}