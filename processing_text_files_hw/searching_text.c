#include <stdio.h>
#include <string.h>

int main()
{
    FILE *file = fopen("example.txt", "r"); // Отваряме файла за четене
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    char searchTerm[] = "търсен текст"; // Текстът, който търсим
    char line[256];                     // Буфер за всяка редица от файла
    int found = 0;                      // Флаг за открит текст

    // Четене на файла ред по ред
    while (fgets(line, sizeof(line), file))
    {
        // Проверка дали търсеният текст се съдържа в текущия ред
        if (strstr(line, searchTerm) != NULL)
        {
            printf("Found the text: %s\n", line);
            found = 1; // Означаваме, че сме намерили текста
        }
    }

    if (!found)
    {
        printf("Text not found in the file.\n");
    }

    fclose(file); // Затваряме файла
    return 0;
}