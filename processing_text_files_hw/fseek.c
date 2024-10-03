#include <stdio.h>

int main()
{
    FILE *file = fopen("example.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    // Преместваме указателя 10 байта от началото на файла
    fseek(file, 10, SEEK_SET);

    // Прочитаме един символ от текущата позиция
    char c = fgetc(file);
    printf("Character at position 10: %c\n", c);

    // Вземаме текущата позиция на указателя
    long position = ftell(file);
    printf("Current file position: %ld\n", position);

    fclose(file);
    return 0;
}