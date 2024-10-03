#include <stdio.h>

int main()
{
    FILE *file = fopen("example.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    // Прочитаме първите 5 символа
    char buffer[6];
    fread(buffer, 1, 5, file);
    buffer[5] = '\0';
    printf("First 5 characters: %s\n", buffer);

    // Преместваме указателя 10 байта напред
    fseek(file, 10, SEEK_CUR);
    printf("Current position: %ld\n", ftell(file));

    // Прочитаме следващите 5 символа
    fread(buffer, 1, 5, file);
    buffer[5] = '\0';
    printf("Next 5 characters: %s\n", buffer);

    // Връщаме се в началото на файла
    rewind(file);
    printf("Position after rewind: %ld\n", ftell(file));

    fclose(file);
    return 0;
}