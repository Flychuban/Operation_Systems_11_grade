#include <stdio.h>

int main()
{
    FILE *file = fopen("test.txt", "w");

    // Задаване на пълно буфериране с размер на буфера 1024 байта
    char buffer[1024];
    setvbuf(file, buffer, _IOFBF, 1024);

    // Писане на данни във файла
    fputs("This is a test string.\n", file);

    // Затваряне на файла
    fclose(file);

    return 0;
}