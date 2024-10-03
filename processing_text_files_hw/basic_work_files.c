#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *file;
    char buffer[100];

    // Отваряне на файл за четене
    file = fopen("example.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    // Четене на всеки ред от файла и отпечатване на екрана
    printf("Content of the file:\n");
    while (fgets(buffer, 100, file) != NULL)
    {
        printf("%s", buffer);
    }

    // Затваряне на файла след четене
    fclose(file);

    // Отваряне на файл за дописване
    file = fopen("example.txt", "a");
    if (file == NULL)
    {
        perror("Error opening file for writing");
        return 1;
    }

    // Добавяне на нов ред към файла
    fputs("New line added to the file.\n", file);

    // Затваряне на файла след писане
    fclose(file);

    return 0;
}