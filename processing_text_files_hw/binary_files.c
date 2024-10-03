#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char name[50];
    int age;
    float salary;
} Employee;

int main()
{
    FILE *file;
    Employee emp = {"John Doe", 30, 50000.0};

    // Отваряме файл за запис в двоичен режим
    file = fopen("employee.dat", "wb");
    if (file == NULL)
    {
        perror("Error opening file for writing");
        return 1;
    }

    // Пишем структурата Employee в двоичен файл
    fwrite(&emp, sizeof(Employee), 1, file);

    // Затваряме файла
    fclose(file);

    // Отваряме файла за четене в двоичен режим
    file = fopen("employee.dat", "rb");
    if (file == NULL)
    {
        perror("Error opening file for reading");
        return 1;
    }

    // Четем структурата Employee от файла
    Employee emp_read;
    fread(&emp_read, sizeof(Employee), 1, file);
    printf("Name: %s, Age: %d, Salary: %.2f\n", emp_read.name, emp_read.age, emp_read.salary);

    fclose(file);

    return 0;
}